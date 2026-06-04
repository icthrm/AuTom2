import QtQuick
import QtQuick.Controls

// 纯布局文件
Item {
    id: root
    width: 200
    height: 4

    Rectangle {
        id: bg
        anchors.fill: parent
        color: "#E0E0E0"
        radius: 2

        Rectangle {
            id: fill
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 0
            color: "#4CAF50"
            radius: 2
        }

        // 等待动画指示器
        Rectangle {
            id: waitingIndicator
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width * 0.3
            color: "#2196F3"
            radius: 2
            opacity: 0
            x: 0

            SequentialAnimation on x {
                running: waitingIndicator.opacity > 0
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0
                    to: bg.width - waitingIndicator.width
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: bg.width - waitingIndicator.width
                    to: 0
                    duration: 1000
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    // 定义视觉状态
    states: [
        State {
            name: "empty"
            PropertyChanges {
                target: fill
                width: 0
            }
            PropertyChanges {
                target: waitingIndicator
                opacity: 0
            }
        },
        State {
            name: "waiting"
            PropertyChanges {
                target: fill
                width: 0
            }
            PropertyChanges {
                target: waitingIndicator
                opacity: 1
            }
        },
        State {
            name: "completed"
            PropertyChanges {
                target: fill
                width: bg.width
            }
            PropertyChanges {
                target: waitingIndicator
                opacity: 0
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            properties: "width,opacity"
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
}
