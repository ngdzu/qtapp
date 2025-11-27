# UI/UX Guide: Z Monitor

**Document ID:** DESIGN-003  
**Version:** 1.0  
**Status:** Approved  
**Last Updated:** 2025-11-27

---

This document outlines the visual design, layout, and user interaction principles for the Z Monitor.

## 1. Visual Design Principles

-   **Theme:** "Cyberpunk/Modern" aesthetic, prioritizing clarity and readability in a low-light clinical environment.
-   **Color Palette:**
    -   **Background:** Dark (`#09090b` / Zinc-950)
    -   **Borders/Separators:** Darker gray (`#27272a` / Zinc-800)
    -   **Text (Primary):** White or light gray for high contrast.
    -   **Text (Secondary/Labels):** Slightly darker gray.
    -   **Accent Colors:**
        -   **Green:** Normal/Healthy vital signs, ECG waveform.
        -   **Blue:** Plethysmograph waveform, informational indicators.
        -   **Yellow/Orange:** Warning states, medium priority alarms.
        -   **Red:** Critical states, high priority alarms, error messages.
        -   **Purple/Cyan:** For specific data visualizations or interactive elements.
-   **Typography:** Clean, sans-serif fonts (e.g., Roboto, Inter) for optimal readability. Font sizes should be appropriate for an 8-inch touch screen, ensuring legibility from a short distance.
-   **Icons:** Modern, minimalist SVG icons for scalability and clarity.

## 2. Layout Structure (Fixed 1280x800 Resolution)

The application adheres to a consistent layout across all main views.

### 2.1. Sidebar (Left)

-   **Purpose:** Primary navigation for main application views.
-   **States:**
    -   **Expanded:** Width ~200px. Displays icon and text label for each navigation item.
    -   **Collapsed:** Width ~64px. Displays icon only. Transition between states should be smooth (e.g., 200ms animation).
-   **Items:** Dashboard, System Settings, Logs/Diagnostics, Trends.
-   **Footer:** "Power/Disconnect" button, clearly indicating device power state.

### 2.2. Header (Top)

-   **Height:** ~60px.
-   **Left Section:**
    -   **Patient Banner:** Persistent display of critical patient information (ID, Name, Age, Allergies).
    -   **Connection Status:** Dynamic indicator showing connection state to the central server:
        -   **"ONLINE" (Green):** Connected and transmitting data.
        -   **"CONNECTING..." (Yellow/Orange):** Attempting to establish connection.
        -   **"OFFLINE" (Red):** Disconnected, data not being transmitted.
-   **Right Section:**
    -   **Notification Bell:** Icon with a badge indicating the count of unread informational/warning messages. Clicking opens a notification panel.
    -   **System Version / Clock:** Displays current software version and system time.

### 2.3. Main Content (Center)

-   **Purpose:** Dynamic area that displays the content of the currently selected navigation view.
-   **Layout:** Adapts to the specific view (e.g., grid for Dashboard, list for Logs, plot for Trends).

## 3. Specific UI Behaviors

### 3.1. Critical Alarm Indication (Screen-wide Flash)

-   **Trigger:** Activation of a "Critical" priority alarm.
-   **Behavior:** The entire screen (or a prominent border/overlay) will flash with a **glowing red** animation. This flash should be highly noticeable but not interfere with the readability of critical data.
-   **Duration/Pattern:** A distinct, repetitive flash pattern (e.g., 1-second on, 0.5-second off) until the critical alarm is acknowledged or resolved.

### 3.2. Notification System

-   **Bell Icon:** Located in the header, displays a numerical badge for unread messages.
-   **Notification Panel:** Clicking the bell icon reveals a temporary overlay or dropdown panel listing informational and warning messages.
-   **Message Types:**
    -   **Informational (Blue/White text):** e.g., "System update complete."
    -   **Warning (Yellow/Orange text):** e.g., "Low battery: 30 minutes remaining."
-   **Dismissal:** Individual messages can be dismissed, or all can be cleared.

### 3.3. Interactive Elements

-   **Buttons/Toggles:** Clear visual feedback on press/release.
-   **Sliders/Inputs:** Intuitive interaction for adjusting settings or simulation parameters.
-   **Graphs/Waveforms:** Smooth, real-time updates. Tooltips or hover effects for detailed data points where applicable.

## 4. Component-Specific UI Details

### 4.1. Stat Cards (Dashboard View)

-   **Layout:** Each card has a primary metric prominently displayed, with secondary metrics and a visualization area.
-   **Visualizations:**
    -   **ECG:** Real-time rolling waveform (Green).
    -   **Plethysmograph:** Real-time filled area waveform (Blue).
    -   **Infusion Pump:** Circular progress gauge, pressure occlusion bar.
-   **Alarm Indication:** Individual cards will show visual cues (e.g., flashing borders, colored backgrounds) for High/Medium priority alarms related to their displayed metrics.

