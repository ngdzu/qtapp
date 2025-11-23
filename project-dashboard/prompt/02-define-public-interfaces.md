Decision: Add header-only interface stubs for core services under `src/core/interfaces/` and corresponding interface docs under `doc/interfaces/`.

Context: Interfaces: IDatabaseManager, INetworkManager, IAlarmManager, IDeviceSimulator, ISettingsManager, IAuthenticationService, IArchiver. Keep signatures minimal; no implementations.

Constraints:
- Use C++17, avoid heavy dependencies in headers, use plain structs and `Result` enum for return values.
- Include brief threading/ownership comments for each interface.

Expected output:
- Files: `src/core/interfaces/IDatabaseManager.h`, `.../INetworkManager.h`, etc. and `doc/interfaces/*.md` with signatures and tests list.

Run/Verify:
- Inspect headers and docs; they should compile in header-only context (no source files required).
