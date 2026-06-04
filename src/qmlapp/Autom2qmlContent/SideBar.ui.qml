

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Autom2qml

Rectangle {
    id: root
    width: 150
    Layout.minimumWidth: 150

    ColumnLayout {
        id: colLayout

        anchors.fill: parent
        anchors.margins: 10
        spacing: 5

        // 步骤按钮列表（动态生成）
        Repeater {
            model: Router.sidebarTitles

            delegate: StyledButton {
                id: stepButton
                required property int index
                required property string modelData

                text: modelData
                Layout.fillWidth: true
                Layout.preferredHeight: 100

                // 高亮当前步骤
                highlighted: Router.currentStepIndex === index

                Connections {
                    function onClicked() {
                        Router.navigateToStep(index)
                    }
                }
            }
        }

        // 返回按钮（仅在非首状态显示）
        Button {
            id: backButton
            text: "Back"

            Layout.fillWidth: true
            Layout.preferredHeight: 100

            visible: Router.canGoBackState

            Connections {
                function onClicked() {
                    Router.goBackState()
                }
            }
        }

        // 弹性空间
        Item {
            Layout.fillHeight: true
        }
    }
}
