# Memory & Resource Management Strategy

**Document ID:** DESIGN-023  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document defines memory management patterns, resource lifecycle management, and performance optimization strategies for the Z Monitor application.

## 1. Guiding Principles

- **RAII (Resource Acquisition Is Initialization):** Resources are acquired in constructors and released in destructors
- **Ownership Clarity:** Clear ownership semantics for all resources
- **No Memory Leaks:** All allocated memory must be freed
- **Performance:** Minimize allocations in hot paths
- **Safety:** Use smart pointers to prevent dangling pointers
- **Predictability:** Pre-allocate buffers for real-time operations

## 2. Memory Management Patterns

### 2.1. Smart Pointers (Preferred)

Use smart pointers for dynamic memory:

```cpp
// Unique ownership
std::unique_ptr<NetworkManager> networkManager = 
    std::make_unique<NetworkManager>(this);

// Shared ownership (use sparingly)
std::shared_ptr<ITelemetryServer> server = 
    std::make_shared<NetworkTelemetryServer>();

// Qt's parent-child ownership (preferred for QObject)
NetworkManager* networkManager = new NetworkManager(this);  // Auto-deleted
```

### 2.2. Qt Parent-Child Ownership

For QObject-derived classes, use Qt's parent-child ownership:

```cpp
class MainApplication : public QObject {
    NetworkManager* m_networkManager;
    DatabaseManager* m_databaseManager;
    
public:
    MainApplication(QObject* parent = nullptr) : QObject(parent) {
        // Children automatically deleted when parent is deleted
        m_networkManager = new NetworkManager(this);
        m_databaseManager = new DatabaseManager(this);
    }
    // No need to delete in destructor
};
```

**When to Use:**
- QObject-derived classes
- UI components
- Controllers
- Services with clear parent

### 2.3. Stack Allocation (Preferred)

Prefer stack allocation when possible:

```cpp
// Good: Stack allocation
QString deviceId = "ZM-001";
QDateTime timestamp = QDateTime::currentDateTime();

// Avoid: Unnecessary heap allocation
QString* deviceId = new QString("ZM-001");  // Don't do this
```

### 2.4. Object Pools (For Hot Paths)

Use object pools for frequently allocated/deallocated objects to avoid heap allocation overhead in performance-critical paths.

**Implementation Status:**
- ⏳ **Not yet implemented** - Example pattern shown below
- **Location:** Would be implemented in `src/infrastructure/utils/` or as a shared utility library
- **Qt Support:** Qt does **not** provide built-in object pooling (only `QThreadPool` for thread recycling)

**Example Pattern:**
```cpp
template<typename T>
class ObjectPool {
public:
    std::unique_ptr<T> acquire() {
        if (pool.empty()) {
            return std::make_unique<T>();
        }
        auto obj = std::move(pool.back());
        pool.pop_back();
        return obj;
    }
    
    void release(std::unique_ptr<T> obj) {
        obj->reset();  // Reset state
        pool.push_back(std::move(obj));
    }
    
private:
    std::vector<std::unique_ptr<T>> pool;
    std::mutex m_mutex;  // Thread-safe if needed
};
```

**When to Use:**
- High-frequency allocations (e.g., per-sample processing)
- Real-time threads
- Performance-critical paths
- TelemetryBatch objects (as mentioned in [12_THREAD_MODEL.md](./12_THREAD_MODEL.md))

**Implementation Options:**
1. **Custom Implementation:** Create `ObjectPool<T>` in `src/infrastructure/utils/`
2. **External Library:** Consider lightweight C++ object pool libraries if available
3. **Qt Alternative:** Use `QThreadPool` for thread recycling (different use case)

## 3. Resource Lifecycle Management

### 3.1. File Handles

Use RAII for file handles:

```cpp
class FileHandle {
public:
    explicit FileHandle(const QString& path) {
        file.setFileName(path);
        if (!file.open(QIODevice::ReadWrite)) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    ~FileHandle() {
        file.close();  // Automatically closed
    }
    
    QFile& get() { return file; }
    
private:
    QFile file;
};
```

### 3.2. Network Connections

