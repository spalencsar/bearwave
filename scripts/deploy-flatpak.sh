#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

deploy_target="ragnar@hosting:/srv/www/flatpak.bearwave.app/index/public/"

if [[ ! -d flatpak_repo/objects ]]; then
  echo "flatpak_repo/ is missing or empty. Run scripts/build-flatpak.sh first." >&2
  exit 1
fi

if [[ ! -f flatpak_repo/summary.sig || ! -f flatpak_repo/summary.idx.sig ]]; then
  echo "Missing summary.sig files. Run scripts/build-flatpak.sh with GPG signing first." >&2
  exit 1
fi

rsync -avz \
  -e 'ssh -o RemoteCommand=none -o RequestTTY=no' \
  --no-group --no-owner --omit-dir-times --no-perms --no-times \
  --exclude='.lock' \
  flatpak_repo/ \
  "${deploy_target}"

for ref_file in bearwave.flatpakref flatpak_repo/bearwave.flatpakref flatpak_repo/bearwave.flatpakrepo; do
  if [[ -f "${ref_file}" ]]; then
    rsync -avz \
      -e 'ssh -o RemoteCommand=none -o RequestTTY=no' \
      --no-group --no-owner --omit-dir-times --no-perms --no-times \
      "${ref_file}" \
      "${deploy_target}$(basename "${ref_file}")"
    echo "Deployed ${ref_file} to ${deploy_target}"
  fi
done

echo "Deployed flatpak_repo/ to ${deploy_target}"
