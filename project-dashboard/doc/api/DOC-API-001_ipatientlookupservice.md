---
doc_id: DOC-API-001
title: IPatientLookupService Interface
version: 1.0
category: API
subcategory: External Integration
status: Approved
created: 2025-11-27
updated: 2025-11-27
tags: [api, interface, his, ehr, patient-lookup, external-integration]
related_docs:
  - DOC-COMP-010 # AdmissionService
  - DOC-COMP-001 # PatientAggregate
  - DOC-COMP-014 # IPatientRepository
  - DOC-ARCH-003 # DDD Patterns
authors:
  - Z Monitor Team
reviewers:
  - Architecture Team
---

# IPatientLookupService Interface

## 1. Overview

The `IPatientLookupService` interface defines the contract for retrieving patient demographic information from external Hospital Information Systems (HIS) or Electronic Health Record (EHR) systems by Medical Record Number (MRN).

> **⚠️ Important:** All production code examples use variables for runtime data (e.g., `mrn` parameter). Never hardcode values like `"MRN-12345"` in production code. Test code may use hardcoded test data. See coding guidelines for "No Hardcoded Values" rules.

**Purpose:**
- Abstract HIS/EHR integration details from application logic
- Enable testing with mock implementations
- Support multiple HIS vendors (Epic, Cerner, Allscripts, etc.)
- Provide asynchronous patient lookup with error handling

**Key Characteristics:**
- **Asynchronous:** All methods return `QFuture<T>` for non-blocking operations
- **Error Handling:** Uses `Result<T, Error>` monad for explicit success/failure handling
- **Caching:** Supports local cache fallback when HIS unavailable
- **Vendor-Agnostic:** Interface abstraction allows switching HIS implementations

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

### 3.3 Result<T> and Error Types

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
    static Result<T> success(const T& value);
    static Result<T> failure(const Error& error);

    bool isSuccess() const;
    bool isFailure() const;

    const T& value() const;
    const Error& error() const;

private:
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

**Purpose:** Communicates with real hospital HIS via REST API with 24-hour cache TTL.

**Key Features:**
- HTTPS/REST communication with hospital HIS
- 24-hour cache TTL for offline support
- 5-second request timeout
- Automatic cache fallback on network errors

### 4.2 Mock Implementation: MockPatientLookupService

**Technology:** In-memory map

**Purpose:** Returns predefined patient data for development, testing, and demos.

**Key Features:**
- In-memory patient database
- Simulated network delay (configurable)
- Simulated failure modes for testing
- No external dependencies

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
        this, [this](const QString& mrn, const Result<PatientIdentity>& result) {
    if (result.isSuccess()) {
        const PatientIdentity& patient = result.value();
        m_logService->info("Patient lookup succeeded", {
            {"mrn", mrn},
            {"name", patient.name},
            {"age", QString::number(patient.age())}
        });
    } else {
        m_logService->warning("Patient lookup failed", {
            {"mrn", mrn},
            {"error", result.error().message}
        });
    }
});

// Perform lookup
QString mrn = getUserInputMrn();
auto future = lookupService->lookupPatient(mrn);
```

### 5.2 Async/Await Style (Qt 6)

```cpp
// Perform lookup (returns immediately)
QString mrn = getUserInputMrn();
auto future = lookupService->lookupPatient(mrn);

// Wait for result (in async context or use QFutureWatcher)
auto result = future.result();

if (result.isSuccess()) {
    const PatientIdentity& patient = result.value();
    
    // Validate patient
    if (!patient.isValid()) {
        m_logService->warning("Invalid patient data", {{"mrn", mrn}});
        return;
    }
    
    // Check if data is stale
    if (patient.isStale()) {
        m_logService->warning("Patient data is stale", {
            {"mrn", mrn},
            {"ageHours", QString::number(patient.lookupTime.secsTo(QDateTime::currentDateTime()) / 3600)}
        });
    }
    
    // Use patient data
    displayPatient(patient);
} else {
    handleError(result.error());
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
            
            // Check cache
            auto cachedPatient = cache->findByMrn(mrn);
            if (cachedPatient.has_value()) {
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

### 6.1 Error Codes Reference

| Error Code            | HTTP Status | Description              | Retry? | User Action                 |
| --------------------- | ----------- | ------------------------ | ------ | --------------------------- |
| `InvalidMrn`          | 400         | MRN format invalid       | No     | Correct MRN format          |
| `PatientNotFound`     | 404         | Patient not in HIS       | No     | Verify MRN or manual entry  |
| `NetworkError`        | -           | Cannot reach HIS         | Yes    | Wait for network, use cache |
| `Timeout`             | 408         | Request timed out (> 5s) | Yes    | Retry or use cache          |
| `AuthenticationError` | 401         | API key invalid          | No     | Contact IT support          |
| `ServerError`         | 500         | HIS server error         | Yes    | Retry or contact IT         |
| `UnknownError`        | -           | Unexpected error         | Maybe  | Check logs                  |

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
```

---

## 7. Performance Characteristics

### 7.1 Performance Targets

- **Cache Hit:** < 10ms (database query)
- **Cache Miss → HIS:** < 5 seconds (with timeout)
- **Cache TTL:** 24 hours
- **Cache Size:** ~100 patients (10 KB per patient = 1 MB total)

### 7.2 Optimization Strategies

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

## 8. Security Considerations

- **API Key:** Store securely in encrypted settings (never in source code)
- **HTTPS:** Always use HTTPS (never HTTP for production)
- **PHI:** Patient data is PHI - encrypt in transit and at rest, log access, audit all lookups
- **Timeout:** 5-second timeout prevents hanging requests
- **Rate Limiting:** HIS may rate-limit (handle 429 Too Many Requests responses)
- **Authentication:** Use mTLS or OAuth2 for production HIS integration

---

## 9. Testing

### 9.1 Unit Test Examples

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
    EXPECT_EQ(result.value().age(), 59);
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
```

---

## 10. Integration with Dependency Injection

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
```

---

## 11. Related Documents

- **DOC-COMP-010:** AdmissionService - Uses this interface to lookup patients during admission
- **DOC-COMP-001:** PatientAggregate - Domain entity populated from PatientIdentity
- **DOC-COMP-014:** IPatientRepository - Cache layer for patient data
- **DOC-ARCH-003:** DDD Patterns - Infrastructure layer interface pattern
- **DOC-ARCH-005:** Data Flow and Caching - Cache fallback strategy

---

## 12. Changelog

| Version | Date       | Author         | Changes                                        |
| ------- | ---------- | -------------- | ---------------------------------------------- |
| 1.0     | 2025-11-27 | Z Monitor Team | Migrated from INTERFACE-001, added frontmatter |

---

*This interface abstracts HIS/EHR integration, enabling testability, vendor independence, and graceful offline operation.*
