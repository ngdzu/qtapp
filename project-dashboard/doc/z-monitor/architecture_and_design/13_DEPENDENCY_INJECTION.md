# Dependency Injection Options for Qt/C++ Z Monitor

This document evaluates dependency-injection (DI) frameworks and patterns for the Qt/C++ Z Monitor project and recommends an approach.

Context and goals
- Medium-sized Qt6/C++ project with many services (DatabaseManager, NetworkManager, AlarmManager, DataArchiver, SignalProcessor, AnalyticsPool, UI Controllers).
- Goals: manage lifetime and ownership, make testing easier (mocking), support configuration/time-of-day instantiation, avoid global singletons, and keep startup wiring maintainable.
- Constraints: C++17, Qt6, Docker build environments, small embedded targets (1-4 cores) and desktop targets.

Should we use DI?
- Short answer: yes — DI improves testability, explicit dependencies, and flexible wiring. But in C++ and Qt, DI can be introduced incrementally; heavy runtime reflection-based DI is unnecessary.

Frameworks and approaches
1) No-framework / Manual DI (constructor injection + factory functions)
- Description: Pass dependencies explicitly to constructors. Use factory functions or builder classes to assemble object graphs.
- Pros:
  - Simple, zero runtime dependency and zero extra libraries.
  - Compile-time clarity of dependencies and easy to debug.
  - Small binary size and predictable behavior on embedded targets.
- Cons:
  - Boilerplate in wiring at application startup; large graph can be tedious.
  - No automatic lifetime management; requires careful ownership decisions.
- When to use: Small-to-medium projects where explicit wiring is acceptable and you want minimal runtime overhead.

2) Service Locator / Global Registry
- Description: Central registry that provides access to services by interface or key.
- Pros:
  - Reduces constructor parameter lists; convenient to access services from anywhere.
  - Easy incremental adoption: start with a global registry and later refactor to constructor DI.
- Cons:
  - Hides dependencies (less explicit), makes testing harder (global state), coupling to registry.
  - Considered an anti-pattern for large systems where testability and explicitness matter.
- When to use: Transitional or for rare cross-cutting services (config, logger), but avoid for core services.

3) Boost.DI
- Description: A modern, header-only compile-time DI library with strong C++ support.
- Pros:
  - Zero runtime overhead, uses templates and constexpr to wire dependencies at compile time.
  - Great for testability and no RTTI required.
  - Flexible binding and scopes (singleton, unique, etc.).
- Cons:
  - Template-heavy API can lead to complex error messages and longer compile times.
  - Larger template code can bloat compile times on large projects.
- When to use: Projects that value compile-time guarantees and can accept template complexity. Works well for embedded/desktop C++ projects.

4) Fruit (C++ DI)
- Description: Another compile-time DI container for C++.
- Pros:
  - Focus on performance and compile-time wiring.
  - Good scoping and module support.
- Cons:
  - Less active community than Boost.DI.
  - Template complexity similar to Boost.DI.
- When to use: When you need a compile-time DI container and prefer Fruit's API or semantics.

5) Poco/Cpp Micro Services + Simple DI
- Description: Use a lightweight component framework (POCO, or OSGI-like) with service registration and lookup.
- Pros:
  - Built-in lifecycle, component registration and discovery.
  - Useful if you plan dynamic loadable modules/plugins.
- Cons:
  - Larger dependency footprint; more than you need for simple DI.
  - Complexity and additional runtime behavior.
- When to use: If you plan plugin-based architecture with runtime discovery.

6) Qt-specific patterns (QObjects + parent ownership + qobject_cast)
- Description: Use QObject ownership trees, `QObject::parent`, and signals/slots for decoupling.
- Pros:
  - Integrates naturally with Qt's memory management and event loop.
  - Minimal extra code; `QObject` parent/child model handles lifetime.
- Cons:
  - Not a DI framework; dependencies may still be implicit via parent or global pointers.
  - Passing raw `QObject*` and using `qobject_cast` loses interface clarity and type-safety.
- When to use: For UI objects and widgets where QObject lifetimes make sense; still combine with constructor injection for backend services.

7) Koin-like / Lightweight runtime DI (hand-rolled)
- Description: Implement a small runtime container with explicit registration/binding and retrieval functions.
- Pros:
  - Control over features; can be minimal and tailored for testability.
  - Less template complexity than Boost.DI.
- Cons:
  - Requires maintenance and careful design to avoid service locator anti-pattern.
- When to use: When you want some automation for wiring without full template DI complexity.

8) Google Fruit or Dagger-style (not common in C++)
- Description: Dagger-style code generation DI is not mainstream in C++ but could be simulated via code-generation.
- Pros: Clear, generated wiring; minimal runtime overhead.
- Cons: Requires build-time code generation tooling; more complex setup.

Decision criteria
- Testability: Must support easy mocking and unit testing (constructor injection helps).
- Runtime overhead: Embedded targets demand small runtime and binary size — prefer compile-time or manual DI.
- Compile-time cost: Template-heavy DI increases compile times; balance with team tolerance.
- Lifetime management: Prefer approaches that don't rely on global singletons for core services.
- Integration with QObject: UI controllers and QObject-based classes should keep QObject parent/child ownership; backend services (non-QObject) can be simple POJOs with DI.
- Complexity: Avoid adding a large framework unless you need dynamic features or plugin discovery.

Recommendation for this project
- Primary approach: "Manual DI + lightweight runtime container"
  - Use constructor injection for core services (DatabaseManager, SignalProcessor, AlarmManager, NetworkManager). Keep interfaces abstract (pure virtual) for easy mocking.
  - Create a small bootstrapping module or `AppContainer` (hand-written) responsible for wiring and lifetime: it returns unique_ptr/shared_ptr for services. Keep this code localized to `main()` and a small `bootstrap` file.
  - For cross-cutting services (logger, config), a small read-only service locator is acceptable if used sparingly.
  - For QObject-based UI controllers, continue using parent/child ownership; expose interfaces to UI via `Q_PROPERTY` and controller objects created by the bootstrapper.

- Optional: Evaluate `Boost.DI` if the team wants compile-time DI and can accept template complexity. Do a spike: wire a few services and measure compile times and developer ergonomics.

Conformance checklist
- [ ] Define pure interfaces for each core service.
- [ ] Implement concrete service classes with minimal side-effects in constructors.
- [ ] Implement `AppContainer` that wires services and enforces lifetime.
- [ ] Use `std::unique_ptr`/`std::shared_ptr` and `std::weak_ptr` where needed; prefer `unique_ptr` for ownership clarity.
- [ ] Add unit tests that instantiate services with test doubles.
- [ ] Document the bootstrap flow in `README` or `doc/bootstrap.md`.

Appendix: Example wiring sketch (manual)

```cpp
struct IDatabase {
    virtual ~IDatabase() = default;
    virtual void writeRecord(...) = 0;
};

class SqliteDatabase : public IDatabase {
public:
    SqliteDatabase(const DbConfig& cfg);
    void writeRecord(...) override;
};

struct AppContainer {
    std::unique_ptr<IDatabase> db;
    std::unique_ptr<INetwork> network;
    std::unique_ptr<ISignalProcessor> processor;

    static AppContainer create(const AppConfig& cfg) {
        AppContainer c;
        c.db = std::make_unique<SqliteDatabase>(cfg.db);
        c.network = std::make_unique<NetworkManager>(cfg.net);
        c.processor = std::make_unique<SignalProcessor>(...);
        return c;
    }
};
```

---

Generated: 2025-11-23
