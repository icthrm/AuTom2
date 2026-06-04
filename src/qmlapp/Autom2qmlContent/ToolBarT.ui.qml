import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: toolbarBox

    // property var stateManager

    // // 高度根据内容自适应，但设定一个最小高度以保持美观
    // // height: toolsLayout.implicitHeight + 20
    // color: "#ff00ff"
    // border.color: "#dcdcdc"
    border.width: 1
    radius: 8 // 圆角

    GridLayout {
        id: toolsLayout
        anchors.centerIn: parent
        width: parent.width - 20 // 留点内边距

        columns: 1 // 设置为 1 即为单列多行；设置为 2 则为双列多行
        rowSpacing: 8
        columnSpacing: 8

        // --- 工具按钮组 ---
        ViewMrcButton {
            Layout.fillWidth: true
        }

        // Button {
        //     text: "画笔 (S1)"
        //     Layout.fillWidth: true
        //     // 仅在 step1 显示
        //     visible: stateManager.currentStep === "step1"
        //     // onClicked: console.log("画笔 clicked")
        // }

        // Button {
        //     text: "橡皮擦 (S1/S2)"
        //     Layout.fillWidth: true
        //     // 在 step1 或 step2 显示
        //     visible: stateManager.currentStep === "step1"
        //              || stateManager.currentStep === "step2"
        // }

        // Button {
        //     text: "裁剪 (S2)"
        //     Layout.fillWidth: true
        //     // 仅在 step2 显示
        //     visible: stateManager.currentStep === "step2"
        // }

        // Button {
        //     text: "滤镜 A (S3)"
        //     Layout.fillWidth: true
        //     visible: stateManager.currentStep === "step3"
        // }

        // Button {
        //     text: "导出 (S3)"
        //     Layout.fillWidth: true
        //     // 高亮重要按钮
        //     highlighted: true
        //     visible: stateManager.currentStep === "step3"
        // }
    }
}
