# Alarm System Design

This document details the design and behavior of the comprehensive alarm system within the Z Monitor.

## 1. Alarm Priorities

The system implements a four-tier alarm priority level, each with distinct visual and audible characteristics to ensure immediate and appropriate clinician response.

-   **Critical (P1):** Immediate threat to patient life or device integrity. Requires urgent intervention.
-   **High (P2):** Potential threat to patient life or device integrity if not addressed promptly. Requires timely intervention.
-   **Medium (P3):** Requires awareness and follow-up, but not immediate intervention.
-   **Low (P4):** Informational or advisory, typically requiring no direct intervention but logged for review.

## 2. Alarm Triggers

Alarms are triggered by the `AlarmManager` C++ class based on various conditions:

-   **Physiological Thresholds:**
    -   Heart Rate: Above/below configurable limits.
    -   SpO2: Below configurable limits.
    -   Respiration Rate: Above/below configurable limits.
    -   ST-Segment: Exceeds clinical thresholds.
    -   PVC Count: Exceeds clinical thresholds.
-   **Device Status:**
    -   Infusion Pump: Occlusion pressure approaching critical levels.
    -   Battery: Low battery warning, critical battery level.
    -   Sensor Disconnection: e.g., ECG lead off, SpO2 probe disconnected.
-   **Technical Issues:**
    -   Network Connectivity: Loss of connection to the central server.
    -   System Health: High CPU temperature, memory issues.
-   **Predictive Alerts:** Based on simulated predictive analytics (e.g., "High Arrhythmia Risk").

## 3. Visual Alarm Indicators

The UI provides clear and distinct visual cues for each alarm priority.

-   **Critical Alarms:**
    -   **Screen-wide Glowing Red Flash:** The entire screen (or a prominent border/overlay) will flash with a glowing red animation. This is the most intrusive visual cue, designed to immediately capture attention.
    -   **Alarm Indicator:** A dedicated, persistent red icon (e.g., a flashing "CRITICAL" text or symbol) in the header or a dedicated alarm bar.
    -   **Affected Card:** The relevant Stat Card will have a flashing red border and potentially a red background.
-   **High Alarms:**
    -   **Alarm Indicator:** A dedicated, persistent yellow/orange icon (e.g., a flashing "HIGH" text or symbol) in the header or alarm bar.
    -   **Affected Card:** The relevant Stat Card will have a flashing yellow/orange border and potentially a yellow/orange background.
-   **Medium Alarms:**
    -   **Alarm Indicator:** A dedicated, persistent yellow/orange icon (non-flashing) in the header or alarm bar.
    -   **Affected Card:** The relevant Stat Card will have a solid yellow/orange border.
-   **Low Alarms:**
    -   **Notification System:** Typically routed through the Notification System (bell icon) rather than direct alarm indicators, unless specifically configured for a subtle visual cue.

## 4. Audible Alarm Patterns

Audible alarms are crucial for immediate attention when the clinician is not directly looking at the screen. Each priority has a distinct, recognizable pattern.

-   **Critical Alarm:**
    -   **Pattern:** Continuous, rapid, high-pitched beeps. (e.g., "BEEP-BEEP-BEEP... BEEP-BEEP-BEEP...")
    -   **Characteristics:** Loud, urgent, and persistent until acknowledged or resolved.
-   **High Alarm:**
    -   **Pattern:** Intermittent, moderately fast, medium-pitched beeps. (e.g., "BEEP-BEEP... BEEP-BEEP...")
    -   **Characteristics:** Distinct from critical, but still urgent.
-   **Medium Alarm:**
    -   **Pattern:** Slow, single-tone beeps. (e.g., "BEEP... BEEP... BEEP...")
    -   **Characteristics:** Less urgent, designed for awareness.
-   **Low Alarm:**
    -   **Pattern:** A single, soft chime or a very slow, low-pitched beep. (e.g., "Chime" or "BEEP....... BEEP.......")
    -   **Characteristics:** Non-intrusive, advisory.

## 5. Alarm Management

-   **Acknowledgement:** Clinicians can acknowledge alarms (e.g., by tapping an alarm indicator or a dedicated "Silence" button). Acknowledging an alarm silences the audible component and may change the visual cue (e.g., from flashing to solid) until the underlying condition is resolved.
-   **Silence:** Temporary silencing of audible alarms for a predefined period (e.g., 30 seconds, 2 minutes). Visual indicators remain active.
-   **Reset:** Alarms automatically reset when the underlying physiological or technical condition returns to normal.
-   **Alarm History:** All alarms (active and resolved) are logged in the `AlarmManager` and displayed in a dedicated "Alarm History" panel, accessible from the Diagnostics View or a dedicated alarm screen. This includes timestamp, type, priority, and resolution status.