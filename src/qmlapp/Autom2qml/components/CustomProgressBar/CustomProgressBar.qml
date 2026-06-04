import QtQuick
import QtQuick.Controls
import Autom2qml

Item {
    id: control

    implicitWidth: 200
    implicitHeight: 4

    property string bindProgram: ""

    CustomProgressBarForm {
        id: ui
        anchors.fill: parent

        // 根据 AppState.processProgress 驱动视觉状态
        state: {
            console.log("running:", LogicHandler.runningProgram)
            console.log("binding:", bindProgram)
            if ((!LogicHandler.isProcessRunning && !AppState.processProgress === 100) || !(LogicHandler.runningProgram === bindProgram) ) {
                return "empty"
            } else if (AppState.processProgress === 100) {
                return "completed"
            } else {
                return "waiting"
            }
        }
    }
}
