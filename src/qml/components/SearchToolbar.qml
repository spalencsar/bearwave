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

    function runSearch() {
        if (searchField.text.length < 2 || !app.backend)
            return
        app.currentPage = "search"
        app.backend.searchStations(searchField.text)
    }

    function clearSearch() {
        searchField.text = ""
        if (app.backend)
            app.backend.filterQuery = ""
    }

    RowLayout {
        visible: !root.compactMode
        Layout.fillWidth: true
        spacing: 10

        TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.minimumWidth: 180
            placeholderText: qsTr("Search stations (name, genre, country)...")
            onTextChanged: {
                if (app.backend) {
                    app.backend.filterQuery = text
                }
                searchTimer.restart()
            }
            onAccepted: root.runSearch()
        }

        RowLayout {
            spacing: 6

            ThemedButton {
                text: root.app.width < 1180 ? "🔍" : "🔍  " + qsTr("Search")
                primary: true
                Layout.preferredWidth: root.app.width < 1180 ? 44 : 88
                onClicked: root.runSearch()
            }

            ThemedButton {
                text: root.app.width < 1180 ? "✕" : "✕  " + qsTr("Clear")
                Layout.preferredWidth: root.app.width < 1180 ? 44 : 82
                onClicked: root.clearSearch()
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
            onAccepted: root.runSearch()
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            ThemedButton {
                text: "🔍  " + qsTr("Search")
                primary: true
                Layout.fillWidth: true
                onClicked: root.runSearch()
            }

            ThemedButton {
                text: "✕  " + qsTr("Clear")
                Layout.fillWidth: true
                onClicked: root.clearSearch()
            }
        }
    }
}
