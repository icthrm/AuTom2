// MockBackend.qml
// 作用：一个QML单例，模拟真实C++后端的接口和行为。
pragma Singleton
import QtQuick 2.15
QtObject {
id: root
    // 1. 定义与C++ Backend完全相同的信号
    //    这样QML界面就可以用同样的方式监听它们。
    signal dataReceived(string chunk, string program)
    signal commandFinished(int exitCode, string program)

    // --- 内部状态，用于模拟数据流 ---
    property var _timer: null // 定时器引用
    property var _fakeDataChunks: [] // 存储假数据块的数组
    property int _chunkIndex: 0

    function terminateCommand(program) {
        console.log("Mock Backend: Stop command", program)
    }

    // 2. 定义与C++ Backend完全相同的可调用函数
    function runCommand(program, args) {
        console.log("Mock Backend: Running command", program, args)

        // 准备要分块发送的假数据
        _fakeDataChunks = [
            "--- MOCK DATA STREAM ---\n",
            "Executing: " + program + " " + args.join(" ") + "\n",
            "Pinging google.com [142.250.184.206] with 32 bytes of data:\n",
            "1Reply from 142.250.184.206: bytes=32 time=12ms TTL=117\n",
            "2Reply from 142.250.184.206: bytes=32 time=11ms TTL=117\n",
            "3Reply from 142.250.184.206: bytes=32 time=12ms TTL=117\n",
            "4Reply from 142.250.184.206: bytes=32 time=13ms TTL=117\n"
        ];
        _chunkIndex = 0;

        // 如果存在旧的计时器，先销毁它
        if (_timer) { _timer.destroy(); }

        // 创建一个重复触发的计时器来模拟数据流
        _timer = Qt.createQmlObject(`
            import QtQuick 2.0;
            Timer {
                interval: 300; // 每300毫秒发送一块数据
                repeat: true;
                // 每次触发时，调用父对象（即root）的sendNextChunk方法
                // onTriggered: parent.sendNextChunk();
            }`, root);

        _timer.onTriggered.connect(
            function () {
                root.sendNextChunk(program)
            }

        )

        // 启动计时器
        _timer.start();
    }

    function runCommandGlobal(program, args) {
        runCommand(program, args);
    }

    function runCommandOnDir(dir, program, args) {
        console.log("work on dir", dir);
        runCommand(program, args);
    }

    // --- MRC 文件读取 API 模拟 ---

    /**
     * 模拟获取 MRC 文件的切片总数
     * @param filePath - MRC 文件路径
     * @returns 模拟的切片总数（固定返回50层）
     */
    function getMrcSliceCount(filePath) {
        console.log("Mock Backend: getMrcSliceCount called for", filePath)

        if (!filePath || filePath === "") {
            console.warn("Mock Backend: Empty file path")
            return -1
        }

        // 模拟返回50层切片
        var mockSliceCount = 50
        console.log("Mock Backend: Returning", mockSliceCount, "slices for", filePath)
        return mockSliceCount
    }

    /**
     * 模拟加载 MRC 文件的指定切片
     * @param filePath - MRC 文件路径
     * @param sliceIndex - 切片索引（0-based）
     * @returns 模拟的占位图片（data URI）
     */
    function loadMrcSlice(filePath, sliceIndex) {
        console.log("Mock Backend: loadMrcSlice called for", filePath, "index:", sliceIndex)

        if (!filePath || filePath === "") {
            console.warn("Mock Backend: Empty file path")
            return ""
        }

        // 检查索引范围（基于模拟的50层）
        var totalSlices = 50
        if (sliceIndex < 0 || sliceIndex >= totalSlices) {
            console.warn("Mock Backend: Invalid slice index:", sliceIndex, "(max:", totalSlices - 1, ")")
            return ""
        }

        console.log("Mock Backend: Would load slice", sliceIndex, "from", filePath)
        console.log("Mock Backend: Returning placeholder image (Design Studio mode)")

        // 返回一个1x1透明像素的data URI作为占位符
        // 这样Image组件会有有效的source，但不显示任何内容
        // 真实的 C++ 后端会返回实际的 QImage
        return "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg=="
    }

    /**
     * 模拟获取 MRC 文件的头信息
     * @param filePath - MRC 文件路径
     * @returns JSON 字符串，包含 MRC 头信息
     */
    function getMrcHeader(filePath) {
        console.log("Mock Backend: getMrcHeader called for", filePath)

        if (!filePath || filePath === "") {
            console.warn("Mock Backend: Empty file path")
            return JSON.stringify({ error: "Empty file path" })
        }

        // 模拟返回 MRC 头信息 - 匹配新的 JSON 结构
        var mockHeader = {
            dimensions: {
                nx: 4096,
                ny: 4096,
                nz: 61
            },
            mapMode: {
                mapc: 1,
                mapr: 2,
                maps: 3
            },
            cellDimensions: {
                xlen: 4137.984,
                ylen: 4137.984,
                zlen: 61.61
            },
            pixelSize: {
                x: 1.0102,
                y: 1.0102,
                z: 1.0102
            },
            dataMode: {
                code: 2,
                description: "float (32-bit)"
            },
            density: {
                min: -0.2834,
                max: 0.3156,
                mean: 0.0012
            },
            spaceGroup: 1,
            origin: {
                x: 0.0,
                y: 0.0,
                z: 0.0
            },
            nsymbt: 0,
            next: 0
        }

        console.log("Mock Backend: Returning mock MRC header")
        return JSON.stringify(mockHeader)
    }

    // --- SVG 文件 API 模拟 ---

    /**
     * 模拟获取 SVG 文件信息
     * @param filePath - SVG 文件路径
     * @returns JSON 字符串，包含 SVG 信息
     */
    function getSvgInfo(filePath) {
        console.log("Mock Backend: getSvgInfo called for", filePath)

        if (!filePath || filePath === "") {
            console.warn("Mock Backend: Empty file path")
            return JSON.stringify({ isValid: false, error: "Empty file path" })
        }

        // 模拟返回 SVG 信息
        var mockSvgInfo = {
            defaultSize: {
                width: 800,
                height: 600
            },
            viewBox: {
                x: 0,
                y: 0,
                width: 800,
                height: 600
            },
            aspectRatio: 800.0 / 600.0,
            fileSize: 12345,
            isValid: true
        }

        console.log("Mock Backend: Returning mock SVG info")
        return JSON.stringify(mockSvgInfo)
    }

    /**
     * 模拟加载 SVG 文件为图像
     * @param filePath - SVG 文件路径
     * @param width - 目标宽度 (0 表示使用默认尺寸)
     * @param height - 目标高度 (0 表示使用默认尺寸)
     * @returns 缓存键，用于 image:// provider
     */
    function loadSvg(filePath, width, height) {
        console.log("Mock Backend: loadSvg called for", filePath, "size:", width, "x", height)

        if (!filePath || filePath === "") {
            console.warn("Mock Backend: Empty file path")
            return ""
        }

        // 模拟返回缓存键（实际后端会返回唯一标识符）
        var mockCacheKey = "mock_svg_" + Date.now() + "_" + Math.random().toString(36).substr(2, 9)
        console.log("Mock Backend: Returning mock SVG cache key:", mockCacheKey)
        return mockCacheKey
    }

    // --- 批量处理文件扫描 API 模拟 ---

    /**
     * 模拟多文件选择对话框（替代 C++ QFileDialog::getOpenFileNames）
     * @param title - 对话框标题（忽略）
     * @param filter - 文件过滤器（忽略）
     * @returns 固定的本地路径字符串列表
     */
    function selectMultipleFiles(title, filter) {
        console.log("Mock Backend: selectMultipleFiles called, title:", title)
        return [
            "/mock/data/sample01.mrc",
            "/mock/data/sample01.rawtlt",
            "/mock/data/sample02.st",
            "/mock/data/sample02.rawtlt",
            "/mock/data/dataset_A.mrc",
            "/mock/data/dataset_A.rawtlt"
        ]
    }

    /**
     * 模拟扫描文件夹中的 MRC 和 rawtlt 文件配对
     * @param folderPath - 文件夹路径
     * @returns 文件配对数组 [{mrcFile, rawtltFile, basename}, ...]
     */
    function scanMrcRawtltPairs(folderPath) {
        console.log("Mock Backend: scanMrcRawtltPairs called for", folderPath)

        if (!folderPath || folderPath === "") {
            console.warn("Mock Backend: Empty folder path")
            return []
        }

        // 模拟返回固定的文件配对
        var mockPairs = [
            {
                mrcFile: folderPath + "/sample01.mrc",
                rawtltFile: folderPath + "/sample01.rawtlt",
                basename: "sample01"
            },
            {
                mrcFile: folderPath + "/sample02.st",
                rawtltFile: folderPath + "/sample02.rawtlt",
                basename: "sample02"
            },
            {
                mrcFile: folderPath + "/dataset_A.mrc",
                rawtltFile: folderPath + "/dataset_A.rawtlt",
                basename: "dataset_A"
            },
            {
                mrcFile: folderPath + "/BBa.st",
                rawtltFile: folderPath + "/BBa.rawtlt",
                basename: "BBa"
            }
        ]

        console.log("Mock Backend: Returning", mockPairs.length, "file pairs")
        return mockPairs
    }

    // 3. 内部辅助函数，由计时器调用
    function sendNextChunk(program) {
        if (_chunkIndex < _fakeDataChunks.length) {
            // 如果还有数据块要发送，就发射 dataReceived 信号
            root.dataReceived(_fakeDataChunks[_chunkIndex], program);
            _chunkIndex++;
        } else {
            // 如果所有数据块都已发送完毕
            _timer.stop();      // 停止计时器
            _timer.destroy();   // 销毁计时器对象以释放资源
            _timer = null;
            // 发射任务完成信号，退出码为0（表示成功）
            root.commandFinished(0, program);
            console.log("Mock Backend: Command finished.");
        }
    }
}

