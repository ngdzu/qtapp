# Lesson 17: Threading and Concurrency - Debugging and Profiling

## Introduction

Threading bugs are among the most difficult to debug because they're often non-deterministic - they may only appear under specific timing conditions, high load, or particular hardware. This lesson covers comprehensive techniques, tools, and strategies for identifying, diagnosing, and fixing the 30 most common threading problems.

## Essential Debugging Tools

### 1. Valgrind Helgrind - Race Condition Detection

Helgrind detects data races, lock ordering problems, and misuse of POSIX threading API:

```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with Helgrind
valgrind --tool=helgrind --log-file=helgrind.log ./myapp

# Common output for race condition:
# ==12345== Possible data race during write of size 4 at 0x12345678
# ==12345==    at 0x401234: Worker::increment() (worker.cpp:42)
# ==12345==    by 0x401567: Worker::process() (worker.cpp:89)
# ==12345==  This conflicts with a previous read of size 4 by thread 2
# ==12345==    at 0x401890: Worker::getValue() (worker.cpp:55)
```

**Example: Finding a race condition**

```cpp
// Buggy code
class Counter {
    int count = 0;  // Not protected!
public:
    void increment() {
        count++;  // Race condition - Helgrind will catch this
    }
    
    int value() const {
        return count;  // Race condition with increment()
    }
};

// Helgrind output analysis:
// Thread 1: increment() writing to count
// Thread 2: value() reading from count
// Solution: Add QMutex protection
```

### 2. Thread Sanitizer (TSan) - Modern Race Detection

ThreadSanitizer is faster and more accurate than Helgrind:

```cmake
# CMakeLists.txt
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread -g -O1")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=thread")
endif()
```

```bash
# Build and run
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./myapp

# TSan output for data race:
# WARNING: ThreadSanitizer: data race (pid=12345)
#   Write of size 4 at 0x7b0c00001234 by thread T1:
#     #0 Counter::increment() counter.cpp:15
#     #1 WorkerThread::run() worker.cpp:42
#
#   Previous read of size 4 at 0x7b0c00001234 by main thread:
#     #0 Counter::value() const counter.cpp:20
#     #1 main main.cpp:67
```

**TSan Environment Variables:**

```bash
# Increase history size for better stack traces
export TSAN_OPTIONS="history_size=7"

# Suppress known issues
export TSAN_OPTIONS="suppressions=tsan_suppressions.txt"

# tsan_suppressions.txt
race:^ThirdPartyLibrary::
race:known_benign_race
```

### 3. GDB with Thread Debugging

```bash
# Start GDB
gdb ./myapp

# List all threads
(gdb) info threads
  Id   Target Id         Frame
* 1    Thread 0x7ffff7fe1740 (LWP 12345) main (argc=1, argv=0x7fffffffde58)
  2    Thread 0x7ffff6fe0700 (LWP 12346) Worker::process() at worker.cpp:42
  3    Thread 0x7ffff5fdf700 (LWP 12347) Worker::process() at worker.cpp:42

# Switch to specific thread
(gdb) thread 2
[Switching to thread 2 (Thread 0x7ffff6fe0700)]

# See backtrace of current thread
(gdb) bt
#0  Worker::process() at worker.cpp:42
#1  QThread::run() at qthread.cpp:123
#2  start_thread () at pthread_create.c:456

# See backtrace of ALL threads
(gdb) thread apply all bt

# Set breakpoint that triggers only in specific thread
(gdb) break worker.cpp:42 thread 2
(gdb) condition 1 threadId == 2

# Freeze all threads except current
(gdb) set scheduler-locking on

# Step through current thread without other threads interfering
(gdb) next
(gdb) step
```

**Advanced GDB Threading Example:**

