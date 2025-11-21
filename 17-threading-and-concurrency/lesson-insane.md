# Lesson 17: Threading and Concurrency - Insane

## Warning

The techniques in this lesson are for **expert-level scenarios** only. They involve complex synchronization patterns, lock-free data structures, and subtle memory ordering semantics. Incorrect implementation can lead to silent data corruption, deadlocks, and heisenbugs that only appear under load. Use these patterns only when profiling proves standard approaches are insufficient.

## Lock-Free MPMC Queue with ABA Prevention

Multi-producer, multi-consumer lock-free queue resistant to the ABA problem:

```cpp
template<typename T>
class LockFreeMPMCQueue {
private:
    struct Node {
        QAtomicPointer<T> data{nullptr};
        QAtomicPointer<Node> next{nullptr};
        QAtomicInteger<quint64> version{0};  // ABA prevention
    };
    
    struct VersionedPointer {
        Node *ptr;
        quint64 version;
        
        VersionedPointer() : ptr(nullptr), version(0) {}
        VersionedPointer(Node *p, quint64 v) : ptr(p), version(v) {}
    };
    
    // Pack pointer and version into 128-bit for atomic CAS
    struct PackedPointer {
        quintptr ptrBits;
        quint64 version;
        
        static PackedPointer pack(Node *ptr, quint64 ver) {
            return {reinterpret_cast<quintptr>(ptr), ver};
        }
        
        Node* getPtr() const { return reinterpret_cast<Node*>(ptrBits); }
    };
    
    alignas(16) std::atomic<PackedPointer> head;
    alignas(16) std::atomic<PackedPointer> tail;
    
    static_assert(sizeof(PackedPointer) == 16, "Need 128-bit CAS");
    
public:
    LockFreeMPMCQueue() {
        Node *dummy = new Node;
        PackedPointer initial = PackedPointer::pack(dummy, 0);
        head.store(initial, std::memory_order_relaxed);
        tail.store(initial, std::memory_order_relaxed);
    }
    
    void enqueue(T *value) {
        Node *node = new Node;
        node->data.storeRelease(value);
        
        while (true) {
            PackedPointer tailSnap = tail.load(std::memory_order_acquire);
            Node *tailNode = tailSnap.getPtr();
            PackedPointer next = {
                reinterpret_cast<quintptr>(tailNode->next.loadAcquire()),
                tailNode->version.loadAcquire()
            };
            
            // Check if tail is consistent
            if (tailSnap.ptrBits == reinterpret_cast<quintptr>(
                tail.load(std::memory_order_acquire).getPtr())) {
                
                if (next.getPtr() == nullptr) {
                    // Tail points to last node, try to insert
                    PackedPointer newNext = PackedPointer::pack(node, next.version + 1);
                    
                    if (compareExchange128(&tailNode->next, nullptr, node,
                                          next.version, next.version + 1)) {
                        // Success, try to swing tail
                        PackedPointer newTail = PackedPointer::pack(node, tailSnap.version + 1);
                        tail.compare_exchange_weak(tailSnap, newTail,
                            std::memory_order_release,
                            std::memory_order_relaxed);
                        return;
                    }
                } else {
                    // Tail is behind, try to advance it
                    PackedPointer newTail = PackedPointer::pack(next.getPtr(), 
                                                                tailSnap.version + 1);
                    tail.compare_exchange_weak(tailSnap, newTail,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                }
            }
        }
    }
    
    T* dequeue() {
        while (true) {
            PackedPointer headSnap = head.load(std::memory_order_acquire);
            PackedPointer tailSnap = tail.load(std::memory_order_acquire);
            Node *headNode = headSnap.getPtr();
            
            PackedPointer nextSnap = {
                reinterpret_cast<quintptr>(headNode->next.loadAcquire()),
                headNode->version.loadAcquire()
            };
            
            if (headSnap.ptrBits == reinterpret_cast<quintptr>(
                head.load(std::memory_order_acquire).getPtr())) {
                
                if (headNode == tailSnap.getPtr()) {
                    if (nextSnap.getPtr() == nullptr) {
                        return nullptr; // Queue is empty
                    }
                    // Tail is behind, advance it
                    PackedPointer newTail = PackedPointer::pack(nextSnap.getPtr(),
                                                                tailSnap.version + 1);
                    tail.compare_exchange_weak(tailSnap, newTail,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                } else {
                    T* data = nextSnap.getPtr()->data.loadAcquire();
                    PackedPointer newHead = PackedPointer::pack(nextSnap.getPtr(),
                                                                headSnap.version + 1);
                    
                    if (head.compare_exchange_weak(headSnap, newHead,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                        
                        // Successful dequeue, can delete old head
                        // In production, use hazard pointers or epoch-based reclamation
                        safeDelete(headNode);
                        return data;
                    }
                }
            }
        }
    }
    
private:
    // Requires 128-bit compare-and-swap (cmpxchg16b on x86-64)
    bool compareExchange128(QAtomicPointer<Node> *atomic, 
                           Node *expected, Node *desired,
                           quint64 expectedVer, quint64 desiredVer) {
        #ifdef __x86_64__
            __int128 exp = ((__int128)expectedVer << 64) | 
                          reinterpret_cast<quintptr>(expected);
            __int128 des = ((__int128)desiredVer << 64) | 
                          reinterpret_cast<quintptr>(desired);
            
            return __sync_bool_compare_and_swap_16(
                reinterpret_cast<volatile __int128*>(atomic), exp, des);
        #else
            // Fallback to versioned pointer with two CAS operations
            // (Not truly lock-free but works on all platforms)
            return false;
        #endif
    }
    
    void safeDelete(Node *node) {
        // In production: use hazard pointers or RCU
        // Simple but unsafe: just leak in high-contention scenarios
        static QAtomicInt deletionCount{0};
        if (deletionCount.fetchAndAddRelaxed(1) % 1000 == 0) {
            delete node; // Periodically delete
        }
    }
};
```

