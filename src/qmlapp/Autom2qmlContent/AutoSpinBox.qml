import QtQuick
import QtQuick.Controls

SpinBox {
    id: control

    textFromValue: function(value, locale) {
        return value === -1 ? "Auto" : value.toString()
    }

    valueFromText: function(text, locale) {
        if (text === "Auto") {
            return -1
        }
        return Number.fromLocaleString(locale, text)
    }
}
