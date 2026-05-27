# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- SPDX license identifiers (GPL-3.0-or-later) to all source code files.
- OARS content rating metadata to the AppStream configuration.

### Changed
- Relicensed the project from MIT to GNU GPLv3 (GPL-3.0-or-later).

### Fixed
- Fixed a metadata overwrite bug where the media player would clear valid ICY stream metadata.
- Fixed a DBus crash (segmentation fault) under KDE Plasma Wayland by making notification requests fully asynchronous.
- Fixed a regex parser bug that failed to extract stream titles containing apostrophes.

---

## [1.0.1] - 2026-05-22

### Added
- Added debounced online search for radio stations.

### Changed
- Migrated playback engine from the legacy Phonon backend to modern QtMultimedia.

### Fixed
- Fixed a crash (segmentation fault) inside `NotificationManager`.

---

## [1.0.0-beta.1] - 2026-05-20

### Added
- Initial public beta release of BearWave.
- C++ state and playback engine with system tray integration.
- Kirigami/QML user interface with compact and non-compact views.
- MPRIS integration for desktop media keys and lock screen controls.
- Persistent favorites and volume state settings.
- Cover art fetcher integration.
