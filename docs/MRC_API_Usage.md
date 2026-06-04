# MRC 读取 API 使用说明

## 概述

本文档描述了如何在 QML 前端使用 C++ 后端提供的 MRC 文件读取功能。该 API 允许你读取 MRC 文件的单张切片并在 QML 界面中显示。

---

## API 参考

### `getMrcSliceCount(filePath)`

获取 MRC 文件中的切片总数。

**函数签名：**
```qml
int realBackend.getMrcSliceCount(QString filePath)
```

**参数：**
- `filePath` (QString): MRC 文件的完整路径（绝对路径或相对路径）

**返回值：**
- `int`: MRC 文件中的切片总数（nz 字段）
- 如果加载失败，返回 -1

**注意事项：**
1. 该方法只读取 MRC 文件头，非常快速
2. 返回 -1 表示文件打开失败
3. 错误信息会输出到控制台

**示例：**
```qml
var totalSlices = realBackend.getMrcSliceCount("/path/to/file.mrc");
if (totalSlices > 0) {
    console.log("MRC file has", totalSlices, "slices");
    sliceSpinBox.to = totalSlices - 1;
} else {
    console.error("Failed to read MRC file");
}
```

---

### `loadMrcSlice(filePath, sliceIndex)`

从 MRC 文件中加载指定索引的切片图像。

**函数签名：**
```qml
QImage realBackend.loadMrcSlice(QString filePath, int sliceIndex)
```

**参数：**
- `filePath` (QString): MRC 文件的完整路径（绝对路径或相对路径）
- `sliceIndex` (int): 要加载的切片索引，从 0 开始

**返回值：**
- `QImage`: 加载的图片（8位灰度图，已归一化到 0-255 范围）
- 如果加载失败，返回空的 QImage（width = 0, height = 0）

**支持的 MRC 模式：**
- `MRC_MODE_BYTE` (mode 0): 8位无符号整数
- `MRC_MODE_SHORT` (mode 1): 16位有符号整数
- `MRC_MODE_FLOAT` (mode 2): 32位浮点数

**注意事项：**
1. 该方法在主线程中**同步执行**，加载大文件可能会短暂阻塞 UI
2. 图片自动进行 min-max 归一化处理（映射到 0-255 范围）
3. 返回的图片格式为 `QImage::Format_Grayscale8`
4. 错误信息会输出到控制台（qWarning/qDebug）
5. 切片索引超出范围或文件不存在时，会在控制台输出警告并返回空图片

---

## 基本用法示例

### 1. 最简单的示例

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 800
    height: 600
    visible: true

    Column {
        anchors.centerIn: parent
        spacing: 20

        Image {
            id: mrcImage
            width: 512
            height: 512
            fillMode: Image.PreserveAspectFit
        }

        Button {
            text: "Load MRC Slice"
            onClicked: {
                // 加载 MRC 文件的第 0 张切片
                var qimage = realBackend.loadMrcSlice(
                    "/path/to/your/file.mrc",
                    0
                );
                mrcImage.source = qimage;
            }
        }
    }
}
```

---

### 2. 完整的 MRC 查看器示例

包含文件选择、切片索引控制和状态显示：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

ApplicationWindow {
    id: mainWindow
    width: 1000
    height: 700
    visible: true
    title: "MRC Viewer"

    property int totalSlices: 0

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // 文件路径选择
        Row {
            spacing: 10
            width: parent.width

            Label {
                text: "MRC File:"
                width: 100
                anchors.verticalCenter: parent.verticalCenter
            }

            TextField {
                id: filePathField
                width: parent.width - 220
                placeholderText: "Select an MRC file..."
                readOnly: true
            }

            Button {
                text: "Browse..."
                onClicked: fileDialog.open()
            }
        }

        // 切片索引控制
        Row {
            spacing: 10
            width: parent.width

            Label {
                text: "Slice Index:"
                width: 100
                anchors.verticalCenter: parent.verticalCenter
            }

            SpinBox {
                id: sliceIndexSpinBox
                from: 0
                to: 999
                value: 0
                editable: true
            }

            Label {
                text: "/ " + (totalSlices - 1)
                visible: totalSlices > 0
                anchors.verticalCenter: parent.verticalCenter
            }

            Button {
                text: "Load Slice"
                enabled: filePathField.text !== ""
                onClicked: loadMrcSlice()
            }

            Button {
                text: "Previous"
                enabled: sliceIndexSpinBox.value > 0
                onClicked: {
                    sliceIndexSpinBox.value--;
                    loadMrcSlice();
                }
            }

            Button {
                text: "Next"
                enabled: sliceIndexSpinBox.value < totalSlices - 1
                onClicked: {
                    sliceIndexSpinBox.value++;
                    loadMrcSlice();
                }
            }
        }

        // 图片显示区域
        Rectangle {
            width: parent.width
            height: parent.height - 150
            color: "#2a2a2a"
            border.color: "#555"
            border.width: 1

            Image {
                id: mrcImage
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                width: parent.width - 20
                height: parent.height - 20

                Text {
                    anchors.centerIn: parent
                    text: "No image loaded"
                    color: "#888"
                    visible: mrcImage.source == ""
                    font.pixelSize: 16
                }
            }
        }

        // 状态信息
        Label {
            id: statusLabel
            width: parent.width
            text: "Ready"
            color: "#666"
        }
    }

    // 文件选择对话框
    FileDialog {
        id: fileDialog
        title: "Select MRC File"
        nameFilters: ["MRC files (*.mrc)", "All files (*)"]
        onAccepted: {
            filePathField.text = selectedFile.toString().replace("file://", "");

            // 获取 MRC 文件的切片总数
            totalSlices = realBackend.getMrcSliceCount(filePathField.text);

            if (totalSlices > 0) {
                sliceIndexSpinBox.to = totalSlices - 1;
                statusLabel.text = "File loaded: " + totalSlices + " slices";
                statusLabel.color = "#00aa00";
            } else {
                statusLabel.text = "Failed to read MRC file";
                statusLabel.color = "#cc0000";
            }
        }
    }

    // 加载 MRC 切片的函数
    function loadMrcSlice() {
        try {
            statusLabel.text = "Loading slice " + sliceIndexSpinBox.value + "...";
            statusLabel.color = "#0066cc";

            var qimage = realBackend.loadMrcSlice(
                filePathField.text,
                sliceIndexSpinBox.value
            );

            if (qimage.width > 0 && qimage.height > 0) {
                mrcImage.source = qimage;
                statusLabel.text = "Loaded: " + qimage.width + "x" + qimage.height +
                                   " (Slice " + sliceIndexSpinBox.value + ")";
                statusLabel.color = "#00aa00";
            } else {
                statusLabel.text = "Failed to load slice. Check console for errors.";
                statusLabel.color = "#cc0000";
            }
        } catch (error) {
            statusLabel.text = "Error: " + error;
            statusLabel.color = "#cc0000";
        }
    }
}
```

