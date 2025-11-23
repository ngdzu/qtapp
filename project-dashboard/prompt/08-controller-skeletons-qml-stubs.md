Decision: Add QObject-based controller stubs (`DashboardController`, `AlarmController`, `SystemController`, `PatientController`, `SettingsController`, `TrendsController`, `NotificationController`) and register them to QML in `main.cpp` with placeholder `Main.qml`.

Context: Controllers expose Q_PROPERTY and signals; no heavy logic in stubs; used by QML placeholders.

Constraints:
- Use idiomatic Qt signals/slots and Q_PROPERTY; keep implementations minimal and header/source separated.

Expected output:
- `src/ui/*.h/.cpp` controller stubs, `src/main.cpp` QML registration, `resources/qml/Main.qml` placeholder.

Run/Verify:
- Run app (or minimal launcher) to verify QML loads and can access stub properties.
