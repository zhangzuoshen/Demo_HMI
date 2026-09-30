import QtQuick

/*
 * 示例：非模态轻提示（Low 优先级 + reject）
 *
 * - kind = modeless：BasePopup 的 MouseArea 自动 enabled=false，
 *   点击穿透到下层页面，不打断用户操作。
 * - conflict = reject：栈里已有优先级 >= 10 的弹窗时，open() 直接返回 0，
 *   不会弹出也不会有 popupResult —— 调用方的 openPopup 会立即以 null 回调。
 * - 自行用 Timer 关闭：PopupContext.dismiss() 等价 close(false)。
 */
BasePopup {
    contentWidth: 320
    contentHeight: 72

    closeOnDimClick: false

    Rectangle {
        anchors.fill: parent

        radius: 6
        color: "#CC1E1E1E"
        border.color: "#666666"
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: PopupContext ? PopupContext.arg("text", "操作已完成") : ""
            color: "white"
            font.pixelSize: 16
        }
    }

    Timer {
        interval: PopupContext ? PopupContext.arg("timeout", 2000) : 2000
        repeat: false
        running: true
        onTriggered: {
            if (PopupContext)
                PopupContext.dismiss()
        }
    }
}