---

### 3. 在现有 Autom2 项目中集成

#### 3.1 在 AppState.qml 中添加属性

```qml
// src/qmlapp/Autom2qml/AppState.qml
pragma Singleton
import QtQuick

QtObject {
    // ... 现有属性 ...

    // MRC 查看器相关属性
    property string viewMrcFile: ""
    property int viewMrcSliceIndex: 0
    property var viewMrcImage: null
}
```

#### 3.2 在 LogicHandler.qml 中添加辅助函数

```qml
// src/qmlapp/Autom2qml/LogicHandler.qml
pragma Singleton
import QtQuick

QtObject {
    // ... 现有属性和函数 ...

    // 加载 MRC 切片
    function loadMrcSlice(backend, filePath, sliceIndex) {
        console.log("Loading MRC slice:", filePath, "index:", sliceIndex);

        if (!filePath || filePath === "") {
            console.warn("MRC file path is empty");
            return null;
        }

        var image = backend.loadMrcSlice(filePath, sliceIndex);

        if (image.width > 0 && image.height > 0) {
            console.log("MRC slice loaded successfully:", image.width, "x", image.height);
            AppState.viewMrcImage = image;
            AppState.viewMrcFile = filePath;
            AppState.viewMrcSliceIndex = sliceIndex;
        } else {
            console.error("Failed to load MRC slice");
            AppState.viewMrcImage = null;
        }

        return image;
    }

    // 加载当前配置的 MRC 文件
    function loadCurrentMrc(backend, sliceIndex) {
        if (AppState.initialMrcFile !== "") {
            return loadMrcSlice(backend, AppState.initialMrcFile, sliceIndex);
        } else if (AppState.alignedMrcFile !== "") {
            return loadMrcSlice(backend, AppState.alignedMrcFile, sliceIndex);
        } else if (AppState.reconstructionMrcFile !== "") {
            return loadMrcSlice(backend, AppState.reconstructionMrcFile, sliceIndex);
        }
        console.warn("No MRC file available to load");
        return null;
    }
}
```

#### 3.3 在任何 Screen 中使用

```qml
// 例如在 Screen01.ui.qml 中添加一个预览按钮
import Autom2qml

Rectangle {
    // ... 现有 UI 元素 ...

    Button {
        text: "Preview MRC"
        enabled: AppState.initialMrcFile !== ""
        onClicked: {
            LogicHandler.loadMrcSlice(
                backend,
                AppState.initialMrcFile,
                0  // 加载第一张切片
            );
            // 可以显示一个弹出窗口或切换到查看器页面
        }
    }

    // MRC 图片预览（可选）
    Image {
        id: mrcPreview
        source: AppState.viewMrcImage
        width: 200
        height: 200
        fillMode: Image.PreserveAspectFit
    }
}
```

---

## 进阶用法

### 4. 创建一个可重用的 MRC 查看器组件

创建一个新文件 `MrcViewer.qml`:

