#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

gpg_key_id="${BEARWAVE_FLATPAK_GPG_KEY:-AA1D2F0170800855}"
mode="build"
deploy=false

for arg in "$@"; do
  case "${arg}" in
    --reexport) mode="reexport" ;;
    --deploy) deploy=true ;;
    --reexport-deploy)
      mode="reexport"
      deploy=true
      ;;
  esac
done

if [[ "${mode}" == "reexport" ]]; then
  flatpak-builder \
    --repo=flatpak_repo \
    --gpg-sign="${gpg_key_id}" \
    --export-only \
    flatpak_build \
    de.nerdbear.bearwave.json
else
  flatpak-builder \
    --repo=flatpak_repo \
    --gpg-sign="${gpg_key_id}" \
    --force-clean \
    flatpak_build \
    de.nerdbear.bearwave.json
fi

flatpak build-update-repo --gpg-sign="${gpg_key_id}" flatpak_repo

echo "Flatpak repo updated in flatpak_repo/."

if [[ "${deploy}" == true ]]; then
  exec "${root}/scripts/deploy-flatpak.sh"
fi

echo "Deploy with: scripts/deploy-flatpak.sh"