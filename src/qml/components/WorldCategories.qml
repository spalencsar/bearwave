// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0
import "../utils/FlagUtils.js" as FlagUtils

ScrollView {
    id: root

    required property var app
    required property bool compactMode

    clip: true
    ScrollBar.vertical.policy: ScrollBar.AsNeeded

    Column {
        width: root.availableWidth - 12
        spacing: 16

        RowLayout {
            width: parent.width
            spacing: 10

            Label {
                text: qsTr("Explore World Stations")
                font.bold: true
                font.pixelSize: 16
                color: BearTheme.textMain
                Layout.fillWidth: true
            }

            TextField {
                placeholderText: qsTr("Filter countries...")
                font.pixelSize: 11
                Layout.preferredWidth: compactMode ? 140 : 200
                text: app.countrySearchText
                onTextChanged: app.countrySearchText = text
            }
        }

        Label {
            text: qsTr("Choose a country or music style to find radio stations from all over the world.")
            font.pixelSize: 11
            color: BearTheme.textMuted
            wrapMode: Text.WordWrap
            width: parent.width
            visible: app.countrySearchText === ""
        }

        Rectangle {
            width: parent.width
            height: 1
            color: BearTheme.cardBorder
            opacity: 0.4
            visible: app.countrySearchText === ""
        }

        Label {
            text: app.countrySearchText === "" ? qsTr("Countries") : qsTr("Filtered Countries")
            font.bold: true
            font.pixelSize: 13
            color: BearTheme.accent
        }

        Flow {
            id: countryFlow
            width: parent.width
            spacing: 8

            Repeater {
                model: app.getFilteredCountries()
                delegate: Rectangle {
                    id: countryCard
                    required property var modelData
                    width: compactMode ? (countryFlow.width - 8) / 2 : (countryFlow.width - 16) / 3
                    height: 62
                    radius: 8
                    color: countryMouse.containsMouse ? BearTheme.cardHover : BearTheme.card
                    border.color: countryMouse.containsMouse ? BearTheme.accent : BearTheme.cardBorder
                    border.width: 1
                    scale: countryMouse.containsMouse ? 1.02 : 1.0

                    Behavior on color { ColorAnimation { duration: 100 } }
                    Behavior on border.color { ColorAnimation { duration: 100 } }
                    Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }

                    MouseArea {
                        id: countryMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            app.selectedWorldCategory = modelData.name
                            app.selectedWorldType = "country"
                            if (modelData.code === "GLOBAL") {
                                app.backend.loadWorldStations()
                            } else {
                                app.backend.loadByCountryCode(modelData.code)
                            }
                        }
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 12

                        Label {
                            text: modelData.code === "GLOBAL" ? "🌎" : FlagUtils.flagEmoji(modelData.code)
                            font.pixelSize: 26
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Column {
                            width: parent.width - 38
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Label {
                                width: parent.width
                                text: modelData.name
                                color: BearTheme.textMain
                                font.bold: true
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }

                            Label {
                                width: parent.width
                                text: modelData.code === "GLOBAL" ? qsTr("Top 200") :
                                      (modelData.count ? modelData.count + " " + qsTr("stations") : modelData.code)
                                color: BearTheme.textMuted
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }

        Column {
            width: parent.width
            spacing: 16
            visible: app.countrySearchText === ""

            Rectangle {
                width: parent.width
                height: 1
                color: BearTheme.cardBorder
                opacity: 0.4
            }

            Label {
                text: qsTr("Popular Music Styles")
                font.bold: true
                font.pixelSize: 13
                color: BearTheme.accent
            }

            Flow {
                id: tagFlow
                width: parent.width
                spacing: 8

                Repeater {
                    model: BearTheme.worldTags
                    delegate: Rectangle {
                        id: tagCard
                        required property var modelData
                        width: compactMode ? (tagFlow.width - 8) / 2 : (tagFlow.width - 16) / 3
                        height: 54
                        radius: 8
                        color: tagMouse.containsMouse ? BearTheme.cardHover : BearTheme.card
                        border.color: tagMouse.containsMouse ? BearTheme.accent : BearTheme.cardBorder
                        border.width: 1
                        scale: tagMouse.containsMouse ? 1.02 : 1.0

                        Behavior on color { ColorAnimation { duration: 100 } }
                        Behavior on border.color { ColorAnimation { duration: 100 } }
                        Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }

                        MouseArea {
                            id: tagMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                app.selectedWorldCategory = modelData.name
                                app.selectedWorldType = "tag"
                                app.backend.loadByTag(modelData.tag)
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 12

                            Label {
                                text: modelData.icon
                                font.pixelSize: 22
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Label {
                                width: parent.width - 34
                                text: modelData.name
                                color: BearTheme.textMain
                                font.bold: true
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }
    }
}