## Hazard Pointers for Safe Memory Reclamation

Solves the problem of when it's safe to delete nodes in lock-free data structures:

```cpp
class HazardPointerManager {
private:
    static constexpr int MAX_THREADS = 64;
    static constexpr int HAZARDS_PER_THREAD = 4;
    
    struct alignas(64) HazardRecord {
        QAtomicPointer<void> hazards[HAZARDS_PER_THREAD];
        bool active{false};
        char padding[64 - sizeof(hazards) - sizeof(active)];
    };
    
    HazardRecord records[MAX_THREADS];
    QThreadStorage<int> threadID;
    QAtomicInt nextID{0};
    
    struct RetiredNode {
        void *ptr;
        void (*deleter)(void*);
        RetiredNode *next;
    };
    
    QThreadStorage<RetiredNode*> retiredList;
    static constexpr int RETIRED_THRESHOLD = 100;
    
public:
    static HazardPointerManager& instance() {
        static HazardPointerManager inst;
        return inst;
    }
    
    // Acquire a hazard pointer slot
    void* protect(int slot, void *ptr) {
        int tid = getThreadID();
        records[tid].hazards[slot].storeRelease(ptr);
        return ptr;
    }
    
    // Clear hazard pointer
    void clear(int slot) {
        int tid = getThreadID();
        records[tid].hazards[slot].storeRelease(nullptr);
    }
    
    // Retire a pointer for later deletion
    template<typename T>
    void retire(T *ptr) {
        RetiredNode *node = new RetiredNode{
            static_cast<void*>(ptr),
            [](void *p) { delete static_cast<T*>(p); },
            retiredList.localData()
        };
        retiredList.setLocalData(node);
        
        // Check if we should scan
        if (countRetired() >= RETIRED_THRESHOLD) {
            scan();
        }
    }
    
private:
    int getThreadID() {
        if (!threadID.hasLocalData()) {
            int id = nextID.fetchAndAddAcquire(1);
            Q_ASSERT(id < MAX_THREADS);
            threadID.setLocalData(id);
            records[id].active = true;
        }
        return threadID.localData();
    }
    
    int countRetired() {
        int count = 0;
        RetiredNode *node = retiredList.localData();
        while (node) {
            count++;
            node = node->next;
        }
        return count;
    }
    
    void scan() {
        // Build set of currently protected pointers
        QSet<void*> protected_ptrs;
        
        for (int i = 0; i < MAX_THREADS; i++) {
            if (!records[i].active) continue;
            
            for (int j = 0; j < HAZARDS_PER_THREAD; j++) {
                void *ptr = records[i].hazards[j].loadAcquire();
                if (ptr) {
                    protected_ptrs.insert(ptr);
                }
            }
        }
        
        // Scan retired list and delete unprotected nodes
        RetiredNode *node = retiredList.localData();
        RetiredNode *prev = nullptr;
        RetiredNode *newHead = nullptr;
        
        while (node) {
            if (!protected_ptrs.contains(node->ptr)) {
                // Safe to delete
                node->deleter(node->ptr);
                RetiredNode *temp = node;
                node = node->next;
                delete temp;
            } else {
                // Still protected, keep in list
                if (prev) {
                    prev->next = node;
                } else {
                    newHead = node;
                }
                prev = node;
                node = node->next;
            }
        }
        
        if (prev) {
            prev->next = nullptr;
        }
        
        retiredList.setLocalData(newHead);
    }
};

// Usage in lock-free stack
template<typename T>
class HazardPointerStack {
private:
    struct Node {
        T data;
        QAtomicPointer<Node> next;
    };
    
    QAtomicPointer<Node> head{nullptr};
    
public:
    void push(const T &value) {
        Node *node = new Node{value, {nullptr}};
        Node *oldHead = head.loadAcquire();
        
        do {
            node->next.storeRelease(oldHead);
        } while (!head.testAndSetRelease(oldHead, node));
    }
    
    bool pop(T &result) {
        HazardPointerManager &hp = HazardPointerManager::instance();
        Node *oldHead;
        
        do {
            oldHead = head.loadAcquire();
            if (!oldHead) return false;
            
            // Protect oldHead from deletion
            hp.protect(0, oldHead);
            
            // Verify head hasn't changed (ABA check)
            if (head.loadAcquire() != oldHead) {
                continue;
            }
            
            Node *next = oldHead->next.loadAcquire();
            
            if (head.testAndSetRelease(oldHead, next)) {
                result = oldHead->data;
                hp.clear(0);
                hp.retire(oldHead); // Safe deferred deletion
                return true;
            }
        } while (true);
    }
};
```

