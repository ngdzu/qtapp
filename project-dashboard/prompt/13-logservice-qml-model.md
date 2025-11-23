Decision: Implement `LogService` that appends log records and exposes them as a `QAbstractListModel` for the Diagnostics QML view.

Context: Use for runtime diagnostics and the Logs/Diagnostics view; levels Info/Warn/Error and timestamps required.

Constraints:
- Keep model API simple and efficient; support tailing and limited history size.

Expected output:
- `src/core/LogService.h/.cpp` and `src/ui/LogModel.h/.cpp` plus example usage in `DiagnosticsView.qml`.

Run/Verify:
- Append logs programmatically and confirm QML model updates in the Diagnostics view.
