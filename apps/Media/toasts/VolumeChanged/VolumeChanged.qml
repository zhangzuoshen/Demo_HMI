import QtQuick

import "../../../components"

/*
 * 示例：collapse 型 Toast（Low 优先级，重复触发只保留一条）
 *
 * duplicate = collapse：同 key 再来一条时**重置计时器并更新 args**，
 * 不新增条目。典型场景是音量连按 —— 只显示一条，数字不断刷新。
 *
 * 所以这里的文案必须用绑定（ToastContext.arg(...)）而不是在
 * Component.onCompleted 里缓存一次，否则刷新后看不到新值。
 */
BaseToast {
    width: 260
    height: 56

    Row {
        anchors.centerIn: parent
        spacing: 10

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "🔊"
            font.pixelSize: 18
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: "音量 " + (ToastContext ? ToastContext.arg("value", 0) : 0)
            color: "white"
            font.pixelSize: 16
        }
    }
}
