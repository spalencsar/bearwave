// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property color bgA: "#0f141b"
    readonly property color bgB: "#131b25"
    readonly property color panel: "#182433"
    readonly property color card: "#1b2a3d"
    readonly property color cardHover: "#223654"
    readonly property color cardBorder: "#2d4566"
    readonly property color accent: "#2bb0ff"
    readonly property color textMain: "#eaf1fb"
    readonly property color textMuted: "#9eb1c9"
    readonly property color warn: "#ff8b8b"

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