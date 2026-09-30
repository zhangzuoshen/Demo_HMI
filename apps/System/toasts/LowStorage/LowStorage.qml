import QtQuick

import "../../../components"

/*
 * 示例：抢占型 Toast（High 优先级 + preempt）
 *
 * 满位（maxVisible 默认 3）时，它会顶掉 active 中优先级**严格低于**
 * 80 的那条（取其中最低、最老的）。同优先级不干预。
 *
 * 验证方法：先连点几次 "Toast (collapse)" 把 3 个位置占满，
 * 再点 "Toast (preempt)"，会看到最下面那条被换掉。
 */
BaseToast {
    width: 400
    height: 56

    Row {
        anchors.centerIn: parent
        spacing: 12

        Text {
            anchors.verticalCenter: parent.verticalCenter

            text: "⚠ 存储空间不足 "
                  + (ToastContext ? ToastContext.arg("free", "?") : "")
            color: "#FFD666"
            font.pixelSize: 16
        }
    }
}
