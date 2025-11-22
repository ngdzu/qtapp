#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<EOF
Usage: $(basename "$0") [allow|revoke]

Grant or revoke X11 access for Docker containers.
On macOS with XQuartz, this disables/enables access control entirely,
as XQuartz doesn't reliably support network-based access control for Docker.

Examples:
  # Allow Docker containers to connect to X11
  $(basename "$0") allow

  # Revoke previously-added permission
  $(basename "$0") revoke

Action defaults to 'allow'.
EOF
}

ACTION=${1:-allow}

if ! command -v xhost >/dev/null 2>&1; then
  echo "xhost is required but not found in PATH" >&2
  exit 2
fi

if [[ "$ACTION" == "-h" || "$ACTION" == "--help" ]]; then
  usage
  exit 0
fi

if [[ "$ACTION" =~ ^(revoke|remove|-) ]]; then
  echo "Enabling X11 access control (Docker containers will be blocked)"
  xhost - || true
else
  echo "Disabling X11 access control (Docker containers can connect)"
  xhost +
fi

echo "Done. Use 'xhost' to inspect current access control status."
