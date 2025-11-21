# Lesson 17: Threading and Concurrency - Intermediate

## QThreadPool and QRunnable Deep Dive

QThreadPool manages a pool of reusable threads, automatically limiting concurrency to the ideal thread count for your CPU. This is more efficient than creating new QThread objects for short-lived tasks.

### Basic QRunnable Usage

```cpp
class ImageProcessor : public QRunnable {
public:
    ImageProcessor(const QString &path) : imagePath(path) {
        setAutoDelete(true); // Automatically delete after run()
    }
    
    void run() override {
        QImage image(imagePath);
        // Process image - resize, filter, etc.
        QImage processed = image.scaled(800, 600);
        processed.save(imagePath.replace(".jpg", "_thumb.jpg"));
    }
    
private:
    QString imagePath;
};

// Submit tasks to the global thread pool
QThreadPool::globalInstance()->start(new ImageProcessor("photo1.jpg"));
QThreadPool::globalInstance()->start(new ImageProcessor("photo2.jpg"));
QThreadPool::globalInstance()->start(new ImageProcessor("photo3.jpg"));
```

### Controlling Thread Pool Behavior

```cpp
QThreadPool *pool = QThreadPool::globalInstance();

// Check and set thread count
int idealCount = QThread::idealThreadCount(); // Usually CPU core count
pool->setMaxThreadCount(idealCount);

// Set expiry timeout for idle threads (ms)
pool->setExpiryTimeout(30000); // 30 seconds

// Wait for all tasks to complete
pool->waitForDone(); // Blocks until all tasks finish
pool->waitForDone(5000); // Wait max 5 seconds

// Check pool status
int active = pool->activeThreadCount();
qDebug() << "Active threads:" << active << "/ Max:" << pool->maxThreadCount();
```

### Priority Execution

```cpp
class HighPriorityTask : public QRunnable {
public:
    HighPriorityTask(int priority) {
        // Higher priority = runs first (default is 0)
        // Not to be confused with thread priority!
    }
    void run() override {
        // Critical task
    }
};

QThreadPool::globalInstance()->start(new NormalTask(), 0);      // Normal priority
QThreadPool::globalInstance()->start(new ImportantTask(), 10);  // Higher priority
QThreadPool::globalInstance()->start(new CriticalTask(), 100);  // Highest priority
```

### Creating Custom Thread Pools

```cpp
class DownloadManager {
public:
    DownloadManager() {
        // Dedicated pool for downloads, limit to 4 concurrent
        downloadPool.setMaxThreadCount(4);
        downloadPool.setExpiryTimeout(-1); // Threads never expire
    }
    
    void downloadFile(const QUrl &url) {
        downloadPool.start(new FileDownloader(url));
    }
    
    void waitForAllDownloads() {
        downloadPool.waitForDone();
    }
    
private:
    QThreadPool downloadPool; // Custom pool, not global
};
```

## QMutex and Locking Strategies

### Basic Mutex Protection

```cpp
class ThreadSafeCounter {
public:
    void increment() {
        QMutexLocker locker(&mutex); // Locks on construction
        ++count;
        // Automatically unlocks when locker goes out of scope (RAII)
    }
    
    int value() const {
        QMutexLocker locker(&mutex);
        return count;
    }
    
private:
    mutable QMutex mutex; // mutable so const methods can lock
    int count = 0;
};
```

### Recursive Mutex

When a function needs to lock a mutex it might already own:

```cpp
class Account {
public:
    void deposit(int amount) {
        QMutexLocker locker(&mutex);
        balance += amount;
        logTransaction("deposit", amount); // Calls another method that locks
    }
    
    void withdraw(int amount) {
        QMutexLocker locker(&mutex);
        if (balance >= amount) {
            balance -= amount;
            logTransaction("withdraw", amount);
        }
    }
    
private:
    void logTransaction(const QString &type, int amount) {
        QMutexLocker locker(&mutex); // Would deadlock with QMutex!
        // This works because we use QRecursiveMutex
        transactions.append(QString("%1: %2").arg(type).arg(amount));
    }
    
    QRecursiveMutex mutex; // Allows same thread to lock multiple times
    int balance = 0;
    QStringList transactions;
};
```

