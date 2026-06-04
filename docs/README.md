# Autom2 文档目录

## MRC API 文档

### [MRC_API_QuickStart.md](./MRC_API_QuickStart.md)
**快速开始指南** - 5分钟上手 MRC 读取 API

包含：
- 一行代码示例
- 最简示例
- 常见用法
- 快速调试

**适合：** 快速集成、查找代码片段

---

### [MRC_API_Usage.md](./MRC_API_Usage.md)
**完整使用手册** - MRC 读取 API 详细文档

包含：
- API 完整参考
- 基础到进阶示例
- 错误处理
- 性能优化
- 技术实现细节
- 在 Autom2 项目中集成

**适合：** 深入学习、解决问题、理解原理

---

## 快速导航

### 我想...

- **快速开始使用** → [MRC_API_QuickStart.md](./MRC_API_QuickStart.md)
- **查看完整示例** → [MRC_API_Usage.md](./MRC_API_Usage.md) 第2节
- **了解 API 参数** → [MRC_API_Usage.md](./MRC_API_Usage.md) 第1节
- **在现有项目中集成** → [MRC_API_Usage.md](./MRC_API_Usage.md) 第3节
- **创建可重用组件** → [MRC_API_Usage.md](./MRC_API_Usage.md) 第4节
- **调试错误** → [MRC_API_Usage.md](./MRC_API_Usage.md) 第5节

---

## 修改的文件清单

实现 MRC 读取功能修改了以下文件：

```
src/invoker/procinvoker.h         # 添加 loadMrcSlice 方法声明
src/invoker/procinvoker.cpp       # 实现 loadMrcSlice 方法
src/invoker/CMakeLists.txt        # 链接 mrcimg 和 Qt6::Gui
src/CMakeLists.txt                # 添加 mrcimg 子目录
```

**零破坏性修改** - 所有修改都是新增功能，不影响现有代码。

---

## API 概览

```qml
// 获取 MRC 文件的切片总数
int realBackend.getMrcSliceCount(QString filePath)

// 加载 MRC 文件的单张切片
QImage realBackend.loadMrcSlice(QString filePath, int sliceIndex)
```

**示例：**
```qml
// 获取切片总数
var count = realBackend.getMrcSliceCount("/path/to/file.mrc");
console.log("Total slices:", count);

// 加载第一张切片
var image = realBackend.loadMrcSlice("/path/to/file.mrc", 0);
myImageComponent.source = image;
```

---

## 支持的 MRC 格式

- ✅ MRC_MODE_BYTE (mode 0) - 8位无符号整数
- ✅ MRC_MODE_SHORT (mode 1) - 16位有符号整数
- ✅ MRC_MODE_FLOAT (mode 2) - 32位浮点数
- ❌ MRC_MODE_COMPLEX_SHORT (mode 3) - 暂不支持
- ❌ MRC_MODE_COMPLEX_FLOAT (mode 4) - 暂不支持

---

## 更新日志

### 2026-01-18 (v2)
- ✨ 新功能：添加 `getMrcSliceCount()` API 获取切片总数
- 📝 文档：更新所有示例代码
- 🎯 改进：切片导航更加安全和友好

### 2026-01-18 (v1)
- ✨ 新功能：添加 `loadMrcSlice()` API
- 📝 文档：创建完整使用文档
- 🔧 构建：更新 CMakeLists 配置

---

## 贡献者

本功能由 Claude Code 实现。

---

**最后更新**: 2026-01-18
