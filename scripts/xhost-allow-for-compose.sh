#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<EOF
Usage: $(basename "$0") [service] [allow|revoke]

Examples:
  # Allow the IP(s) of the `app-dev` service containers to connect to X11
  $(basename "$0") app-dev allow

  # Revoke previously-added permission
  $(basename "$0") app-dev revoke

If service is omitted the default is 'app'. Action defaults to 'allow'.
EOF
}

SERVICE=${1:-app}
ACTION=${2:-allow}

if ! command -v docker >/dev/null 2>&1; then
  echo "docker is required but not found in PATH" >&2
  exit 2
fi

if ! command -v xhost >/dev/null 2>&1; then
  echo "xhost is required but not found in PATH" >&2
  exit 2
fi

if [[ "$ACTION" == "-h" || "$ACTION" == "--help" ]]; then
  usage
  exit 0
fi

ids=$(docker compose ps -q "$SERVICE" 2>/dev/null || true)
if [[ -z "$ids" ]]; then
  echo "No running containers found for service '$SERVICE'." >&2
  echo "Start the service (e.g. 'docker compose up -d $SERVICE') and try again." >&2
  exit 3
fi

for id in $ids; do
  ip=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' "$id" 2>/dev/null || true)
  if [[ -z "$ip" ]]; then
    echo "Could not determine IP for container $id; skipping." >&2
    continue
  fi

  if [[ "$ACTION" =~ ^(revoke|remove|-) ]]; then
    echo "Revoking X access for $ip"
    xhost - "$ip" || true
  else
    echo "Allowing X access for $ip"
    xhost +"$ip"
  fi
done

echo "Done. Use 'xhost' to inspect current access list. To undo, run with 'revoke'."