### Try-Lock Pattern

Non-blocking lock attempts:

```cpp
class DataCache {
public:
    QByteArray getData(const QString &key) {
        if (mutex.tryLock(100)) { // Try to lock, wait max 100ms
            QByteArray data = cache.value(key);
            mutex.unlock();
            return data;
        } else {
            // Couldn't get lock, return default
            qDebug() << "Cache busy, returning default";
            return QByteArray();
        }
    }
    
    void setData(const QString &key, const QByteArray &data) {
        QMutexLocker locker(&mutex);
        cache.insert(key, data);
    }
    
private:
    QMutex mutex;
    QHash<QString, QByteArray> cache;
};
```

### Lock-Free Alternatives

For simple atomic operations, use QAtomicInt/QAtomicPointer instead of mutex:

```cpp
class AtomicCounter {
public:
    void increment() {
        counter.fetchAndAddRelaxed(1); // No mutex needed!
    }
    
    int value() const {
        return counter.loadRelaxed();
    }
    
    // Compare-and-swap operation
    bool compareAndSet(int expected, int newValue) {
        return counter.testAndSetOrdered(expected, newValue);
    }
    
private:
    QAtomicInt counter{0};
};
```

## QReadWriteLock for Shared Data

When you have many readers and few writers, QReadWriteLock is more efficient than QMutex:

```cpp
class PhoneBook {
public:
    QString lookup(const QString &name) const {
        QReadLocker locker(&lock); // Multiple readers can proceed simultaneously
        return contacts.value(name);
    }
    
    QStringList allNames() const {
        QReadLocker locker(&lock);
        return contacts.keys();
    }
    
    void addContact(const QString &name, const QString &number) {
        QWriteLocker locker(&lock); // Exclusive access for writing
        contacts.insert(name, number);
    }
    
    void removeContact(const QString &name) {
        QWriteLocker locker(&lock);
        contacts.remove(name);
    }
    
private:
    mutable QReadWriteLock lock;
    QHash<QString, QString> contacts;
};

// Usage: Many threads can call lookup() simultaneously
// But addContact() blocks all readers and other writers
```

### Read-Write Lock Recursion

```cpp
class DocumentEditor {
public:
    QString getTitle() const {
        QReadLocker locker(&lock);
        return title;
    }
    
    QString getFullMetadata() const {
        QReadLocker locker(&lock);
        // Can't call getTitle() here - would try to acquire read lock again
        // Use QReadWriteLock::Recursive mode:
        return title + " - " + author; // Access data directly instead
    }
    
private:
    mutable QReadWriteLock lock;
    QString title;
    QString author;
};
```

## QSemaphore for Resource Counting

QSemaphore controls access to a limited number of identical resources:

```cpp
class DatabaseConnectionPool {
public:
    DatabaseConnectionPool(int maxConnections) 
        : available(maxConnections), maxConn(maxConnections) 
    {
        for (int i = 0; i < maxConnections; ++i) {
            connections.append(createConnection());
        }
    }
    
    QSqlDatabase acquire() {
        available.acquire(); // Blocks if no connections available
        QMutexLocker locker(&mutex);
        return connections.takeFirst();
    }
    
    void release(QSqlDatabase conn) {
        QMutexLocker locker(&mutex);
        connections.append(conn);
        available.release(); // Signal that a connection is available
    }
    
    bool tryAcquire(QSqlDatabase &conn, int timeout = 1000) {
        if (available.tryAcquire(1, timeout)) {
            QMutexLocker locker(&mutex);
            conn = connections.takeFirst();
            return true;
        }
        return false;
    }
    
private:
    QSemaphore available;
    QMutex mutex;
    QList<QSqlDatabase> connections;
    int maxConn;
    
    QSqlDatabase createConnection() {
        // Create and configure database connection
        return QSqlDatabase::addDatabase("QSQLITE");
    }
};

// Usage
DatabaseConnectionPool pool(5); // Max 5 concurrent connections

void queryDatabase() {
    QSqlDatabase db = pool.acquire(); // Blocks if all 5 in use
    QSqlQuery query(db);
    query.exec("SELECT * FROM users");
    // Process results...
    pool.release(db); // Return to pool
}
```

