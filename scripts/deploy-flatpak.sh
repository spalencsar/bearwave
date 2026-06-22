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
  --no-group --no-owner \
  --exclude='.lock' \
  flatpak_repo/ \
  "${deploy_target}"

echo "Deployed flatpak_repo/ to ${deploy_target}"
echo "bearwave.flatpakrepo on the server is left unchanged."