

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import Autom2qml

Button {
    id: viewMrcButton
    text: "View MRC"

    // 使用 anchors 定位到右下角
    //anchors.right: parent.right
    //anchors.bottom: parent.bottom
    //anchors.margins: 15 // 设置按钮距离窗口边缘的距离

    // 按钮的启用逻辑：只有当选择了mrc文件且进程未在运行时才可点击
    enabled: !isProcessRunning && (pathselector_mrc.path !== ""
                                   || AppState.initialMrcFile !== "")

    // 点击事件的处理器
    Connections {
        target: viewMrcButton

        onClicked: {

            backend.runCommand("Autom3D", [pathselector_mrc.path])

            // 临时打印日志，确认按钮工作正常
            console.log("View MRC button clicked for path: " + pathselector_mrc.path)
        }
    }
}