## Wait-Free Ring Buffer with Sequence Numbers

Implements a bounded MPMC queue that is wait-free for producers and consumers:

```cpp
template<typename T, size_t SIZE>
class WaitFreeRingBuffer {
private:
    static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be power of 2");
    
    struct alignas(64) Cell {
        QAtomicInteger<quint64> sequence;
        T data;
        char padding[64 - sizeof(QAtomicInteger<quint64>) - sizeof(T)];
    };
    
    Cell buffer[SIZE];
    
    alignas(64) QAtomicInteger<quint64> enqueuePos{0};
    alignas(64) QAtomicInteger<quint64> dequeuePos{0};
    
    static constexpr quint64 INDEX_MASK = SIZE - 1;
    
public:
    WaitFreeRingBuffer() {
        for (size_t i = 0; i < SIZE; i++) {
            buffer[i].sequence.storeRelease(i);
        }
    }
    
    bool enqueue(const T &data) {
        quint64 pos;
        Cell *cell;
        
        while (true) {
            pos = enqueuePos.loadAcquire();
            cell = &buffer[pos & INDEX_MASK];
            quint64 seq = cell->sequence.loadAcquire();
            qint64 diff = static_cast<qint64>(seq) - static_cast<qint64>(pos);
            
            if (diff == 0) {
                // Cell is ready for writing
                if (enqueuePos.testAndSetRelease(pos, pos + 1)) {
                    break;
                }
            } else if (diff < 0) {
                // Buffer is full
                return false;
            }
            // diff > 0: Another thread is ahead, retry
        }
        
        // Write data
        cell->data = data;
        
        // Publish by incrementing sequence
        cell->sequence.storeRelease(pos + 1);
        return true;
    }
    
    bool dequeue(T &data) {
        quint64 pos;
        Cell *cell;
        
        while (true) {
            pos = dequeuePos.loadAcquire();
            cell = &buffer[pos & INDEX_MASK];
            quint64 seq = cell->sequence.loadAcquire();
            qint64 diff = static_cast<qint64>(seq) - static_cast<qint64>(pos + 1);
            
            if (diff == 0) {
                // Cell has data ready
                if (dequeuePos.testAndSetRelease(pos, pos + 1)) {
                    break;
                }
            } else if (diff < 0) {
                // Buffer is empty
                return false;
            }
            // diff > 0: Another thread is ahead, retry
        }
        
        // Read data
        data = cell->data;
        
        // Release cell for reuse
        cell->sequence.storeRelease(pos + SIZE);
        return true;
    }
    
    size_t approximateSize() const {
        qint64 size = static_cast<qint64>(enqueuePos.loadAcquire()) - 
                     static_cast<qint64>(dequeuePos.loadAcquire());
        return size < 0 ? 0 : static_cast<size_t>(size);
    }
};
```

