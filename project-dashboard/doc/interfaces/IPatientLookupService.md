# IPatientLookupService Interface

## Purpose

Provides a standardized interface for looking up patient information from external systems (e.g., Hospital Information System, Electronic Health Records). This interface allows the Z Monitor to retrieve patient demographics and safety information by patient ID without requiring manual data entry.

## Responsibilities

- Query external patient information systems by patient ID (or MRN)
- Return structured patient data including demographics, allergies, and safety information
- Handle lookup failures gracefully (network errors, patient not found, etc.)
- Support both synchronous and asynchronous lookup patterns
- Cache lookup results locally to reduce external system load

## Threading & Ownership

- `IPatientLookupService` implementations should be owned by `AppContainer` or `main()`
- Lookup operations may be blocking (network I/O) and should run on a worker thread or use Qt's async networking
- The interface should be thread-safe for concurrent lookups from multiple UI threads
- For Qt-based implementations, prefer `QNetworkAccessManager` on a dedicated network thread

## Key API (Suggested Signatures)

### C++ Interface

```cpp
class IPatientLookupService {
public:
    virtual ~IPatientLookupService() = default;
    
    // Synchronous lookup (blocking, use with caution)
    virtual std::optional<PatientInfo> LookupPatient(const QString& patientId) = 0;
    
    // Asynchronous lookup (preferred)
    virtual void LookupPatientAsync(
        const QString& patientId,
        std::function<void(const std::optional<PatientInfo>&)> callback
    ) = 0;
    
    // Check if service is available/configured
    virtual bool IsAvailable() const = 0;
    
    // Get last error message (if lookup failed)
    virtual QString GetLastError() const = 0;
    
signals:
    // Qt-style signal for async lookups (alternative to callback)
    void patientLookupCompleted(const QString& patientId, const PatientInfo& info);
    void patientLookupFailed(const QString& patientId, const QString& error);
};
```

### PatientInfo Structure

```cpp
struct PatientInfo {
    QString patientId;      // Primary identifier
    QString mrn;             // Medical Record Number
    QString name;             // Full name
    QDate dateOfBirth;        // Date of birth
    QString sex;              // "M", "F", or other
    QStringList allergies;    // List of known allergies
    QString room;             // Current room/bed assignment
    QDateTime lastUpdated;    // When this info was last refreshed
};
```

## Error Semantics

- Return `std::nullopt` or emit `patientLookupFailed` signal when:
  - Patient ID not found
  - Network/connection error
  - Service unavailable
  - Authentication/authorization failure
  - Invalid patient ID format

- Use `GetLastError()` to provide detailed error messages for logging and user feedback

## Example Code Paths

### Synchronous Lookup (Simple Case)
```cpp
auto lookupService = appContainer->GetPatientLookupService();
auto patientInfo = lookupService->LookupPatient("P12345");
if (patientInfo) {
    patientManager->SetCurrentPatient(*patientInfo);
} else {
    logService->LogError("Patient lookup failed: " + lookupService->GetLastError());
}
```

### Asynchronous Lookup (Preferred)
```cpp
auto lookupService = appContainer->GetPatientLookupService();
lookupService->LookupPatientAsync("P12345", [this](const auto& info) {
    if (info) {
        patientManager->SetCurrentPatient(*info);
        patientController->updatePatientDisplay();
    } else {
        notificationController->AddNotification(
            "Patient lookup failed", "error"
        );
    }
});
```

### Qt Signal-Based (Alternative)
```cpp
connect(lookupService, &IPatientLookupService::patientLookupCompleted,
        this, [this](const QString& id, const PatientInfo& info) {
    patientManager->SetCurrentPatient(info);
});

connect(lookupService, &IPatientLookupService::patientLookupFailed,
        this, [this](const QString& id, const QString& error) {
    notificationController->AddNotification(
        QString("Failed to lookup patient %1: %2").arg(id, error),
        "error"
    );
});

lookupService->LookupPatientAsync("P12345", nullptr); // Uses signals
```

## Implementation Notes

### Mock Implementation (for Testing)
- `MockPatientLookupService`: Returns hardcoded patient data for unit tests
- Supports injection of test data and simulated failures

### Network Implementation (Production)
- `NetworkPatientLookupService`: Connects to hospital HIS/EHR via REST API
- Uses mTLS for secure communication
- Implements retry logic and timeout handling
- Caches results locally to reduce network load

### Local Database Implementation (Fallback)
- `DatabasePatientLookupService`: Queries local `patients` table
- Used when external system is unavailable
- Requires patient data to be pre-loaded into local database

## Integration with PatientManager

`PatientManager` should use `IPatientLookupService` for patient lookups:

```cpp
class PatientManager {
    std::unique_ptr<IPatientLookupService> lookupService;
    
public:
    void LoadPatientById(const QString& patientId) {
        // First check local cache/database
        auto localPatient = databaseManager->GetPatient(patientId);
        if (localPatient) {
            SetCurrentPatient(*localPatient);
            return;
        }
        
        // If not found locally, lookup from external system
        lookupService->LookupPatientAsync(patientId, [this](const auto& info) {
            if (info) {
                // Save to local database for future use
                databaseManager->SavePatient(*info);
                SetCurrentPatient(*info);
            }
        });
    }
};
```

## Tests to Write

1. **Synchronous Lookup Tests:**
   - Valid patient ID returns correct PatientInfo
   - Invalid patient ID returns nullopt
   - Network error handling
   - Timeout handling

2. **Asynchronous Lookup Tests:**
   - Callback invoked with correct data
   - Callback invoked with nullopt on failure
   - Signal emission on success/failure

3. **Error Handling Tests:**
   - GetLastError() returns meaningful messages
   - Multiple concurrent lookups handled correctly
   - Service unavailable state handled

4. **Integration Tests:**
   - PatientManager integration with lookup service
   - Local database caching after lookup
   - UI updates via PatientController after lookup

## Security Considerations

- Patient lookups must be logged for audit purposes
- Only authorized users (Clinician/Technician roles) should trigger lookups
- Patient data must be encrypted in transit (HTTPS/mTLS)
- Consider rate limiting to prevent abuse
- Cache patient data locally with appropriate retention policies

