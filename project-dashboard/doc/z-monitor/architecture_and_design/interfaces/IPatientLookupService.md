# IPatientLookupService Interface

**Interface ID:** IFACE-001  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

## 1. Overview

The `IPatientLookupService` interface defines the contract for retrieving patient demographic information from external systems (HIS/EHR) by Medical Record Number (MRN).

**Purpose:**
- Abstract HIS/EHR integration details
- Enable testing with mock implementations
- Support multiple HIS vendors (Epic, Cerner, etc.)

**Related Documents:**
- **Requirements:** [REQ-FUN-PAT-010](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md), [REQ-INT-HIS-001](../../requirements/06_INTERFACE_REQUIREMENTS.md)
- **Use Cases:** [UC-PM-004](../../requirements/02_USE_CASES.md)
- **Class Design:** [09_CLASS_DESIGNS.md](../09_CLASS_DESIGNS.md)

---

## 2. Interface Definition

### 2.1 C++ Header

```cpp
/**
 * @interface IPatientLookupService
 * @brief Interface for patient demographic lookup from HIS/EHR.
 * 
 * This interface abstracts the patient lookup mechanism, allowing
 * multiple implementations (production HIS, mock, cache).
 * 
 * @note All methods are asynchronous (return QFuture or use signals)
 * @see PatientAggregate, AdmissionService
 * @ingroup Infrastructure
 */
class IPatientLookupService : public QObject {
    Q_OBJECT

public:
    virtual ~IPatientLookupService() = default;

    /**
     * @brief Lookup patient by Medical Record Number (MRN).
     * 
     * Queries the HIS/EHR system for patient demographics. If network
     * is unavailable, implementations should check local cache.
     * 
     * @param mrn Patient Medical Record Number (format: MRN-XXXXX)
     * @return QFuture<Result<PatientIdentity>> Future resolving to patient data or error
     * 
     * @note Asynchronous operation (5-second timeout typical)
     * @see PatientIdentity, Result
     */
    virtual QFuture<Result<PatientIdentity>> lookupPatient(const QString& mrn) = 0;

    /**
     * @brief Check if patient lookup service is available.
     * 
     * Tests connectivity to HIS/EHR system.
     * 
     * @return QFuture<bool> Future resolving to true if HIS reachable
     * 
     * @note Quick check (1-second timeout)
     */
    virtual QFuture<bool> isAvailable() = 0;

    /**
     * @brief Get service metadata (HIS name, version, endpoint).
     * 
     * Returns information about the connected HIS/EHR system.
     * 
     * @return ServiceMetadata Metadata about the HIS service
     */
    virtual ServiceMetadata getServiceInfo() const = 0;

signals:
    /**
     * @brief Emitted when patient lookup completes.
     * 
     * @param mrn Patient MRN
     * @param result Success or error result
     */
    void lookupCompleted(const QString& mrn, const Result<PatientIdentity>& result);

    /**
     * @brief Emitted when lookup fails.
     * 
     * @param mrn Patient MRN
     * @param error Error details
     */
    void lookupFailed(const QString& mrn, const Error& error);

    /**
     * @brief Emitted when service availability changes.
     * 
     * @param available true if HIS is now reachable
     */
    void availabilityChanged(bool available);
};
```

---

## 3. Data Structures

### 3.1 PatientIdentity (Value Object)

```cpp
/**
 * @struct PatientIdentity
 * @brief Patient demographic information from HIS.
 * 
 * Immutable value object representing patient demographics.
 */
struct PatientIdentity {
    QString mrn;              ///< Medical Record Number (e.g., "MRN-12345")
    QString name;             ///< Full name (e.g., "John Doe")
    QDate dateOfBirth;        ///< Date of birth
    QString sex;              ///< Sex ("M", "F", "U" for unknown)
    QString bedLocation;      ///< Current bed location (e.g., "ICU-Bed-3")
    QStringList allergyFlags; ///< Allergies (e.g., ["Penicillin", "Latex"])
    QString codeStatus;       ///< Code status (e.g., "Full Code", "DNR")
    QDateTime lookupTime;     ///< When this data was retrieved
    LookupSource source;      ///< HIS, CACHE, or MANUAL

    /**
     * @brief Validate patient identity data.
     * @return true if all required fields populated
     */
    bool isValid() const {
        return !mrn.isEmpty() && !name.isEmpty() && dateOfBirth.isValid();
    }

    /**
     * @brief Check if data is stale (> 24 hours old).
     * @return true if lookup time > 24 hours ago
     */
    bool isStale() const {
        return lookupTime.secsTo(QDateTime::currentDateTime()) > 86400; // 24 hours
    }

    /**
     * @brief Get patient age in years.
     * @return Age calculated from date of birth
     */
    int age() const {
        return dateOfBirth.daysTo(QDate::currentDate()) / 365;
    }
};

/**
 * @enum LookupSource
 * @brief Source of patient information.
 */
enum class LookupSource {
    HIS,     ///< Retrieved from HIS/EHR (fresh)
    CACHE,   ///< Retrieved from local cache
    MANUAL   ///< Manually entered (emergency mode)
};
```

