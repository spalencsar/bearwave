# Contributing to BearWave

BearWave is a KDE-focused desktop internet radio app built with C++/Qt 6, QML, KDE Frameworks, and Phonon.

The project favors:

- stability over feature churn
- minimal dependencies
- readable, conservative code
- Plasma-friendly desktop behavior

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
qmllint src/qml/Main.qml
```

## Change Guidelines

- Follow the existing style in each file.
- Keep changes minimal and scoped.
- Do not block the UI thread.
- Keep network operations asynchronous.
- Preserve MPRIS behavior and user state compatibility.
- When changing resources, register them in `src/qml.qrc`.

## Manual Checks

Please smoke-test the area you changed when possible:

- app launches successfully
- station list loads
- search and filtering behave correctly
- play/pause/stop work
- favorites persist across restart
- resume restores last station and volume
- tray and MPRIS behavior still work

## Pull Requests

Useful pull requests usually include:

- a short description of the user-visible change
- any relevant screenshots for UI changes
- exact verification commands that were run
- known limitations or follow-up work
