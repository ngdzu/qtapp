Decision: Prototype alarm UI visuals in QML: full-screen critical flash, per-card highlight, Alarm History panel, and an audio stub API.

Context: These are UI prototypes; back-end wiring will call methods like `showCriticalAlarm()` or update an `activeAlarmsModel`.

Constraints:
- No audio implementation; provide placeholder functions and visual triggers for manual testing.

Expected output:
- QML components `AlarmOverlay.qml`, `AlarmHistoryPanel.qml`, and demo triggers in `Main.qml`.

Run/Verify:
- Open `Main.qml` and trigger demo alarm flows to validate visuals.