### 3.2 ServiceMetadata

```cpp
/**
 * @struct ServiceMetadata
 * @brief Metadata about HIS/EHR service.
 */
struct ServiceMetadata {
    QString serviceName;    ///< HIS name (e.g., "Epic", "Cerner")
    QString version;        ///< API version (e.g., "R4")
    QString endpoint;       ///< Base URL (e.g., "https://his.hospital.com/api")
    bool isAvailable;       ///< Current availability status
    QDateTime lastCheck;    ///< Last availability check time
};
```

### 3.3 Result<T> (Error Handling)

```cpp
/**
 * @class Result<T>
 * @brief Result monad for success/error handling.
 * 
 * Inspired by Rust's Result<T, E> and C++23 std::expected.
 */
template<typename T>
class Result {
public:
    /**
     * @brief Create successful result.
     */
    static Result<T> success(const T& value) {
        return Result(value, Error());
    }

    /**
     * @brief Create error result.
     */
    static Result<T> failure(const Error& error) {
        return Result(T(), error);
    }

    bool isSuccess() const { return !m_error.hasError(); }
    bool isFailure() const { return m_error.hasError(); }

    const T& value() const { return m_value; }
    const Error& error() const { return m_error; }

private:
    Result(const T& value, const Error& error)
        : m_value(value), m_error(error) {}

    T m_value;
    Error m_error;
};

/**
 * @struct Error
 * @brief Error information.
 */
struct Error {
    ErrorCode code;         ///< Error code
    QString message;        ///< Human-readable error message
    QVariantMap details;    ///< Additional error context

    bool hasError() const { return code != ErrorCode::None; }
};

/**
 * @enum ErrorCode
 * @brief Standard error codes for patient lookup.
 */
enum class ErrorCode {
    None,                   ///< No error
    InvalidMrn,             ///< MRN format invalid
    PatientNotFound,        ///< Patient not in HIS
    NetworkError,           ///< Network connection failed
    Timeout,                ///< Request timed out
    AuthenticationError,    ///< HIS authentication failed
    ServerError,            ///< HIS server error (500)
    UnknownError            ///< Unexpected error
};
```

---

## 4. Implementations

### 4.1 Production Implementation: HISPatientLookupAdapter

**Technology:** Qt Network (HTTPS/REST)

```cpp
/**
 * @class HISPatientLookupAdapter
 * @brief Production implementation for real HIS/EHR integration.
 * 
 * Communicates with hospital HIS via REST API.
 * 
 * @note Implements caching strategy (24-hour TTL)
 */
class HISPatientLookupAdapter : public IPatientLookupService {
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param baseUrl HIS API base URL (e.g., "https://his.hospital.com/api")
     * @param apiKey Authentication API key
     * @param cache Patient cache repository
     */
    HISPatientLookupAdapter(
        const QString& baseUrl,
        const QString& apiKey,
        IPatientRepository* cache,
        QObject* parent = nullptr
    );

    // IPatientLookupService interface
    QFuture<Result<PatientIdentity>> lookupPatient(const QString& mrn) override;
    QFuture<bool> isAvailable() override;
    ServiceMetadata getServiceInfo() const override;

private slots:
    void onNetworkReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    Result<PatientIdentity> parseLookupResponse(const QByteArray& json);
    Result<PatientIdentity> checkCache(const QString& mrn);
    void updateCache(const PatientIdentity& patient);

    QNetworkAccessManager* m_network;
    QString m_baseUrl;
    QString m_apiKey;
    IPatientRepository* m_cache;
    ServiceMetadata m_metadata;
};
```

### 4.2 Mock Implementation: MockPatientLookupService

**Technology:** In-memory map

```cpp
/**
 * @class MockPatientLookupService
 * @brief Mock implementation for testing and development.
 * 
 * Returns predefined patient data from in-memory map.
 * 
 * @note Useful for UI development, unit testing, demos
 */
class MockPatientLookupService : public IPatientLookupService {
    Q_OBJECT

public:
    MockPatientLookupService(QObject* parent = nullptr);

    /**
     * @brief Add mock patient to in-memory database.
     * @param patient Patient data to add
     */
    void addMockPatient(const PatientIdentity& patient);

    /**
     * @brief Simulate network delay.
     * @param delayMs Delay in milliseconds (default: 500ms)
     */
    void setSimulatedDelay(int delayMs) { m_delayMs = delayMs; }

    /**
     * @brief Simulate network failure.
     * @param shouldFail true to simulate failures
     */
    void setSimulateFailure(bool shouldFail) { m_simulateFailure = shouldFail; }

    // IPatientLookupService interface
    QFuture<Result<PatientIdentity>> lookupPatient(const QString& mrn) override;
    QFuture<bool> isAvailable() override;
    ServiceMetadata getServiceInfo() const override;

private:
    QMap<QString, PatientIdentity> m_patients;
    int m_delayMs = 500;
    bool m_simulateFailure = false;
};
```