```gdb
# Find deadlock - check what each thread is waiting for
(gdb) thread apply all bt

# Thread 1 backtrace shows:
# #0  __lll_lock_wait () at lowlevellock.c:135
# #1  QMutex::lock() at qmutex.cpp:234
# #2  Account::transfer() at account.cpp:45

# Thread 2 backtrace shows:
# #0  __lll_lock_wait () at lowlevellock.c:135
# #1  QMutex::lock() at qmutex.cpp:234
# #2  Account::transfer() at account.cpp:47

# Both threads waiting for mutexes - classic deadlock!
# Examine the mutex addresses:
(gdb) thread 1
(gdb) frame 2
(gdb) print &mutexA
(gdb) thread 2
(gdb) frame 2
(gdb) print &mutexB

# If thread 1 holds mutexA and waits for mutexB,
# while thread 2 holds mutexB and waits for mutexA = DEADLOCK
```

### 4. Qt Creator's Thread Debugger

Enable in Qt Creator: Debug → Start Debugging → Attach to Running Application

**Thread View shows:**
- All active threads with names
- Current state (Running/Blocked/Waiting)
- Stack trace for each thread
- Lock wait chains

**Set thread names for easier debugging:**

```cpp
void Worker::run() {
    QThread::currentThread()->setObjectName("DataProcessor-" + QString::number(id));
    // Now appears as "DataProcessor-1" in debugger
}
```

### 5. perf - Linux Performance Analysis

Profile thread contention and CPU usage:

```bash
# Record all threads with call graphs
perf record -g --call-graph dwarf -p $(pidof myapp)

# Let it run for 10 seconds, then Ctrl+C

# View report
perf report

# See which functions cause most lock contention
perf record -e syscalls:sys_enter_futex -p $(pidof myapp)
perf report

# Flamegraph visualization
perf script | ./flamegraph.pl > perf.svg
```

**Interpreting perf output:**

```
# 45% of CPU time in QMutex::lock() = high contention!
  45.23%  myapp  libQt6Core.so  [.] QMutex::lock
  23.11%  myapp  myapp          [.] DataProcessor::process
  12.45%  myapp  libQt6Core.so  [.] QMutexLocker::~QMutexLocker
```

### 6. Intel VTune Profiler

Commercial tool for advanced threading analysis:

```bash
# Collect threading metrics
vtune -collect threading -result-dir vtune_results -- ./myapp

# View in GUI
vtune-gui vtune_results
```

**VTune shows:**
- Thread utilization timeline
- Lock wait times
- False sharing detection
- Cache misses per thread

## The 30 Most Common Threading Problems

### Problem 1: Basic Data Race

**Symptom:** Occasional crashes, corrupted data, inconsistent results

```cpp
// BUG: Race condition on shared variable
class BuggyCounter {
    int count = 0;  // Multiple threads access without protection
public:
    void increment() {
        count++;  // NOT atomic! Read-modify-write = 3 operations
    }
};

// Debugging approach:
// 1. Run with TSan: -fsanitize=thread
// 2. TSan will report: "data race on count"
// 3. Add logging to see interleaving:

void increment() {
    qDebug() << "Thread" << QThread::currentThreadId() 
             << "reading count=" << count;
    int temp = count + 1;
    qDebug() << "Thread" << QThread::currentThreadId() 
             << "writing count=" << temp;
    count = temp;
}

// Output shows interleaving:
// Thread 0x123 reading count=5
// Thread 0x456 reading count=5  <-- Both read same value!
// Thread 0x123 writing count=6
// Thread 0x456 writing count=6  <-- Lost update!

// SOLUTION:
class FixedCounter {
    QAtomicInt count{0};  // or QMutex + int
public:
    void increment() {
        count.fetchAndAddRelaxed(1);
    }
};
```

### Problem 2: Deadlock - Circular Wait

**Symptom:** Application hangs, threads stuck in lock() calls

