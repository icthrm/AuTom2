// FilePairScanner.qml
// 文件配对扫描器 - 扫描文件夹中的 .mrc 和 .rawtlt 文件配对

import QtQuick
import QtQuick.Dialogs

QtObject {
    id: root

    // ==================== 属性 ====================
    property string selectedFolder: ""
    property var filePairs: []  // [{mrcFile, rawtltFile, basename}, ...]
    property bool isScanning: false
    property int pairCount: 0

    // Backend 引用
    property var backend: null

    // ==================== 信号 ====================
    signal scanCompleted(int pairCount)
    signal scanFailed(string error)

    // ==================== 方法 ====================

    /**
     * 扫描文件夹中的 MRC 和 rawtlt 文件配对
     * @param folderPath - 文件夹路径
     */
    function scanFolder(folderPath) {
        if (isScanning) {
            console.warn("FilePairScanner: Already scanning")
            return false
        }

        if (!folderPath) {
            console.error("FilePairScanner: Empty folder path")
            scanFailed("文件夹路径为空")
            return false
        }

        console.log("FilePairScanner: Scanning folder:", folderPath)
        selectedFolder = folderPath
        isScanning = true

        // 检查 backend 是否支持文件扫描
        if (backend && typeof backend.scanMrcRawtltPairs === "function") {
            // 使用 backend 的文件扫描功能
            scanWithBackend(folderPath)
        } else {
            // Backend 不支持，返回错误
            console.error("FilePairScanner: Backend does not support scanMrcRawtltPairs")
            isScanning = false
            scanFailed("Backend 不支持文件扫描功能，请手动选择文件")
            return false
        }

        return true
    }

    /**
     * 使用 backend 扫描文件
     */
    function scanWithBackend(folderPath) {
        try {
            // 调用 backend 的扫描方法
            // 期望返回格式: [{mrcFile, rawtltFile, basename}, ...]
            let result = backend.scanMrcRawtltPairs(folderPath)

            if (result && Array.isArray(result)) {
                let existingBasenames = filePairs.map(function(p) { return p.basename })
                let newPairs = result.filter(function(p) { return existingBasenames.indexOf(p.basename) === -1 })
                filePairs = filePairs.concat(newPairs)
                pairCount = filePairs.length
                isScanning = false

                console.log(`FilePairScanner: Added ${newPairs.length} new pairs, total ${pairCount}`)
                scanCompleted(pairCount)
            } else {
                isScanning = false
                scanFailed("扫描结果格式错误")
            }
        } catch (error) {
            console.error("FilePairScanner: Error during scan:", error)
            isScanning = false
            scanFailed("扫描失败: " + error)
        }
    }

    /**
     * 手动添加文件配对
     * @param mrcFile - MRC 文件路径
     * @param rawtltFile - Rawtlt 文件路径
     */
    function addFilePair(mrcFile, rawtltFile) {
        if (!mrcFile || !rawtltFile) {
            console.warn("FilePairScanner: Invalid file pair")
            return false
        }

        // 提取 basename
        let basename = extractBaseName(mrcFile)

        let pair = {
            mrcFile: mrcFile,
            rawtltFile: rawtltFile,
            basename: basename
        }

        filePairs.push(pair)
        pairCount = filePairs.length

        console.log("FilePairScanner: Added file pair:", basename)
        return true
    }

    /**
     * 从文件列表中自动配对
     * @param fileUrls - 文件 URL 列表
     */
    function autoMatchPairs(fileUrls) {
        if (!fileUrls || fileUrls.length === 0) {
            console.warn("FilePairScanner: Empty file list")
            return false
        }

        console.log("FilePairScanner: Auto-matching", fileUrls.length, "files")

        // 分离 .mrc 和 .rawtlt 文件
        let mrcFiles = []
        let rawtltFiles = []

        for (let i = 0; i < fileUrls.length; i++) {
            let url = fileUrls[i].toString()
            let path = urlToLocalPath(url)
            let lowerPath = path.toLowerCase()

            if (lowerPath.endsWith(".mrc") || lowerPath.endsWith(".st")) {
                mrcFiles.push(path)
            } else if (lowerPath.endsWith(".rawtlt") || lowerPath.endsWith(".tlt")) {
                rawtltFiles.push(path)
            }
        }

        console.log("FilePairScanner: Found", mrcFiles.length, "MRC files and", rawtltFiles.length, "rawtlt files")

        // 配对文件
        let pairs = []
        for (let i = 0; i < mrcFiles.length; i++) {
            let mrcFile = mrcFiles[i]
            let mrcBasename = extractBaseName(mrcFile)

            // 查找同名的 rawtlt 文件
            let matchedRawtlt = null
            for (let j = 0; j < rawtltFiles.length; j++) {
                let rawtltFile = rawtltFiles[j]
                let rawtltBasename = extractBaseName(rawtltFile)

                if (mrcBasename === rawtltBasename) {
                    matchedRawtlt = rawtltFile
                    break
                }
            }

            // 如果在已选文件中没有找到，尝试在同目录下查找
            if (!matchedRawtlt && backend && typeof backend.fileExists === "function") {
                let dir = getDirectory(mrcFile)
                let candidate = dir + "/" + mrcBasename + ".rawtlt"
                if (backend.fileExists(candidate)) {
                    matchedRawtlt = candidate
                    console.log("FilePairScanner: Auto-found rawtlt in directory:", candidate)
                }
            }

            if (matchedRawtlt) {
                pairs.push({
                    mrcFile: mrcFile,
                    rawtltFile: matchedRawtlt,
                    basename: mrcBasename
                })
                console.log("FilePairScanner: Matched pair:", mrcBasename)
            } else {
                console.warn("FilePairScanner: No rawtlt found for:", mrcBasename)
            }
        }

        // 追加到现有列表，按 basename 去重
        let existingBasenames = filePairs.map(function(p) { return p.basename })
        let newPairs = pairs.filter(function(p) { return existingBasenames.indexOf(p.basename) === -1 })
        filePairs = filePairs.concat(newPairs)
        pairCount = filePairs.length

        console.log(`FilePairScanner: Added ${newPairs.length} new pairs, total ${pairCount}`)
        scanCompleted(pairCount)

        return true
    }

    /**
     * 清空文件列表
     */
    function clear() {
        filePairs = []
        pairCount = 0
        selectedFolder = ""
        console.log("FilePairScanner: Cleared file list")
    }

    /**
     * 移除指定索引的文件配对
     */
    function removePair(index) {
        if (index >= 0 && index < filePairs.length) {
            let removed = filePairs.splice(index, 1)
            pairCount = filePairs.length
            console.log("FilePairScanner: Removed pair:", removed[0].basename)
            return true
        }
        return false
    }

    /**
     * 获取所有文件配对
     */
    function getAllPairs() {
        return filePairs
    }

    // ==================== 辅助函数 ====================

    /**
     * 从文件路径提取所在目录
     */
    function getDirectory(filePath) {
        if (!filePath) return ""
        let lastSlash = filePath.lastIndexOf('/')
        if (lastSlash === -1) lastSlash = filePath.lastIndexOf('\\')
        if (lastSlash === -1) return ""
        return filePath.substring(0, lastSlash)
    }

    /**
     * 从文件路径提取 basename（不含扩展名）
     */
    function extractBaseName(filePath) {
        if (!filePath) return ""

        // 移除路径，只保留文件名
        let fileName = filePath.split('/').pop().split('\\').pop()

        // 移除扩展名
        let lastDotIndex = fileName.lastIndexOf('.')
        if (lastDotIndex > 0) {
            return fileName.substring(0, lastDotIndex)
        }

        return fileName
    }

    /**
     * 将 file:// URL 转换为本地路径
     */
    function urlToLocalPath(url) {
        if (!url) return ""

        let urlString = url.toString()

        // 移除 file:// 前缀
        if (urlString.startsWith("file://")) {
            urlString = urlString.substring(7)
        }

        // 处理 Windows 路径 (file:///C:/path -> C:/path)
        let isWindowsPath = /^\/[a-zA-Z]:/.test(urlString)
        if (isWindowsPath) {
            urlString = urlString.substring(1)
        } else if (/^[a-zA-Z]:/.test(urlString)) {
            // 已经是 C:/path 或 C:\path 格式，保持原样
        } else {
            // Unix 路径，确保有前导斜杠
            while (urlString.startsWith("//")) {
                urlString = urlString.substring(1)
            }
            if (!urlString.startsWith("/")) {
                urlString = "/" + urlString
            }
        }

        // URL 解码
        urlString = decodeURIComponent(urlString)

        return urlString
    }
}
