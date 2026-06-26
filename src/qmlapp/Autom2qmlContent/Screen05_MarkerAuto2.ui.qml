
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

// import Autom2qmlContent
Rectangle {
    id: rectangle

    color: Constants.backgroundColor

    // --- Layout ---
    ColumnLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: parent.width * 0.9
        Layout.fillHeight: true
        spacing: 20

        // Title
        Text {
            text: "Markerauto Alignment"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
            color: "#2C3E50"
        }

        // Subtitle
        Text {
            text: "Marker-based automatic alignment for cryo-electron tomography"
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 20
            color: "#7F8C8D"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.8
        }

        // Input Files Group
        // GroupBox {
        //     title: "Input Files"
        //     Layout.fillWidth: true
        //     Layout.alignment: Qt.AlignHCenter

        //    label: Text {
        //        text: parent.title
        //        font.pixelSize: 18       // 更改字体大小 (像素大小)\
        //    }

        //     ColumnLayout {
        //         anchors.fill: parent
        //         spacing: 15

        //         // MRC File Info
        //         Row {
        //             Layout.fillWidth: true
        //             spacing: 10

        //             Text {
        //                 text: "Input MRC File:"
        //                 font.bold: true
        //                 anchors.verticalCenter: parent.verticalCenter
        //                 color: "#34495E"
        //             }

        //             Text {
        //                 text: AppState.initialMrcFile
        //                 anchors.verticalCenter: parent.verticalCenter
        //                 font.family: "Courier"
        //                 color: "#16A085"
        //                 wrapMode: Text.WrapAnywhere
        //                 Layout.fillWidth: true
        //             }
        //         }

        //         Rectangle {
        //             Layout.fillWidth: true
        //             height: 1
        //             color: "#BDC3C7"
        //         }

        //         // Tilt Angle File Info
        //         Row {
        //             Layout.fillWidth: true
        //             spacing: 10

        //             Text {
        //                 text: "Tilt Angle File:"
        //                 font.bold: true
        //                 anchors.verticalCenter: parent.verticalCenter
        //                 color: "#34495E"
        //             }

        //             Text {
        //                 text: AppState.rawtltFile
        //                 anchors.verticalCenter: parent.verticalCenter
        //                 font.family: "Courier"
        //                 color: "#2980B9"
        //                 wrapMode: Text.WrapAnywhere
        //                 Layout.fillWidth: true
        //             }
        //         }

        //         Rectangle {
        //             Layout.fillWidth: true
        //             height: 1
        //             color: "#BDC3C7"
        //         }

        //         // Output Directory Info
        //         Row {
        //             Layout.fillWidth: true
        //             spacing: 10

        //             Text {
        //                 text: "Output Directory:"
        //                 font.bold: true
        //                 anchors.verticalCenter: parent.verticalCenter
        //                 color: "#34495E"
        //             }

        //             Text {
        //                 text: AppState.outputDir
        //                 anchors.verticalCenter: parent.verticalCenter
        //                 font.family: "Courier"
        //                 color: "#E74C3C"
        //                 wrapMode: Text.WrapAnywhere
        //                 Layout.fillWidth: true
        //             }
        //         }
        //     }
        // }

        // Output Files Preview Group
        // GroupBox {
        //     title: "Output Files (Generated)"
        //     Layout.fillWidth: true
        //     Layout.alignment: Qt.AlignHCenter

        //     ColumnLayout {
        //         anchors.fill: parent
        //         spacing: 10

        //         Text {
        //             text: "• " + AppState.fixedTltFile
        //             font.family: "Courier"
        //             font.pixelSize: 12
        //             color: "#7F8C8D"
        //             wrapMode: Text.WrapAnywhere
        //         }

        //         Text {
        //             text: "• " + AppState.fixedXfFile
        //             font.family: "Courier"
        //             font.pixelSize: 12
        //             color: "#7F8C8D"
        //             wrapMode: Text.WrapAnywhere
        //         }

        //         Text {
        //             text: "• " + AppState.alignedMrcFile + " (after mrcstack)"
        //             font.family: "Courier"
        //             font.pixelSize: 12
        //             color: "#7F8C8D"
        //             wrapMode: Text.WrapAnywhere
        //         }
        //     }
        // }

        // Configuration Group
        GroupBox {
            title: "Alignment Configuration"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            GridLayout {
                anchors.fill: parent
                columns: 4
                rowSpacing: 10
                columnSpacing: 15

                Text {
                    text: "Bead Diameter:"
                    color: "#34495E"
                }

                AutoSpinBox {
                    id: beadDiameterSpinBox
                    from: -1
                    to: 1000
                    value: -1
                    editable: true
                }

                Text {
                    text: "Erase Markers:"
                    color: "#34495E"
                }
                CheckBox {
                    id: eraseMarkersCheckBox
                    checked: false
                    ToolTip.visible: hovered
                    ToolTip.text: "Remove fiducial markers after alignment (using markererase)"
                }

                Text {
                    text: "High-Angle Supplement:"
                    color: "#34495E"
                }
                CheckBox {
                    id: enableSupplementCheckBox
                    checked: AppState.markerAuto2EnableSupplement
                    text: "Enable (-s)"
                    onCheckedChanged: AppState.markerAuto2EnableSupplement = checked
                }
                Text {
                    text: "Angle:"
                    color: "#34495E"
                    enabled: enableSupplementCheckBox.checked
                }
                SpinBox {
                    id: supplementAngleSpinBox
                    from: 0
                    to: 90
                    value: AppState.markerAuto2SupplementAngle
                    enabled: enableSupplementCheckBox.checked
                    onValueModified: AppState.markerAuto2SupplementAngle = value
                }

                // Empty spacers to maintain grid alignment
                Item {
                    width: 1
                    height: 1
                }
                Item {
                    width: 1
                    height: 1
                }
            }
        }

        // Action Buttons Row
        RowLayout {
            Layout.preferredHeight: 45
            Layout.topMargin: 20
            Layout.alignment: Qt.AlignHCenter
            spacing: 15

            Button {
                id: runAlignmentButton
                Layout.preferredWidth: 180
                Layout.preferredHeight: 45

                text: "Start Alignment"
                enabled: !CommandChainManager.isChainRunning

                font.pixelSize: 16
                font.bold: true

                Connections {
                    function onClicked() {
                        if (!enabled)
                            return

                        // 1. 保存配置到 AppState
                        AppState.setMarkerAuto2Config({
                                                          "beadDiameter": beadDiameterSpinBox.value,
                                                          "eraseMarkers": eraseMarkersCheckBox.checked,
                                                          "enableSupplement": enableSupplementCheckBox.checked,
                                                          "supplementAngle": supplementAngleSpinBox.value
                                                      })

                        // 2. 启动 markerauto2 调用链
                        if (!CommandChainManager.startChain("markerauto2-workflow", {})) {
                            console.error("Failed to start markerauto2-workflow")
                        }
                    }
                }
            }

            Button {
                id: stopAlignmentButton
                Layout.preferredWidth: 180
                Layout.preferredHeight: 45

                text: "Stop Alignment"
                enabled: CommandChainManager.isChainRunning

                font.pixelSize: 16
                font.bold: true

                Connections {
                    function onClicked() {
                        if (!enabled)
                            return
                        CommandChainManager.stopChain()
                    }
                }
            }

            Button {
                id: goToReconstructionButton
                Layout.preferredWidth: 200
                Layout.preferredHeight: 45

                text: "Go to Reconstruction"
                enabled: !CommandChainManager.isChainRunning
                         && AppState.processProgress === 100

                font.pixelSize: 16
                font.bold: true

                Connections {
                    function onClicked() {
                        if (!enabled)
                            return
                        Router.navigateToStepById("reconstruction")
                    }
                }

                // 监听调用链完成
                Connections {
                    target: CommandChainManager
                    
                    function onChainCompleted(chainId, success) {
                        if (chainId === "markerauto2-workflow" && success) {
                            console.log("markerauto2-workflow completed, refreshing image comparison...")
                            mrcImageCompare.refreshTrigger++
                        }
                    }
                    
                    function onChainFailed(chainId, failedStep, program, exitCode, message) {
                        if (chainId === "markerauto2-workflow") {
                            console.error("markerauto2-workflow failed at step", failedStep, ":", message)
                        }
                    }
                }
            }

            Button {
                id: showTraceButton
                Layout.preferredWidth: 160
                Layout.preferredHeight: 45

                text: {
                    if (CommandChainManager.activeChainId === "trajtrace-workflow")
                        return "Generating..."
                    else
                        return "Show Trace"
                }
                enabled: !CommandChainManager.isChainRunning
                         && AppState.processProgress === 100

                font.pixelSize: 16
                font.bold: true

                Connections {
                    function onClicked() {
                        if (!enabled)
                            return

                        // 如果已经运行过 trajtrace，直接跳转
                        if (AppState.trajTraceCompleted) {
                            Router.navigateToStepById("trace-compare")
                            return
                        }

                        // 否则启动 trajtrace-workflow
                        if (!CommandChainManager.startChain("trajtrace-workflow", {})) {
                            console.error("Failed to start trajtrace-workflow")
                        }
                    }
                }

                // 监听 trajtrace 完成
                Connections {
                    target: CommandChainManager
                    
                    function onChainCompleted(chainId, success) {
                        if (chainId === "trajtrace-workflow" && success) {
                            console.log("trajtrace-workflow completed, navigating to trace compare...")
                            AppState.trajTraceCompleted = true
                            Router.navigateToStepById("trace-compare")
                        }
                    }
                }
            }
        }

        // Progress Bar
        CustomProgressBar {
            id: progressBar
            Layout.preferredWidth: 250
            Layout.topMargin: 10
            Layout.alignment: Qt.AlignHCenter

            bindProgram: "markerauto2"
        }

        // Image Comparison Group
        GroupBox {
            title: "Image Comparison (Original vs Aligned)"
            Layout.fillWidth: true
            Layout.preferredHeight: 350
            Layout.topMargin: 20
            Layout.alignment: Qt.AlignHCenter

            MrcImageCompare {
                id: mrcImageCompare
                anchors.fill: parent
                originalMrcPath: AppState.initialMrcFile
                alignedMrcPath: AppState.alignedMrcFile
                sliceIndex: 0
                autoLoad: true
            }
        }
    }
}
