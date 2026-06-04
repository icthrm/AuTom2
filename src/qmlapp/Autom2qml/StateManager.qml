pragma Singleton

import QtQuick
import QtQuick.Controls

QtObject {
    id: root

    // --- Workflow State Management ---

    readonly property int stepMarkerauto: 0
    readonly property int step2: 1
    readonly property int step3: 2

    property int currentState: 0
    property int currentStep: stepMarkerauto // 当前激活的步骤
    property var stepsModel: [["Step 1: Select Files", "Step 2: Choose Method"], ["Step 1: xxxx"]]

    // 注意：文件路径管理已迁移到 AppState.qml
    // 如果需要访问文件路径，请使用 AppState.initialMrcFile 等

    signal navigateToStep(int step);
    signal navigateToState(int state);

    // function completeStep0() {
    //     navigateTo(1);
    // }

    function navigateTo(stepIndex) {
        currentStep = stepIndex;
        navigateToStep(stepIndex);
    }

    function navigateStateTo(stateIndex) {
        currentState = stateIndex
        navigateToState(stateIndex)
        navigateTo(0);
    }

}
