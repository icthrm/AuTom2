# MRC Image Provider 使用说明

## 重要变更（2026-01-19）

`loadMrcSlice()` 的返回类型已从 `QImage` 改为 `QString`（图片缓存键）。

## API 更新

### `loadMrcSlice(filePath, sliceIndex)` - 已修改

**新签名：**
```qml
QString realBackend.loadMrcSlice(QString filePath, int sliceIndex)
```

**返回值：**
- `QString`: 图片缓存键（用于 ImageProvider）
- 空字符串表示加载失败

**使用方式：**
```qml
Image {
    id: mrcImage
    source: {
        var key = realBackend.loadMrcSlice("/path/to/file.mrc", 0);
        return key ? "image://mrc/" + key : "";
    }
}
```

### `getMrcSliceCount(filePath)` - 未变

```qml
int realBackend.getMrcSliceCount(QString filePath)
```

## 完整示例

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 1000
    height: 700
    visible: true

    property string mrcFilePath: "/path/to/file.mrc"
    property int totalSlices: 0
    property int currentSlice: 0

    Component.onCompleted: {
        totalSlices = realBackend.getMrcSliceCount(mrcFilePath);
        if (totalSlices > 0) {
            loadSlice(0);
        }
    }

    Column {
        anchors.fill: parent
        spacing: 10

        // 图片显示
        Image {
            id: mrcImage
            width: 800
            height: 600
            fillMode: Image.PreserveAspectFit
            cache: false  // 重要：禁用缓存以便更新
        }

        // 控制按钮
        Row {
            spacing: 10

            Button {
                text: "Previous"
                enabled: currentSlice > 0
                onClicked: loadSlice(currentSlice - 1)
            }

            Text {
                text: "Slice: " + currentSlice + " / " + (totalSlices - 1)
                anchors.verticalCenter: parent.verticalCenter
            }

            Button {
                text: "Next"
                enabled: currentSlice < totalSlices - 1
                onClicked: loadSlice(currentSlice + 1)
            }
        }
    }

    function loadSlice(index) {
        var key = realBackend.loadMrcSlice(mrcFilePath, index);
        if (key) {
            mrcImage.source = "image://mrc/" + key;
            currentSlice = index;
        } else {
            console.error("Failed to load slice", index);
        }
    }
}
```

## 技术细节

### ImageProvider 工作原理

1. `realBackend.loadMrcSlice()` 读取 MRC 文件并缓存图片
2. 返回一个唯一的缓存键（例如 "mrc_0", "mrc_1"）
3. QML Image 组件通过 `image://mrc/` 协议访问缓存的图片
4. ImageProvider 根据缓存键返回对应的 QImage

### 优点

- ✅ 符合 Qt/QML 标准做法
- ✅ 图片在 C++ 端缓存，避免重复读取
- ✅ 支持任意数量的图片
- ✅ QML Image 组件可以正常工作（缩放、缓存等）

### 注意事项

1. **禁用 Image 缓存**：`cache: false`
   - 否则切换图片时可能不更新

2. **内存管理**：
   - 图片缓存在 C++ 端
   - 当前实现会一直缓存，未来可以添加清理机制

3. **错误处理**：
   - 返回空字符串表示失败
   - 检查返回值再设置 source

## 迁移指南

### 旧代码（不工作）

```qml
// ❌ 错误：不能直接赋值 QImage
mrcImage.source = realBackend.loadMrcSlice(filePath, index);
```

### 新代码（正确）

```qml
// ✅ 正确：通过 ImageProvider
var key = realBackend.loadMrcSlice(filePath, index);
if (key) {
    mrcImage.source = "image://mrc/" + key;
}
```

---

**更新时间**: 2026-01-19
