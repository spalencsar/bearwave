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

    spacing: 8

    RowLayout {
        visible: !root.compactMode
        Layout.fillWidth: true
        spacing: 8

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Search stations (name, genre, country)")
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

        Button {
            text: qsTr("Search")
            highlighted: true
            onClicked: {
                if (searchField.text.length < 2 || !app.backend) return
                app.currentPage = "search"
                app.backend.searchStations(searchField.text)
            }
        }

        Button {
            text: qsTr("Sort A-Z")
            onClicked: {
                if (app.backend && app.currentPage !== "favorites") {
                    app.backend.sortStations("name")
                }
            }
        }

        Button {
            text: qsTr("Sort Bitrate")
            onClicked: {
                if (app.backend && app.currentPage !== "favorites") {
                    app.backend.sortStations("bitrate")
                }
            }
        }

        Button {
            text: qsTr("Sort Votes")
            onClicked: {
                if (app.backend && app.currentPage !== "favorites") {
                    app.backend.sortStations("votes")
                }
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

            Button {
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

            Button {
                text: "A-Z"
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("name")
                    }
                }
            }

            Button {
                text: "kb"
                onClicked: {
                    if (app.backend && app.currentPage !== "favorites") {
                        app.backend.sortStations("bitrate")
                    }
                }
            }

            Button {
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
        Layout.fillWidth: true
        text: qsTr("Tip: you are not limited to DE/NL. Search worldwide by country, genre, or station name.")
        color: BearTheme.textMuted
        font.pixelSize: 11
        wrapMode: root.compactMode ? Text.WordWrap : Text.NoWrap
        elide: root.compactMode ? Text.ElideNone : Text.ElideRight
    }
}