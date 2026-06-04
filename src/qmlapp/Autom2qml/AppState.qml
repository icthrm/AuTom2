pragma Singleton

import QtQuick

/**
 * AppState - 应用全局状态管理单例
 *
 * 职责：
 * - 存储所有全局状态（文件路径、进度、配置等）
 * - 提供状态修改方法
 * - 发出状态变化信号
 *
 * 设计原则：
 * - 单一数据源 (Single Source of Truth)
 * - 不包含业务逻辑，只存储状态
 * - 所有状态修改通过方法进行（便于调试和追踪）
 */
QtObject {
    id: root

    // ==================== 文件路径状态 ====================

    // 用户选择的输入文件
    property string initialMrcFile: ""
    property string rawtltFile: ""
    property string outputDir: ""

    // ==================== MRC 头信息状态 ====================
    
    // MRC 头信息对象（由 backend.getMrcHeader 解析后存储）
    property var mrcHeader: null
    
    // 便捷只读属性 - 匹配新的 JSON 结构
    readonly property int mrcNx: mrcHeader && mrcHeader.dimensions ? mrcHeader.dimensions.nx : 0
    readonly property int mrcNy: mrcHeader && mrcHeader.dimensions ? mrcHeader.dimensions.ny : 0
    readonly property int mrcNz: mrcHeader && mrcHeader.dimensions ? mrcHeader.dimensions.nz : 0
    readonly property real mrcPixelSize: mrcHeader && mrcHeader.pixelSize ? mrcHeader.pixelSize.x : 0.0
    readonly property string mrcModeStr: mrcHeader && mrcHeader.dataMode ? mrcHeader.dataMode.description : ""
    readonly property real mrcAmin: mrcHeader && mrcHeader.density ? mrcHeader.density.min : 0.0
    readonly property real mrcAmax: mrcHeader && mrcHeader.density ? mrcHeader.density.max : 0.0
    readonly property real mrcAmean: mrcHeader && mrcHeader.density ? mrcHeader.density.mean : 0.0

    // 自动生成的文件名
    readonly property string baseName: extractBaseName(initialMrcFile)
    readonly property string baseNameOnOutput: outputDir ? outputDir + '/' + baseName + '.mrc' : ""
    readonly property string fixedTltFile: outputDir ? outputDir + '/' + baseName + '.tlt' : ""
    readonly property string fixedXfFile: outputDir ? outputDir + '/' + baseName + '.xf' : ""
    readonly property string fidsFile: outputDir ? outputDir + '/fids.txt' : ""
    readonly property string fids2File: outputDir ? outputDir + '/addfids.txt' : ""
    readonly property string erasedMrcFile: outputDir ? outputDir + '/' + baseName + '_erased.mrc' : ""
    readonly property string alignedMrcFile: outputDir ? outputDir + '/' + baseName + '_aligned.mrc' : ""
    readonly property string resampledMrcFile: outputDir ? outputDir + '/' + baseName + '_aligned_bin' + tiltRecBinFactor + '.mrc' : ""
    readonly property string reconstructionMrcFile: outputDir ? outputDir + '/' + baseName + '_reconstruction.mrc' : ""
    
    // GeoParGen 临时重建文件
    readonly property string geoParGenTempMrcFile: outputDir ? outputDir + '/' + baseName + '_geopargen_temp.mrc' : ""
    
    // CTF 相关文件
    readonly property string ctfListFile: outputDir ? outputDir + '/' + baseName + '.list' : ""
    readonly property string ctfTxtFile: outputDir ? outputDir + '/' + baseName + '_ctf.txt' : ""
    readonly property string ctfMrcFile: outputDir ? outputDir + '/' + baseName + '_ctf.mrc' : ""

    // Trajectory trace 相关文件 (traj / trajplot)
    // 注: traj 命令使用的 xf 文件就是 fixedXfFile (markerauto/markerfree 生成的)
    readonly property string initFidFile: outputDir ? outputDir + '/init.fid.txt' : ""
    readonly property string finFidFile: outputDir ? outputDir + '/fin.fid.txt' : ""
    // trajplot 输出 SVG 图片 (init)
    readonly property string initTraceXZ: outputDir ? outputDir + '/init_trajxz.svg' : ""
    readonly property string initTraceYZ: outputDir ? outputDir + '/init_trajyz.svg' : ""
    readonly property string initTraceXY: outputDir ? outputDir + '/init_trajxy.svg' : ""
    // trajplot 输出 SVG 图片 (fin)
    readonly property string finTraceXZ: outputDir ? outputDir + '/fin_trajxz.svg' : ""
    readonly property string finTraceYZ: outputDir ? outputDir + '/fin_trajyz.svg' : ""
    readonly property string finTraceXY: outputDir ? outputDir + '/fin_trajxy.svg' : ""
    // trajtrace 是否已完成
    property bool trajTraceCompleted: false

    // ==================== 工作流进度状态 ====================

    property int processProgress: 0          // 当前进度 (0-100)
    property int totalFrames: -1             // 总帧数（markerauto2）
    property int totalBundleIterations: 50   // Bundle 迭代总数（markerauto2）

    property color progressColor: "transparent"  // 进度条颜色

    // ==================== TiltRec 配置状态 ====================

    property string tiltRecMethod: "BPT"       // 重建方法
    property string tiltRecAccMethod: "mpi"    // 加速方法 (mpi/cuda)
    property string tiltRecAxis: "y"           // 重建轴 (y/z)
    property int tiltRecThreads: 4             // 线程数
    property int tiltRecBinFactor: 1           // 降采样因子（Bin Factor），默认1（不降采样）

    // 几何参数
    property real geometryOffset: 0.0
    property real geometryPitch: 0.0
    property real geometryZOffset: 0.0
    property real geometryThickness: 100.0

    // 方法特定参数
    property int iterations: 10
    property real relaxation: 1.0
    property int cgIterations: 5
    property real threshold: 0.0

    // ==================== Markerfree 配置状态 ====================

    // 几何参数（7个整数）: offset, tilt axis angle, z-axis offset, thickness,
    // projection matching reconstruction thickness, output image downsampling ratio, GPU ID
    property real markerfreeOffset: 0.0
    property real markerfreeTiltAxisAngle: 0.0
    property real markerfreeZOffset: 0.0
    property real markerfreeThickness: 0.0
    property real markerfreeProjMatchThickness: 300.0
    property real markerfreeDownsampleRatio: 1.0
    property int markerfreeGpuId: 0
    property int markerfreeProjImageCount: 10  // -p 选项：投影匹配时重建的图像数量

    // 保存格式：0=txt格式，1=xf文件格式
    property int markerfreeSaveMode: 1

    // ==================== MarkerAuto2 配置状态 ====================

    property real markerAuto2BeadDiameter: -1.0  // -d 选项：标记点直径，-1 表示自动
    property bool markerAuto2FastMode: false     // -f 选项：快速对齐模式
    property bool markerAuto2EraseMarkers: false // 是否清除标记点（默认不清除）
    property bool markerAuto2EnableSupplement: false // 是否启用 -s 高角度补充
    property int markerAuto2SupplementAngle: 45      // -s 角度值，默认45

    // ==================== 工作流选择状态 ====================
    
    // 用户在 Screen02 选择的对齐方法: "markerfree" 或 "markerauto2"
    property string selectedWorkflow: "markerfree"

    // ==================== CTF 配置状态 ====================

    property bool ctfEnabled: false              // 是否启用 CTF 校正
    property bool ctfCompleted: false            // CTF 是否已完成运行
    property real ctfPixelSize: 10.09            // 像素大小 (Angstrom)
    property real ctfCs: 2.7                     // 球差系数 (mm)
    property int ctfVoltage: 300                 // 加速电压 (kV)
    property real ctfW: 0.07                     // 振幅对比度
    
    // CTF 使用的 tlt 文件（markerauto2 用生成的 tlt，markerfree 用 rawtlt）
    readonly property string ctfTltFile: selectedWorkflow === "markerauto2" ? fixedTltFile : rawtltFile
    
    // CTF 处理后用于重建的输入文件（只有 ctfEnabled 且 ctfCompleted 才使用 CTF 输出）
    readonly property string tiltRecInputMrcFile: ctfEnabled && ctfCompleted 
                                                  ? ctfMrcFile 
                                                  : alignedMrcFile

    // ==================== MRC 图像对比状态 ====================

    property int compareSliceIndex: 0            // 对比的切片索引
    property var compareOriginalImage: null      // 原始MRC图像
    property var compareAlignedImage: null       // 对齐后MRC图像

    // ==================== 状态查询（只读） ====================

    readonly property bool hasInputFiles: initialMrcFile !== "" && rawtltFile !== ""
    readonly property bool hasOutputDir: outputDir !== ""
    readonly property bool hasValidInputFiles: hasInputFiles && hasOutputDir
    readonly property bool isProcessing: processProgress > 0 && processProgress < 100
    readonly property bool isProcessComplete: processProgress === 100

    // TiltRec 配置验证
    readonly property bool hasAlignedMrcFile: alignedMrcFile !== ""
    readonly property bool hasValidTiltRecInput: hasAlignedMrcFile && fixedTltFile !== ""

    // ==================== 信号 ====================

    signal stateChanged(string key, var value)  // 通用状态变化信号
    signal progressChanged(int progress)         // 进度变化
    signal filesSelected()                       // 文件选择完成
    signal validationFailed(string message)      // 验证失败

    // ==================== 数据验证方法 ====================

    /**
     * 验证输入文件是否有效
     * @returns 验证结果对象 { valid: bool, message: string }
     */
    function validateInputFiles() {
        let errors = []

        if (!initialMrcFile) {
            errors.push("请选择 MRC 文件")
        }

        if (!rawtltFile) {
            errors.push("请选择 Rawtlt 角度文件")
        }

        if (!outputDir) {
            errors.push("请选择输出目录")
        }

        // 检查文件是否存在（简单的路径检查）
        if (initialMrcFile && !initialMrcFile.endsWith(".mrc") && !initialMrcFile.endsWith(".st")) {
            errors.push("MRC 文件格式不正确（应为 .mrc 或 .st）")
        }

        if (rawtltFile && !rawtltFile.endsWith(".rawtlt")) {
            errors.push("Rawtlt 文件格式不正确（应为 .rawtlt）")
        }

        let isValid = errors.length === 0
        let message = isValid ? "输入文件验证通过" : errors.join("；")

        if (!isValid) {
            validationFailed(message)
        }

        return {
            valid: isValid,
            message: message,
            errors: errors
        }
    }

    /**
     * 验证 TiltRec 配置是否有效
     * @returns 验证结果对象 { valid: bool, message: string }
     */
    function validateTiltRecConfig() {
        let errors = []

        if (!alignedMrcFile) {
            errors.push("缺少对齐后的 MRC 文件（请先完成步骤 1 和 2）")
        }

        if (!fixedTltFile) {
            errors.push("缺少 Tilt 角度文件（请先完成步骤 1）")
        }

        if (!outputDir) {
            errors.push("缺少输出目录")
        }

        if (tiltRecThreads < 1 || tiltRecThreads > 128) {
            errors.push("线程数应在 1-128 之间")
        }

        if (geometryThickness <= 0) {
            errors.push("几何厚度必须大于 0")
        }

        // 方法特定验证
        if (tiltRecMethod === "SART" || tiltRecMethod === "SIRT" || tiltRecMethod === "ADMM") {
            if (iterations < 1) {
                errors.push("迭代次数必须大于 0")
            }
        }

        if (tiltRecMethod === "ADMM") {
            if (cgIterations < 1) {
                errors.push("CG 迭代次数必须大于 0")
            }
        }

        let isValid = errors.length === 0
        let message = isValid ? "TiltRec 配置验证通过" : errors.join("；")

        if (!isValid) {
            validationFailed(message)
        }

        return {
            valid: isValid,
            message: message,
            errors: errors
        }
    }

    /**
     * 验证所有状态（用于调试）
     * @returns 完整验证结果
     */
    function validateAll() {
        return {
            inputFiles: validateInputFiles(),
            tiltRecConfig: validateTiltRecConfig()
        }
    }

    // ==================== 公共方法 ====================

    /**
     * 设置输入文件（Step 1）
     */
    function setInputFiles(mrcPath, tltPath, outDir) {
        console.log("AppState: Setting input files")
        console.log("  MRC:", mrcPath)
        console.log("  TLT:", tltPath)
        console.log("  Output:", outDir)

        initialMrcFile = mrcPath
        rawtltFile = tltPath
        outputDir = outDir

        filesSelected()
    }

    /**
     * 更新进度（由 LogicHandler 调用）
     */
    function updateProgress(progress) {
        if (processProgress !== progress) {
            processProgress = progress
            progressChanged(progress)
        }
    }

    /**
     * 设置进度条颜色
     */
    function setProgressColor(color) {
        progressColor = color
    }

    /**
     * 重置进度状态（开始新任务前调用）
     */
    function resetProgress() {
        processProgress = 0
        totalFrames = -1
        progressColor = "transparent"
    }

    /**
     * 设置总帧数（markerauto2 解析到时调用）
     */
    function setTotalFrames(frames) {
        totalFrames = frames
        console.log("AppState: Total frames set to", frames)
    }

    /**
     * 设置 TiltRec 配置
     */
    function setTiltRecConfig(config) {
        if (config.method !== undefined) tiltRecMethod = config.method
        if (config.accmethod !== undefined) tiltRecAccMethod = config.accmethod
        if (config.axis !== undefined) tiltRecAxis = config.axis
        if (config.threads !== undefined) tiltRecThreads = config.threads
        if (config.binFactor !== undefined) tiltRecBinFactor = config.binFactor

        if (config.geometryOffset !== undefined) geometryOffset = config.geometryOffset
        if (config.geometryPitch !== undefined) geometryPitch = config.geometryPitch
        if (config.geometryZOffset !== undefined) geometryZOffset = config.geometryZOffset
        if (config.geometryThickness !== undefined) geometryThickness = config.geometryThickness

        if (config.iterations !== undefined) iterations = config.iterations
        if (config.relaxation !== undefined) relaxation = config.relaxation
        if (config.cgIterations !== undefined) cgIterations = config.cgIterations
        if (config.threshold !== undefined) threshold = config.threshold
    }

    /**
     * 设置 Markerfree 配置
     */
    function setMarkerfreeConfig(config) {
        if (config.offset !== undefined) markerfreeOffset = config.offset
        if (config.tiltAxisAngle !== undefined) markerfreeTiltAxisAngle = config.tiltAxisAngle
        if (config.zOffset !== undefined) markerfreeZOffset = config.zOffset
        if (config.thickness !== undefined) markerfreeThickness = config.thickness
        if (config.projMatchThickness !== undefined) markerfreeProjMatchThickness = config.projMatchThickness
        if (config.downsampleRatio !== undefined) markerfreeDownsampleRatio = config.downsampleRatio
        if (config.gpuId !== undefined) markerfreeGpuId = config.gpuId
        if (config.saveMode !== undefined) markerfreeSaveMode = config.saveMode
        if (config.projImageCount !== undefined) markerfreeProjImageCount = config.projImageCount
    }

    /**
     * 设置 MarkerAuto2 配置
     */
    function setMarkerAuto2Config(config) {
        if (config.beadDiameter !== undefined) markerAuto2BeadDiameter = config.beadDiameter
        if (config.fastMode !== undefined) markerAuto2FastMode = config.fastMode
        if (config.eraseMarkers !== undefined) markerAuto2EraseMarkers = config.eraseMarkers
        if (config.enableSupplement !== undefined) markerAuto2EnableSupplement = config.enableSupplement
        if (config.supplementAngle !== undefined) markerAuto2SupplementAngle = config.supplementAngle
    }

    /**
     * 验证 Markerfree 配置是否有效
     * @returns 验证结果对象 { valid: bool, message: string }
     */
    function validateMarkerfreeConfig() {
        let errors = []

        if (!initialMrcFile) {
            errors.push("缺少输入 MRC 文件")
        }

        if (!rawtltFile) {
            errors.push("缺少 Tilt 角度文件")
        }

        if (!outputDir) {
            errors.push("缺少输出目录")
        }

        if (markerfreeProjMatchThickness <= 0) {
            errors.push("投影匹配重建厚度必须大于 0")
        }

        if (markerfreeDownsampleRatio <= 0) {
            errors.push("下采样比率必须大于 0")
        }

        if (markerfreeGpuId < 0) {
            errors.push("GPU ID 必须 >= 0")
        }

        let isValid = errors.length === 0
        let message = isValid ? "Markerfree 配置验证通过" : errors.join("；")

        if (!isValid) {
            validationFailed(message)
        }

        return {
            valid: isValid,
            message: message,
            errors: errors
        }
    }

    /**
     * 清空所有状态（用于重新开始）
     */
    function clearAll() {
        console.log("AppState: Clearing all state")

        initialMrcFile = ""
        rawtltFile = ""
        outputDir = ""

        resetProgress()

        // 不清空 TiltRec 配置（保留用户设置）
    }

    // ==================== 工具函数 ====================

    /**
     * 从完整文件路径提取基本文件名（不含扩展名）
     * @param filePath - 完整文件路径
     * @returns 基本文件名（如 "sample" from "C:/data/sample.mrc"）
     */
    function extractBaseName(filePath) {
        if (!filePath) return ""

        let lastSlashIndex = Math.max(filePath.lastIndexOf('/'), filePath.lastIndexOf('\\'))
        let fileName = filePath.substring(lastSlashIndex + 1)
        let lastDotIndex = fileName.lastIndexOf('.')

        return (lastDotIndex === -1) ? fileName : fileName.substring(0, lastDotIndex)
    }

    /**
     * 获取当前状态快照（用于调试）
     */
    function getSnapshot() {
        return {
            files: {
                initial: initialMrcFile,
                rawtlt: rawtltFile,
                output: outputDir,
                baseName: baseName,
                aligned: alignedMrcFile,
                reconstruction: reconstructionMrcFile
            },
            progress: {
                current: processProgress,
                totalFrames: totalFrames,
                color: progressColor
            },
            tiltRec: {
                method: tiltRecMethod,
                accmethod: tiltRecAccMethod,
                axis: tiltRecAxis,
                threads: tiltRecThreads,
                geometry: {
                    offset: geometryOffset,
                    pitch: geometryPitch,
                    zOffset: geometryZOffset,
                    thickness: geometryThickness
                }
            }
        }
    }
}
