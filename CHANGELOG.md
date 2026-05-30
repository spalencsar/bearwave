# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Flatpak build manifest and deployment guides for hosting an independent repository.

## [1.0.3] - 2026-05-30

### Added

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