---

## 5. Usage Examples

### 5.1 Basic Usage (with Signals)

```cpp
// Create HIS adapter
auto* lookupService = new HISPatientLookupAdapter(
    "https://his.hospital.com/api",
    "api-key-12345",
    patientRepository,
    this
);

// Connect signals
connect(lookupService, &IPatientLookupService::lookupCompleted,
        this, [](const QString& mrn, const Result<PatientIdentity>& result) {
    if (result.isSuccess()) {
        const PatientIdentity& patient = result.value();
        qDebug() << "Found patient:" << patient.name;
        qDebug() << "Age:" << patient.age() << "years";
    } else {
        qDebug() << "Lookup failed:" << result.error().message;
    }
});

// Perform lookup
auto future = lookupService->lookupPatient("MRN-12345");
```

### 5.2 Async/Await Style (Qt 6)

```cpp
// Perform lookup (returns immediately)
auto future = lookupService->lookupPatient("MRN-12345");

// Wait for result (in async context or use QFutureWatcher)
auto result = future.result();

if (result.isSuccess()) {
    const PatientIdentity& patient = result.value();
    
    // Validate patient
    if (!patient.isValid()) {
        qWarning() << "Invalid patient data";
        return;
    }
    
    // Check if data is stale
    if (patient.isStale()) {
        qWarning() << "Patient data is stale (> 24 hours old)";
    }
    
    // Use patient data
    displayPatient(patient);
} else {
    const Error& error = result.error();
    
    // Handle specific errors
    switch (error.code) {
        case ErrorCode::PatientNotFound:
            showError("Patient not found. Please verify MRN.");
            break;
        case ErrorCode::NetworkError:
            showError("Network unavailable. Using cached data.");
            // Try cache fallback
            tryCache("MRN-12345");
            break;
        case ErrorCode::Timeout:
            showError("HIS timeout. Retry?");
            break;
        default:
            showError("Lookup failed: " + error.message);
            break;
    }
}
```

### 5.3 Cache Fallback Strategy

```cpp
/**
 * @brief Lookup patient with automatic cache fallback.
 * 
 * Tries HIS first, falls back to cache if HIS unavailable.
 */
QFuture<Result<PatientIdentity>> lookupPatientWithFallback(
    const QString& mrn,
    IPatientLookupService* lookupService,
    IPatientRepository* cache
) {
    // Try HIS first
    auto future = lookupService->lookupPatient(mrn);
    
    return future.then([mrn, cache](const Result<PatientIdentity>& result) {
        if (result.isSuccess()) {
            return result; // HIS success
        }
        
        // HIS failed, try cache
        if (result.error().code == ErrorCode::NetworkError ||
            result.error().code == ErrorCode::Timeout) {
            
            qDebug() << "HIS unavailable, checking cache...";
            
            // Check cache
            auto cachedPatient = cache->findByMrn(mrn);
            if (cachedPatient.has_value()) {
                qDebug() << "Using cached patient data";
                return Result<PatientIdentity>::success(cachedPatient.value());
            }
        }
        
        // Both failed
        return result; // Return original HIS error
    });
}
```

---

## 6. Error Handling

### 6.1 Error Codes

| Error Code | HTTP Status | Description | Retry? | User Action |
|------------|------------|-------------|--------|-------------|
| `InvalidMrn` | 400 | MRN format invalid | No | Correct MRN format |
| `PatientNotFound` | 404 | Patient not in HIS | No | Verify MRN or manual entry |
| `NetworkError` | - | Cannot reach HIS | Yes | Wait for network, use cache |
| `Timeout` | 408 | Request timed out (> 5s) | Yes | Retry or use cache |
| `AuthenticationError` | 401 | API key invalid | No | Contact IT support |
| `ServerError` | 500 | HIS server error | Yes | Retry or contact IT |
| `UnknownError` | - | Unexpected error | Maybe | Check logs |

### 6.2 Error Handling Best Practices

