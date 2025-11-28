# Low Latency Techniques and Tools

**Document ID:** DESIGN-042  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides a comprehensive guide to low-latency techniques, tools, and libraries for the Z Monitor application. It covers when to use specific techniques, which libraries to choose, and implementation guidelines for critical path operations.

> **📋 Related Documents:**
> - [Thread Model (12_THREAD_MODEL.md)](./12_THREAD_MODEL.md) - Latency targets and thread architecture ⭐
> - [Memory & Resource Management (23_MEMORY_RESOURCE_MANAGEMENT.md)](./23_MEMORY_RESOURCE_MANAGEMENT.md) - Memory management patterns
> - [Data Caching Strategy (36_DATA_CACHING_STRATEGY.md)](./36_DATA_CACHING_STRATEGY.md) - Critical path caching
> - [Performance Requirements](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md) - Latency targets and performance requirements
> - [Benchmark Strategy (40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md)](./40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md) - Performance measurement

---

## 1. Overview

The Z Monitor application has strict latency requirements, particularly for alarm detection (< 50ms). This document provides:

- **Low-latency techniques** for critical path operations
- **Tool and library recommendations** with performance characteristics
- **Decision guidelines** for when to use which technique
- **Implementation plans** for Z Monitor-specific scenarios

### 1.1 Critical Latency Targets

| Operation | Target Latency | Criticality | Path |
|-----------|---------------|-------------|------|
| Sensor read → sample enqueued | < 1 ms | Critical | Sensor I/O → RT Thread |
| Sample → processed (derived metrics) | < 5 ms | Critical | RT Thread processing |
| Alarm detection → UI visible | < 50 ms | **CRITICAL** | RT Thread → UI Thread |
| DB write (background batch) | < 200 ms | Normal | Background Thread |
| DB write (critical alarm) | < 100 ms | High | Background Thread |
| UI response (user interaction) | < 500 ms | Must Have | UI Thread |

---

## 2. Low-Latency Techniques

### 2.1 Pre-Allocation

**Principle:** Allocate all memory at startup, never during critical path operations.

**When to Use:**
- ✅ Real-time threads (RT Thread, Sensor I/O Thread)
- ✅ High-frequency operations (per-sample processing)
- ✅ Alarm detection path
- ✅ Lock-free queue buffers
- ✅ String buffers for logging

**When NOT to Use:**
- ❌ One-time initialization operations
- ❌ Background threads (unless performance critical)
- ❌ Operations with unpredictable memory needs

**Implementation:**
```cpp
// Pre-allocate at startup
class SignalProcessor {
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    std::array<float, BUFFER_SIZE> m_sampleBuffer;  // Stack or member
    std::array<float, BUFFER_SIZE> m_filterBuffer;
    
public:
    SignalProcessor() {
        // Buffers allocated, no heap allocation during processing
    }
    
    void processSample(float sample) {
        // Zero allocation - uses pre-allocated buffers
        m_sampleBuffer[m_writeIndex] = sample;
        // Process...
    }
};
```

**Z Monitor Usage:**
- `VitalsCache` - Pre-allocated for 3-day capacity (~39 MB)
- `WaveformCache` - Pre-allocated circular buffer (30 seconds)
- `LockFreeQueue` buffers - Pre-allocated ring buffers
- Alarm detection structures - Pre-allocated threshold check buffers

---

### 2.2 Lock-Free Data Structures

**Principle:** Use lock-free queues and atomic operations to avoid mutex contention.

**When to Use:**
- ✅ High-frequency inter-thread communication
- ✅ Real-time thread → Background thread (SPSC)
- ✅ Multiple producers → Single consumer (MPSC)
- ✅ Ring buffers for waveform samples
- ✅ Logging queues

**When NOT to Use:**
- ❌ Low-frequency operations (mutex overhead acceptable)
- ❌ Complex data structures requiring consistency
- ❌ Operations where blocking is acceptable

**Z Monitor Usage:**
- Sensor I/O → RT Thread: SPSC lock-free queue
- RT Thread → Database Thread: MPSC lock-free queue (telemetry batches)
- RT Thread → UI Thread: Qt signals (queued connection, acceptable latency)

---

### 2.3 Zero-Copy Patterns

**Principle:** Minimize data copying by using references, move semantics, and shared memory.

**When to Use:**
- ✅ Large data structures (TelemetryBatch, waveform arrays)
- ✅ Cross-thread data passing
- ✅ Serialization/deserialization paths
- ✅ Logging message construction

