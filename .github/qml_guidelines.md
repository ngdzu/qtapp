---
appliesTo:
  - "**/*.qml"
  - "**/*.qmltypes"
  - "project-dashboard/scripts/capture_screenshot.sh"
  - "project-dashboard"
---
# QML Coding Guidelines

## Overview

This document defines the QML coding standards and best practices for the Z Monitor project. These guidelines ensure consistent, maintainable, and performant QML code that integrates seamlessly with C++ controllers and follows Qt Quick best practices.

**Key Principles:**
- **Component-Based Design** - Reusable, composable QML components
- **Property Bindings** - Leverage QML's declarative binding system
- **Performance First** - Optimize for 60 FPS rendering and smooth animations
- **C++ Integration** - Use controllers (QObject) to bridge QML and application logic
- **UI/UX Consistency** - Follow design system (Theme, spacing, typography)
- **Accessibility** - Ensure UI is accessible and usable

---

## 1. File Organization and Structure

### Directory Structure

- **Main QML files**: `resources/qml/` or `src/interface/qml/`
- **Components**: `resources/qml/components/` (reusable components)
- **Views**: `resources/qml/views/` (main application views)
- **Theme**: `resources/qml/Theme.qml` (singleton for design system)
- **Icons**: `resources/qml/icons/` (SVG icons)

### File Naming

- **Use PascalCase** for QML files (e.g., `PatientBanner.qml`, `WaveformChart.qml`)
- **Component files** should match component name exactly
- **View files** should end with `View` (e.g., `DashboardView.qml`, `SettingsView.qml`)

### QML File Structure

```qml
// ✅ Good: Proper QML file structure
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import "../components"
import "../Theme"

Rectangle {
    id: root  // Always name root element 'root'
    
    // Properties (grouped logically)
    property string title: ""
    property bool isActive: false
    
    // Signals
    signal clicked()
    signal valueChanged(real value)
    
    // Component implementation
    // ...
}
```

### Import Organization

- **Order imports consistently**:
  1. Qt Quick modules (QtQuick, QtQuick.Controls, etc.)
  2. Qt Quick Layouts
  3. Local components (`import "../components"`)
  4. Theme/singletons (`import "../Theme"`)
  5. C++ types (`import qml 1.0`)

---

## 2. Component Design

### Component Reusability

- **Create reusable components** for repeated UI patterns:
  ```qml
  // ✅ Good: Reusable component
  // components/VitalCard.qml
  Rectangle {
      id: root
      property string label: ""
      property real value: 0
      property string unit: ""
      property color accentColor: Theme.green
      
      // Component implementation
  }
  
  // Usage
  VitalCard {
      label: "Heart Rate"
      value: controller.heartRate
      unit: "bpm"
      accentColor: Theme.green
  }
  ```

### Component Properties

- **Use properties for customization**: Expose properties that allow component reuse
- **Provide sensible defaults**: All properties should have default values
- **Document properties**: Use comments to explain property purpose
  ```qml
  // ✅ Good: Well-documented properties
  Rectangle {
      id: root
      
      // Title text displayed at the top of the card
      property string title: ""
      
      // Whether the card is in an active/selected state
      property bool isActive: false
      
      // Background color (defaults to theme card background)
      property color backgroundColor: Theme.cardBackground
  }
  ```

### Component IDs

- **Always name root element `root`**: Makes component self-documenting
- **Use descriptive IDs** for child elements when needed:
  ```qml
  // ✅ Good: Clear IDs
  Rectangle {
      id: root
      
      Column {
          id: contentColumn
          // ...
      }
      
      Button {
          id: actionButton
          // ...
      }
  }
  ```

---

## 3. Property Bindings

### Use Bindings, Not Assignments

- **Prefer property bindings** over JavaScript assignments:
  ```qml
  // ✅ Good: Property binding (reactive)
  Text {
      text: controller.patientName
      color: controller.isAdmitted ? Theme.green : Theme.textMuted
  }
  
  // ❌ Bad: JavaScript assignment (not reactive)
  Text {
      Component.onCompleted: {
          text = controller.patientName  // Won't update when controller changes
      }
  }
  ```

