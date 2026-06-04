import QtQuick
import QtQuick.Controls

// 这是一个纯布局文件
Item {
    id: root
    width: 120
    height: 40

    // 暴露给外部逻辑层的属性
    property alias text: label.text
    property alias bodyColor: bg.color
    property alias borderColor: bg.border.color

    // 背景
    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 6
        color: "#0078D4"
        border.width: 1
        border.color: "transparent"

        // 内部文字
        Text {
            id: label
            text: "Button"
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 14
            font.family: "Microsoft YaHei UI"
        }
    }

    // 定义视觉状态
    states: [
        State {
            name: "hover"
            PropertyChanges {
                target: bg
                color: "#106EBE"
            }
        },
        State {
            name: "pressed"
            PropertyChanges {
                target: bg
                color: "#005A9E"
                scale: 0.95
            }
        },
        State {
            name: "normal"
            PropertyChanges {
                target: bg
                color: "#0078D4"
                scale: 1.0
            }
        }
    ]

    // 状态切换动画
    transitions: Transition {
        NumberAnimation {
            properties: "scale"
            duration: 100
            easing.type: Easing.OutQuad
        }
        ColorAnimation {
            duration: 150
        }
    }
}
