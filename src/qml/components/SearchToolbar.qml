// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

// Soft pill search bar, not stock Fusion/KDE TextField chrome.
ColumnLayout {
    id: root

    required property var app
    required property var searchTimer
    required property bool compactMode

    property alias searchField: searchField

    spacing: 0

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
        app.currentPage = "top"
        if (app.backend)
            app.backend.loadTopStations()
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 10

        // Pill field
        Rectangle {
            id: searchShell
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            radius: 21
            color: BearTheme.isLight ? "#ebebf0" : "#1a1a1e"
            border.width: searchField.activeFocus ? 1.5 : 1
            border.color: searchField.activeFocus ? BearTheme.accent : BearTheme.cardBorder

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 8
                spacing: 8

                Label {
                    text: "🔍"
                    font.pixelSize: 15
                    font.family: "Noto Color Emoji, Noto Sans, sans-serif"
                    opacity: 0.85
                }

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    verticalAlignment: TextInput.AlignVCenter
                    leftPadding: 0
                    rightPadding: 0
                    topPadding: 0
                    bottomPadding: 0
                    color: BearTheme.textMain
                    placeholderTextColor: BearTheme.textMuted
                    placeholderText: qsTr("Search stations, genre, country…")
                    font.pixelSize: 14
                    selectByMouse: true
                    background: Item {}

                    onTextChanged: {
                        if (app.backend)
                            app.backend.filterQuery = text
                        searchTimer.restart()
                    }
                    onAccepted: root.runSearch()
                    Keys.onEscapePressed: root.clearSearch()
                }

                // Inline clear
                Rectangle {
                    visible: searchField.text.length > 0
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 28
                    radius: 14
                    color: clearMa.containsMouse ? BearTheme.cardHover : "transparent"

                    Label {
                        anchors.centerIn: parent
                        text: "✕"
                        color: BearTheme.textMuted
                        font.pixelSize: 12
                        font.bold: true
                    }
                    MouseArea {
                        id: clearMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.clearSearch()
                    }
                }
            }
        }

        // Primary search action
        Rectangle {
            Layout.preferredHeight: 42
            Layout.preferredWidth: Math.max(searchBtnLabel.implicitWidth + 28, 88)
            radius: 21
            color: searchMa.containsMouse || searchMa.pressed ? Qt.darker(BearTheme.accent, 1.1) : BearTheme.accent

            Label {
                id: searchBtnLabel
                anchors.centerIn: parent
                text: qsTr("Search")
                color: "#ffffff"
                font.pixelSize: 13
                font.bold: true
            }
            MouseArea {
                id: searchMa
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.runSearch()
            }
        }
    }
}
