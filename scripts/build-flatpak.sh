#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

gpg_key_id="${BEARWAVE_FLATPAK_GPG_KEY:-AA1D2F0170800855}"

flatpak-builder \
  --repo=flatpak_repo \
  --gpg-sign="${gpg_key_id}" \
  --force-clean \
  flatpak_build \
  de.nerdbear.bearwave.json
flatpak build-update-repo --gpg-sign="${gpg_key_id}" flatpak_repo

echo "Flatpak repo updated in flatpak_repo/."
echo "Deploy with: scripts/deploy-flatpak.sh"

if [[ "${1:-}" == "--deploy" ]]; then
  exec "${root}/scripts/deploy-flatpak.sh"
fi