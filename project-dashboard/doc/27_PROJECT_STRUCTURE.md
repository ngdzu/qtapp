# Project Structure Reference

This document maps the `qtapp` workspace, highlighting where the executable lives (`z-monitor/`), where the documentation resides (`project-dashboard/`), and how supporting utilities fit together.

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

```
z-monitor/
├── CMakeLists.txt              # Root build file for the device app
├── Dockerfile                  # Multi-stage build for Qt executable
├── README.md                   # App-level instructions
├── src/                        # C++ backend (core services, controllers)
├── resources/                  # QML UI, assets, translations, certs
├── tests/                      # Unit/integration harness
└── ...                         # Additional helpers (configs, data)
```

### 2.1. `src/` Highlights
- `core/` – DeviceSimulator, AlarmManager, NetworkManager, DatabaseManager, PatientManager, SettingsManager, AuthenticationService, LogService, DataArchiver.
- `controllers/` (sometimes named `ui/`) – DashboardController, AlarmController, SystemController, PatientController, SettingsController, TrendsController, NotificationController.
- `interfaces/` & `models/` – Shared data contracts and abstract interfaces.

### 2.2. `resources/`
- `qml/` – `Main.qml`, `components/` (Sidebar, TopBar, StatCard, etc.), `views/` (DashboardView, DiagnosticsView, TrendsView, SettingsView, LoginView).
- `assets/`, `i18n/`, `certs/` – Icons, translations, mTLS material.

#### 2.2.1 Detailed Tree (based on `prompt-dashboard.md`)

```text
z-monitor/
├── CMakeLists.txt
├── Dockerfile
├── README.md
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── DeviceSimulator.cpp/h
│   │   ├── AlarmManager.cpp/h
│   │   ├── NetworkManager.cpp/h
│   │   ├── DatabaseManager.cpp/h
│   │   ├── PatientManager.cpp/h
│   │   ├── SettingsManager.cpp/h
│   │   ├── AuthenticationService.cpp/h
│   │   ├── LogService.cpp/h
│   │   └── DataArchiver.cpp/h
│   └── controllers/
│       ├── DashboardController.cpp/h
│       ├── AlarmController.cpp/h
│       ├── SystemController.cpp/h
│       ├── PatientController.cpp/h
│       ├── SettingsController.cpp/h
│       ├── TrendsController.cpp/h
│       └── NotificationController.cpp/h
├── resources/
│   ├── qml/
│   │   ├── Main.qml
│   │   ├── components/
│   │   │   ├── Sidebar.qml
│   │   │   ├── TopBar.qml
│   │   │   ├── StatCard.qml
│   │   │   ├── SparkLine.qml
│   │   │   ├── PatientBanner.qml
│   │   │   ├── AlarmIndicator.qml
│   │   │   └── NotificationBell.qml
│   │   └── views/
│   │       ├── DashboardView.qml
│   │       ├── DiagnosticsView.qml
│   │       ├── TrendsView.qml
│   │       ├── SettingsView.qml
│   │       └── LoginView.qml
│   ├── assets/
│   ├── i18n/
│   └── certs/
├── tests/
└── (additional helpers)
```

---

## 3. `project-dashboard/` (Documentation, Prompts, Automation)

Although the executable runs from `z-monitor/`, most written guidance stays in `project-dashboard/`.

| Directory | Purpose |
| --- | --- |
| `doc/` | Architecture, security, workflow docs (`01_OVERVIEW.md` … `27_PROJECT_STRUCTURE.md`). |
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

- **Developer Setup:** `doc/07_SETUP_GUIDE.md` (includes quick tree + tooling steps).
- **Architecture:** `doc/02_ARCHITECTURE.md` + Mermaid diagrams.
- **Testing:** `doc/18_TESTING_WORKFLOW.md` and `scripts/run_tests.sh`.
- **ZTODO:** `ZTODO.md` (documentation + verification requirements).
- **Rules:** `.cursor/rules/*.mdc` (auto-applied coding/documentation policies).
- **DDD Guidance:** `doc/28_DOMAIN_DRIVEN_DESIGN.md`.

Use this document with the setup guide and architecture specs to stay oriented. When layout changes, update this reference and the related docs accordingly.

