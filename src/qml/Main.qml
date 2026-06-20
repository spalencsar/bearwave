// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0
import components 1.0

ApplicationWindow {
    id: root
    title: qsTr("BearWave")
    width: 980
    height: 700
    minimumWidth: 520
    minimumHeight: 460
    visible: true

    onClosing: function(close) {
        close.accepted = false
        root.hide()
    }

    property var currentPage: "top"
    property var backend: (typeof radioBackend !== "undefined" ? radioBackend : null)
    property bool compactMode: width < 780
    property real contentOpacity: 1.0
    property string activeQuickFilter: ""
    property string selectedWorldCategory: ""
    property string selectedWorldType: ""
    property string countrySearchText: ""
    property alias searchField: searchToolbar.searchField

    function toast(message) {
        toastLabel.text = message
        toastPopup.open()
    }

    function resetSearchFilter() {
        searchTimer.stop()
        if (searchField.text !== "") {
            searchField.text = ""
        } else if (backend && backend.filterQuery !== "") {
            backend.filterQuery = ""
        }
    }

    function getFilteredCountries() {
        if (!backend || !backend.countries) return [];
        var query = countrySearchText.toLowerCase().trim();
        var list = [];
        if (query === "") {
            list.push({ name: qsTr("Top Global"), code: "GLOBAL" });
        }
        for (var i = 0; i < backend.countries.length; i++) {
            var c = backend.countries[i];
            if (query === "" || c.name.toLowerCase().indexOf(query) !== -1 || c.code.toLowerCase().indexOf(query) !== -1) {
                list.push(c);
            }
        }
        return list;
    }

    function activeModel() {
        if (!backend) {
            return []
        }
        if (currentPage === "favorites") {
            return backend.favoriteStations
        } else if (currentPage === "history") {
            return backend.recentStations
        } else if (currentPage === "world" && selectedWorldCategory === "") {
            return []
        } else {
            return backend.stations
        }
    }

    onCurrentPageChanged: {
        contentOpacity = 0.85
        contentFadeRestart.restart()
        if (currentPage !== "search") {
            resetSearchFilter()
        }
        if (currentPage !== "world") {
            selectedWorldCategory = ""
            selectedWorldType = ""
            countrySearchText = ""
        }
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
            GradientStop { position: 0.0; color: BearTheme.bgA }
            GradientStop { position: 1.0; color: BearTheme.bgB }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: headerContent.implicitHeight + 20
            radius: 12
            color: BearTheme.panel
            border.color: BearTheme.cardBorder

            ColumnLayout {
                id: headerContent
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                HeaderNavigation {
                    Layout.fillWidth: true
                    app: root
                    compactMode: root.compactMode
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: BearTheme.cardBorder
                    opacity: 0.6
                }

                SearchToolbar {
                    id: searchToolbar
                    Layout.fillWidth: true
                    app: root
                    compactMode: root.compactMode
                    searchTimer: searchTimer
                }

                QuickFilters {
                    Layout.fillWidth: true
                    app: root
                    compactMode: root.compactMode
                }

                Label {
                    Layout.fillWidth: true
                    visible: backend && backend.lastError.length > 0
                    text: backend ? (qsTr("Error: ") + backend.lastError) : ""
                    color: BearTheme.warn
                    elide: Text.ElideRight
                    font.pixelSize: 12
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: BearTheme.panel
            border.color: BearTheme.cardBorder
            clip: true
            opacity: contentOpacity

            Behavior on opacity {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                WorldCategoryHeader {
                    Layout.fillWidth: true
                    visible: currentPage === "world" && selectedWorldCategory !== ""
                    app: root
                }

                ScrollView {
                    id: stationScrollView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: !(currentPage === "world" && selectedWorldCategory === "")
                    clip: true
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                    ListView {
                        id: stationList
                        width: stationScrollView.availableWidth
                        spacing: 8
                        model: activeModel()
                        visible: count > 0

                        delegate: StationCard {
                            app: root
                            compactMode: root.compactMode
                            listWidth: stationScrollView.availableWidth
                        }
                    }
                }

                WorldCategories {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: currentPage === "world" && selectedWorldCategory === ""
                    app: root
                    compactMode: root.compactMode
                }
            }

            Column {
                anchors.centerIn: parent
                spacing: 8
                visible: stationList.count === 0 && !(currentPage === "world" && selectedWorldCategory === "")

                Label {
                    text: currentPage === "history" ? qsTr("No playback history") : qsTr("No stations loaded yet")
                    color: BearTheme.textMain
                    font.bold: true
                }

                Label {
                    text: currentPage === "history" ? qsTr("Play some stations to build history") : qsTr("Load DE/NL stations or use search")
                    color: BearTheme.textMuted
                }
            }
        }

        PlayerBar {
            Layout.fillWidth: true
            app: root
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
            color: BearTheme.panel
            border.color: BearTheme.cardBorder

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
                    text: qsTr("Loading stations...")
                    color: BearTheme.textMain
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
        title: qsTr("Add station manually")
        modal: true
        anchors.centerIn: parent
        width: compactMode ? 320 : 420
        standardButtons: Dialog.Ok | Dialog.Cancel

        contentItem: ColumnLayout {
            spacing: 8
            TextField { id: manualName; Layout.fillWidth: true; placeholderText: qsTr("Name") }
            TextField { id: manualUrl; Layout.fillWidth: true; placeholderText: qsTr("Stream URL (http/https)") }
            TextField { id: manualCountry; Layout.fillWidth: true; placeholderText: qsTr("Country (optional)") }
        }

        onAccepted: {
            if (backend) {
                backend.addManualStation(manualName.text, manualUrl.text, manualCountry.text)
                toast(qsTr("Station added"))
            }
            manualName.text = ""
            manualUrl.text = ""
            manualCountry.text = ""
        }
    }

    Dialog {
        id: editDialog
        title: qsTr("Edit station")
        modal: true
        anchors.centerIn: parent
        width: compactMode ? 320 : 420
        standardButtons: Dialog.Ok | Dialog.Cancel

        property var stationObject: null

        contentItem: ColumnLayout {
            spacing: 8
            TextField { id: editName; Layout.fillWidth: true; placeholderText: qsTr("Name") }
            TextField { id: editUrl; Layout.fillWidth: true; placeholderText: qsTr("Stream URL (http/https)") }
            TextField { id: editCountry; Layout.fillWidth: true; placeholderText: qsTr("Country (optional)") }
        }

        function setupAndOpen() {
            if (stationObject) {
                editName.text = stationObject.name || ""
                editUrl.text = stationObject.url || ""
                editCountry.text = (stationObject.country === qsTr("Manual") ? "" : (stationObject.country || ""))
                open()
            }
        }

        onAccepted: {
            if (backend && stationObject) {
                backend.editManualStation(stationObject, editName.text, editUrl.text, editCountry.text)
                toast(qsTr("Station updated"))
            }
            stationObject = null
            editName.text = ""
            editUrl.text = ""
            editCountry.text = ""
        }

        onRejected: {
            stationObject = null
            editName.text = ""
            editUrl.text = ""
            editCountry.text = ""
        }
    }

    AboutDialog {
        id: aboutDialog
        compactMode: root.compactMode
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
            border.color: BearTheme.accent
        }

        contentItem: Label {
            id: toastLabel
            color: BearTheme.textMain
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
        id: searchTimer
        interval: 600
        repeat: false
        onTriggered: {
            var text = searchField.text.trim()
            if (text.length < 2 || !backend) return

            if (currentPage === "search" || stationList.count === 0) {
                currentPage = "search"
                backend.searchStations(text)
            }
        }
    }

    Timer {
        id: contentFadeRestart
        interval: 120
        repeat: false
        onTriggered: contentOpacity = 1.0
    }
}
