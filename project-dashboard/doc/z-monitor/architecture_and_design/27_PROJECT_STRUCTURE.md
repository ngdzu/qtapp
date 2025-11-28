# Project Structure Reference

**Document ID:** DESIGN-027  
**Version:** 2.1  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document provides a **workspace-level map** of the `qtapp` repository, highlighting where the executable lives (`z-monitor/`), where the documentation resides (`project-dashboard/`), and how supporting utilities fit together.

> **📋 Related Documents:**
> - [Code Organization (22_CODE_ORGANIZATION.md)](./22_CODE_ORGANIZATION.md) - **Detailed code structure and organization rules** ⭐ (see this for src/ directory details)
> - [Architecture (02_ARCHITECTURE.md)](./02_ARCHITECTURE.md) - High-level architecture and DDD layer structure
> - [System Components & DDD Strategy (29_SYSTEM_COMPONENTS.md)](./29_SYSTEM_COMPONENTS.md) - DDD strategy and component inventory

---

## 1. Repository Overview (`/Users/dustinwind/Development/Qt/qtapp`)

| Path | Description |
| --- | --- |
| `.cursor/` | Cursor-specific configuration (rules, caches). |
| `.github/` | GitHub Actions workflows (builds, Doxygen, etc.). |
| `z-monitor/` | **Primary Z Monitor application** (Qt/C++, QML, tests). |
| `project-dashboard/` | Documentation, prompts, automation scripts, ZTODO. |
| `sensor-simulator/` | Qt/QML simulator that streams vitals over WebSocket. |
| `qt-style-telemetry-simulator/`, `vibelink-remote-builder/` | UI prototypes / experiments. |
| `prompt-dashboard.md`, `prompt.md`, `docker_useful_commands.md` | Prompt and operations references. |
| Other files (`docker-compose.simulator.yml`, etc.) | Environment helpers. |

---

## 2. `z-monitor/` Layout (Primary Application)

The Z Monitor application follows Domain-Driven Design (DDD) principles with a layered architecture.

**High-Level Structure:**
```
z-monitor/
├── CMakeLists.txt              # Root build file for the device app
├── Dockerfile                  # Multi-stage build for Qt executable
├── README.md                   # App-level instructions
├── src/                        # C++ source code (DDD layers)
│   ├── domain/                 # Domain layer (pure business logic)
│   ├── application/            # Application layer (use cases)
│   ├── infrastructure/         # Infrastructure layer (adapters)
│   ├── interface/              # Interface layer (UI controllers)
│   └── main.cpp                # Application entry point
├── resources/                  # Resources (QML, assets, translations, certs)
│   ├── qml/                    # QML UI files
│   ├── assets/                 # Images, icons
│   ├── i18n/                   # Translations
│   └── certs/                  # Certificates (mTLS)
├── tests/                      # Test code
│   ├── unit/                   # Unit tests
│   ├── integration/            # Integration tests
│   └── e2e/                    # End-to-end tests
└── ...                         # Additional helpers (configs, data)
```

> **📋 For Detailed Code Structure:** See [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) for:
> - Complete `src/` directory tree with all files
> - Layer boundaries and dependencies
> - Namespace conventions
> - File naming conventions
> - Code organization rules

**Layer Overview:**
- **Domain Layer** (`src/domain/`) – Pure business logic, aggregates, value objects, domain events, repository interfaces
- **Application Layer** (`src/application/`) – Use-case orchestration, application services, DTOs
- **Infrastructure Layer** (`src/infrastructure/`) – Technical implementations (persistence, network, sensors, caching, security, Qt adapters, system services, utilities)
  - Includes `utils/` subdirectory for shared utility classes (ObjectPool, LockFreeQueue, LogBuffer, etc.) - see [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) Section 12
- **Interface Layer** (`src/interface/`) – UI integration (QML controllers and QML UI)

See [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) Section 2 for the complete directory structure and [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) Section 8 for dependency rules.

---

## 3. `project-dashboard/` (Documentation, Prompts, Automation)

Although the executable runs from `z-monitor/`, most written guidance stays in `project-dashboard/`.

| Directory | Purpose |
| --- | --- |
| `doc/z-monitor/architecture_and_design/` | Z Monitor architecture, security, workflow docs. |
| `doc/foundation/` | General software engineering knowledge (74 foundational topics organized in 14 categories). |
| `prompt/` | Reusable prompt files for scoped tasks (e.g., certificate provisioning, CI setup). |
| `scripts/` | Tooling (`run_tests.sh`, Mermaid generators, Doxygen helpers, screenshots). |
| `ZTODO.md` | Master task list with documentation + verification requirements. |
| `README.md` | Links to core docs and workflows. |

Use this tree when you need specifications, design references, or automation scripts.

---

## 4. Supporting Projects

| Path | Purpose |
| --- | --- |
| `sensor-simulator/` | Qt/QML simulator emitting vitals/alarms over WebSocket (consumed by adapters). |
| `qt-style-telemetry-simulator/`, `vibelink-remote-builder/` | Front-end experiments for telemetry dashboards. |

---

## 5. Prompts & Operational Docs

- `prompt-dashboard.md` – Master cheat sheet for tackling tasks (outside `project-dashboard/`).
- `prompt.md` – High-level instructions for AI-assisted work.
- `docker_useful_commands.md` – Handy Docker command reference.
- These live at repo root and complement the `project-dashboard/prompt/` files.

---

## 6. GitHub Workflows & Automation

- Workflows live in `.github/workflows/` (e.g., `doxygen-docs.yml` for nightly API docs).
- Additional workflows (build/test, releases) should follow guidance in `prompt/19-ci-workflows-build-tests.md`.

---

## 7. Cross-References

- **Code Organization:** [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md) - **Detailed code structure, conventions, and organization rules** ⭐ (use this for src/ directory details)
- **Memory Management:** [23_MEMORY_RESOURCE_MANAGEMENT.md](./23_MEMORY_RESOURCE_MANAGEMENT.md) - Memory management patterns, utility classes (ObjectPool, LockFreeQueue, LogBuffer), and resource lifecycle
- **Architecture:** [02_ARCHITECTURE.md](./02_ARCHITECTURE.md) - High-level architecture and DDD layer structure
- **DDD Guidance:** [29_SYSTEM_COMPONENTS.md](./29_SYSTEM_COMPONENTS.md) - DDD strategy and component inventory
- **Developer Setup:** `07_SETUP_GUIDE.md` (includes quick tree + tooling steps).
- **Testing:** `18_TESTING_WORKFLOW.md` and `scripts/run_tests.sh`.
- **ZTODO:** `ZTODO.md` (documentation + verification requirements).
- **Rules:** `.cursor/rules/*.mdc` (auto-applied coding/documentation policies).
- **Foundation Knowledge:** `../../foundation/00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` (74 foundational topics).

---

**Document Purpose:** This document provides a **workspace-level map** (where things are in the repository). For detailed code organization rules and conventions, see [22_CODE_ORGANIZATION.md](./22_CODE_ORGANIZATION.md).

