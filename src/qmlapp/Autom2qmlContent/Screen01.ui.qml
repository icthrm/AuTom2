
/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
...
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Autom2qml

Rectangle {
    id: rectangle

    color: Constants.backgroundColor

    ColumnLayout {

        anchors.fill: parent
        spacing: 20
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        // --- Layout ---
        ColumnLayout {
            id: mainLayout
            // anchors.centerIn: parent
            // width: parent.width * 0.9
            spacing: 20

            //Layout.preferredHeight: 5 // 可以是任意数字，代表权重
            Layout.fillWidth: true // 允许按钮拉伸以填充分配的空间
            Layout.fillHeight: true
            // Layout.preferredHeight: 7 // 权重为7（用于比例分配）

            // Title
            Text {
                text: "Step 1: File Selection & Marker Detection"
                font.pixelSize: 24
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 10
                color: "#2C3E50"
            }

            // Subtitle
            Text {
                text: "Select your MRC file, tilt angle file, and output directory to begin marker-based alignment"
                font.pixelSize: 14
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 20
                color: "#7F8C8D"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.8
            }

            // File Selection Group
            GroupBox {
                title: "Input Files"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                ColumnLayout {
                    anchors.fill: parent

                    PathSelector {
                        id: pathselector_mrc
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        buttonText: "Browse MRC File"
                        nameFilters: ["MRC files (*.mrc *.st)"]
                        path: "/home/xzh/atmp/BBa.st"
                    }

                    PathSelector {
                        id: pathselector_tlt
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        buttonText: "Browse Tilt Angle File"
                        nameFilters: ["Tilt files (*.rawtlt)"]
                        path: "/home/xzh/atmp/BBa.rawtlt"
                    }
                }
            }

            // Output Directory Group
            GroupBox {
                title: "Output Location"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                PathSelector {
                    id: pathselector_output
                    anchors.fill: parent
                    buttonText: "Browse Output Directory"
                    selectFolder: true
                    path: "/home/xzh/atmp/output"
                }
            }

            // Action Section
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                // Processing Mode Selection
                RowLayout {
                    spacing: 20
                    Layout.alignment: Qt.AlignHCenter

                    StyledButton {
                        id: singleFileButton
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 60

                        text: "Single File Processing"
                        enabled: pathselector_mrc.path !== ""
                                 && pathselector_tlt.path !== ""
                                 && pathselector_output.path !== ""

                        font.pixelSize: 16
                        font.bold: true

                        Connections {
                            function onClicked() {
                                if (!enabled)
                                    return

                                // 1. 保存用户选择的文件到 AppState
                                AppState.setInputFiles(pathselector_mrc.path,
                                                       pathselector_tlt.path,
                                                       pathselector_output.path)

                                // 2. 读取 MRC 头信息
                                var headerJson = backend.getMrcHeader(pathselector_mrc.path)
                                try {
                                    var header = JSON.parse(headerJson)
                                    if (header.error) {
                                        console.error("Failed to read MRC header:", header.error)
                                    } else {
                                        AppState.mrcHeader = header
                                        console.log("MRC header loaded:", header.nx, "x", header.ny, "x", header.nz)
                                    }
                                } catch (e) {
                                    console.error("Failed to parse MRC header JSON:", e)
                                }

                                let ret = AppState.validateInputFiles()
                                if (!ret.valid) {
                                    console.log(ret.errors)
                                    mainWindow.showError(ret.errors)
                                } else {
                                    Router.navigateToStep(1)
                                }
                            }
                        }
                    }

                    StyledButton {
                        id: batchProcessingButton
                        Layout.preferredWidth: 250
                        Layout.preferredHeight: 60

                        text: "Batch Processing"
                        enabled: true  // 批量处理不需要预先选择文件

                        font.pixelSize: 16
                        font.bold: true

                        Connections {
                            function onClicked() {
                                // 直接导航到批量处理界面
                                Router.navigateToStepById("batch-processing")
                            }
                        }
                    }
                }

                StyledButton {
                    id: viewButton
                    Layout.preferredWidth: 250
                    Layout.preferredHeight: 45
                    Layout.alignment: Qt.AlignHCenter

                    text: "View Mrc"
                    enabled: pathselector_mrc.path !== ""

                    font.pixelSize: 16
                    font.bold: true

                    Connections {
                        function onClicked() {
                            if (!enabled)
                                return

                            LogicHandler.viewMrcFile(pathselector_mrc.path)
                        }
                    }
                }
            }
        }
    }
}