**When NOT to Use:**
- ❌ Small POD types (copy is cheaper than indirection)
- ❌ Simple value objects (PatientIdentity, VitalRecord)

**Implementation:**
```cpp
// Good: Move semantics (zero copy)
void enqueueTelemetry(TelemetryBatch&& batch) {
    m_queue.enqueue(std::move(batch));  // Move, no copy
}

// Good: Pass by reference
void processVitals(const VitalRecord& record) {
    // No copy, just reference
}

// Avoid: Unnecessary copy
void processVitals(VitalRecord record) {  // Copy made
    // ...
}
```

**Z Monitor Usage:**
- `TelemetryBatch` - Moved between threads, not copied
- Waveform samples - Passed by reference in processing pipeline
- Log entries - Moved to logging queue

---

### 2.4 Object Pooling

**Principle:** Reuse objects instead of allocating/deallocating repeatedly.

**When to Use:**
- ✅ High-frequency object creation (TelemetryBatch objects)
- ✅ Temporary objects in hot paths
- ✅ Objects with expensive construction
- ✅ Real-time threads

**When NOT to Use:**
- ❌ One-time object creation
- ❌ Objects with complex state (hard to reset)
- ❌ Background threads (unless performance critical)

**Z Monitor Usage:**
- `TelemetryBatch` objects - Pooled for RT Thread → Database Thread
- Log buffer objects - Pooled for high-frequency logging
- Network request objects - Pooled for telemetry transmission

---

### 2.5 CPU Affinity and Priority

**Principle:** Pin threads to CPU cores and set high priority for real-time operations.

**When to Use:**
- ✅ Real-time threads (RT Thread, Sensor I/O Thread)
- ✅ Critical path operations (alarm detection)
- ✅ High-frequency processing

**When NOT to Use:**
- ❌ Background threads (normal priority sufficient)
- ❌ UI thread (should remain responsive but not starve other threads)
- ❌ Database thread (I/O bound, priority less important)

**Implementation:**
```cpp
// Set thread priority and affinity
QThread* rtThread = new QThread();
rtThread->setPriority(QThread::HighPriority);

#ifdef Q_OS_LINUX
    // Pin to CPU core 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
    // Set real-time scheduling
    struct sched_param param;
    param.sched_priority = 50;  // High priority
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
#endif
```

**Z Monitor Usage:**
- RT Thread: High priority, pinned to CPU core 0
- Sensor I/O Thread: High priority, pinned to CPU core 1
- Database Thread: Normal priority
- Network Thread: Normal priority

---

### 2.6 Batch Processing

**Principle:** Process multiple items together to amortize overhead.

**When to Use:**
- ✅ Database writes (batch inserts)
- ✅ Network transmission (batch telemetry)
- ✅ Log file writes
- ✅ Cache persistence

**When NOT to Use:**
- ❌ Real-time operations (alarm detection must be immediate)
- ❌ User interactions (must respond immediately)
- ❌ Critical path operations

**Z Monitor Usage:**
- Database writes: Batch 10 vitals records per transaction
- Telemetry transmission: Batch vitals every 10 seconds
- Cache persistence: Batch write every 10 minutes

---

### 2.7 Memory-Mapped Files

**Principle:** Use memory-mapped files for large data structures to avoid copying.

**When to Use:**
- ✅ Large read-only data (patient lookup cache)
- ✅ Shared memory between processes
- ✅ Large log files

**When NOT to Use:**
- ❌ Small data structures
- ❌ Frequently updated data (overhead of sync)
- ❌ Real-time operations (page faults can cause jitter)

**Z Monitor Usage:**
- Patient lookup cache (if large enough to benefit)
- Historical trend data (read-only access)

---

## 3. Tools and Libraries

### 3.1 Lock-Free Queue Libraries

#### 3.1.1 boost::lockfree::spsc_queue

**Type:** Single Producer, Single Consumer  
**Performance:** Fastest for SPSC scenarios  
**Latency:** < 10 ns per operation  
**Memory:** Pre-allocated ring buffer

**When to Use:**
- ✅ Sensor I/O → RT Thread (one sensor, one processor)
- ✅ RT Thread → Database Thread (if single producer)
- ✅ Highest performance SPSC scenarios

**When NOT to Use:**
- ❌ Multiple producers (use MPSC queue)
- ❌ Multiple consumers (use MPMC queue)

