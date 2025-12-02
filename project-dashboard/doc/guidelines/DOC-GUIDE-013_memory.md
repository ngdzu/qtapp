---
doc_id: DOC-GUIDE-013
title: Memory and Resource Management
version: 1.0
status: Approved
created: 2025-12-01
updated: 2025-12-01
category: Guideline
tags: [memory, raii, smart-pointers, resource-management, performance, qt-ownership]
related:
  - DOC-ARCH-003
  - DOC-GUIDE-001
source: 23_MEMORY_RESOURCE_MANAGEMENT.md
---

# Memory and Resource Management

## Overview

This guideline defines memory management patterns, resource lifecycle management, and performance optimization strategies for the Z Monitor application. The system uses a combination of RAII, smart pointers, Qt parent-child ownership, and pre-allocation strategies to ensure safe, deterministic, and efficient resource management.

## Core Principles

1. **RAII (Resource Acquisition Is Initialization):** Resources are acquired in constructors and released in destructors
2. **Automatic Memory Management:** Prefer smart pointers and Qt parent-child ownership over manual `new`/`delete`
3. **Deterministic Cleanup:** Resources are released immediately when they go out of scope
4. **No Memory Leaks:** All resources must be properly cleaned up
5. **Performance:** Minimize allocations in hot paths (real-time threads)

## Smart Pointers (C++ Standard)

### std::unique_ptr (Exclusive Ownership)

Use `std::unique_ptr` for exclusive ownership (one owner, cannot be copied):

```cpp
// ✅ GOOD: Unique ownership
class NetworkManager {
private:
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    std::unique_ptr<QTimer> m_retryTimer;
    
public:
    NetworkManager() {
        m_networkManager = std::make_unique<QNetworkAccessManager>();
        m_retryTimer = std::make_unique<QTimer>();
        
        // Resources automatically deleted when NetworkManager is destroyed
    }
    
    // No need for destructor - unique_ptr handles cleanup
};
```

**When to Use:**
- One clear owner of the resource
- Non-QObject resources (files, network connections, buffers)
- Resources that should not be shared

### std::shared_ptr (Shared Ownership)

Use `std::shared_ptr` for shared ownership (multiple owners, reference counted):

```cpp
// ✅ GOOD: Shared ownership
class TelemetryBatch {
public:
    std::shared_ptr<VitalSignData> data;  // Shared across batches
};

class VitalSignCache {
private:
    std::map<QString, std::shared_ptr<VitalSignData>> m_cache;
    
public:
    std::shared_ptr<VitalSignData> get(const QString& patientMrn) {
        auto it = m_cache.find(patientMrn);
        if (it != m_cache.end()) {
            return it->second;  // Return shared pointer (ref count incremented)
        }
        return nullptr;
    }
    
    // Data is automatically deleted when last shared_ptr is destroyed
};
```

**When to Use:**
- Multiple owners need access to the same resource
- Resource lifetime is not clearly defined
- Caching scenarios where data is shared

### std::weak_ptr (Non-Owning Reference)

Use `std::weak_ptr` to break circular references:

```cpp
// ✅ GOOD: Break circular reference
class Patient;
class BedAssignment;

class Patient {
public:
    std::shared_ptr<BedAssignment> bed;  // Patient owns bed assignment
};

class BedAssignment {
public:
    std::weak_ptr<Patient> patient;  // Bed does NOT own patient (weak reference)
};

// Usage
auto patient = std::make_shared<Patient>();
auto bed = std::make_shared<BedAssignment>();
patient->bed = bed;
bed->patient = patient;  // Weak reference breaks cycle

// When patient is destroyed, bed can detect it:
if (auto patientPtr = bed->patient.lock()) {
    // Patient still exists
} else {
    // Patient has been deleted
}
```