```qml
// src/qmlapp/Autom2qmlContent/MrcViewer.qml
import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    // 公开属性
    property string mrcFilePath: ""
    property int currentSlice: 0
    property alias image: mrcImage.source

    color: "#2a2a2a"
    border.color: "#555"
    border.width: 1

    // 自动加载
    onMrcFilePathChanged: {
        if (mrcFilePath !== "") {
            loadSlice(currentSlice);
        }
    }

    onCurrentSliceChanged: {
        if (mrcFilePath !== "") {
            loadSlice(currentSlice);
        }
    }

    // 图片显示
    Image {
        id: mrcImage
        anchors.fill: parent
        anchors.margins: 10
        fillMode: Image.PreserveAspectFit

        Text {
            anchors.centerIn: parent
            text: "No MRC loaded"
            color: "#888"
            visible: mrcImage.source == ""
        }
    }

    // 控制栏
    Row {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 10
        spacing: 10

        Button {
            text: "◀"
            enabled: root.currentSlice > 0
            onClicked: root.currentSlice--
        }

        Label {
            text: "Slice: " + root.currentSlice
            color: "#fff"
            anchors.verticalCenter: parent.verticalCenter
        }

        Button {
            text: "▶"
            onClicked: root.currentSlice++
        }
    }

    // 加载函数
    function loadSlice(index) {
        var img = realBackend.loadMrcSlice(mrcFilePath, index);
        if (img.width > 0 && img.height > 0) {
            mrcImage.source = img;
        } else {
            console.error("Failed to load slice", index, "from", mrcFilePath);
        }
    }
}
```

使用该组件：

```qml
import Autom2qmlContent

Rectangle {
    MrcViewer {
        anchors.fill: parent
        mrcFilePath: AppState.initialMrcFile
        currentSlice: 0
    }
}
```

---

## 错误处理

### 常见错误及解决方案

1. **文件不存在**
   ```
   Failed to open MRC file: /path/to/file.mrc
   ```
   - 检查文件路径是否正确
   - 检查文件是否存在
   - 检查文件权限

2. **切片索引超出范围**
   ```
   Invalid slice index: 100 Total slices: 50
   ```
   - 确保 sliceIndex 在 0 到 (totalSlices-1) 范围内
   - 可以通过日志查看实际的切片数量

3. **返回空图片**
   - 检查 MRC 文件格式是否正确
   - 查看控制台输出的错误信息
   - 验证 MRC 文件的 mode 字段是否支持（0, 1, 2）

### 调试技巧

在加载 MRC 时添加详细的日志：

```qml
function loadMrcSliceWithDebug(filePath, sliceIndex) {
    console.log("=== Loading MRC ===");
    console.log("File:", filePath);
    console.log("Slice:", sliceIndex);

    var startTime = new Date().getTime();
    var image = realBackend.loadMrcSlice(filePath, sliceIndex);
    var loadTime = new Date().getTime() - startTime;

    console.log("Load time:", loadTime, "ms");
    console.log("Image size:", image.width, "x", image.height);
    console.log("==================");

    return image;
}
```

---

## 性能考虑

1. **加载时间**
   - 小文件 (< 1MB): < 100ms
   - 中等文件 (1-10MB): 100-500ms
   - 大文件 (> 10MB): 可能超过 500ms

2. **优化建议**
   - 避免在循环中频繁加载
   - 考虑缓存常用的切片
   - 对于大文件，考虑在后台线程加载（未来改进）

3. **内存使用**
   - 每张图片占用: width × height × 1 字节（灰度图）
   - 例如：2048×2048 的图片约占用 4MB 内存

---

## 技术实现细节

### C++ 后端实现

MRC 读取功能位于：
- **头文件**: `src/invoker/procinvoker.h`
- **实现**: `src/invoker/procinvoker.cpp`
- **MRC 读取库**: `src/trajplot_opencv4/src/mrcimg/mrc2img.h`

实现流程：
1. 使用 `util::MrcStack` 打开 MRC 文件
2. 读取指定索引的切片（返回 `cv::Mat`）
3. 归一化到 0-255 范围（`cv::normalize`）
4. 转换为 8 位灰度图（`cv::CV_8UC1`）
5. 转换为 `QImage::Format_Grayscale8`
6. 返回深拷贝的 `QImage`

### 构建配置

相关的 CMakeLists.txt 修改：
- `src/CMakeLists.txt`: 添加了 `mrcimg` 子目录
- `src/invoker/CMakeLists.txt`: 链接了 `Qt6::Gui` 和 `mrcimg` 库

---

## 完整修改清单

本功能涉及以下文件的修改：

1. `src/invoker/procinvoker.h`
   - 添加 `QImage` 头文件
   - 添加 `loadMrcSlice()` 方法声明

2. `src/invoker/procinvoker.cpp`
   - 添加 MRC 和 OpenCV 头文件
   - 实现 `loadMrcSlice()` 方法

3. `src/invoker/CMakeLists.txt`
   - 添加 mrcimg 头文件路径
   - 链接 Qt6::Gui 和 mrcimg 库

4. `src/CMakeLists.txt`
   - 添加 `trajplot_opencv4/src/mrcimg` 子目录

---

## 许可证

本 API 是 Autom2 项目的一部分，遵循项目的开源许可证。

---

## 联系与支持

如有问题或建议，请通过项目仓库的 Issue 提交。

**最后更新**: 2026-01-18