### Binding Performance

- **Avoid expensive bindings in hot paths**: Cache complex calculations
  ```qml
  // ❌ Bad: Expensive calculation in binding
  Text {
      text: someList.map(item => item.value).reduce((a, b) => a + b, 0)
  }
  
  // ✅ Good: Cache expensive calculation
  property real cachedSum: {
      var sum = 0
      for (var i = 0; i < someList.length; i++) {
          sum += someList[i].value
      }
      return sum
  }
  Text {
      text: cachedSum
  }
  ```

### Binding Dependencies

- **Minimize binding dependencies**: Only bind to what you need
- **Use `Qt.binding()`** when you need to create bindings programmatically:
  ```qml
  // ✅ Good: Explicit binding creation
  property bool isConnected: false
  
  Component.onCompleted: {
      isConnected = Qt.binding(function() { return networkController.status === "connected" })
  }
  ```

---

## 4. Signals and Slots

### Signal Definitions

- **Define signals clearly**: Use descriptive signal names and parameters
  ```qml
  // ✅ Good: Clear signal definition
  signal patientSelected(string mrn, string name)
  signal alarmAcknowledged(int alarmId, string userId)
  
  // Usage
  onPatientSelected: function(mrn, name) {
      console.log("Selected patient:", mrn, name)
  }
  ```

### Signal Connections

- **Use signal handlers** for simple connections:
  ```qml
  // ✅ Good: Signal handler
  Button {
      onClicked: {
          controller.admitPatient(mrn)
      }
  }
  ```

- **Use `Connections`** for connecting to external objects:
  ```qml
  // ✅ Good: Connections for external objects
  Connections {
      target: controller
      function onPatientAdmitted(mrn) {
          showNotification("Patient admitted: " + mrn)
      }
  }
  ```

### C++ Signal Integration

- **Connect C++ signals to QML**: Use `Q_PROPERTY` and signals in controllers
  ```cpp
  // C++ Controller
  class PatientController : public QObject {
      Q_OBJECT
      Q_PROPERTY(QString patientMrn READ patientMrn NOTIFY patientMrnChanged)
  signals:
      void patientAdmitted(const QString& mrn);
  };
  ```

  ```qml
  // QML Usage
  Connections {
      target: patientController
      function onPatientAdmitted(mrn) {
          // Handle patient admission
      }
  }
  ```

---

## 5. Layout and Positioning

### Use Layouts, Not Anchors (When Possible)

- **Prefer Layouts** for flexible, responsive layouts:
  ```qml
  // ✅ Good: Using Layouts
  ColumnLayout {
      anchors.fill: parent
      spacing: Theme.spacingMd
      
      Text {
          Layout.fillWidth: true
          text: "Title"
      }
      
      Button {
          Layout.alignment: Qt.AlignHCenter
          text: "Action"
      }
  }
  ```

- **Use Anchors** for fixed positioning or when Layouts are insufficient:
  ```qml
  // ✅ Good: Anchors for fixed positioning
  Rectangle {
      anchors.fill: parent
      anchors.margins: Theme.spacingLg
  }
  ```

### Layout Best Practices

- **Use appropriate layout types**:
  - `ColumnLayout` / `RowLayout` for linear arrangements
  - `GridLayout` for grid arrangements
  - `StackLayout` for stacked views (tabs, pages)
  - `Flow` for wrapping content

- **Set spacing consistently**: Use Theme spacing constants
  ```qml
  ColumnLayout {
      spacing: Theme.spacingMd  // ✅ Good: Use theme constant
      // spacing: 10  // ❌ Bad: Hardcoded value
  }
  ```

---

## 6. Performance Optimization

### Avoid Creating Objects in Loops

