// Autom2qml/PathSelector.qml

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

// This component uses the UI form and adds the business logic.
PathSelectorForm {
    id: root

    property var nameFilters: []
    property bool selectFolder: false

    // 跨平台默认路径
    readonly property string defaultFolder: Qt.platform.os === "windows"
                                           ? "file:///C:/"
                                           : "file:///home"

    // --- Logic for the UI elements ---

    // 当'path'属性改变时触发 (无论是手动输入还是通过文件对话框设置)
    // 这是 QML 自动提供的处理器，无需在 .ui.qml 中手动发射信号。
    onPathChanged: {
        console.log("Path has changed to: " + root.path)
        // 在这里可以添加需要响应路径变化的其他逻辑
    }

    // 处理按钮点击
    browseButton.onClicked: {
        if (selectFolder == false)
            fileDialog.open()
        else
            folderDialog.open()
    }


    // --- Backend Logic Component (non-visual) ---

    /**
     * 将 file:// URL 转换为本地路径
     * 跨平台兼容处理
     */
    function urlToLocalPath(urlString) {
        // 1. 解码 URI 编码字符（如空格 %20）
        let decodedString = decodeURIComponent(urlString)

        // 2. 移除 file:// 协议前缀
        // 注意：file:// URL 格式：
        // - Windows: file:///C:/path/to/file
        // - Unix:    file:///home/user/file
        // 都是三个斜杠，所以统一移除前 7 个字符 "file://"
        if (decodedString.startsWith("file://")) {
            decodedString = decodedString.substring(7)
        }

        // 3. 处理平台差异
        // Windows: 路径以盘符开头，保留为 /C:/path，然后去掉开头的 /
        // Unix: 路径以 / 开头，保持为 //home/user，然后去掉多余的 /

        // 检测是否为 Windows 路径（包含盘符）
        // 格式：/C:/path 或 /c:/path
        let isWindowsPath = /^\/[a-zA-Z]:/.test(decodedString)

        if (isWindowsPath) {
            // Windows: 移除开头的 /
            // /C:/path → C:/path
            return decodedString.substring(1)
        } else {
            // Unix: 确保只有一个开头的 /
            // 可能是 //home 或 /home
            while (decodedString.startsWith("//")) {
                decodedString = decodedString.substring(1)
            }
            // 确保至少有一个 /
            if (!decodedString.startsWith("/")) {
                decodedString = "/" + decodedString
            }
            return decodedString
        }
    }

    FileDialog {
        id: fileDialog
        title: root.fileDialogTitle
        currentFolder: root.defaultFolder

        nameFilters: root.nameFilters

        onAccepted: {
            let urlString = fileDialog.selectedFile

            if (urlString) {
                root.path = root.urlToLocalPath(urlString)
                console.log("File selected: " + root.path)
            } else {
                console.warn("FileDialog was accepted, but the file string is empty.")
            }
        }

        onRejected: {
            console.log("User canceled file selection")
        }
    }

    FolderDialog {
        id: folderDialog
        title: root.fileDialogTitle
        currentFolder: root.defaultFolder

        onAccepted: {
            let urlString = folderDialog.selectedFolder

            if (urlString) {
                root.path = root.urlToLocalPath(urlString)
                console.log("Folder selected: " + root.path)
            } else {
                console.warn("FolderDialog was accepted, but the folder string is empty.")
            }
        }

        onRejected: {
            console.log("User canceled folder selection")
        }
    }

}
