// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.15

// Light/dark surfaces for Linux desktops.
// Prefers ColorSchemeController (portal, gsettings, optional shell session);
// falls back to Qt.styleHints when the controller is absent.
// Dark = near-black "pure dark" (not slate-gray).
QtObject {
    id: theme

    readonly property bool isLight: {
        if (typeof bearwaveColorScheme !== "undefined" && bearwaveColorScheme)
            return !!bearwaveColorScheme.lightMode
        return Qt.styleHints.colorScheme === Qt.ColorScheme.Light
    }

    // --- surfaces ---
    readonly property color bgA: isLight ? "#f4f4f6" : "#0a0a0b"
    readonly property color bgB: isLight ? "#ececf0" : "#0e0e10"
    readonly property color panel: isLight ? "#ffffff" : "#121214"
    readonly property color panelAlt: isLight ? "#f7f7f9" : "#161618"
    readonly property color sidebar: isLight ? "#f0f0f3" : "#0c0c0e"
    readonly property color card: isLight ? "#ffffff" : "#141416"
    readonly property color cardHover: isLight ? "#e8e8ed" : "#1c1c20"
    readonly property color cardBorder: isLight ? "#d4d4dc" : "#2a2a30"
    readonly property color selection: isLight ? "#ffe4ee" : "#24242a"
    readonly property color selectionHover: isLight ? "#ffd6e6" : "#2e2e36"
    readonly property color selectionBorder: isLight ? "#ff4f86" : "#3a3a44"
    readonly property color imageWell: isLight ? "#e4e4ea" : "#1a1a1e"

    // Brand accent stays the same in both modes (slightly stronger on light for contrast).
    readonly property color accent: isLight ? "#e63d72" : "#ff4f86"
    readonly property color textMain: isLight ? "#141418" : "#f2f2f5"
    readonly property color textMuted: isLight ? "#5c5c66" : "#9898a0"
    readonly property color warn: isLight ? "#c62828" : "#ff6b6b"

    readonly property var worldTags: [
        { name: qsTr("Pop"), tag: "pop", icon: "🎵" },
        { name: qsTr("Rock"), tag: "rock", icon: "🎸" },
        { name: qsTr("Electronic"), tag: "electronic", icon: "🎹" },
        { name: qsTr("Classical"), tag: "classical", icon: "🎻" },
        { name: qsTr("Jazz"), tag: "jazz", icon: "🎷" },
        { name: qsTr("Metal"), tag: "metal", icon: "🤘" },
        { name: qsTr("Hip Hop"), tag: "hiphop", icon: "🎤" },
        { name: qsTr("Chillout"), tag: "chillout", icon: "🌴" },
        { name: qsTr("News / Talk"), tag: "news", icon: "📻" },
        { name: qsTr("Soundtracks"), tag: "soundtrack", icon: "🎬" },
        { name: qsTr("Ambient"), tag: "ambient", icon: "🌌" },
        { name: qsTr("Blues / Soul"), tag: "blues", icon: "🎺" }
    ]
}
