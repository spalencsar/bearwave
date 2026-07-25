# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.2.0] - 2026-07-25

### Added

- Add Dutch UI translation.
- Add a persistent language selector for system default, German, English, Dutch, and Russian.
- Localize World country names and country search in German, Dutch, and Russian using bundled Unicode CLDR data.
- Add a validated asynchronous cache for station favicons and cover art.
- Add shared station-logo rendering with deterministic initials and homepage,
  manifest, touch-icon, favicon, and Open Graph discovery.
- Show connecting, buffering, retrying, paused, and unavailable stream states
  in the player bar and station details.
- Add an in-app changelog with bundled release notes in the About dialog.

### Changed

- Resolve and validate Radio Browser nodes dynamically while retaining
  `all.api.radio-browser.info` as the canonical fallback, with bounded retries
  and failed-node cooldown.
- Limit the cover cache to 50 MiB and remove entries unused for more than 30 days.
- Restructure the About dialog into clearer information, settings, actions,
  and license sections, using the illustrated BearWave PNG logo.
- Use one consistent BearWave button style across the main interface and
  dialogs while preserving keyboard focus.

### Fixed

- Complete Russian translations for the redesigned navigation and station details.
- Complete German translations for the redesigned interface.
- Restore spacing and fallback artwork in the right station detail panel.
- Make station detail bindings null-safe when station or player data is not available yet.
- Start with an adaptive wider window and switch to compact layout before translated controls can be clipped.
- Fall back cleanly when station images are oversized, mislabeled, or cannot be decoded.
- Fix the clipped player bar and remove a leftover layout marker.
- Prevent stale network replies from producing repeated closed-`QIODevice`
  warnings during metadata and cover requests.
- Clear stale metadata and artwork safely when switching stations.
- Keep release notes inside the About dialog instead of opening a second popup.
- Avoid persistent mouse-click focus outlines while retaining visible keyboard
  focus.
- Correct desktop menu categories and migrate AppStream developer metadata to
  the current schema.
- Remove the unsupported Flatpak manifest `version` property; Flatpak receives
  the release version from AppStream metadata.

## Version notes (1.1.0 → 1.2.0)

- **1.1.0** — desktop UI redesign release.
- **1.1.1** — published patch with SVG app/tray icons, button style fixes, and
  stronger stream URL validation.
- **1.2.0** — prepared feature release with expanded localization, resilient
  networking, validated station artwork, visible connection states, and an
  in-app changelog.

## [1.1.1] - 2026-07-21

### Security

- Validate stream URLs (`http`/`https` only) before every playback attempt and
  when loading favorites/history/resume state from disk.

### Added

- Use `bearwave.svg` as the primary app and tray icon; install it as the hicolor
  scalable theme icon (`de.nerdbear.bearwave.svg`). PNG remains as a raster
  fallback for MPRIS and older desktops.

### Changed

- Set the Flatpak app version to `1.1.1`.
- Refresh README/AppStream screenshots and demo video assets for the 1.1
  desktop layout.

### Fixed

- Remove light borders on search and dialog buttons on the dark UI.
- Keep the About page header consistent with the main app and improve return
  navigation.
- Prevent Flatpak deployment from failing on remote permission attributes.

## [1.1.0] - 2026-06-27

### Added

- Add Russian language support and translation (thanks to [@aaly11](https://github.com/aaly11)).
- Add a Mac-inspired desktop layout with persistent sidebar navigation, flat
  station rows, right-side station details, and a compact now-playing bar.
- Add an embedded About page with full GPLv3 license text loaded from the
  bundled `LICENSE` resource and a third-party technologies section.

### Changed

- Replace the Flatpak repository signing key with
  `Bearwave App <dev@bearwave.app>` (`5BAA384577671E45`).
- Replace the modal About dialog with an in-app About page reachable from
  sidebar and compact navigation.
- Refresh the QML visual theme toward a neutral dark palette.
- Use the BearWave line logo asset in the sidebar and remove duplicate branding
  from the top search toolbar.
- Make the top search/filter area and bottom player bar more responsive to
  narrower desktop window sizes.

### Fixed

- Fix German translation contexts broken after the QML refactor (thanks to [@aaly11](https://github.com/aaly11)).
- Fix the embedded GPL text failing to load in QML.
- Fix the About license view formatting.
- Fix right-side station details and top toolbar clipping.
- Fix About-page navigation.
- Reduce Denglish in the German UI while keeping common technical terms.
- Restore German and Russian translations for tray, backend error, and
  notification strings.

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
