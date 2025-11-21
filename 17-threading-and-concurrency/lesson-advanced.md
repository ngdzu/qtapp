# Lesson 17: Threading and Concurrency - Advanced

## QtConcurrent Advanced Patterns

### Map-Reduce with Custom Functions

Process large datasets in parallel and combine results:

```cpp
QList<QString> files = getAllLogFiles(); // Thousands of files

// Map: Extract error count from each file (runs in parallel)
auto extractErrors = [](const QString &file) -> int {
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) return 0;
    
    int errors = 0;
    while (!f.atEnd()) {
        if (f.readLine().contains("ERROR"))
            ++errors;
    }
    return errors;
};

// Reduce: Sum all error counts
auto sumErrors = [](int &total, int fileErrors) {
    total += fileErrors;
};

// Execute in parallel
QFuture<int> future = QtConcurrent::mappedReduced(
    files, 
    extractErrors,
    sumErrors
);

int totalErrors = future.result(); // Blocks until complete
qDebug() << "Total errors across all logs:" << totalErrors;
```

### Filtered with In-Place Modification

```cpp
QList<QImage> images = loadImages();

// Filter: Keep only images larger than 1MB
auto isLarge = [](const QImage &img) {
    return img.sizeInBytes() > 1024 * 1024;
};

QtConcurrent::blockingFilter(images, isLarge); // Modifies images in-place
// images now contains only large images

// Alternative: Create new filtered list without modifying original
QList<QImage> largeImages = QtConcurrent::blockingFiltered(images, isLarge);
```

### Map with Progress Tracking

```cpp
QList<QString> videos = getVideoPaths();
QProgressDialog dialog("Converting videos...", "Cancel", 0, videos.size());

auto convertVideo = [](const QString &path) -> QString {
    // Conversion work...
    return path.replace(".avi", ".mp4");
};

QFutureWatcher<QString> watcher;
connect(&watcher, &QFutureWatcher<QString>::progressValueChanged,
        &dialog, &QProgressDialog::setValue);
connect(&watcher, &QFutureWatcher<QString>::finished,
        &dialog, &QProgressDialog::reset);
connect(&dialog, &QProgressDialog::canceled,
        &watcher, &QFutureWatcher<QString>::cancel);

QFuture<QString> future = QtConcurrent::mapped(videos, convertVideo);
watcher.setFuture(future);

// Wait for completion
QList<QString> converted = future.results();
```

### Custom Iterators

QtConcurrent works with any STL-compatible container:

```cpp
std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Process subset using iterators
QFuture<int> future = QtConcurrent::mapped(
    numbers.begin() + 2,  // Start from 3rd element
    numbers.end() - 1,     // Stop before last element
    [](int x) { return x * x; }
);

QList<int> results = future.results(); // [9, 16, 25, 36, 49, 64, 81]
```

### FilteredReduced - Combine Filter and Reduce

```cpp
QList<QString> transactions = loadTransactions();

// Filter valid transactions and sum their amounts
auto isValid = [](const QString &t) {
    return t.startsWith("VALID:");
};

auto extractAmount = [](const QString &t) -> double {
    return t.split(":").last().toDouble();
};

auto sumAmounts = [](double &total, double amount) {
    total += amount;
};

QFuture<double> future = QtConcurrent::filteredReduced(
    transactions,
    isValid,
    [&](double &total, const QString &t) {
        total += extractAmount(t);
    }
);

double validTotal = future.result();
```

## Advanced QThread Patterns

### Thread with Event Loop

Create a thread that processes events and can receive signals:

```cpp
class EventProcessor : public QObject {
    Q_OBJECT
public:
    EventProcessor() {
        moveToThread(&thread);
        thread.start();
    }
    
    ~EventProcessor() {
        thread.quit();
        thread.wait();
    }
    
public slots:
    void processEvent(const QString &event) {
        // This runs in the worker thread's event loop
        qDebug() << "Processing" << event << "on thread" << QThread::currentThreadId();
        
        // Can do asynchronous work here
        QTimer::singleShot(1000, this, [this, event]() {
            emit eventProcessed(event);
        });
    }
    
signals:
    void eventProcessed(const QString &event);
    
private:
    QThread thread;
};

// Usage from main thread
EventProcessor processor;
QObject::connect(mainObject, &Main::eventOccurred,
                 &processor, &EventProcessor::processEvent);
// Signal automatically queued to worker thread's event loop
```

### Thread Priority Control

```cpp
QThread thread;
thread.start();

// Set thread priority (OS-dependent)
thread.setPriority(QThread::HighPriority);
// Priorities: IdlePriority, LowestPriority, LowPriority, NormalPriority,
//             HighPriority, HighestPriority, TimeCriticalPriority, InheritPriority

// Check if thread is running
if (thread.isRunning()) {
    qDebug() << "Thread is active";
}

// Request interruption (cooperative)
thread.requestInterruption();

// Worker checks periodically:
void Worker::process() {
    while (!thread()->isInterruptionRequested()) {
        // Do work
    }
}
```

