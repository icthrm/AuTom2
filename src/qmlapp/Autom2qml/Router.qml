pragma Singleton

import QtQuick

QtObject {
    id: root

    // ==================== 路由表定义 ====================

    readonly property var routes: [
        // State 0: 完整工作流
        {
            stateName: "Processing Workflow",
            steps: [
                {
                    id: "file-selection",
                    title: "Step 1: Select Files",
                    screen: "Screen01.ui.qml",
                    icon: "📁",
                    showCommandView: false
                },
                {
                    id: "method-selection",
                    title: "Step 2: Select Method",
                    screen: "Screen02.ui.qml",
                    icon: "🔀",
                    showCommandView: false
                },
                {
                    id: "reconstruction",
                    title: "Step 3: 3D Reconstruction",
                    screen: "Screen03.ui.qml",
                    icon: "🧊",
                    showCommandView: true
                },
                {
                    id: "summary",
                    title: "Step 4: Summary",
                    screen: "Screen07_Summary.ui.qml",
                    icon: "✅",
                    showCommandView: false
                }
            ]
        },

        // State 1: Markerfree 工作流
        {
            stateName: "Markerfree Alignment",
            steps: [
                {
                    id: "markerfree",
                    title: "Step 3: Markerfree",
                    screen: "Screen04_Markerfree.ui.qml",
                    icon: "⚡",
                    showCommandView: true
                }
            ]
        },

        // State 2: Markerauto2 工作流
        {
            stateName: "Markerauto Alignment",
            steps: [
                {
                    id: "markerauto2",
                    title: "Step 3: Markerauto",
                    screen: "Screen05_MarkerAuto2.ui.qml",
                    icon: "🎯",
                    showCommandView: true
                },
                {
                    id: "trace-compare",
                    title: "Trace Comparison",
                    screen: "ScreenTraceCompare.ui.qml",
                    icon: "📊",
                    showCommandView: false
                }
            ]
        },

        // State 3: 批量处理工作流
        {
            stateName: "Batch Processing",
            steps: [
                {
                    id: "batch-processing",
                    title: "Batch Processing",
                    screen: "Screen_Batch.ui.qml",
                    icon: "📦",
                    showCommandView: false
                }
            ]
        }
    ]

    // ==================== 导航状态 ====================

    property int currentStateIndex: 0
    property int currentStepIndex: 0

    // 当前路由信息（只读）
    readonly property var currentState: routes[currentStateIndex]
    readonly property var currentStep: currentState.steps[currentStepIndex]
    readonly property string currentScreen: currentStep.screen

    // 侧边栏标题列表（动态生成）
    readonly property var sidebarTitles: currentState.steps.map(step => step.title)

    // 是否可以返回上一个 State
    readonly property bool canGoBackState: currentStateIndex > 0

    // 当前页面是否显示 CommandView
    readonly property bool shouldShowCommandView: currentStep.showCommandView !== undefined
                                                   ? currentStep.showCommandView
                                                   : true  // 默认显示

    // ==================== 导航信号 ====================

    signal navigated(string screenPath)
    signal stateChanged(int stateIndex)

    // ==================== 公共方法 ====================

    /**
     * 导航到指定步骤
     * @param stepIndex - 目标步骤索引（在当前 state 内）
     */
    function navigateToStep(stepIndex) {
        if (stepIndex < 0 || stepIndex >= currentState.steps.length) {
            console.warn("Invalid step index:", stepIndex)
            return
        }

        currentStepIndex = stepIndex
        navigated(currentStep.screen)
    }

    /**
     * 导航到指定状态的第一步
     * @param stateIndex - 目标状态索引
     */
    function navigateToState(stateIndex) {
        if (stateIndex < 0 || stateIndex >= routes.length) {
            console.warn("Invalid state index:", stateIndex)
            return
        }

        currentStateIndex = stateIndex
        currentStepIndex = 0
        stateChanged(stateIndex)
        navigated(currentStep.screen)
    }

    /**
     * 返回上一个状态（回到 State 0 的 Step 1）
     */
    function goBackState() {
        if (!canGoBackState) {
            console.warn("Already at first state")
            return
        }
        navigateToState(0)
        navigateToStep(1)  // 直接到 Step 2
    }

    /**
     * 根据 stepId 查找并导航
     * @param stepId - 步骤唯一 ID（如 "file-selection"）
     */
    function navigateToStepById(stepId) {

        // 其他步骤在当前路由表中查找
        for (let stateIdx = 0; stateIdx < routes.length; stateIdx++) {
            const state = routes[stateIdx]
            const stepIdx = state.steps.findIndex(s => s.id === stepId)
            if (stepIdx !== -1) {
                currentStateIndex = stateIdx
                navigateToStep(stepIdx)
                return
            }
        }
        console.warn("Step not found:", stepId)
    }

    /**
     * 获取下一步信息（用于"下一步"按钮）
     */
    function getNextStep() {
        if (currentStepIndex + 1 < currentState.steps.length) {
            return {
                exists: true,
                title: currentState.steps[currentStepIndex + 1].title
            }
        }
        return { exists: false }
    }
}
