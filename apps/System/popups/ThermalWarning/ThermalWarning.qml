import QtQuick

import "../../../components"

/*
 * 示例：系统级强制弹窗（Critical 优先级 + preempt）
 *
 * - closeOnDimClick / closeOnBackKey 均为 false：只能点按钮关闭，
 *   返回键对它无效（PopupManager::closeTop 会跳过）。
 * - conflict = preempt：弹出时会关掉所有优先级严格低于 100 的弹窗。
 *   被顶掉的弹窗照样收到 popupResult（result 为 null），
 *   调用方 callback 不会泄漏。
 */
BasePopup {
    contentWidth: 480
    contentHeight: 240

    closeOnDimClick: false

    Rectangle {
        anchors.fill: parent

        radius: 8
        color: "#4A1A1A"
        border.color: "#FF5555"
        border.width: 2

        Column {
            anchors.centerIn: parent

            // 显式宽度：子项要用 horizontalCenter 锚点，
            // Column 若只有隐式宽度会形成绑定回环
            width: parent.width
            spacing: 18

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "⚠ 系统过热"
                color: "#FF6666"
                font.pixelSize: 26
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: PopupContext
                      ? PopupContext.arg("message", "设备温度过高，即将自动关机")
                      : ""
                color: "white"
                font.pixelSize: 15
            }

            AppButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "我知道了"
                onClicked: {
                    if (PopupContext)
                        PopupContext.close(true)
                }
            }
        }
    }
}
