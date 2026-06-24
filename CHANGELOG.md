# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Add Russian language support and translation (thanks to [@aaly11](https://github.com/aaly11)).

### Changed

- Flatpak repository signing key replaced with `Bearwave App <dev@bearwave.app>` (`5BAA384577671E45`).

### Fixed

- Fix German translation contexts broken after the QML refactor (thanks to [@aaly11](https://github.com/aaly11)).

## Version notes (1.0.3 → 1.0.5)

- **1.0.3** — last GitHub release before the security/refactor batch.
- **1.0.4** — git tag with security hardening, playback/tray/MPRIS fixes, and QML refactor; tagged before CI was green, so it was not published as a GitHub release at first.
- **1.0.5** — current recommended release: everything in 1.0.4 plus CI, packaging, and post-release bugfixes (AUR/Flatpak rebuilds use `pkgrel` bumps, not a new app version).

## [1.0.5] - 2026-06-22

### Fixed

- Fix unit tests aborting in headless CI/Docker by using Qt offscreen platform and FFmpeg media backend.
- Fix QML resource paths so `qrc:/qml/Main.qml` loads in AUR and release builds (regression from 1.0.4 refactor).
- Fix About, Add station, and Edit station dialogs not opening after QML refactor (missing `property alias` in `Main.qml`).
- Fix About dialog version label binding and disable stale QML disk cache in development builds.
- Fix About dialog crash when opening the license scroll area.
- Fix startup when re-launching: MPRIS `Raise` now always shows the window (even without tray).
- Flatpak: grant GPU access via `--device=dri` (fixes EGL/MESA warnings on some systems).
- Flatpak: inject git build id into About when `.git` is unavailable in the builder sandbox.

### Changed

- Document 1.0.4 changes in README and CONTRIBUTING; run `ctest` and broader `qmllint` in CI.
- About dialog shows app version and git build id; removed “Public beta” label.
- Flatpak: upgrade to KDE Platform/SDK 6.10 (EOL 6.6 runtimes); GPG-sign repo summaries on build and deploy.

## [1.0.4] - 2026-06-20

### Security

- Validate manual station URLs: only `http://` and `https://` schemes are accepted.
- Restrict notification cover art downloads to HTTPS URLs.
- Add network transfer timeouts to ICY metadata, cover art, and notification fetches.

### Fixed

- Fix playback index for history, resume, and next/previous navigation.
- Fix Radio Browser API race where stale responses could overwrite newer results.
- Suppress error banner when cached station data is still available.
- Fix search filter persisting across page changes and compact-mode debounce.
- Fix stale list index after station list reload.
- Fix Wayland system tray menu using native `QSystemTrayIcon`.
- Fix MPRIS metadata publishing for desktop media widgets (e.g. PlasMusic).

### Changed

- Refactor monolithic `Main.qml` into reusable QML components and `BearTheme`.
- Remove unused Kirigami, I18n, and CoreAddons dependencies.
- Add backend unit tests for playback navigation and API race handling.

## [1.0.3] - 2026-05-30

### Added

- Flatpak build manifest and deployment guides for hosting an independent, GPG-signed repository at `flatpak.bearwave.app`.
- Dynamic World Categories dashboard showing country flags and popular genre tags.
- Dynamic country name localization mapping for German translation support.
- Local JSON caching for countries retrieved from the Radio Browser API.

### Changed

- Optimized Categories view layout to prevent text clipping (dynamic grid columns and scroll margins).

## [1.0.2] - 2026-05-28

### Added

- SPDX license identifiers (GPL-3.0-or-later) to all source files.
- Single instance application check via DBus to prevent multiple concurrent instances.

### Changed

- Relicensed the project from MIT to GNU GPLv3 (GPL-3.0-or-later).

### Fixed

- Fixed infinite loading of radio stations by adding transfer timeout and using `all.api.radio-browser.info`.
- Fixed metadata overwrite bug where valid ICY stream metadata was being cleared.
- Fixed DBus crash (segmentation fault) under KDE Plasma Wayland by making notification requests asynchronous.
- Fixed regex parser bug with stream titles containing apostrophes.
- Fixed crash (segmentation fault) in IcyReader when changing or stopping streams.

---

## [1.0.1] - 2026-05-22

### Added

- Debounced online search for radio stations.

### Changed

- Migrated playback engine from legacy Phonon to modern QtMultimedia.

### Fixed

- Fixed crash (segmentation fault) inside NotificationManager.

---

## [1.0.0-beta.1] - 2026-05-20

### Added

- Initial public beta release.
- C++ backend with system tray integration.
- Kirigami/QML based user interface.
- MPRIS integration for media controls.
- Persistent favorites and volume settings.
- Cover art fetching and caching.
