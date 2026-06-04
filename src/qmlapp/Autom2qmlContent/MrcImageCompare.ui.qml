
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

    property string originalMrcPath: ""
    property string alignedMrcPath: ""
    property int sliceIndex: 0
    property bool autoLoad: true
    property bool autoLoadMiddleSlice: true  // 是否自动加载中间层
    property int refreshTrigger: 0  // 刷新触发器，改变这个值会重新加载图像

    property int originalTotalSlices: 0
    property int alignedTotalSlices: 0

    color: "#ECEFF1"
    border.color: "#BDC3C7"
    border.width: 1

    // 监听刷新触发器，手动刷新图像
    onRefreshTriggerChanged: {
        console.log("MrcImageCompare: refreshTrigger changed, reloading images...")
        if (originalMrcPath !== "") loadOriginalImage()
        if (alignedMrcPath !== "") loadAlignedImage()
    }

    // 当路径改变时自动获取总层数并加载图像
    onOriginalMrcPathChanged: {
        if (originalMrcPath !== "") {
            originalTotalSlices = backend.getMrcSliceCount(originalMrcPath)
            if (originalTotalSlices > 0) {
                console.log("Original MRC has", originalTotalSlices, "slices")

                // 如果启用自动加载中间层，计算中间层索引
                if (autoLoadMiddleSlice) {
                    sliceIndex = Math.floor(originalTotalSlices / 2)
                    console.log("Auto-loading middle slice:", sliceIndex)
                }

                if (autoLoad) {
                    loadOriginalImage()
                }
            } else {
                console.warn("Failed to get slice count for:", originalMrcPath)
            }
        }
    }

    onAlignedMrcPathChanged: {
        if (alignedMrcPath !== "") {
            alignedTotalSlices = backend.getMrcSliceCount(alignedMrcPath)
            if (alignedTotalSlices > 0) {
                console.log("Aligned MRC has", alignedTotalSlices, "slices")

                if (autoLoad) {
                    loadAlignedImage()
                }
            } else {
                console.warn("Failed to get slice count for:", alignedMrcPath)
            }
        }
    }

    onSliceIndexChanged: {
        if (autoLoad) {
            if (originalMrcPath !== "") loadOriginalImage()
            if (alignedMrcPath !== "") loadAlignedImage()
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 左侧：原始图像
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#2a2a2a"
            border.color: "#555"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 标题
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "#34495E"

                    Text {
                        anchors.centerIn: parent
                        text: {
                            var sliceText = "Slice " + sliceIndex
                            if (originalTotalSlices > 0) {
                                sliceText += " / " + (originalTotalSlices - 1)
                            }
                            return "Original MRC (" + sliceText + ")"
                        }
                        color: "#ECF0F1"
                        font.pixelSize: 12
                        font.bold: true
                    }
                }

                // 图像显示区域
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Image {
                        id: originalImage
                        anchors.fill: parent
                        anchors.margins: 5
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: {
                                if (originalMrcPath === "") {
                                    return "No file selected"
                                } else if (originalImage.source.toString().startsWith("data:")) {
                                    return "Mock mode - placeholder image"
                                } else if (originalImage.source == "") {
                                    return "Loading..."
                                } else {
                                    return ""
                                }
                            }
                            color: "#888"
                            visible: text !== ""
                            font.pixelSize: 14
                        }
                    }
                }

                // 状态栏
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 25
                    color: "#2C3E50"

                    Text {
                        id: originalStatus
                        anchors.centerIn: parent
                        text: originalImage.sourceSize.width > 0
                              ? originalImage.sourceSize.width + " × " + originalImage.sourceSize.height
                              : "No image"
                        color: "#95A5A6"
                        font.pixelSize: 10
                        font.family: "Courier"
                    }
                }
            }
        }

        // 右侧：对齐后图像
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#2a2a2a"
            border.color: "#555"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 标题
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: "#16A085"

                    Text {
                        anchors.centerIn: parent
                        text: {
                            var sliceText = "Slice " + sliceIndex
                            if (alignedTotalSlices > 0) {
                                sliceText += " / " + (alignedTotalSlices - 1)
                            }
                            return "Aligned MRC (" + sliceText + ")"
                        }
                        color: "#ECF0F1"
                        font.pixelSize: 12
                        font.bold: true
                    }
                }

                // 图像显示区域
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Image {
                        id: alignedImage
                        anchors.fill: parent
                        anchors.margins: 5
                        fillMode: Image.PreserveAspectFit
                        cache: false

                        Text {
                            anchors.centerIn: parent
                            text: {
                                if (alignedMrcPath === "") {
                                    return "Not aligned yet"
                                } else if (alignedImage.source.toString().startsWith("data:")) {
                                    return "Mock mode - placeholder image"
                                } else if (alignedImage.source == "") {
                                    return "Loading..."
                                } else {
                                    return ""
                                }
                            }
                            color: "#888"
                            visible: text !== ""
                            font.pixelSize: 14
                        }
                    }
                }

                // 状态栏
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 25
                    color: "#2C3E50"

                    Text {
                        id: alignedStatus
                        anchors.centerIn: parent
                        text: alignedImage.sourceSize.width > 0
                              ? alignedImage.sourceSize.width + " × " + alignedImage.sourceSize.height
                              : "No image"
                        color: "#95A5A6"
                        font.pixelSize: 10
                        font.family: "Courier"
                    }
                }
            }
        }
    }

    // 控制栏（切片索引调整）
    RowLayout {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 10
        spacing: 10

        Button {
            text: "◀ Prev"
            enabled: sliceIndex > 0
            onClicked: sliceIndex--
        }

        Text {
            text: {
                var maxSlices = Math.max(originalTotalSlices, alignedTotalSlices)
                if (maxSlices > 0) {
                    return "Slice: " + sliceIndex + " / " + (maxSlices - 1)
                } else {
                    return "Slice: " + sliceIndex
                }
            }
            color: "#34495E"
            font.bold: true
            font.pixelSize: 12
        }

        Button {
            text: "Next ▶"
            enabled: {
                var maxSlices = Math.max(originalTotalSlices, alignedTotalSlices)
                return maxSlices > 0 ? sliceIndex < (maxSlices - 1) : true
            }
            onClicked: sliceIndex++
        }

        Item { Layout.preferredWidth: 20 }

        Button {
            text: "Reload"
            onClicked: {
                loadOriginalImage()
                loadAlignedImage()
            }
        }

        Button {
            text: "Middle Slice"
            enabled: originalTotalSlices > 0
            onClicked: {
                sliceIndex = Math.floor(originalTotalSlices / 2)
            }
        }
    }

    // 加载函数
    function loadOriginalImage() {
        if (originalMrcPath === "") {
            originalImage.source = ""
            return
        }

        try {
            var img = backend.loadMrcSlice(originalMrcPath, sliceIndex)

            // 检查是否是有效的图片标识符
            if (img && img !== "") {
                // 如果是MockBackend的data URI，直接使用
                if (img.toString().startsWith("data:")) {
                    originalImage.source = img
                    console.log("Original image loaded (Mock mode)")
                } else {
                    // 真实Backend返回ImageProvider的缓存key，需要转换为image:// URL
                    originalImage.source = "image://mrc/" + img
                    console.log("Original image loaded from ImageProvider:", img)
                }
            } else {
                originalImage.source = ""
                console.warn("Failed to load original MRC slice:", originalMrcPath, "index:", sliceIndex)
            }
        } catch (error) {
            console.error("Error loading original MRC:", error)
            originalImage.source = ""
        }
    }

    function loadAlignedImage() {
        if (alignedMrcPath === "") {
            alignedImage.source = ""
            return
        }

        try {
            var img = backend.loadMrcSlice(alignedMrcPath, sliceIndex)

            // 检查是否是有效的图片标识符
            if (img && img !== "") {
                // 如果是MockBackend的data URI，直接使用
                if (img.toString().startsWith("data:")) {
                    alignedImage.source = img
                    console.log("Aligned image loaded (Mock mode)")
                } else {
                    // 真实Backend返回ImageProvider的缓存key，需要转换为image:// URL
                    alignedImage.source = "image://mrc/" + img
                    console.log("Aligned image loaded from ImageProvider:", img)
                }
            } else {
                alignedImage.source = ""
                console.warn("Failed to load aligned MRC slice:", alignedMrcPath, "index:", sliceIndex)
            }
        } catch (error) {
            console.error("Error loading aligned MRC:", error)
            alignedImage.source = ""
        }
    }
}
