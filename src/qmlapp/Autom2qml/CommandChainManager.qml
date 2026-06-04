// CommandChainManager.qml
// 命令链管理器 - 管理多程序调用链的执行
pragma Singleton

import QtQuick

QtObject {
    id: root

    // ==================== 后端引用 ====================
    property var backend: null

    // ==================== 统一状态 ====================
    property string activeChainId: ""           // 当前活动的调用链 ID
    property int currentStepIndex: -1           // 当前步骤索引 (0-based)
    property int totalSteps: 0                  // 总步骤数
    property bool isChainRunning: false         // 调用链是否在运行
    property string currentProgram: ""          // 当前执行的程序名
    property int overallProgress: 0             // 整体进度 (0-100)

    // ==================== 调用链定义 ====================
    property var chainQueue: []                 // 当前调用链队列
    property var chainDefinitions: ({})         // 预定义的调用链模板

    // ==================== 信号 ====================
    signal chainStarted(string chainId)
    signal stepStarted(string chainId, int stepIndex, string program)
    signal stepCompleted(string chainId, int stepIndex, string program, int exitCode)
    signal chainCompleted(string chainId, bool success)
    signal chainFailed(string chainId, int failedStep, string program, int exitCode, string message)
    signal chainStopped(string chainId)
    
    // GeoParGen 专用信号：解析到几何参数时发出
    signal geoParGenResultParsed(real angle, real thickness, real zOffset)

    // ==================== 初始化 ====================

    Component.onCompleted: {
        registerAllChains()
    }

    /**
     * 注册所有预定义的调用链
     */
    function registerAllChains() {
        // ========== MarkerAuto2 工作流 ==========
        registerChain("markerauto2-workflow", function(ctx) {
            return [
                {
                    id: "markerauto2",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        let args = [
                            "-n", "2",
                            "./markerauto2",
                            "-i", AppState.initialMrcFile,
                            "-a", AppState.rawtltFile,
                            "-n", AppState.fixedTltFile,
                            "-o", AppState.fixedXfFile,
                            "-d", AppState.markerAuto2BeadDiameter.toString()
                        ]
                        if (AppState.markerAuto2EnableSupplement) {
                            args.push("-s", AppState.markerAuto2SupplementAngle.toString())
                        }
                        return args
                    }
                },
                {
                    id: "markererase",
                    program: "markererase",
                    mode: "normal",
                    condition: function() { return AppState.markerAuto2EraseMarkers },
                    args: function() {
                        return [
                            "-i", AppState.initialMrcFile,
                            "-f", AppState.fidsFile,
                            "-d", AppState.markerAuto2BeadDiameter.toString(),
                            "-o", AppState.erasedMrcFile,
                            "-a", AppState.fids2File
                        ]
                    }
                },
                {
                    id: "mrcstack",
                    program: "mrcstack",
                    mode: "normal",
                    args: function() {
                        let inputMrc = AppState.markerAuto2EraseMarkers
                                     ? AppState.erasedMrcFile
                                     : AppState.initialMrcFile
                        return [
                            "-i", inputMrc,
                            "-o", AppState.alignedMrcFile,
                            "-x", AppState.fixedXfFile
                        ]
                    }
                }
            ]
        })

        // ========== Markerfree 工作流 ==========
        registerChain("markerfree-workflow", function(ctx) {
            return [
                {
                    id: "markerfree",
                    program: "Markerfree",
                    mode: "normal",
                    args: function() {
                        let geometry = AppState.markerfreeOffset + ","
                                     + AppState.markerfreeTiltAxisAngle + ","
                                     + AppState.markerfreeZOffset + ","
                                     + AppState.markerfreeThickness + ","
                                     + AppState.markerfreeProjMatchThickness + ","
                                     + AppState.markerfreeDownsampleRatio + ","
                                     + AppState.markerfreeGpuId
                        return [
                            "-i", AppState.initialMrcFile,
                            "-o", AppState.baseNameOnOutput,
                            "-a", AppState.rawtltFile,
                            "-g", geometry,
                            "-s", AppState.markerfreeSaveMode.toString(),
                            "-p", AppState.markerfreeProjImageCount.toString()
                        ]
                    }
                },
                {
                    id: "mrcstack",
                    program: "mrcstack",
                    mode: "normal",
                    args: function() {
                        return [
                            "-i", AppState.initialMrcFile,
                            "-o", AppState.alignedMrcFile,
                            "-x", AppState.fixedXfFile
                        ]
                    }
                }
            ]
        })

        // ========== TiltRec 工作流 ==========
        registerChain("tiltrec-workflow", function(ctx) {
            return [
                {
                    id: "resample",
                    program: "resample",
                    mode: "normal",
                    condition: function() { return AppState.tiltRecBinFactor > 1 },
                    args: function() {
                        // 使用 tiltRecInputMrcFile（如果启用CTF则为ctfMrcFile，否则为alignedMrcFile）
                        return [
                            "-i", AppState.tiltRecInputMrcFile,
                            "-o", AppState.resampledMrcFile,
                            "-b", AppState.tiltRecBinFactor.toString()
                        ]
                    }
                },
                {
                    id: "tiltrec",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        let args = ["-n", AppState.tiltRecThreads.toString()]

                        let tiltrecName = "./TiltRec" 
                                        + (AppState.tiltRecAxis === "z" ? "Z" : "")
                                        + "-" + AppState.tiltRecAccMethod

                        args.push(tiltrecName)

                        // 确定输入文件（考虑 CTF 和降采样）
                        let inputMrc = AppState.tiltRecBinFactor > 1
                                     ? AppState.resampledMrcFile
                                     : AppState.tiltRecInputMrcFile

                        args.push("--input", inputMrc)
                        args.push("--output", AppState.reconstructionMrcFile)
                        args.push("--tiltfile", AppState.rawtltFile)

                        // 几何参数
                        let geometry = AppState.geometryOffset + "," 
                                     + AppState.geometryPitch + ","
                                     + AppState.geometryZOffset + "," 
                                     + AppState.geometryThickness
                        args.push("--geometry", geometry)

                        // 方法特定参数
                        if (AppState.tiltRecMethod === "BPT" 
                            || AppState.tiltRecMethod === "FBP"
                            || AppState.tiltRecMethod === "WBP") {
                            args.push("--method", AppState.tiltRecMethod)
                        } else if (AppState.tiltRecMethod === "SART" 
                                 || AppState.tiltRecMethod === "SIRT") {
                            args.push("--method", AppState.tiltRecMethod + ","
                                    + AppState.iterations + "," + AppState.relaxation)
                        } else if (AppState.tiltRecMethod === "ADMM") {
                            args.push("--method", AppState.tiltRecMethod + ","
                                    + AppState.iterations + "," + AppState.cgIterations + ","
                                    + AppState.relaxation + "," + AppState.threshold)
                        }

                        return args
                    }
                }
            ]
        })

        // ========== GeoParGen 工作流（获取几何参数） ==========
        registerChain("geopargen-workflow", function(ctx) {
            return [
                {
                    id: "tiltrec-temp",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        // 固定参数：2线程, cuda, SIRT方法
                        let args = ["-n", "2"]
                        
                        let tiltrecName = "./TiltRec" 
                                        + (AppState.tiltRecAxis === "z" ? "Z" : "")
                                        + "-cuda"

                        args.push(tiltrecName)

                        // 使用对齐后的 MRC 文件（或 CTF 处理后的）
                        args.push("--input", AppState.tiltRecInputMrcFile)
                        args.push("--output", AppState.geoParGenTempMrcFile)
                        args.push("--tiltfile", AppState.rawtltFile)

                        // 使用默认几何参数（0,0,0,300）
                        args.push("--geometry", "0,0,0,300")
                        
                        // 使用 SIRT 方法，10次迭代，1.0松弛因子
                        args.push("--method", "SIRT,10,1.0")

                        return args
                    }
                },
                {
                    id: "geopargen",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        return [
                            "-n", AppState.tiltRecThreads.toString(), "./GeoParGen", AppState.geoParGenTempMrcFile
                        ]
                    }
                }
            ]
        })

        registerChain("geom-workflow", function(ctx) {
            return [{
                    id: "geom",
                    program: "FindOffset",
                    mode: "normal",
                    args: function() {
                        return [
                            "-i", AppState.initialMrcFile, "-a", AppState.rawtltFile, "-g", "0"
                        ]
                    }
                }
            ]
        })

        // ========== CTF 工作流（CTF 校正） ==========
        registerChain("ctf-workflow", function(ctx) {
            return [
                {
                    id: "stack_to_list",
                    program: "stack_to_list",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return [
                            AppState.alignedMrcFile,
                            AppState.ctfTltFile,
                            "0",
                            AppState.ctfListFile
                        ]
                    }
                },
                {
                    id: "ctfmeasure",
                    program: "CTFMeasure",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return [
                            "--input_micrograph_list", AppState.ctfListFile,
                            "--pixel_size", AppState.ctfPixelSize.toString(),
                            "--cs", AppState.ctfCs.toString(),
                            "--voltage", AppState.ctfVoltage.toString(),
                            "--w", AppState.ctfW.toString()
                        ]
                    }
                },
                {
                    id: "ctf_deconvolve",
                    program: "ctf_deconvolve",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return [
                            AppState.ctfTxtFile,
                            AppState.ctfMrcFile,
                            AppState.ctfPixelSize.toString(),
                            AppState.ctfCs.toString(),
                            AppState.ctfVoltage.toString(),
                            AppState.ctfW.toString()
                        ]
                    }
                }
            ]
        })

        // ========== Trajectory Trace 工作流（轨迹追踪可视化） ==========
        registerChain("trajtrace-workflow", function(ctx) {
            return [
                {
                    id: "traj",
                    program: "traj",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return [
                            "-i", AppState.initialMrcFile,
                            "-a", AppState.rawtltFile,
                            "-x", AppState.fixedXfFile
                        ]
                    }
                },
                {
                    id: "trajplot-init-xz",
                    program: "trajplot",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return ["-i", AppState.initFidFile, "-m", "xz"]
                    }
                },
                {
                    id: "trajplot-init-yz",
                    program: "trajplot",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return ["-i", AppState.initFidFile, "-m", "yz"]
                    }
                },
                {
                    id: "trajplot-init-xy",
                    program: "trajplot",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return ["-i", AppState.initFidFile, "-m", "xy"]
                    }
                },
                {
                    id: "trajplot-fin-xz",
                    program: "trajplot",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return ["-i", AppState.finFidFile, "-m", "xz"]
                    }
                },
                {
                    id: "trajplot-fin-yz",
                    program: "trajplot",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return ["-i", AppState.finFidFile, "-m", "yz"]
                    }
                },
                {
                    id: "trajplot-fin-xy",
                    program: "trajplot",
                    mode: "workdir",
                    workdir: function() { return AppState.outputDir },
                    args: function() {
                        return ["-i", AppState.finFidFile, "-m", "xy"]
                    }
                }
            ]
        })

        // ========== MarkerAuto2 + TiltRec 组合工作流 ==========
        registerChain("markerauto2-tiltrec-workflow", function(ctx) {
            return [
                {
                    id: "markerauto2",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        let args = [
                            "-n", "2",
                            "./markerauto2",
                            "-i", AppState.initialMrcFile,
                            "-a", AppState.rawtltFile,
                            "-n", AppState.fixedTltFile,
                            "-o", AppState.fixedXfFile,
                            "-d", AppState.markerAuto2BeadDiameter.toString()
                        ]
                        if (AppState.markerAuto2EnableSupplement) {
                            args.push("-s", AppState.markerAuto2SupplementAngle.toString())
                        }
                        return args
                    }
                },
                {
                    id: "markererase",
                    program: "markererase",
                    mode: "normal",
                    condition: function() { return AppState.markerAuto2EraseMarkers },
                    args: function() {
                        return [
                            "-i", AppState.initialMrcFile,
                            "-f", AppState.fidsFile,
                            "-d", AppState.markerAuto2BeadDiameter.toString(),
                            "-o", AppState.erasedMrcFile,
                            "-a", AppState.fids2File
                        ]
                    }
                },
                {
                    id: "mrcstack",
                    program: "mrcstack",
                    mode: "normal",
                    args: function() {
                        let inputMrc = AppState.markerAuto2EraseMarkers
                                     ? AppState.erasedMrcFile
                                     : AppState.initialMrcFile
                        return [
                            "-i", inputMrc,
                            "-o", AppState.alignedMrcFile,
                            "-x", AppState.fixedXfFile
                        ]
                    }
                },
                {
                    id: "resample",
                    program: "resample",
                    mode: "normal",
                    condition: function() { return AppState.tiltRecBinFactor > 1 },
                    args: function() {
                        return [
                            "-i", AppState.tiltRecInputMrcFile,
                            "-o", AppState.resampledMrcFile,
                            "-b", AppState.tiltRecBinFactor.toString()
                        ]
                    }
                },
                {
                    id: "tiltrec",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        let args = ["-n", AppState.tiltRecThreads.toString()]

                        let tiltrecName = "./TiltRec"
                                        + (AppState.tiltRecAxis === "z" ? "Z" : "")
                                        + "-" + AppState.tiltRecAccMethod

                        args.push(tiltrecName)

                        let inputMrc = AppState.tiltRecBinFactor > 1
                                     ? AppState.resampledMrcFile
                                     : AppState.tiltRecInputMrcFile

                        args.push("--input", inputMrc)
                        args.push("--output", AppState.reconstructionMrcFile)
                        args.push("--tiltfile", AppState.fixedTltFile)

                        let geometry = AppState.geometryOffset + ","
                                     + AppState.geometryPitch + ","
                                     + AppState.geometryZOffset + ","
                                     + AppState.geometryThickness
                        args.push("--geometry", geometry)

                        if (AppState.tiltRecMethod === "BPT"
                            || AppState.tiltRecMethod === "FBP"
                            || AppState.tiltRecMethod === "WBP") {
                            args.push("--method", AppState.tiltRecMethod)
                        } else if (AppState.tiltRecMethod === "SART"
                                 || AppState.tiltRecMethod === "SIRT") {
                            args.push("--method", AppState.tiltRecMethod + ","
                                    + AppState.iterations + "," + AppState.relaxation)
                        } else if (AppState.tiltRecMethod === "ADMM") {
                            args.push("--method", AppState.tiltRecMethod + ","
                                    + AppState.iterations + "," + AppState.cgIterations + ","
                                    + AppState.threshold)
                        }

                        return args
                    }
                }
            ]
        })

        // ========== Markerfree + TiltRec 组合工作流 ==========
        registerChain("markerfree-tiltrec-workflow", function(ctx) {
            return [
                {
                    id: "markerfree",
                    program: "Markerfree",
                    mode: "normal",
                    args: function() {
                        let geometry = AppState.markerfreeOffset + ","
                                     + AppState.markerfreeTiltAxisAngle + ","
                                     + AppState.markerfreeZOffset + ","
                                     + AppState.markerfreeThickness + ","
                                     + AppState.markerfreeProjMatchThickness + ","
                                     + AppState.markerfreeDownsampleRatio + ","
                                     + AppState.markerfreeGpuId
                        return [
                            "-i", AppState.initialMrcFile,
                            "-o", AppState.baseNameOnOutput,
                            "-a", AppState.rawtltFile,
                            "-g", geometry,
                            "-s", AppState.markerfreeSaveMode.toString(),
                            "-p", AppState.markerfreeProjImageCount.toString()
                        ]
                    }
                },
                {
                    id: "mrcstack",
                    program: "mrcstack",
                    mode: "normal",
                    args: function() {
                        return [
                            "-i", AppState.initialMrcFile,
                            "-o", AppState.alignedMrcFile,
                            "-x", AppState.fixedXfFile
                        ]
                    }
                },
                {
                    id: "resample",
                    program: "resample",
                    mode: "normal",
                    condition: function() { return AppState.tiltRecBinFactor > 1 },
                    args: function() {
                        return [
                            "-i", AppState.tiltRecInputMrcFile,
                            "-o", AppState.resampledMrcFile,
                            "-b", AppState.tiltRecBinFactor.toString()
                        ]
                    }
                },
                {
                    id: "tiltrec",
                    program: "mpirun",
                    mode: "global",
                    args: function() {
                        let args = ["-n", AppState.tiltRecThreads.toString()]

                        let tiltrecName = "./TiltRec"
                                        + (AppState.tiltRecAxis === "z" ? "Z" : "")
                                        + "-" + AppState.tiltRecAccMethod

                        args.push(tiltrecName)

                        let inputMrc = AppState.tiltRecBinFactor > 1
                                     ? AppState.resampledMrcFile
                                     : AppState.tiltRecInputMrcFile

                        args.push("--input", inputMrc)
                        args.push("--output", AppState.reconstructionMrcFile)
                        args.push("--tiltfile", AppState.rawtltFile)

                        let geometry = AppState.geometryOffset + ","
                                     + AppState.geometryPitch + ","
                                     + AppState.geometryZOffset + ","
                                     + AppState.geometryThickness
                        args.push("--geometry", geometry)

                        if (AppState.tiltRecMethod === "BPT"
                            || AppState.tiltRecMethod === "FBP"
                            || AppState.tiltRecMethod === "WBP") {
                            args.push("--method", AppState.tiltRecMethod)
                        } else if (AppState.tiltRecMethod === "SART"
                                 || AppState.tiltRecMethod === "SIRT") {
                            args.push("--method", AppState.tiltRecMethod + ","
                                    + AppState.iterations + "," + AppState.relaxation)
                        } else if (AppState.tiltRecMethod === "ADMM") {
                            args.push("--method", AppState.tiltRecMethod + ","
                                    + AppState.iterations + "," + AppState.cgIterations + ","
                                    + AppState.threshold)
                        }

                        return args
                    }
                }
            ]
        })

        console.log("CommandChainManager: All chains registered")
    }

    // ==================== 公共方法 ====================

    /**
     * 注册调用链模板
     * @param chainId - 调用链唯一标识符
     * @param stepsBuilder - 返回步骤数组的函数
     */
    function registerChain(chainId, stepsBuilder) {
        chainDefinitions[chainId] = stepsBuilder
        console.log("CommandChainManager: Registered chain:", chainId)
    }

    /**
     * 启动调用链
     * @param chainId - 调用链标识符
     * @param context - 可选的上下文参数
     * @returns 是否成功启动
     */
    function startChain(chainId, context) {
        if (isChainRunning) {
            console.warn("CommandChainManager: Another chain is running:", activeChainId)
            return false
        }

        let stepsBuilder = chainDefinitions[chainId]
        if (!stepsBuilder) {
            console.error("CommandChainManager: Unknown chain:", chainId)
            return false
        }

        if (!backend) {
            console.error("CommandChainManager: Backend not initialized")
            return false
        }

        // 构建步骤队列
        chainQueue = stepsBuilder(context || {})
        activeChainId = chainId
        currentStepIndex = -1
        totalSteps = chainQueue.length
        isChainRunning = true
        overallProgress = 0

        // 重置 AppState 进度
        AppState.resetProgress()
        
        // 如果是 geopargen-workflow，重置解析状态
        if (chainId === "geopargen-workflow") {
            resetGeoParGenParsing()
        }

        console.log("CommandChainManager: Starting chain:", chainId, "with", totalSteps, "steps")
        chainStarted(chainId)

        // 执行第一步
        executeNextStep()
        return true
    }

    /**
     * 停止当前调用链
     */
    function stopChain() {
        if (!isChainRunning) {
            console.warn("CommandChainManager: No chain is running")
            return
        }

        console.log("CommandChainManager: Stopping chain:", activeChainId)

        if (currentProgram && backend) {
            backend.terminateCommand(currentProgram)
        }

        let stoppedChainId = activeChainId
        resetState()
        chainStopped(stoppedChainId)
    }

    /**
     * 执行下一步
     */
    function executeNextStep() {
        currentStepIndex++

        // 跳过条件不满足的步骤
        while (currentStepIndex < chainQueue.length) {
            let step = chainQueue[currentStepIndex]

            // 检查条件（可选）
            if (step.condition && !step.condition()) {
                console.log("CommandChainManager: Skipping step", currentStepIndex, 
                          "(", step.id, ") - condition not met")
                currentStepIndex++
                continue
            }

            break
        }

        // 检查是否全部完成
        if (currentStepIndex >= chainQueue.length) {
            finishChain(true)
            return
        }

        let step = chainQueue[currentStepIndex]
        currentProgram = step.program

        // 构建参数
        let args = typeof step.args === "function" ? step.args() : step.args

        console.log("CommandChainManager: Executing step", currentStepIndex, 
                  "(", step.id, "):", step.program, args.join(" "))

        stepStarted(activeChainId, currentStepIndex, step.program)

        // 根据模式执行命令
        let mode = step.mode || "normal"
        if (mode === "workdir") {
            let workdir = typeof step.workdir === "function" ? step.workdir() : step.workdir
            backend.runCommandOnDir(workdir, step.program, args)
        } else if (mode === "global") {
            backend.runCommandGlobal(step.program, args)
        } else {
            backend.runCommand(step.program, args)
        }
    }

    /**
     * 完成调用链
     * @param success - 是否成功完成
     * @param exitCode - 失败时的退出码
     */
    function finishChain(success, exitCode) {
        let finishedChainId = activeChainId
        let failedStep = currentStepIndex
        let failedProgram = currentProgram

        if (success) {
            overallProgress = 100
            AppState.updateProgress(100)
            AppState.setProgressColor("green")
            console.log("CommandChainManager: Chain completed successfully:", finishedChainId)
            resetState()
            chainCompleted(finishedChainId, true)
        } else {
            AppState.setProgressColor("red")
            console.error("CommandChainManager: Chain failed at step", failedStep, 
                        "program:", failedProgram, "exitCode:", exitCode)
            resetState()
            chainFailed(finishedChainId, failedStep, failedProgram, exitCode, 
                       "Step failed with exit code " + exitCode)
        }
    }

    /**
     * 重置内部状态
     */
    function resetState() {
        isChainRunning = false
        currentProgram = ""
        activeChainId = ""
        currentStepIndex = -1
        totalSteps = 0
        chainQueue = []
    }

    // ==================== 后端信号监听 ====================

    property var _backendConnection: Connections {
        target: backend

        function onCommandFinished(exitCode, program) {
            // 检查是否是当前调用链的程序
            if (!isChainRunning || program !== currentProgram) {
                return
            }

            console.log("CommandChainManager: Step completed - program:", program, 
                      "exitCode:", exitCode)

            stepCompleted(activeChainId, currentStepIndex, program, exitCode)

            if (exitCode !== 0) {
                finishChain(false, exitCode)
                return
            }

            // 更新整体进度
            let completedSteps = currentStepIndex + 1
            let actualSteps = countActualSteps()
            overallProgress = Math.round(completedSteps / actualSteps * 100)

            // 执行下一步
            executeNextStep()
        }
        
        // 监听数据输出，解析 GeoParGen 结果
        function onDataReceived(chunk, program) {
            if (activeChainId !== "geom-workflow" || program !== "FindOffset") {
                return
            }
            
            // 解析 GeoParGen 输出
            // 格式: "average angle : X", "average thick : Y", "average z : Z"
            parseGeomOutput(chunk)
        }
    }
    
    // ==================== GeoParGen 输出解析 ====================
    
    // 临时存储解析到的值
    property real _parsedAngle: 0
    property real _parsedThickness: 0
    property real _parsedZOffset: 0
    property bool _angleFound: false
    property bool _thicknessFound: false
    property bool _zOffsetFound: false
    
    function parseGeoParGenOutput(chunk) {
        // 正则表达式匹配
        const angleRegex = /average angle\s*:\s*([-\d.]+)/i
        const thickRegex = /average thick\s*:\s*([-\d.]+)/i
        const zRegex = /average z\s*:\s*([-\d.]+)/i
        
        let angleMatch = angleRegex.exec(chunk)
        if (angleMatch) {
            _parsedAngle = parseFloat(angleMatch[1])
            _angleFound = true
            console.log("GeoParGen: Parsed angle =", _parsedAngle)
        }
        
        let thickMatch = thickRegex.exec(chunk)
        if (thickMatch) {
            _parsedThickness = parseFloat(thickMatch[1])
            _thicknessFound = true
            console.log("GeoParGen: Parsed thickness =", _parsedThickness)
        }
        
        let zMatch = zRegex.exec(chunk)
        if (zMatch) {
            _parsedZOffset = parseFloat(zMatch[1])
            _zOffsetFound = true
            console.log("GeoParGen: Parsed zOffset =", _parsedZOffset)
        }
        
        // 如果三个值都解析到了，发出信号
        if (_angleFound && _thicknessFound && _zOffsetFound) {
            console.log("GeoParGen: All parameters parsed, emitting signal")
            geoParGenResultParsed(_parsedAngle, _parsedThickness, _parsedZOffset)
            // 重置标志
            _angleFound = false
            _thicknessFound = false
            _zOffsetFound = false
        }
    }

    function parseGeomOutput(chunk) {
        // 正则表达式匹配
        const angleRegex = /angleOffset\s*=\s*([-\d.]+)/i

        let angleMatch = angleRegex.exec(chunk)
        if (angleMatch) {
            _parsedAngle = parseFloat(angleMatch[1])
            _angleFound = true
            console.log("GeoParGen: Parsed angle =", _parsedAngle)
        }

        // 如果值都解析到了，发出信号
        if (_angleFound) {
            console.log("Geom: All parameters parsed, emitting signal")
            geoParGenResultParsed(_parsedAngle, 0, 0)
            // 重置标志
            _angleFound = false
            _thicknessFound = false
            _zOffsetFound = false
        }
    }
    
    // 重置 GeoParGen 解析状态
    function resetGeoParGenParsing() {
        _parsedAngle = 0
        _parsedThickness = 0
        _parsedZOffset = 0
        _angleFound = false
        _thicknessFound = false
        _zOffsetFound = false
    }

    /**
     * 计算实际需要执行的步骤数（排除条件不满足的步骤）
     */
    function countActualSteps() {
        let count = 0
        for (let i = 0; i < chainQueue.length; i++) {
            let step = chainQueue[i]
            if (!step.condition || step.condition()) {
                count++
            }
        }
        return Math.max(count, 1)
    }

    // ==================== 兼容性方法 ====================

    /**
     * 检查是否有进程在运行（兼容旧代码）
     */
    readonly property bool isProcessRunning: isChainRunning

    /**
     * 获取当前运行的程序名（兼容旧代码）
     */
    readonly property string runningProgram: currentProgram
}
