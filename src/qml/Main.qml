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
    readonly property string appVersion: (typeof bearwaveVersion !== "undefined" ? bearwaveVersion : Qt.application.version)
    readonly property string appBuildId: (typeof bearwaveBuildId !== "undefined" ? bearwaveBuildId : "?")
    property bool compactMode: width < 780
    property real contentOpacity: 1.0
    property string activeQuickFilter: ""
    property string selectedWorldCategory: ""
    property string selectedWorldType: ""
    property string countrySearchText: ""
    property alias searchField: searchToolbar.searchField
    property alias stationList: stationPanel.stationList

    function toast(message) {
        toastPopup.show(message)
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

                StationListPanel {
                    id: stationPanel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    app: root
                }

                WorldCategories {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: currentPage === "world" && selectedWorldCategory === ""
                    app: root
                    compactMode: root.compactMode
                }
            }
        }

        PlayerBar {
            Layout.fillWidth: true
            app: root
        }
    }

    LoadingOverlay {
        app: root
    }

    Component.onCompleted: {
        if (backend) {
            backend.loadTopStations()
        }
    }

    AddStationDialog {
        id: addDialog
        app: root
        compactMode: root.compactMode
    }

    EditStationDialog {
        id: editDialog
        app: root
        compactMode: root.compactMode
    }

    AboutDialog {
        id: aboutDialog
        compactMode: root.compactMode
        appVersion: root.appVersion
        buildId: root.appBuildId
    }

    ToastPopup {
        id: toastPopup
        window: root
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
