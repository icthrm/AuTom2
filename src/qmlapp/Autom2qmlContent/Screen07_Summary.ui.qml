
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
    id: rectangle

    color: Constants.backgroundColor

    // --- Layout ---
    ColumnLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: parent.width * 0.9
        spacing: 20

        // Title
        Text {
            text: "Processing Complete ✅"
            font.pixelSize: 20
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 20
        }

        // Input Files Group
        GroupBox {
            title: "Input Files"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 15

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "MRC File:"
                        font.pixelSize: 14
                        Layout.preferredWidth: 150
                    }

                    Text {
                        text: AppState.initialMrcFile
                        font.pixelSize: 14
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Tilt Angles:"
                        font.pixelSize: 14
                        Layout.preferredWidth: 150
                    }

                    Text {
                        text: AppState.rawtltFile
                        font.pixelSize: 14
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // Generated Files Group
        GroupBox {
            title: "Generated Files"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 15

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Transform Matrix (.xf):"
                        font.pixelSize: 14
                        Layout.preferredWidth: 150
                    }

                    Text {
                        text: AppState.fixedXfFile
                        font.pixelSize: 14
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Aligned MRC:"
                        font.pixelSize: 14
                        Layout.preferredWidth: 150
                    }

                    Text {
                        text: AppState.alignedMrcFile
                        font.pixelSize: 14
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text {
                        text: "Reconstructed Volume:"
                        font.pixelSize: 14
                        Layout.preferredWidth: 150
                    }

                    Text {
                        text: AppState.reconstructionMrcFile
                        font.pixelSize: 14
                        wrapMode: Text.WrapAnywhere
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // Output Directory
        GroupBox {
            title: "Output Directory"
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Text {
                    text: "Output:"
                    font.pixelSize: 14
                    Layout.preferredWidth: 150
                }

                Text {
                    text: AppState.outputDir
                    font.pixelSize: 14
                    wrapMode: Text.WrapAnywhere
                    Layout.fillWidth: true
                }
            }
        }

        // Action Buttons
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 20
            spacing: 20
            Layout.alignment: Qt.AlignHCenter

            Button {
                text: "View Reconstruction"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 45
                font.pixelSize: 14

                Connections {
                    function onClicked() {
                        // 调用 Autom3D 查看重建结果
                        LogicHandler.viewMrcFile(AppState.reconstructionMrcFile)
                    }
                }
            }

            Button {
                text: "Start New Processing"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 45
                font.pixelSize: 14

                Connections {
                    function onClicked() {
                        // 清空状态，返回第一步
                        AppState.clearAll()
                        Router.navigateToState(0)
                        Router.navigateToStep(0)
                    }
                }
            }
        }
    }
}