**Implementation:**
```cpp
#include <boost/lockfree/spsc_queue.hpp>

// Pre-allocate queue
boost::lockfree::spsc_queue<Sample, boost::lockfree::capacity<4096>> sampleQueue;

// Producer (Sensor I/O Thread)
void onSensorData(const Sample& sample) {
    sampleQueue.push(sample);  // Lock-free, < 10 ns
}

// Consumer (RT Thread)
void processSamples() {
    Sample sample;
    while (sampleQueue.pop(sample)) {
        processSample(sample);  // Process without blocking
    }
}
```

**Z Monitor Usage:**
- **Sensor I/O → RT Thread:** `boost::lockfree::spsc_queue<SensorSample, capacity<4096>>`
- **Performance:** < 1 ms latency target met

---

#### 3.1.2 boost::lockfree::queue

**Type:** Multiple Producer, Multiple Consumer  
**Performance:** Fast for MPSC/MPMC scenarios  
**Latency:** < 50 ns per operation  
**Memory:** Dynamic allocation (nodes)

**When to Use:**
- ✅ Multiple producers → Single consumer (MPSC)
- ✅ Multiple producers → Multiple consumers (MPMC)
- ✅ RT Thread → Database Thread (if multiple RT workers)

**When NOT to Use:**
- ❌ SPSC scenarios (spsc_queue is faster)
- ❌ Zero-allocation requirements (uses dynamic allocation)

**Implementation:**
```cpp
#include <boost/lockfree/queue.hpp>

boost::lockfree::queue<TelemetryBatch> telemetryQueue;

// Multiple producers (RT Thread workers)
void enqueueTelemetry(TelemetryBatch&& batch) {
    telemetryQueue.push(std::move(batch));  // Lock-free, < 50 ns
}

// Single consumer (Database Thread)
void persistTelemetry() {
    TelemetryBatch batch;
    while (telemetryQueue.pop(batch)) {
        persistBatch(batch);  // Batch write to database
    }
}
```

**Z Monitor Usage:**
- **RT Thread → Database Thread:** `boost::lockfree::queue<TelemetryBatch>` (MPSC)
- **Performance:** < 200 ms latency target for background persistence

---

#### 3.1.3 moodycamel::ConcurrentQueue

**Type:** Multiple Producer, Multiple Consumer  
**Performance:** Very fast, header-only  
**Latency:** < 30 ns per operation  
**Memory:** Pre-allocated blocks with dynamic growth

**When to Use:**
- ✅ MPSC/MPMC scenarios
- ✅ Header-only requirement (no linking)
- ✅ High throughput requirements
- ✅ Variable queue sizes

**When NOT to Use:**
- ❌ SPSC scenarios (spsc_queue is faster)
- ❌ Fixed-size requirement (grows dynamically)

**Implementation:**
```cpp
#include "concurrentqueue.h"

moodycamel::ConcurrentQueue<LogEntry> logQueue;

// Multiple producers
void logMessage(const LogEntry& entry) {
    logQueue.enqueue(entry);  // Lock-free, < 30 ns
}

// Single consumer (Database I/O Thread)
void flushLogs() {
    LogEntry entry;
    while (logQueue.try_dequeue(entry)) {
        writeLogEntry(entry);
    }
}
```

**Z Monitor Usage:**
- **Logging Queue:** `moodycamel::ConcurrentQueue<LogEntry>` (MPSC)
- **Alternative to boost::lockfree::queue** if header-only preferred

---

#### 3.1.4 Custom SPSC Ring Buffer

**Type:** Single Producer, Single Consumer  
**Performance:** Fastest (no library overhead)  
**Latency:** < 5 ns per operation  
**Memory:** Pre-allocated array

**When to Use:**
- ✅ SPSC scenarios with highest performance requirements
- ✅ Zero-allocation requirement
- ✅ Minimal dependencies
- ✅ Custom behavior needed

**When NOT to Use:**
- ❌ Multiple producers/consumers
- ❌ Variable queue sizes
- ❌ When library maintenance is preferred

**Implementation:**
```cpp
template<typename T, size_t Size>
class SPSCRingBuffer {
private:
    std::array<T, Size> m_buffer;
    std::atomic<size_t> m_writeIndex{0};
    std::atomic<size_t> m_readIndex{0};
    
public:
    bool push(const T& item) {
        size_t next = (m_writeIndex.load() + 1) % Size;
        if (next == m_readIndex.load()) {
            return false;  // Full
        }
        m_buffer[m_writeIndex.load()] = item;
        m_writeIndex.store(next, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        if (m_readIndex.load() == m_writeIndex.load()) {
            return false;  // Empty
        }
        item = m_buffer[m_readIndex.load()];
        m_readIndex.store((m_readIndex.load() + 1) % Size, std::memory_order_acquire);
        return true;
    }
};
```