### 4.2. Diagnostics View (Logs)

-   **Split View:** Command input area (top/left) and scrolling log terminal (bottom/right).
-   **Log Styling:** Monospace font, distinct colors for log levels (Info=White, Warn=Yellow, Error=Red).

### 4.3. Trends View

-   **Plotting Area:** Displays historical data for selected vital signs.
-   **Controls:** Time range selection (e.g., 1h, 8h, 24h), parameter selection (e.g., HR, SpO2).
-   **Interactivity:** Zoom/pan functionality for detailed analysis.

### 4.4. Settings View

-   **Layout:** Organized into sections (Device Configuration, Alarms, Display, Sound, Network).
-   **Controls:**
    -   **Device Configuration:**
        -   **Device Label:** Display of static device identifier/asset tag (e.g., "ICU-MON-04"). Read-only for most users, editable by Technician role. This is the fixed technical identifier for the device itself, separate from patient assignment.
        -   **Device ID:** Text input for unique device identifier used for telemetry transmission (e.g., "ZM-001"). Used for device identification in server communication.
        -   **Measurement Unit:** Dropdown or toggle for selecting measurement system preference ("Metric" or "Imperial"). Affects display of vital signs, temperatures, and infusion rates throughout the application.
    -   **Patient Management (New Section):**
        -   **Current Patient Status:** Display of current patient information if admitted, or "No Patient Admitted" if in standby
        -   **Admission Actions:**
            -   "Admit Patient" button (opens Admission Modal)
            -   "Discharge Patient" button (if patient admitted, requires confirmation)
            -   "Transfer Patient" button (if patient admitted, requires Technician role)
        -   **Patient History:** Link to view admission/discharge history for current or previous patients
    -   **Alarm Limits:** Number inputs or sliders for setting upper/lower thresholds for each vital sign.
    -   **Display Settings:** Brightness slider, Day/Night theme toggle.
    -   **Sound Settings:** Alarm volume slider, test alarm button.
    -   **Network Settings:**
        -   **Provisioning Section (Primary):**
            -   **Provisioning Status:** Current provisioning state indicator (Not Provisioned, Ready to Pair, Pairing, Configuring, Provisioned, Error)
            -   **QR Code Display:** Large QR code for scanning by authorized tablet (regenerated every 30 seconds)
            -   **Pairing Code:** Human-readable pairing code (e.g., "ABC-123-XYZ") with copy button
            -   **Expiration Timer:** Countdown showing remaining time until pairing code expires (e.g., "8:45 remaining")
            -   **Status Message:** Descriptive message for current provisioning state
            -   **Action Buttons:**
                -   "Enter Provisioning Mode" (when not provisioned, requires Technician role)
                -   "Regenerate QR Code" (when ready to pair)
                -   "Cancel Provisioning" (to exit provisioning mode)
                -   "Re-provision Device" (when already provisioned, requires Technician role and confirmation)
                -   "Simulate Configuration" (development/testing only, simulates Central Station push)
        -   **Connection Status Section (Read-Only, when provisioned):**
            -   **Connection Status:** Connected/Disconnected indicator with visual status (green/yellow/red)
            -   **Server URL:** Current server URL (read-only, displayed for information)
            -   **Certificate Status:** Certificate expiration date, validation status, expiration warnings
            -   **Last Connected:** Timestamp of last successful connection
            -   **Connection Statistics:** Uptime, data transmitted, connection errors (if any)
        -   **Legacy Manual Configuration (Hidden/Deprecated):**
            -   Manual server URL and certificate inputs are removed in favor of provisioning workflow
            -   For development/testing, "Simulate Configuration" button provides mock provisioning
-   **Actions:** Save/Reset buttons for applying or reverting changes.
-   **Access Control:** Device Configuration settings require Technician role for modification.

### 4.5. Admission Modal (Patient Admission Workflow)

-   **Purpose:** Admit a patient to the device using one of three methods: Manual Entry, Barcode Scan, or Central Station Push.
-   **Layout:** Modal overlay accessible from Patient Banner (when no patient admitted) or Settings → Patient Management.
-   **Components:**
    -   **Admission Method Selection:**
        -   Radio buttons or tabs: "Manual Entry", "Barcode Scan", "Central Station"
    -   **Manual Entry Section:**
        -   **MRN/Name Input:** Text field for entering MRN or patient name
        -   **Lookup Button:** Triggers asynchronous patient lookup via `IPatientLookupService`
        -   **Loading Indicator:** Shows "Looking up patient..." during lookup operation
    -   **Barcode Scanner Section:**
        -   **Camera View:** Live camera feed for barcode scanning
        -   **Scan Indicator:** Visual feedback when barcode is detected
        -   **Automatic Lookup:** Automatically extracts MRN and looks up patient
    -   **Patient Preview:**
        -   Displays retrieved patient information:
            -   Name (prominent)
            -   MRN
            -   Date of Birth
            -   Sex
            -   Allergies (highlighted if present)
            -   Bed Location (editable, can override HIS assignment)
    -   **Action Buttons:**
        -   "Admit Patient" (confirms admission, creates patient association)
        -   "Cancel" (closes modal without admitting)
    -   **Error Display:** Shows error message if lookup fails (patient not found, network error, etc.)
