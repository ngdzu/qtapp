---
owner: TBD Team
reviewers:
  - Process stakeholders
last_reviewed: 2025-12-03
next_review: 2026-03-01
diagram_files:
  - diagrams/DOC-PROC-016_workflow.mmd
  - diagrams/DOC-PROC-016_workflow.svg
doc_id: DOC-PROC-016
title: Testing Workflow
version: 1.1
category: Processes
status: Approved
related_docs:
  - DOC-ARCH-011  # Thread Model
  - DOC-GUIDE-011 # Error Handling
  - DOC-GUIDE-012 # Logging Strategy
  - DOC-COMP-026  # Data Caching Component
  - DOC-PROC-007  # Testing Workflow and Strategy
tags:
  - testing
  - unit-tests
  - integration-tests
  - qml-tests
  - qt-quick-test
  - ci
source:
  original_id: DESIGN-018
  file: 18_TESTING_WORKFLOW.md
  migrated_date: 2025-12-01
  updated_date: 2025-12-03
---

# DOC-PROC-016: Testing Workflow

## 2. Process Flow

![Process Flowchart](diagrams/DOC-PROC-016_workflow.svg)

## 3. Steps

### 3.1 Step 1: {Step Name}

**Responsible:** {Role}

**Prerequisites:**
- {Prerequisite}

**Actions:**
1. {Action}

**Outputs:**
- {Output}

**Success Criteria:**
- {Criterion}


## 2. Process Flow

![Process Flowchart](diagrams/DOC-PROC-016_workflow.svg)

## 3. Steps

### 3.1 Step 1: {Step Name}

**Responsible:** {Role}

**Prerequisites:**
- {Prerequisite}

**Actions:**
1. {Action}

**Outputs:**
- {Output}

**Success Criteria:**
- {Criterion}


## Purpose

Define the testing strategy, types, environments, and CI integration for Z Monitor.

## Test Types

- Unit tests: Fast, isolated, per class/module
- Integration tests: Repository + Database, Network + DTO serialization
- System tests: End-to-end device simulation and workflows
- Performance tests: Benchmarks for caching and UI rendering

## Environments

- Local dev: macOS with Qt Test framework
- CI: Docker-based builds, headless Qt tests
- Hardware-in-the-loop (optional): Sensor simulator and device provisioning

## Example Test: Query Registry

```cpp
TEST_F(QueryRegistryTest, PatientFindByMrnPreparesAndExecutes) {
    DatabaseManager db;
    auto q = db.prepareQuery(QueryId::Patient::FIND_BY_MRN);
    ASSERT_TRUE(q.isValid());
    q.bindValue(":mrn", "TEST123");
    EXPECT_TRUE(q.exec());
}
```

## Example Test: DTO Validation

```cpp
TEST(DTOTest, AdmitPatientCommandValidation) {
    AdmitPatientCommand cmd{.mrn = "", .name = "A", .bedLocation = "ICU-5A", .admissionSource = "manual"};
    EXPECT_FALSE(cmd.isValid());
}
```

## QML Component Testing

### Overview

QML components are tested using Qt Quick Test framework with a template-based test main generation pattern to avoid CMake string escaping issues and ensure proper QML import path configuration.

### Test Infrastructure

**Location:** `tests/qml/`

**Structure:**
```
tests/qml/
  ├── CMakeLists.txt           # Test infrastructure setup
  ├── test_main.in.cpp         # Template for generating test mains
  ├── components/              # Component tests
  │   ├── tst_AlarmPanelTest.qml
  │   ├── tst_TrendPanelTest.qml
  │   └── tst_VitalTileTest.qml
  └── views/                   # View tests
      ├── tst_PatientBannerTest.qml
      ├── tst_TrendsViewTest.qml
      └── tst_WaveformDisplayTest.qml
```

### Test Main Template Pattern

**File:** `tests/qml/test_main.in.cpp`

Each test gets a unique main.cpp generated from the template using CMake's `configure_file()`. This approach:
- Avoids CMake string escaping nightmares with inline `file(WRITE)`
- Ensures consistent QML import path configuration across all tests
- Uses `QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath)` for Qt plugin discovery
- Creates `QApplication` instead of `QGuiApplication` (required for QtCharts)

**Template structure:**
```cpp
#include <QtQuickTest>
#include <QApplication>
#include <QLibraryInfo>
#include <QProcessEnvironment>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    
    // Configure QML import path for Qt plugins
    QString importPath = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    QString currentPath = qEnvironmentVariable("QML2_IMPORT_PATH");
    QString combinedPath = currentPath.isEmpty() 
        ? importPath 
        : currentPath + ":" + importPath;
    qputenv("QML2_IMPORT_PATH", combinedPath.toUtf8());
    
    return quick_test_main(argc, argv, "@TEST_NAME@", "@SOURCE_DIR@");
}
```

### CMake Configuration

**Function:** `add_qml_test(TEST_NAME QML_FILE)`

