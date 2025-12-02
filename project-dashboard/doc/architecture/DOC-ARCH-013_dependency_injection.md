---
title: "Dependency Injection Architecture"
doc_id: DOC-ARCH-013
version: 1.0
category: Architecture
phase: 6D
status: Draft
created: 2025-12-01
author: migration-bot
related:
  - DOC-ARCH-001_system_architecture.md
  - DOC-ARCH-012_configuration_management.md
---

# Dependency Injection Architecture

This document evaluates dependency injection frameworks for Qt/C++ projects and recommends a lightweight manual dependency injection approach for the Z Monitor application.

**Key Principles:**
- **Constructor Injection:** All dependencies injected via constructor
- **No Singletons:** Avoid global state; dependencies passed explicitly
- **No Service Locator:** Service locator is an anti-pattern; use explicit DI
- **Qt Integration:** Respect Qt's QObject parent/child memory management
- **Testability:** All dependencies mockable for unit testing

## 1. DI Framework Evaluation

### 1.1 Option 1: Manual Dependency Injection

**Approach:** Manually wire dependencies via constructors and factory functions.

**Pros:**
- ✅ **Zero Library Dependency:** No external DI framework required
- ✅ **Explicit Dependencies:** All dependencies visible in constructors
- ✅ **Compile-Time Safety:** Type errors caught at compile time
- ✅ **Full Control:** No magic; clear data flow
- ✅ **Qt-Friendly:** Works naturally with Qt's QObject parent/child
- ✅ **Testability:** Easy to inject mocks for unit testing

**Cons:**
- ⚠️ **Boilerplate:** Requires manual wiring code
- ⚠️ **Large Dependency Trees:** Can become verbose with many dependencies

**Example:**

```cpp
// Domain interface
class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual Result<void, Error> save(const Patient& patient) = 0;
};

// Infrastructure implementation
class SQLiteDatabase : public IDatabase {
public:
    explicit SQLiteDatabase(const QString& dbPath) : dbPath_(dbPath) {}
    Result<void, Error> save(const Patient& patient) override { /* ... */ }
private:
    QString dbPath_;
};

// Application service with DI
class AdmissionService {
public:
    explicit AdmissionService(IDatabase* db, IAuditLogger* audit)
        : db_(db), audit_(audit) {}
    
    Result<void, Error> admitPatient(const PatientIdentity& identity) {
        Patient patient{identity};
        auto result = db_->save(patient);
        if (result.isOk()) {
            audit_->log("Patient admitted: " + identity.mrn);
        }
        return result;
    }

private:
    IDatabase* db_;       // Injected dependency (non-owning pointer)
    IAuditLogger* audit_; // Injected dependency
};

// Factory function for wiring
std::unique_ptr<AdmissionService> createAdmissionService() {
    auto db = std::make_unique<SQLiteDatabase>("/path/to/db.sqlite");
    auto audit = std::make_unique<FileAuditLogger>("/var/log/audit.log");
    return std::make_unique<AdmissionService>(db.get(), audit.get());
}
```

---

### 1.2 Option 2: Boost.DI

**Approach:** Compile-time dependency injection framework using C++14/17.

**Pros:**
- ✅ **Compile-Time Resolution:** Dependencies resolved at compile time (no runtime overhead)
- ✅ **Automatic Wiring:** No manual factory code
- ✅ **Type Safety:** Compile-time errors for missing dependencies
- ✅ **Header-Only:** No separate library build required

**Cons:**
- ⚠️ **Learning Curve:** Template-heavy API requires training
- ⚠️ **Compile-Time Overhead:** Can slow down compilation
- ⚠️ **Error Messages:** Template errors can be cryptic
- ⚠️ **Limited Community:** Smaller community than mainstream frameworks
- ⚠️ **Qt Integration:** Requires careful integration with Qt's QObject parent/child

**Example:**

```cpp
#include <boost/di.hpp>

namespace di = boost::di;

auto injector = di::make_injector(
    di::bind<IDatabase>.to<SQLiteDatabase>(),
    di::bind<IAuditLogger>.to<FileAuditLogger>()
);

auto service = injector.create<AdmissionService>();
```

**Verdict:** ⚠️ **Not Recommended** - Adds complexity without sufficient benefit for this project.

---

### 1.3 Option 3: Service Locator Pattern

**Approach:** Global registry that provides dependencies on demand.

**Example:**

```cpp
class ServiceLocator {
public:
    static IDatabase* getDatabase() { return db_; }
    static IAuditLogger* getAuditLogger() { return audit_; }
    
    static void registerDatabase(IDatabase* db) { db_ = db; }
    static void registerAuditLogger(IAuditLogger* audit) { audit_ = audit; }

private:
    static IDatabase* db_;
    static IAuditLogger* audit_;
};

class AdmissionService {
public:
    Result<void, Error> admitPatient(const PatientIdentity& identity) {
        auto db = ServiceLocator::getDatabase();
        auto audit = ServiceLocator::getAuditLogger();
        // ...
    }
};
```

