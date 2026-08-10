# Contributing to BearWave

BearWave is a Qt 6 desktop internet radio app for Linux, built with C++/QML and QtMultimedia.

The project favors:

- stability over feature churn
- minimal dependencies (Qt only at runtime)
- readable, conservative code
- solid desktop integration (MPRIS, tray) across environments

## Before You Contribute

- Check existing issues before opening a new one.
- Prefer focused pull requests over broad refactors.
- Keep behavior predictable for desktop users.
- Avoid adding dependencies unless there is a clear payoff.

## Local Setup

Build from the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

Optional local install:

```bash
cmake --install build --prefix "$HOME/.local"
```

If you changed QML:

```bash
qmllint src/qml/Main.qml src/qml/components/*.qml src/qml/theme/BearTheme.qml
```

### Layout modes to smoke-test after UI changes

| Window | Expectation |
|--------|-------------|
| Wide + tall (`≥1240` × `≥960`) | Now Playing stage + transport **dock** in the stage |
| Wide + short (e.g. `1440×900`) | Stage open, **compact cover**, bottom **transport-only** strip |
| Narrow (`<1240`) | No stage; **full** bottom player bar |

```bash
# Example short-window check
./build/src/bearwave
# then resize to ~1440×900
```

Run unit tests after backend changes:

```bash
ctest --test-dir build --output-on-failure
```

## Change Guidelines

- Follow the existing style in each file.
- Keep changes minimal and scoped.
- Do not block the UI thread.
- Keep network operations asynchronous.
- Preserve MPRIS behavior and user state compatibility.
- When changing resources, register them in `src/qml.qrc`.
- When changing the About page, keep the full GPL text resource visible and keep third-party technology notes accurate.

## Manual Checks

Please smoke-test the area you changed when possible:

- app launches successfully
- station list loads
- search and filtering behave correctly
- play/pause/stop work
- favorites persist across restart
- resume restores last station and volume
- tray and MPRIS behavior still work
- About page opens from sidebar/compact navigation, social links work, and the GPL license text is scrollable

## Pull Requests

Useful pull requests usually include:

- a short description of the user-visible change
- any relevant screenshots for UI changes
- exact verification commands that were run
- known limitations or follow-up work