```cpp
// BUG: Classic deadlock scenario
QMutex mutexA, mutexB;

void thread1() {
    mutexA.lock();
    QThread::msleep(10);  // Timing window
    mutexB.lock();  // Waits for thread2
    // Work...
    mutexB.unlock();
    mutexA.unlock();
}

void thread2() {
    mutexB.lock();
    QThread::msleep(10);  // Timing window
    mutexA.lock();  // Waits for thread1 - DEADLOCK!
    // Work...
    mutexA.unlock();
    mutexB.unlock();
}

// Debugging approach:
// 1. Application hangs - attach GDB:
gdb -p $(pidof myapp)

// 2. Check all thread states:
(gdb) thread apply all bt

// Output shows:
// Thread 1: waiting in mutexB.lock()
// Thread 2: waiting in mutexA.lock()

// 3. Examine lock addresses:
(gdb) thread 1
(gdb) print &mutexB
$1 = (QMutex *) 0x55555556789a

(gdb) thread 2
(gdb) print &mutexA
$2 = (QMutex *) 0x555555567890

// 4. Use Helgrind to detect lock order inconsistency:
valgrind --tool=helgrind ./myapp

// Helgrind output:
// Thread #1: lock order "0x567890 before 0x56789a"
// Thread #2: lock order "0x56789a before 0x567890"
// Lock order violated

// SOLUTION: Always acquire locks in consistent order
void safeLockBoth() {
    QMutex *first = &mutexA < &mutexB ? &mutexA : &mutexB;
    QMutex *second = &mutexA < &mutexB ? &mutexB : &mutexA;
    first->lock();
    second->lock();
    // Work...
    second->unlock();
    first->unlock();
}
```

### Problem 3: Forgotten Mutex Unlock

**Symptom:** Threads hang waiting for mutex that's never released

```cpp
// BUG: Exception before unlock
void procesData() {
    mutex.lock();
    
    if (data.isEmpty()) {
        return;  // BUG: Forgot to unlock!
    }
    
    processDataInternal();  // May throw exception - mutex stays locked!
    
    mutex.unlock();
}

// Debugging approach:
// 1. Application hangs, attach GDB
// 2. Find which thread holds the lock:

(gdb) info threads
// Thread 1: waiting in mutex.lock()
// Thread 2: in procesData() after return

// 3. Check mutex state (implementation-dependent):
(gdb) print mutex.d
// Shows owner thread ID

// SOLUTION: Always use RAII
void processData() {
    QMutexLocker locker(&mutex);  // Automatic unlock
    
    if (data.isEmpty()) {
        return;  // locker destructor unlocks
    }
    
    processDataInternal();  // Exception safe
}
```

### Problem 4: Accessing Deleted Object

**Symptom:** Crashes with "invalid pointer" or segmentation fault

```cpp
// BUG: Object deleted while thread still using it
QThread *thread = new QThread;
Worker *worker = new Worker;
worker->moveToThread(thread);

connect(thread, &QThread::finished, thread, &QThread::deleteLater);
connect(thread, &QThread::finished, worker, &QObject::deleteLater);

thread->start();
// ... later ...
thread->quit();
// BUG: worker might still be processing when deleted!

// Debugging approach:
// 1. Enable Qt object tracking:
qputenv("QT_FATAL_WARNINGS", "1");

// 2. Crash occurs, check backtrace:
(gdb) bt
#0  Worker::process() at worker.cpp:42
#1  QMetaObject::activate() 
// Shows worker deleted during signal emission

// 3. Use AddressSanitizer:
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..

// ASan output:
// ERROR: AddressSanitizer: heap-use-after-free
// WRITE of size 8 at 0x60300000eff0 thread T1
//   #0 Worker::process() worker.cpp:42

// SOLUTION: Wait for thread to finish
thread->quit();
thread->wait();  // Wait for thread to actually finish
delete worker;
delete thread;

// Or use stronger connection:
connect(worker, &Worker::finished, thread, &QThread::quit);
connect(thread, &QThread::finished, [worker, thread]() {
    worker->deleteLater();
    thread->deleteLater();
});
```

### Problem 5: GUI Access from Worker Thread

**Symptom:** Random crashes, "Cannot send events to objects owned by different thread"

