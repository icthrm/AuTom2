import QtQuick
import QtQuick.Controls

Button {
    id: control

    // 引用 UI 文件
    // 注意：StyledButtonForm 是由 StyledButtonForm.ui.qml 自动生成的组件名
    contentItem: StyledButtonForm {
        id: ui
        text: control.text

        // 根据 Button 的逻辑状态驱动 UI 的视觉状态
        state: {
            if (control.pressed) return "pressed"
            if (control.hovered) return "hover"
            return "normal"
        }
    }

    // 隐藏原生的背景，完全交给 UI 文件处理
    background: null

    // 设置鼠标形状
    HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
}
