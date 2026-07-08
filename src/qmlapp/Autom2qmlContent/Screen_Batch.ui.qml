
// Screen_Batch.ui.qml
// Batch Processing Interface
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Autom2qml

Rectangle {
    id: root
    anchors.fill: parent
    color: "#f5f5f5"

    property bool isInDesignMode: Qt.application.name.startsWith("qmlpuppet")
    property var backend: isInDesignMode ? MockBackend : realBackend
    property var batchStartTime: null

    // File scanner
    FilePairScanner {
        id: fileScanner
        backend: root.backend
    }

    Timer {
        id: elapsedTimer
        interval: 1000
        running: BatchProcessHandler.isProcessing
        repeat: true
        onTriggered: {
            if (batchStartTime) {
                let now = new Date()
                let diff = Math.floor((now - batchStartTime) / 1000)
                elapsedTimeText.text = "Elapsed: " + formatElapsedTime(diff)
            }
        }
    }

    // Main layout with ScrollView for flexibility
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        contentHeight: mainColumnLayout.implicitHeight
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        ColumnLayout {
            id: mainColumnLayout
            width: parent.width
            spacing: 20
            // Top spacing
            Item {
                Layout.preferredHeight: 20
            }

            // ========== Title ==========
            Text {
                text: "Batch Processing"
                font.pixelSize: 28
                font.bold: true
                color: "#2C3E50"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Select multiple files to automatically pair MRC and Rawtlt files for batch processing"
                font.pixelSize: 14
                color: "#7F8C8D"
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: Math.min(parent.width * 0.8, 800)
            }

            // ========== File Selection Area ==========
            GroupBox {
                title: "Step 1: Select Files"
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.maximumWidth: 1200
                Layout.alignment: Qt.AlignHCenter

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 15

                    RowLayout {
                        spacing: 15
                        Layout.fillWidth: true

                        Button {
                            text: "Select Files..."
                            font.pixelSize: 14
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 45
                            onClicked: fileDialog.open()

                            background: Rectangle {
                                color: parent.down ? "#95A5A6" : "#7F8C8D"
                                radius: 4
                            }

                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: "#FFFFFF"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Button {
                            text: "Scan Folder..."
                            font.pixelSize: 14
                            Layout.preferredWidth: 150
                            Layout.preferredHeight: 45
                            onClicked: {
                                let path = realBackend.selectFolder("Select Folder to Scan for MRC and Rawtlt Files", "/host")
                                if (path) {
                                    console.log("Scanning folder:", path)
                                    fileScanner.scanFolder(path)
                                }
                            }

                            background: Rectangle {
                                color: parent.down ? "#2980B9" : "#3498DB"
                                radius: 4
                            }

                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: "#FFFFFF"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Text {
                            text: {
                                if (fileScanner.isScanning) {
                                    return "Scanning..."
                                } else if (fileScanner.pairCount > 0) {
                                    return `Found ${fileScanner.pairCount} file pairs`
                                } else {
                                    return "No files selected"
                                }
                            }
                            font.pixelSize: 14
                            color: fileScanner.isScanning ? "#E67E22" : (fileScanner.pairCount > 0 ? "#27AE60" : "#7F8C8D")
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "Clear List"
                            font.pixelSize: 14
                            Layout.preferredWidth: 120
                            Layout.preferredHeight: 45
                            enabled: fileScanner.pairCount > 0
                            onClicked: {
                                fileScanner.clear()
                                fileListModel.clear()
                            }
                        }
                    }

                    Text {
                        text: "Tip: \"Select Files...\" lets you pick multiple .mrc/.st and .rawtlt files; they are auto-paired by basename. \"Scan Folder...\" only shows folders—after you select one, the backend scans inside it automatically."
                        font.pixelSize: 12
                        color: "#95A5A6"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            // ========== File List ==========
            GroupBox {
                title: `File List (${fileScanner.pairCount} pairs)`
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 250
                Layout.minimumHeight: 150
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.maximumWidth: 1200
                Layout.alignment: Qt.AlignHCenter

                ScrollView {
                    anchors.fill: parent
                    clip: true

                    ListView {
                        id: fileListView
                        model: fileListModel
                        spacing: 5

                        delegate: Rectangle {
                            width: fileListView.width
                            height: 70
                            color: index % 2 === 0 ? "#FFFFFF" : "#F8F9FA"
                            border.color: "#E0E0E0"
                            border.width: 1
                            radius: 4

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10

                                Text {
                                    text: (index + 1).toString()
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: "#7F8C8D"
                                    Layout.preferredWidth: 30
                                }

                                ColumnLayout {
                                    spacing: 5
                                    Layout.fillWidth: true

                                    Text {
                                        text: "Basename: " + model.basename
                                        font.pixelSize: 13
                                        font.bold: true
                                        color: "#2C3E50"
                                    }

                                    Text {
                                        text: "MRC: " + model.mrcFile
                                        font.pixelSize: 11
                                        color: "#16A085"
                                        elide: Text.ElideMiddle
                                        Layout.fillWidth: true
                                    }

                                    Text {
                                        text: "Rawtlt: " + model.rawtltFile
                                        font.pixelSize: 11
                                        color: "#2980B9"
                                        elide: Text.ElideMiddle
                                        Layout.fillWidth: true
                                    }
                                }

                                Button {
                                    text: "Remove"
                                    font.pixelSize: 12
                                    Layout.preferredWidth: 80
                                    Layout.preferredHeight: 35
                                    onClicked: {
                                        fileScanner.removePair(index)
                                        fileListModel.remove(index)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ========== Processing Configuration Area ==========
            GroupBox {
                title: "Step 2: Configure Processing Options"
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.maximumWidth: 1200
                Layout.alignment: Qt.AlignHCenter

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 15

                    // Output directory
                    RowLayout {
                        spacing: 10
                        Layout.fillWidth: true

                        Text {
                            text: "Output Directory:"
                            font.pixelSize: 14
                            Layout.preferredWidth: 120
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            color: "#FFFFFF"
                            border.color: "#BDC3C7"
                            border.width: 1
                            radius: 4

                            Text {
                                anchors.fill: parent
                                anchors.margins: 8
                                text: outputDirField.text || "Not selected"
                                font.pixelSize: 12
                                color: outputDirField.text ? "#2C3E50" : "#95A5A6"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideMiddle
                            }
                        }

                        Button {
                            text: "Browse..."
                            font.pixelSize: 14
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 35
                            onClicked: {
                                let path = realBackend.selectFolder("Select Output Directory", outputDirField.text || "/host")
                                if (path) {
                                    outputDirField.text = path
                                    console.log("Output directory selected:", path)
                                }
                            }
                        }
                    }

                    // Alignment method selection
                    RowLayout {
                        spacing: 10
                        Layout.fillWidth: true

                        Text {
                            text: "Alignment Method:"
                            font.pixelSize: 14
                            Layout.preferredWidth: 120
                        }

                        ComboBox {
                            id: methodComboBox
                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 35
                            model: ["Markerauto", "Markerfree"]
                            currentIndex: 0
                        }

                        Text {
                            text: methodComboBox.currentIndex === 0 ? "Traditional marker-based alignment" : "GPU-accelerated marker-free alignment"
                            font.pixelSize: 12
                            color: "#7F8C8D"
                            Layout.fillWidth: true
                        }
                    }

                    // Markerauto parameters
                    GroupBox {
                        title: "Markerauto Parameters"
                        visible: methodComboBox.currentIndex === 0
                        Layout.fillWidth: true

                        GridLayout {
                            anchors.fill: parent
                            columns: 4
                            columnSpacing: 15
                            rowSpacing: 10

                            Text {
                                text: "Bead Diameter:"
                                font.pixelSize: 13
                            }

                            SpinBox {
                                id: beadDiameterSpinBox
                                from: -1
                                to: 100
                                value: -1
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "(-1 = auto detect)"
                                font.pixelSize: 11
                                color: "#7F8C8D"
                                Layout.columnSpan: 2
                            }

                            CheckBox {
                                id: eraseMarkersCheckBox
                                text: "Erase Markers"
                                font.pixelSize: 13
                                Layout.columnSpan: 4
                            }

                            Text {
                                text: "High-Angle Supplement:"
                                font.pixelSize: 13
                            }
                            CheckBox {
                                id: enableSupplementCheckBox
                                text: "Enable (-s)"
                                font.pixelSize: 13
                                checked: false
                            }
                            Text {
                                text: "Angle:"
                                font.pixelSize: 13
                                color: enableSupplementCheckBox.checked ? "#2C3E50" : "#BDC3C7"
                            }
                            SpinBox {
                                id: supplementAngleSpinBox
                                from: 0
                                to: 90
                                value: 45
                                enabled: enableSupplementCheckBox.checked
                                Layout.preferredWidth: 120
                            }
                        }
                    }

                    // Markerfree parameters
                    GroupBox {
                        title: "Markerfree Parameters"
                        visible: methodComboBox.currentIndex === 1
                        Layout.fillWidth: true

                        GridLayout {
                            anchors.fill: parent
                            columns: 4
                            columnSpacing: 15
                            rowSpacing: 10

                            // Offset
                            Text {
                                text: "Offset:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: markerfreeOffsetSpinBox
                                from: -1000
                                to: 1000
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            // Tilt Axis Angle
                            Text {
                                text: "Tilt Axis Angle:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: markerfreeTiltAngleSpinBox
                                from: -180
                                to: 180
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            // Z-Axis Offset
                            Text {
                                text: "Z-Axis Offset:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: markerfreeZOffsetSpinBox
                                from: -1000
                                to: 1000
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            // Thickness
                            Text {
                                text: "Thickness:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: markerfreeThicknessSpinBox
                                from: 0
                                to: 2000
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            // Proj Match Thickness
                            Text {
                                text: "Proj Match Thickness:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: markerfreeProjThicknessSpinBox
                                from: 0
                                to: 1000
                                value: 300
                                Layout.preferredWidth: 120
                            }

                            // Downsample Ratio
                            Text {
                                text: "Downsample Ratio:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: markerfreeDownsampleSpinBox
                                from: 1
                                to: 100
                                value: 10
                                stepSize: 1
                                property real realValue: value / 10.0
                                textFromValue: function (value) {
                                    return (value / 10.0).toFixed(1)
                                }
                                valueFromText: function (text) {
                                    return Math.round(parseFloat(text) * 10)
                                }
                                Layout.preferredWidth: 120
                            }

                            // GPU ID
                            Text {
                                text: "GPU ID:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: gpuIdSpinBox
                                from: 0
                                to: 7
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            // Save Mode
                            Text {
                                text: "Save Mode:"
                                font.pixelSize: 13
                            }
                            ComboBox {
                                id: markerfreeSaveModeComboBox
                                model: ["TXT Format (0)", "XF Format (1)"]
                                currentIndex: 1
                                Layout.preferredWidth: 150
                            }
                        }
                    }

                    // TiltRec option
                    CheckBox {
                        id: enableTiltRecCheckBox
                        text: "Enable 3D Reconstruction (TiltRec)"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    // TiltRec parameters
                    GroupBox {
                        title: "TiltRec Parameters"
                        visible: enableTiltRecCheckBox.checked
                        Layout.fillWidth: true

                        GridLayout {
                            anchors.fill: parent
                            columns: 4
                            columnSpacing: 15
                            rowSpacing: 10

                            // Method
                            Text {
                                text: "Method:"
                                font.pixelSize: 13
                            }
                            ComboBox {
                                id: tiltRecMethodComboBox
                                Layout.preferredWidth: 150
                                model: ["BPT", "FBP", "WBP", "SART", "SIRT", "ADMM"]
                                currentIndex: 3 // SART
                            }

                            // Acceleration
                            Text {
                                text: "Acceleration:"
                                font.pixelSize: 13
                            }
                            ComboBox {
                                id: tiltRecAccMethodComboBox
                                Layout.preferredWidth: 150
                                model: ["mpi", "cuda"]
                                currentIndex: 0
                            }

                            // Threads
                            Text {
                                text: "Threads:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecThreadsSpinBox
                                from: 1
                                to: 64
                                value: 4
                                Layout.preferredWidth: 120
                            }

                            // Axis
                            Text {
                                text: "Reconstruction Axis:"
                                font.pixelSize: 13
                            }
                            ComboBox {
                                id: tiltRecAxisComboBox
                                Layout.preferredWidth: 150
                                model: ["Y", "Z"]
                                currentIndex: 0
                            }

                            // Bin Factor
                            Text {
                                text: "Bin Factor:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecBinFactorSpinBox
                                from: 1
                                to: 8
                                value: 1
                                Layout.preferredWidth: 120
                            }

                            // Geometry parameters
                            Text {
                                text: "Geometry Offset:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecGeoOffsetSpinBox
                                from: -1000
                                to: 1000
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "Geometry Pitch:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecGeoPitchSpinBox
                                from: -1000
                                to: 1000
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "Geometry Z-Offset:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecGeoZOffsetSpinBox
                                from: -1000
                                to: 1000
                                value: 0
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "Geometry Thickness:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecGeoThicknessSpinBox
                                from: 0
                                to: 2000
                                value: 100
                                editable: true
                                Layout.preferredWidth: 120
                            }

                            // Method-specific parameters
                            Text {
                                text: "Iterations:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecIterationsSpinBox
                                from: 1
                                to: 1000
                                value: 10
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "Relaxation:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecRelaxationSpinBox
                                from: 1
                                to: 100
                                value: 10
                                stepSize: 1
                                property real realValue: value / 10.0
                                textFromValue: function (value) {
                                    return (value / 10.0).toFixed(1)
                                }
                                valueFromText: function (text) {
                                    return Math.round(parseFloat(text) * 10)
                                }
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "CG Iterations:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecCgIterationsSpinBox
                                from: 1
                                to: 100
                                value: 5
                                Layout.preferredWidth: 120
                            }

                            Text {
                                text: "Threshold:"
                                font.pixelSize: 13
                            }
                            SpinBox {
                                id: tiltRecThresholdSpinBox
                                from: 0
                                to: 100
                                value: 0
                                stepSize: 1
                                property real realValue: value / 10.0
                                textFromValue: function (value) {
                                    return (value / 10.0).toFixed(1)
                                }
                                valueFromText: function (text) {
                                    return Math.round(parseFloat(text) * 10)
                                }
                                Layout.preferredWidth: 120
                            }
                        }
                    }
                }
            }

            // ========== Progress Display ==========
            GroupBox {
                title: "Processing Progress"
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.maximumWidth: 1200
                Layout.alignment: Qt.AlignHCenter
                visible: BatchProcessHandler.isProcessing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    Text {
                        text: `Processing: ${BatchProcessHandler.currentFileName} (${BatchProcessHandler.currentIndex
                              + 1}/${BatchProcessHandler.totalCount})`
                        font.pixelSize: 14
                        color: "#2C3E50"
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 25
                        from: 0
                        to: 100
                        value: BatchProcessHandler.overallProgress
                    }

                    Text {
                        text: `Completed: ${BatchProcessHandler.successCount} | Failed: ${BatchProcessHandler.failedCount}`
                        font.pixelSize: 13
                        color: "#7F8C8D"
                    }

                    Text {
                        id: elapsedTimeText
                        text: "Elapsed: 00:00:00"
                        font.pixelSize: 13
                        color: "#7F8C8D"
                    }
                }
            }

            // ========== Action Buttons ==========
            RowLayout {
                spacing: 15
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 10
                Layout.bottomMargin: 30

                Button {
                    text: "Start Batch Processing"
                    font.pixelSize: 16
                    font.bold: true
                    Layout.preferredWidth: 220
                    Layout.preferredHeight: 50
                    enabled: !BatchProcessHandler.isProcessing
                             && fileScanner.pairCount > 0
                             && outputDirField.text !== ""

                    onClicked: startBatchProcessing()

                    background: Rectangle {
                        color: parent.enabled ? "#27AE60" : "#BDC3C7"
                        radius: 6
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: "Stop Processing"
                    font.pixelSize: 16
                    Layout.preferredWidth: 170
                    Layout.preferredHeight: 50
                    enabled: BatchProcessHandler.isProcessing
                    onClicked: BatchProcessHandler.stopBatch()

                    background: Rectangle {
                        color: parent.enabled ? "#E74C3C" : "#BDC3C7"
                        radius: 6
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    // ========== File List Model ==========
    ListModel {
        id: fileListModel
    }

    // ========== Hidden Fields ==========
    QtObject {
        id: outputDirField
        property string text: ""
    }

    // ========== File Dialog ==========
    FileDialog {
        id: fileDialog
        title: "Select MRC and Rawtlt Files"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["MRC and Rawtlt files (*.mrc *.st *.rawtlt *.tlt)", "All files (*)"]

        onAccepted: {
            fileScanner.autoMatchPairs(selectedFiles)
        }
    }



    // ========== File Scanner Signal Handling ==========
    Connections {
        target: fileScanner

        function onScanCompleted(pairCount) {
            console.log("Scan completed, found", pairCount, "pairs")

            // Update file list model
            fileListModel.clear()
            let pairs = fileScanner.getAllPairs()
            for (var i = 0; i < pairs.length; i++) {
                fileListModel.append(pairs[i])
            }
        }

        function onScanFailed(error) {
            console.error("Scan failed:", error)
        }
    }

    // ========== Batch Processing Signal Handling ==========
    Connections {
        target: BatchProcessHandler

        function onBatchCompleted(successCount, failedCount) {
            console.log("Batch completed:", successCount, "succeeded,",
                        failedCount, "failed")
            batchResultTitle.text = failedCount > 0 ? "Batch Completed with Warnings" : "Batch Completed Successfully"
            batchResultTitle.color = failedCount > 0 ? "#E67E22" : "#27AE60"

            // 【修改】去掉 Math.floor，直接除以 1000 得到高精度的浮点数秒
            let totalElapsed = batchStartTime ? (new Date() - batchStartTime) / 1000 : 0

            batchResultMessage.text = `Total: ${BatchProcessHandler.totalCount}\nSuccess: ${successCount}\nFailed: ${failedCount}\nTime: ${formatElapsedTime(totalElapsed)}`
            batchResultPopup.open()
        }

        function onBatchStopped() {
            console.log("Batch stopped by user")
            batchResultTitle.text = "Batch Stopped"
            batchResultTitle.color = "#E67E22"
            let totalElapsed = batchStartTime ? Math.floor((new Date() - batchStartTime) / 1000) : 0
            batchResultMessage.text = "The batch processing was stopped by user.\nTime: " + formatElapsedTime(totalElapsed)
            batchResultPopup.open()
        }
    }

    // ========== Batch Result Popup ==========
    Popup {
        id: batchResultPopup
        anchors.centerIn: parent
        width: 360
        height: 200
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#FFFFFF"
            border.color: "#BDC3C7"
            border.width: 1
            radius: 8
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Text {
                id: batchResultTitle
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                id: batchResultMessage
                font.pixelSize: 14
                color: "#7F8C8D"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                text: "OK"
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 100
                onClicked: batchResultPopup.close()
            }
        }
    }

    // ========== Functions ==========

    function formatElapsedTime(totalSeconds) {
        let hours = Math.floor(totalSeconds / 3600)
        let minutes = Math.floor((totalSeconds % 3600) / 60)
        let seconds = totalSeconds % 60
        let pad = function(n) { return n < 10 ? "0" + n : n.toString() }
        return pad(hours) + ":" + pad(minutes) + ":" + pad(seconds)
    }

    /**
     * Start batch processing
     */
    function startBatchProcessing() {
        batchStartTime = new Date()
        // Build configuration object
        let config = {
            "method": methodComboBox.currentIndex === 0 ? "markerauto2" : "markerfree",
            "outputDir": outputDirField.text,
            "markerauto2": {
                "beadDiameter": beadDiameterSpinBox.value,
                "eraseMarkers": eraseMarkersCheckBox.checked,
                "enableSupplement": enableSupplementCheckBox.checked,
                "supplementAngle": supplementAngleSpinBox.value
            },
            "markerfree": {
                "geometry": [markerfreeOffsetSpinBox.value, markerfreeTiltAngleSpinBox.value, markerfreeZOffsetSpinBox.value, markerfreeThicknessSpinBox.value, markerfreeProjThicknessSpinBox.value, markerfreeDownsampleSpinBox.realValue, gpuIdSpinBox.value],
                "saveMode": markerfreeSaveModeComboBox.currentIndex
            },
            "enableTiltRec": enableTiltRecCheckBox.checked,
            "tiltRec": {
                "method": tiltRecMethodComboBox.currentText,
                "accMethod": tiltRecAccMethodComboBox.currentText.toLowerCase(),
                "threads": tiltRecThreadsSpinBox.value,
                "geometry": [tiltRecGeoOffsetSpinBox.value, tiltRecGeoPitchSpinBox.value, tiltRecGeoZOffsetSpinBox.value, tiltRecGeoThicknessSpinBox.value],
                "axis": tiltRecAxisComboBox.currentText.toLowerCase(),
                "binFactor": tiltRecBinFactorSpinBox.value,
                "iterations": tiltRecIterationsSpinBox.value,
                "relaxation": tiltRecRelaxationSpinBox.realValue,
                "cgIterations": tiltRecCgIterationsSpinBox.value,
                "threshold": tiltRecThresholdSpinBox.realValue
            }
        }

        // Get file list
        let files = fileScanner.getAllPairs()

        console.log("Starting batch processing with config:",
                    JSON.stringify(config, null, 2))
        console.log("Files:", files.length)

        // Start batch processing
        let success = BatchProcessHandler.startBatch(files, config)

        if (!success) {
            console.error("Failed to start batch processing")
        }
    }
}
