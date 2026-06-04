import QtQuick
import Autom2qml
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Window {

    id: mainWindow

    width: Constants.width
    height: Constants.height
    minimumWidth: mainLayout.implicitWidth + mainLayout.anchors.leftMargin + mainLayout.anchors.rightMargin
    minimumHeight: mainLayout.implicitHeight + mainLayout.anchors.topMargin + mainLayout.anchors.bottomMargin

    visible: true
    title: "Autom2qml"

    readonly property bool isInDesignMode: Qt.application.name.startsWith("qmlpuppet")

    property var backend: isInDesignMode ? MockBackend : realBackend

    // 控制 CommandView 的显示/隐藏（用户手动控制）
    property bool userWantsCommandView: false

    // 实际是否显示 CommandView（考虑路由配置和用户偏好）
    readonly property bool showCommandView: Router.shouldShowCommandView && userWantsCommandView


    Component.onCompleted: {
            // 初始化后端引用
            LogicHandler.backend = mainWindow.backend;
            CommandChainManager.backend = mainWindow.backend;
            console.log("Backend 已初始化到 LogicHandler 和 CommandChainManager");
        }


    RowLayout {
        id: mainLayout

        anchors.fill: parent
        anchors.leftMargin: -6
        spacing: 0

        SideBar {
            id: sideBar
            Layout.preferredWidth: 0.1 * parent.width
            Layout.fillHeight: true
        }

        // 主内容区域（StackView）
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 400

            // 主内容 StackView
            StackView {
                id: stackView
                anchors.fill: parent

                initialItem: "ScreenBlank.ui.qml"

                Component.onCompleted: {
                    console.log("StackView is complete. Initializing router...")
                    Router.navigateToStep(0)
                }
            }

            // 全局 CommandView 覆盖层（浮动在内容上方）
            Rectangle {
                id: commandViewOverlay
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: showCommandView ? 300 : 50
                visible: Router.shouldShowCommandView
                z: 100  // 提升层级，浮在StackView上方

                Behavior on height {
                    NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
                }

                // 折叠/展开按钮
                Rectangle {
                    id: commandViewHeader
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 50
                    color: "#34495E"

                    // 添加阴影效果
                    layer.enabled: true
                    layer.effect: DropShadow {
                        verticalOffset: -3
                        color: "#40000000"
                        radius: 8
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 15
                        anchors.rightMargin: 15
                        spacing: 10

                        Text {
                            text: "命令输出"
                            color: "white"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Rectangle {
                            width: 60
                            height: 20
                            color: CommandChainManager.isChainRunning
                                   ? "#27AE60" : "transparent"
                            radius: 3
                            visible: CommandChainManager.isChainRunning

                            Text {
                                anchors.centerIn: parent
                                text: "运行中"
                                color: "white"
                                font.pixelSize: 11
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Button {
                            text: userWantsCommandView ? "▼ 隐藏" : "▲ 显示"
                            flat: true

                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                userWantsCommandView = !userWantsCommandView
                            }
                        }
                    }
                }

                // 全局 CommandView（接收所有程序输出）
                CommandView {
                    id: globalCommandView
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: commandViewHeader.bottom
                    anchors.bottom: parent.bottom
                    visible: userWantsCommandView

                    // 接收所有程序的输出（不过滤）
                    bindProgram: ["markerauto2", "mrcstack", "mpirun", "Markerfree", "markererase"]
                }
            }
        }
    }

    // ==================== 路由导航监听 ====================
    Connections {
        target: Router

        function onNavigated(screenPath) {
            console.log("Navigating to:", screenPath)

            // 清理 StackView（保留初始页面）
            if (stackView.depth > 1) {
                stackView.pop()
            }

            // 推入新页面
            stackView.push(screenPath)
        }
    }

    // ================错误处理=================

    function showError(msg) {
        globalErrorDialog.errorText = msg;
        globalErrorDialog.open();
    }

    Dialog {
        id: globalErrorDialog
        title: "⚠️ 系统错误"

        // 1. 设置 Dialog 的宽度（或者根据窗口比例设置，例如 rootWindow.width * 0.8）
        width: Math.min(parent.width * 0.9, 400)

        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok

        property alias errorText: messageLabel.text

        // 2. 使用 ColumnLayout 或 Item 容纳内容
        Column {
            width: parent.width // 让 Column 撑满对话框宽度
            spacing: 15
            topPadding: 10
            bottomPadding: 10

            Label {
                id: messageLabel
                width: parent.width // 关键：让 Label 撑满 Column
                text: ""
                wrapMode: Text.WordWrap      // 关键：允许自动换行
                font.pixelSize: 14
                lineHeight: 1.2
                horizontalAlignment: Text.AlignLeft // 文字左对齐

                // 如果报错信息特别长，可以限制最大高度
                // elide: Text.ElideRight
            }
        }
    }



}

