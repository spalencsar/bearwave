// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Item {
    id: root

    required property bool compactMode
    property string appVersion: Qt.application.version
    property string buildId: "?"
    property string licenseText: ""

    readonly property string versionLine: {
        var version = root.appVersion
        if (version === undefined || version === null || version === "")
            version = Qt.application.version
        var build = root.buildId
        if (build === undefined || build === null || build === "")
            build = "?"
        return qsTr("Version") + " " + String(version) + " · " + qsTr("Build") + " " + String(build)
    }

    readonly property var thirdPartyItems: [
        {
            name: qsTr("Qt 6"),
            usage: qsTr("Application framework and UI runtime: Core, DBus, Network, Quick, Quick Controls 2, Widgets, Multimedia, and LinguistTools."),
            license: qsTr("Subject to the upstream licenses of the installed Qt distribution.")
        },
        {
            name: qsTr("Qt Multimedia backend"),
            usage: qsTr("Audio playback runs through Qt Multimedia and the backend available on the system, such as GStreamer or FFmpeg."),
            license: qsTr("Backend components are provided by the operating system or Qt build and retain their own licenses.")
        },
        {
            name: qsTr("Radio Browser API"),
            usage: qsTr("Public station directory API for station data and stream discovery. BearWave does not bundle any Radio Browser server code."),
            license: qsTr("External service or API; subject to the upstream project and service terms.")
        },
        {
            name: qsTr("freedesktop.org integrations"),
            usage: qsTr("D-Bus, MPRIS, desktop notifications, desktop entry, and AppStream metadata."),
            license: qsTr("Uses standards and interfaces for desktop integration; no third-party implementation is bundled.")
        }
    ]

    ScrollView {
        id: pageScroll
        anchors.fill: parent
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        ColumnLayout {
            width: Math.max(pageScroll.availableWidth, 1)
            spacing: root.compactMode ? 18 : 22

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: root.compactMode ? 18 : 34
                Layout.leftMargin: root.compactMode ? 18 : 46
                Layout.rightMargin: root.compactMode ? 18 : 46
                spacing: 16

                Image {
                    Layout.preferredWidth: root.compactMode ? 58 : 72
                    Layout.preferredHeight: root.compactMode ? 58 : 72
                    sourceSize.width: root.compactMode ? 58 : 72
                    sourceSize.height: root.compactMode ? 58 : 72
                    source: "qrc:/assets/app/bearwave.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Image {
                        Layout.preferredWidth: root.compactMode ? 170 : 210
                        Layout.preferredHeight: root.compactMode ? 40 : 48
                        source: "qrc:/assets/app/bearwave_line.png"
                        fillMode: Image.PreserveAspectFit
                        horizontalAlignment: Image.AlignLeft
                        smooth: true
                        mipmap: true
                    }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Internet radio player for KDE")
                        color: BearTheme.textMain
                        font.pixelSize: root.compactMode ? 14 : 16
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.versionLine
                        color: BearTheme.textMuted
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: root.compactMode ? 18 : 46
                Layout.rightMargin: root.compactMode ? 18 : 46
                Layout.preferredHeight: 1
                color: BearTheme.cardBorder
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: root.compactMode ? 18 : 46
                Layout.rightMargin: root.compactMode ? 18 : 46
                spacing: 10

                Label {
                    text: qsTr("Author")
                    color: BearTheme.textMuted
                    font.bold: true
                    font.pixelSize: 11
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 14

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Sebastian Palencsár")
                        color: BearTheme.textMain
                        font.bold: true
                        font.pixelSize: 15
                        elide: Text.ElideRight
                    }

                    ToolButton {
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 30
                        onClicked: Qt.openUrlExternally("https://palencsar.pro")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open website")
                        contentItem: Item {
                            Image {
                                anchors.centerIn: parent
                                width: 18
                                height: 18
                                source: "qrc:/assets/ui/globe.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }
                    }

                    ToolButton {
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 30
                        onClicked: Qt.openUrlExternally("https://github.com/spalencsar")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open GitHub")
                        contentItem: Item {
                            Image {
                                anchors.centerIn: parent
                                width: 18
                                height: 18
                                source: "qrc:/assets/ui/github.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }
                    }

                    ToolButton {
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 30
                        onClicked: Qt.openUrlExternally("https://www.linkedin.com/in/spalencsar/")
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Open LinkedIn")
                        contentItem: Item {
                            Image {
                                anchors.centerIn: parent
                                width: 18
                                height: 18
                                source: "qrc:/assets/ui/linkedin.svg"
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: root.compactMode ? 18 : 46
                Layout.rightMargin: root.compactMode ? 18 : 46
                Layout.preferredHeight: 1
                color: BearTheme.cardBorder
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: root.compactMode ? 18 : 46
                Layout.rightMargin: root.compactMode ? 18 : 46
                spacing: 10

                Label {
                    text: qsTr("Technologies and third-party components")
                    color: BearTheme.textMain
                    font.bold: true
                    font.pixelSize: 14
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("BearWave uses the following frameworks, services, and desktop standards. These components are not relicensed by BearWave.")
                    color: BearTheme.textMuted
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }

                Repeater {
                    model: root.thirdPartyItems

                    delegate: Rectangle {
                        required property var modelData

                        Layout.fillWidth: true
                        implicitHeight: thirdPartyRow.implicitHeight + 16
                        radius: 6
                        color: "#24252b"
                        border.color: BearTheme.cardBorder
                        border.width: 1

                        RowLayout {
                            id: thirdPartyRow
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 14

                            Label {
                                Layout.preferredWidth: root.compactMode ? 120 : 180
                                text: modelData.name
                                color: BearTheme.textMain
                                font.bold: true
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.usage
                                    color: BearTheme.textMain
                                    font.pixelSize: 12
                                    wrapMode: Text.WordWrap
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.license
                                    color: BearTheme.textMuted
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: root.compactMode ? 18 : 46
                Layout.rightMargin: root.compactMode ? 18 : 46
                Layout.bottomMargin: root.compactMode ? 18 : 34
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("GNU GPLv3 License")
                        color: BearTheme.textMain
                        font.bold: true
                        font.pixelSize: 14
                    }

                    Label {
                        text: qsTr("Copyright (c) 2026")
                        color: BearTheme.textMuted
                        font.pixelSize: 11
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.compactMode ? 240 : 280
                    radius: 6
                    color: "#202127"
                    border.color: BearTheme.cardBorder
                    border.width: 1

                    ScrollView {
                        id: licenseScroll
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                        TextArea {
                            id: licenseTextArea
                            width: Math.max(licenseScroll.availableWidth, 1)
                            readOnly: true
                            selectByMouse: true
                            textFormat: TextEdit.PlainText
                            wrapMode: TextEdit.Wrap
                            text: root.licenseText.length > 0 ? root.licenseText : qsTr("License text could not be loaded.")
                            color: BearTheme.textMain
                            font.family: "monospace"
                            font.pixelSize: 12
                            background: Rectangle {
                                color: "transparent"
                            }
                        }
                    }
                }
            }
        }
    }
}
