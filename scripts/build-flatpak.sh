#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

flatpak-builder --repo=flatpak_repo --force-clean flatpak_build de.nerdbear.bearwave.json
flatpak build-update-repo flatpak_repo

echo "Flatpak repo updated in flatpak_repo/."
echo "Deploy with: scripts/deploy-flatpak.sh"

if [[ "${1:-}" == "--deploy" ]]; then
  exec "${root}/scripts/deploy-flatpak.sh"
fi