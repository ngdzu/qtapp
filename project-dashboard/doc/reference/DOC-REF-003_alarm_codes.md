---
doc_id: DOC-REF-003
title: Alarm Codes and Priorities Reference
version: 1.0
status: Approved
created: 2025-12-01
updated: 2025-12-01
category: Reference
tags: [alarms, priorities, vital-signs, safety, clinical]
related:
  - DOC-COMP-006
  - DOC-COMP-008
  - DOC-REF-001
source: AlarmThreshold.h (domain/monitoring)
---

# Alarm Codes and Priorities Reference

## Overview

This reference document catalogs all alarm priorities and alarm types used in the Z Monitor application. Alarms are triggered when monitored vital signs exceed configured thresholds, providing clinical staff with timely notifications of patient status changes.

## Alarm Priority Levels

### Priority Enumeration

```cpp
enum class AlarmPriority {
    LOW,      ///< Low priority (advisory)
    MEDIUM,   ///< Medium priority (warning)
    HIGH      ///< High priority (critical)
};
```

### Priority Descriptions

| Priority   | Code  | IEC 60601-1-8 Level | Visual         | Audio                              | Description                                                                         |
| ---------- | ----- | ------------------- | -------------- | ---------------------------------- | ----------------------------------------------------------------------------------- |
| **HIGH**   | P1-P2 | High Priority       | Red, Flashing  | Continuous beep (high pitch, fast) | Immediate threat to patient life or device integrity, requiring urgent intervention |
| **MEDIUM** | P3    | Medium Priority     | Yellow, Steady | Intermittent beep (medium pitch)   | Potential threat requiring awareness and follow-up, but not immediate intervention  |
| **LOW**    | P4    | Low Priority        | Yellow, Steady | Single beep (low pitch)            | Advisory or informational, requiring no direct intervention                         |

### Priority Guidelines

**HIGH Priority (Critical)**
- **Clinical Impact:** Immediate life-threatening condition
- **Response Time:** < 30 seconds
- **Examples:** Severe hypoxia (SpO2 < 85%), extreme bradycardia (HR < 40), apnea (RR = 0)
- **Actions:** Immediate clinical intervention required
- **Audio:** Continuous high-pitched beep, cannot be silenced (only paused)
- **Visual:** Red flashing alarm indicator

**MEDIUM Priority (Warning)**
- **Clinical Impact:** Potential threat if not addressed
- **Response Time:** < 2 minutes
- **Examples:** Moderate tachycardia (HR > 120), hypotension (MAP < 50), low SpO2 (88-90%)
- **Actions:** Awareness and follow-up required
- **Audio:** Intermittent beep, can be silenced for 2 minutes
- **Visual:** Yellow steady alarm indicator

**LOW Priority (Advisory)**
- **Clinical Impact:** Informational only
- **Response Time:** No specific time requirement
- **Examples:** Lead disconnection, low battery, sensor calibration needed
- **Actions:** No immediate action required
- **Audio:** Single beep, can be silenced indefinitely
- **Visual:** Yellow steady alarm indicator

---

## Vital Sign Alarm Types

### Heart Rate (HR) Alarms

| Alarm Code              | Vital Type | Description                      | Default Threshold | Priority |
| ----------------------- | ---------- | -------------------------------- | ----------------- | -------- |
| `HR_BRADYCARDIA_SEVERE` | HR         | Severe bradycardia               | < 40 bpm          | HIGH     |
| `HR_BRADYCARDIA`        | HR         | Bradycardia                      | < 50 bpm          | MEDIUM   |
| `HR_TACHYCARDIA`        | HR         | Tachycardia                      | > 120 bpm         | MEDIUM   |
| `HR_TACHYCARDIA_SEVERE` | HR         | Severe tachycardia               | > 150 bpm         | HIGH     |
| `HR_SENSOR_FAULT`       | HR         | HR sensor disconnected or faulty | N/A               | LOW      |

**Clinical Significance:**
- **Bradycardia:** Low heart rate may indicate heart block, medication effect, or cardiac ischemia
- **Tachycardia:** High heart rate may indicate arrhythmia, fever, hypovolemia, or anxiety

---

### Oxygen Saturation (SpO2) Alarms

