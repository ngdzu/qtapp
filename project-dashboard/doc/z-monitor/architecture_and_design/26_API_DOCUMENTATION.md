# API Documentation Generation

This document describes the API documentation generation strategy, tools, configuration, and maintenance process for the Z Monitor project.

## 1. Overview

API documentation is automatically generated from source code comments using **Doxygen**, a documentation generation tool for C++ projects. This ensures that API documentation stays synchronized with the codebase and provides a comprehensive reference for developers.

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

- **Target:** 100% coverage of public APIs
- **Measurement:** Doxygen can report undocumented items
- **Enforcement:** Fail CI if public APIs are undocumented

### 12.2. Documentation Tests

```cmake
# Check documentation coverage
add_custom_target(docs-check
    COMMAND ${DOXYGEN_EXECUTABLE} -w html ${CMAKE_BINARY_DIR}/Doxyfile.warn
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking documentation coverage"
)

# Fail if warnings exceed threshold
```

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

## 16. Related Documents

- [09_CLASS_DESIGNS.md](./09_CLASS_DESIGNS.md) - Class design details
- [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) - Code organization
- [18_TESTING_WORKFLOW.md](./18_TESTING_WORKFLOW.md) - Testing documentation
- [07_SETUP_GUIDE.md](./07_SETUP_GUIDE.md) - Setup instructions