```cpp
// BUG: Updating GUI from worker thread
class BadWorker : public QObject {
    Q_OBJECT
    QLabel *label;  // Owned by main thread
    
public slots:
    void process() {
        // This runs in worker thread!
        label->setText("Done");  // CRASH! GUI access from wrong thread
    }
};

// Debugging approach:
// 1. Qt prints warning:
// QObject::setProperty: Cannot send events to objects owned by different thread

// 2. Enable fatal warnings:
qputenv("QT_FATAL_WARNINGS", "1");

// 3. Check thread affinity in debugger:
(gdb) print label->thread()
(gdb) print QThread::currentThread()
// Different addresses = wrong thread!

// 4. Add assertion in debug builds:
void setText(const QString &text) {
    Q_ASSERT(QThread::currentThread() == this->thread());
    label->setText(text);
}

// SOLUTION: Use signals
class GoodWorker : public QObject {
    Q_OBJECT
    
signals:
    void resultReady(const QString &text);
    
public slots:
    void process() {
        // Work in worker thread
        emit resultReady("Done");  // Signal crosses thread boundary safely
    }
};

// In main thread:
connect(worker, &GoodWorker::resultReady, 
        label, &QLabel::setText);  // Runs in label's thread (main)
```

### Problem 6: Race Condition in Lazy Initialization

**Symptom:** Occasional crashes, double initialization, memory leaks

```cpp
// BUG: Non-thread-safe singleton
class BuggyDatabase {
    static BuggyDatabase *instance;
    
public:
    static BuggyDatabase* getInstance() {
        if (!instance) {  // Race condition!
            instance = new BuggyDatabase;  // Multiple threads may enter
        }
        return instance;
    }
};

BuggyDatabase* BuggyDatabase::instance = nullptr;

// Debugging approach:
// 1. Add logging:
static BuggyDatabase* getInstance() {
    qDebug() << "Thread" << QThread::currentThreadId() << "checking instance";
    if (!instance) {
        qDebug() << "Thread" << QThread::currentThreadId() << "creating instance";
        instance = new BuggyDatabase;
        qDebug() << "Thread" << QThread::currentThreadId() 
                 << "created" << (void*)instance;
    }
    return instance;
}

// Output shows:
// Thread 0x123 checking instance
// Thread 0x456 checking instance
// Thread 0x123 creating instance
// Thread 0x456 creating instance  <-- Both threads create!
// Thread 0x123 created 0xdeadbeef
// Thread 0x456 created 0xbaadf00d  <-- Memory leak!

// 2. Run with TSan - detects race on instance variable

// SOLUTION: Thread-safe initialization
Q_GLOBAL_STATIC(Database, database)  // Qt provides this

// Or manual double-checked locking:
class SafeDatabase {
    static QAtomicPointer<SafeDatabase> instance;
    static QMutex mutex;
    
public:
    static SafeDatabase* getInstance() {
        SafeDatabase *tmp = instance.loadAcquire();
        if (!tmp) {
            QMutexLocker locker(&mutex);
            tmp = instance.loadAcquire();
            if (!tmp) {
                tmp = new SafeDatabase;
                instance.storeRelease(tmp);
            }
        }
        return tmp;
    }
};
```

### Problem 7: Lost Wakeup

**Symptom:** Thread waits forever even though condition is satisfied

```cpp
// BUG: Checking condition without holding mutex
QMutex mutex;
QWaitCondition condition;
bool dataReady = false;

void producer() {
    dataReady = true;  // BUG: Set without lock
    condition.wakeOne();
}

void consumer() {
    mutex.lock();
    if (!dataReady) {  // Check before wait
        mutex.unlock();
        condition.wait(&mutex);  // BUG: May miss wakeup!
    }
    mutex.unlock();
}

// Debugging approach:
// 1. Consumer hangs even though data is ready
// 2. Add detailed logging:

void producer() {
    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") 
             << "Setting dataReady";
    dataReady = true;
    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") 
             << "Waking consumer";
    condition.wakeOne();
}

void consumer() {
    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") 
             << "Locking mutex";
    mutex.lock();
    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") 
             << "Checking dataReady:" << dataReady;
    if (!dataReady) {
        qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz") 
                 << "Waiting";
        mutex.unlock();
        condition.wait(&mutex);
    }
    mutex.unlock();
}

// Output reveals timing:
// 10:30:45.123 Checking dataReady: false
// 10:30:45.124 Setting dataReady
// 10:30:45.125 Waking consumer  <-- Wakeup sent
// 10:30:45.126 Waiting  <-- Started waiting AFTER wakeup - lost!

// SOLUTION: Always check condition in while loop with mutex held
void consumer() {
    QMutexLocker locker(&mutex);
    while (!dataReady) {  // while, not if!
        condition.wait(&mutex);  // Atomically unlocks and waits
    }
    // Process data
}

void producer() {
    QMutexLocker locker(&mutex);
    dataReady = true;
    condition.wakeOne();
}
```

