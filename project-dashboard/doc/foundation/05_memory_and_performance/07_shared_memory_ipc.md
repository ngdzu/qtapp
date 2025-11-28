# Shared Memory IPC with memfd and Unix Domain Sockets

**Category:** Memory and Performance  
**Related Documents:** [Sensor Integration Summary](../../z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md) (see section "Understanding memfd and Socket Handshake Architecture")

---

## Overview

This document explains the architecture pattern of using **shared memory** (`memfd`) for high-performance data transfer combined with **Unix domain sockets** for control operations and file descriptor passing. This pattern achieves sub-16ms latency for real-time sensor data while maintaining security and process isolation.

---

## Key Concepts

### memfd (Memory File Descriptor)

**`memfd_create`** is a Linux system call (since kernel 3.17) that creates an anonymous file descriptor backed by RAM.

**Advantages over traditional `shm_open`:**
- ✅ **No filesystem namespace pollution** - No entries in `/dev/shm/`
- ✅ **Automatic cleanup** - Memory freed when last FD is closed
- ✅ **Better security** - Not accessible via filesystem paths
- ✅ **Sealing support** - Can prevent modifications after initialization

**Example:**
```cpp
#include <sys/mman.h>
#include <sys/memfd.h>
#include <unistd.h>
#include <fcntl.h>

// Create anonymous shared memory
int memfdFd = memfd_create("my-ring-buffer", MFD_CLOEXEC | MFD_ALLOW_SEALING);

// Set size
size_t bufferSize = 1024 * 1024; // 1 MB
ftruncate(memfdFd, bufferSize);

// Map into process address space
void* memory = mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, 
                    MAP_SHARED, memfdFd, 0);
```

### Unix Domain Sockets with SCM_RIGHTS

**Problem:** File descriptors cannot be passed through shared memory itself.

**Solution:** Unix domain sockets support `SCM_RIGHTS` ancillary data to pass file descriptors between processes.

**Why This is Necessary:**
- File descriptors are process-local identifiers
- Cannot be serialized and passed through shared memory
- `SCM_RIGHTS` is the standard Linux mechanism for FD passing

**Example: Sending a file descriptor**
```cpp
#include <sys/socket.h>
#include <sys/un.h>

// Send file descriptor via Unix domain socket
struct msghdr msg = {0};
struct iovec iov;
char control_buf[CMSG_SPACE(sizeof(int))];
char data = 0;

iov.iov_base = &data;
iov.iov_len = 1;
msg.msg_iov = &iov;
msg.msg_iovlen = 1;

// Prepare ancillary data for file descriptor
msg.msg_control = control_buf;
msg.msg_controllen = sizeof(control_buf);

struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
cmsg->cmsg_level = SOL_SOCKET;
cmsg->cmsg_type = SCM_RIGHTS;
cmsg->cmsg_len = CMSG_LEN(sizeof(int));
memcpy(CMSG_DATA(cmsg), &memfdFd, sizeof(int));

sendmsg(socketFd, &msg, 0);
```

**Example: Receiving a file descriptor**
```cpp
struct msghdr msg = {0};
struct iovec iov;
char buffer[1];
char cmsg_buffer[CMSG_SPACE(sizeof(int))];

iov.iov_base = buffer;
iov.iov_len = sizeof(buffer);
msg.msg_iov = &iov;
msg.msg_iovlen = 1;
msg.msg_control = cmsg_buffer;
msg.msg_controllen = sizeof(cmsg_buffer);

ssize_t received = recvmsg(socketFd, &msg, 0);

// Extract file descriptor
struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
if (cmsg && cmsg->cmsg_level == SOL_SOCKET && 
    cmsg->cmsg_type == SCM_RIGHTS) {
    int fd;
    memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
    // Use fd to mmap shared memory
}
```

---

## Architecture Pattern: Control Channel + Data Channel

This is a **standard high-performance IPC pattern**:

### Control Channel (Socket)
- **Purpose:** Setup/teardown, passing metadata, file descriptors
- **Frequency:** Low (one-time handshake, occasional control messages)
- **Latency:** Not critical (happens during connection setup)

### Data Channel (Shared Memory)
- **Purpose:** High-frequency data transfer
- **Frequency:** High (60-250 Hz for sensor data)
- **Latency:** Critical (< 16ms requirement)
- **Mechanism:** Direct memory access, zero-copy

### Why This Pattern?

1. **Separation of Concerns:** Control operations are separate from data transfer
2. **Performance:** Data transfer avoids syscall overhead
3. **Scalability:** Shared memory supports multiple readers efficiently
4. **Latency:** Direct memory access achieves < 16ms vs. > 60ms for socket-based transfer

---

## Performance Comparison