## Read-Copy-Update (RCU) with Grace Periods

Advanced RCU implementation for read-heavy scenarios with deferred reclamation:

```cpp
template<typename T>
class RCUManager {
private:
    struct Version {
        QAtomicPointer<T> data;
        QAtomicInteger<quint64> version;
        QAtomicInt readerCount{0};
    };
    
    static constexpr int MAX_VERSIONS = 3;
    Version versions[MAX_VERSIONS];
    
    QAtomicInteger<quint64> currentVersion{0};
    QAtomicInt writeInProgress{0};
    
    struct GracePeriod {
        quint64 version;
        T *oldData;
        std::function<void(T*)> deleter;
    };
    
    QMutex graceMutex;
    QList<GracePeriod> gracePeriods;
    
public:
    RCUManager() {
        versions[0].data.storeRelease(new T);
        versions[0].version.storeRelease(0);
    }
    
    // Read-side critical section (wait-free)
    class ReadGuard {
    public:
        ReadGuard(RCUManager *mgr) : manager(mgr) {
            quint64 ver = manager->currentVersion.loadAcquire();
            versionIndex = ver % MAX_VERSIONS;
            manager->versions[versionIndex].readerCount.fetchAndAddAcquire(1);
            
            // Memory barrier: ensure increment is visible before reading data
            std::atomic_thread_fence(std::memory_order_acquire);
            
            dataPtr = manager->versions[versionIndex].data.loadAcquire();
        }
        
        ~ReadGuard() {
            manager->versions[versionIndex].readerCount.fetchAndSubRelease(1);
        }
        
        T* operator->() const { return dataPtr; }
        const T& operator*() const { return *dataPtr; }
        
        ReadGuard(const ReadGuard&) = delete;
        ReadGuard& operator=(const ReadGuard&) = delete;
        
    private:
        RCUManager *manager;
        int versionIndex;
        T *dataPtr;
    };
    
    // Update with new data (returns false if update in progress)
    template<typename UpdateFunc>
    bool update(UpdateFunc &&updater) {
        // Only one writer at a time
        int expected = 0;
        if (!writeInProgress.testAndSetAcquire(expected, 1)) {
            return false;
        }
        
        quint64 oldVer = currentVersion.loadAcquire();
        int oldIdx = oldVer % MAX_VERSIONS;
        int newIdx = (oldVer + 1) % MAX_VERSIONS;
        
        // Create new version
        T *oldData = versions[oldIdx].data.loadAcquire();
        T *newData = new T(*oldData); // Copy
        updater(*newData); // Apply update
        
        // Publish new version
        versions[newIdx].data.storeRelease(newData);
        versions[newIdx].version.storeRelease(oldVer + 1);
        versions[newIdx].readerCount.storeRelease(0);
        
        // Switch current version atomically
        currentVersion.storeRelease(oldVer + 1);
        
        // Add old data to grace period tracking
        {
            QMutexLocker locker(&graceMutex);
            gracePeriods.append({oldVer, oldData, [](T *p) { delete p; }});
        }
        
        // Try to reclaim old versions
        reclaimOldVersions();
        
        writeInProgress.storeRelease(0);
        return true;
    }
    
private:
    void reclaimOldVersions() {
        QMutexLocker locker(&graceMutex);
        
        auto it = gracePeriods.begin();
        while (it != gracePeriods.end()) {
            quint64 ver = it->version;
            int idx = ver % MAX_VERSIONS;
            
            // Check if any readers are still accessing this version
            if (versions[idx].readerCount.loadAcquire() == 0 &&
                versions[idx].version.loadAcquire() != ver) {
                
                // Grace period elapsed, safe to delete
                it->deleter(it->oldData);
                it = gracePeriods.erase(it);
            } else {
                ++it;
            }
        }
    }
};

// Usage example
struct Config {
    QString serverUrl;
    int timeout;
    QMap<QString, QString> settings;
};

RCUManager<Config> globalConfig;

// Reader threads (wait-free, no blocking)
void useConfig() {
    auto config = globalConfig.ReadGuard(&globalConfig);
    
    // Access config without any locks
    QString url = config->serverUrl;
    int timeout = config->timeout;
    
    // config automatically released when guard goes out of scope
}

// Writer thread
void updateConfig(const QString &newUrl) {
    globalConfig.update([&](Config &cfg) {
        cfg.serverUrl = newUrl;
        cfg.timeout = 5000;
    });
}
```