### Problem 8: Priority Inversion

**Symptom:** High-priority thread blocked by low-priority thread

```cpp
// BUG: Low priority thread holds lock needed by high priority thread
QMutex sharedMutex;

void lowPriorityWork() {
    sharedMutex.lock();
    // Long operation - 10 seconds
    QThread::sleep(10);
    sharedMutex.unlock();
}

void highPriorityWork() {
    // Urgent task blocked by low priority thread!
    sharedMutex.lock();
    // Critical work
    sharedMutex.unlock();
}

// Debugging approach:
// 1. Use perf to see thread states:
perf sched record -p $(pidof myapp)
perf sched latency

// Shows high priority thread waiting excessively

// 2. In Qt Creator debugger, check thread priorities:
// Low priority: QThread::LowPriority
// High priority: QThread::HighPriority
// High is waiting for Low!

// SOLUTION: Minimize lock hold time
void lowPriorityWork() {
    QByteArray data;
    {
        QMutexLocker locker(&sharedMutex);
        data = readSharedData();  // Quick
    }
    processData(data);  // Long operation outside lock
}

// Or use priority inheritance (OS-specific)
// Or redesign to avoid shared locks between priorities
```

### Problem 9: ABA Problem in Lock-Free Code

**Symptom:** Corruption in lock-free data structures

```cpp
// BUG: Lock-free stack with ABA problem
template<typename T>
class BuggyLockFreeStack {
    struct Node {
        T data;
        Node *next;
    };
    
    QAtomicPointer<Node> head{nullptr};
    
public:
    void push(const T &value) {
        Node *node = new Node{value, nullptr};
        Node *oldHead = head.loadAcquire();
        do {
            node->next = oldHead;
        } while (!head.testAndSetRelease(oldHead, node));
    }
    
    bool pop(T &result) {
        Node *oldHead = head.loadAcquire();
        while (oldHead) {
            Node *next = oldHead->next;
            
            // BUG: ABA problem here
            // 1. Thread A: reads head (A)
            // 2. Thread B: pops A, pops B, pushes A back
            // 3. Thread A: CAS succeeds (head is A again!)
            //    But A->next is now different!
            
            if (head.testAndSetRelease(oldHead, next)) {
                result = oldHead->data;
                delete oldHead;  // May delete wrong node!
                return true;
            }
            oldHead = head.loadAcquire();
        }
        return false;
    }
};

// Debugging approach:
// 1. Very hard to reproduce - requires specific timing
// 2. Use stress test that rapidly pushes/pops from many threads
// 3. Enable ASan to catch use-after-free:

// ASan output:
// ERROR: AddressSanitizer: heap-use-after-free
// READ of size 8 at 0x60700000eff8 thread T3
//   #0 BuggyLockFreeStack::pop() stack.h:28

// 4. Add versioning to detect ABA:
struct VersionedPointer {
    Node *ptr;
    uint64_t version;
};

// Use 128-bit CAS or hazard pointers

// SOLUTION: Use tagged pointers or hazard pointers
template<typename T>
class FixedLockFreeStack {
    struct Node {
        T data;
        QAtomicPointer<Node> next;
    };
    
    struct TaggedPointer {
        Node *ptr;
        uintptr_t tag;  // ABA prevention
    };
    
    std::atomic<TaggedPointer> head{{nullptr, 0}};
    
public:
    void push(const T &value) {
        Node *node = new Node{value, {nullptr}};
        TaggedPointer oldHead = head.load(std::memory_order_acquire);
        TaggedPointer newHead;
        
        do {
            node->next.store(oldHead.ptr, std::memory_order_release);
            newHead = {node, oldHead.tag + 1};  // Increment tag
        } while (!head.compare_exchange_weak(oldHead, newHead,
                    std::memory_order_release,
                    std::memory_order_acquire));
    }
};
```

### Problem 10: False Sharing

