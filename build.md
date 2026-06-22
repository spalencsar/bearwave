# BearWave – Build, Deploy & Fehlerprotokoll

Stand: 2026-06-22 · Release-Version: **1.0.5** (AUR + Flatpak)

## Versionspolitik

| Was | Regel |
|-----|--------|
| **App-Version** (`CMakeLists.txt`, About, Flatpak-Manifest) | Nur bei echtem Release erhöhen (aktuell **1.0.5**, siehe `CHANGELOG.md`) |
| **AUR** (`PKGBUILD`) | `pkgver=1.0.5` beibehalten, bei Bugfixes nur **`pkgrel`** erhöhen (aktuell **8**) |
| **Flatpak** (`de.nerdbear.bearwave.json` → `version`) | Gleiche Versionsnummer wie Release (**1.0.5**), neuer Build = neuer OSTree-Commit |
| **Build identifizieren** | Git-Hash im About-Dialog (`BEARWAVE_GIT_HASH`), nicht die App-Version hochzählen |

**Fehler (2026-06-22):** Mehrfach fälschlich 1.0.6–1.0.8 eingeführt, obwohl GitHub/AUR bei 1.0.5 bleiben. Bugfixes brauchen **keine** neue Minor-Version.

---

## AUR bauen

```bash
# Im PKGBUILD-Verzeichnis (AUR) oder lokal:
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

- Version kommt aus `project(bearwave VERSION 1.0.5)` in `CMakeLists.txt`
- Git-Hash: `src/CMakeLists.txt` → `git rev-parse --short HEAD` (lokal vorhanden)

---

## Flatpak bauen & deployen

```bash
# Vollständiger Build + Repo + optional Deploy
scripts/build-flatpak.sh --deploy

