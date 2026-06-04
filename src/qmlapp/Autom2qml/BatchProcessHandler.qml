// BatchProcessHandler.qml
// 批量处理管理器 - 管理多文件批量处理的执行
pragma Singleton

import QtQuick

QtObject {
    id: root

    // ==================== 状态属性 ====================
    property bool isProcessing: false           // 是否正在批量处理
    property int currentIndex: 0                // 当前处理的文件索引
    property int totalCount: 0                  // 总文件数
    property int successCount: 0                // 成功处理的文件数
    property int failedCount: 0                 // 失败的文件数

    // 文件列表：[{mrcFile, rawtltFile, basename, status}, ...]
    // status: "pending" | "processing" | "success" | "failed"
    property var fileList: []

    // ==================== 批量配置（用户一次性传入的所有选项）====================
    property var batchConfig: ({
        // 基础配置
        method: "markerauto2",              // "markerauto2" | "markerfree"
        outputDir: "",

        // MarkerAuto2 参数
        markerauto2: {
            beadDiameter: -1,               // -d 参数，-1 表示自动检测
            eraseMarkers: false,            // 是否擦除标记点
            enableSupplement: false,        // 是否启用 -s 高角度补充
            supplementAngle: 45             // -s 角度值
        },

        // Markerfree 参数
        markerfree: {
            geometry: [0, 0, 0, 0, 300, 1.0, 0],  // 7个几何参数
            saveMode: 1                     // 0=TXT, 1=XF
        },

        // 是否执行 3D 重建
        enableTiltRec: false,

        // TiltRec 参数
        tiltRec: {
            method: "SART",
            accMethod: "mpi",               // 加速方法 (mpi/cuda)
            threads: 4,
            geometry: [0, 0, 0, 100],       // 4个几何参数：offset, pitch, zOffset, thickness
            axis: "y",
            binFactor: 1,                   // 降采样因子
            iterations: 10,
            relaxation: 1.0,
            cgIterations: 5,
            threshold: 0.0
        }
    })

    // ==================== 进度计算 ====================
    readonly property int overallProgress: totalCount > 0 ?
        Math.floor(((successCount + failedCount) / totalCount) * 100) : 0

    readonly property string currentFileName:
        (currentIndex >= 0 && currentIndex < fileList.length) ?
        fileList[currentIndex].basename : ""

    readonly property int currentFileProgress:
        CommandChainManager.isChainRunning ? CommandChainManager.overallProgress : 0

    // ==================== 信号 ====================
    signal batchStarted(int totalCount)
    signal fileStarted(int index, string basename)
    signal fileCompleted(int index, string basename, bool success)
    signal fileFailed(int index, string basename, string error)
    signal batchCompleted(int successCount, int failedCount)
    signal batchStopped()

    // ==================== 核心方法 ====================

    /**
     * 开始批量处理
     * @param files - 文件列表 [{mrcFile, rawtltFile, basename}, ...]
     * @param config - 完整的处理配置对象（包含所有 autom2 选项）
     * @return bool - 是否成功启动
     */
    function startBatch(files, config) {
        if (isProcessing) {
            console.warn("BatchProcessHandler: Batch processing already in progress")
            return false
        }

        // 验证配置
        let validation = validateBatchConfig(files, config)
        if (!validation.valid) {
            console.error("BatchProcessHandler: Batch config validation failed:", validation.message)
            return false
        }

        console.log("BatchProcessHandler: Starting batch processing with", files.length, "files")
        console.log("BatchProcessHandler: Method:", config.method)
        console.log("BatchProcessHandler: Output directory:", config.outputDir)

        // 初始化状态
        fileList = files.map(f => ({
            mrcFile: f.mrcFile,
            rawtltFile: f.rawtltFile,
            basename: f.basename,
            status: "pending"
        }))

        batchConfig = config
        currentIndex = 0
        successCount = 0
        failedCount = 0
        totalCount = files.length
        isProcessing = true

        batchStarted(totalCount)

        // 开始处理第一个文件
        processNextFile()
        return true
    }

    /**
     * 停止批量处理
     */
    function stopBatch() {
        if (!isProcessing) {
            console.warn("BatchProcessHandler: No batch processing in progress")
            return
        }

        console.log("BatchProcessHandler: Stopping batch processing")

        // 停止当前命令链
        CommandChainManager.stopChain()

        // 重置状态
        isProcessing = false

        batchStopped()
    }

    /**
     * 处理下一个文件
     */
    function processNextFile() {
        if (!isProcessing) {
            console.log("BatchProcessHandler: Processing stopped, not continuing")
            return
        }

        if (currentIndex >= fileList.length) {
            // 批量处理完成
            finalizeBatch()
            return
        }

        let file = fileList[currentIndex]
        file.status = "processing"

        console.log(`BatchProcessHandler: Processing file ${currentIndex + 1}/${totalCount}: ${file.basename}`)
        fileStarted(currentIndex, file.basename)

        // 根据配置生成命令链和参数
        let chainId = determineChainId(batchConfig)
        let success = setupAppStateForFile(file, batchConfig)

        if (!success) {
            console.error("BatchProcessHandler: Failed to setup AppState for file:", file.basename)
            handleFileFailure("Failed to setup AppState")
            return
        }

        // 启动命令链
        console.log("BatchProcessHandler: Starting chain:", chainId)
        CommandChainManager.startChain(chainId, {})
    }

    /**
     * 根据配置确定使用哪个命令链
     */
    function determineChainId(config) {
        if (config.method === "markerfree") {
            return config.enableTiltRec ? "markerfree-tiltrec-workflow" : "markerfree-workflow"
        } else if (config.method === "markerauto2") {
            return config.enableTiltRec ? "markerauto2-tiltrec-workflow" : "markerauto2-workflow"
        }
        return ""
    }

    /**
     * 为单个文件设置 AppState
     * 关键：将批量配置应用到具体文件
     */
    function setupAppStateForFile(file, config) {
        try {
            // 设置基础文件路径
            AppState.initialMrcFile = file.mrcFile
            AppState.rawtltFile = file.rawtltFile
            AppState.outputDir = config.outputDir

            // 根据方法设置工作流
            AppState.selectedWorkflow = config.method

            // 根据方法设置特定参数
            if (config.method === "markerfree") {
                // 设置 Markerfree 几何参数（7个值）
                let geo = config.markerfree.geometry
                AppState.markerfreeOffset = geo[0]
                AppState.markerfreeTiltAxisAngle = geo[1]
                AppState.markerfreeZOffset = geo[2]
                AppState.markerfreeThickness = geo[3]
                AppState.markerfreeProjMatchThickness = geo[4]
                AppState.markerfreeDownsampleRatio = geo[5]
                AppState.markerfreeGpuId = geo[6]
                AppState.markerfreeSaveMode = config.markerfree.saveMode

            } else if (config.method === "markerauto2") {
                // 设置 Markerauto 参数
                AppState.markerAuto2BeadDiameter = config.markerauto2.beadDiameter
                AppState.markerAuto2EraseMarkers = config.markerauto2.eraseMarkers
                AppState.markerAuto2EnableSupplement = config.markerauto2.enableSupplement
                AppState.markerAuto2SupplementAngle = config.markerauto2.supplementAngle
            }

            // 如果启用 TiltRec，设置重建参数
            if (config.enableTiltRec) {
                AppState.tiltRecMethod = config.tiltRec.method
                AppState.tiltRecAccMethod = config.tiltRec.accMethod || "mpi"
                AppState.tiltRecThreads = config.tiltRec.threads
                AppState.tiltRecAxis = config.tiltRec.axis
                AppState.tiltRecBinFactor = config.tiltRec.binFactor || 1

                // 设置 TiltRec 几何参数（4个值）
                let geo = config.tiltRec.geometry
                AppState.geometryOffset = geo[0]
                AppState.geometryPitch = geo[1]
                AppState.geometryZOffset = geo[2]
                AppState.geometryThickness = geo[3]

                // 设置方法特定参数
                AppState.iterations = config.tiltRec.iterations
                AppState.relaxation = config.tiltRec.relaxation
                AppState.cgIterations = config.tiltRec.cgIterations
                AppState.threshold = config.tiltRec.threshold
            }

            return true

        } catch (error) {
            console.error("BatchProcessHandler: Error setting up AppState:", error)
            return false
        }
    }

    /**
     * 验证批量配置
     */
    function validateBatchConfig(files, config) {
        let errors = []

        // 验证文件列表
        if (!files || files.length === 0) {
            errors.push("文件列表为空")
        } else {
            // 验证每个文件
            for (let i = 0; i < files.length; i++) {
                let file = files[i]
                if (!file.mrcFile || !file.rawtltFile || !file.basename) {
                    errors.push(`文件 ${i + 1} 信息不完整`)
                }
            }
        }

        // 验证输出目录
        if (!config.outputDir) {
            errors.push("未指定输出目录")
        }

        // 验证处理方法
        if (!config.method || (config.method !== "markerauto2" && config.method !== "markerfree")) {
            errors.push("未指定有效的处理方法")
        }

        // 验证方法特定参数
        if (config.method === "markerfree") {
            if (!config.markerfree || !config.markerfree.geometry ||
                config.markerfree.geometry.length !== 7) {
                errors.push("Markerfree 几何参数不完整（需要7个值）")
            }
            if (config.markerfree.saveMode !== 0 && config.markerfree.saveMode !== 1) {
                errors.push("Markerfree 保存模式无效（必须是 0 或 1）")
            }
        }

        if (config.method === "markerauto2") {
            if (!config.markerauto2) {
                errors.push("Markerauto 配置缺失")
            }
        }

        // 验证 TiltRec 参数（如果启用）
        if (config.enableTiltRec) {
            if (!config.tiltRec || !config.tiltRec.method) {
                errors.push("启用了 TiltRec 但未指定重建方法")
            }
            if (config.tiltRec && config.tiltRec.threads <= 0) {
                errors.push("TiltRec 线程数必须大于 0")
            }
        }

        return {
            valid: errors.length === 0,
            message: errors.join("；"),
            errors: errors
        }
    }

    /**
     * 处理文件失败
     */
    function handleFileFailure(errorMessage) {
        if (currentIndex >= fileList.length) return

        let file = fileList[currentIndex]
        file.status = "failed"
        failedCount++

        console.error(`BatchProcessHandler: File ${file.basename} failed:`, errorMessage)
        fileFailed(currentIndex, file.basename, errorMessage)

        // 继续处理下一个文件
        currentIndex++
        processNextFile()
    }

    /**
     * 完成批量处理
     */
    function finalizeBatch() {
        isProcessing = false

        console.log(`BatchProcessHandler: Batch processing completed`)
        console.log(`  - Total: ${totalCount}`)
        console.log(`  - Success: ${successCount}`)
        console.log(`  - Failed: ${failedCount}`)

        batchCompleted(successCount, failedCount)
    }

    /**
     * 获取文件列表（用于 UI 显示）
     */
    function getFileList() {
        return fileList
    }

    /**
     * 获取指定索引的文件信息
     */
    function getFileAt(index) {
        if (index >= 0 && index < fileList.length) {
            return fileList[index]
        }
        return null
    }

    // ==================== 初始化和信号连接 ====================
    Component.onCompleted: {
        // 连接 CommandChainManager 的信号
        CommandChainManager.chainCompleted.connect(handleChainCompleted)
        CommandChainManager.chainFailed.connect(handleChainFailed)
        CommandChainManager.chainStopped.connect(handleChainStopped)
    }

    // ==================== 信号处理函数 ====================

    function handleChainCompleted(chainId, success) {
        if (!isProcessing) return

        console.log(`BatchProcessHandler: Chain completed for file ${currentIndex + 1}/${totalCount}, success: ${success}`)

        if (currentIndex >= fileList.length) return

        let file = fileList[currentIndex]
        file.status = success ? "success" : "failed"

        if (success) {
            successCount++
            fileCompleted(currentIndex, file.basename, true)
        } else {
            failedCount++
            fileFailed(currentIndex, file.basename, "命令链执行失败")
        }

        // 处理下一个文件
        currentIndex++
        processNextFile()
    }

    function handleChainFailed(chainId, failedStep, program, exitCode, message) {
        if (!isProcessing) return

        console.error(`BatchProcessHandler: Chain failed at step ${failedStep} (${program}):`, message)

        if (currentIndex >= fileList.length) return

        let file = fileList[currentIndex]
        file.status = "failed"
        failedCount++

        let errorMsg = `步骤 ${failedStep} 失败 (${program}, 退出码: ${exitCode}): ${message}`
        fileFailed(currentIndex, file.basename, errorMsg)

        // 继续处理下一个文件（失败后继续）
        currentIndex++
        processNextFile()
    }

    function handleChainStopped(chainId) {
        if (!isProcessing) return

        console.log("BatchProcessHandler: Chain stopped by user")

        // 标记当前文件为失败
        if (currentIndex < fileList.length) {
            let file = fileList[currentIndex]
            file.status = "failed"
            failedCount++
        }

        // 停止批量处理
        isProcessing = false
        batchStopped()
    }
}
