Z Monitor Bootstrap
====================

This directory contains the Z Monitor executable, organized according to the
Domain-Driven Design (DDD) structure described in the architecture documents.

At this bootstrap stage the project provides:

- A minimal Qt/QML executable (`z-monitor`) with a placeholder UI.
- A DDD-aligned source tree (`src/domain`, `src/application`,
  `src/infrastructure`, `src/interface`).
- Resource structure for QML under `resources/qml`.

Directory layout
----------------

- `src/`
  - `domain/` – Domain layer (aggregates, value objects, domain events,
    repository interfaces).
  - `application/` – Application layer (services, DTOs, orchestration).
  - `infrastructure/` – Infrastructure layer (persistence, network, sensors,
    logging, platform adapters).
  - `interface/` – Interface layer (QObject controllers, QML-related C++).
  - `main.cpp` – Application entry point.
- `resources/`
  - `qml/` – QML entry point and views (`Main.qml`, etc.).
  - `assets/` – Images, icons (placeholder).
  - `i18n/` – Translation files (placeholder).
  - `certs/` – Development certificates for mTLS (placeholder).
- `tests/`
  - `unit/`, `integration/`, `e2e/`, `benchmarks/` – Test suites (placeholders).
- `schema/` – YAML schema definitions and generated DDL/migrations (placeholder).
- `proto/` – Protobuf schema definitions (placeholder).
- `openapi/` – OpenAPI specifications (placeholder).

Build and run
-------------

From the repository root:

```bash
cmake -S . -B build
cmake --build build
./build/z-monitor/z-monitor
```

Expected behavior
-----------------

The application starts a QML-based window sized for 1280x800 and displays a
simple placeholder dashboard message indicating that the DDD structure is in
place. No real device logic, controllers, or services are wired yet.


