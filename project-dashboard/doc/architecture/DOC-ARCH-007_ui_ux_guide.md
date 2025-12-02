---
doc_id: DOC-ARCH-007
title: UI/UX Design Guide - Visual Design and Interaction Principles
version: v1.0
category: Architecture
subcategory: Interface Design
status: Approved
owner: UX Team
reviewers: 
  - UX Team
  - Clinical Team
  - Architecture Team
last_reviewed: 2025-12-08
next_review: 2026-03-08
related_docs:
  - DOC-ARCH-006 # System Overview
  - DOC-ARCH-001 # Software Architecture
  - DOC-ARCH-008 # Alarm System
  - DOC-PROC-013 # ADT Workflow
  - DOC-REF-003 # Alarm Codes
related_tasks:
  - PHASE-6A # Architecture document migration
related_requirements:
  - REQ-UI-001 # UI requirements
  - REQ-UX-001 # UX requirements
  - IEC-60601-1-8 # Alarm system standard
tags:
  - ui-ux
  - visual-design
  - interaction-design
  - color-palette
  - layout
  - components
  - medical-device-ui
  - touch-screen
  - accessibility
source: 03_UI_UX_GUIDE.md (z-monitor/architecture_and_design)
---

# UI/UX Design Guide - Visual Design and Interaction Principles

## Document Purpose

This document outlines the visual design, layout, and user interaction principles for the Z Monitor embedded medical device. It defines the color palette, typography, layout structure, component specifications, and interaction patterns to ensure a consistent, clinically safe, and user-friendly interface.

---

## 1. Visual Design Principles

### 1.1. Design Theme

**Theme:** "Cyberpunk/Modern" aesthetic with clinical clarity.

**Rationale:**
- **Low-light environments:** ICU and operating rooms often have reduced lighting
- **High readability:** Critical vitals must be instantly recognizable
- **Reduced eye strain:** Dark backgrounds reduce glare during extended monitoring
- **Modern aesthetic:** Conveys advanced medical technology

### 1.2. Color Palette

The color palette is carefully selected for clinical environments with strict contrast requirements:

#### Background Colors

| Color                | Hex Code  | Name     | Usage                       |
| -------------------- | --------- | -------- | --------------------------- |
| Primary Background   | `#09090b` | Zinc-950 | Main application background |
| Secondary Background | `#18181b` | Zinc-900 | Card backgrounds, panels    |
| Borders/Separators   | `#27272a` | Zinc-800 | Dividers, borders           |

#### Text Colors

| Color          | Hex Code  | Usage    |
| -------------- | --------- | -------- |
| Primary Text   | `#ffffff` | White    | Primary labels, vital sign values |
| Secondary Text | `#a1a1aa` | Zinc-400 | Secondary labels, units           |
| Disabled Text  | `#52525b` | Zinc-600 | Disabled controls                 |

#### Accent Colors (Clinical Significance)

| Color      | Hex Code  | Clinical Meaning | Usage                                                |
| ---------- | --------- | ---------------- | ---------------------------------------------------- |
| **Green**  | `#22c55e` | Normal/Healthy   | ECG waveform, normal vital ranges                    |
| **Blue**   | `#3b82f6` | Informational    | Plethysmograph waveform, info messages               |
| **Yellow** | `#eab308` | Warning          | Medium priority alarms, caution states               |
| **Orange** | `#f97316` | Elevated Warning | High priority alarms                                 |
| **Red**    | `#ef4444` | Critical         | Critical alarms, error messages, out-of-range vitals |
| **Purple** | `#a855f7` | Analytical       | Trend visualizations                                 |
| **Cyan**   | `#06b6d4` | Interactive      | Active controls, selected states                     |

**Color Accessibility:**
- All text/background combinations meet WCAG 2.1 Level AA (4.5:1 contrast ratio)
- Critical alarms use 7:1 contrast ratio (Level AAA)
- Color is never the sole indicator (always paired with text or icons)

### 1.3. Typography

**Font Family:** Clean, sans-serif fonts for optimal readability
- **Primary:** Roboto (preferred)
- **Fallback:** Inter, system sans-serif

**Font Sizing (8-inch touch screen, 1280x800 resolution):**

