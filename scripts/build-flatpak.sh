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

git_hash="$(git -C "${root}" rev-parse --short HEAD 2>/dev/null || echo unknown)"
build_manifest="${root}/.flatpak-builder-manifest.json"
python3 - "${root}/de.nerdbear.bearwave.json" "${build_manifest}" "${git_hash}" <<'PY'
import json, sys
src, dst, git_hash = sys.argv[1], sys.argv[2], sys.argv[3]
with open(src, encoding="utf-8") as f:
    manifest = json.load(f)
module = manifest["modules"][0]
module.setdefault("build-options", {}).setdefault("env", {})["BEARWAVE_GIT_HASH"] = git_hash
with open(dst, "w", encoding="utf-8") as f:
    json.dump(manifest, f, indent=4)
    f.write("\n")
PY

if [[ "${mode}" == "reexport" ]]; then
  flatpak-builder \
    --repo=flatpak_repo \
    --gpg-sign="${gpg_key_id}" \
    --export-only \
    flatpak_build \
    "${build_manifest}"
else
  flatpak-builder \
    --repo=flatpak_repo \
    --gpg-sign="${gpg_key_id}" \
    --force-clean \
    flatpak_build \
    "${build_manifest}"
fi

flatpak build-update-repo --gpg-sign="${gpg_key_id}" flatpak_repo

echo "Flatpak repo updated in flatpak_repo/."

if [[ "${deploy}" == true ]]; then
  exec "${root}/scripts/deploy-flatpak.sh"
fi

echo "Deploy with: scripts/deploy-flatpak.sh"