Manage network connection lifecycle:

```cpp
class NetworkManager {
private:
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    QNetworkReply* m_currentReply = nullptr;
    
public:
    ~NetworkManager() {
        // Cancel pending requests
        if (m_currentReply) {
            m_currentReply->abort();
            m_currentReply->deleteLater();
        }
    }
    
    void sendRequest() {
        // Previous request is automatically cleaned up
        m_currentReply = m_networkManager->get(request);
        connect(m_currentReply, &QNetworkReply::finished, 
                this, &NetworkManager::onRequestFinished);
    }
    
private slots:
    void onRequestFinished() {
        // Clean up
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
};
```

### 3.3. Database Connections

Manage database connections:

```cpp
class DatabaseManager {
private:
    QSqlDatabase m_database;
    bool m_isOpen = false;
    
public:
    bool openDatabase(const QString& path) {
        if (m_isOpen) {
            closeDatabase();  // Close existing connection
        }
        
        m_database = QSqlDatabase::addDatabase("QSQLITE");
        m_database.setDatabaseName(path);
        m_isOpen = m_database.open();
        return m_isOpen;
    }
    
    void closeDatabase() {
        if (m_isOpen) {
            m_database.close();
            QSqlDatabase::removeDatabase(m_database.connectionName());
            m_isOpen = false;
        }
    }
    
    ~DatabaseManager() {
        closeDatabase();  // Ensure cleanup
    }
};
```

## 4. Pre-allocation Strategies

### 4.1. Real-Time Buffers

Pre-allocate all real-time buffers at startup:

```cpp
class SignalProcessor {
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    std::array<float, BUFFER_SIZE> m_sampleBuffer;  // Pre-allocated
    std::array<float, BUFFER_SIZE> m_filterBuffer;  // Pre-allocated
    
public:
    SignalProcessor() {
        // Buffers are allocated on stack or as class members
        // No heap allocation during processing
    }
    
    void processSample(float sample) {
        // Use pre-allocated buffers
        m_sampleBuffer[m_writeIndex] = sample;
        // Process...
    }
};
```

### 4.2. Queue Buffers

Pre-allocate queue buffers for high-performance inter-thread communication.

**Implementation Status:**
- ⏳ **Not yet implemented** - Example pattern shown below
- **Location:** Would be implemented in `src/infrastructure/utils/` or use external libraries
- **Qt Support:** Qt does **not** provide lock-free queues (only `QQueue` which is not lock-free)

**Recommended Approach:** Use external libraries for production-quality lock-free queues (see [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) Section 6):
- **SPSC (Single Producer, Single Consumer):** `boost::lockfree::spsc_queue`, `folly::ProducerConsumerQueue`
- **MPSC (Multiple Producer, Single Consumer):** `boost::lockfree::queue`, `moodycamel::ConcurrentQueue`

**Example Pattern (Simple SPSC Ring Buffer):**
```cpp
template<typename T, size_t QueueSize>
class LockFreeQueue {
private:
    static constexpr size_t QUEUE_SIZE = QueueSize;
    std::array<T, QUEUE_SIZE> m_buffer;  // Pre-allocated
    std::atomic<size_t> m_writeIndex{0};
    std::atomic<size_t> m_readIndex{0};
    
public:
    bool enqueue(const T& entry) {
        // Use pre-allocated buffer, no allocation
        size_t next = (m_writeIndex.load() + 1) % QUEUE_SIZE;
        if (next == m_readIndex.load()) {
            return false;  // Queue full
        }
        m_buffer[m_writeIndex.load()] = entry;
        m_writeIndex.store(next, std::memory_order_release);
        return true;
    }
    
    bool dequeue(T& entry) {
        if (m_readIndex.load() == m_writeIndex.load()) {
            return false;  // Queue empty
        }
        entry = m_buffer[m_readIndex.load()];
        m_readIndex.store((m_readIndex.load() + 1) % QUEUE_SIZE, std::memory_order_acquire);
        return true;
    }
};
```

**When to Use:**
- High-frequency data transfer between threads
- Real-time thread communication
- Logging queue (as mentioned in [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md))
- Telemetry batch queuing (RT Thread → Database Thread)