**Symptom:** Poor scalability, high cache misses despite no lock contention

```cpp
// BUG: Adjacent data accessed by different threads
struct BuggyCounters {
    int counterA;  // Thread A increments
    int counterB;  // Thread B increments  
    // Both on same cache line (typically 64 bytes)
    // Causes cache line ping-pong between CPUs!
};

BuggyCounters counters;

void threadA() {
    for (int i = 0; i < 1000000; i++) {
        counters.counterA++;  // Invalidates cache line for thread B
    }
}

void threadB() {
    for (int i = 0; i < 1000000; i++) {
        counters.counterB++;  // Invalidates cache line for thread A
    }
}

// Debugging approach:
// 1. Performance is poor despite no locks
// 2. Use perf to check cache misses:

perf stat -e cache-misses,cache-references ./myapp

// Output:
// 45,234,567 cache-misses    # 45% miss rate!
//  100,123,456 cache-references

// 3. Use perf c2c (cache-to-cache) to find false sharing:
perf c2c record ./myapp
perf c2c report

// Shows counterA and counterB on same cache line accessed by different CPUs

// 4. Examine memory layout:
(gdb) print &counters.counterA
$1 = (int *) 0x555555556000
(gdb) print &counters.counterB  
$2 = (int *) 0x555555556004  // Only 4 bytes apart - same 64-byte cache line!

// SOLUTION: Align to cache line boundaries
struct FixedCounters {
    alignas(64) int counterA;
    char padding1[64 - sizeof(int)];
    alignas(64) int counterB;
    char padding2[64 - sizeof(int)];
};

// Or use Qt's approach:
struct FixedCounters {
    QAtomicInt counterA;  // Already optimized for cache line
    char padding[64 - sizeof(QAtomicInt)];
    QAtomicInt counterB;
};

// After fix, perf shows:
// 1,234,567 cache-misses    # 1% miss rate - 45x improvement!
```

### Problem 11: Starvation

**Symptom:** Some threads never get to run

```cpp
// BUG: Writers starve readers in unfair read-write lock
QReadWriteLock rwlock;

void continuousWriter() {
    while (true) {
        QWriteLocker locker(&rwlock);
        writeData();
        // Immediately reacquires write lock
    }
}

void reader() {
    QReadLocker locker(&rwlock);  // May never acquire!
    readData();
}

// Debugging approach:
// 1. Add timing metrics:
void reader() {
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Reader waiting for lock...";
    QReadLocker locker(&rwlock);
    qDebug() << "Reader acquired lock after" << timer.elapsed() << "ms";
    
    readData();
}

// Output shows:
// Reader waiting for lock...
// Reader acquired lock after 45234 ms  // 45 seconds!

// 2. Monitor thread states with systemtap:
stap -e 'probe scheduler.cpu_on { 
    if (pid() == target() && execname() == "myapp")
        printf("%s %d %s\n", ctime(gettimeofday_s()), tid(), 
               task_state(task_current()))
}'

// Shows reader thread in TASK_UNINTERRUPTIBLE for extended periods

// SOLUTION: Use fair locking or add yield
void continuousWriter() {
    while (true) {
        {
            QWriteLocker locker(&rwlock);
            writeData();
        }  // Release lock
        QThread::yieldCurrentThread();  // Give others a chance
    }
}

// Or use QReadWriteLock::RecursionMode::Recursive
// Or implement writer throttling
```

### Problem 12: Thread Leak

**Symptom:** Increasing number of threads, memory growth

```cpp
// BUG: Creating threads without cleanup
void processRequest() {
    QThread *thread = new QThread;
    Worker *worker = new Worker;
    worker->moveToThread(thread);
    thread->start();
    // BUG: Never deleted! Thread keeps running
}

// Debugging approach:
// 1. Check thread count:
ps -eLf | grep myapp | wc -l

// Shows growing number of threads over time

// 2. Use valgrind to detect memory leak:
valgrind --leak-check=full ./myapp

// Shows:
// definitely lost: 24,576 bytes in 6 blocks (QThread objects)

// 3. Monitor with custom tracking:
class TrackedThread : public QThread {
    static QAtomicInt threadCount;
    
public:
    TrackedThread() {
        int count = threadCount.fetchAndAddAcquire(1);
        qDebug() << "Thread created, total:" << count + 1;
    }
    
    ~TrackedThread() {
        int count = threadCount.fetchAndSubAcquire(1);
        qDebug() << "Thread destroyed, total:" << count - 1;
    }
};

// Output shows thread count only increasing, never decreasing

// SOLUTION: Ensure cleanup
void processRequest() {
    QThread *thread = new QThread;
    Worker *worker = new Worker;
    worker->moveToThread(thread);
    
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    
    thread->start();
}

// Or use QThreadPool for automatic management
```