### Producer-Consumer with Semaphores

```cpp
const int BufferSize = 100;
QSemaphore freeSpace(BufferSize); // Available slots
QSemaphore usedSpace(0);          // Filled slots
QQueue<QByteArray> buffer;
QMutex bufferMutex;

void producer() {
    while (true) {
        QByteArray data = generateData();
        
        freeSpace.acquire(); // Wait for free space
        {
            QMutexLocker locker(&bufferMutex);
            buffer.enqueue(data);
        }
        usedSpace.release(); // Signal data available
    }
}

void consumer() {
    while (true) {
        usedSpace.acquire(); // Wait for data
        QByteArray data;
        {
            QMutexLocker locker(&bufferMutex);
            data = buffer.dequeue();
        }
        freeSpace.release(); // Signal space available
        
        processData(data);
    }
}
```

## QWaitCondition for Complex Synchronization

QWaitCondition allows threads to wait for specific conditions:

```cpp
class MessageQueue {
public:
    void enqueue(const QString &message) {
        QMutexLocker locker(&mutex);
        queue.enqueue(message);
        condition.wakeOne(); // Wake one waiting thread
    }
    
    QString dequeue() {
        QMutexLocker locker(&mutex);
        while (queue.isEmpty()) {
            condition.wait(&mutex); // Atomically unlock and wait
            // Mutex is re-locked when wait() returns
        }
        return queue.dequeue();
    }
    
    // Dequeue with timeout
    QString dequeueWithTimeout(int ms) {
        QMutexLocker locker(&mutex);
        while (queue.isEmpty()) {
            if (!condition.wait(&mutex, ms)) {
                return QString(); // Timeout, return empty
            }
        }
        return queue.dequeue();
    }
    
    void clear() {
        QMutexLocker locker(&mutex);
        queue.clear();
        condition.wakeAll(); // Wake all waiting threads
    }
    
private:
    QMutex mutex;
    QWaitCondition condition;
    QQueue<QString> queue;
};
```

### Multiple Conditions

```cpp
class WorkQueue {
public:
    void addWork(const Task &task, bool highPriority = false) {
        QMutexLocker locker(&mutex);
        if (highPriority) {
            highPriorityQueue.enqueue(task);
            highPriorityAvailable.wakeOne();
        } else {
            normalQueue.enqueue(task);
            normalAvailable.wakeOne();
        }
    }
    
    Task getWork() {
        QMutexLocker locker(&mutex);
        
        // Try high priority first
        while (highPriorityQueue.isEmpty() && normalQueue.isEmpty()) {
            normalAvailable.wait(&mutex); // Wait for any work
        }
        
        if (!highPriorityQueue.isEmpty()) {
            return highPriorityQueue.dequeue();
        } else {
            return normalQueue.dequeue();
        }
    }
    
    void shutdown() {
        QMutexLocker locker(&mutex);
        shuttingDown = true;
        highPriorityAvailable.wakeAll();
        normalAvailable.wakeAll();
    }
    
private:
    QMutex mutex;
    QWaitCondition highPriorityAvailable;
    QWaitCondition normalAvailable;
    QQueue<Task> highPriorityQueue;
    QQueue<Task> normalQueue;
    bool shuttingDown = false;
};
```

## Key Intermediate Takeaways

- **QThreadPool** is ideal for managing many short-lived tasks - more efficient than creating QThread instances
- **setAutoDelete(true)** on QRunnable ensures automatic cleanup
- **QMutex vs QRecursiveMutex**: Use recursive only when necessary (performance cost)
- **QMutexLocker** provides RAII - always prefer it over manual lock/unlock
- **QReadWriteLock** dramatically improves performance for read-heavy scenarios
- **QSemaphore** manages N identical resources elegantly
- **QWaitCondition** enables complex thread coordination patterns
- **QAtomicInt/QAtomicPointer** for lock-free simple counters and flags
- **tryLock()** with timeout prevents indefinite blocking
- Always hold locks for the **minimum time** necessary - long locks kill concurrency
