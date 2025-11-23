Decision: Add `scripts/render-mermaid.sh` to render all `.mmd` files in `doc/` and fail on any parse error; integrate into CI.

Context: Existing `.mmd` files live in `doc/`; some were previously failing to render due to fences/duplicate directives.

Constraints:
- Use `npx @mermaid-js/mermaid-cli` or installed binary; script must exit non-zero if any render fails.

Expected output:
- `scripts/render-mermaid.sh` and CI job invoking it.

Run/Verify:
- `bash scripts/render-mermaid.sh` should produce `.svg` files in `doc/` and exit 0 on success.
