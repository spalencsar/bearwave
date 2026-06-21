# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## Version notes (1.0.3 → 1.0.5)

- **1.0.3** — last GitHub release before the security/refactor batch.
- **1.0.4** — git tag with security hardening, playback/tray/MPRIS fixes, and QML refactor; tagged before CI was green, so it was not published as a GitHub release at first.
- **1.0.5** — current recommended release: everything in 1.0.4 plus headless CI test fixes.

## [1.0.5] - 2026-06-20

### Fixed

- Fix unit tests aborting in headless CI/Docker by using Qt offscreen platform and FFmpeg media backend.

### Changed

- Document 1.0.4 changes in README and CONTRIBUTING; run `ctest` and broader `qmllint` in CI.

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