```cmake
function(add_qml_test TEST_NAME QML_FILE)
    # Generate unique test main from template
    set(TEST_MAIN "${CMAKE_CURRENT_BINARY_DIR}/test_main_${TEST_NAME}.cpp")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/test_main.in.cpp"
        "${TEST_MAIN}"
        @ONLY
    )
    
    # Create test executable
    add_executable(${TEST_NAME} ${TEST_MAIN})
    target_link_libraries(${TEST_NAME} 
        PRIVATE 
        Qt6::Quick 
        Qt6::QuickTest
        Qt6::Widgets  # Required for QtCharts
        Qt6::Charts   # Required for QML Charts
    )
    
    # Register with CTest
    add_test(NAME ${TEST_NAME} 
             COMMAND ${TEST_NAME} -input ${QML_FILE})
endfunction()
```

### Writing QML Tests

**Window Root Pattern:**

Qt Quick Test components need a proper Window root for visibility and sizing assertions:

```qml
import QtQuick
import QtQuick.Window
import QtTest

TestCase {
    id: testCase
    name: "TrendsViewTest"
    
    Window {
        id: testWin
        visible: true
        width: 1024
        height: 768
        
        TrendsView {
            id: trendsView
            anchors.fill: parent
        }
    }
    
    function test_viewRendersWithoutCrash() {
        verify(testWin.visible && trendsView.visible, "View visible")
        testWin.width = 1280
        verify(trendsView.width > 0, "Responsive resize")
    }
}
```

**Context Property Guards:**

When components depend on context properties that may not exist in tests, guard all access:

```qml
Component.onCompleted: {
    if (typeof trendsController !== 'undefined') {
        trendsController.setStartTime(startTime)
        trendsController.setEndTime(endTime)
    }
}

onClicked: {
    if (typeof trendsController !== 'undefined') {
        trendsController.setRange(hours)
    }
}
```

### Running QML Tests

**Local execution:**
```bash
cd build
ctest -R "QML_" --output-on-failure
```

**Individual test:**
```bash
./tests/qml/TrendsViewTest -input tests/qml/views/tst_TrendsViewTest.qml
```

**Expected output:**
```
100% tests passed, 0 tests failed out of 6
Total Test time (real) = 5.22 sec
```

### Test Labels

QML tests are tagged with multiple labels for filtering:
- `qml` - All QML tests
- `ui` - User interface tests
- `component` - Reusable component tests
- `view` - Application view tests
- Specific component labels: `alarm`, `patient`, `trend`, `vital`, `waveform`

**Filter by label:**
```bash
ctest -L qml
ctest -L "component"
ctest -L "waveform"
```

### Common Patterns

**Test initial state:**
```qml
function test_initialState() {
    verify(component.visible)
    compare(component.width, 800)
    compare(component.property, expectedValue)
}
```

**Test property changes:**
```qml
function test_propertyUpdate() {
    component.property = newValue
    compare(component.property, newValue)
    verify(component.internalState === "updated")
}
```

**Test user interaction:**
```qml
function test_buttonClick() {
    mouseClick(button, button.width/2, button.height/2)
    verify(button.clicked)
    compare(component.state, "after-click")
}
```

**Test responsive resize:**
```qml
function test_responsiveResize() {
    testWin.width = 1280
    testWin.height = 1024
    verify(component.width > 0, "Component resized")
}
```

### QtCharts Requirements

Components using QtCharts QML module require:
1. **QApplication** instead of QGuiApplication (QGraphicsItem infrastructure)
2. **Qt6::Widgets** and **Qt6::Charts** linkage in test executable
3. **QML2_IMPORT_PATH** configured to Qt's QML plugin directory

These are handled automatically by the test infrastructure.

### Troubleshooting

**Problem:** "module 'QtCharts' is not installed"
- **Solution:** Verify `QML2_IMPORT_PATH` is set in test main template
- **Verification:** Check `QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath)` returns valid path

**Problem:** Test crashes on component creation
- **Solution:** Add context property existence guards (`typeof x !== 'undefined'`)
- **Verification:** Run test with `--output-on-failure` to see detailed errors

**Problem:** Visibility assertions fail
- **Solution:** Wrap component in `Window` root instead of `Item`
- **Verification:** Check `testWin.visible && component.visible`

**Problem:** CMake generation errors with QML2_IMPORT_PATH
- **Solution:** Use template file with `configure_file()` instead of inline `file(WRITE)`
- **Verification:** Check generated `test_main_*.cpp` files in build directory

## CI Integration

- Build matrix: macOS + Linux
- Targets: `cmake --build build` then run `ctest`
- QML tests: `ctest -R "QML_"` or `ctest -L qml`
- Artifacts: Test reports (XML), coverage (gcov/lcov)

## Verification Checklist

- All unit tests pass locally and in CI
- All QML component tests pass (6/6 tests)
- Integration tests cover DB, network, and key workflows
- Performance thresholds tracked and enforced
- Logs capture failures with clear error contexts
- QML tests use Window root pattern for proper windowing
- Context properties are guarded for test isolation

## Revision History

| Version | Date       | Changes                                                                                                                                                                     |
| ------- | ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1.1     | 2025-12-03 | Added comprehensive QML testing guidelines: template-based test main generation, QML import path configuration, Window root pattern, QtCharts requirements, troubleshooting |
| 1.0     | 2025-12-01 | Initial migration from DESIGN-018 to DOC-PROC-016. Comprehensive testing workflow and CI integration.                                                                       |
