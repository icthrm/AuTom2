// PathSelectorForm.ui.qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

// import QtGraphicalEffects

// This is a UI-only form, containing the visual components.
RowLayout {
    // We give the root an id to be referenced by the logic file.
    id: root

    // --- 1. Properties exposed to the outside world ---
    property alias path: pathInput.text
    property string fileDialogTitle: "Select File or Folder"

    // --- NEW: Expose internal components via alias ---
    // This allows the logic file (PathSelector.qml) to access them by their id.
    // Syntax: property alias <public_name>: <internal_id>
    property alias pathInput: pathInput
    property alias browseButton: browseButton
    property alias buttonText: browseButton.text

    // --- 3. UI Components ---
    Rectangle {
        // 1. 给容器一个固定的高度，确保它比 TextInput 大
        implicitHeight: 40 // 使用 implicitHeight，这样外部布局还能覆盖它

        color: "#f0f0f0"
        border.color: "#888888" // 边框颜色调灰一点可能更好看
        border.width: 1
        radius: 5
        Layout.fillWidth: true // 这个属性保持，用于水平填充

        TextField {
            id: pathInput

            // 2. 将输入框在垂直和水平方向上居中于容器
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 10 // 留出左内边距
            anchors.rightMargin: 10 // 留出右内边距

            // 3. (最关键一步) 让输入框自己的背景消失！
            background: null
        }
    }

    Button {
        id: browseButton
        text: qsTr("Browse...")

        // --- 逻辑 ---
        // 文本颜色会根据状态改变
        contentItem: Text {
            text: browseButton.text
            font: browseButton.font
            // 当鼠标悬停或按下时，文本变白，否则为蓝色
            color: browseButton.hovered
                   || browseButton.down ? "#FFFFFF" : "#007BFF"

            // 文本颜色的平滑过渡
            Behavior on color {
                ColorAnimation {
                    duration: 150
                }
            }
        }

        background: Rectangle {
            // --- 可自定义参数 ---
            property color normalBorderColor: "#007BFF" // 边框颜色
            property color fillColor: "#007BFF" // 填充颜色
            property int borderRadius: 5 // 圆角
            property int borderWidth: 2 // 边框宽度

            // --- 逻辑 ---
            id: backgroundRect
            // 默认背景透明，悬停或按下时填充颜色
            color: browseButton.hovered
                   || browseButton.down ? fillColor : "transparent"
            border.color: normalBorderColor
            border.width: borderWidth
            radius: borderRadius

            // 背景颜色的平滑过渡
            Behavior on color {
                ColorAnimation {
                    duration: 150
                }
            }
        }
    }
}