## Transactional Memory Simulation with Software TM

Software Transactional Memory for composable lock-free operations:

```cpp
class STMTransaction {
private:
    struct VersionedValue {
        QAtomicInteger<quint64> version{0};
        QAtomicPointer<void> data{nullptr};
        QMutex lock;
    };
    
    struct ReadSet {
        VersionedValue *var;
        quint64 version;
        void *value;
    };
    
    struct WriteSet {
        VersionedValue *var;
        void *oldValue;
        void *newValue;
    };
    
    QList<ReadSet> readSet;
    QList<WriteSet> writeSet;
    bool aborted{false};
    
    static QAtomicInteger<quint64> globalClock;
    quint64 readVersion;
    
public:
    STMTransaction() : readVersion(globalClock.loadAcquire()) {}
    
    template<typename T>
    T read(VersionedValue *var) {
        if (aborted) throw std::runtime_error("Transaction aborted");
        
        // Check write set first
        for (const auto &w : writeSet) {
            if (w.var == var) {
                return *static_cast<T*>(w.newValue);
            }
        }
        
        // Read from memory
        quint64 version = var->version.loadAcquire();
        void *value = var->data.loadAcquire();
        
        // Validate version
        if (version > readVersion) {
            aborted = true;
            throw std::runtime_error("Read-write conflict");
        }
        
        // Add to read set
        readSet.append({var, version, value});
        
        return *static_cast<T*>(value);
    }
    
    template<typename T>
    void write(VersionedValue *var, const T &value) {
        if (aborted) throw std::runtime_error("Transaction aborted");
        
        // Check if already in write set
        for (auto &w : writeSet) {
            if (w.var == var) {
                delete static_cast<T*>(w.newValue);
                w.newValue = new T(value);
                return;
            }
        }
        
        // Add to write set
        void *oldValue = var->data.loadAcquire();
        writeSet.append({var, oldValue, new T(value)});
    }
    
    bool commit() {
        if (aborted) return false;
        if (writeSet.isEmpty()) return true; // Read-only transaction
        
        // Phase 1: Lock write set
        QList<QMutexLocker*> lockers;
        for (auto &w : writeSet) {
            lockers.append(new QMutexLocker(&w.var->lock));
        }
        
        // Phase 2: Validate read set
        for (const auto &r : readSet) {
            if (r.var->version.loadAcquire() != r.version) {
                // Conflict detected
                for (auto *locker : lockers) delete locker;
                cleanup();
                return false;
            }
        }
        
        // Phase 3: Get commit version
        quint64 commitVersion = globalClock.fetchAndAddRelease(1) + 1;
        
        // Phase 4: Write values
        for (auto &w : writeSet) {
            w.var->data.storeRelease(w.newValue);
            w.var->version.storeRelease(commitVersion);
        }
        
        // Phase 5: Release locks
        for (auto *locker : lockers) delete locker;
        
        // Clear sets (don't delete new values - they're now live)
        readSet.clear();
        writeSet.clear();
        
        return true;
    }
    
    ~STMTransaction() {
        if (!writeSet.isEmpty() && aborted) {
            cleanup();
        }
    }
    
private:
    void cleanup() {
        for (auto &w : writeSet) {
            delete static_cast<int*>(w.newValue); // Simplified, needs type-erased deleter
        }
        readSet.clear();
        writeSet.clear();
    }
};

QAtomicInteger<quint64> STMTransaction::globalClock{0};

// Usage with automatic retry
template<typename Func>
auto atomically(Func &&func) -> decltype(func(std::declval<STMTransaction&>())) {
    constexpr int MAX_RETRIES = 100;
    
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        try {
            STMTransaction txn;
            auto result = func(txn);
            
            if (txn.commit()) {
                return result;
            }
            
            // Exponential backoff
            QThread::msleep(1 << std::min(retry, 10));
        } catch (const std::runtime_error &) {
            // Transaction aborted, retry
        }
    }
    
    throw std::runtime_error("Transaction failed after max retries");
}

// Example: Transfer between accounts atomically
STMTransaction::VersionedValue accountA;
STMTransaction::VersionedValue accountB;

void transfer(int amount) {
    atomically([&](STMTransaction &txn) {
        int balanceA = txn.read<int>(&accountA);
        int balanceB = txn.read<int>(&accountB);
        
        if (balanceA >= amount) {
            txn.write(&accountA, balanceA - amount);
            txn.write(&accountB, balanceB + amount);
        }
        
        return true;
    });
}
```

