# BearWave

BearWave is a desktop internet radio player for KDE Plasma, built with C++/Qt 6, QML, KDE Frameworks, and Phonon.

It is designed for fast station browsing, simple playback controls, and good integration into the Plasma desktop experience.

BearWave currently targets Linux desktop environments, with KDE Plasma as the primary focus.

## Screenshots

![Main Window](screenshots/screen01.png)
![Station Browser](screenshots/screen02.png)
![About Dialog](screenshots/screen03.png)

## Highlights

- Internet radio via the Radio Browser API (with ultra-fast local JSON caching)
- Station pages for Top, Germany, Netherlands, and global/genre filters
- Real-time local search and filtering (name, genre, country)
- Sorting (name, bitrate, votes)
- Favorites with persistent local storage
- Manual station add
- Playback controls and metadata display (title/artist when available)
- Resume support for last station and volume
- MPRIS integration (Plasma media controls, media keys, media applets with cover art fallback)
- System Tray integration for seamless background playback
- About dialog with project links and embedded MIT license text

## Project Status

BearWave is a source-first desktop project in an early public stage.

It is already usable, but it should currently be treated as software for testers, contributors, and technically comfortable Linux users rather than a polished end-user release.

Current priorities:

- stability in normal playback flows
- predictable KDE/Plasma integration
- conservative packaging and installation behavior
- keeping the codebase small and maintainable

Current distribution status:

- source repository is the primary delivery format
- Arch Linux is the best-supported packaging path at the moment
- no official Flatpak, AppImage, or broad distro release pipeline yet

## Target Platform

BearWave is intentionally KDE-first.

- primary desktop target: KDE Plasma 6
- primary platform target: Linux
- primary development environment: Arch Linux
- other Linux distributions may build from source, but are not yet documented or tested to the same level

## Tested On

BearWave is currently aligned with and tested primarily against:

- Arch Linux
- KDE Plasma 6
- Qt 6
- KDE Frameworks 6
- Phonon4Qt6

If support for other distributions becomes reliable, they should be added here explicitly instead of implied.

## Language Support

The current application UI is primarily in German.

- UI labels, dialogs, and user-facing controls are currently mostly German
- README, repository metadata, and development-facing material are in English
- full localization support is not implemented yet

If you open the project expecting a fully translated multi-language UI, that is not the current state yet.

## Tech Stack

- **Language:** C++
- **UI:** QML (`QtQuick`, `QtQuick.Controls`, `QtQuick.Layouts`)
- **Frameworks:** Qt 6, KDE Frameworks 6 (Kirigami where available)
- **Audio:** Phonon4Qt6 backend (e.g. VLC backend)
- **Networking:** `QNetworkAccessManager` against Radio Browser
- **Build system:** CMake

## Project Layout

```text
bearwave/
  CMakeLists.txt
  LICENSE
  README.md
  org.kde.bearwave.desktop.in
  src/
    main.cpp
    radiobackend.*
    radiobrowser.*
    radiostation.*
    bearplayer.*
    mprisadaptor.*
    bearwavecontroladaptor.*
    qml/Main.qml
    qml.qrc
```

## Runtime Requirements

- Linux desktop session (KDE Plasma recommended)
- Qt 6 runtime libraries
- Phonon4Qt6 + a Phonon backend (VLC backend recommended)
- Network access to Radio Browser instances

## Dependencies

### Arch Linux

```bash
sudo pacman -S cmake extra-cmake-modules qt6-base qt6-declarative qt6-tools \
  kirigami phonon-qt6 phonon-qt6-vlc vlc
```

### KDE Neon / Ubuntu-based

```bash
sudo apt install cmake ninja-build qt6-base-dev qt6-declarative-dev qt6-tools-dev \
  libkf6kirigami-dev libkf6i18n-dev libkf6coreaddons-dev phonon4qt6-dev
```

Note: exact package names can vary between distro releases, and non-Arch dependency sets should currently be treated as best-effort guidance rather than a guaranteed tested path.

## Installation Status

BearWave should currently be understood as:

- officially documented for source builds
- most naturally aligned with Arch Linux packaging
- likely portable to other Linux distributions with Qt 6 / KF6 / Phonon packages available
- not yet positioned as a broadly packaged consumer desktop app

If you want the least surprising path today, use either:

