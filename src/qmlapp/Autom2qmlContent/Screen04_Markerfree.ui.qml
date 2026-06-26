
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
        spacing: 15

        // Title
        Text {
            text: "Markerfree Alignment"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
            color: "#2C3E50"
        }

        // Subtitle
        Text {
            text: "GPU-accelerated robust marker-free alignment for high-resolution cryo-electron tomography"
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 15
            color: "#7F8C8D"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.8
        }
        // Geometry Parameters
        GroupBox {
            title: "Geometry Parameters (7 values)"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            GridLayout {
                anchors.fill: parent
                columns: 4
                rowSpacing: 10
                columnSpacing: 15

                Text {
                    text: "Offset:"
                    color: "#34495E"
                }
                SpinBox {
                    id: offsetSpinBox
                    from: -1000
                    to: 1000
                    value: 0
                }

                Text {
                    text: "Tilt Axis Angle:"
                    color: "#34495E"
                }
                SpinBox {
                    id: tiltAxisAngleSpinBox
                    from: -180
                    to: 180
                    value: 0
                }

                Text {
                    text: "Z-Axis Offset:"
                    color: "#34495E"
                }
                SpinBox {
                    id: zOffsetSpinBox
                    from: -1000
                    to: 1000
                    value: 0
                }

                Text {
                    text: "Thickness:"
                    color: "#34495E"
                }
                SpinBox {
                    id: thicknessSpinBox
                    from: 0
                    to: 2000
                    value: 0
                }

                Text {
                    text: "Proj. Match Thickness:"
                    color: "#34495E"
                }
                SpinBox {
                    id: projMatchThicknessSpinBox
                    from: 1
                    to: 2000
                    value: 300
                }

                Text {
                    text: "Downsample Ratio:"
                    color: "#34495E"
                }

                SpinBox {
                    id: downsampleRatioSpinBox
                    from: 1
                    to: 16
                    value: 1
                }

                Text {
                    text: "GPU ID:"
                    color: "#34495E"
                }
                SpinBox {
                    id: gpuIdSpinBox
                    from: 0
                    to: 7
                    value: 0
                }

                Text {
                    text: "Proj. Image Count:"
                    color: "#34495E"
                }
                SpinBox {
                    id: projImageCountSpinBox
                    from: 1
                    to: 100
                    value: 10
                }
            }
        }

        // Save Format Selection
        GroupBox {
            title: "Output Format"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            RowLayout {
                anchors.fill: parent
                spacing: 20

                Text {
                    text: "Save Mode:"
                    font.bold: true
                    color: "#34495E"
                }

                RadioButton {
                    id: saveModeXfRadio
                    text: "XF File (Affine Transformation Matrix)"
                    checked: true
                }

                RadioButton {
                    id: saveModeTxtRadio
                    text: "TXT File (Tilt Axis Angle & Translation)"
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
                        AppState.setMarkerfreeConfig({
                                                         "offset": offsetSpinBox.value,
                                                         "tiltAxisAngle": tiltAxisAngleSpinBox.value,
                                                         "zOffset": zOffsetSpinBox.value,
                                                         "thickness": thicknessSpinBox.value,
                                                         "projMatchThickness": projMatchThicknessSpinBox.value,
                                                         "downsampleRatio": downsampleRatioSpinBox.value,
                                                         "gpuId": gpuIdSpinBox.value,
                                                         "saveMode": saveModeXfRadio.checked ? 1 : 0,
                                                         "projImageCount": projImageCountSpinBox.value
                                                     })

                        // 2. 启动 markerfree 调用链
                        if (!CommandChainManager.startChain("markerfree-workflow", {})) {
                            console.error("Failed to start markerfree-workflow")
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
                        if (chainId === "markerfree-workflow" && success) {
                            console.log("markerfree-workflow completed, refreshing image comparison...")
                            mrcImageCompare.refreshTrigger++
                        }
                    }
                    
                    function onChainFailed(chainId, failedStep, program, exitCode, message) {
                        if (chainId === "markerfree-workflow") {
                            console.error("markerfree-workflow failed at step", failedStep, ":", message)
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

            bindProgram: "Markerfree"
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