**Cons:**
- ❌ **Hidden Dependencies:** Dependencies not visible in constructor
- ❌ **Global State:** Service locator is global mutable state
- ❌ **Testing Difficulty:** Hard to mock dependencies for unit tests
- ❌ **Runtime Errors:** Missing dependencies detected at runtime, not compile time
- ❌ **Anti-Pattern:** Considered an anti-pattern in modern C++

**Verdict:** ❌ **Strongly Discouraged** - Avoid service locator; use constructor injection.

---

### 1.4 Option 4: Qt's QObject Parent/Child System (Not DI)

**Approach:** Qt's automatic memory management for QObject hierarchies.

**Key Insight:** Qt's parent/child system is **not a dependency injection framework**. It manages **memory ownership**, not **dependency resolution**.

**Rules:**
- QObject children are deleted when parent is deleted
- Useful for UI components (widgets, QML objects)
- **Not suitable** for business logic dependencies (services, repositories)

**Recommendation:**
- Use Qt parent/child for **UI components only**
- Use **constructor injection** for business logic and infrastructure components

**Example:**

```cpp
// ✅ Correct: UI components use Qt parent/child
auto* mainWindow = new QMainWindow();
auto* centralWidget = new QWidget(mainWindow); // Parent = mainWindow
mainWindow->setCentralWidget(centralWidget);

// ✅ Correct: Services use constructor injection
class MonitoringService {
public:
    explicit MonitoringService(IVitalsRepository* repo) : repo_(repo) {}
private:
    IVitalsRepository* repo_; // Injected dependency
};
```

---

## 2. Recommended Approach

**Strategy:** **Manual Dependency Injection** with a lightweight application container for bootstrapping.

**Rationale:**
- Simple, explicit, and maintainable
- No external dependencies
- Full type safety at compile time
- Easy to understand for new developers
- Works naturally with Qt
- Supports testing via mock injection

---

## 3. Implementation Pattern

### 3.1 Application Container

Create a lightweight container to bootstrap and hold top-level services.

```cpp
class AppContainer {
public:
    AppContainer() {
        // Create infrastructure components
        db_ = std::make_unique<SQLiteDatabase>("/path/to/db.sqlite");
        audit_ = std::make_unique<FileAuditLogger>("/var/log/audit.log");
        settings_ = std::make_unique<SettingsManager>();
        
        // Create repositories (depend on db)
        patientRepo_ = std::make_unique<SQLitePatientRepository>(db_.get());
        vitalsRepo_ = std::make_unique<SQLiteVitalsRepository>(db_.get());
        
        // Create application services (depend on repositories)
        admissionService_ = std::make_unique<AdmissionService>(
            patientRepo_.get(),
            audit_.get()
        );
        monitoringService_ = std::make_unique<MonitoringService>(
            vitalsRepo_.get(),
            settings_.get()
        );
    }
    
    // Accessors
    AdmissionService* admissionService() { return admissionService_.get(); }
    MonitoringService* monitoringService() { return monitoringService_.get(); }
    
private:
    // Infrastructure
    std::unique_ptr<SQLiteDatabase> db_;
    std::unique_ptr<FileAuditLogger> audit_;
    std::unique_ptr<SettingsManager> settings_;
    
    // Repositories
    std::unique_ptr<IPatientRepository> patientRepo_;
    std::unique_ptr<IVitalsRepository> vitalsRepo_;
    
    // Application services
    std::unique_ptr<AdmissionService> admissionService_;
    std::unique_ptr<MonitoringService> monitoringService_;
};
```

### 3.2 Bootstrapping

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Bootstrap application container
    AppContainer container;
    
    // Create UI controllers and inject services
    auto* patientController = new PatientController(
        container.admissionService(),
        &app // Qt parent for memory management
    );
    
    auto* dashboardController = new DashboardController(
        container.monitoringService(),
        &app
    );
    
    // Load QML and expose controllers
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("patientController", patientController);
    engine.rootContext()->setContextProperty("dashboardController", dashboardController);
    engine.load(QUrl("qrc:/main.qml"));
    
    return app.exec();
}
```

### 3.3 Testing with Mocks

```cpp
// Mock database for testing
class MockDatabase : public IDatabase {
public:
    MOCK_METHOD(Result<void, Error>, save, (const Patient&), (override));
};

