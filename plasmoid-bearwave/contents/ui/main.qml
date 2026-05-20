import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.plasma5support as Plasma5Support

PlasmoidItem {
    id: root

    property string sourceName: "org.mpris.MediaPlayer2.bearwave"
    property bool available: mprisSource.data[sourceName] !== undefined
    property string playbackStatus: available ? (mprisSource.data[sourceName]["PlaybackStatus"] || "Stopped") : "Stopped"
    property var metadata: available ? (mprisSource.data[sourceName]["Metadata"] || ({})) : ({})
    property string titleText: metadata["xesam:title"] || "BearWave"
    property string artistText: (metadata["xesam:artist"] && metadata["xesam:artist"].length > 0) ? metadata["xesam:artist"][0] : ""

    preferredRepresentation: compactRepresentation

    function callMpris(methodName) {
        if (!available) {
            return
        }
        const service = mprisSource.serviceForSource(sourceName)
        const operation = service.operationDescription(methodName)
        if (operation === undefined) {
            return
        }
        service.startOperationCall(operation)
    }

    compactRepresentation: Item {
        implicitWidth: 220
        implicitHeight: 28

        PlasmaComponents.Button {
            anchors.fill: parent
            text: available
                  ? (artistText.length > 0 ? (artistText + " - " + titleText) : titleText)
                  : "BearWave nicht aktiv"
            onClicked: root.expanded = true
        }
    }

    fullRepresentation: Item {
        implicitWidth: 320
        implicitHeight: 170

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            PlasmaComponents.Label {
                Layout.fillWidth: true
                text: available ? titleText : "BearWave nicht aktiv"
                elide: Text.ElideRight
                font.bold: true
            }

            PlasmaComponents.Label {
                Layout.fillWidth: true
                text: artistText.length > 0 ? artistText : (available ? "Live-Radio" : "Bitte BearWave starten")
                elide: Text.ElideRight
                opacity: 0.8
            }

            PlasmaComponents.Label {
                Layout.fillWidth: true
                text: "Status: " + playbackStatus
                opacity: 0.8
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                PlasmaComponents.Button {
                    text: "⏮"
                    enabled: available
                    onClicked: callMpris("Previous")
                }
                PlasmaComponents.Button {
                    text: playbackStatus === "Playing" ? "⏸" : "▶"
                    enabled: available
                    onClicked: callMpris("PlayPause")
                }
                PlasmaComponents.Button {
                    text: "⏹"
                    enabled: available
                    onClicked: callMpris("Stop")
                }
                PlasmaComponents.Button {
                    text: "⏭"
                    enabled: available
                    onClicked: callMpris("Next")
                }
            }
        }
    }

    Plasma5Support.DataSource {
        id: mprisSource
        engine: "mpris2"
        connectedSources: [sourceName]
    }
}