### 4.3. String Buffers

Pre-allocate string buffers for logging to avoid repeated allocations.

**Implementation Status:**
- ⏳ **Not yet implemented** - Example pattern shown below
- **Location:** Would be implemented in `src/infrastructure/utils/` or as part of `LogService`
- **Qt Support:** Qt's `QString` uses implicit sharing, but pre-allocated buffers can still help in hot paths

**Example Pattern:**
```cpp
class LogBuffer {
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    char m_buffer[BUFFER_SIZE];
    size_t m_pos = 0;
    
public:
    void append(const QString& str) {
        QByteArray bytes = str.toUtf8();
        if (m_pos + bytes.size() < BUFFER_SIZE) {
            std::memcpy(m_buffer + m_pos, bytes.data(), bytes.size());
            m_pos += bytes.size();
        }
    }
    
    void reset() {
        m_pos = 0;
    }
    
    QString toString() const {
        return QString::fromUtf8(m_buffer, m_pos);
    }
};
```

**When to Use:**
- High-frequency logging operations
- Building log messages incrementally
- Reducing allocations in logging hot paths

## 5. Memory Allocation Rules

### 5.1. Hot Path Rules

**Real-time threads (Sensor I/O, Signal Processor):**
- ❌ No heap allocations in inner loops
- ❌ No dynamic containers (QList, QVector) growth
- ✅ Use pre-allocated buffers
- ✅ Use fixed-size arrays
- ✅ Use object pools for temporary objects

### 5.2. Background Thread Rules

**Background threads (DB Writer, Network, Logger):**
- ✅ Heap allocation allowed (but minimize)
- ✅ Use smart pointers
- ✅ Batch allocations when possible
- ✅ Reuse buffers across operations

### 5.3. UI Thread Rules

**UI thread:**
- ✅ Standard Qt memory management (parent-child)
- ✅ Use QML property bindings (automatic memory management)
- ✅ Avoid large allocations that block UI
- ✅ Use async operations for heavy work

## 6. Resource Cleanup

### 6.1. Destructor Guidelines

- Always clean up resources in destructors
- Use RAII for all resources
- Cancel pending operations
- Close file handles
- Release network connections
- Clear caches

```cpp
class NetworkManager {
private:
    QNetworkAccessManager* m_networkManager;
    QList<QNetworkReply*> m_pendingReplies;
    
public:
    ~NetworkManager() {
        // Cancel all pending requests
        for (QNetworkReply* reply : m_pendingReplies) {
            reply->abort();
            reply->deleteLater();
        }
        m_pendingReplies.clear();
        
        // Network manager is parented, auto-deleted
    }
};
```

### 6.2. Explicit Cleanup Methods

Provide explicit cleanup methods for resources that should be released before destruction:

```cpp
class DatabaseManager {
public:
    void closeDatabase() {
        if (m_database.isOpen()) {
            // Flush pending writes
            flushPendingWrites();
            
            // Close connection
            m_database.close();
            m_isOpen = false;
        }
    }
    
    ~DatabaseManager() {
        closeDatabase();  // Ensure cleanup
    }
};
```

## 7. Memory Leak Prevention

### 7.1. Common Leak Sources

- **Forgotten deletes:** Use smart pointers or parent-child
- **Circular references:** Break cycles with weak pointers
- **Unclosed resources:** Use RAII
- **Event loop objects:** Use `deleteLater()` for QObject cleanup

### 7.2. Leak Detection

Use tools to detect leaks:

- **Valgrind:** `valgrind --leak-check=full ./z-monitor`
- **AddressSanitizer:** Compile with `-fsanitize=address`
- **Qt Creator Memory Analyzer:** Built-in leak detection

### 7.3. Best Practices

- ✅ Use smart pointers for dynamic memory
- ✅ Use Qt parent-child for QObject
- ✅ Use RAII for all resources
- ✅ Test with leak detection tools
- ✅ Review code for resource cleanup

## 8. Performance Optimization

### 8.1. Allocation Reduction

