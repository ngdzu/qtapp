Decision: Create QML UI skeleton components (`Sidebar.qml`, `TopBar.qml`, `StatCard.qml`, `PatientBanner.qml`) and simple views (`DashboardView.qml`, `DiagnosticsView.qml`, `TrendsView.qml`, `SettingsView.qml`, `LoginView.qml`) under `resources/qml/`.

Context: Target screen resolution is 1280x800; Main.qml should use that root size for layout validation.

Constraints:
- Visual placeholders only; no data-binding complexity. Use scalable anchors and avoid external assets.

Expected output:
- `resources/qml/` components and `Main.qml` that composes them and displays placeholders.

Run/Verify:
- Launch QML app or `qmlscene` pointing at `Main.qml` to visually confirm placeholder layout.
