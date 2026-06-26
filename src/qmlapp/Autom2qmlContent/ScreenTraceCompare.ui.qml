
/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Autom2qml

Rectangle {
    id: root

    color: Constants.backgroundColor

    // SVG 缓存键存储
    property string initXZKey: ""
    property string initYZKey: ""
    property string initXYKey: ""
    property string finXZKey: ""
    property string finYZKey: ""
    property string finXYKey: ""

    // 加载所有 SVG 图像
    function loadAllSvgs() {
        if (AppState.initTraceXZ !== "") {
            initXZKey = backend.loadSvg(AppState.initTraceXZ, 0, 0)
        }
        if (AppState.initTraceYZ !== "") {
            initYZKey = backend.loadSvg(AppState.initTraceYZ, 0, 0)
        }
        if (AppState.initTraceXY !== "") {
            initXYKey = backend.loadSvg(AppState.initTraceXY, 0, 0)
        }
        if (AppState.finTraceXZ !== "") {
            finXZKey = backend.loadSvg(AppState.finTraceXZ, 0, 0)
        }
        if (AppState.finTraceYZ !== "") {
            finYZKey = backend.loadSvg(AppState.finTraceYZ, 0, 0)
        }
        if (AppState.finTraceXY !== "") {
            finXYKey = backend.loadSvg(AppState.finTraceXY, 0, 0)
        }
    }

    // 页面加载时自动加载 SVG
    Component.onCompleted: {
        loadAllSvgs()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Title
        Text {
            text: "Trajectory Trace Comparison"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "#2C3E50"
        }

        // Subtitle
        Text {
            text: "Comparing initial (before alignment) and final (after alignment) marker trajectories"
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
            color: "#7F8C8D"
            horizontalAlignment: Text.AlignHCenter
        }

        // Main comparison area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // Row labels
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Text {
                    text: "View"
                    font.bold: true
                    Layout.preferredWidth: 80
                    horizontalAlignment: Text.AlignHCenter
                    color: "#34495E"
                }

                Text {
                    text: "XZ Plane"
                    font.bold: true
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    color: "#2980B9"
                }

                Text {
                    text: "YZ Plane"
                    font.bold: true
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    color: "#27AE60"
                }

                Text {
                    text: "XY Plane"
                    font.bold: true
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    color: "#E74C3C"
                }
            }

            // Init row (before alignment)
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // Row label
                Rectangle {
                    Layout.preferredWidth: 80
                    Layout.fillHeight: true
                    color: "#3498DB"
                    radius: 5

                    Text {
                        anchors.centerIn: parent
                        text: "Initial\n(Before)"
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // Init XZ
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ECF0F1"
                    radius: 5
                    border.color: "#BDC3C7"
                    border.width: 1

                    Image {
                        id: initXZImage
                        anchors.fill: parent
                        anchors.margins: 5
                        source: root.initXZKey !== "" ? "image://svg/" + root.initXZKey : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        // Placeholder when no image
                        Text {
                            anchors.centerIn: parent
                            text: "Init XZ"
                            color: "#95A5A6"
                            visible: initXZImage.status !== Image.Ready
                        }
                    }
                }

                // Init YZ
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ECF0F1"
                    radius: 5
                    border.color: "#BDC3C7"
                    border.width: 1

                    Image {
                        id: initYZImage
                        anchors.fill: parent
                        anchors.margins: 5
                        source: root.initYZKey !== "" ? "image://svg/" + root.initYZKey : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: "Init YZ"
                            color: "#95A5A6"
                            visible: initYZImage.status !== Image.Ready
                        }
                    }
                }

                // Init XY
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ECF0F1"
                    radius: 5
                    border.color: "#BDC3C7"
                    border.width: 1

                    Image {
                        id: initXYImage
                        anchors.fill: parent
                        anchors.margins: 5
                        source: root.initXYKey !== "" ? "image://svg/" + root.initXYKey : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: "Init XY"
                            color: "#95A5A6"
                            visible: initXYImage.status !== Image.Ready
                        }
                    }
                }
            }

            // Fin row (after alignment)
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // Row label
                Rectangle {
                    Layout.preferredWidth: 80
                    Layout.fillHeight: true
                    color: "#27AE60"
                    radius: 5

                    Text {
                        anchors.centerIn: parent
                        text: "Final\n(After)"
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // Fin XZ
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ECF0F1"
                    radius: 5
                    border.color: "#BDC3C7"
                    border.width: 1

                    Image {
                        id: finXZImage
                        anchors.fill: parent
                        anchors.margins: 5
                        source: root.finXZKey !== "" ? "image://svg/" + root.finXZKey : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: "Fin XZ"
                            color: "#95A5A6"
                            visible: finXZImage.status !== Image.Ready
                        }
                    }
                }

                // Fin YZ
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ECF0F1"
                    radius: 5
                    border.color: "#BDC3C7"
                    border.width: 1

                    Image {
                        id: finYZImage
                        anchors.fill: parent
                        anchors.margins: 5
                        source: root.finYZKey !== "" ? "image://svg/" + root.finYZKey : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: "Fin YZ"
                            color: "#95A5A6"
                            visible: finYZImage.status !== Image.Ready
                        }
                    }
                }

                // Fin XY
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#ECF0F1"
                    radius: 5
                    border.color: "#BDC3C7"
                    border.width: 1

                    Image {
                        id: finXYImage
                        anchors.fill: parent
                        anchors.margins: 5
                        source: root.finXYKey !== "" ? "image://svg/" + root.finXYKey : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: "Fin XY"
                            color: "#95A5A6"
                            visible: finXYImage.status !== Image.Ready
                        }
                    }
                }
            }
        }

        // Bottom buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20

            Button {
                id: backButton
                text: "Back to Markerauto"
                Layout.preferredWidth: 180
                Layout.preferredHeight: 45

                font.pixelSize: 14
                font.bold: true

                Connections {
                    function onClicked() {
                        Router.navigateToStepById("markerauto2")
                    }
                }
            }

            Button {
                id: goToReconstructionButton
                text: "Go to Reconstruction"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 45

                font.pixelSize: 14
                font.bold: true

                Connections {
                    function onClicked() {
                        Router.navigateToStepById("reconstruction")
                    }
                }
            }

            Button {
                id: refreshButton
                text: "Refresh Images"
                Layout.preferredWidth: 150
                Layout.preferredHeight: 45

                font.pixelSize: 14

                Connections {
                    function onClicked() {
                        // 重新加载所有 SVG
                        root.loadAllSvgs()
                    }
                }
            }
        }
    }
}