## CPU Cache Line Optimization and False Sharing Elimination

```cpp
// Cache line size detection at compile time
constexpr size_t CACHE_LINE_SIZE = 64;

template<typename T>
struct alignas(CACHE_LINE_SIZE) CacheAligned {
    T value;
    char padding[CACHE_LINE_SIZE - sizeof(T) % CACHE_LINE_SIZE];
    
    CacheAligned() : value() {}
    CacheAligned(const T &v) : value(v) {}
    
    operator T&() { return value; }
    operator const T&() const { return value; }
    T& operator=(const T &v) { value = v; return value; }
};

// Striped counter to eliminate false sharing
template<int STRIPES = 16>
class StripedCounter {
private:
    struct alignas(CACHE_LINE_SIZE) Stripe {
        QAtomicInteger<qint64> count{0};
        char padding[CACHE_LINE_SIZE - sizeof(QAtomicInteger<qint64>)];
    };
    
    Stripe stripes[STRIPES];
    
    static int getStripeIndex() {
        // Use thread ID to select stripe
        return static_cast<int>(
            reinterpret_cast<quintptr>(QThread::currentThreadId()) % STRIPES
        );
    }
    
public:
    void increment() {
        int stripe = getStripeIndex();
        stripes[stripe].count.fetchAndAddRelaxed(1);
    }
    
    qint64 value() const {
        qint64 total = 0;
        for (int i = 0; i < STRIPES; i++) {
            total += stripes[i].count.loadRelaxed();
        }
        return total;
    }
    
    void reset() {
        for (int i = 0; i < STRIPES; i++) {
            stripes[i].count.storeRelaxed(0);
        }
    }
};

// Prefetching for cache optimization
template<typename T>
void prefetchForRead(const T *ptr) {
    #ifdef __GNUC__
        __builtin_prefetch(ptr, 0, 3); // Read, high temporal locality
    #endif
}

template<typename T>
void prefetchForWrite(T *ptr) {
    #ifdef __GNUC__
        __builtin_prefetch(ptr, 1, 3); // Write, high temporal locality
    #endif
}

// Cache-oblivious data structure example
template<typename T>
class CacheObliviousArray {
private:
    T *data;
    size_t size;
    
    // Recursive layout for cache efficiency
    void buildLayout(T *dest, const T *src, size_t start, size_t end, size_t &pos) {
        if (end - start <= CACHE_LINE_SIZE / sizeof(T)) {
            // Base case: fits in cache line, copy linearly
            for (size_t i = start; i < end; i++) {
                dest[pos++] = src[i];
            }
        } else {
            // Recursive case: divide and conquer
            size_t mid = (start + end) / 2;
            buildLayout(dest, src, start, mid, pos);
            buildLayout(dest, src, mid, end, pos);
        }
    }
    
public:
    CacheObliviousArray(const T *source, size_t n) : size(n) {
        data = new T[n];
        size_t pos = 0;
        buildLayout(data, source, 0, n, pos);
    }
    
    ~CacheObliviousArray() {
        delete[] data;
    }
    
    // Optimized sequential access
    class Iterator {
        T *ptr;
        size_t index;
        CacheObliviousArray *array;
        
    public:
        Iterator(CacheObliviousArray *arr, size_t idx) 
            : array(arr), index(idx) {
            ptr = &array->data[index];
            prefetchForRead(ptr + 8); // Prefetch ahead
        }
        
        T& operator*() { return *ptr; }
        
        Iterator& operator++() {
            ++index;
            ++ptr;
            if (index % 8 == 0) {
                prefetchForRead(ptr + 8);
            }
            return *this;
        }
        
        bool operator!=(const Iterator &other) const {
            return index != other.index;
        }
    };
    
    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, size); }
};
```

