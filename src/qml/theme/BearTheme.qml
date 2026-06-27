// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property color bgA: "#1d1d22"
    readonly property color bgB: "#202027"
    readonly property color panel: "#22232a"
    readonly property color sidebar: "#2b2c33"
    readonly property color card: "#2a2b31"
    readonly property color cardHover: "#33343b"
    readonly property color cardBorder: "#3a3b43"
    readonly property color accent: "#d7d7df"
    readonly property color playingAccent: "#0a84ff"
    readonly property color textMain: "#f0f0f4"
    readonly property color textMuted: "#a3a4ad"
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
