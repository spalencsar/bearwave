# BearWave

KDE-focused desktop internet radio app for Linux, built with Qt 6, QML, KDE Frameworks, and Phonon.

BearWave is designed for fast station browsing, simple playback controls, favorites, resume support, tray behavior, and clean Plasma integration without turning into a heavy media suite.

[![Build](https://github.com/spalencsar/bearwave/actions/workflows/build.yml/badge.svg)](https://github.com/spalencsar/bearwave/actions/workflows/build.yml)
![Linux](https://img.shields.io/badge/platform-Linux-blue)
![KDE Plasma](https://img.shields.io/badge/desktop-KDE%20Plasma-1f6feb)
![Qt 6](https://img.shields.io/badge/Qt-6-41cd52)
![License: MIT](https://img.shields.io/badge/license-MIT-lightgrey)

## Screenshots

| Main window | Station browser |
| --- | --- |
| ![Main Window](screenshots/screen01.png) | ![Station Browser](screenshots/screen02.png) |

![About Dialog](screenshots/screen03.png)

Screenshots: KDE Plasma on Linux.

---

## Quick Start

Two sensible paths right now:

### Option A: Arch Linux

Use the included `PKGBUILD`:

```bash
makepkg -si
```

### Option B: Local source build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
cmake --install build --prefix "$HOME/.local"
```

Then launch:

```bash
~/.local/bin/bearwave
```

## What BearWave Is

BearWave focuses on:

- KDE-first internet radio playback
- fast browsing via the Radio Browser API
- favorites and resume support
- lightweight desktop integration through tray + MPRIS
- straightforward local installation and operation

BearWave intentionally does not aim to be:

- a local music library manager
- a podcast client
- a broad cross-platform media application
- a feature-heavy all-purpose audio suite

## Features

- internet radio via the Radio Browser API with local JSON caching
- station pages for Top, Germany, Netherlands, and quick world/genre filters
- local search and filtering by name, genre, and country
- sorting by name, bitrate, and votes
- favorites with persistent local storage
- manual station add
- playback metadata display when streams provide it
- resume support for last station and volume
- MPRIS integration for Plasma media controls and media keys
- system tray integration for background playback
- embedded About dialog with links and MIT license text

## Project Status

BearWave is an early public, source-first desktop project.

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

## Platform And Support

BearWave is intentionally KDE-first.

- primary platform target: Linux
- primary desktop target: KDE Plasma 6
- primary development environment: Arch Linux
- other Linux distributions may build from source, but are not yet documented or tested to the same level

### Tested On

BearWave is currently aligned with and tested primarily against:

- Arch Linux
- KDE Plasma 6
- Qt 6
- KDE Frameworks 6
- Phonon4Qt6

If support for other distributions becomes reliable, they should be added here explicitly instead of implied.

### Language Support

The current application UI is primarily in German.

- UI labels, dialogs, and user-facing controls are currently mostly German
- README, repository metadata, and development-facing material are in English
- full localization support is not implemented yet

If you expect a fully translated multi-language UI, that is not the current state yet.

## Installation Status

BearWave should currently be understood as:

- officially documented for source builds
- most naturally aligned with Arch Linux packaging
- likely portable to other Linux distributions with Qt 6 / KF6 / Phonon packages available
- not yet positioned as a broadly packaged consumer desktop app

If you want the least surprising path today, use either:

- a local source build
- the included Arch `PKGBUILD`

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

## Build And Install

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

Optional local install:

```bash
cmake --install build --prefix "$HOME/.local"
update-desktop-database "$HOME/.local/share/applications"
kbuildsycoca6
```

This installs:

- binary: `~/.local/bin/bearwave`
- desktop file: `~/.local/share/applications/org.kde.bearwave.desktop`
- icon: `~/.local/share/icons/hicolor/256x256/apps/org.kde.bearwave.png`

Note: the generated desktop file uses the install prefix chosen during `cmake --install`.

If you want a clean rebuild:

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

## Runtime Requirements

- Linux desktop session
- KDE Plasma recommended
- Qt 6 runtime libraries
- Phonon4Qt6 with a working backend
- network access to Radio Browser instances

## Usage

### Basic flow

1. Open a station page such as Top, DE, NL, Favorites, or a quick filter.
2. Select a station and start playback.
3. Add favorites for quick reuse.
4. Use Resume to continue from the last station and volume state.

### Keyboard shortcuts

- `Space`: play/pause
- `Ctrl+F`: focus search field

### Sorting

Sort station lists by:

- name
- bitrate
- votes

## Data And Persistence

BearWave stores user state under:

- favorites: `~/.config/bearwave/favorites.json`
- last station + volume: `~/.config/bearwave/state.json`
- API cache: `~/.cache/bearwave/api_cache/`

If these files are removed, app state resets to defaults or performs a fresh API sync.

## Plasma Integration

BearWave exposes playback through MPRIS, so it works with:

- Plasma media applets/widgets
- global media key handling
- external MPRIS-capable controllers

## Autostart

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

- ensure `phonon-qt6` and `phonon-qt6-vlc` or equivalent are installed
- ensure `vlc` is installed
- test another station URL because some streams go offline

### App icon or launcher entry not updating

Re-run:

```bash
update-desktop-database "$HOME/.local/share/applications"
kbuildsycoca6
```

If desktop caches are stale, a logout/login cycle may still be required.

### Station list empty or slow

- verify internet connectivity
- Radio Browser may be temporarily rate-limited or degraded
- try another station category or filter

## Current Limitations

- UI language is currently primarily German
- packaging and install guidance are strongest on Arch Linux
- broader distro support is not yet validated to the same standard
- there are no official binary releases for non-technical end users yet

## Contributing

Issues and focused pull requests are welcome.

Before opening a larger change, it is worth checking whether it matches the project direction:

- KDE-first desktop behavior
- no unnecessary dependencies
- small, maintainable changes
- stability before feature breadth

See [CONTRIBUTING.md](CONTRIBUTING.md) for local build and review expectations.

## Development Notes

- main UI: `src/qml/Main.qml`
- backend orchestration: `src/radiobackend.cpp`
- stream playback: `src/bearplayer.cpp`
- API layer: `src/radiobrowser.cpp`
- MPRIS adapter: `src/mprisadaptor.cpp`

For contributor and agent guardrails, see `AGENTS.md`.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
