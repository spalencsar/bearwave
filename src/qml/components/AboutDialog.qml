// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Dialog {
    id: root

    required property bool compactMode
    required property var languageSettings
    required property string changelogDocument
    property string appVersion: Qt.application.version
    property string buildId: "?"
    property bool showingChangelog: false
    readonly property var languageOptions: [
        {
            code: "system",
            label: qsTr("System default")
        },
        {
            code: "de",
            label: "Deutsch"
        },
        {
            code: "en",
            label: "English"
        },
        {
            code: "nl",
            label: "Nederlands"
        },
        {
            code: "ru",
            label: "Русский"
        }
    ]

    function languageIndex(language) {
        for (var i = 0; i < languageOptions.length; ++i) {
            if (languageOptions[i].code === language)
                return i;
        }
        return 0;
    }

    readonly property string versionLine: {
        var version = root.appVersion;
        if (version === undefined || version === null || version === "")
            version = Qt.application.version;
        var build = root.buildId;
        if (build === undefined || build === null || build === "")
            build = "?";
        return qsTr("Version") + " " + String(version) + " · " + qsTr("build") + " " + String(build);
    }

    modal: true
    anchors.centerIn: parent
    width: compactMode ? 440 : 640
    height: compactMode ? 620 : 680
    padding: 18
    standardButtons: Dialog.NoButton
    onClosed: showingChangelog = false

    background: Rectangle {
        color: BearTheme.panel
        radius: 12
        border.color: BearTheme.cardBorder
        border.width: 1
    }

    footer: Item {
        implicitHeight: 54

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            anchors.bottomMargin: 10
            spacing: 8

            AppButton {
                visible: root.showingChangelog
                text: qsTr("Back")
                onClicked: root.showingChangelog = false
            }

            Item {
                Layout.fillWidth: true
            }

            AppButton {
                text: qsTr("Close")
                onClicked: root.close()
            }
        }
    }

    contentItem: StackLayout {
        currentIndex: root.showingChangelog ? 1 : 0

        ColumnLayout {
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 142
                radius: 10
                color: BearTheme.panelAlt
                border.color: BearTheme.cardBorder

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 20

                    Image {
                        Layout.preferredWidth: 104
                        Layout.preferredHeight: 104
                        sourceSize.width: 208
                        sourceSize.height: 208
                        source: "qrc:/assets/app/bearwave.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("BearWave")
                            color: BearTheme.textMain
                            font.bold: true
                            font.pixelSize: 28
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Internet Radio Player for KDE")
                            color: BearTheme.textMuted
                            font.pixelSize: 14
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.topMargin: 5
                            text: root.versionLine
                            color: BearTheme.accent
                            font.pixelSize: 12
                        }

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Author: Sebastian Palencsár")
                            color: BearTheme.textMain
                            font.pixelSize: 12
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: root.languageSettings && root.languageSettings.restartRequired ? 148 : 116
                radius: 10
                color: BearTheme.panelAlt
                border.color: BearTheme.cardBorder

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Language")
                            color: BearTheme.textMain
                            font.bold: true
                        }

                        ComboBox {
                            id: languageCombo
                            Layout.preferredWidth: 190
                            model: root.languageOptions
                            textRole: "label"
                            currentIndex: root.languageIndex(root.languageSettings ? root.languageSettings.language : "system")
                            enabled: root.languageSettings !== null
                            onActivated: function (index) {
                                if (root.languageSettings) {
                                    root.languageSettings.language = root.languageOptions[index].code;
                                }
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: root.languageSettings && root.languageSettings.restartRequired
                        text: qsTr("Restart BearWave to apply the language change.")
                        color: BearTheme.accent
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: BearTheme.cardBorder
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        SocialButton {
                            iconSource: "qrc:/assets/ui/globe.svg"
                            tooltipText: qsTr("Open website")
                            targetUrl: "https://palencsar.pro"
                        }
                        SocialButton {
                            iconSource: "qrc:/assets/ui/github.svg"
                            tooltipText: qsTr("Open GitHub")
                            targetUrl: "https://github.com/spalencsar"
                        }
                        SocialButton {
                            iconSource: "qrc:/assets/ui/linkedin.svg"
                            tooltipText: qsTr("Open LinkedIn")
                            targetUrl: "https://www.linkedin.com/in/spalencsar/"
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        AppButton {
                            text: qsTr("What's New")
                            onClicked: root.showingChangelog = true
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
                    Layout.minimumHeight: 150
                    radius: 10
                    color: "#101a26"
                    border.color: BearTheme.cardBorder

                    ScrollView {
                        id: licenseScroll
                        anchors.fill: parent
                        anchors.margins: 10
                        clip: true
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        Label {
                            width: Math.max(licenseScroll.width, 1)
                            color: BearTheme.textMain
                            font.pixelSize: 11
                            wrapMode: Text.WordWrap
                            text: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>."
                        }
                    }
                }
            }
        }

        ColumnLayout {
            spacing: 10

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("What's New")
                        color: BearTheme.textMain
                        font.bold: true
                        font.pixelSize: 22
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.versionLine
                        color: BearTheme.textMuted
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: "#101a26"
                border.color: BearTheme.cardBorder

                ScrollView {
                    id: changelogScroll
                    anchors.fill: parent
                    anchors.margins: 14
                    clip: true
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    TextEdit {
                        width: Math.max(changelogScroll.availableWidth, 1)
                        text: root.changelogDocument.length > 0 ? root.changelogDocument : qsTr("The changelog could not be loaded.")
                        textFormat: TextEdit.MarkdownText
                        color: BearTheme.textMain
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        readOnly: true
                        selectByMouse: true
                        onLinkActivated: function (link) {
                            if (link.indexOf("https://") === 0 || link.indexOf("http://") === 0) {
                                Qt.openUrlExternally(link);
                            }
                        }
                    }
                }
            }
        }
    }

    component SocialButton: ToolButton {
        id: socialButton

        required property string iconSource
        required property string tooltipText
        required property string targetUrl

        Layout.preferredWidth: 36
        Layout.preferredHeight: 36
        focusPolicy: Qt.NoFocus
        onClicked: Qt.openUrlExternally(targetUrl)
        ToolTip.visible: hovered
        ToolTip.text: tooltipText

        contentItem: Item {
            Image {
                anchors.centerIn: parent
                width: 17
                height: 17
                source: socialButton.iconSource
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
        }

        background: Rectangle {
            radius: 7
            color: socialButton.hovered ? BearTheme.cardHover : BearTheme.imageWell
            border.color: socialButton.hovered ? BearTheme.accent : BearTheme.cardBorder
            border.width: 1
        }
    }

}
