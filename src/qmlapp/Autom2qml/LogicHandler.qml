// LogicHandler.qml
// 注意：链式调用逻辑已迁移至 CommandChainManager.qml
// 本文件保留：进度解析、工具函数、向后兼容方法
pragma Singleton

import QtQuick

Item {
    id: root

    property var backend: null

    // ==================== 常量 ====================
    readonly property int totalBundleIterations: 50

    // ==================== 兼容性属性（推荐使用 CommandChainManager） ====================
    // 这些属性保留是为了向后兼容，新代码应使用 CommandChainManager.isChainRunning
    readonly property bool isProcessRunning: CommandChainManager.isChainRunning
    readonly property string runningProgram: CommandChainManager.currentProgram

    // ==================== 信号 ====================
    signal clearLog()
    signal progressUpdated(color color)

    // ==================== 工具方法 ====================

    /**
     * 停止当前进程
     * @param program - 程序名（可选，用于兼容旧代码）
     */
    function stopProcess(program) {
        if (CommandChainManager.isChainRunning) {
            CommandChainManager.stopChain()
        } else if (backend && program) {
            backend.terminateCommand(program)
        }
    }

    /**
     * 查看 MRC 文件
     * @param mrcpath - MRC 文件路径
     */
    function viewMrcFile(mrcpath) {
        if (backend) {
            backend.runCommand("Autom3D", [mrcpath])
        }
    }

    // ==================== 进度解析 ====================

    /**
     * 解析命令输出，更新进度
     * @param chunk - 命令输出片段
     */
    function parseProgress(chunk) {
        // --- 正则表达式定义 ---
        const totalFramesRegex = /(\d+)\s+frames\(0 fixed\)/g
        const detectorRegex = /Processing MRC\[(\d+)\]\[Finished\]/g
        const bundleIterRegex = /\*{5,}(\d+)\*{5,}/g

        // --- 解析总帧数 ---
        if (AppState.totalFrames === -1) {
            let framesMatch = totalFramesRegex.exec(chunk)
            if (framesMatch) {
                AppState.setTotalFrames(parseInt(framesMatch[1], 10))
            }
        }

        // --- 解析进度 ---
        let match
        let lastFoundIndex = -1

        // 检查 Bundle 阶段（50%-99%）
        while ((match = bundleIterRegex.exec(chunk)) !== null) {
            lastFoundIndex = parseInt(match[1], 10)
        }
        if (lastFoundIndex !== -1) {
            let stageProgress = (lastFoundIndex + 1) / root.totalBundleIterations
            let progress = Math.round(50 + stageProgress * 49)
            AppState.updateProgress(progress)
            return
        }

        // 重置 lastFoundIndex
        lastFoundIndex = -1

        // 检查 Detector 阶段（0%-50%）
        if (AppState.totalFrames > 0) {
            while ((match = detectorRegex.exec(chunk)) !== null) {
                lastFoundIndex = parseInt(match[1], 10)
            }
            if (lastFoundIndex !== -1) {
                let stageProgress = (lastFoundIndex + 1) / AppState.totalFrames
                let progress = Math.round(stageProgress * 50)
                AppState.updateProgress(progress)
                return
            }
        }
    }

    // ==================== 向后兼容方法 ====================
    // 注意：以下方法已废弃，保留是为了兼容旧代码
    // 新代码应使用 CommandChainManager.startChain()

    /**
     * @deprecated 使用 CommandChainManager.startChain("markerauto2-workflow") 替代
     */
    function runMarkerAuto2(backend) {
        console.warn("LogicHandler.runMarkerAuto2() is deprecated, use CommandChainManager.startChain('markerauto2-workflow')")
        
        // 验证输入
        let validation = AppState.validateInputFiles()
        if (!validation.valid) {
            console.error("Input validation failed:", validation.message)
            return false
        }
        
        return CommandChainManager.startChain("markerauto2-workflow", {})
    }

    /**
     * @deprecated 使用 CommandChainManager.startChain("markerfree-workflow") 替代
     */
    function runMarkerfree(backend) {
        console.warn("LogicHandler.runMarkerfree() is deprecated, use CommandChainManager.startChain('markerfree-workflow')")
        
        // 验证配置
        let validation = AppState.validateMarkerfreeConfig()
        if (!validation.valid) {
            console.error("Markerfree validation failed:", validation.message)
            return false
        }
        
        return CommandChainManager.startChain("markerfree-workflow", {})
    }

    /**
     * @deprecated 使用 CommandChainManager.startChain("tiltrec-workflow") 替代
     */
    function runTiltRec(backend) {
        console.warn("LogicHandler.runTiltRec() is deprecated, use CommandChainManager.startChain('tiltrec-workflow')")
        
        // 验证配置
        let validation = AppState.validateTiltRecConfig()
        if (!validation.valid) {
            console.error("TiltRec validation failed:", validation.message)
            return false
        }
        
        return CommandChainManager.startChain("tiltrec-workflow", {})
    }

    /**
     * @deprecated 不再需要单独调用
     */
    function runMrcStack(backend, useErasedMrc) {
        console.warn("LogicHandler.runMrcStack() is deprecated - mrcstack is now part of workflow chains")
        return false
    }

    /**
     * @deprecated 不再需要单独调用
     */
    function runMarkerErase(backend) {
        console.warn("LogicHandler.runMarkerErase() is deprecated - markererase is now part of markerauto2-workflow")
        return false
    }

    /**
     * @deprecated 不再需要单独调用
     */
    function runTiltRecInternal(backend, inputMrcFile) {
        console.warn("LogicHandler.runTiltRecInternal() is deprecated - use CommandChainManager.startChain('tiltrec-workflow')")
        return false
    }

    /**
     * 重置进度状态
     */
    function reset() {
        AppState.resetProgress()
    }
}
