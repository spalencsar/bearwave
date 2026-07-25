// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property color bgA: "#1f1f25"
    readonly property color bgB: "#1b1c22"
    readonly property color panel: "#202129"
    readonly property color panelAlt: "#23242b"
    readonly property color sidebar: "#2a2b31"
    readonly property color card: "#202129"
    readonly property color cardHover: "#2d2e35"
    readonly property color cardBorder: "#33343c"
    readonly property color selection: "#49494a"
    readonly property color selectionHover: "#545455"
    readonly property color selectionBorder: "#606066"
    readonly property color imageWell: "#3a3a40"
    readonly property color accent: "#ff4f86"
    readonly property color textMain: "#eeeeF2"
    readonly property color textMuted: "#a8a8b2"
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
