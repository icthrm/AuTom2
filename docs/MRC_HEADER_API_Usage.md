# MRC Header API 使用指南

## 概述

在 ProcInvoker 后端中添加了新的 `getMrcHeader` API，用于获取 MRC 文件的头信息。API 返回 JSON 格式的数据，便于在 QML 中解析和使用。

## API 接口

### C++ 接口

```cpp
Q_INVOKABLE QString getMrcHeader(const QString& filePath);
```

**参数：**
- `filePath`: MRC 文件的完整路径

**返回值：**
- 成功：返回 JSON 格式的头信息字符串
- 失败：返回包含 error 字段的 JSON 字符串

### QML 调用示例

```qml
// 在 QML 中调用
Button {
    text: "显示 MRC 头信息"
    onClicked: {
        var headerJson = backend.getMrcHeader("/path/to/file.mrc")
        var headerObj = JSON.parse(headerJson)

        // 检查是否有错误
        if (headerObj.error) {
            console.error("Error:", headerObj.error)
            return
        }

        // 访问头信息
        console.log("Dimensions:", headerObj.dimensions.nx, "x",
                    headerObj.dimensions.ny, "x", headerObj.dimensions.nz)
        console.log("Pixel size:", headerObj.pixelSize.x, headerObj.pixelSize.y,
                    headerObj.pixelSize.z)
        console.log("Data mode:", headerObj.dataMode.description)

        // 在 UI 中显示
        dimensionsText.text = headerObj.dimensions.nx + " × " +
                              headerObj.dimensions.ny + " × " +
                              headerObj.dimensions.nz
        pixelSizeText.text = headerObj.pixelSize.x.toFixed(3) + " Å"
        dataModeText.text = headerObj.dataMode.description
    }
}
```

## JSON 返回格式

### 成功响应

```json
{
  "dimensions": {
    "nx": 4096,
    "ny": 4096,
    "nz": 61
  },
  "mapMode": {
    "mapc": 1,
    "mapr": 2,
    "maps": 3
  },
  "cellDimensions": {
    "xlen": 4137.984,
    "ylen": 4137.984,
    "zlen": 61.61
  },
  "pixelSize": {
    "x": 1.0102,
    "y": 1.0102,
    "z": 1.0102
  },
  "dataMode": {
    "code": 2,
    "description": "float (32-bit)"
  },
  "density": {
    "min": -0.2834,
    "max": 0.3156,
    "mean": 0.0012
  },
  "spaceGroup": 1,
  "origin": {
    "x": 0.0,
    "y": 0.0,
    "z": 0.0
  },
  "nsymbt": 0,
  "next": 0
}
```

### 错误响应

```json
{
  "error": "Failed to open MRC file"
}
```

或

```json
{
  "error": "Failed to read MRC header"
}
```

## 字段说明

### dimensions
- `nx`: 图像列数（X 方向像素数）
- `ny`: 图像行数（Y 方向像素数）
- `nz`: 图像切片数（Z 方向切片数）

### mapMode
- `mapc`: 列轴映射 (1=X, 2=Y, 3=Z)
- `mapr`: 行轴映射 (1=X, 2=Y, 3=Z)
- `maps`: 切片轴映射 (1=X, 2=Y, 3=Z)

### cellDimensions
- `xlen`: X 方向尺寸（单位：埃）
- `ylen`: Y 方向尺寸（单位：埃）
- `zlen`: Z 方向尺寸（单位：埃）

### pixelSize
- `x`: X 方向像素大小（单位：埃）
- `y`: Y 方向像素大小（单位：埃）
- `z`: Z 方向像素大小（单位：埃）
- 计算方式：`pixelSize.x = cellDimensions.xlen / dimensions.nx`

### dataMode
- `code`: 数据模式代码
  - 0: byte (unsigned 8-bit)
  - 1: short (signed 16-bit)
  - 2: float (32-bit)
  - 3: complex short
  - 4: complex float
  - 6: unsigned short (16-bit)
  - 16: RGB (3 x 8-bit)
- `description`: 数据模式的文字描述

### density
- `min`: 最小密度值
- `max`: 最大密度值
- `mean`: 平均密度值

### 其他字段
- `spaceGroup`: 空间群编号
- `origin`: 原点坐标 (x, y, z)
- `nsymbt`: 符号表大小
- `next`: 扩展头大小

## 完整 QML 示例

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    Button {
        text: "加载 MRC 头信息"
        onClicked: loadMrcHeader()
    }

    GridLayout {
        columns: 2

        Label { text: "尺寸:" }
        Label { id: dimensionsLabel }

        Label { text: "像素大小:" }
        Label { id: pixelSizeLabel }

        Label { text: "数据类型:" }
        Label { id: dataModeLabel }

        Label { text: "密度范围:" }
        Label { id: densityLabel }
    }

    function loadMrcHeader() {
        var jsonStr = backend.getMrcHeader("/path/to/file.mrc")
        var header = JSON.parse(jsonStr)

        if (header.error) {
            console.error("加载失败:", header.error)
            return
        }

        dimensionsLabel.text = header.dimensions.nx + " × " +
                               header.dimensions.ny + " × " +
                               header.dimensions.nz

        pixelSizeLabel.text = header.pixelSize.x.toFixed(3) + " × " +
                              header.pixelSize.y.toFixed(3) + " × " +
                              header.pixelSize.z.toFixed(3) + " Å"

        dataModeLabel.text = header.dataMode.description

        densityLabel.text = header.density.min.toFixed(3) + " ~ " +
                            header.density.max.toFixed(3) +
                            " (mean: " + header.density.mean.toFixed(3) + ")"
    }
}
```

## 与其他 MRC API 配合使用

```qml
// 获取切片总数
var sliceCount = backend.getMrcSliceCount(mrcPath)

// 获取头信息
var headerJson = backend.getMrcHeader(mrcPath)
var header = JSON.parse(headerJson)

// 验证切片数是否一致
console.log("Slice count from API:", sliceCount)
console.log("Slice count from header:", header.dimensions.nz)

// 加载特定切片
var imageId = backend.loadMrcSlice(mrcPath, sliceIndex)
```

## 参考

- MRC 格式规范: `/home/xzh/autom2/src/mrcimg/mrcheader.h`
- C++ 实现: `/home/xzh/autom2/src/invoker/procinvoker.cpp:235-337`
- Mock 实现: `/home/xzh/autom2/src/qmlapp/Autom2qml/MockBackend.qml:121-162`