```cpp
// DON'T: Ignore errors
auto future = lookupService->lookupPatient(mrn);
auto result = future.result();
displayPatient(result.value()); // CRASH if error!

// DO: Always check result
auto future = lookupService->lookupPatient(mrn);
auto result = future.result();

if (result.isSuccess()) {
    displayPatient(result.value());
} else {
    handleError(result.error());
}

// DO: Use pattern matching (if available)
auto result = future.result();
match(result,
    [](const PatientIdentity& patient) { displayPatient(patient); },
    [](const Error& error) { handleError(error); }
);
```

---

## 7. Testing

### 7.1 Unit Test Example

```cpp
TEST(IPatientLookupService, LookupSuccess) {
    // Arrange
    MockPatientLookupService service;
    service.addMockPatient(PatientIdentity{
        .mrn = "MRN-12345",
        .name = "John Doe",
        .dateOfBirth = QDate(1965, 3, 15),
        .sex = "M",
        .bedLocation = "ICU-Bed-3"
    });
    
    // Act
    auto future = service.lookupPatient("MRN-12345");
    auto result = future.result();
    
    // Assert
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().name, "John Doe");
    EXPECT_EQ(result.value().age(), 59); // Assuming current year 2024
}

TEST(IPatientLookupService, PatientNotFound) {
    // Arrange
    MockPatientLookupService service;
    
    // Act
    auto future = service.lookupPatient("MRN-99999");
    auto result = future.result();
    
    // Assert
    ASSERT_TRUE(result.isFailure());
    EXPECT_EQ(result.error().code, ErrorCode::PatientNotFound);
}

TEST(IPatientLookupService, NetworkTimeout) {
    // Arrange
    MockPatientLookupService service;
    service.setSimulateFailure(true);
    
    // Act
    auto future = service.lookupPatient("MRN-12345");
    auto result = future.result();
    
    // Assert
    ASSERT_TRUE(result.isFailure());
    EXPECT_EQ(result.error().code, ErrorCode::Timeout);
}
```

---

## 8. Integration with Dependency Injection

```cpp
/**
 * @brief Dependency injection setup.
 */
class ServiceContainer {
public:
    void configure() {
        // Register interface with implementation
        if (isProduction()) {
            registerSingleton<IPatientLookupService>([]() {
                return new HISPatientLookupAdapter(
                    Settings::instance()->hisBaseUrl(),
                    Settings::instance()->hisApiKey(),
                    container.resolve<IPatientRepository>()
                );
            });
        } else {
            registerSingleton<IPatientLookupService>([]() {
                return new MockPatientLookupService();
            });
        }
    }
};

// Usage in AdmissionService
class AdmissionService {
public:
    AdmissionService(IPatientLookupService* lookupService)
        : m_lookupService(lookupService) {}
        
    void admitPatient(const QString& mrn) {
        auto future = m_lookupService->lookupPatient(mrn);
        // ...
    }

private:
    IPatientLookupService* m_lookupService;
};
```

---

## 9. Performance Considerations

### 9.1 Caching Strategy

- **Cache Hit:** < 10ms (database query)
- **Cache Miss → HIS:** < 5 seconds (with timeout)
- **Cache TTL:** 24 hours
- **Cache Size:** ~100 patients (10 KB per patient = 1 MB total)

### 9.2 Optimization

```cpp
// DON'T: Sequential lookups
for (const QString& mrn : mrns) {
    auto result = lookupService->lookupPatient(mrn).result(); // SLOW!
}

// DO: Parallel lookups
QList<QFuture<Result<PatientIdentity>>> futures;
for (const QString& mrn : mrns) {
    futures.append(lookupService->lookupPatient(mrn));
}

// Wait for all
for (auto& future : futures) {
    auto result = future.result();
    // Process result
}
```

---

## 10. Security Considerations

- **API Key:** Store securely (not in source code)
- **HTTPS:** Always use HTTPS (never HTTP for production)
- **PHI:** Patient data is PHI - encrypt, log access, audit
- **Timeout:** 5-second timeout prevents hanging
- **Rate Limiting:** HIS may rate-limit (handle 429 responses)

---

## 11. Related Documents

- **Requirements:** [03_FUNCTIONAL_REQUIREMENTS.md](../../requirements/03_FUNCTIONAL_REQUIREMENTS.md) (REQ-FUN-PAT-010)
- **Requirements:** [06_INTERFACE_REQUIREMENTS.md](../../requirements/06_INTERFACE_REQUIREMENTS.md) (REQ-INT-HIS-001)
- **Use Cases:** [02_USE_CASES.md](../../requirements/02_USE_CASES.md) (UC-PM-004)
- **Data Flow:** [11_DATA_FLOW_AND_CACHING.md](../11_DATA_FLOW_AND_CACHING.md)
- **Security:** [06_SECURITY.md](../06_SECURITY.md)

---

*This interface abstracts HIS/EHR integration, enabling testability and supporting multiple hospital systems.*