# Nur Re-Export (kein neuer Source-Build; finish-args ändern sich damit NICHT)
scripts/build-flatpak.sh --reexport-deploy
```

**Wichtig:** `--reexport` übernimmt **keine** Änderungen aus `de.nerdbear.bearwave.json` (z. B. `--device=dri`). Nach Manifest-Änderungen immer **vollen** Build ohne `--reexport`.

### Deploy-Ziel

- Server: `ragnar@hosting:/srv/www/flatpak.bearwave.app/index/public/`
- Es wird **`flatpak_repo/`** per rsync deployed, **nicht** `flatpak_build/`
- `bearwave.flatpakrepo` auf dem Server bleibt unverändert
- SSH-Host `hosting` hat `RemoteCommand` → rsync nutzt `-e 'ssh -o RemoteCommand=none -o RequestTTY=no'`

### GPG-Signierung

- Key: `AA1D2F0170800855` (über `BEARWAVE_FLATPAK_GPG_KEY` überschreibbar)
- `flatpak-builder --gpg-sign=…` und `flatpak build-update-repo --gpg-sign=…`
- Ohne `summary.sig` / `summary.idx.sig` schlagen Client-Updates fehl („BAD signature“)

### Git-Hash im Flatpak-Binary

`flatpak-builder` kopiert **kein** `.git` in die Sandbox → `git rev-parse` in CMake liefert sonst `unknown`.

Lösung: `scripts/build-flatpak.sh` schreibt `BEARWAVE_GIT_HASH` per generiertem Manifest (`.flatpak-builder-manifest.json`) in `build-options.env`. `src/CMakeLists.txt` liest `ENV{BEARWAVE_GIT_HASH}`.

### Runtime

- KDE **6.10** (`org.kde.Platform`), ersetzt EOL 6.6 / Freedesktop 23.08 (Issue #5)
- Branch im Repo: **`master`** (nicht `stable`)

### Finish-Args (aktuell)

- `--device=dri` – GPU/EGL-Zugriff (`/dev/dri`), ohne diesen: MESA/ZINK/EGL-Warnungen
- `--socket=wayland`, `--socket=fallback-x11`, `--share=ipc`, `--share=network`
- MPRIS: `--own-name=org.mpris.MediaPlayer2.bearwave`

---

## Installation (Endnutzer)

```bash
flatpak remote-add --user bearwave-repo https://flatpak.bearwave.app/bearwave.flatpakrepo
flatpak install --user bearwave-repo de.nerdbear.bearwave
flatpak update de.nerdbear.bearwave
flatpak run de.nerdbear.bearwave
```

Nach Update bei seltsamen QML-Fehlern Cache leeren:

```bash
rm -rf ~/.var/app/de.nerdbear.bearwave/cache/BearWave/BearWave/qmlcache
killall bearwave
```

---

## Bekannte Fehler & Fixes

### 1. AUR: `qrc:/qml/Main.qml not found`

**Ursache:** QRC-Prefix `/qml` + Pfad `qml/Main.qml` → Ressource unter `qrc:/qml/qml/Main.qml`.

**Fix:** Commit `a725430` – Prefix `/` in `src/qml.qrc`. Regressionstest: `tests/qrc_resources_test.cpp`.

---

### 2. About zeigt 1.0.3, `flatpak info` zeigt 1.0.6+

**Ursache:** Zwei getrennte Quellen:

- `flatpak info` → AppStream-Metainfo
- About → laufendes Binary (`BEARWAVE_VERSION` / `Qt.application.version`)

Typische Fälle:

1. **Alte Instanz läuft noch** (Single-Instance über MPRIS/DBus) – Update ersetzt Dateien, Prozess bleibt alt
2. **AUR-Binary** (`/usr/bin/bearwave`) statt Flatpak gestartet
3. Früher: Metainfo aktualisiert, Binary auf Server noch alt (Commit-Mismatch)

**Prüfen:**

```bash
flatpak info de.nerdbear.bearwave | grep -E 'Version|Commit'
which bearwave
killall bearwave && flatpak run de.nerdbear.bearwave
```

---

### 3. About / Manual+ / Edit öffnen nicht

**Symptom:**

```
HeaderNavigation.qml:112: TypeError: Cannot call method 'open' of undefined
```

**Ursache:** Seit QML-Refactor (2026-06-20) fehlten `property alias` in `Main.qml`. `HeaderNavigation` nutzt `app.aboutDialog`, `app.addDialog` – IDs allein exportieren keine Properties.

**Fix:** Commit `2de7706` (pkgrel 4):

```qml
property alias addDialog: addDialogPane
property alias editDialog: editDialogPane
property alias aboutDialog: aboutDialogPane
```

---

### 4. About: `String.arg(): Invalid arguments`

**Symptom:** Zeile in `AboutDialog.qml`, App startet teilweise (`Loaded 100 stations`).

**Ursachen (mehrere übereinander):**

1. **QML:** `qsTr("…%1…%2").arg(a, b)` – in QML nur **ein** Argument pro `.arg()`; korrekt: `.arg(a).arg(b)`
2. **Stale QML-Disk-Cache:** Altes kompiliertes QML in  
   `~/.var/app/de.nerdbear.bearwave/cache/BearWave/BearWave/qmlcache`  
   → Fehler bleibt trotz neuem Binary
3. **Fix im Code:** String-Verkettung statt `.arg()`; `QML_DISABLE_DISK_CACHE=1` in `main.cpp`

**Cache leeren** (siehe oben) oder App neu installieren.

---

### 5. Flatpak: EGL / MESA / ZINK-Warnungen

```
libEGL warning: failed to get driver name for fd -1
MESA: error: ZINK: failed to choose pdev
```

**Ursache:** Kein `--device=dri` im Manifest → kein `/dev/dri` in der Sandbox.

**Fix:** `--device=dri` in `de.nerdbear.bearwave.json` (voller Rebuild, nicht `--reexport`).

---

### 6. `flatpak run` – „es passiert nichts“

**Ursachen:**

1. **Single-Instance:** Zweiter Start ruft nur `Raise` auf und beendet sich (`main.cpp`). Wenn `raiseRequested` nicht verbunden war (nur über Tray), passiert nichts Sichtbares.
2. **Fix:** `raiseRequested` immer an Hauptfenster binden; `show()` / `requestActivate()` nach QML-Load; MPRIS-Registrierung **nach** erfolgreichem QML-Load (Commit `872670b`).

```bash
killall bearwave
busctl --user list | grep bearwave
```

---

### 7. About: `build unknown`

**Ursache:** Flatpak-Build ohne `.git` → `BEARWAVE_GIT_HASH=unknown`.

**Fix:** `BEARWAVE_GIT_HASH` via `build-flatpak.sh` → Manifest-`env` (siehe oben).

---

### 8. About-Dialog: Crash beim Öffnen

**Symptom:** About zeigt kurz „Version 1.0.5 · build unknown“, dann Absturz.

**Vermutliche Ursache:** `ScrollView` + `licenseScroll.availableWidth` unter KDE-Dialog/Flatpak (ungültige Breite beim ersten Layout).

**Fix (minimal, Layout unverändert):** Commit `315b5b6`

- `width: licenseScroll.availableWidth` → `width: Math.max(licenseScroll.width, 1)`
- `ScrollBar.AlwaysOn` → `ScrollBar.AsNeeded`

**Nicht tun:** Komplettes About-UI durch vereinfachtes Layout ersetzen (fehlerhafter Versuch in `a9f286b`, zurückgenommen in `315b5b6`).

---

### 9. PipeWire-Warnung

```
Failed to connect to pipewire instance "Der Rechner ist nicht aktiv"
```

Harmlos, wenn PulseAudio verfügbar ist. Kein BearWave-Bug.

---

### 10. Flatpak-Deploy: rsync exit 23

```
rsync: failed to set times on "...": Operation not permitted
```

Meist nur Timestamp/Owner auf dem Webserver – **Deploy-Inhalt ist trotzdem angekommen**. `--no-group --no-owner` ist gesetzt.

---

### 11. GPG Pinentry-Timeout beim Build

```
Failure signing commit file: Pinentry: Timeout
```

Build-Umgebung braucht funktionierendes GPG-Agent/Pinentry (interaktiv oder `loopback`).

---

## QML / Architektur-Hinweise

- Version im About: `Main.qml` → `appVersion` / `appBuildId` als Properties an `AboutDialog` (nicht auf globale Context-Properties im Modul verlassen)
- `setApplicationVersion()` **nach** `QApplication app(argc, argv)` in `main.cpp`
- Dialoge: immer `property alias` in `Main.qml`, wenn Kind-Komponenten sie über `app.*` ansprechen

---

## Commit-Übersicht (Bugfix-Serie 2026-06-22)

| Commit | Inhalt |
|--------|--------|
| `a725430` | QRC-Pfade |
| `ccb2152` | Flatpak KDE 6.10 |
| `15691bc` / `a8815cd` | GPG-Signierung Repo |
| `75313d7` | SSH Deploy RemoteCommand |
| `448aa13` | Version wieder auf 1.0.5 vereinheitlicht |
| `fe0591e` | `--device=dri` |
| `872670b` | Fenster show/Raise, MPRIS nach QML-Load |
| `2de7706` | Dialog-Property-Aliase |
| `2224307` / `818efc0` | About-Version, QML-Cache aus |
| `a9f286b` | Git-Hash Flatpak (About-UI-Vereinfachung – **zurückgenommen**) |
| `315b5b6` | About Original-Layout + minimaler ScrollView-Fix |

---

## Checkliste vor Release/Deploy

- [ ] `CMakeLists.txt` Version = Release-Nummer (1.0.5)
- [ ] `PKGBUILD` nur `pkgrel` erhöhen, wenn kein neues Release
- [ ] `de.nerdbear.bearwave.json` `version` = gleiche Nummer
- [ ] Voller Flatpak-Build (`scripts/build-flatpak.sh --deploy`), kein `--reexport` nach Manifest-Änderungen
- [ ] GPG-Signatur auf Repo prüfen
- [ ] Lokal: `flatpak update`, Cache ggf. leeren, About + Manual+ testen
- [ ] `killall bearwave` vor Test, nicht AUR- und Flatpak-Binary gleichzeitig laufen lassen