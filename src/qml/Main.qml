import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform

ApplicationWindow {
    id: root
    title: "BearWave"
    width: 980
    height: 700
    minimumWidth: 520
    minimumHeight: 460
    visible: true

    onClosing: function(close) {
        close.accepted = false
        root.hide()
    }

    Platform.SystemTrayIcon {
        id: systray
        visible: true
        icon.name: "multimedia-player"
        tooltip: "BearWave"

        menu: Platform.Menu {
            Platform.MenuItem {
                text: backend && backend.player && backend.player.playing ? "Pause" : "Play"
                onTriggered: {
                    if (backend && backend.player) {
                        backend.player.togglePlayPause()
                    }
                }
            }
            Platform.MenuItem {
                text: root.visible ? "Verstecken" : "Zeigen"
                onTriggered: {
                    if (root.visible) {
                        root.hide()
                    } else {
                        root.show()
                        root.raise()
                        root.requestActivate()
                    }
                }
            }
            Platform.MenuSeparator {}
            Platform.MenuItem {
                text: "Beenden"
                onTriggered: Qt.quit()
            }
        }

        onActivated: function(reason) {
            if (reason === Platform.SystemTrayIcon.Trigger) {
                if (root.visible) {
                    root.hide()
                } else {
                    root.show()
                    root.raise()
                    root.requestActivate()
                }
            }
        }
    }

    property var currentPage: "top"
    property var backend: (typeof radioBackend !== "undefined" ? radioBackend : null)
    property bool compactMode: width < 780
    property real contentOpacity: 1.0
    property string activeQuickFilter: ""

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

    function toast(message) {
        toastLabel.text = message
        toastPopup.open()
    }

    function activeModel() {
        if (!backend) {
            return []
        }
        return currentPage === "favorites" ? backend.favoriteStations : backend.stations
    }

    onCurrentPageChanged: {
        contentOpacity = 0.85
        contentFadeRestart.restart()
    }

    Shortcut {
        sequence: "Space"
        onActivated: {
            if (backend && backend.player) {
                backend.player.togglePlayPause()
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+F"
        onActivated: searchField.forceActiveFocus()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: bgA }
            GradientStop { position: 1.0; color: bgB }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: compactMode ? 208 : 176
            radius: 12
            color: panel
            border.color: cardBorder

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: "BearWave"
                        color: textMain
                        font.pixelSize: compactMode ? 18 : 22
                        font.bold: true
                    }

                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                        color: cardBorder
                        opacity: 0.6
                    }

                    Button {
                        text: "Top"
                        highlighted: currentPage === "top"
                        onClicked: {
                            if (!backend) return
                            currentPage = "top"
                            activeQuickFilter = ""
                            backend.loadTopStations()
                        }
                    }

                    Button {
                        text: "DE"
                        highlighted: currentPage === "german"
                        onClicked: {
                            if (!backend) return
                            currentPage = "german"
                            activeQuickFilter = ""
                            backend.loadGermanStations()
                        }
                    }

                    Button {
                        text: "NL"
                        highlighted: currentPage === "dutch"
                        onClicked: {
                            if (!backend) return
                            currentPage = "dutch"
                            activeQuickFilter = ""
                            backend.loadDutchStations()
                        }
                    }

                    Button {
                        text: "Favoriten"
                        highlighted: currentPage === "favorites"
                        onClicked: {
                            currentPage = "favorites"
                            activeQuickFilter = ""
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: compactMode ? "+" : "Manuell +"
                        onClicked: addDialog.open()
                    }

                    Button {
                        text: "About"
                        onClicked: aboutDialog.open()
                    }

                    Button {
                        visible: backend && backend.canResumeLastStation
                        text: compactMode ? "↺" : "Resume"
                        onClicked: {
                            if (backend) {
                                backend.resumeLastStation()
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: "Sender suchen (Name, Genre, Land)"
                        onTextChanged: {
                            if (backend) {
                                backend.filterQuery = text
                            }
                        }
                        onAccepted: {
                            if (text.length < 2 || !backend) return
                            currentPage = "search"
                            backend.searchStations(text)
                        }
                    }

                    Button {
                        text: "Suche"
                        highlighted: true
                        onClicked: {
                            if (searchField.text.length < 2 || !backend) return
                            currentPage = "search"
                            backend.searchStations(searchField.text)
                        }
                    }

                    Button {
                        text: compactMode ? "A-Z" : "Sort A-Z"
                        onClicked: {
                            if (backend && currentPage !== "favorites") {
                                backend.sortStations("name")
                            }
                        }
                    }

                    Button {
                        text: compactMode ? "kb" : "Sort Bitrate"
                        onClicked: {
                            if (backend && currentPage !== "favorites") {
                                backend.sortStations("bitrate")
                            }
                        }
                    }

                    Button {
                        text: compactMode ? "❤" : "Sort Votes"
                        onClicked: {
                            if (backend && currentPage !== "favorites") {
                                backend.sortStations("votes")
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    text: "Tipp: Du bist nicht auf DE/NL begrenzt - suche nach Ländern, Genres oder Sendernamen weltweit."
                    color: textMuted
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Label {
                        text: "Genre:"
                        color: textMuted
                        font.pixelSize: 11
                    }

                    Button {
                        text: "Rock"
                        highlighted: activeQuickFilter === "tag:rock"
                        onClicked: {
                            if (!backend) return
                            currentPage = "genre-rock"
                            activeQuickFilter = "tag:rock"
                            backend.loadByTag("rock")
                        }
                    }

                    Button {
                        text: "News"
                        highlighted: activeQuickFilter === "tag:news"
                        onClicked: {
                            if (!backend) return
                            currentPage = "genre-news"
                            activeQuickFilter = "tag:news"
                            backend.loadByTag("news")
                        }
                    }

                    Button {
                        text: "Jazz"
                        highlighted: activeQuickFilter === "tag:jazz"
                        onClicked: {
                            if (!backend) return
                            currentPage = "genre-jazz"
                            activeQuickFilter = "tag:jazz"
                            backend.loadByTag("jazz")
                        }
                    }

                    Item { Layout.preferredWidth: 10 }

                    Label {
                        text: "Land:"
                        color: textMuted
                        font.pixelSize: 11
                    }

                    Button {
                        text: "US"
                        highlighted: activeQuickFilter === "cc:US"
                        onClicked: {
                            if (!backend) return
                            currentPage = "country-us"
                            activeQuickFilter = "cc:US"
                            backend.loadByCountryCode("US")
                        }
                    }

                    Button {
                        text: "UK"
                        highlighted: activeQuickFilter === "cc:GB"
                        onClicked: {
                            if (!backend) return
                            currentPage = "country-gb"
                            activeQuickFilter = "cc:GB"
                            backend.loadByCountryCode("GB")
                        }
                    }

                    Button {
                        text: "FR"
                        highlighted: activeQuickFilter === "cc:FR"
                        onClicked: {
                            if (!backend) return
                            currentPage = "country-fr"
                            activeQuickFilter = "cc:FR"
                            backend.loadByCountryCode("FR")
                        }
                    }

                    Button {
                        text: "WORLD"
                        highlighted: activeQuickFilter === "world"
                        onClicked: {
                            if (!backend) return
                            currentPage = "world"
                            activeQuickFilter = "world"
                            backend.loadWorldStations()
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                Label {
                    Layout.fillWidth: true
                    visible: backend && backend.lastError.length > 0
                    text: backend ? ("Fehler: " + backend.lastError) : ""
                    color: warn
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: panel
            border.color: cardBorder
            clip: true
            opacity: contentOpacity

            Behavior on opacity {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            ScrollView {
                id: stationScrollView
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                ListView {
                    id: stationList
                    width: stationScrollView.availableWidth
                    spacing: 8
                    model: activeModel()
                    visible: count > 0

                    delegate: Rectangle {
                        id: stationCard
                        required property int index
                        required property var modelData
                        width: stationScrollView.availableWidth
                        height: compactMode ? 72 : 78
                        radius: 10
                    color: cardMouse.containsMouse ? cardHover : card
                    border.color: cardBorder

                    MouseArea {
                        id: cardMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: {
                            if (!backend) return
                            if (currentPage === "favorites") {
                                backend.playFavoriteStation(index)
                            } else {
                                backend.playStation(index)
                            }
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 44
                            Layout.preferredHeight: 44
                            radius: 8
                            color: "#123154"
                            border.color: accent

                            Image {
                                anchors.fill: parent
                                anchors.margins: 3
                                source: (stationCard.modelData.favicon && stationCard.modelData.favicon.startsWith("https://"))
                                        ? stationCard.modelData.favicon
                                        : ""
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                cache: true
                                visible: source !== "" && status === Image.Ready
                            }

                            Label {
                                anchors.centerIn: parent
                                text: "FM"
                                color: "#d8ecff"
                                font.bold: true
                                visible: !parent.children[0].visible
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Label {
                                Layout.fillWidth: true
                                text: stationCard.modelData.name
                                color: textMain
                                font.bold: true
                                font.pixelSize: compactMode ? 13 : 14
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                text: stationCard.modelData.country + "  •  "
                                      + (stationCard.modelData.bitrate > 0 ? stationCard.modelData.bitrate + " kbps" : "Stream")
                                color: textMuted
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                        }

                        Button {
                            Layout.preferredWidth: compactMode ? 34 : 40
                            Layout.preferredHeight: 40
                            text: stationCard.modelData.isFavorite ? "★" : "☆"
                            onClicked: {
                                if (!backend) return
                                backend.toggleFavoriteById(stationCard.modelData.uuid, stationCard.modelData.urlResolved)
                                toast(stationCard.modelData.isFavorite ? "Aus Favoriten entfernt" : "Zu Favoriten hinzugefügt")
                            }
                        }

                        Button {
                            Layout.preferredWidth: compactMode ? 34 : 40
                            Layout.preferredHeight: 40
                            text: "▶"
                            onClicked: {
                                if (!backend) return
                                if (currentPage === "favorites") {
                                    backend.playFavoriteStation(index)
                                } else {
                                    backend.playStation(index)
                                }
                            }
                        }
                    }
                }
            }
            }

            Column {
                anchors.centerIn: parent
                spacing: 8
                visible: stationList.count === 0

                Label {
                    text: "Noch keine Sender geladen"
                    color: textMain
                    font.bold: true
                }

                Label {
                    text: "Lade DE/NL oder nutze die Suche"
                    color: textMuted
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 108
            radius: 12
            color: panel
            border.color: cardBorder

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 12

                Rectangle {
                    Layout.preferredWidth: 88
                    Layout.preferredHeight: 88
                    radius: 8
                    color: "#182637"
                    clip: true
                    border.color: cardBorder

                    Image {
                        id: coverImage
                        anchors.fill: parent
                        source: backend && backend.player && backend.player.currentCoverArtUrl ? backend.player.currentCoverArtUrl : "qrc:/bearwave.png"
                        fillMode: backend && backend.player && backend.player.currentCoverArtUrl ? Image.PreserveAspectCrop : Image.PreserveAspectFit
                        smooth: true
                        asynchronous: true
                        anchors.margins: backend && backend.player && backend.player.currentCoverArtUrl ? 0 : 16
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 6

                    Label {
                        Layout.fillWidth: true
                        text: backend && backend.player ? (backend.player.currentStationName || "Kein Sender ausgewählt") : "Kein Sender ausgewählt"
                        color: textMain
                        font.pixelSize: 16
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: backend && backend.player && backend.player.currentNowPlaying.length > 0
                              ? ("Jetzt läuft: " + backend.player.currentNowPlaying)
                              : "Jetzt läuft: Keine Titelinfo"
                        color: textMuted
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            text: "⏮"
                            Layout.preferredWidth: 44
                            enabled: backend ? backend.hasPreviousStation() : false
                            onClicked: {
                                if (backend) {
                                    backend.playPreviousStation()
                                }
                            }
                        }

                        Button {
                            text: backend && backend.player && backend.player.playing ? "⏸" : "▶"
                            Layout.preferredWidth: 44
                            onClicked: {
                                if (backend && backend.player) {
                                    backend.player.togglePlayPause()
                                }
                            }
                        }

                        Button {
                            text: "⏹"
                            Layout.preferredWidth: 44
                            onClicked: {
                                if (backend && backend.player) {
                                    backend.player.stop()
                                }
                            }
                        }

                        Button {
                            text: "⏭"
                            Layout.preferredWidth: 44
                            enabled: backend ? backend.hasNextStation() : false
                            onClicked: {
                                if (backend) {
                                    backend.playNextStation()
                                }
                            }
                        }

                        Label {
                            text: "Lautstärke"
                            color: textMuted
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: 1
                            value: backend && backend.player ? backend.player.volume : 0.5
                            onMoved: {
                                if (backend && backend.player) {
                                    backend.player.setVolume(value)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        visible: backend && backend.loading
        anchors.fill: parent
        color: "#7f0b121a"
        z: 20

        Rectangle {
            anchors.centerIn: parent
            width: 210
            height: 110
            radius: 12
            color: panel
            border.color: cardBorder

            Column {
                anchors.centerIn: parent
                spacing: 10

                BusyIndicator {
                    running: true
                    width: 36
                    height: 36
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Label {
                    text: "Lade Sender..."
                    color: textMain
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    Component.onCompleted: {
        if (backend) {
            backend.loadTopStations()
        }
    }

    Dialog {
        id: addDialog
        title: "Sender manuell hinzufügen"
        modal: true
        anchors.centerIn: parent
        width: compactMode ? 320 : 420
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: ColumnLayout {
            spacing: 8
            TextField { id: manualName; Layout.fillWidth: true; placeholderText: "Name" }
            TextField { id: manualUrl; Layout.fillWidth: true; placeholderText: "Stream URL (http/https)" }
            TextField { id: manualCountry; Layout.fillWidth: true; placeholderText: "Land (optional)" }
        }

        onAccepted: {
            if (backend) {
                backend.addManualStation(manualName.text, manualUrl.text, manualCountry.text)
                toast("Sender hinzugefügt")
            }
            manualName.text = ""
            manualUrl.text = ""
            manualCountry.text = ""
        }
    }

    Dialog {
        id: aboutDialog
        modal: true
        anchors.centerIn: Overlay.overlay
        width: compactMode ? 360 : 520
        height: compactMode ? 560 : 680
        standardButtons: Dialog.Ok

        contentItem: ColumnLayout {
            spacing: 16

            // --- HEADER ---
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 6

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 72
                    sourceSize.width: 72
                    sourceSize.height: 72
                    source: "qrc:/bearwave.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "BearWave"
                    color: textMain
                    font.bold: true
                    font.pixelSize: 26
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Internet Radio Player for KDE"
                    color: textMuted
                    font.pixelSize: 14
                }
            }

            // --- CREDITS & LINKS ---
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8
                Layout.topMargin: 8
                Layout.bottomMargin: 8

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Autor: Sebastian Palencsár"
                    color: textMain
                    font.bold: true
                    font.pixelSize: 14
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 16

                    ToolButton {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        icon.source: "qrc:/globe.svg"
                        icon.width: 19
                        icon.height: 19
                        onClicked: Qt.openUrlExternally("https://palencsar.pro")
                        ToolTip.visible: hovered
                        ToolTip.text: "Website öffnen"
                        background: Rectangle {
                            radius: 20
                            color: parent.hovered ? "#274261" : "#1f3147"
                            border.color: cardBorder
                            border.width: 1
                        }
                    }

                    ToolButton {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        icon.source: "qrc:/github.svg"
                        icon.width: 19
                        icon.height: 19
                        onClicked: Qt.openUrlExternally("https://github.com/spalencsar")
                        ToolTip.visible: hovered
                        ToolTip.text: "GitHub öffnen"
                        background: Rectangle {
                            radius: 20
                            color: parent.hovered ? "#274261" : "#1f3147"
                            border.color: cardBorder
                            border.width: 1
                        }
                    }

                    ToolButton {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        icon.source: "qrc:/linkedin.svg"
                        icon.width: 19
                        icon.height: 19
                        onClicked: Qt.openUrlExternally("https://www.linkedin.com/in/spalencsar/")
                        ToolTip.visible: hovered
                        ToolTip.text: "LinkedIn öffnen"
                        background: Rectangle {
                            radius: 20
                            color: parent.hovered ? "#274261" : "#1f3147"
                            border.color: cardBorder
                            border.width: 1
                        }
                    }
                }
            }

            // --- LICENSE ---
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: "MIT License"
                        color: textMain
                        font.bold: true
                    }
                    Label {
                        text: "Copyright (c) 2026"
                        color: textMuted
                        font.pixelSize: 11
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 140
                    radius: 8
                    color: "#101a26"
                    border.color: cardBorder

                    ScrollView {
                        id: licenseScroll
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                        Label {
                            width: licenseScroll.availableWidth
                            color: textMain
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            text: "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: toastPopup
        x: (root.width - width) / 2
        y: root.height - height - 20
        padding: 10
        closePolicy: Popup.NoAutoClose
        background: Rectangle {
            radius: 10
            color: "#24364e"
            border.color: accent
        }

        contentItem: Label {
            id: toastLabel
            color: textMain
            font.pixelSize: 12
        }

        Timer {
            interval: 1400
            running: toastPopup.visible
            repeat: false
            onTriggered: toastPopup.close()
        }
    }

    Timer {
        id: contentFadeRestart
        interval: 120
        repeat: false
        onTriggered: contentOpacity = 1.0
    }
}
