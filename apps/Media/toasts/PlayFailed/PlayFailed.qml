import QtQuick

import "../../../components"

/*
 * 示例：业务 Toast（Normal 优先级，collapse，preload）
 *
 * - duplicate = collapse：连续触发只保留一条，重置计时并更新文案
 *   （连点按钮就能看到计时器被不断刷新）
 * - preload = true：启动时预编译 QQmlComponent，首次 show 无卡顿
 * - 带 action：ToastContext.triggerAction("retry") → ToastManager.actionTriggered
 */
BaseToast {
    width: 360
    height: 56

    Row {
        anchors.centerIn: parent
        spacing: 12

        Text {
            anchors.verticalCenter: parent.verticalCenter

            // 绑定而非缓存：collapse 更新 args 后这里要跟着刷新
            text: ToastContext ? ToastContext.arg("message", "播放失败") : ""
            color: "white"
            font.pixelSize: 16
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter

            text: "重试"
            color: "#4DA3FF"
            font.pixelSize: 16
            visible: ToastContext ? ToastContext.arg("retryable", false) : false

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (ToastContext)
                        ToastContext.triggerAction("retry")
                }
            }
        }
    }
}
