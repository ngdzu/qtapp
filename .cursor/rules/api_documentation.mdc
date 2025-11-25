---
alwaysApply: true
---

# API Documentation Guidelines

## Overview

All code in the Z Monitor project must include comprehensive API documentation using **Doxygen-style comments** from the very beginning. This ensures that API documentation is generated automatically and stays synchronized with the codebase.

## Critical Rule: Document as You Code

**NEVER write code without documentation comments.** Documentation is not optional - it is a required part of every public API.

## Documentation Tool

- **Tool:** Doxygen (configured in `project-dashboard/Doxyfile`)
- **Comment Style:** Doxygen-style (`/** */` comments)
- **Generation:** Run `cmake --build build --target docs` to generate HTML documentation

See `project-dashboard/doc/26_API_DOCUMENTATION.md` for complete API documentation strategy.

## Required Documentation

### All Public Classes

Every public class must have a class-level documentation comment:

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
 * @ingroup CoreServices
 */
class NetworkManager : public QObject {
    // ...
};
```

### All Public Methods

Every public method must be documented with:
- Brief description
- Parameter descriptions (`@param`)
- Return value description (`@return`)
- Exceptions/errors (`@throws` or `@note`)
- Usage notes (`@note`)
- Examples for complex APIs (`@code` / `@endcode`)

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
 */
bool connectToServer(const QString& url = QString());
```

### All Public Properties (Qt)

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

### All Enumerations

Document enum types and values:

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

### All Namespaces

Document namespaces:

```cpp
/**
 * @namespace ZMonitor::Core
 * @brief Core services and business logic.
 * 
 * This namespace contains all core services that implement the
 * business logic of the Z Monitor application. These services
 * have no UI dependencies and can be used independently.
 */
namespace ZMonitor {
namespace Core {
    // ...
}
}
```

### File Headers

Every source file must have a header comment:

```cpp
/**
 * @file NetworkManager.h
 * @brief Manages secure network connectivity to the central server.
 * 
 * This file contains the NetworkManager class which handles mTLS
 * connections, certificate validation, and telemetry data transmission.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */
```

## Documentation Standards

### Comment Format

- Use `/** */` for Doxygen comments (not `/* */` or `//`)
- Start with `/**` on its own line
- Use `@brief` for brief descriptions
- Use `@param` for parameters
- Use `@return` for return values
- Use `@note` for important notes
- Use `@see` or `@sa` for cross-references
- Use `@code` / `@endcode` for examples

### What to Document

**DO Document:**
- ✅ Purpose and responsibility of classes
- ✅ What methods do (not how they do it)
- ✅ Parameters and their constraints
- ✅ Return values and their meaning
- ✅ Exceptions and error conditions
- ✅ Usage examples for complex APIs
- ✅ Thread safety and concurrency notes
- ✅ Performance considerations
- ✅ Related classes and methods

**DON'T Document:**
- ❌ Implementation details (how it works internally)
- ❌ Obvious things (getters that just return a value)
- ❌ Private/internal APIs (unless necessary for understanding)

### Examples

Include examples for complex APIs:

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

## Module Organization

Use `@defgroup` to organize classes into modules:

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

/**
 * @}
 */
```

## Code Review Checklist

When reviewing code, verify:

- [ ] All public classes have class documentation
- [ ] All public methods have method documentation
- [ ] All parameters are documented with `@param`
- [ ] Return values are documented with `@return`
- [ ] Exceptions/errors are documented
- [ ] Examples are provided for complex APIs
- [ ] Cross-references (@see, @sa) are included
- [ ] File headers are present

## Integration with Build

Documentation is generated automatically:

```bash
# Generate API documentation
cmake --build build --target docs

# Documentation available in docs/api/html/
```

## Enforcement

- **Code Review:** All pull requests must include documentation for new public APIs
- **CI/CD:** Documentation coverage checks can be added to fail builds if public APIs are undocumented
- **Quality Gate:** No code should be merged without proper documentation

## Related Documents

- `project-dashboard/doc/26_API_DOCUMENTATION.md` - Complete API documentation strategy
- `project-dashboard/Doxyfile` - Doxygen configuration
- `project-dashboard/doc/22_CODE_ORGANIZATION.md` - Code organization guidelines
