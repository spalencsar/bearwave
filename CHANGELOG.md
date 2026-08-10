# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.3.0] - 2026-08-10

Major multi-desktop UI release: universal Linux positioning, Now Playing stage,
responsive transport, My stations, and a redesigned station list.

### Added

- System light/dark appearance (xdg-desktop-portal, gsettings, Qt style hints,
  optional shell session files). Override with `BEARWAVE_THEME=light|dark`.
- **Now Playing** stage (right column): active stream only, large artwork,
  current/previous track cards, session track history from ICY metadata
  (max 30, deduped; cleared on stop/station change).
- **My stations** library for manually added streams
  (`~/.config/bearwave/my_stations.json`), separate from Favorites; edit and
  remove; country via localized dropdown.
- Shared monochrome **MediaIcon** / **IconButton** set (no emoji chrome).
- Shared **PlayerTransport** used by stage dock and bottom bar.
- Three transport layouts driven by window size:
  - wide + tall (`width ≥ 1240`, `height ≥ 960`): transport dock in the stage
  - wide + short (e.g. 1440×900): stage keeps Now Playing (compact cover);
    bottom strip is transport-only
  - narrow (`width < 1240`): full bottom player bar with station meta

### Changed

- Product framing: **Qt 6 Linux desktop app** (not KDE-only, not tied to a
  compositor or shell). About, AppStream, desktop entry, README, packaging.
- Flatpak: keep **org.kde.Platform/Sdk 6.10** as the official Qt stack on
  Flathub; document that this is not a Plasma requirement. Finish-args and
  cleanup tidied.
- Control D-Bus interface: `org.kde.BearWave.Control` →
  `de.nerdbear.BearWave.Control`.
- Desktop entry drops FreeDesktop `KDE` category; German Comment/Keywords.
- Radio Browser HTTP `User-Agent`: `BearWave/<version>`.
- Outside KDE sessions: Fusion Quick Controls, no KDE platform theme,
  frameless client chrome with BearTheme drag (`startSystemMove`).
- Left sidebar restyled (Browse / Library, DE·NL chips, monochrome icons).
- Station list: card layout, meta chips, Live marker, header with count,
  clearer empty states; single-click plays.
- Add/Edit station dialogs restyled to BearTheme.
- Responsive layout: detail column below ~1240px width; list margins keep
  card borders clear of the stage edge.

### Fixed

- Live streams no longer stick on **Buffering…** while audio is already
  playing (Qt BufferingMedia on radio).
- ICY metadata: force HTTP/1.1 (HTTP/2 breaks metaint), wait for headers,
  abort cleanly when a stream has no titles, broader StreamTitle parsing.
- Light mode when the desktop prefers light but Qt alone stayed dark.
- Arch packaging: `qt6-tools` makedepend; drop unused `extra-cmake-modules`.
- **Flatpak:** persist favorites, My stations, and state via XDG config/cache
  paths (`GenericConfigLocation` / `GenericCacheLocation`) instead of a hard-coded
  `~/.config/bearwave` under `$HOME`, which the sandbox cannot write without
  overrides ([#7](https://github.com/spalencsar/bearwave/issues/7)). Optional
  one-shot migration from the legacy host path when the new dir is empty.

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

## Version notes (1.1.0 → 1.3.0)

- **1.1.0** — public multi-desktop packaging baseline and Flatpak distribution.
- **1.1.1** — security and packaging follow-up (stream URL scheme enforcement).
- **1.2.0** — localization, resilient Radio Browser, artwork, connection states.
- **1.3.0** — multi-desktop UI overhaul, Now Playing stage, My stations,
  responsive transport layouts, universal product framing.

---

## [1.1.1] - 2026-07-21

### Security

- Enforce `http`/`https` stream URLs on every playback path and for persisted
  last-station / recent-station entries.

### Fixed

- Prefer the SVG app/tray icon and keep the PNG as a fallback when SVG load fails.
- Restore readable light borders on search and dialog buttons in the dark theme.
- Avoid Flatpak deploy failures on hosts that reject permission attributes on
  repository files.

### Changed

- Set the Flatpak app version to `1.1.1`.

## [1.1.0] - 2026-06-27

### Added

- Multi-desktop packaging baseline (desktop entry, AppStream, icons).
- Independent GPG-signed Flatpak repository distribution.

### Changed

- Packaging and install documentation for Flatpak, AUR, and source builds.

## [1.0.5] and earlier

See git history and older AppStream release notes for 1.0.x details.