| Element             | Size | Weight    | Usage                     |
| ------------------- | ---- | --------- | ------------------------- |
| **Large Display**   | 48px | Bold      | Vital sign primary values |
| **Page Title**      | 32px | Bold      | View titles               |
| **Section Heading** | 24px | Semi-Bold | Section headers           |
| **Body Text**       | 16px | Regular   | Labels, descriptions      |
| **Small Text**      | 14px | Regular   | Units, timestamps         |
| **Caption**         | 12px | Regular   | Fine print, metadata      |

**Readability Guidelines:**
- Minimum font size: 12px (legible from 18-24 inches)
- Line height: 1.5× font size for body text
- Letter spacing: Normal (adjusted for all-caps labels)

### 1.4. Icons

**Style:** Modern, minimalist SVG icons
- **Icon Set:** Material Design Icons or Feather Icons
- **Sizes:** 16px, 24px, 32px, 48px (consistent sizing)
- **Stroke Width:** 2px for consistency
- **Color:** Match parent text color or accent color

**Icon Usage:**
- Always pair with text labels for critical actions
- Use universally recognized symbols (heart for HR, lungs for SpO2)
- Maintain consistent icon-to-function mapping

---

## 2. Layout Structure (Fixed 1280×800 Resolution)

The application uses a consistent layout across all views to ensure predictability and reduce cognitive load.

### 2.1. Sidebar (Left Navigation)

**Purpose:** Primary navigation for main application views.

**Dimensions:**
- **Expanded:** 200px width
- **Collapsed:** 64px width
- **Height:** Full screen (800px)

**States:**

| State     | Width | Content           | Animation         |
| --------- | ----- | ----------------- | ----------------- |
| Expanded  | 200px | Icon + text label | 200ms ease-in-out |
| Collapsed | 64px  | Icon only         | 200ms ease-in-out |

**Navigation Items:**
1. **Dashboard** (Home icon) - Real-time vitals view
2. **Trends** (Chart icon) - Historical data plots
3. **Settings** (Gear icon) - Device configuration
4. **Diagnostics** (Terminal icon) - System logs

**Footer:**
- **Power/Disconnect Button** - Clear indication of device power state
- **Version Number** - Small text below power button

**Interaction:**
- Tap icon to navigate to view
- Long press sidebar to toggle expanded/collapsed state
- Active view highlighted with accent color

### 2.2. Header (Top Bar)

**Dimensions:**
- **Height:** 60px
- **Width:** Full screen (1280px)

**Layout:**

```
┌─────────────────────────────────────────────────────────────────┐
│ [Patient Banner] [Connection Status]    [Bell Icon] [Clock]    │
└─────────────────────────────────────────────────────────────────┘
```

**Left Section (Patient Banner):**
- **Patient Name:** Prominent display (24px bold)
- **MRN:** Below name (14px regular)
- **Bed Location:** Badge or secondary text
- **Allergies:** Highlighted in red/orange if present

**Center Section (Connection Status):**
- **ONLINE (Green):** Connected, transmitting data
- **CONNECTING... (Yellow/Orange):** Attempting connection
- **OFFLINE (Red):** Disconnected, buffering locally

**Right Section:**
- **Notification Bell:** Icon with badge count
- **System Clock:** Current time and date
- **Software Version:** Small text (optional, can be in settings)

### 2.3. Main Content Area

**Dimensions:**
- **Width:** 1280px - sidebar width (1216px expanded, 1080px collapsed)
- **Height:** 800px - header height (740px)

**Layout:** Adapts to the specific view:
- **Dashboard:** Grid of stat cards (2×3 or 3×2 layout)
- **Trends:** Large plot area with controls
- **Settings:** Vertical scrolling form
- **Diagnostics:** Split view (command input + log output)

---

## 3. Specific UI Behaviors

### 3.1. Critical Alarm Indication (Screen-wide Flash)

**Trigger:** Critical priority alarm activated (IEC 60601-1-8 compliance).

**Visual Behavior:**
- **Full-screen red overlay** with 50% opacity
- **Flashing pattern:** 1 second ON, 0.5 seconds OFF (repeats until acknowledged)
- **Border flash:** 10px red glowing border around entire screen
- **Alarm text:** "CRITICAL ALARM" in large text (48px bold)

**Interaction:**
- Any touch dismisses the full-screen flash (but alarm remains active)
- Alarm indicator remains in header until acknowledged
- Alarm details accessible via alarm panel

**Compliance:**
- Meets IEC 60601-1-8 visual alarm requirements
- Does not obscure critical vital signs (overlay, not replacement)
- Distinct from high/medium priority visual indicators

