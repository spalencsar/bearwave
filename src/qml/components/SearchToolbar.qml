// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

ColumnLayout {
    id: root

    required property var app
    required property var searchTimer
    required property bool compactMode

    property alias searchField: searchField

    spacing: 0

    RowLayout {
        visible: !root.compactMode
        Layout.fillWidth: true
        spacing: 8

        TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            placeholderText: qsTr("Station, genre, country search")
            onTextChanged: {
                if (app.backend) {
                    app.backend.filterQuery = text
                }
                searchTimer.restart()
            }
            onAccepted: {
                if (text.length < 2 || !app.backend) return
                app.currentPage = "search"
                app.backend.searchStations(text)
            }
        }

        AppButton {
            text: qsTr("Search")
            Layout.preferredHeight: 30
            onClicked: {
                if (searchField.text.length < 2 || !app.backend) return
                app.currentPage = "search"
                app.backend.searchStations(searchField.text)
            }
        }

        AppButton {
            text: qsTr("Clear")
            Layout.preferredHeight: 30
            enabled: searchField.text.length > 0
            onClicked: {
                searchField.text = ""
                if (app.backend) {
                    app.backend.filterQuery = ""
                }
                app.currentPage = "top"
                if (app.backend) app.backend.loadTopStations()
            }
        }
    }

    ColumnLayout {
        visible: root.compactMode
        Layout.fillWidth: true
        spacing: 8

        TextField {
            id: compactSearchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search stations (name, genre, country)")
            text: searchField.text
            onTextChanged: {
                if (searchField.text !== text) {
                    searchField.text = text
                }
                if (app.backend) {
                    app.backend.filterQuery = text
                }
                searchTimer.restart()
            }
            onAccepted: {
                if (text.length < 2 || !app.backend) return
                app.currentPage = "search"
                app.backend.searchStations(text)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Item { Layout.fillWidth: true }

            AppButton {
                text: qsTr("Search")
                highlighted: true
                onClicked: {
                    if (compactSearchField.text.length < 2 || !app.backend) return
                    app.currentPage = "search"
                    app.backend.searchStations(compactSearchField.text)
                }
            }
        }

        Flow {
            Layout.fillWidth: true
            width: parent.width
            spacing: 8

            AppButton {
                text: "A-Z"
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("name")
                    }
                }
            }

            AppButton {
                text: "kb"
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("bitrate")
                    }
                }
            }

            AppButton {
                text: "❤"
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("votes")
                    }
                }
            }
        }
    }

    Label {
        visible: false
        Layout.fillWidth: true
        text: ""
        color: BearTheme.textMuted
        font.pixelSize: 11
        wrapMode: root.compactMode ? Text.WordWrap : Text.NoWrap
        elide: root.compactMode ? Text.ElideNone : Text.ElideRight
    }
}
