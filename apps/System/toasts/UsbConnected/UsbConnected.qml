import QtQuick

/*
 * 示例：sticky Toast（duration = 0）
 *
 * duration 为 0 表示不会自动消失，只能代码 dismiss —— 典型场景是
 * "USB 已连接"这类与硬件状态绑定的提示，拔出时再关掉。
 *
 * 注意：sticky **不豁免**被抢占。"USB 已连接"该被"系统过热"顶掉。
 * 真要免疫就把 priority 提到 Critical(100)，不额外加 noPreempt 字段。
 */
BaseToast {
    width: 320
    height: 56

    Row {
        anchors.centerIn: parent
        spacing: 12

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: ToastContext
                  ? "已连接 " + ToastContext.arg("device", "U盘")
                  : ""
            color: "white"
            font.pixelSize: 16
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter

            text: "断开"
            color: "#FF8080"
            font.pixelSize: 15

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (ToastContext)
                        ToastContext.dismiss()
                }
            }
        }
    }
}