### Problem 13-30: Quick Reference

**Problem 13: Livelock** - Threads keep responding to each other without progress
- **Debug:** Add state logging, look for oscillating states
- **Tool:** perf record -g to see spinning in user space
- **Fix:** Add randomized backoff or yield

**Problem 14: Signal/Slot Connection Type Confusion**
- **Symptom:** Deadlock when using BlockingQueuedConnection in same thread
- **Debug:** Check connection type and thread affinity
- **Fix:** Use Qt::AutoConnection or explicit queued

**Problem 15: QObject Parent Thread Mismatch**
- **Symptom:** "Cannot create children for a parent in different thread"
- **Debug:** Print parent->thread() vs QThread::currentThread()
- **Fix:** Create child objects in correct thread

**Problem 16: Race in QSharedPointer**
- **Symptom:** Occasional crashes with reference count
- **Debug:** Enable QT_SHARED_POINTER_BACKTRACE_SUPPORT
- **Fix:** Use atomic operations or proper locking

**Problem 17: Incorrect Memory Order**
- **Symptom:** Reordering causes wrong values on weakly-ordered CPUs
- **Debug:** Reproduce on ARM or use TSan
- **Fix:** Use acquire/release semantics correctly

**Problem 18: Missing Memory Barrier**
- **Symptom:** Works on x86, fails on ARM/PowerPC
- **Debug:** Test on different architectures
- **Fix:** Use QAtomicPointer or explicit barriers

**Problem 19: Recursive Lock Deadlock**
- **Symptom:** Deadlock when same thread locks non-recursive mutex twice
- **Debug:** Stack trace shows same function multiple times
- **Fix:** Use QRecursiveMutex or redesign call structure

**Problem 20: Thread Pool Exhaustion**
- **Symptom:** Tasks queue up, not executing
- **Debug:** QThreadPool::globalInstance()->activeThreadCount()
- **Fix:** Increase max threads or don't block in tasks

**Problem 21: Spurious Wakeup**
- **Symptom:** wait() returns but condition not met
- **Debug:** Log condition checks
- **Fix:** Always use while loop, not if

**Problem 22: Lock Convoy**
- **Symptom:** Threads wake up sequentially, poor throughput
- **Debug:** perf sched to see wake patterns
- **Fix:** Batch wakeups or redesign locking