- **Use `Repeater`** or `ListView`** for dynamic lists:
  ```qml
  // ✅ Good: Using Repeater
  Row {
      Repeater {
          model: vitalSigns
          VitalCard {
              label: modelData.label
              value: modelData.value
          }
      }
  }
  
  // ❌ Bad: Creating objects in JavaScript
  Component.onCompleted: {
      for (var i = 0; i < vitalSigns.length; i++) {
          var card = Qt.createQmlObject("...", parent)  // Expensive!
      }
  }
  ```

### Use Loader for Conditional Components

- **Use `Loader`** to defer component creation:
  ```qml
  // ✅ Good: Lazy loading with Loader
  Loader {
      id: modalLoader
      active: showModal
      sourceComponent: AdmissionModal {}
  }
  ```

### Optimize ListViews

- **Use `ListView`** with proper `delegate` and `model`:
  ```qml
  // ✅ Good: Efficient ListView
  ListView {
      model: alarmModel
      delegate: AlarmItem {
          alarmId: model.id
          message: model.message
      }
      cacheBuffer: 100  // Cache items outside visible area
  }
  ```

### Avoid Unnecessary Repaints

- **Use `opacity`** instead of `visible` when animating:
  ```qml
  // ✅ Good: Opacity for smooth transitions
  Rectangle {
      opacity: isVisible ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 200 } }
  }
  
  // ❌ Bad: Visible causes layout recalculation
  Rectangle {
      visible: isVisible  // Causes layout changes
  }
  ```

---

## 7. Styling and Theme

### Use Theme Singleton

- **Always use Theme singleton** for colors, spacing, typography:
  ```qml
  // ✅ Good: Using Theme
  Rectangle {
      color: Theme.cardBackground
      border.color: Theme.border
      radius: Theme.radiusMd
  }
  
  Text {
      color: Theme.textPrimary
      font.pixelSize: Theme.fontSizeMd
  }
  
  // ❌ Bad: Hardcoded values
  Rectangle {
      color: "#1a1a1a"  // Hardcoded color
      radius: 8  // Hardcoded radius
  }
  ```

### Theme Constants

- **Access theme properties** consistently:
  ```qml
  // Theme.qml (singleton)
  pragma Singleton
  import QtQuick 2.15
  
  QtObject {
      // Colors
      readonly property color background: "#09090b"
      readonly property color cardBackground: "#18181b"
      readonly property color textPrimary: "#ffffff"
      readonly property color textMuted: "#71717a"
      
      // Spacing
      readonly property int spacingXs: 4
      readonly property int spacingSm: 8
      readonly property int spacingMd: 16
      readonly property int spacingLg: 24
      
      // Typography
      readonly property int fontSizeXs: 10
      readonly property int fontSizeSm: 12
      readonly property int fontSizeMd: 14
      readonly property int fontSizeLg: 18
  }
  ```

### Color Usage

- **Follow color semantics** from UI/UX guide:
  - **Green**: Normal/healthy states, success
  - **Blue**: Informational, pleth waveform
  - **Yellow/Orange**: Warning states, medium priority
  - **Red**: Critical states, errors, high priority alarms
  - **Purple/Cyan**: Special visualizations

---

## 8. Animation and Transitions

### Smooth Animations

- **Target 60 FPS**: Keep animations smooth and performant
- **Use `Behavior`** for property animations:
  ```qml
  // ✅ Good: Smooth property animation
  Rectangle {
      x: isExpanded ? 0 : -200
      Behavior on x {
          NumberAnimation {
              duration: 200
              easing.type: Easing.InOutQuad
          }
      }
  }
  ```

### Transition Best Practices

- **Keep durations short**: 200-300ms for most transitions
- **Use appropriate easing**: `Easing.InOutQuad` for most cases
- **Avoid animating expensive properties**: Prefer `opacity` over `visible`

### State Transitions

