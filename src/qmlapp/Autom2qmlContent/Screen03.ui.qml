

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
        Layout.fillHeight: true
        spacing: 15

        // Title
        Text {
            text: "TiltRec 3D Reconstruction"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
        }

        // Output file selector
        // File Information Group
        RowLayout {
            // Method selection
            Layout.alignment: Qt.AlignHCenter
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    text: "Method:"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                }

                ComboBox {
                    id: methodComboBox
                    model: ["BPT", "FBP", "WBP", "SART", "SIRT", "ADMM"]
                    currentIndex: 2 // Default to WBP
                }
            }

            // Thread count
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    text: "Thread Count:"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                }

                SpinBox {
                    id: threadCountSpinBox
                    from: 1
                    to: 32
                    value: 2
                }
            }

            // Reconstruction axis
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    text: "Reconstruction Axis:"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                }

                ComboBox {
                    id: axisComboBox
                    model: ["y", "z"]
                    currentIndex: 0 // Default to y
                }
            }

            // Bin Factor
            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    text: "Bin Factor:"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                }

                SpinBox {
                    id: binFactorSpinBox
                    from: 1
                    to: 16
                    value: 1
                }
            }

            Row {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    text: "Acceleration Method:"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                }

                ComboBox {
                    id: accComboBox
                    model: ["mpi", "cuda"]
                    currentIndex: 0 // Default to y
                }
            }
        }

        // Geometry parameters
        GroupBox {
            title: "Geometry Parameters"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            ColumnLayout {
                anchors.fill: parent
                Layout.fillWidth: true
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "Offset:"
                    }
                    SpinBox {
                        id: offsetSpinBox
                        from: -1000
                        to: 1000
                        value: 0
                    }

                    Text {
                        text: "Pitch Angle:"
                    }
                    SpinBox {
                        id: pitchSpinBox
                        from: -180
                        to: 180
                        value: 0
                    }

                    Text {
                        text: "Z-Axis Offset:"
                    }
                    SpinBox {
                        id: zOffsetSpinBox
                        from: -1000
                        to: 1000
                        value: 0
                    }

                    Text {
                        text: "Thickness:"
                    }
                    SpinBox {
                        id: thicknessSpinBox
                        from: 1
                        to: 2000
                        value: 300
                    }

                    // Get Geometry Parameters 按钮
                    Button {
                        id: getGeometryButton
                        Layout.preferredWidth: 200
                        Layout.alignment: Qt.AlignRight

                        text: CommandChainManager.activeChainId
                              === "geom-workflow" ? "Getting Parameters..." : "Get Geometry Parameters"
                        enabled: !CommandChainManager.isChainRunning

                        Connections {
                            function onClicked() {
                                if (!enabled)
                                    return

                                // 启动 geopargen-workflow
                                if (!CommandChainManager.startChain(
                                            "geom-workflow", {})) {
                                    console.error(
                                                "Failed to start geom-workflow")
                                }
                            }
                        }

                        // 监听 GeoParGen 结果
                        Connections {
                            target: CommandChainManager

                            function onGeoParGenResultParsed(angle, thickness, zOffset) {
                                console.log("Screen03: Received geom results - angle:",
                                            angle)
                                // 自动填入解析到的值
                                offsetSpinBox.value = Math.round(angle)
                            }

                            function onChainCompleted(chainId, success) {
                                if (chainId === "geom-workflow") {
                                    if (success) {
                                        console.log("Geom workflow completed successfully")
                                    } else {
                                        console.error("Geom workflow failed")
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // CTF Correction
        GroupBox {
            title: "CTF Correction"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                // Enable checkbox
                CheckBox {
                    id: ctfEnabledCheckBox
                    text: "Enable CTF Correction"
                    checked: AppState.ctfEnabled
                    onCheckedChanged: AppState.ctfEnabled = checked
                }

                // CTF Parameters grid
                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    visible: ctfEnabledCheckBox.checked

                    Text {
                        text: "Pixel Size (Å):"
                    }
                    DecimalSpinBox {
                        id: ctfPixelSizeSpinBox
                        fromValue: 0.1
                        toValue: 100.0
                        stepValue: 0.1
                        decimals: 2
                        realValue: AppState.ctfPixelSize
                    }

                    Text {
                        text: "Cs (mm):"
                    }
                    DecimalSpinBox {
                        id: ctfCsSpinBox
                        fromValue: 0.0
                        toValue: 10.0
                        stepValue: 0.1
                        decimals: 1
                        realValue: AppState.ctfCs
                    }

                    Text {
                        text: "Voltage (kV):"
                    }
                    SpinBox {
                        id: ctfVoltageSpinBox
                        from: 50
                        to: 500
                        value: AppState.ctfVoltage
                    }

                    Text {
                        text: "W (amplitude contrast):"
                    }
                    DecimalSpinBox {
                        id: ctfWSpinBox
                        fromValue: 0.0
                        toValue: 1.0
                        stepValue: 0.01
                        decimals: 2
                        realValue: AppState.ctfW
                    }
                }

                // CTF Status Info
                Row {
                    Layout.fillWidth: true
                    spacing: 10
                    visible: ctfEnabledCheckBox.checked

                    Text {
                        text: ctfStatusText.ctfReady ? "✓ CTF will be applied" : "CTF will run before reconstruction"
                        font.italic: true
                        color: ctfStatusText.ctfReady ? "#27AE60" : "#F39C12"

                        // 辅助属性
                        QtObject {
                            id: ctfStatusText
                            // CTF 已就绪（已运行过且输出文件存在）
                            property bool ctfReady: AppState.ctfCompleted
                        }
                    }
                }
            }
        }

        // Method-specific parameters
        GroupBox {
            title: "Method Parameters"
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: methodComboBox.currentText === "SART"
                     || methodComboBox.currentText === "SIRT"
                     || methodComboBox.currentText === "ADMM"

            GridLayout {
                anchors.fill: parent
                columns: 2

                Text {
                    text: "Iterations:"
                    visible: methodComboBox.currentText === "SART"
                             || methodComboBox.currentText === "SIRT"
                             || methodComboBox.currentText === "ADMM"
                }
                SpinBox {
                    id: iterationsSpinBox
                    from: 1
                    to: 1000
                    value: 10
                    visible: methodComboBox.currentText === "SART"
                             || methodComboBox.currentText === "SIRT"
                             || methodComboBox.currentText === "ADMM"
                }

                Text {
                    text: "Relaxation Parameter:"
                    visible: methodComboBox.currentText === "SART"
                             || methodComboBox.currentText === "SIRT"
                             || methodComboBox.currentText === "ADMM"
                }

                DecimalSpinBox {
                    id: relaxationSpinBox

                    // 现在您可以直接设置浮点数属性了！
                    fromValue: 0.0
                    toValue: 1.0
                    stepValue: 0.1
                    decimals: 1 // 可以覆盖默认的小数位数

                    // 您原来的可见性绑定，保持不变
                    visible: methodComboBox.currentText === "SART"
                             || methodComboBox.currentText === "SIRT"
                             || methodComboBox.currentText === "ADMM"

                    // 如果需要获取值，请使用 realValue
                    // 例如: onClicked: { console.log(relaxationSpinBox.realValue) }
                }

                Text {
                    text: "CG Iterations:"
                    visible: methodComboBox.currentText === "ADMM"
                }
                SpinBox {
                    id: cgIterationsSpinBox
                    from: 1
                    to: 100
                    value: 5
                    visible: methodComboBox.currentText === "ADMM"
                }

                Text {
                    text: "Threshold:"
                    visible: methodComboBox.currentText === "ADMM"
                }
                SpinBox {
                    id: thresholdSpinBox
                    from: 1
                    to: 1000
                    value: 100
                    visible: methodComboBox.currentText === "ADMM"
                }
            }
        }

        Button {
            id: runReconstructionButton
            Layout.preferredWidth: 200
            Layout.topMargin: 20
            Layout.alignment: Qt.AlignHCenter

            text: {
                if (CommandChainManager.activeChainId === "ctf-workflow")
                    return "Running CTF..."
                else if (CommandChainManager.activeChainId === "tiltrec-workflow")
                    return "Reconstructing..."
                else
                    return "Start Reconstruction"
            }
            enabled: !CommandChainManager.isChainRunning

            Connections {
                function onClicked() {
                    if (!enabled)
                        return

                    // 1. 保存 TiltRec 配置到 AppState
                    AppState.setTiltRecConfig({
                                                  "method": methodComboBox.currentText,
                                                  "accmethod": accComboBox.currentText,
                                                  "threads": threadCountSpinBox.value,
                                                  "axis": axisComboBox.currentText,
                                                  "binFactor": binFactorSpinBox.value,
                                                  "geometryOffset": offsetSpinBox.value,
                                                  "geometryPitch": pitchSpinBox.value,
                                                  "geometryZOffset": zOffsetSpinBox.value,
                                                  "geometryThickness": thicknessSpinBox.value,
                                                  "iterations": iterationsSpinBox.value,
                                                  "relaxation": relaxationSpinBox.realValue,
                                                  "cgIterations": cgIterationsSpinBox.value,
                                                  "threshold": thresholdSpinBox.value
                                              })

                    // 2. 保存 CTF 配置到 AppState
                    AppState.ctfEnabled = ctfEnabledCheckBox.checked
                    if (ctfEnabledCheckBox.checked) {
                        AppState.ctfPixelSize = ctfPixelSizeSpinBox.realValue
                        AppState.ctfCs = ctfCsSpinBox.realValue
                        AppState.ctfVoltage = ctfVoltageSpinBox.value
                        AppState.ctfW = ctfWSpinBox.realValue
                    }

                    // 3. 如果启用 CTF 且还没运行过，先运行 ctf-workflow
                    if (ctfEnabledCheckBox.checked && !AppState.ctfCompleted) {
                        console.log("CTF enabled, running ctf-workflow first...")
                        if (!CommandChainManager.startChain("ctf-workflow",
                                                            {})) {
                            console.error("Failed to start ctf-workflow")
                        }
                    } else {
                        // 直接启动 tiltrec-workflow
                        if (!CommandChainManager.startChain("tiltrec-workflow",
                                                            {})) {
                            console.error("Failed to start tiltrec-workflow")
                        }
                    }
                }
            }

            // 监听调用链完成
            Connections {
                target: CommandChainManager

                function onChainCompleted(chainId, success) {
                    if (chainId === "ctf-workflow") {
                        if (success) {
                            console.log("CTF workflow completed, now starting tiltrec-workflow...")
                            // 标记 CTF 已完成
                            AppState.ctfCompleted = true
                            // CTF 完成后自动启动 tiltrec-workflow
                            if (!CommandChainManager.startChain(
                                        "tiltrec-workflow", {})) {
                                console.error(
                                            "Failed to start tiltrec-workflow after CTF")
                            }
                        } else {
                            console.error(
                                        "CTF workflow failed, reconstruction aborted")
                        }
                    } else if (chainId === "tiltrec-workflow" && success) {
                        console.log("tiltrec-workflow completed, navigating to summary...")
                        Router.navigateToStepById("summary")
                    }
                }

                function onChainFailed(chainId, failedStep, program, exitCode, message) {
                    if (chainId === "ctf-workflow") {
                        console.error("ctf-workflow failed at step",
                                      failedStep, ":", message)
                    } else if (chainId === "tiltrec-workflow") {
                        console.error("tiltrec-workflow failed at step",
                                      failedStep, ":", message)
                    }
                }
            }
        }
        Button {
            id: stopReconstructionButton
            Layout.preferredWidth: 200
            Layout.topMargin: 20
            Layout.alignment: Qt.AlignHCenter

            text: "Stop Reconstruction"
            enabled: CommandChainManager.isChainRunning

            Connections {
                function onClicked() {
                    if (!enabled)
                        return
                    CommandChainManager.stopChain()
                }
            }
        }

        CustomProgressBar {
            id: progressBar
            Layout.preferredWidth: 200
            Layout.topMargin: 20
            Layout.alignment: Qt.AlignHCenter

            bindProgram: "TiltRec"
        }
    }
}
