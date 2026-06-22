// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Dialog {
    id: root

    required property bool compactMode
    property string appVersion: Qt.application.version
    property string buildId: "?"

    modal: true
    anchors.centerIn: parent
    width: compactMode ? 360 : 520
    height: compactMode ? 560 : 680
    standardButtons: Dialog.Ok

    contentItem: ColumnLayout {
        spacing: 16

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72
                sourceSize.width: 72
                sourceSize.height: 72
                source: "qrc:/assets/app/bearwave.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("BearWave")
                color: BearTheme.textMain
                font.bold: true
                font.pixelSize: 26
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Internet Radio Player for KDE")
                color: BearTheme.textMuted
                font.pixelSize: 14
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Version %1 · build %2").arg(String(root.appVersion), String(root.buildId))
                color: BearTheme.textMuted
                font.pixelSize: 12
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8
            Layout.topMargin: 8
            Layout.bottomMargin: 8

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Author: Sebastian Palencsár")
                color: BearTheme.textMain
                font.bold: true
                font.pixelSize: 14
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 16

                ToolButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    onClicked: Qt.openUrlExternally("https://palencsar.pro")
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Open website")
                    contentItem: Item {
                        Image {
                            anchors.centerIn: parent
                            width: 19
                            height: 19
                            source: "qrc:/assets/ui/globe.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }
                    background: Rectangle {
                        radius: 20
                        color: parent.hovered ? "#274261" : "#1f3147"
                        border.color: BearTheme.cardBorder
                        border.width: 1
                    }
                }

                ToolButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    onClicked: Qt.openUrlExternally("https://github.com/spalencsar")
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Open GitHub")
                    contentItem: Item {
                        Image {
                            anchors.centerIn: parent
                            width: 19
                            height: 19
                            source: "qrc:/assets/ui/github.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }
                    background: Rectangle {
                        radius: 20
                        color: parent.hovered ? "#274261" : "#1f3147"
                        border.color: BearTheme.cardBorder
                        border.width: 1
                    }
                }

                ToolButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    onClicked: Qt.openUrlExternally("https://www.linkedin.com/in/spalencsar/")
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Open LinkedIn")
                    contentItem: Item {
                        Image {
                            anchors.centerIn: parent
                            width: 19
                            height: 19
                            source: "qrc:/assets/ui/linkedin.svg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }
                    background: Rectangle {
                        radius: 20
                        color: parent.hovered ? "#274261" : "#1f3147"
                        border.color: BearTheme.cardBorder
                        border.width: 1
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                Label {
                    Layout.fillWidth: true
                    text: qsTr("GNU GPLv3 License")
                    color: BearTheme.textMain
                    font.bold: true
                }
                Label {
                    text: qsTr("Copyright (c) 2026")
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 140
                radius: 8
                color: "#101a26"
                border.color: BearTheme.cardBorder

                ScrollView {
                    id: licenseScroll
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                    Label {
                        width: licenseScroll.availableWidth
                        color: BearTheme.textMain
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        text: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>."
                    }
                }
            }
        }
    }
}