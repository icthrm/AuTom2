import QtQuick
import QtQuick.Controls

// 这是一个常规的 .qml 组件, 封装了处理小数的逻辑
SpinBox {
    id: root

    editable: true

    // 暴露给外部使用的属性
    property int decimals: 2
    
    // 真实的浮点数值（可读写）
    property real realValue: fromValue
    
    // 标记是否已初始化完成
    property bool _initialized: false

    // 将 from 和 to 的类型改为 real 以便外部可以设置小数
    property real fromValue: 0.0
    property real toValue: 100.0
    property real stepValue: 0.1

    // 内部逻辑对象
    QtObject {
        id: internalProps
        readonly property int decimalFactor: Math.pow(10, root.decimals)
    }

    function decimalToReal(val) {
        return val / internalProps.decimalFactor
    }

    function realToDecimal(real) {
        return Math.round(real * internalProps.decimalFactor)
    }

    // 初始化时，将外部的浮点数属性转换为内部的整数属性
    Component.onCompleted: {
        // 必须先设置 from/to，再设置 value，否则会被 clamp
        root.from = realToDecimal(fromValue)
        root.to = realToDecimal(toValue)
        root.stepSize = realToDecimal(stepValue)
        // 使用 realValue 初始化
        root.value = realToDecimal(realValue)
        // 标记初始化完成
        root._initialized = true
    }

    // 当内部 value 变化时，更新 realValue
    onValueChanged: {
        if (!_initialized) return
        var newRealValue = decimalToReal(value)
        if (Math.abs(realValue - newRealValue) > 0.0001) {
            realValue = newRealValue
        }
    }

    // 当外部 realValue 变化时，更新内部 value
    onRealValueChanged: {
        if (!_initialized) return
        var targetValue = realToDecimal(realValue)
        if (value !== targetValue) {
            value = targetValue
        }
    }

    validator: DoubleValidator {
        bottom: root.fromValue
        top: root.toValue
        decimals: root.decimals
        notation: DoubleValidator.StandardNotation
    }

    textFromValue: function(value, locale) {
        return Number(decimalToReal(value)).toLocaleString(locale, 'f', root.decimals)
    }

    valueFromText: function(text, locale) {
        return realToDecimal(Number.fromLocaleString(locale, text))
    }
}
