Decision: Add GitHub Actions workflows for build, unit-tests, mermaid rendering, and optional integration that starts the server simulator.

Context: Workflows should be modular; caching for CMake and dependencies recommended.

Constraints:
- Runner: use Ubuntu-latest; integration job optional and gated.
- Secrets: do not store dev cert private keys in CI.

Expected output:
- `.github/workflows/build.yml`, `tests` job, `render-mermaid.yml` or combined pipeline.

Run/Verify:
- Push workflow and ensure actions run; mermaid job should render SVGs and fail on parse errors.