// Unit test
TEST(AdmissionServiceTest, AdmitPatient) {
    MockDatabase mockDb;
    MockAuditLogger mockAudit;
    
    EXPECT_CALL(mockDb, save(_)).WillOnce(Return(Result<void, Error>::ok()));
    EXPECT_CALL(mockAudit, log(_)).Times(1);
    
    AdmissionService service(&mockDb, &mockAudit);
    
    PatientIdentity identity{"12345", "John Doe", QDate(1980, 1, 1), Sex::Male};
    auto result = service.admitPatient(identity);
    
    ASSERT_TRUE(result.isOk());
}
```

---

## 4. Guidelines

### 4.1 Constructor Injection

**Rule:** All dependencies must be injected via constructor.

```cpp
// ✅ Correct: Dependencies injected via constructor
class MonitoringService {
public:
    explicit MonitoringService(IVitalsRepository* repo, ISettingsManager* settings)
        : repo_(repo), settings_(settings) {}
private:
    IVitalsRepository* repo_;
    ISettingsManager* settings_;
};

// ❌ Wrong: Dependencies obtained via global state
class MonitoringService {
public:
    MonitoringService() {
        repo_ = ServiceLocator::getVitalsRepository(); // Anti-pattern!
    }
private:
    IVitalsRepository* repo_;
};
```

### 4.2 Interface-Based Design

**Rule:** Depend on interfaces, not implementations.

```cpp
// ✅ Correct: Depend on interface
class AdmissionService {
public:
    explicit AdmissionService(IPatientRepository* repo) : repo_(repo) {}
private:
    IPatientRepository* repo_; // Interface
};

// ❌ Wrong: Depend on implementation
class AdmissionService {
public:
    explicit AdmissionService(SQLitePatientRepository* repo) : repo_(repo) {}
private:
    SQLitePatientRepository* repo_; // Concrete implementation
};
```

### 4.3 Ownership and Lifetimes

**Rule:** AppContainer owns services; services hold non-owning pointers to dependencies.

```cpp
class AppContainer {
public:
    AppContainer() {
        db_ = std::make_unique<SQLiteDatabase>("/path/to/db.sqlite");
        repo_ = std::make_unique<SQLitePatientRepository>(db_.get()); // Non-owning pointer
    }
private:
    std::unique_ptr<SQLiteDatabase> db_;          // Owner
    std::unique_ptr<IPatientRepository> repo_;    // Owner (repo_ holds non-owning db_.get())
};
```

**Lifetime Guarantee:** AppContainer ensures `db_` outlives `repo_` by declaration order.

### 4.4 Avoid Circular Dependencies

**Rule:** Services must form a directed acyclic graph (DAG). No circular dependencies allowed.

```cpp
// ❌ Wrong: Circular dependency
class ServiceA {
public:
    explicit ServiceA(ServiceB* b) : b_(b) {}
private:
    ServiceB* b_;
};

class ServiceB {
public:
    explicit ServiceB(ServiceA* a) : a_(a) {} // Circular!
private:
    ServiceA* a_;
};
```

**Solution:** Extract shared logic into a third service or use domain events for decoupling.

---

## 5. Qt Integration

### 5.1 QObject Controllers

QObject controllers depend on application services, not infrastructure.

```cpp
class DashboardController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString heartRate READ heartRate NOTIFY heartRateChanged)
    
public:
    explicit DashboardController(MonitoringService* service, QObject* parent = nullptr)
        : QObject(parent), service_(service) {
        connect(service_, &MonitoringService::vitalsUpdated,
                this, &DashboardController::onVitalsUpdated);
    }
    
    QString heartRate() const { return heartRate_; }
    
signals:
    void heartRateChanged();
    
private slots:
    void onVitalsUpdated(const VitalRecord& record) {
        heartRate_ = QString::number(record.heartRate);
        emit heartRateChanged();
    }
    
private:
    MonitoringService* service_; // Injected dependency
    QString heartRate_;
};
```

### 5.2 Memory Management Rules

- **Qt Parent/Child:** Use for QObject-based UI components
- **std::unique_ptr:** Use for non-QObject business logic (services, repositories)
- **Raw Pointers:** Use for injected dependencies (non-owning)

---

## 6. Related Documents

- **[DOC-ARCH-001: Architecture Overview](./DOC-ARCH-001_architecture_overview.md)** - High-level system architecture
- **[DOC-ARCH-012: Configuration Management](./DOC-ARCH-012_configuration_management.md)** - SettingsManager as injected dependency
- **[DOC-ARCH-016: System Components](./DOC-ARCH-016_system_components.md)** - Complete component inventory
- **[DOC-ARCH-017: Database Design](./DOC-ARCH-017_database_design.md)** - Repository pattern and database access

---
**Status:** ✅ Migrated from legacy 13_DEPENDENCY_INJECTION.md