| Approach | Latency | Throughput | CPU Overhead | Use Case |
|----------|---------|------------|--------------|----------|
| **WebSocket + JSON** | > 60ms | ~100 KB/s | High | Web applications, remote access |
| **Unix Socket + Binary** | ~20-30ms | ~1 MB/s | Medium | Local IPC, moderate latency OK |
| **Shared Memory (memfd)** | < 16ms ✅ | ~10+ MB/s | Low | Real-time systems, high-frequency data |

### Why Shared Memory Achieves < 16ms Latency

1. **Zero-Copy:** Direct memory write, no copying between processes
2. **No Syscalls on Hot Path:** After initial `mmap()`, pure memory operations
3. **CPU Cache Friendly:** Stays in L3 cache, avoiding main memory access
4. **Lock-Free Ring Buffer:** Atomic operations, no mutex contention

### Latency Breakdown

**WebSocket Overhead:**
- JSON serialization: ~5-10ms
- Network stack (TCP/IP): ~10-20ms
- Socket I/O syscalls: ~5-10ms per frame
- JSON deserialization: ~5-10ms
- **Total: > 60ms** ❌

**Shared Memory Overhead:**
- Direct memory write: ~0.1µs
- Atomic index update: ~0.1µs
- Memory read (cache hit): ~0.1µs
- Frame decode: ~200µs
- **Total: < 16ms** ✅

---

## Security Considerations

### memfd Security

1. **File Descriptor Isolation:** Not accessible via filesystem paths
2. **Permission Model:** Set via `fcntl()` (e.g., `0600` for owner-only)
3. **Sealing:** Can prevent modifications after initialization
4. **Automatic Cleanup:** Memory freed when all FDs are closed

### Socket Security

1. **Unix Domain Socket Permissions:** Filesystem permissions (use `0600` in production)
2. **SCM_RIGHTS Limitations:** FD permissions inherited from original
3. **Process Isolation:** Only processes that can connect receive the FD

### Best Practices

- ✅ Create `memfd` with `0600` permissions
- ✅ Use Unix domain socket with `0600` permissions
- ✅ Validate `magic` number and `version` before trusting data
- ✅ Use CRC32 validation on each frame
- ✅ Implement heartbeat mechanism for stall detection

---

## Common Patterns

### Pattern 1: Ring Buffer for High-Frequency Data

```cpp
struct RingBufferHeader {
    uint32_t magic;
    uint32_t version;
    std::atomic<uint64_t> writeIndex;
    std::atomic<uint64_t> heartbeatTimestamp;
    uint32_t frameSize;
    uint32_t frameCount;
};

// Writer
void writeFrame(const Frame& frame) {
    uint64_t nextIndex = writeIndex.load() + 1;
    uint64_t slot = nextIndex % frameCount;
    memcpy(buffer + slot * frameSize, &frame, frameSize);
    writeIndex.store(nextIndex, std::memory_order_release);
}

// Reader
void pollFrames() {
    uint64_t currentRead = readIndex.load();
    uint64_t currentWrite = writeIndex.load(std::memory_order_acquire);
    
    while (currentRead < currentWrite) {
        uint64_t slot = currentRead % frameCount;
        Frame frame;
        memcpy(&frame, buffer + slot * frameSize, frameSize);
        processFrame(frame);
        currentRead++;
    }
    readIndex.store(currentRead);
}
```

### Pattern 2: Multiple Readers

Shared memory naturally supports multiple readers:
- Each reader maintains its own `readIndex`
- Writer's `writeIndex` is shared (atomic)
- No coordination needed between readers

### Pattern 3: Heartbeat for Stall Detection

```cpp
// Writer updates heartbeat every frame
heartbeatTimestamp.store(getCurrentTime(), std::memory_order_release);

// Reader checks for stalls
uint64_t lastHeartbeat = heartbeatTimestamp.load(std::memory_order_acquire);
uint64_t now = getCurrentTime();
if (now - lastHeartbeat > STALL_THRESHOLD_MS) {
    // Writer is stalled or disconnected
    handleStall();
}
```

---

## When to Use This Pattern

**Use shared memory + socket when:**
- ✅ Real-time data transfer required (< 20ms latency)
- ✅ High-frequency data (60+ Hz)
- ✅ Local processes (same machine)
- ✅ Zero-copy performance critical
- ✅ Multiple readers needed

**Don't use when:**
- ❌ Remote processes (use network protocols)
- ❌ Low-frequency data (socket overhead acceptable)
- ❌ Cross-platform required (memfd is Linux-specific)
- ❌ Security isolation required (shared memory is accessible to all readers)

---

## References

- **Linux `memfd_create` man page:** `man 2 memfd_create`
- **Unix domain sockets:** `man 7 unix`
- **SCM_RIGHTS:** `man 7 socket`
- **Z-Monitor Implementation:** `doc/z-monitor/architecture_and_design/37_SENSOR_INTEGRATION.md`

---

## Related Foundation Documents

- `01_memory_management.md` - General memory management principles
- `03_cache_optimization.md` - CPU cache optimization techniques
- `04_concurrency_and_threading/01_thread_safety.md` - Thread-safe shared memory access

