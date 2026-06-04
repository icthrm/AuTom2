# MRC API 快速开始

## 一行代码使用

```qml
// 获取 MRC 文件的切片总数
var count = realBackend.getMrcSliceCount("/path/to/file.mrc");

// 加载指定切片
var image = realBackend.loadMrcSlice("/path/to/file.mrc", 0);
```

---

## API 签名

```qml
// 获取切片总数
int getMrcSliceCount(QString filePath)

// 加载指定切片
QImage loadMrcSlice(QString filePath, int sliceIndex)
```

**参数：**
- `filePath`: MRC 文件路径
- `sliceIndex`: 切片索引（从 0 开始）

**返回：**
- `getMrcSliceCount`: 切片总数（失败返回 -1）
- `loadMrcSlice`: 8位灰度图（已归一化到 0-255）

---

## 最简示例

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 800
    height: 600
    visible: true

    property string mrcFile: "/path/to/your/file.mrc"
    property int totalSlices: 0

    Component.onCompleted: {
        // 获取切片总数
        totalSlices = realBackend.getMrcSliceCount(mrcFile);
        console.log("Total slices:", totalSlices);
    }

    Image {
        id: mrcImage
        anchors.centerIn: parent
        width: 512
        height: 512
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Load MRC (Slice 0 / " + (totalSlices - 1) + ")"
        enabled: totalSlices > 0
        onClicked: {
            mrcImage.source = realBackend.loadMrcSlice(mrcFile, 0);
        }
    }
}
```

---

## 在现有项目中使用

### 方法 1: 直接使用

```qml
Button {
    text: "View MRC"
    onClicked: {
        // 先获取切片总数
        var count = realBackend.getMrcSliceCount(AppState.initialMrcFile);
        console.log("MRC has", count, "slices");

        // 加载第一张切片
        myImage.source = realBackend.loadMrcSlice(
            AppState.initialMrcFile,
            0
        );
    }
}
```

### 方法 2: 通过 LogicHandler

在 `LogicHandler.qml` 中添加：

```qml
function loadMrcSlice(backend, filePath, sliceIndex) {
    var image = backend.loadMrcSlice(filePath, sliceIndex);
    if (image.width > 0) {
        AppState.viewMrcImage = image;
        return true;
    }
    return false;
}

function getMrcInfo(backend, filePath) {
    var count = backend.getMrcSliceCount(filePath);
    if (count > 0) {
        console.log("MRC file has", count, "slices");
        return count;
    }
    console.error("Failed to read MRC file");
    return -1;
}
```

在 QML 中调用：

```qml
Button {
    text: "Load"
    onClicked: {
        var sliceCount = LogicHandler.getMrcInfo(backend, AppState.initialMrcFile);
        if (sliceCount > 0) {
            LogicHandler.loadMrcSlice(backend, AppState.initialMrcFile, 0);
        }
    }
}
}
```

---

## 常见用法

### 切片导航

```qml
property int currentSlice: 0
property int totalSlices: 0
property string mrcFilePath: ""

Component.onCompleted: {
    // 获取总切片数
    totalSlices = realBackend.getMrcSliceCount(mrcFilePath);
}

Row {
    Button {
        text: "Previous"
        enabled: currentSlice > 0
        onClicked: {
            currentSlice--;
            loadSlice();
        }
    }

    Text { text: "Slice: " + currentSlice + " / " + (totalSlices - 1) }

    Button {
        text: "Next"
        enabled: currentSlice < totalSlices - 1
        onClicked: {
            currentSlice++;
            loadSlice();
        }
    }
}

function loadSlice() {
    mrcImage.source = realBackend.loadMrcSlice(
        mrcFilePath,
        currentSlice
    );
}
```

### 错误检查

```qml
function loadMrcSafe(filePath, index) {
    // 先检查文件和切片数
    var sliceCount = realBackend.getMrcSliceCount(filePath);

    if (sliceCount <= 0) {
        statusText.text = "Failed to open MRC file";
        return false;
    }

    if (index < 0 || index >= sliceCount) {
        statusText.text = "Invalid slice index: " + index + " (max: " + (sliceCount - 1) + ")";
        return false;
    }

    // 加载切片
    var img = realBackend.loadMrcSlice(filePath, index);

    if (img.width > 0 && img.height > 0) {
        mrcImage.source = img;
        statusText.text = "Loaded: " + img.width + "x" + img.height;
        return true;
    } else {
        statusText.text = "Failed to load slice";
        return false;
    }
}
```

---

## 注意事项

1. **同步执行**：在主线程执行，大文件可能阻塞 UI
2. **自动归一化**：图片自动归一化到 0-255
3. **灰度图**：返回单通道 8 位灰度图
4. **错误处理**：失败时返回空图片（width = 0）

---

## 调试

查看控制台输出：

```bash
# 成功
C++ ProcInvoker: Loading MRC slice from "/path/to/file.mrc" index 0

# 失败
Failed to open MRC file: /path/to/file.mrc
# 或
Invalid slice index: 100 Total slices: 50
```

---

## 下一步

查看完整文档：`docs/MRC_API_Usage.md`

包含：
- 完整示例代码
- 高级用法
- 性能优化
- 错误处理
- 技术细节