### Thread Affinity and Cross-Thread Method Calls

```cpp
class DataProcessor : public QObject {
    Q_OBJECT
public:
    void processInCurrentThread(const QByteArray &data) {
        qDebug() << "Processing on" << QThread::currentThreadId();
        // Process data...
        emit dataProcessed();
    }
    
signals:
    void dataProcessed();
};

// Create processor in main thread
DataProcessor *processor = new DataProcessor;

// Move to worker thread
QThread *workerThread = new QThread;
processor->moveToThread(workerThread);
workerThread->start();

// Invoke method in worker thread from main thread
QMetaObject::invokeMethod(processor, 
    "processInCurrentThread",
    Qt::QueuedConnection,  // Queued = runs in processor's thread
    Q_ARG(QByteArray, data));

// Can also use BlockingQueuedConnection to wait for result
QByteArray result;
QMetaObject::invokeMethod(processor,
    "processAndReturn",
    Qt::BlockingQueuedConnection, // Blocks until complete
    Q_RETURN_ARG(QByteArray, result),
    Q_ARG(QByteArray, input));
```

## Thread-Local Storage Deep Dive

### QThreadStorage for Per-Thread Data

```cpp
class DatabaseManager {
public:
    QSqlDatabase getConnection() {
        // Each thread gets its own database connection
        if (!connections.hasLocalData()) {
            QSqlDatabase db = QSqlDatabase::addDatabase(
                "QSQLITE", 
                QString("conn_%1").arg((quintptr)QThread::currentThreadId())
            );
            db.setDatabaseName("mydb.sqlite");
            db.open();
            connections.setLocalData(db);
        }
        return connections.localData();
    }
    
private:
    QThreadStorage<QSqlDatabase> connections;
};

// Each thread calling getConnection() gets its own QSqlDatabase instance
// No locking needed - thread-safe by design
```

### Thread-Local Cache

```cpp
class ImageCache {
public:
    QPixmap getPixmap(const QString &path) {
        if (!cache.hasLocalData()) {
            cache.setLocalData(new QCache<QString, QPixmap>(100));
        }
        
        QCache<QString, QPixmap> *localCache = cache.localData();
        
        if (!localCache->contains(path)) {
            QPixmap pixmap(path);
            localCache->insert(path, new QPixmap(pixmap));
        }
        
        return *localCache->object(path);
    }
    
private:
    QThreadStorage<QCache<QString, QPixmap>*> cache;
};

// Each thread maintains its own 100-item pixmap cache
// No contention between threads
```

## Lock-Free Programming with Atomics

### Atomic Flags and Operations

```cpp
class LockFreeQueue {
public:
    void enqueue(int value) {
        Node *node = new Node{value, nullptr};
        Node *oldTail;
        
        do {
            oldTail = tail.loadAcquire();
            node->next.storeRelease(nullptr);
        } while (!tail.testAndSetOrdered(oldTail, node));
        
        oldTail->next.storeRelease(node);
    }
    
    bool dequeue(int &value) {
        Node *oldHead;
        
        do {
            oldHead = head.loadAcquire();
            if (!oldHead) return false;
            
            Node *next = oldHead->next.loadAcquire();
            if (!next) return false;
            
            value = next->value;
        } while (!head.testAndSetOrdered(oldHead, oldHead->next.loadAcquire()));
        
        return true;
    }
    
private:
    struct Node {
        int value;
        QAtomicPointer<Node> next;
    };
    
    QAtomicPointer<Node> head{new Node{0, nullptr}};
    QAtomicPointer<Node> tail{head.loadAcquire()};
};
```

### Memory Ordering

```cpp
class SharedData {
public:
    void publishData(int value) {
        data = value;
        ready.storeRelease(1); // Release: all writes before this are visible
    }
    
    int consumeData() {
        while (ready.loadAcquire() == 0) { // Acquire: see all writes after store
            QThread::yieldCurrentThread();
        }
        return data;
    }
    
private:
    int data = 0;
    QAtomicInt ready{0};
};

// Memory ordering guarantees:
// - loadAcquire() sees all writes before corresponding storeRelease()
// - Prevents compiler/CPU reordering across the atomic operation
```

## Advanced Synchronization Patterns

### Read-Copy-Update (RCU) Pattern

```cpp
class ConfigManager {
public:
    ConfigManager() : config(new Config) {}
    
    Config getConfig() const {
        QReadLocker locker(&lock);
        return *config; // Return copy
    }
    
    void updateConfig(const Config &newConfig) {
        // Create new config object
        Config *updated = new Config(newConfig);
        
        // Swap pointer atomically
        {
            QWriteLocker locker(&lock);
            Config *old = config;
            config = updated;
            // old will be deleted after write lock released
            QTimer::singleShot(100, [old]() { delete old; });
        }
        
        // Readers that started before update finish with old config
        // New readers get new config
        // No reader is ever blocked!
    }
    
private:
    mutable QReadWriteLock lock;
    Config *config;
};
```

