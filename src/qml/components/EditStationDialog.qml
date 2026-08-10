// Copyright (c) 2026 Sebastian Palencsar
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import theme 1.0

Dialog {
    id: root

    required property var app
    required property bool compactMode

    property var stationObject: null

    modal: true
    anchors.centerIn: parent
    width: compactMode ? 340 : 440
    padding: 0
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    readonly property var countryOptions: (app && app.backend)
                                          ? app.backend.countryPickerOptions : []

    background: Rectangle {
        color: BearTheme.panel
        radius: 14
        border.color: BearTheme.cardBorder
        border.width: 1
    }

    Overlay.modal: Rectangle {
        color: BearTheme.isLight ? "#66000000" : "#99000000"
    }

    function findCountryIndex(countryName) {
        if (!countryName || countryName.length === 0)
            return 0
        for (var i = 0; i < countryOptions.length; ++i) {
            if (countryOptions[i].name === countryName)
                return i
        }
        // Legacy "Manual" / free text → Not specified
        return 0
    }

    function selectedCountryName() {
        if (countryCombo.currentIndex < 0 || countryCombo.currentIndex >= countryOptions.length)
            return ""
        if (countryCombo.currentIndex === 0)
            return ""
        return countryOptions[countryCombo.currentIndex].name || ""
    }

    function setupAndOpen() {
        if (stationObject) {
            editName.text = stationObject.name || ""
            editUrl.text = stationObject.url || ""
            countryCombo.currentIndex = findCountryIndex(stationObject.country || "")
            errorLabel.text = ""
            open()
        }
    }

    function clearFields() {
        stationObject = null
        editName.text = ""
        editUrl.text = ""
        countryCombo.currentIndex = 0
        errorLabel.text = ""
    }

    function canSave() {
        return editName.text.trim().length > 0
               && editUrl.text.trim().length > 0
    }

    onRejected: clearFields()

    contentItem: ColumnLayout {
        spacing: 0

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: 18
            Layout.bottomMargin: 14
            spacing: 4

            Label {
                Layout.fillWidth: true
                text: qsTr("Edit station")
                color: BearTheme.textMain
                font.pixelSize: 18
                font.bold: true
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("Changes are stored with My stations on this device.")
                color: BearTheme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: BearTheme.cardBorder
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: 16
            Layout.bottomMargin: 8
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Label {
                    text: qsTr("Name")
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 0.5
                }
                ThemedField {
                    id: editName
                    Layout.fillWidth: true
                    placeholderText: qsTr("Station name")
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Label {
                    text: qsTr("Stream URL")
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 0.5
                }
                ThemedField {
                    id: editUrl
                    Layout.fillWidth: true
                    placeholderText: qsTr("https://…")
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Label {
                    text: qsTr("Country")
                    color: BearTheme.textMuted
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 0.5
                }
                ComboBox {
                    id: countryCombo
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    model: root.countryOptions
                    textRole: "name"
                    valueRole: "code"
                    font.pixelSize: 14
                    background: Rectangle {
                        radius: 10
                        color: BearTheme.isLight ? "#f0f0f3" : "#1a1a1e"
                        border.width: countryCombo.activeFocus || countryCombo.popup.visible ? 1.5 : 1
                        border.color: (countryCombo.activeFocus || countryCombo.popup.visible)
                                      ? BearTheme.accent : BearTheme.cardBorder
                    }
                    contentItem: Label {
                        leftPadding: 12
                        rightPadding: countryCombo.indicator.width + 12
                        text: countryCombo.displayText
                        color: BearTheme.textMain
                        font: countryCombo.font
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    popup: Popup {
                        y: countryCombo.height
                        width: countryCombo.width
                        implicitHeight: Math.min(320, contentItem.implicitHeight + 16)
                        padding: 6
                        background: Rectangle {
                            color: BearTheme.panel
                            radius: 10
                            border.color: BearTheme.cardBorder
                            border.width: 1
                        }
                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: countryCombo.popup.visible ? countryCombo.delegateModel : null
                            currentIndex: countryCombo.highlightedIndex
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }
                    }
                    delegate: ItemDelegate {
                        width: countryCombo.width
                        height: 36
                        highlighted: countryCombo.highlightedIndex === index
                        contentItem: Label {
                            text: modelData.name
                            color: BearTheme.textMain
                            font.pixelSize: 13
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 8
                        }
                        background: Rectangle {
                            color: parent.highlighted
                                   ? BearTheme.selection
                                   : (parent.hovered ? BearTheme.cardHover : "transparent")
                            radius: 6
                        }
                    }
                }
            }

            Label {
                id: errorLabel
                Layout.fillWidth: true
                visible: text.length > 0
                color: BearTheme.warn
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: BearTheme.cardBorder
            Layout.topMargin: 8
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.topMargin: 12
            Layout.bottomMargin: 14
            spacing: 8

            Item { Layout.fillWidth: true }

            AppButton {
                text: qsTr("Cancel")
                flat: true
                onClicked: root.reject()
            }
            AppButton {
                text: qsTr("Save")
                highlighted: true
                enabled: root.canSave()
                onClicked: {
                    if (!app.backend || !stationObject) {
                        root.reject()
                        return
                    }
                    const url = editUrl.text.trim()
                    if (url.indexOf("http://") !== 0 && url.indexOf("https://") !== 0) {
                        errorLabel.text = qsTr("Stream URL must start with http:// or https://")
                        return
                    }
                    app.backend.editManualStation(stationObject, editName.text, editUrl.text,
                                                  root.selectedCountryName())
                    if (app.backend.lastError && app.backend.lastError.length > 0) {
                        errorLabel.text = app.backend.lastError
                        return
                    }
                    app.toast(qsTr("Station updated"))
                    clearFields()
                    root.accept()
                }
            }
        }
    }

    component ThemedField: TextField {
        id: field
        color: BearTheme.textMain
        placeholderTextColor: BearTheme.textMuted
        font.pixelSize: 14
        selectByMouse: true
        leftPadding: 12
        rightPadding: 12
        topPadding: 10
        bottomPadding: 10
        background: Rectangle {
            radius: 10
            color: BearTheme.isLight ? "#f0f0f3" : "#1a1a1e"
            border.width: field.activeFocus ? 1.5 : 1
            border.color: field.activeFocus ? BearTheme.accent : BearTheme.cardBorder
        }
    }
}
