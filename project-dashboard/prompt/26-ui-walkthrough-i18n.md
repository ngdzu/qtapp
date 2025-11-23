Decision: Run UI walkthrough at 1280x800, collect polish tasks, and add i18n skeletons (`i18n/en_US.ts`, `i18n/es_ES.ts`) plus extraction script.

Context: Ensure `qsTr()` usage and accessibility (contrast) for alarm visuals.

Constraints:
- Provide script to extract strings (`lupdate`) and compile `.qm` files (`lrelease`).

Expected output:
- UX polish list, `i18n/` `.ts` placeholders, and `scripts/i18n_extract.sh`.

Run/Verify:
- Launch QML at target resolution and run `scripts/i18n_extract.sh` to produce `.ts` files.