**Z Monitor Usage:**
- **Waveform samples:** Custom SPSC ring buffer (highest performance)
- **Performance:** < 1 ms latency for sensor → RT Thread

---

### 3.2 Memory Management Libraries

#### 3.2.1 Custom Object Pool

**Type:** Object pooling utility  
**Performance:** Eliminates allocation overhead  
**Latency:** < 10 ns per acquire/release

**When to Use:**
- ✅ High-frequency object creation (TelemetryBatch)
- ✅ Real-time threads
- ✅ Objects with expensive construction

**Implementation:** See [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) Section 2.4

**Z Monitor Usage:**
- **TelemetryBatch pool:** Pool of 100 pre-allocated batches
- **Location:** `src/infrastructure/utils/ObjectPool.h/cpp`

---

#### 3.2.2 boost::pool

**Type:** Memory pool allocator  
**Performance:** Fast allocation for fixed-size objects  
**Latency:** < 20 ns per allocation

**When to Use:**
- ✅ Fixed-size object allocation
- ✅ High-frequency allocations
- ✅ When boost is already a dependency

**When NOT to Use:**
- ❌ Variable-size objects
- ❌ Minimal dependency requirement

**Z Monitor Usage:**
- **Alternative to custom ObjectPool** if boost is available
- **Not currently planned** (custom implementation preferred)

---

### 3.3 Profiling and Measurement Tools

#### 3.3.1 Google Benchmark

**Type:** Microbenchmark framework  
**Performance:** Low overhead measurement  
**Latency:** < 1% measurement overhead

**When to Use:**
- ✅ Performance regression testing
- ✅ Latency measurement
- ✅ Throughput measurement
- ✅ CI/CD performance monitoring

**Z Monitor Usage:**
- **Alarm detection latency:** `bench_alarm_detection_latency`
- **Queue performance:** `bench_lockfree_queue_throughput`
- **See:** [40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md](./40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md)

---

#### 3.3.2 perf (Linux)

**Type:** Performance profiling tool  
**Performance:** Low overhead (< 1%)  
**Latency:** Real-time profiling

**When to Use:**
- ✅ Performance analysis
- ✅ Hot spot identification
- ✅ Cache miss analysis
- ✅ CPU utilization profiling

**Z Monitor Usage:**
- **Performance analysis:** `perf record ./z-monitor`
- **Hot spot analysis:** `perf report`
- **Cache analysis:** `perf stat -e cache-misses ./z-monitor`

---

#### 3.3.3 Intel VTune / AMD uProf

**Type:** Advanced performance profiler  
**Performance:** Detailed analysis  
**Latency:** Post-processing analysis

**When to Use:**
- ✅ Deep performance analysis
- ✅ Memory bandwidth analysis
- ✅ CPU pipeline analysis
- ✅ Advanced optimization

**Z Monitor Usage:**
- **Advanced optimization** (if available on target platform)
- **Not required** for basic performance tuning

---

## 4. Decision Guidelines

### 4.1 Choosing Lock-Free Queue Libraries

**Decision Tree:**

```
Is it SPSC (Single Producer, Single Consumer)?
├─ YES → Use boost::lockfree::spsc_queue
│         (Fastest, pre-allocated, < 10 ns)
│
└─ NO → Is it MPSC (Multiple Producers, Single Consumer)?
    ├─ YES → Use boost::lockfree::queue OR moodycamel::ConcurrentQueue
    │         (boost::lockfree::queue if boost available)
    │         (moodycamel if header-only preferred)
    │
    └─ NO → Use moodycamel::ConcurrentQueue
            (MPMC support, header-only)
```

**Z Monitor Decisions:**
- **Sensor I/O → RT Thread:** `boost::lockfree::spsc_queue` (SPSC, highest performance)
- **RT Thread → Database Thread:** `boost::lockfree::queue` (MPSC, if multiple RT workers)
- **Logging Queue:** `moodycamel::ConcurrentQueue` (MPSC, header-only preferred)

---

### 4.2 Choosing Memory Management Techniques

**Decision Tree:**