- **Use `State` and `Transition`** for complex state changes:
  ```qml
  Rectangle {
      id: root
      states: [
          State {
              name: "expanded"
              PropertyChanges { target: root; width: 200 }
          },
          State {
              name: "collapsed"
              PropertyChanges { target: root; width: 64 }
          }
      ]
      
      transitions: Transition {
          NumberAnimation { properties: "width"; duration: 200 }
      }
  }
  ```

---

## 9. C++ Integration

### Controller Pattern

- **Use C++ controllers** to bridge QML and application logic:
  ```cpp
  // C++ Controller
  class DashboardController : public QObject {
      Q_OBJECT
      Q_PROPERTY(int heartRate READ heartRate NOTIFY heartRateChanged)
  public:
      Q_INVOKABLE void startMonitoring();
  signals:
      void heartRateChanged();
  };
  ```

  ```qml
  // QML Usage
  Connections {
      target: dashboardController
      function onHeartRateChanged() {
          vitalCard.value = dashboardController.heartRate
      }
  }
  
  Button {
      onClicked: dashboardController.startMonitoring()
  }
  ```

### Q_PROPERTY Best Practices

- **Expose data via Q_PROPERTY**: Use `NOTIFY` signals for reactivity
- **Use Q_INVOKABLE** for methods callable from QML
- **Keep controllers lightweight**: Delegate business logic to application services

### Context Properties

- **Register controllers** in `main.cpp`:
  ```cpp
  engine.rootContext()->setContextProperty("dashboardController", &dashboardController);
  ```

  ```qml
  // Access in QML
  Text {
      text: dashboardController.patientName
  }
  ```

---

## 10. Error Handling and Validation

### Input Validation

- **Validate user input** before processing:
  ```qml
  TextField {
      id: mrnInput
      validator: RegExpValidator {
          regExp: /^[A-Z0-9-]+$/
      }
      
      onAccepted: {
          if (mrnInput.acceptableInput) {
              controller.lookupPatient(mrnInput.text)
          } else {
              showError("Invalid MRN format")
          }
      }
  }
  ```

### Error Display

- **Show errors clearly** to users:
  ```qml
  // ✅ Good: Clear error display
  Rectangle {
      visible: controller.hasError
      color: Theme.errorBackground
      Text {
          text: controller.errorMessage
          color: Theme.errorText
      }
  }
  ```

---

## 11. Accessibility

### Accessibility Properties

- **Set accessibility properties** for screen readers:
  ```qml
  Button {
      text: "Admit Patient"
      Accessible.name: "Admit Patient Button"
      Accessible.description: "Opens the patient admission dialog"
  }
  ```

### Touch Targets

- **Ensure adequate touch target size**: Minimum 44x44 pixels for touch interfaces
- **Provide visual feedback**: Highlight buttons on press

---

## 12. Testing and Validation

### Visual Validation

- **After editing QML files, validate the UI**:
  - Run screenshot/build script to capture UI state
  - For sensor simulator: `project-dashboard/scripts/capture_screenshot.sh`
  - Compare screenshots to detect visual regressions

### Screenshot Workflow

- **Screenshots are stored in git** to track UI evolution
- **Future**: Automated pixel comparison tests will validate UI consistency
- **Current**: Manual comparison of screenshots after changes

### QML Linting

- **Use `qmlformat`** to format QML files consistently:
  ```bash
  qmlformat -i MyComponent.qml
  ```

---

## 13. Common Patterns

### Modal Dialogs

```qml
// ✅ Good: Modal dialog pattern
Dialog {
    id: admissionDialog
    modal: true
    title: "Admit Patient"
    
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    onAccepted: {
        controller.admitPatient(mrnField.text)
    }
}
```

### Loading States

```qml
// ✅ Good: Loading state pattern
Rectangle {
    BusyIndicator {
        anchors.centerIn: parent
        running: controller.isLoading
        visible: controller.isLoading
    }
    
    Column {
        visible: !controller.isLoading
        // Content
    }
}
```

### Empty States