See **DOC-ARCH-008** (Alarm System) for complete alarm architecture.

### 3.2. Notification System

**Bell Icon (Header):**
- **Location:** Top right of header
- **Badge:** Numerical count of unread notifications (max display: "9+")
- **Color:** Blue (info) or Yellow (warning) based on highest priority

**Notification Panel:**
- **Trigger:** Tap bell icon
- **Layout:** Dropdown overlay (300px width × 400px height)
- **Position:** Anchored to bell icon, expands downward

**Message Types:**

| Type              | Color         | Icon             | Example                             |
| ----------------- | ------------- | ---------------- | ----------------------------------- |
| **Informational** | Blue          | Info circle      | "System update complete"            |
| **Warning**       | Yellow/Orange | Warning triangle | "Low battery: 30 minutes remaining" |

**Interaction:**
- **Individual Dismiss:** X button on each message
- **Clear All:** Button at bottom of panel
- **Tap Message:** Opens relevant view (e.g., tap "Low battery" → Settings)

### 3.3. Interactive Elements

**Buttons:**
- **Visual States:** Default, Hover, Active, Disabled
- **Feedback:** 100ms press animation (scale 0.95)
- **Touch Target:** Minimum 44×44px (WCAG 2.1 Level AAA)

**Sliders:**
- **Track:** 6px height, gray background
- **Thumb:** 20px diameter, accent color
- **Value Display:** Shows current value above thumb

**Toggles:**
- **ON State:** Accent color background, white checkmark
- **OFF State:** Gray background, no checkmark
- **Animation:** 200ms slide transition

**Graphs/Waveforms:**
- **Update Rate:** 60 FPS for waveforms, 1 Hz for vitals
- **Tooltips:** Show on touch/hover with 200ms delay
- **Zoom/Pan:** Pinch-to-zoom, drag-to-pan (trends view)

---

## 4. Component-Specific UI Details

### 4.1. Stat Cards (Dashboard View)

**Purpose:** Display real-time vital signs with visualizations.

**Layout (per card):**
```
┌─────────────────────────┐
│ [Icon] Parameter Name   │
│                         │
│   [Large Value] [Unit]  │
│                         │
│ [Visualization Area]    │
│                         │
│ [Min] [Avg] [Max]       │
└─────────────────────────┘
```

**Card Dimensions:**
- **Width:** ~360px (3 columns in 1080px content area)
- **Height:** ~220px
- **Padding:** 16px
- **Border Radius:** 8px

**Visualizations:**

| Vital Sign         | Visualization             | Update Rate | Color |
| ------------------ | ------------------------- | ----------- | ----- |
| **ECG**            | Rolling waveform (Canvas) | 250 Hz      | Green |
| **Plethysmograph** | Filled area waveform      | 250 Hz      | Blue  |
| **Infusion Pump**  | Circular progress gauge   | 1 Hz        | Cyan  |

**Alarm Indication (on cards):**
- **Critical:** Red flashing border (2px, 1Hz flash)
- **High:** Orange solid border (2px)
- **Medium:** Yellow solid border (1px)
- **Low:** No border change (alarm indicator in header)

### 4.2. Diagnostics View (Logs)

**Layout:** Split view (horizontal or vertical based on user preference).

**Command Input Area:**
- **Location:** Top or left (200px height/width)
- **Input Field:** Monospace font (14px), dark background
- **Prompt:** `>` symbol indicating ready state
- **History:** Up/down arrows navigate command history

**Log Terminal:**
- **Location:** Bottom or right (540px height / remaining width)
- **Font:** Monospace (14px), auto-scrolling
- **Colors:**
  - **INFO:** White text
  - **WARN:** Yellow text
  - **ERROR:** Red text
  - **DEBUG:** Gray text (dimmed)

**Interaction:**
- **Auto-scroll:** Enabled by default, disabled when user scrolls up
- **Copy Text:** Long press to select and copy log entries
- **Filter:** Dropdown to filter by log level

### 4.3. Trends View

**Layout:**
```
┌─────────────────────────────────────────────┐
│ [Parameter Selector] [Time Range Selector] │
│                                             │
│                                             │
│         [Large Plotting Area]               │
│                                             │
│                                             │
│ [Legend] [Zoom Controls]                    │
└─────────────────────────────────────────────┘
```

**Controls:**
- **Parameter Selector:** Dropdown or tabs (HR, SpO2, RR, Temp, BP)
- **Time Range Selector:** Buttons (1h, 8h, 24h, Custom)