-   **Behavior:**
    -   On successful lookup, patient information is displayed for confirmation
    -   Clinician can override bed location if different from HIS assignment
    -   User confirms admission, which updates `PatientManager` and creates admission record
    -   Patient data is cached locally in `patients` table
    -   Admission event is logged to `admission_events` and `audit_log`
    -   On failure, error message is displayed with option to retry
-   **Access:** Available to Clinician and Technician roles. Accessible via:
    -   Tap/click on Patient Banner when showing "DISCHARGED / STANDBY"
    -   Settings View → Patient Management → "Admit Patient" button
    -   Quick action button in header (when no patient admitted)

### 4.6. Patient Discharge Workflow

-   **Purpose:** Discharge a patient from the device when monitoring is no longer needed.
-   **Access:** Available from Patient Banner (long press menu) or Settings → Patient Management.
-   **Components:**
    -   **Discharge Confirmation Dialog:**
        -   Displays current patient information
        -   Confirmation message: "Discharge [Patient Name] from this device?"
        -   "Discharge" button (confirms discharge)
        -   "Cancel" button (cancels discharge)
    -   **Discharge Options (Optional):**
        -   "Transfer to another device" option
        -   "Archive patient data" option
-   **Behavior:**
    -   On confirmation, patient is discharged from device
    -   Patient data is preserved for audit (retention policy applies)
    -   Device enters "STANDBY" state
    -   Header updates to show "DISCHARGED / STANDBY"
    -   Discharge event is logged to `admission_events` and `audit_log`
    -   Device is ready for next patient admission

### 4.6. Login View

-   **Layout:** Centered on screen with device branding.
-   **Components:**
    -   **PIN Entry:** Numeric keypad for PIN entry.
    -   **Display:** Masked input showing entered digits as asterisks.
    -   **Login Button:** Submits credentials.
    -   **Status:** Error message display for invalid PIN.
-   **Behavior:** On successful login, transitions to Dashboard View.

## 5. Reusable Component Specifications

### 5.1. PatientBanner

-   **Location:** Top of header, left section.
-   **Display States:**
    -   **When Patient Admitted:**
        -   **Patient Name:** Prominently displayed (large, bold font)
        -   **MRN:** Displayed below name (smaller font, e.g., "MRN: 12345")
        -   **Bed Location:** Displayed as badge or secondary text (e.g., "Bed: ICU-4B")
        -   **Allergies:** Comma-separated list, highlighted in red/orange if present
        -   **Admission Time:** Displayed (e.g., "Admitted: 2h 15m ago")
    -   **When No Patient Admitted:**
        -   **Status Text:** "DISCHARGED / STANDBY" (prominently displayed, large font)
        -   **Action Hint:** "Tap to admit patient" (smaller text below status)
-   **Styling:** Compact layout, high contrast text, allergy field uses warning colors. Patient name uses prominent styling when admitted.
-   **Interaction:**
    -   When no patient is admitted: Tap/click opens Admission Modal
    -   When patient is admitted: Tap/click opens Patient Details view
    -   Long press opens quick actions menu (Discharge, Transfer, View History)

### 5.2. AlarmIndicator

-   **Location:** Header or dedicated alarm bar.
-   **States:**
    -   **No Alarms:** Hidden or shows "All Clear" in green.
    -   **Active Alarms:** Displays highest priority alarm with appropriate color:
        -   **Critical (Red):** Flashing red icon with "CRITICAL" text.
        -   **High (Orange):** Flashing orange icon with "HIGH" text.
        -   **Medium (Yellow):** Solid yellow icon with "MEDIUM" text.
        -   **Low (Blue):** Solid blue icon with "LOW" text.
-   **Interaction:** Clicking opens alarm details panel or navigates to full alarm view.

### 5.3. NotificationBell

-   **Location:** Top right of header.
-   **Appearance:** Bell icon with numerical badge showing unread count.
-   **Badge:**
    -   **Hidden:** When count is 0.
    -   **Visible:** Shows count (e.g., "3") in a colored circle.
-   **Interaction:** Clicking opens notification dropdown/panel.
-   **Dropdown Panel:**
    -   **Layout:** List of notification messages with timestamps.
    -   **Message Types:** Color-coded by severity (Info=Blue, Warning=Yellow/Orange).
    -   **Actions:** Individual dismiss buttons, "Clear All" button at bottom.
