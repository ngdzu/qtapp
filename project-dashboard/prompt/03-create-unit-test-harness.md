Decision: Scaffold a unit test harness using GoogleTest with mock headers for each interface in `tests/mocks/`.

Context: Tests should run without production implementations; mocks implement interface headers created earlier.

Constraints:
- Keep tests lightweight and fast; add `tests/CMakeLists.txt` to integrate with main CMake.
- Provide at least one example test demonstrating dependency injection with mocks.

Expected output:
- `tests/CMakeLists.txt`, `tests/mocks/mock_IDatabaseManager.h`, `tests/test_alarm_manager.cpp` example.

Run/Verify:
- `cmake -S . -B build && cmake --build build --target tests` (or equivalent) to see tests discovered (no failure if mocks only).