**When to Use:**
- Break circular references
- Observer pattern (observers don't own subject)
- Cache entries that can be evicted

## Qt Parent-Child Ownership (QObject)

### Automatic Cleanup with Parent-Child

Qt automatically deletes child QObjects when parent is destroyed:

```cpp
// ✅ GOOD: Qt parent-child ownership
class DashboardController : public QObject {
    Q_OBJECT
    
public:
    DashboardController(QObject* parent = nullptr)
        : QObject(parent) {
        
        // Children are parented to 'this'
        m_patientModel = new PatientListModel(this);  // ← Parent is 'this'
        m_alarmModel = new AlarmListModel(this);      // ← Parent is 'this'
        
        // Children are automatically deleted when DashboardController is destroyed
    }
    
    // No need for destructor - Qt handles cleanup
    
private:
    PatientListModel* m_patientModel;  // Raw pointer OK (Qt-managed)
    AlarmListModel* m_alarmModel;
};
```

**When to Use:**
- QObject-derived classes
- UI components (QML controllers, models)
- Qt event loop objects

**Important Rules:**
- **Always pass parent in constructor:** `new MyObject(this)`
- **Never delete children manually:** Qt handles deletion
- **Use raw pointers for Qt-managed objects:** `MyObject* m_object;`

### Qt Parent-Child Ownership Rules

1. **Parent deletes children:** When parent is destroyed, all children are deleted
2. **Children are deleted in reverse order:** Last child added is deleted first
3. **Use `deleteLater()` for event loop objects:** Safe deletion after event processing
4. **Don't delete children manually:** Let Qt handle it

```cpp
// ✅ GOOD: Let Qt handle deletion
class MyService : public QObject {
public:
    MyService(QObject* parent = nullptr) : QObject(parent) {
        m_timer = new QTimer(this);  // ← Parented to 'this'
        // Timer is automatically deleted when MyService is destroyed
    }
    
private:
    QTimer* m_timer;  // Raw pointer OK (Qt-managed)
};

// ❌ BAD: Manual deletion
class MyService : public QObject {
public:
    ~MyService() {
        delete m_timer;  // ❌ Unnecessary! Qt already deletes children
    }
    
private:
    QTimer* m_timer;
};
```

## RAII for Resource Management

### File Handles

```cpp
// ✅ GOOD: RAII with QFile (automatic cleanup)
class ConfigManager {
public:
    Result<Config, Error> loadConfig(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return Error(ErrorCode::FileOpenFailed, "Cannot open config file");
        }
        
        QByteArray data = file.readAll();
        // File is automatically closed when QFile goes out of scope
        
        return parseConfig(data);
    }
};
```

### Network Connections

```cpp
// ✅ GOOD: RAII for network connections
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

### Database Connections

```cpp
// ✅ GOOD: RAII for database connections
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

## Pre-Allocation Strategies

### Real-Time Buffers

Pre-allocate all real-time buffers at startup to avoid heap allocations during processing:

```cpp
// ✅ GOOD: Pre-allocated buffers (no heap allocation in hot path)
class SignalProcessor {
private:
    static constexpr size_t BUFFER_SIZE = 1024;
    std::array<float, BUFFER_SIZE> m_sampleBuffer;   // Pre-allocated on stack/class
    std::array<float, BUFFER_SIZE> m_filterBuffer;   // Pre-allocated on stack/class
    
public:
    SignalProcessor() {
        // Buffers are allocated on stack or as class members
        // No heap allocation during processing
    }
    
    void processSample(float sample) {
        // Use pre-allocated buffers (no allocation)
        m_sampleBuffer[m_writeIndex] = sample;
        // Process...
    }
};

// ❌ BAD: Heap allocation in hot path
class SignalProcessor {
public:
    void processSample(float sample) {
        std::vector<float> buffer(1024);  // ❌ Heap allocation every call!
        buffer[0] = sample;
        // Process...
    }
};
```

### Container Capacity Reservation

Reserve capacity for containers to avoid repeated reallocations:

```cpp
// ✅ GOOD: Reserve capacity
void loadPatients(const QList<QString>& mrnList) {
    QList<Patient> patients;
    patients.reserve(mrnList.size());  // Pre-allocate for N items
    
    for (const QString& mrn : mrnList) {
        patients.append(loadPatient(mrn));  // No reallocation
    }
}

// ❌ BAD: No capacity reservation
void loadPatients(const QList<QString>& mrnList) {
    QList<Patient> patients;  // Starts with small capacity
    
    for (const QString& mrn : mrnList) {
        patients.append(loadPatient(mrn));  // May cause repeated reallocations
    }
}
```

### Object Pools (Future Implementation)

**Note:** Object pools are **not yet implemented** but are planned for high-frequency allocations.

**Example Pattern:**
```cpp
// Future implementation: Object pool for telemetry batches
template<typename T, size_t PoolSize>
class ObjectPool {
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

// Usage (future)
ObjectPool<TelemetryBatch, 100> g_telemetryPool;

TelemetryBatch* batch = g_telemetryPool.allocate();  // Reuse from pool
// ... use batch ...
g_telemetryPool.deallocate(batch);  // Return to pool
```

## Memory Allocation Rules by Thread

### Real-Time Threads (Sensor I/O, Signal Processor)

**Hot Path Rules:**
- ❌ **NO heap allocations** in inner loops
- ❌ **NO dynamic container growth** (QList, QVector)
- ✅ **Use pre-allocated buffers**
- ✅ **Use fixed-size arrays**
- ✅ **Use object pools** for temporary objects (future)

```cpp
// ✅ GOOD: No allocations in real-time thread
class SensorReader {
private:
    std::array<float, 256> m_buffer;  // Pre-allocated
    
public:
    void readSamples() {
        // Read directly into pre-allocated buffer (no allocation)
        m_sensorInterface->read(m_buffer.data(), m_buffer.size());
    }
};
```

### Background Threads (Database, Network, Logger)

**Background Thread Rules:**
- ✅ Heap allocation allowed (but minimize)
- ✅ Use smart pointers
- ✅ Batch allocations when possible
- ✅ Reuse buffers across operations

```cpp
// ✅ GOOD: Batch allocations in background thread
class DatabaseWriter {
public:
    void writeBatch(const QList<Record>& records) {
        QSqlQuery query;
        query.prepare("INSERT INTO records VALUES (?, ?)");
        
        // Batch insert (minimize allocations)
        for (const Record& record : records) {
            query.addBindValue(record.id);
            query.addBindValue(record.data);
            query.exec();
        }
    }
};
```

### UI Thread

**UI Thread Rules:**
- ✅ Standard Qt memory management (parent-child)
- ✅ Use QML property bindings (automatic memory management)
- ✅ Avoid large allocations that block UI
- ✅ Use async operations for heavy work

```cpp
// ✅ GOOD: Async operation in UI thread
class DashboardController : public QObject {
    Q_OBJECT
    
public:
    Q_INVOKABLE void loadPatients() {
        // Start async operation (non-blocking)
        QtConcurrent::run([this]() {
            auto patients = m_repository->loadAll();  // Heavy work in background
            QMetaObject::invokeMethod(this, [this, patients]() {
                m_patientModel->setPatients(patients);  // Update UI on UI thread
            }, Qt::QueuedConnection);
        });
    }
};
```

## Resource Cleanup Best Practices

### Destructor Guidelines

Always clean up resources in destructors using RAII:

```cpp
// ✅ GOOD: Comprehensive cleanup
class NetworkManager {
private:
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    QList<QNetworkReply*> m_pendingReplies;
    
public:
    ~NetworkManager() {
        // Cancel all pending requests
        for (QNetworkReply* reply : m_pendingReplies) {
            reply->abort();
            reply->deleteLater();
        }
        m_pendingReplies.clear();
        
        // Network manager is automatically deleted by unique_ptr
    }
};
```

### Explicit Cleanup Methods

Provide explicit cleanup methods for resources that should be released before destruction:

```cpp
// ✅ GOOD: Explicit cleanup method
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
        closeDatabase();  // Ensure cleanup even if not called explicitly
    }
};
```

## Memory Leak Prevention

### Common Leak Sources

1. **Forgotten deletes:** Use smart pointers or parent-child
2. **Circular references:** Break cycles with `std::weak_ptr`
3. **Unclosed resources:** Use RAII
4. **Event loop objects:** Use `deleteLater()` for QObject cleanup

### Leak Detection Tools

- **Valgrind:** `valgrind --leak-check=full ./z-monitor`
- **AddressSanitizer:** Compile with `-fsanitize=address`
- **Qt Creator Memory Analyzer:** Built-in leak detection

### Best Practices Checklist

- ✅ Use smart pointers for dynamic memory
- ✅ Use Qt parent-child for QObject
- ✅ Use RAII for all resources (files, network, database)
- ✅ Test with leak detection tools
- ✅ Review code for resource cleanup
- ✅ Use `deleteLater()` for QObject in event loop
- ✅ Break circular references with `std::weak_ptr`

## Performance Optimization

### Minimize Allocations

```cpp
// ✅ GOOD: Minimize allocations
void processVitals(const QList<VitalSign>& vitals) {
    QList<float> filtered;
    filtered.reserve(vitals.size());  // Pre-allocate
    
    for (const VitalSign& vital : vitals) {
        if (vital.isValid()) {
            filtered.append(vital.value);  // No reallocation
        }
    }
}

// ❌ BAD: Repeated allocations
void processVitals(const QList<VitalSign>& vitals) {
    QList<float> filtered;  // Starts small
    
    for (const VitalSign& vital : vitals) {
        if (vital.isValid()) {
            filtered.append(vital.value);  // May cause repeated reallocations
        }
    }
}
```

### Reuse Buffers

```cpp
// ✅ GOOD: Reuse buffer across operations
class DataProcessor {
private:
    QByteArray m_workBuffer;  // Reused buffer
    
public:
    void processData(const QByteArray& input) {
        m_workBuffer.clear();  // Clear but don't deallocate
        m_workBuffer.reserve(input.size());  // Reuse capacity
        
        // Process into work buffer
        for (char c : input) {
            m_workBuffer.append(transform(c));
        }
    }
};
```

### Stack Allocation

```cpp
// ✅ GOOD: Stack allocation for small buffers
void processSmallData() {
    char buffer[256];  // Stack allocation (fast)
    // Process...
}

// ❌ BAD: Heap allocation for small buffers
void processSmallData() {
    char* buffer = new char[256];  // Heap allocation (slower)
    // Process...
    delete[] buffer;
}
```

## Related Documents

<!-- TODO: Link to thread model doc (DOC-ARCH-003) when created -->
- [DOC-GUIDE-001: Code Organization](./DOC-GUIDE-001_code_organization.md) - Resource ownership by layer

## Enforcement

Memory management is enforced through:
- **Code Review:** All PRs must use RAII and smart pointers
- **Static Analysis:** Clang-tidy checks for memory leaks
- **Runtime Testing:** Valgrind and AddressSanitizer in CI/CD
- **Unit Tests:** Test resource cleanup in destructors
