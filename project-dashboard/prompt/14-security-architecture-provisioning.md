Decision: Draft `doc/06_SECURITY.md` describing the mTLS trust model, certificate provisioning, storage locations (`resources/certs/`), rotation policy and secure storage recommendations.

Context: Security design must be agreed before cert generation scripts; outline manufacturing vs field provisioning differences.

Constraints:
- Avoid platform-specific secure storage details beyond recommendations; note hardware-backed options as TODOs.

Expected output:
- `doc/06_SECURITY.md` with CA/trust model, provisioning steps, rotation notes, and file layout for certs.

Run/Verify:
- Manual review; ensure script requirements later match documented layout.