- a local source build
- the included Arch `PKGBUILD`

## Build

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

If you want a clean rebuild:

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

## Arch Linux (PKGBUILD)

For a clean, system-wide installation on Arch Linux, use the included `PKGBUILD`:

```bash
makepkg -si
```
This will automatically resolve dependencies, compile BearWave, and install it via pacman (`bearwave-git`).

## Install (User-local)

```bash
cmake --install build --prefix "$HOME/.local"
update-desktop-database "$HOME/.local/share/applications"
kbuildsycoca6
```

This installs:

- Binary: `~/.local/bin/bearwave`
- Desktop file: `~/.local/share/applications/org.kde.bearwave.desktop`
- Icon: `~/.local/share/icons/hicolor/256x256/apps/org.kde.bearwave.png`

Note: the generated desktop file uses the install prefix chosen during `cmake --install`.

## Run

- From launcher/menu: search for **BearWave**
- Or terminal:

```bash
~/.local/bin/bearwave
```

## Usage Guide

### Basic Flow

1. Open a station page (Top/DE/NL/Favorites or quick filters)
2. Pick a station and press play
3. Add favorites for quick access
4. Use Resume to continue from last session

### Keyboard Shortcuts

- `Space`: Play/Pause
- `Ctrl+F`: Focus search field

### Sorting

Use the sort controls to reorder station lists by:

- Name (A-Z)
- Bitrate
- Votes

## Data and Persistence

BearWave stores user state under:

- Favorites: `~/.config/bearwave/favorites.json`
- Last station + volume: `~/.config/bearwave/state.json`
- API Cache (RadioBrowser): `~/.cache/bearwave/api_cache/`

If these files are removed, app state resets to defaults or performs a fresh API sync.

## Plasma Integration

BearWave exposes playback through MPRIS, so it works with:

- Plasma media applets/widgets
- Global media key handling
- External MPRIS-capable controllers

## Scope

BearWave is intentionally focused.

What it is:

- a KDE-oriented internet radio desktop app
- a lightweight browser/player for online radio streams
- a Plasma-friendly app with MPRIS and tray integration

What it is not:

- a local music library manager
- a general-purpose podcast client
- a cross-platform media suite targeting every desktop equally

## Autostart (Optional)

Enable autostart:

```bash
mkdir -p "$HOME/.config/autostart"
cp "$HOME/.local/share/applications/org.kde.bearwave.desktop" "$HOME/.config/autostart/"
```

Disable autostart:

```bash
rm -f "$HOME/.config/autostart/org.kde.bearwave.desktop"
```

## Troubleshooting

### No audio playback

- Ensure `phonon-qt6` and `phonon-qt6-vlc` (or equivalent) are installed
- Ensure `vlc` is installed
- Test another station URL (some streams go offline)

### App icon/menu entry not updating

- Re-run:

```bash
update-desktop-database "$HOME/.local/share/applications"
kbuildsycoca6
```

- Log out/in if desktop cache is stale

### Station list empty or slow

- Verify internet connectivity
- Radio Browser endpoint may be rate-limited temporarily
- Try another station category/filter

## Contributing

Issues and focused pull requests are welcome.

Before opening a larger change, it is worth checking whether it matches the project direction:

- KDE-first desktop behavior
- no unnecessary dependencies
- small, maintainable changes
- stability before feature breadth

See [CONTRIBUTING.md](CONTRIBUTING.md) for local build and review expectations.

## Release Notes

For an initial public repository, the recommended install paths are:

- build from source
- user-local install via CMake
- Arch Linux via the included `PKGBUILD`

If broader distribution is added later, Flatpak or AppImage would be reasonable follow-ups.

## Current Limitations

- UI language is currently primarily German
- packaging and install guidance are strongest on Arch Linux
- broader distro support is not yet validated to the same standard
- there are no official binary releases for non-technical end users yet

## Development Notes

- Main UI: `src/qml/Main.qml`
- Backend orchestration: `src/radiobackend.cpp`
- Stream playback: `src/bearplayer.cpp`
- API layer: `src/radiobrowser.cpp`
- MPRIS adapter: `src/mprisadaptor.cpp`

For contributor/agent workflow and guardrails, see `AGENTS.md`.

## License

This project is licensed under the MIT License.
See `LICENSE` for full text.