**Plotting Area:**
- **Width:** Full content width (~1080px)
- **Height:** ~600px
- **Axes:** Time (X-axis), Value (Y-axis)
- **Grid:** Subtle gray grid lines
- **Data Points:** Line chart with markers

**Interactivity:**
- **Zoom:** Pinch-to-zoom or mouse wheel
- **Pan:** Drag to scroll horizontally
- **Tooltip:** Touch/hover shows timestamp and exact value
- **Export:** Button to export data as CSV (future)

### 4.4. Settings View

**Layout:** Vertical scrolling form with sections.

**Sections:**

#### Device Configuration
- **Device Label:** Text display (read-only for most users)
  - Example: "ICU-MON-04"
  - Editable by Technician role only
- **Device ID:** Text input for telemetry transmission ID
  - Example: "ZM-001"
  - Used for server communication
- **Measurement Unit:** Dropdown (Metric / Imperial)
  - Affects vitals, temperature, infusion rates

#### Patient Management
- **Current Patient Status:**
  - If admitted: Patient name, MRN, bed location
  - If not admitted: "No Patient Admitted"
- **Actions:**
  - "Admit Patient" button (opens Admission Modal)
  - "Discharge Patient" button (if patient admitted, requires confirmation)
  - "Transfer Patient" button (if patient admitted, Technician role only)
- **Patient History:** Link to view admission/discharge history

#### Alarm Limits
- **Per Vital Sign:** Number inputs or sliders
- **Parameters:** HR, SpO2, RR, Temp, BP (systolic/diastolic)
- **Thresholds:** Low Critical, Low Warning, High Warning, High Critical

#### Display Settings
- **Brightness:** Slider (0-100%)
- **Theme:** Toggle (Day/Night mode)
- **Screen Timeout:** Dropdown (1 min, 5 min, 15 min, Never)

#### Sound Settings
- **Alarm Volume:** Slider (0-100%, minimum 20% for safety)
- **Test Alarm:** Button to play test alarm tone
- **Mute:** Temporary mute button (2-minute auto-unmute)

#### Network Settings

**Provisioning Section (Primary):**
- **Provisioning Status:** Indicator (Not Provisioned, Ready to Pair, Pairing, Configuring, Provisioned, Error)
- **QR Code Display:** Large QR code (200×200px, regenerated every 30 seconds)
- **Pairing Code:** Human-readable code (e.g., "ABC-123-XYZ") with copy button
- **Expiration Timer:** Countdown (e.g., "8:45 remaining")
- **Status Message:** Descriptive text for current state
- **Actions:**
  - "Enter Provisioning Mode" (requires Technician role)
  - "Regenerate QR Code"
  - "Cancel Provisioning"
  - "Re-provision Device" (requires confirmation)
  - "Simulate Configuration" (development/testing only)

**Connection Status Section (Read-Only, when provisioned):**
- **Connection Status:** Visual indicator (green/yellow/red)
- **Server URL:** Read-only display
- **Certificate Status:** Expiration date, validation status
- **Last Connected:** Timestamp of last successful connection
- **Connection Statistics:** Uptime, data transmitted, errors

**Access Control:**
- Device Configuration requires **Technician** role
- Patient Management available to **Clinician** and **Technician** roles

### 4.5. Admission Modal (Patient Admission Workflow)

**Purpose:** Admit a patient using Manual Entry, Barcode Scan, or Central Station Push.

**Layout:** Modal overlay (600×500px, centered on screen).

**Components:**

#### Admission Method Selection
- **Radio Buttons or Tabs:** Manual Entry | Barcode Scan | Central Station

#### Manual Entry Section
- **MRN/Name Input:** Text field with search icon
- **Lookup Button:** "Search" button (triggers `IPatientLookupService`)
- **Loading Indicator:** Spinner with "Looking up patient..." text

#### Barcode Scanner Section
- **Camera View:** Live feed for barcode scanning (400×300px)
- **Scan Indicator:** Visual feedback when barcode detected
- **Automatic Lookup:** Extracts MRN and triggers lookup

#### Patient Preview
Displays retrieved patient information:
- **Name:** Large, prominent (24px bold)
- **MRN:** Below name
- **Date of Birth:** Small text
- **Sex:** M/F/O/U
- **Allergies:** Highlighted in red/orange if present
- **Bed Location:** Editable field (can override HIS assignment)