**Problem 23: Read-Modify-Write Race**
- **Symptom:** Lost updates (counter doesn't reach expected value)
- **Debug:** TSan detects, or add logging
- **Fix:** Use QAtomicInt or protect with mutex

**Problem 24: Thread Affinity Violation**
- **Symptom:** Crash when posting events to object in terminated thread
- **Debug:** Check thread()->isFinished()
- **Fix:** Disconnect signals before quitting thread

**Problem 25: Exception in Thread**
- **Symptom:** Silent thread termination
- **Debug:** Wrap run() in try-catch, log exceptions
- **Fix:** Proper exception handling and error reporting

**Problem 26: Signal Queue Overflow**
- **Symptom:** Events not processed, application freezes
- **Debug:** Check event queue size
- **Fix:** Process events in batches or use direct connections

**Problem 27: Lock-Free Algorithm Bug**
- **Symptom:** Subtle corruption, hard to reproduce
- **Debug:** Stress test, model checker, formal verification
- **Fix:** Use proven algorithms or stick to locks

**Problem 28: Cache Line Bouncing**
- **Symptom:** Poor performance with correct synchronization
- **Debug:** perf c2c to see cache line transfers
- **Fix:** Pad structures, reduce sharing

**Problem 29: Memory Leak in Thread-Local**
- **Symptom:** QThreadStorage data never freed
- **Debug:** valgrind shows increasing allocation
- **Fix:** Manual cleanup or use thread destructors

**Problem 30: Busy-Wait Spin**
- **Symptom:** High CPU usage while "waiting"
- **Debug:** perf shows time in comparison loop
- **Fix:** Use proper wait primitives, not spin loops

## Comprehensive Debugging Strategy

### Step 1: Reproduce Reliably

```cpp
// Add stress testing
void stressTest() {
    QVector<QThread*> threads;
    
    for (int i = 0; i < 100; i++) {
        QThread *t = QThread::create([this]() {
            for (int j = 0; j < 10000; j++) {
                performOperation();
            }
        });
        t->start();
        threads.append(t);
    }
    
    for (QThread *t : threads) {
        t->wait();
        delete t;
    }
}
```

### Step 2: Enable All Sanitizers

```cmake
set(CMAKE_CXX_FLAGS_DEBUG 
    "-fsanitize=thread,undefined -g -O1 -fno-omit-frame-pointer")
```

### Step 3: Add Instrumentation

```cpp
#define THREAD_DEBUG qDebug() << QThread::currentThreadId() << __FUNCTION__ << __LINE__

class DebugMutex {
    QMutex mutex;
    QAtomicPointer<void> owner{nullptr};
    
public:
    void lock() {
        THREAD_DEBUG << "attempting lock";
        mutex.lock();
        owner.store(QThread::currentThreadId());
        THREAD_DEBUG << "acquired lock";
    }
    
    void unlock() {
        THREAD_DEBUG << "releasing lock";
        owner.store(nullptr);
        mutex.unlock();
    }
    
    void* getOwner() const { return owner.load(); }
};
```

### Step 4: Analyze with Multiple Tools

1. **TSan** - finds races
2. **Helgrind** - finds lock issues
3. **ASan** - finds memory corruption
4. **perf** - finds performance issues
5. **GDB** - examines deadlocks
6. **Valgrind callgrind** - profiles lock contention

### Step 5: Visualization

```python
# Generate thread timeline from logs
import matplotlib.pyplot as plt
import re

# Parse log file
events = []
with open('thread.log') as f:
    for line in f:
        m = re.match(r'(\d+\.\d+) Thread-(\d+) (\w+)', line)
        if m:
            time, thread, event = m.groups()
            events.append((float(time), int(thread), event))

# Plot timeline
fig, ax = plt.subplots()
for time, thread, event in events:
    color = 'g' if event == 'lock' else 'r'
    ax.scatter(time, thread, c=color)

plt.xlabel('Time (s)')
plt.ylabel('Thread ID')
plt.title('Thread Activity Timeline')
plt.show()
```

## Prevention Checklist

✅ **Always use RAII** (QMutexLocker, not manual lock/unlock)  
✅ **Acquire locks in consistent order** (prevent deadlock)  
✅ **Use QAtomicInt** for simple counters (not mutex + int)  
✅ **While loops** with QWaitCondition (not if)  
✅ **Set thread names** for easier debugging  
✅ **Connect cleanup signals** (finished → deleteLater)  
✅ **Test with sanitizers** regularly  
✅ **Profile** before optimizing (don't guess)  
✅ **Document** thread ownership and lock order  
✅ **Code review** all threading code  

## Emergency Debugging Toolkit

```bash
# Quick deadlock check
pstack $(pidof myapp) | grep -A 5 "mutex\|lock"

# See which threads are running
ps -eLf | grep myapp

# Check CPU usage per thread  
top -H -p $(pidof myapp)

# Attach debugger to hanging process
gdb -p $(pidof myapp) -ex "thread apply all bt" -ex quit

# Quick race detection
valgrind --tool=helgrind --log-file=/tmp/helgrind.log ./myapp &
# Let it run for a minute
kill %1
grep "data race" /tmp/helgrind.log
```

Threading bugs are hard, but with the right tools and techniques, they're findable and fixable. **Always test threading code under load with sanitizers enabled.**