```
Is it in the critical path (< 50ms requirement)?
├─ YES → Pre-allocate all buffers
│         Use object pools for temporary objects
│         Zero allocations in hot path
│
└─ NO → Is it high-frequency (> 1000 ops/sec)?
    ├─ YES → Use object pools
    │         Pre-allocate where possible
    │
    └─ NO → Standard allocation acceptable
            Use smart pointers
            Qt parent-child for QObject
```

**Z Monitor Decisions:**
- **Alarm detection path:** Pre-allocated buffers, zero allocations
- **TelemetryBatch creation:** Object pool (high-frequency)
- **Background operations:** Standard allocation acceptable

---

### 4.3 Choosing Synchronization Primitives

**Decision Tree:**

```
Is it high-frequency (> 1000 ops/sec)?
├─ YES → Use lock-free structures
│         Atomic operations
│         Avoid mutexes
│
└─ NO → Is it low-frequency (< 100 ops/sec)?
    ├─ YES → Mutex acceptable
    │         QMutex for Qt code
    │         std::mutex for standard C++
    │
    └─ NO → Qt signals/slots (queued connection)
            Acceptable latency for UI updates
```

**Z Monitor Decisions:**
- **Sensor → RT Thread:** Lock-free queue (high-frequency)
- **RT → Database Thread:** Lock-free queue (high-frequency)
- **RT → UI Thread:** Qt signals (acceptable latency, < 50ms)
- **Configuration updates:** Mutex (low-frequency)

---

## 5. Z Monitor Implementation Plan

### 5.1 Critical Path (< 50ms): Alarm Detection

**Requirements:**
- Sensor read → sample enqueued: < 1 ms
- Sample → processed: < 5 ms
- Alarm detection → UI visible: < 50 ms

**Techniques:**
1. **Pre-allocated buffers:** All alarm detection structures pre-allocated
2. **Lock-free queue:** `boost::lockfree::spsc_queue` for Sensor → RT Thread
3. **Zero allocations:** No heap allocations in alarm detection path
4. **High priority thread:** RT Thread at SCHED_FIFO priority
5. **CPU affinity:** RT Thread pinned to CPU core 0

**Libraries:**
- `boost::lockfree::spsc_queue<SensorSample, capacity<4096>>`
- Custom SPSC ring buffer for waveform samples (if needed)

**Location:**
- `src/infrastructure/sensors/WebSocketSensorDataSource.cpp` - Producer
- `src/application/services/MonitoringService.cpp` - Consumer (RT Thread)
- `src/domain/monitoring/AlarmAggregate.cpp` - Alarm detection

---

### 5.2 High-Frequency Path: Telemetry Batching

**Requirements:**
- RT Thread → Database Thread: < 200 ms (background)
- Batch size: 10 records per transaction

**Techniques:**
1. **Lock-free queue:** `boost::lockfree::queue` for MPSC
2. **Object pooling:** Pool of TelemetryBatch objects
3. **Batch processing:** Group 10 records per database transaction
4. **Move semantics:** Move batches, don't copy

**Libraries:**
- `boost::lockfree::queue<TelemetryBatch>`
- Custom `ObjectPool<TelemetryBatch>` (100 objects)

**Location:**
- `src/application/services/MonitoringService.cpp` - Producer (RT Thread)
- `src/infrastructure/persistence/SQLiteTelemetryRepository.cpp` - Consumer (Database Thread)
- `src/infrastructure/utils/ObjectPool.h/cpp` - Object pool implementation

---

### 5.3 High-Frequency Path: Logging

**Requirements:**
- Log entry enqueue: < 1 ms
- Background flush: < 200 ms

**Techniques:**
1. **Lock-free queue:** `moodycamel::ConcurrentQueue` for MPSC
2. **Pre-allocated buffers:** LogBuffer for message construction
3. **Batch writes:** Flush multiple entries per file write

**Libraries:**
- `moodycamel::ConcurrentQueue<LogEntry>`
- Custom `LogBuffer` (pre-allocated 1KB buffer)

**Location:**
- `src/infrastructure/logging/LogService.cpp` - Producer (all threads, enqueues to lock-free queue)
- `src/infrastructure/logging/LogService.cpp` - Consumer (Database I/O Thread)
- `src/infrastructure/utils/LogBuffer.h/cpp` - Log buffer implementation

---

### 5.4 Real-Time Path: Waveform Display

**Requirements:**
- Waveform sample → display: < 16 ms (60 FPS)
- Smooth rendering without jitter