#### Action Buttons
- **"Admit Patient":** Confirms admission (primary button, accent color)
- **"Cancel":** Closes modal without admitting (secondary button, gray)

#### Error Display
- **Error Message:** Red text with retry button if lookup fails

**Behavior:**
1. User selects admission method
2. Enters MRN or scans barcode
3. Patient data retrieved and displayed
4. User confirms bed location
5. Tap "Admit Patient" to complete
6. Patient association created, admission logged

**Access:** Clinician and Technician roles

**Entry Points:**
- Tap Patient Banner when showing "DISCHARGED / STANDBY"
- Settings → Patient Management → "Admit Patient"
- Quick action button in header (when no patient admitted)

See **DOC-PROC-013** (ADT Workflow) for complete admission process.

### 4.6. Patient Discharge Workflow

**Purpose:** Discharge a patient from the device.

**Access:** Patient Banner (long press menu) or Settings → Patient Management.

**Discharge Confirmation Dialog:**
- **Layout:** Modal overlay (400×300px, centered)
- **Patient Info:** Display current patient name and MRN
- **Message:** "Discharge [Patient Name] from this device?"
- **Actions:**
  - "Discharge" button (red, confirms discharge)
  - "Cancel" button (gray, cancels action)

**Optional Discharge Options:**
- "Transfer to another device" (opens transfer workflow)
- "Archive patient data" (for long-term storage)

**Behavior:**
1. User initiates discharge
2. Confirmation dialog displayed
3. On confirmation:
   - Patient discharged from device
   - Patient data preserved (retention policy: 90 days)
   - Device enters "STANDBY" state
   - Header updates to "DISCHARGED / STANDBY"
   - Discharge event logged to `admission_events` and `audit_log`
4. Device ready for next patient admission

### 4.7. Login View

**Purpose:** User authentication via PIN.

**Layout:** Centered on screen (400×500px).

**Components:**
- **Device Branding:** Logo or device name at top
- **User ID Field:** Text input (read-only, pre-filled or selectable)
- **PIN Entry:** Numeric keypad (3×4 grid, buttons 60×60px)
- **Display:** Masked input showing asterisks (e.g., "****")
- **Login Button:** "Login" button below keypad
- **Status:** Error message area (red text) for invalid PIN

**Keypad Layout:**
```
[1] [2] [3]
[4] [5] [6]
[7] [8] [9]
[C] [0] [✓]
```

**Interaction:**
1. User taps User ID (if not pre-filled)
2. User enters PIN via keypad
3. Tap "✓" or "Login" to submit
4. On success: Transition to Dashboard View
5. On failure: Display error, clear PIN field, allow retry

---

## 5. Reusable Component Specifications

### 5.1. PatientBanner

**Location:** Top of header, left section.

**Dimensions:**
- **Width:** ~500px
- **Height:** 60px (full header height)

**Display States:**

#### When Patient Admitted
- **Patient Name:** Large, bold (24px), white text
- **MRN:** Below name (14px), gray text ("MRN: 12345")
- **Bed Location:** Badge or secondary text ("Bed: ICU-4B")
- **Allergies:** Comma-separated list, red/orange highlight
- **Admission Time:** Small text ("Admitted: 2h 15m ago")

#### When No Patient Admitted
- **Status Text:** "DISCHARGED / STANDBY" (20px bold, white)
- **Action Hint:** "Tap to admit patient" (14px, gray)

**Styling:**
- **Background:** Slightly lighter than header background (for visual separation)
- **Border:** 1px solid Zinc-700 (right side)
- **Padding:** 12px

**Interaction:**
- **No Patient:** Tap opens Admission Modal
- **Patient Admitted:** Tap opens Patient Details view
- **Long Press:** Opens quick actions menu (Discharge, Transfer, View History)

### 5.2. AlarmIndicator

**Location:** Header or dedicated alarm bar (right of patient banner).

**Dimensions:**
- **Width:** Variable (auto-size based on content)
- **Height:** 40px

**States:**

#### No Alarms
- **Display:** "All Clear" in green with checkmark icon
- **Background:** Transparent or subtle green background

#### Active Alarms
Displays highest priority alarm:

| Priority     | Color  | Icon | Text       | Animation        |
| ------------ | ------ | ---- | ---------- | ---------------- |
| **Critical** | Red    | ⚠️    | "CRITICAL" | Flashing (1Hz)   |
| **High**     | Orange | ⚠️    | "HIGH"     | Flashing (0.5Hz) |
| **Medium**   | Yellow | ⚠️    | "MEDIUM"   | Solid (no flash) |
| **Low**      | Blue   | ℹ️    | "LOW"      | Solid (no flash) |