## Key Insane Takeaways

- **Lock-free MPMC queue** requires 128-bit CAS and ABA prevention with versioning
- **Hazard pointers** solve safe memory reclamation in lock-free structures
- **Wait-free ring buffer** uses sequence numbers for bounded MPMC without locks
- **RCU with grace periods** provides wait-free reads for read-heavy workloads
- **Software TM** enables composable atomic operations but has high overhead
- **Cache line alignment** prevents false sharing - critical for performance
- **Striped counters** distribute contention across cache lines
- **Prefetching** can dramatically improve sequential access patterns
- These techniques require **extensive testing** under load
- **Memory ordering semantics** (acquire/release) are critical for correctness
- Wrong implementation leads to **silent corruption** - use thread sanitizer
- In most cases, **well-designed locking** performs better than complex lock-free code
- **Profile first** - premature lock-free optimization is the root of all evil

## When to Use These Techniques

✅ **Use when:**
- Profiling shows lock contention is your bottleneck
- You need guaranteed progress for real-time systems
- Read-to-write ratio is extremely high (RCU)
- You're building core infrastructure (language runtimes, databases)

❌ **Avoid when:**
- Lock-based solution performs adequately
- Team lacks expertise in concurrent algorithms
- Code must be maintained by average developers
- Correctness is more important than performance

Remember: **The best lock-free code is the code you don't write.**