```qml
// ✅ Good: Empty state pattern
Column {
    anchors.centerIn: parent
    visible: alarmModel.count === 0
    spacing: Theme.spacingMd
    
    Text {
        text: "No active alarms"
        color: Theme.textMuted
    }
}
```

---

## 14. Anti-Patterns to Avoid

### ❌ Don't Use JavaScript for Everything

```qml
// ❌ Bad: JavaScript for reactive updates
Text {
      Component.onCompleted: {
          text = controller.name
      }
  }

// ✅ Good: Property binding
Text {
      text: controller.name
  }
```

### ❌ Don't Create Objects Dynamically

```qml
// ❌ Bad: Dynamic object creation
Component.onCompleted: {
      for (var i = 0; i < 10; i++) {
          var rect = Qt.createQmlObject("Rectangle {}", parent)
      }
  }

// ✅ Good: Use Repeater
Repeater {
      model: 10
      Rectangle {
          // ...
      }
  }
```

### ❌ Don't Hardcode Values

```qml
// ❌ Bad: Hardcoded values
Rectangle {
      color: "#1a1a1a"
      radius: 8
      anchors.margins: 16
  }

// ✅ Good: Use Theme constants
Rectangle {
      color: Theme.cardBackground
      radius: Theme.radiusMd
      anchors.margins: Theme.spacingLg
  }
```

### ❌ Don't Mix Business Logic in QML

```qml
// ❌ Bad: Business logic in QML
Button {
      onClicked: {
          // Complex business logic here
          var patient = lookupPatient(mrn)
          if (patient.age > 65) {
              // ...
          }
      }
  }

// ✅ Good: Delegate to controller
Button {
      onClicked: controller.admitPatient(mrn)
  }
```

---

## 15. Best Practices Summary

### Top 10 Rules

1. **Component-based design**: Create reusable, composable components
2. **Property bindings**: Use bindings, not JavaScript assignments
3. **Theme consistency**: Always use Theme singleton for styling
4. **Performance first**: Optimize for 60 FPS, use ListView/Repeater
5. **C++ integration**: Use controllers to bridge QML and application logic
6. **Layout over anchors**: Prefer Layouts for flexible layouts
7. **Smooth animations**: Keep transitions under 300ms, use appropriate easing
8. **No hardcoded values**: Use Theme constants for all styling
9. **Accessibility**: Set Accessible properties for screen readers
10. **Visual validation**: Test UI changes with screenshot comparison

### Quick Reference

| Topic | Guideline |
|-------|-----------|
| **File Naming** | PascalCase (e.g., `PatientBanner.qml`) |
| **Root ID** | Always name root element `root` |
| **Imports** | Qt modules → Local components → Theme → C++ types |
| **Properties** | Use properties for customization, provide defaults |
| **Bindings** | Prefer bindings over JavaScript assignments |
| **Layouts** | Use Layouts for flexible layouts, Anchors for fixed |
| **Theme** | Always use Theme singleton, no hardcoded values |
| **Performance** | Use ListView/Repeater, avoid dynamic object creation |
| **Controllers** | Use C++ controllers for business logic |
| **Animations** | Target 60 FPS, keep durations 200-300ms |

---

## Related Guidelines

- **UI/UX Guide**: See `doc/03_UI_UX_GUIDE.md` for design system and layout specifications
- **C++ Guidelines**: See `.cursor/rules/cpp_guidelines.mdc` for C++ controller implementation
- **Code Organization**: See `doc/22_CODE_ORGANIZATION.md` for project structure
- **API Documentation**: See `.cursor/rules/api_documentation.mdc` for documenting QML components

---

## Enforcement

- **Code Review**: All QML code must follow these guidelines
- **Visual Validation**: Screenshots must be updated after UI changes
- **Theme Compliance**: No hardcoded colors, spacing, or typography values
- **Performance**: UI must maintain 60 FPS during normal operation

---

**Remember:** QML is declarative - let the framework handle updates through property bindings. Write less JavaScript, use more bindings. Keep components small, reusable, and focused on presentation.