**Interaction:**
- **Tap:** Opens alarm details panel or navigates to full alarm view
- **Badge:** Shows count of active alarms (e.g., "3 ACTIVE")

### 5.3. NotificationBell

**Location:** Top right of header.

**Dimensions:**
- **Icon:** 24×24px
- **Badge:** 18×18px circle (positioned top-right of icon)

**Appearance:**
- **Bell Icon:** Outline bell icon (Material Design)
- **Badge Color:** Blue (info) or Yellow (warning)
- **Badge Text:** White text (count of unread notifications)

**Badge States:**
- **Hidden:** When count is 0
- **Visible:** Shows count (e.g., "3")
- **Max Display:** "9+" for counts > 9

**Interaction:**
- **Tap:** Opens notification dropdown panel

**Dropdown Panel:**
- **Dimensions:** 300×400px
- **Position:** Anchored to bell icon, expands downward and leftward
- **Background:** Dark (Zinc-900) with border
- **Shadow:** Subtle drop shadow for depth

**Panel Layout:**
- **Header:** "Notifications" title with "Clear All" button
- **Message List:** Scrollable list of notifications
- **Empty State:** "No notifications" message when count is 0

**Message Item:**
- **Icon:** Info (blue) or Warning (yellow) icon
- **Title:** Message title (bold, 14px)
- **Body:** Message description (regular, 12px)
- **Timestamp:** Relative time (e.g., "5 min ago")
- **Dismiss:** X button (top-right of message)

---

## 6. Accessibility and Compliance

### 6.1. Touch Targets

**Minimum Size:** 44×44px (WCAG 2.1 Level AAA)
- All interactive elements meet minimum touch target size
- Spacing between adjacent targets: minimum 8px

### 6.2. Color Contrast

**Text Contrast:**
- Normal text: 4.5:1 (WCAG 2.1 Level AA)
- Large text (>24px): 3:1 (WCAG 2.1 Level AA)
- Critical alarms: 7:1 (WCAG 2.1 Level AAA)

**Non-text Contrast:**
- UI components: 3:1 contrast ratio
- Alarm indicators: 4.5:1 contrast ratio

### 6.3. Clinical Compliance

**IEC 60601-1-8 (Medical Alarm Systems):**
- Visual alarm indicators meet requirements for critical/high/medium/low priorities
- Color coding consistent with medical device standards
- Screen-wide flash for critical alarms
- Alarm acknowledgment workflow

**FDA Human Factors Guidance:**
- Clear visual hierarchy (vital signs most prominent)
- Consistent interaction patterns (reduce training time)
- Error prevention (confirmation dialogs for critical actions)
- Readable from 18-24 inches (typical viewing distance)

---

## 7. Related Documentation

### 7.1. Architecture Documents

- **DOC-ARCH-006:** System Overview (project scope and features)
- **DOC-ARCH-001:** Software Architecture (DDD layers and components)
- **DOC-ARCH-008:** Alarm System (alarm architecture and compliance)

### 7.2. Process Documents

- **DOC-PROC-013:** ADT Workflow (admission/discharge/transfer process)

### 7.3. Reference

- **DOC-REF-003:** Alarm Codes (alarm priorities and thresholds)

---

## 8. Document Metadata

| Field                     | Value                                                 |
| ------------------------- | ----------------------------------------------------- |
| **Original Document ID**  | DESIGN-003                                            |
| **Original Version**      | 1.0                                                   |
| **Original Status**       | Approved                                              |
| **Original Last Updated** | 2025-11-27                                            |
| **Migration Date**        | 2025-12-08                                            |
| **Migrated From**         | `z-monitor/architecture_and_design/03_UI_UX_GUIDE.md` |
| **New Document ID**       | DOC-ARCH-007                                          |
| **Category**              | Architecture                                          |
| **Subcategory**           | Interface Design                                      |

---

## Revision History

| Version | Date       | Author  | Changes                                                                                |
| ------- | ---------- | ------- | -------------------------------------------------------------------------------------- |
| v1.0    | 2025-12-08 | UX Team | Migrated from 03_UI_UX_GUIDE.md, expanded component specs, added accessibility section |

---

**End of Document**
