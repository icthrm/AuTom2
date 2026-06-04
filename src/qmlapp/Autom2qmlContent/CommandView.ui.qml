

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import Autom2qml

Rectangle {
    border.width: 1
    radius: 8 // 圆角

    property var bindProgram

    ScrollView {
        id: logScrollView // 给 ScrollView 一个ID

        // 设置滚动条策略
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        ScrollBar.horizontal.policy: ScrollBar.AsNeeded

        height: parent.height
        width: parent.width

        TextArea {
            id: resultLabel

            // TextArea 不再需要布局属性，它可以在 ScrollView 内部自由增长
            font.pixelSize: 16
            color: "navy"
            readOnly: true
            // 注意: 当使用水平滚动条时，可以考虑关闭自动换行
            // wrapMode: Text.NoWrap
            wrapMode: TextArea.Wrap // 或者保持自动换行，这样通常不需要水平滚动条

            // --- 关键改动: 自动滚动逻辑 ---
            // 当内容高度变化时 (即添加了新行), 将滚动条移动到底部
            Connections {
                function onContentHeightChanged() {
                    // 确保只有在滚动条接近底部时才自动滚动
                    // 这样如果用户手动向上滚动查看日志，就不会被强制拉回底部
                    if (logScrollView.ScrollBar.vertical.position > 0.95) {
                        logScrollView.ScrollBar.vertical.position = 1.0
                    }
                }
            }
        }
        Connections {
            target: backend

            function onDataReceived(chunk, program) {
                resultLabel.append(chunk)
                LogicHandler.parseProgress(chunk)
            }
        }
        Connections {
            target: LogicHandler

            // When the logic handler wants to clear the log...
            function onClearLog() {
                resultLabel.clear()
                resultLabel.color = "navy" // Reset color
            }

            // When the logic handler sends a color update...
            function onProgressUpdated(newColor) {
                resultLabel.color = newColor
            }
        }
    }
}