- **Pre-allocate buffers:** Avoid repeated allocations
- **Reuse objects:** Use object pools
- **Stack allocation:** Prefer stack over heap
- **Reserve capacity:** Pre-allocate container capacity

```cpp
// Good: Reserve capacity
QList<VitalSign> vitals;
vitals.reserve(1000);  // Pre-allocate for 1000 items

// Avoid: Repeated reallocation
QList<VitalSign> vitals;  // Grows dynamically, causes reallocations
```

### 8.2. Memory Pool Patterns

Use memory pools for frequent allocations:

```cpp
template<typename T, size_t PoolSize>
class MemoryPool {
private:
    std::array<T, PoolSize> m_pool;
    std::bitset<PoolSize> m_used;
    
public:
    T* allocate() {
        for (size_t i = 0; i < PoolSize; ++i) {
            if (!m_used[i]) {
                m_used[i] = true;
                return &m_pool[i];
            }
        }
        return nullptr;  // Pool exhausted
    }
    
    void deallocate(T* ptr) {
        size_t index = ptr - m_pool.data();
        if (index < PoolSize) {
            m_used[index] = false;
        }
    }
};
```

### 8.3. Zero-Copy Patterns

Minimize data copying:

```cpp
// Good: Pass by reference
void processData(const TelemetryData& data);  // No copy

// Avoid: Unnecessary copy
void processData(TelemetryData data);  // Copy made

// Good: Move semantics
void storeData(TelemetryData&& data);  // Move, no copy
```

## 9. Thread-Safe Memory Management

### 9.1. Shared Data

Use atomic operations or mutexes for shared data:

```cpp
class ThreadSafeCounter {
private:
    std::atomic<int> m_count{0};
    
public:
    void increment() {
        m_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    int get() const {
        return m_count.load(std::memory_order_relaxed);
    }
};
```

### 9.2. Lock-Free Structures

Use lock-free data structures for high-performance inter-thread communication.

**Implementation Status:**
- ⏳ **Not yet implemented** - Use external libraries for production code
- **Location:** External libraries (boost, folly, moodycamel) or custom implementation in `src/infrastructure/utils/`
- **Qt Support:** Qt does **not** provide lock-free data structures

**Recommended Libraries:**
- **boost::lockfree::spsc_queue** - Single producer, single consumer (fastest)
- **boost::lockfree::queue** - Multiple producer, multiple consumer
- **folly::ProducerConsumerQueue** - Facebook's high-performance queue
- **moodycamel::ConcurrentQueue** - Lock-free concurrent queue

**Example Pattern (Node-based, not recommended for production):**
```cpp
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<Node*> next;
        T data;
    };
    
    std::atomic<Node*> m_head;
    std::atomic<Node*> m_tail;
    
public:
    void enqueue(const T& data) {
        Node* node = new Node{nullptr, data};  // ⚠️ Still allocates!
        Node* prev = m_tail.exchange(node, std::memory_order_acq_rel);
        prev->next.store(node, std::memory_order_release);
    }
};
```

**Note:** The node-based approach above still allocates memory. For true zero-allocation, use ring buffers (see Section 4.2) or external libraries with pre-allocated buffers.

## 10. Resource Monitoring

### 10.1. Memory Usage Tracking

Monitor memory usage:

```cpp
class MemoryMonitor {
public:
    static size_t getCurrentMemoryUsage() {
        // Platform-specific memory usage query
        #ifdef Q_OS_LINUX
            // Read /proc/self/status
        #elif defined(Q_OS_MACOS)
            // Use mach_task_basic_info
        #endif
    }
    
    static void logMemoryUsage(const QString& context) {
        size_t usage = getCurrentMemoryUsage();
        LogService::info("Memory usage", {
            {"context", context},
            {"bytes", QString::number(usage)},
            {"mb", QString::number(usage / 1024 / 1024)}
        });
    }
};
```

### 10.2. Resource Limits

Set resource limits and monitor:

