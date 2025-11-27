# API Documentation Generation

This document describes the API documentation generation strategy, tools, configuration, and maintenance process for the Z Monitor project.

> **Requirements Traceability:**  
> **Non-Functional Requirements:** [REQ-NFR-MAIN-010](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md#req-nfr-main-010-api-documentation) - API Documentation  
> **Interface Requirements:** [06_INTERFACE_REQUIREMENTS.md](../../requirements/06_INTERFACE_REQUIREMENTS.md) - Section 10: API Documentation Standards  
> **Regulatory Requirements:** [REQ-REG-62304-002](../../requirements/07_REGULATORY_REQUIREMENTS.md) - Software Design Documentation

---

## 1. Overview

API documentation is automatically generated from source code comments using **Doxygen**, a documentation generation tool for C++ projects. This ensures that API documentation stays synchronized with the codebase and provides a comprehensive reference for developers.

### 1.1 Requirements Addressed

This document implements the following requirements:

- **REQ-NFR-MAIN-010:** Comprehensive API documentation using Doxygen
  - **Target:** >95% coverage of public APIs
  - **Method:** Automatic generation from Doxygen comments
  - **Enforcement:** CI/CD checks, pre-commit hooks (optional)

- **IEC 62304 Compliance:** Software design documentation
  - Detailed design documentation
  - Interface specifications
  - Traceability to requirements

## 2. Tool Selection: Doxygen

### 2.1. Why Doxygen?

- **C++ Native:** Designed specifically for C++ projects
- **Qt Support:** Excellent support for Qt-specific constructs (signals, slots, properties)
- **Multiple Output Formats:** HTML, PDF, LaTeX, XML
- **Wide Adoption:** Industry standard for C++ documentation
- **IDE Integration:** Works well with Qt Creator and other IDEs
- **Mermaid Support:** Can generate diagrams from code structure

### 2.2. Alternatives Considered

- **Sphinx + Breathe:** More complex setup, better for Python projects
- **Natural Docs:** Less common, fewer features for C++
- **Manual Documentation:** Too time-consuming and error-prone

## 3. Documentation Structure

### 3.1. Generated Documentation Includes

- **Modules:** Logical grouping of related classes
- **Classes:** All classes with descriptions, methods, properties
- **Functions:** Function signatures, parameters, return values
- **Enumerations:** All enum types and values
- **Namespaces:** Namespace documentation
- **Files:** File-level documentation
- **Examples:** Code examples from source
- **Diagrams:** Class diagrams, dependency graphs

### 3.2. Documentation Output

- **HTML:** Primary format, browsable documentation
- **PDF:** For offline reference (optional)
- **XML:** For integration with other tools (optional)

## 4. Doxygen Configuration

### 4.1. Configuration File: `Doxyfile`

Located at project root: `project-dashboard/Doxyfile`

```ini
# Project Information
PROJECT_NAME           = "Z Monitor"
PROJECT_NUMBER         = "1.0"
PROJECT_BRIEF          = "Real-time Patient Monitoring System"
PROJECT_LOGO           = 

# Source Code
INPUT                  = src/
INPUT_ENCODING         = UTF-8
FILE_PATTERNS          = *.cpp *.h *.qml
RECURSIVE              = YES
EXCLUDE                = 
EXCLUDE_PATTERNS       = */tests/* */mocks/* */build/*
EXCLUDE_SYMLINKS        = NO

# Output
OUTPUT_DIRECTORY       = docs/api/
HTML_OUTPUT            = html
HTML_FILE_EXTENSION    = .html
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
GENERATE_XML           = NO

# Source Browser
SOURCE_BROWSER          = YES
INLINE_SOURCES          = NO
STRIP_CODE_COMMENTS     = NO

# References
REFERENCED_BY_RELATION  = YES
REFERENCES_RELATION     = YES
REFERENCES_LINK_SOURCE  = YES

# Qt-Specific
MACRO_EXPANSION         = YES
EXPAND_AS_DEFINED       = Q_OBJECT Q_PROPERTY Q_INVOKABLE

# Diagrams
HAVE_DOT                = YES
CLASS_DIAGRAMS           = YES
CALL_GRAPH              = NO
CALLER_GRAPH            = NO
GRAPHICAL_HIERARCHY      = YES
DIRECTORY_GRAPH          = YES

# Build
GENERATE_HTMLHELP       = NO
GENERATE_QHP            = YES
QHP_NAMESPACE           = "com.zmonitor.documentation"
QHP_VIRTUAL_FOLDER      = "doc"
```

### 4.2. Qt-Specific Configuration

Doxygen is configured to recognize Qt-specific constructs:

- **Q_OBJECT:** Classes with Qt meta-object system
- **Q_PROPERTY:** Qt properties
- **Q_INVOKABLE:** Invokable methods
- **Signals/Slots:** Qt signal-slot mechanism

## 5. Comment Style Guidelines

### 5.1. File Header Comments

Every source file should have a header comment:

```cpp
/**
 * @file NetworkManager.h
 * @brief Manages secure network connectivity to the central server.
 * 
 * This class handles mTLS connections, certificate validation, and
 * telemetry data transmission. It integrates with ITelemetryServer
 * for server communication.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */
```

### 5.2. Class Documentation

Every class should have comprehensive documentation:

```cpp
/**
 * @class NetworkManager
 * @brief Manages secure network connectivity to the central server.
 * 
 * This class provides secure communication with the central monitoring
 * server using mutual TLS (mTLS). It handles certificate validation,
 * data signing, rate limiting, and security audit logging.
 * 
 * @note All network operations are asynchronous and non-blocking.
 * 
 * @see ITelemetryServer
 * @see CertificateManager
 * 
 * @ingroup CoreServices
 */
class NetworkManager : public QObject {
    Q_OBJECT
    // ...
};
```

### 5.3. Method Documentation

All public methods must be documented:

```cpp
/**
 * @brief Connects to the central server using mTLS.
 * 
 * Initiates a secure connection to the configured central server.
 * The connection is asynchronous and non-blocking. Monitor the
 * connectionStatusChanged() signal for connection state updates.
 * 
 * @param url Optional server URL (defaults to configured server URL)
 * @return true if connection initiated successfully, false otherwise
 * 
 * @note Connection is asynchronous. Monitor connectionStatusChanged()
 *       signal for connection state updates.
 * 
 * @see disconnectFromServer()
 * @see connectionStatusChanged()
 * 
 * @sa ITelemetryServer::Connect()
 */
bool connectToServer(const QString& url = QString());
```

### 5.4. Parameter Documentation

Document all parameters:

```cpp
/**
 * @brief Sends telemetry data to the server.
 * 
 * @param data Telemetry data to send (must include patient MRN)
 * @return Result containing server response or error
 * 
 * @note Automatically includes current patient MRN from PatientManager.
 *       If no patient is admitted, patient telemetry is not sent.
 * 
 * @throws NetworkError if connection fails
 */
Result<ServerResponse, NetworkError> sendTelemetry(const TelemetryData& data);
```

### 5.5. Property Documentation (Qt)

Document Q_PROPERTY declarations:

```cpp
/**
 * @property ConnectionStatus status
 * @brief Current connection status.
 * 
 * Indicates whether the device is connected to the central server.
 * 
 * @see connectionStatusChanged()
 */
Q_PROPERTY(ConnectionStatus status READ status NOTIFY statusChanged)
```

### 5.6. Enum Documentation

Document enumerations:

```cpp
/**
 * @enum ConnectionStatus
 * @brief Network connection status.
 * 
 * Represents the current state of the network connection to the
 * central server.
 */
enum class ConnectionStatus {
    Disconnected,  ///< Not connected to server
    Connecting,    ///< Connection in progress
    Connected,     ///< Successfully connected
    Error          ///< Connection error occurred
};
```

### 5.7. Namespace Documentation

Document namespaces:

```cpp
/**
 * @namespace ZMonitor::Core
 * @brief Core services and business logic.
 * 
 * This namespace contains all core services that implement the
 * business logic of the Z Monitor application. These services
 * have no UI dependencies and can be used independently.
 * 
 * @see ZMonitor::Controllers
 */
namespace ZMonitor {
namespace Core {
    // ...
}
}
```

## 6. Module Organization

### 6.1. Module Groups

Organize documentation into logical modules:

```cpp
/**
 * @defgroup CoreServices Core Services
 * @brief Core business logic services.
 * 
 * These services implement the core functionality of the Z Monitor
 * application. They have no UI dependencies and can be tested
 * independently.
 * 
 * @{
 */

class NetworkManager { /* ... */ };
class DatabaseManager { /* ... */ };
class AlarmManager { /* ... */ };

/**
 * @}
 */
```

### 6.2. Module List

- **CoreServices:** Core business logic (NetworkManager, DatabaseManager, etc.)
- **Controllers:** UI controllers (DashboardController, AlarmController, etc.)
- **Interfaces:** Abstract interfaces (ITelemetryServer, IPatientLookupService, etc.)
- **Models:** Data models (Patient, VitalSign, Alarm, etc.)
- **Utils:** Utility functions (CryptoUtils, DateTimeUtils, etc.)

## 7. Build Integration

### 7.1. CMake Integration

Add Doxygen generation to CMake:

```cmake
# Find Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    # Configure Doxyfile
    configure_file(
        ${CMAKE_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_BINARY_DIR}/Doxyfile
        @ONLY
    )
    
    # Add custom target
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    # Install documentation
    install(DIRECTORY ${CMAKE_BINARY_DIR}/docs/api/html
            DESTINATION share/doc/z-monitor/api
            OPTIONAL
    )
endif()
```

### 7.2. Build Commands

```bash
# Generate documentation
cmake --build build --target docs

# Or directly
doxygen Doxyfile
```

## 7.3. CI/CD Integration

### Pre-commit Hook (Optional)

A lightweight pre-commit hook is available to check for Doxygen comments:

```bash
# Install pre-commit
pip install pre-commit

# Install hooks
pre-commit install

# Run manually
pre-commit run --all-files
```

The pre-commit hook (`scripts/pre-commit-doxygen-check.sh`) performs a lightweight check:
- **Warns** if public APIs may be missing Doxygen comments
- **Does not fail** the commit (warning only)
- **Skips** if Doxygen is not installed

**Note:** The pre-commit hook is optional and configured to warn only. It does not block commits to avoid developer friction.

### GitHub Actions Workflow (Recommended)

A GitHub Actions workflow (`.github/workflows/doxygen-docs.yml`) automatically generates documentation:

**Triggers:**
- **Nightly:** Runs at 2 AM UTC daily
- **On Push:** Runs when code changes are pushed to main/master
- **On PR:** Runs on pull requests (can be disabled if too slow)
- **Manual:** Can be triggered manually via workflow_dispatch

**Features:**
- Generates complete API documentation
- Checks documentation coverage
- Uploads documentation as artifacts
- Comments on PRs with documentation status
- Fails if too many undocumented items (>10 threshold)

**Benefits:**
- No developer overhead - runs automatically
- Catches documentation gaps early
- Provides documentation artifacts for review
- Can be deployed to GitHub Pages (optional)

**Configuration:**
- Workflow file: `.github/workflows/doxygen-docs.yml`
- Doxyfile location: `project-dashboard/Doxyfile`
- Output location: `project-dashboard/docs/api/html`

To disable PR checks (if they're too slow), edit the workflow and remove or comment out the `pull_request` trigger.

## 8. Documentation Maintenance

### 8.1. Code Review Checklist

- [ ] All public classes have class documentation
- [ ] All public methods have method documentation
- [ ] All parameters are documented
- [ ] Return values are documented
- [ ] Exceptions/errors are documented
- [ ] Examples are provided for complex APIs
- [ ] Cross-references (@see, @sa) are included

### 8.2. Documentation Updates

- **New Classes:** Add comprehensive class documentation
- **New Methods:** Document immediately when adding methods
- **API Changes:** Update documentation when changing APIs
- **Refactoring:** Update documentation when refactoring

### 8.3. Documentation Review

- Review generated documentation before releases
- Verify all public APIs are documented
- Check for broken links
- Verify examples are correct

## 9. Documentation Output

### 9.1. HTML Output Structure

```
docs/api/html/
├── index.html              # Main entry point
├── classes.html            # Class list
├── functions.html          # Function list
├── modules.html            # Module list
├── files.html              # File list
├── namespacemembers.html  # Namespace members
└── class_NetworkManager.html  # Individual class pages
```

### 9.2. Publishing Documentation

- **Local Development:** View in browser from `docs/api/html/`
- **CI/CD:** Publish to documentation server on release
- **Offline:** Generate PDF for offline reference (optional)

## 10. Examples and Tutorials

### 10.1. Code Examples

Include examples in documentation:

```cpp
/**
 * @brief Example: Sending telemetry data
 * 
 * @code
 * NetworkManager* network = new NetworkManager(this);
 * 
 * TelemetryData data;
 * data.deviceId = "ZM-001";
 * data.patientMrn = "12345";
 * data.vitals = { /* vitals data */ };
 * 
 * auto result = network->sendTelemetry(data);
 * if (result.isSuccess()) {
 *     qDebug() << "Telemetry sent successfully";
 * } else {
 *     qDebug() << "Error:" << result.error().message();
 * }
 * @endcode
 */
```

### 10.2. Tutorial Pages

Create tutorial pages using Doxygen pages:

```markdown
/**
 * @page tutorial_networking Network Communication Tutorial
 * 
 * This tutorial explains how to use NetworkManager to send
 * telemetry data to the central server.
 * 
 * @section setup Setup
 * 
 * First, configure the server URL...
 * 
 * @section sending Sending Data
 * 
 * To send telemetry data...
 */
```

## 11. Integration with Existing Documentation

### 11.1. Cross-References

Link to design documents:

```cpp
/**
 * @brief Manages secure network connectivity.
 * 
 * @see doc/06_SECURITY.md for security architecture
 * @see doc/09_CLASS_DESIGNS.md for class design details
 * @see doc/20_ERROR_HANDLING_STRATEGY.md for error handling
 */
```

### 11.2. Design Document Links

Design documents can link to API documentation:

```markdown
See [API Documentation](../api/html/class_NetworkManager.html) for
complete API reference.
```

## 12. Quality Assurance

### 12.1. Documentation Coverage

**Requirement:** REQ-NFR-MAIN-010 specifies >95% coverage of public APIs

- **Target:** 100% coverage of public APIs (exceeds requirement)
- **Minimum:** 95% coverage (requirement threshold)
- **Measurement:** Doxygen can report undocumented items
- **Enforcement:** Fail CI if coverage < 95%

### 12.2. Coverage Metrics

Track the following metrics:

|| Metric | Target | Measurement Method |
||--------|--------|-------------------|
|| **Public Class Coverage** | 100% | Count documented classes / total public classes |
|| **Public Method Coverage** | 100% | Count documented methods / total public methods |
|| **Parameter Documentation** | 100% | All method parameters have @param tags |
|| **Return Value Documentation** | 100% | All non-void methods have @return tags |
|| **Example Coverage** | >50% | Complex APIs have @code examples |
|| **Cross-Reference Usage** | >80% | Methods link to related classes/docs |

### 12.3. Documentation Tests

```cmake
# Check documentation coverage
add_custom_target(docs-check
    COMMAND ${DOXYGEN_EXECUTABLE} -w html ${CMAKE_BINARY_DIR}/Doxyfile.warn
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking documentation coverage"
)

# Fail if coverage below threshold (95%)
add_custom_target(docs-coverage-check
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/check_doxygen_coverage.py
            --threshold 95
            --fail-on-undocumented
    DEPENDS docs
    COMMENT "Verifying documentation coverage meets REQ-NFR-MAIN-010 (>95%)"
)
```

### 12.4. Compliance Verification

**IEC 62304 Requirements:**

|| IEC 62304 Section | Requirement | Implementation |
||-------------------|-------------|----------------|
|| 5.2.2 | Software detailed design | Doxygen class/method documentation |
|| 5.2.3 | Interface specification | Interface documentation (ISensorDataSource, etc.) |
|| 5.2.4 | Software unit specification | Unit documentation with test references |
|| 5.7.3 | Traceability | @see tags link to requirement docs |

**Verification:**
- [ ] All public APIs documented (>95% coverage)
- [ ] All interfaces have complete documentation
- [ ] Traceability links exist (@see requirements)
- [ ] Examples provided for complex APIs
- [ ] Documentation published and accessible

## 13. Best Practices

### 13.1. Do's

- ✅ Document all public APIs
- ✅ Use consistent comment style
- ✅ Include examples for complex APIs
- ✅ Keep documentation up-to-date
- ✅ Use cross-references (@see, @sa)
- ✅ Document parameters and return values
- ✅ Include usage notes and warnings

### 13.2. Don'ts

- ❌ Don't document implementation details
- ❌ Don't duplicate code in comments
- ❌ Don't leave outdated documentation
- ❌ Don't document private/internal APIs (unless necessary)
- ❌ Don't use ambiguous language

## 14. Tools and Resources

### 14.1. Doxygen Resources

- **Official Documentation:** https://www.doxygen.nl/
- **Qt Integration:** https://doc.qt.io/qt-6/qdoc.html (alternative)
- **Doxygen Wizard:** GUI tool for configuration

### 14.2. Related Tools

- **Graphviz:** For generating diagrams (required for Doxygen diagrams)
- **Doxywizard:** GUI configuration tool
- **Doxybook2:** Convert Doxygen XML to other formats

## 15. Maintenance Tasks

### 15.1. Regular Tasks

- **Weekly:** Review new code for documentation
- **Monthly:** Regenerate and review documentation
- **Per Release:** Full documentation review and update

### 15.2. Documentation Metrics

Track:
- Documentation coverage percentage
- Number of undocumented public APIs
- Documentation update frequency
- Broken link count

## 16. Security and Audit Requirements

### 16.1 Security Event Documentation

Per **REQ-SEC-AUDIT-001**, all security-relevant events must be logged. This includes:

**Documentation Requirements for Security Classes:**

```cpp
/**
 * @class SecurityService
 * @brief Manages user authentication and session management.
 * 
 * @security This class handles sensitive operations:
 * - PIN validation (bcrypt hashing)
 * - Session token generation
 * - Brute force protection
 * - Audit logging of all authentication attempts
 * 
 * @note All security events are logged to security_audit_log table
 *       per REQ-SEC-AUDIT-001
 * 
 * @see IAuditRepository
 * @see REQ-SEC-AUTH-001 (PIN authentication)
 * @see REQ-SEC-AUDIT-001 (comprehensive audit logging)
 */
class SecurityService : public QObject {
    // ...
};
```

### 16.2 Audit Log Protection

Per **REQ-SEC-AUDIT-002**, audit logs must be protected. Document security mechanisms:

```cpp
/**
 * @class AuditRepository
 * @brief Tamper-evident audit log repository with hash chain.
 * 
 * @security Implements hash chain for tamper detection:
 * - Each entry contains SHA-256 hash of previous entry
 * - Validates chain integrity on read operations
 * - Detects unauthorized modifications
 * 
 * @note Only administrators can view audit logs (REQ-SEC-AUTHZ-002)
 * 
 * @see REQ-SEC-AUDIT-002 (audit log protection)
 * @see 10_DATABASE_DESIGN.md (security_audit_log schema)
 */
```

### 16.3 Sensitive Data Handling

Document all classes that handle PHI or sensitive data:

```cpp
/**
 * @class PatientAggregate
 * @brief Patient demographic and clinical data aggregate.
 * 
 * @warning Contains Protected Health Information (PHI)
 * - Access requires VIEW_PATIENT_DATA permission
 * - All access logged per HIPAA requirements
 * - Data encrypted at rest (SQLCipher AES-256)
 * 
 * @see REQ-SEC-ENC-003 (encryption at rest)
 * @see REQ-REG-HIPAA-001 (PHI protection)
 */
```

---

## 17. Regulatory Compliance Documentation

### 17.1 IEC 62304 Traceability

All public APIs must include traceability to requirements:

```cpp
/**
 * @brief Triggers high-priority alarm.
 * 
 * @requirements
 * - REQ-FUN-ALARM-001: System shall trigger alarms
 * - REQ-FUN-ALARM-010: Alarm latency < 50ms
 * - REQ-REG-60601-001: IEC 60601-1-8 compliance
 * 
 * @param vital Vital signs that triggered alarm
 * @param threshold Threshold that was exceeded
 * 
 * @performance Target latency: < 50ms
 * @thread Real-Time Processing Thread
 * 
 * @see AlarmAggregate
 * @see 04_ALARM_SYSTEM.md
 */
void raiseAlarm(const VitalRecord& vital, const AlarmThreshold& threshold);
```

### 17.2 Medical Device Classification

Document safety classification per IEC 62304:

```cpp
/**
 * @class AlarmManager
 * @brief Critical alarm detection and escalation system.
 * 
 * @safety CLASS C (IEC 62304)
 * - Alarm failure could result in patient injury or death
 * - Requires comprehensive testing and validation
 * - All alarm events logged for audit
 * 
 * @performance Critical timing requirements:
 * - Alarm detection: < 50ms (REQ-FUN-ALARM-010)
 * - Audio latency: < 100ms (REQ-REG-60601-002)
 * - Escalation delay: 60s HIGH, 120s MEDIUM (REQ-FUN-ALARM-030)
 * 
 * @see REQ-REG-62304-001 (software safety classification)
 * @see 04_ALARM_SYSTEM.md (complete alarm specification)
 */
```

---

## 18. Interface Documentation Standards

### 18.1 External Service Interfaces

All external service interfaces must have complete documentation:

**Example: ISensorDataSource**

Location: [interfaces/ISensorDataSource.md](./interfaces/ISensorDataSource.md)

Required documentation:
- ✅ Complete C++ interface definition with Doxygen comments
- ✅ All implementations (WebSocketSensorDataSource, MockSensorDataSource, etc.)
- ✅ Data structures (VitalRecord, WaveformSample)
- ✅ Usage examples
- ✅ Testing strategies
- ✅ Performance considerations
- ✅ Security notes

**Documented Interfaces:**
1. [ISensorDataSource.md](./interfaces/ISensorDataSource.md) - Sensor data acquisition
2. [IPatientLookupService.md](./interfaces/IPatientLookupService.md) - Patient demographic lookup
3. [ITelemetryServer.md](./interfaces/ITelemetryServer.md) - Telemetry transmission
4. [IProvisioningService.md](./interfaces/IProvisioningService.md) - Device provisioning

### 18.2 Repository Interfaces

All repository interfaces must document:
- Domain methods (no SQL leakage)
- Return types (domain objects, not QSqlRecord)
- Error handling (std::optional for not found)
- Thread safety guarantees

**Example:**

```cpp
/**
 * @interface IPatientRepository
 * @brief Repository for patient aggregate persistence.
 * 
 * @thread Database I/O Thread
 * @performance Non-critical (background operations)
 * 
 * @see PatientAggregate
 * @see 30_DATABASE_ACCESS_STRATEGY.md
 * 
 * @ingroup Repositories
 */
class IPatientRepository {
    // ...
};
```

---

## 19. Performance Documentation

### 19.1 Critical Path Documentation

Document performance-critical components:

```cpp
/**
 * @class VitalsCache
 * @brief Thread-safe in-memory cache for vital signs (3-day capacity).
 * 
 * @performance PRIORITY 1 (CRITICAL PATH)
 * - Target latency: < 5ms for append()
 * - Target latency: < 10ms for getRange()
 * - Thread-safe using QReadWriteLock
 * 
 * @capacity
 * - 3 days of vitals (~39 MB)
 * - ~259,200 records at 1 Hz
 * - FIFO eviction when full
 * 
 * @thread Real-Time Processing Thread
 * 
 * @see 36_DATA_CACHING_STRATEGY.md
 * @see MonitoringService
 * 
 * @ingroup DataCaching
 */
class VitalsCache {
    // ...
};
```

### 19.2 Thread Assignment Documentation

All components must document thread assignment:

```cpp
/**
 * @class PersistenceScheduler
 * @brief Manages periodic persistence of in-memory cache to database.
 * 
 * @thread Database I/O Thread
 * @priority PRIORITY 3 (MEDIUM - Background)
 * 
 * @schedule Every 10 minutes OR when 10,000 records accumulated
 * 
 * @performance
 * - Target latency: < 5 seconds (batch insert)
 * - Non-blocking (runs on background thread)
 * 
 * @see VitalsCache
 * @see IVitalsRepository
 * @see 36_DATA_CACHING_STRATEGY.md
 * @see 12_THREAD_MODEL.md
 */
```

---

## 20. Data Caching Documentation

### 20.1 Caching Components

Per [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md), document caching architecture:

**Required documentation for caching classes:**

1. **VitalsCache:**
   - 3-day in-memory capacity
   - Thread safety (QReadWriteLock)
   - FIFO eviction policy
   - Critical path (< 5ms latency)

2. **WaveformCache:**
   - 30-second circular buffer
   - Thread safety
   - Overwrite oldest policy
   - Display-only (not persisted)

3. **PersistenceScheduler:**
   - 10-minute schedule
   - Batch size (10,000 records)
   - Background thread operation
   - Failure handling

4. **DataCleanupService:**
   - Daily schedule (3 AM)
   - 7-day retention policy
   - Batch delete strategy
   - Vacuum frequency

---

## 21. Component Count and Coverage

### 21.1 Documentation Scope

Per [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md), the system has **117 components**:

|| Layer | Components | Documentation Required |
||-------|------------|----------------------|
|| Domain | 45 | All public aggregates, value objects, interfaces |
|| Application | 13 | All services and DTOs |
|| Infrastructure | 39 | All public adapters and implementations |
|| Interface | 29 | All controllers and QML component APIs |

**Target:** Document all 117 components' public APIs

### 21.2 Priority Components (Must Document First)

**CRITICAL (PRIORITY 1):**
1. `MonitoringService` - Core monitoring orchestration
2. `VitalsCache` - In-memory cache (critical path)
3. `WaveformCache` - Waveform display cache
4. `AlarmManager` - Alarm detection and escalation
5. `ISensorDataSource` - Sensor data abstraction

**HIGH (PRIORITY 2):**
6. `ITelemetryServer` - Telemetry transmission
7. `IPatientLookupService` - Patient lookup
8. `IProvisioningService` - Device provisioning
9. `AdmissionService` - ADT workflow
10. `SecurityService` - Authentication and authorization

**MEDIUM (PRIORITY 3):**
11. All repository interfaces and implementations
12. `PersistenceScheduler` - Database persistence
13. `DataCleanupService` - Data retention
14. All QML controllers

**LOW (PRIORITY 4):**
15. Utility classes
16. Internal implementation details (optional)

---

## 22. External Interface Documentation

### 22.1 External Service Interface Standards

Per **Section 10 of 06_INTERFACE_REQUIREMENTS.md**, external APIs follow these standards:

**REST API Documentation:**
- HTTP methods clearly documented
- Status codes specified
- Request/response formats (JSON schemas)
- Error handling and codes
- Rate limiting policies
- Authentication requirements

**Example:**

```cpp
/**
 * @interface ITelemetryServer
 * @brief Secure telemetry transmission to central server.
 * 
 * @protocol HTTPS with mTLS
 * @endpoint POST /api/v1/telemetry/batch
 * @contentType application/json
 * @authentication mTLS (device certificate)
 * @rateLimit 100 requests/minute
 * 
 * @request TelemetrySubmission (JSON)
 * @response ServerResponse (JSON)
 * 
 * @errorCodes
 * - 200: Success
 * - 400: Invalid data
 * - 401: Authentication failed
 * - 429: Rate limit exceeded
 * - 500: Server error
 * 
 * @see doc/interfaces/ITelemetryServer.md (complete specification)
 * @see REQ-INT-SRV-001 (telemetry server interface)
 */
```

---

## 23. Enforcement and Workflow

### 23.1 Developer Workflow

**When writing new code:**

1. **Write Doxygen comments FIRST** (before implementation)
   - Class documentation
   - Method documentation
   - Parameter/return documentation

2. **Implement the code**

3. **Verify documentation:**
   ```bash
   # Generate docs and check for warnings
   cmake --build build --target docs
   
   # Check coverage
   python3 scripts/check_doxygen_coverage.py --threshold 95
   ```

4. **Fix any undocumented APIs**

### 23.2 Code Review Checklist

**For all pull requests, verify:**

- [ ] All new public classes have Doxygen class comments
- [ ] All new public methods have Doxygen method comments
- [ ] All parameters documented with @param
- [ ] All return values documented with @return
- [ ] Exceptions/errors documented
- [ ] Examples provided for complex APIs (if applicable)
- [ ] Cross-references (@see, @sa) included
- [ ] File headers present
- [ ] Traceability to requirements (@see REQ-XXX)
- [ ] Performance notes (@performance tag)
- [ ] Thread assignment (@thread tag)
- [ ] Safety classification (@safety tag, if applicable)

### 23.3 CI/CD Enforcement

**GitHub Actions Workflow:**

The workflow (`.github/workflows/doxygen-docs.yml`) automatically:

1. **Generates documentation** on every push/PR
2. **Checks coverage** against 95% threshold
3. **Uploads artifacts** for review
4. **Comments on PRs** with coverage status
5. **Fails build** if coverage < 95% (REQ-NFR-MAIN-010)

**Triggers:**
- Nightly at 2 AM UTC
- On push to main/master
- On pull requests (optional, can be disabled)
- Manual trigger via workflow_dispatch

---

## 24. Related Documents

### 24.1 Requirements

- [REQ-NFR-MAIN-010](../../requirements/04_NON_FUNCTIONAL_REQUIREMENTS.md#req-nfr-main-010-api-documentation) - API Documentation requirement (>95% coverage)
- [06_INTERFACE_REQUIREMENTS.md](../../requirements/06_INTERFACE_REQUIREMENTS.md) - Section 10: API Documentation Standards
- [07_REGULATORY_REQUIREMENTS.md](../../requirements/07_REGULATORY_REQUIREMENTS.md) - IEC 62304 documentation requirements

### 24.2 Architecture and Design

- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - Class design details
- [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) - Code organization
- [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) - Complete component list (117 components)
- [36_DATA_CACHING_STRATEGY.md](./36_DATA_CACHING_STRATEGY.md) - Caching architecture
- [30_DATABASE_ACCESS_STRATEGY.md](./30_DATABASE_ACCESS_STRATEGY.md) - Database strategy

### 24.3 Interfaces

- [interfaces/ISensorDataSource.md](./interfaces/ISensorDataSource.md) - Sensor data interface
- [interfaces/IPatientLookupService.md](./interfaces/IPatientLookupService.md) - Patient lookup interface
- [interfaces/ITelemetryServer.md](./interfaces/ITelemetryServer.md) - Telemetry server interface
- [interfaces/IProvisioningService.md](./interfaces/IProvisioningService.md) - Provisioning interface

### 24.4 Testing and Quality

- [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Testing documentation
- [07_SETUP_GUIDE.md](./07_SETUP_GUIDE.md) - Setup instructions

---

**Document Version:** 2.0  
**Last Updated:** 2025-11-27  
**Status:** Updated to align with requirements and new architecture components

*This document defines the API documentation strategy for Z Monitor, ensuring compliance with REQ-NFR-MAIN-010 (>95% coverage), IEC 62304 (traceability), and HIPAA (security documentation).*