**Techniques:**
1. **Pre-allocated circular buffer:** 30-second waveform cache
2. **Lock-free access:** Atomic indices for buffer access
3. **Zero-copy rendering:** QML Canvas API with direct buffer access
4. **Double buffering:** Separate read/write buffers

**Libraries:**
- Custom `WaveformCache` (circular buffer)
- Qt QML Canvas API

**Location:**
- `src/infrastructure/caching/WaveformCache.h/cpp` - Circular buffer
- `src/interface/qml/components/WaveformDisplay.qml` - QML rendering

---

### 5.5 Background Path: Database Writes

**Requirements:**
- Batch write: < 200 ms (background)
- Critical alarm write: < 100 ms (immediate)

**Techniques:**
1. **Batch processing:** 10 records per transaction
2. **Prepared statements:** Reuse SQL statements
3. **WAL mode:** SQLite WAL for concurrent reads
4. **Normal priority:** Database thread at normal priority

**Libraries:**
- SQLite with WAL mode
- QxOrm or custom prepared statements

**Location:**
- `src/infrastructure/persistence/SQLiteVitalsRepository.cpp`
- `src/infrastructure/persistence/DatabaseManager.cpp`

---

## 6. Performance Guidelines

### 6.1 Critical Path Guidelines

**DO:**
- ✅ Pre-allocate all buffers at startup
- ✅ Use lock-free structures for inter-thread communication
- ✅ Set high thread priority for RT threads
- ✅ Pin RT threads to CPU cores
- ✅ Use zero-copy patterns (move semantics, references)
- ✅ Measure latency continuously

**DON'T:**
- ❌ Allocate memory in critical path
- ❌ Use mutexes in high-frequency operations
- ❌ Copy large data structures
- ❌ Block RT threads with I/O operations
- ❌ Use dynamic containers that grow

---

### 6.2 Measurement Guidelines

**DO:**
- ✅ Measure latency at 95th and 99th percentiles
- ✅ Monitor jitter (variance in latency)
- ✅ Track latency trends over time
- ✅ Set up automated regression detection
- ✅ Profile with `perf` or similar tools

**DON'T:**
- ❌ Rely on average latency only
- ❌ Ignore tail latencies (99th percentile)
- ❌ Measure in debug builds
- ❌ Measure with other processes running

---

### 6.3 Optimization Guidelines

**DO:**
- ✅ Profile before optimizing
- ✅ Measure impact of optimizations
- ✅ Optimize critical path first
- ✅ Use appropriate tools for each scenario
- ✅ Document performance characteristics

**DON'T:**
- ❌ Premature optimization
- ❌ Optimize non-critical paths
- ❌ Sacrifice code clarity for micro-optimizations
- ❌ Optimize without measurement

---

## 7. Library Dependencies

### 7.1 Required Libraries

| Library | Purpose | License | Dependency Type |
|---------|---------|---------|----------------|
| `boost::lockfree` | Lock-free queues | Boost License | Required for SPSC/MPSC queues |
| `moodycamel::ConcurrentQueue` | Lock-free queue (alternative) | Public Domain | Optional (header-only alternative) |

### 7.2 Optional Libraries

| Library | Purpose | License | When to Use |
|---------|---------|---------|-------------|
| `boost::pool` | Memory pooling | Boost License | If boost available and custom pool not needed |
| `Google Benchmark` | Performance benchmarks | Apache 2.0 | CI/CD performance testing |

---

## 8. Testing and Validation

### 8.1 Latency Testing

**Tools:**
- Google Benchmark for microbenchmarks
- Custom latency measurement in production code
- `perf` for profiling

**Test Scenarios:**
- Alarm detection latency (< 50ms)
- Queue throughput (operations/second)
- Memory allocation overhead
- Thread priority effectiveness

**See:** [40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md](./40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md)

---

### 8.2 Regression Testing

**Automated:**
- Nightly benchmark execution
- Latency regression detection
- Performance trend analysis

**Manual:**
- Profiling with `perf` on target hardware
- Stress testing under load
- Jitter analysis

---

## 9. Related Documents

- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Thread architecture and latency targets ⭐
- [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) - Memory management patterns
- [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - Critical path caching
- [40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md](./40_BENCHMARK_AND_PERFORMANCE_MEASUREMENT.md) - Performance measurement
- [Performance Requirements](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md) - Latency targets

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-27  
**Status:** Approved

*This document provides comprehensive guidance for implementing low-latency operations in the Z Monitor application. Follow the decision guidelines and implementation plans for each critical path.*