```cpp
class ResourceLimits {
public:
    static constexpr size_t MAX_MEMORY_MB = 512;
    static constexpr size_t MAX_FILE_HANDLES = 100;
    static constexpr size_t MAX_NETWORK_CONNECTIONS = 10;
    
    static bool checkMemoryLimit() {
        size_t current = MemoryMonitor::getCurrentMemoryUsage();
        return current < MAX_MEMORY_MB * 1024 * 1024;
    }
};
```

## 11. Best Practices

### 11.1. Do's

- ✅ Use smart pointers for dynamic memory
- ✅ Use Qt parent-child for QObject
- ✅ Pre-allocate buffers for real-time operations
- ✅ Use RAII for all resources
- ✅ Monitor memory usage
- ✅ Test with leak detection tools
- ✅ Use move semantics to avoid copies

### 11.2. Don'ts

- ❌ Don't allocate in hot paths
- ❌ Don't forget to clean up resources
- ❌ Don't use raw pointers for ownership
- ❌ Don't create circular references
- ❌ Don't ignore memory leaks
- ❌ Don't allocate large objects on stack

## 12. Utility Classes and Shared Libraries

### 12.1. Implementation Status

The following utility classes are **mentioned in design documents but not yet implemented**:

| Utility Class | Status | Location | Notes |
|--------------|--------|----------|-------|
| `ObjectPool<T>` | ⏳ Not implemented | `src/infrastructure/utils/` | Example pattern in Section 2.4 |
| `LockFreeQueue<T>` | ⏳ Not implemented | External libraries recommended | Use `boost::lockfree` or `moodycamel::ConcurrentQueue` |
| `LogBuffer` | ⏳ Not implemented | `src/infrastructure/utils/` or `LogService` | Example pattern in Section 4.3 |
| `MemoryPool<T>` | ⏳ Not implemented | `src/infrastructure/utils/` | Example pattern in Section 8.2 |

### 12.2. Where Utility Classes Should Live

Based on DDD structure (see [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md)):

```
src/infrastructure/utils/          # Shared utility classes
├── ObjectPool.h/cpp               # Object pooling utility
├── MemoryPool.h/cpp               # Memory pool allocator
├── LogBuffer.h/cpp                # Pre-allocated log buffer
├── LockFreeQueue.h/cpp            # Custom lock-free queue (if not using external lib)
└── ...                            # Other utilities
```

**Alternative:** If utilities are used across layers, consider a separate `src/common/` or `src/shared/` directory, but this violates DDD principles. Prefer keeping utilities in infrastructure layer.

### 12.3. Qt's Built-in Options

**What Qt Provides:**
- ✅ `QThreadPool` - Thread recycling (not object pooling)
- ✅ `QQueue<T>` - Standard queue (not lock-free, uses mutexes)
- ✅ `QString` - Implicit sharing (reduces copies)
- ✅ `QByteArray` - Pre-allocated buffer support
- ❌ **No object pooling** - Must implement custom or use external library
- ❌ **No lock-free queues** - Must use external libraries or custom implementation

**Recommendation:** Use Qt's built-in classes where appropriate, but for high-performance lock-free structures, use external libraries (`boost::lockfree`, `moodycamel::ConcurrentQueue`).

### 12.4. External Library Recommendations

For production use, prefer battle-tested external libraries:

1. **Lock-Free Queues:**
   - `boost::lockfree::spsc_queue` - Single producer, single consumer (fastest)
   - `boost::lockfree::queue` - Multiple producer, multiple consumer
   - `moodycamel::ConcurrentQueue` - Header-only, very fast

2. **Object Pools:**
   - Custom implementation recommended (simple enough to implement)
   - Or lightweight C++ object pool libraries

3. **Memory Pools:**
   - Custom implementation for specific use cases
   - Or `boost::pool` for general-purpose memory pooling

## 13. Related Documents

- [12_THREAD_MODEL.md](./12_THREAD_MODEL.md) - Memory management in multi-threaded context, lock-free queue recommendations
- [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) - Code organization and directory structure
- [20_ERROR_HANDLING_STRATEGY.md](./20_ERROR_HANDLING_STRATEGY.md) - Error handling for resource failures
- [21_LOGGING_STRATEGY.md](./21_LOGGING_STRATEGY.md) - Logging implementation using lock-free queues