| Alarm Code            | Vital Type | Description                        | Default Threshold | Priority |
| --------------------- | ---------- | ---------------------------------- | ----------------- | -------- |
| `SPO2_SEVERE_HYPOXIA` | SPO2       | Severe hypoxia                     | < 85%             | HIGH     |
| `SPO2_HYPOXIA`        | SPO2       | Hypoxia                            | < 90%             | MEDIUM   |
| `SPO2_LOW`            | SPO2       | Low oxygen saturation              | < 92%             | MEDIUM   |
| `SPO2_SENSOR_FAULT`   | SPO2       | SpO2 sensor disconnected or faulty | N/A               | LOW      |

**Clinical Significance:**
- **Hypoxia:** Low oxygen saturation may indicate respiratory failure, airway obstruction, or cardiac arrest
- **Threshold Notes:** SpO2 < 90% is clinically significant hypoxia; < 85% is critical

---

### Respiration Rate (RR) Alarms

| Alarm Code            | Vital Type | Description                      | Default Threshold | Priority |
| --------------------- | ---------- | -------------------------------- | ----------------- | -------- |
| `RR_APNEA`            | RR         | Apnea (no breathing detected)    | RR = 0 for > 20s  | HIGH     |
| `RR_BRADYPNEA_SEVERE` | RR         | Severe bradypnea                 | < 8 bpm           | HIGH     |
| `RR_BRADYPNEA`        | RR         | Bradypnea                        | < 10 bpm          | MEDIUM   |
| `RR_TACHYPNEA`        | RR         | Tachypnea                        | > 25 bpm          | MEDIUM   |
| `RR_TACHYPNEA_SEVERE` | RR         | Severe tachypnea                 | > 35 bpm          | HIGH     |
| `RR_SENSOR_FAULT`     | RR         | RR sensor disconnected or faulty | N/A               | LOW      |

**Clinical Significance:**
- **Apnea:** Complete cessation of breathing is life-threatening
- **Bradypnea:** Low respiration rate may indicate respiratory depression (opioid overdose, CNS depression)
- **Tachypnea:** High respiration rate may indicate respiratory distress, pain, or metabolic acidosis

---

### Blood Pressure (BP) Alarms

| Alarm Code               | Vital Type | Description                      | Default Threshold | Priority |
| ------------------------ | ---------- | -------------------------------- | ----------------- | -------- |
| `BP_SEVERE_HYPOTENSION`  | MAP        | Severe hypotension               | MAP < 50 mmHg     | HIGH     |
| `BP_HYPOTENSION`         | MAP        | Hypotension                      | MAP < 60 mmHg     | MEDIUM   |
| `BP_HYPERTENSION`        | MAP        | Hypertension                     | MAP > 110 mmHg    | MEDIUM   |
| `BP_SEVERE_HYPERTENSION` | MAP        | Severe hypertension              | MAP > 130 mmHg    | HIGH     |
| `BP_SENSOR_FAULT`        | BP         | BP sensor disconnected or faulty | N/A               | LOW      |

**Clinical Significance:**
- **Hypotension:** Low blood pressure may indicate shock, hemorrhage, or cardiac failure
- **Hypertension:** High blood pressure may indicate hypertensive crisis or medication effect
- **MAP (Mean Arterial Pressure):** Calculated as `(SBP + 2*DBP) / 3`

---

### Technical Alarms

| Alarm Code             | Vital Type | Description                 | Default Threshold | Priority |
| ---------------------- | ---------- | --------------------------- | ----------------- | -------- |
| `SENSOR_DISCONNECTED`  | Any        | Sensor lead disconnected    | N/A               | LOW      |
| `SENSOR_FAULT`         | Any        | Sensor malfunction or error | N/A               | LOW      |
| `BATTERY_LOW`          | System     | Device battery low          | < 20%             | LOW      |
| `BATTERY_CRITICAL`     | System     | Device battery critical     | < 5%              | MEDIUM   |
| `NETWORK_DISCONNECTED` | System     | Network connection lost     | N/A               | LOW      |
| `DATABASE_ERROR`       | System     | Database operation failed   | N/A               | MEDIUM   |

---

## Alarm Threshold Configuration

### AlarmThreshold Structure

```cpp
struct AlarmThreshold {
    const std::string vitalType;      // "HR", "SPO2", "RR", etc.
    const float lowThreshold;         // Low limit (alarm if value < low)
    const float highThreshold;        // High limit (alarm if value > high)
    const float hysteresis;           // Hysteresis value (prevents alarm flapping)
    const AlarmPriority priority;     // LOW, MEDIUM, HIGH
    const bool enabled;               // Whether alarm is enabled
};
```

### Default Threshold Values

