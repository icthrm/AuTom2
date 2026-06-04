
/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
...
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Autom2qml

Rectangle {
    id: root

    color: Constants.backgroundColor

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        spacing: 30

        // Title
        Text {
            text: "Select Alignment Method"
            font.pixelSize: 28
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "#2C3E50"
        }

        // Subtitle
        Text {
            text: "Choose between marker-free alignment or marker-based alignment"
            font.pixelSize: 16
            Layout.alignment: Qt.AlignHCenter
            color: "#7F8C8D"
            horizontalAlignment: Text.AlignHCenter
        }

        // File Info Display
        GroupBox {
            title: "Selected Files"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: "Input MRC: " + AppState.baseName + ".mrc"
                    font.pixelSize: 14
                    color: "#16A085"
                }

                Text {
                    text: "Tilt Angle: " + AppState.baseName + ".rawtlt"
                    font.pixelSize: 14
                    color: "#2980B9"
                }

                Text {
                    text: "Output Dir: " + AppState.outputDir
                    font.pixelSize: 14
                    color: "#E74C3C"
                    wrapMode: Text.WrapAnywhere
                }
            }
        }

        // MRC Header Info Display
        GroupBox {
            title: "MRC File Information"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: AppState.mrcHeader !== null

            GridLayout {
                anchors.fill: parent
                columns: 4
                columnSpacing: 20
                rowSpacing: 8

                // 图像尺寸
                Text {
                    text: "Dimensions:"
                    font.bold: true
                    color: "#34495E"
                }
                Text {
                    text: AppState.mrcNx + " × " + AppState.mrcNy + " × " + AppState.mrcNz
                    color: "#2C3E50"
                    Layout.columnSpan: 3
                }

                // 像素大小
                Text {
                    text: "Pixel Size:"
                    font.bold: true
                    color: "#34495E"
                }
                Text {
                    text: AppState.mrcPixelSize.toFixed(4) + " Å"
                    color: "#2C3E50"
                }

                // 数据类型
                Text {
                    text: "Data Mode:"
                    font.bold: true
                    color: "#34495E"
                }
                Text {
                    text: AppState.mrcModeStr
                    color: "#2C3E50"
                }

                // 密度值
                Text {
                    text: "Density Range:"
                    font.bold: true
                    color: "#34495E"
                }
                Text {
                    text: AppState.mrcAmin.toFixed(4) + " ~ " + AppState.mrcAmax.toFixed(4)
                    color: "#2C3E50"
                }

                // 平均密度
                Text {
                    text: "Mean:"
                    font.bold: true
                    color: "#34495E"
                }
                Text {
                    text: AppState.mrcAmean.toFixed(4)
                    color: "#2C3E50"
                }
            }
        }

        // Buttons Row
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 30
            spacing: 40

            // Markerfree Button
            Button {
                id: markerfreeButton
                text: "Markerfree\n(GPU-Accelerated)"

                Layout.preferredWidth: parent.width * 0.4
                Layout.preferredHeight: 150
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                font.pixelSize: 18
                font.bold: true

                contentItem: ColumnLayout {
                    spacing: 10

                    Text {
                        text: "Markerfree"
                        font.pixelSize: 20
                        font.bold: true
                        color: markerfreeButton.down ? "#2C3E50" : "#FFFFFF"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "(GPU-Accelerated)"
                        font.pixelSize: 14
                        color: markerfreeButton.down ? "#7F8C8D" : "#ECF0F1"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Marker-free alignment for\nhigh-resolution cryo-ET"
                        font.pixelSize: 12
                        color: markerfreeButton.down ? "#95A5A6" : "#BDC3C7"
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                background: Rectangle {
                    color: markerfreeButton.down ? "#ECF0F1" : "#3498DB"
                    radius: 8
                    border.color: "#2980B9"
                    border.width: 2
                }

                Connections {
                    function onClicked() {
                        // 记录用户选择的工作流
                        AppState.selectedWorkflow = "markerfree"
                        // 导航到 Markerfree 配置界面
                        Router.navigateToStepById("markerfree")
                    }
                }
            }

            // MarkerAuto2 Button
            Button {
                id: markerAuto2Button
                text: "Markerauto\n(With Markers)"

                Layout.preferredWidth: parent.width * 0.4
                Layout.preferredHeight: 150
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                font.pixelSize: 18
                font.bold: true

                contentItem: ColumnLayout {
                    spacing: 10

                    Text {
                        text: "Markerauto"
                        font.pixelSize: 20
                        font.bold: true
                        color: markerAuto2Button.down ? "#2C3E50" : "#FFFFFF"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "(With Markers)"
                        font.pixelSize: 14
                        color: markerAuto2Button.down ? "#7F8C8D" : "#ECF0F1"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: "Traditional marker-based\nalignment method"
                        font.pixelSize: 12
                        color: markerAuto2Button.down ? "#95A5A6" : "#BDC3C7"
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                background: Rectangle {
                    color: markerAuto2Button.down ? "#ECF0F1" : "#27AE60"
                    radius: 8
                    border.color: "#229954"
                    border.width: 2
                }

                Connections {
                    function onClicked() {
                        // 记录用户选择的工作流
                        AppState.selectedWorkflow = "markerauto2"
                        // 导航到 MarkerAuto2 执行界面
                        Router.navigateToStepById("markerauto2")
                    }
                }
            }
        }
    }
}