### Double-Checked Locking (Singleton Pattern)

```cpp
class Singleton {
public:
    static Singleton* instance() {
        // First check without locking (fast path)
        Singleton *tmp = inst.loadAcquire();
        if (!tmp) {
            // Lock and check again
            QMutexLocker locker(&mutex);
            tmp = inst.loadAcquire();
            if (!tmp) {
                tmp = new Singleton;
                inst.storeRelease(tmp);
            }
        }
        return tmp;
    }
    
private:
    Singleton() = default;
    static QAtomicPointer<Singleton> inst;
    static QMutex mutex;
};

QAtomicPointer<Singleton> Singleton::inst{nullptr};
QMutex Singleton::mutex;
```

### Thread-Safe Lazy Initialization

```cpp
class LazyResource {
public:
    ExpensiveObject* get() {
        // Q_GLOBAL_STATIC alternative for lazy initialization
        static QMutex mutex;
        static QAtomicPointer<ExpensiveObject> instance{nullptr};
        
        ExpensiveObject *tmp = instance.loadAcquire();
        if (!tmp) {
            QMutexLocker locker(&mutex);
            tmp = instance.loadAcquire();
            if (!tmp) {
                tmp = new ExpensiveObject;
                instance.storeRelease(tmp);
            }
        }
        return tmp;
    }
};

// Qt 6 also provides Q_GLOBAL_STATIC for this pattern
Q_GLOBAL_STATIC(ExpensiveObject, globalObject)
// Access: globalObject() returns pointer
```

## Performance Optimization

### Reduce Lock Contention

```cpp
// BAD: Fine-grained locking on every operation
class BadCounter {
    void increment() {
        QMutexLocker locker(&mutex);
        ++count; // Lock held for trivial operation
    }
    QMutex mutex;
    int count = 0;
};

// GOOD: Thread-local counters, periodic merge
class GoodCounter {
public:
    void increment() {
        if (!localCount.hasLocalData())
            localCount.setLocalData(0);
        localCount.setLocalData(localCount.localData() + 1);
    }
    
    int total() {
        int sum = 0;
        // Each thread's local count is independent
        // Merge when needed (rare operation)
        return globalCount.loadAcquire() + getAllLocalCounts();
    }
    
private:
    QThreadStorage<int> localCount;
    QAtomicInt globalCount{0};
};
```

### Avoid False Sharing

```cpp
// BAD: Adjacent data accessed by different threads
struct BadLayout {
    int threadACounter;  // These might be on same cache line
    int threadBCounter;  // Cache line ping-pong = slow!
};

// GOOD: Pad to separate cache lines
struct GoodLayout {
    alignas(64) int threadACounter;  // 64-byte cache line alignment
    char padding[64 - sizeof(int)];
    alignas(64) int threadBCounter;
};

// Or use QAtomicInt which handles this internally
```

### Lock Hierarchies to Prevent Deadlock

```cpp
class Account {
public:
    void transfer(Account &to, int amount) {
        // Always lock accounts in same order (by address)
        Account *first = this < &to ? this : &to;
        Account *second = this < &to ? &to : this;
        
        QMutexLocker lock1(&first->mutex);
        QMutexLocker lock2(&second->mutex);
        
        if (this != &to && balance >= amount) {
            balance -= amount;
            to.balance += amount;
        }
    }
    
private:
    QMutex mutex;
    int balance = 0;
};
```

## Debugging Threading Issues

### Detect Race Conditions with QMutexLocker Asserts

```cpp
class ProtectedData {
public:
    void modifyData() {
        QMutexLocker locker(&mutex);
        Q_ASSERT(locker.isLocked()); // Debug assertion
        data = "modified";
    }
    
private:
    QMutex mutex;
    QString data;
};
```

### Thread Sanitizer Integration

Compile with thread sanitizer to detect data races:

```bash
# CMake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

# Run and check for race condition reports
./myapp
```

### Logging Thread Context

```cpp
void debugThreadContext(const QString &message) {
    qDebug() << QThread::currentThreadId() 
             << QThread::currentThread()->objectName()
             << message;
}

// Set thread names for easier debugging
workerThread->setObjectName("WorkerThread-1");
```

## Key Advanced Takeaways

- **QtConcurrent** map/reduce scales automatically to CPU cores - ideal for data parallelism
- **QThreadStorage** eliminates lock contention for per-thread data
- **QAtomicInt/QAtomicPointer** with proper memory ordering enables lock-free algorithms
- **Double-checked locking** requires atomics with acquire/release semantics
- **Lock hierarchies** (always acquire locks in same order) prevent deadlocks
- **False sharing** between cache lines kills performance - use alignment
- **QMetaObject::invokeMethod** with Qt::BlockingQueuedConnection can deadlock - avoid in event loop
- **Thread sanitizers** catch races that testing misses
- **Lock-free algorithms** are fast but extremely difficult to get right
- **Profile before optimizing** - premature lock removal causes bugs