| Vital Type | Low Threshold | High Threshold | Hysteresis | Priority |
| ---------- | ------------- | -------------- | ---------- | -------- |
| **HR**     | 50 bpm        | 120 bpm        | 5 bpm      | MEDIUM   |
| **SPO2**   | 90%           | 100%           | 2%         | MEDIUM   |
| **RR**     | 10 bpm        | 25 bpm         | 2 bpm      | MEDIUM   |
| **MAP**    | 60 mmHg       | 110 mmHg       | 5 mmHg     | MEDIUM   |
| **TEMP**   | 36.0°C        | 38.5°C         | 0.5°C      | MEDIUM   |

**Note:** Thresholds are configurable per patient and can be adjusted by clinical staff.

---

## Alarm Hysteresis

**Purpose:** Prevents alarm "flapping" (rapid on/off cycling) when vital sign value hovers near threshold.

**Mechanism:**
- **Alarm Activation:** Triggers when value crosses threshold
- **Alarm Deactivation:** Only deactivates when value returns to safe zone by at least `hysteresis` amount

**Example (Heart Rate):**
- **Low Threshold:** 50 bpm
- **Hysteresis:** 5 bpm
- **Alarm Triggers:** When HR drops below 50 bpm
- **Alarm Clears:** When HR rises above 55 bpm (50 + 5)

**Configuration:**
```cpp
AlarmThreshold hrThreshold{
    .vitalType = "HR",
    .lowThreshold = 50.0f,
    .highThreshold = 120.0f,
    .hysteresis = 5.0f,        // 5 bpm hysteresis
    .priority = AlarmPriority::MEDIUM,
    .enabled = true
};
```

---

## Alarm State Machine

### Alarm States

1. **INACTIVE:** No alarm condition detected
2. **TRIGGERED:** Alarm condition detected, alarm activated
3. **ACKNOWLEDGED:** User acknowledged alarm (audio silenced)
4. **RESOLVED:** Alarm condition no longer present, alarm cleared

### State Transitions

```
INACTIVE → TRIGGERED (when vital crosses threshold)
TRIGGERED → ACKNOWLEDGED (when user acknowledges alarm)
ACKNOWLEDGED → RESOLVED (when vital returns to safe range + hysteresis)
TRIGGERED → RESOLVED (when vital returns to safe range + hysteresis)
RESOLVED → INACTIVE (after resolution delay)
```

---

## Alarm Audio Patterns

### Audio Profiles

| Priority   | Pattern           | Frequency            | Duration           | Silenceable        |
| ---------- | ----------------- | -------------------- | ------------------ | ------------------ |
| **HIGH**   | Continuous beep   | 1000 Hz, 3 beeps/sec | Continuous         | Pause only (2 min) |
| **MEDIUM** | Intermittent beep | 800 Hz, 1 beep/2 sec | Until acknowledged | Yes (2 min)        |
| **LOW**    | Single beep       | 600 Hz, 1 beep       | Once               | Yes (indefinite)   |

**Note:** Audio patterns follow IEC 60601-1-8 medical alarm standard.

---

## Related Documents

- [DOC-COMP-006: AlarmAggregate](../components/DOC-COMP-006_alarmaggregate.md) - Alarm domain model
- [DOC-COMP-008: AlarmThreshold](../components/DOC-COMP-008_alarmthreshold.md) - Threshold value object
- [DOC-REF-001: Glossary](./DOC-REF-001_glossary.md) - Project terminology

---

## Compliance Notes

### IEC 60601-1-8 (Medical Electrical Equipment - Alarms)

- **Alarm Priorities:** High, Medium, Low (as defined above)
- **Audio Patterns:** Compliant with pulse characteristics and frequency ranges
- **Visual Indicators:** Red for high priority, yellow for medium/low
- **Alarm Silence:** High-priority alarms can only be paused, not permanently silenced

### FDA 510(k) Requirements

- **Alarm Logging:** All alarms logged with timestamp, vital sign values, and user acknowledgments
- **Alarm Configuration:** Default thresholds meet clinical safety guidelines
- **Alarm Fatigue Mitigation:** Hysteresis, adjustable thresholds, and acknowledgment workflows

---

## Clinical References

- **Heart Rate:** Normal adult resting HR: 60-100 bpm
- **SpO2:** Normal oxygen saturation: 95-100% (> 90% acceptable for some COPD patients)
- **Respiration Rate:** Normal adult RR: 12-20 bpm
- **Blood Pressure:** Normal MAP: 70-100 mmHg
- **Temperature:** Normal core temperature: 36.5-37.5°C (97.7-99.5°F)
