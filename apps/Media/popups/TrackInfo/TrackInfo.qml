import QtQuick

import "../../../components"

/*
 * 示例：业务弹窗（模态，Normal 优先级，stack）
 *
 * 取参数一律走 arg(key, default)，不要直接写 PopupContext.args.title
 * —— 缺字段时会拿到 undefined 而不是 undefined 安全值。
 */
BasePopup {
    contentWidth: 420
    contentHeight: 240

    Rectangle {
        anchors.fill: parent

        radius: 8
        color: "#2A2A2A"
        border.color: "#555555"
        border.width: 1

        Column {
            anchors.centerIn: parent

            // 显式宽度：子项要用 horizontalCenter 锚点，
            // Column 若只有隐式宽度会形成绑定回环
            width: parent.width
            spacing: 18

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: PopupContext ? PopupContext.arg("title", "音轨信息") : ""
                color: "white"
                font.pixelSize: 22
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: PopupContext ? PopupContext.arg("artist", "未知艺术家") : ""
                color: "#AAAAAA"
                font.pixelSize: 15
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20

                AppButton {
                    text: "确定"
                    onClicked: {
                        // 结果回传给 openPopup 的 callback
                        if (PopupContext)
                            PopupContext.close(true)
                    }
                }

                AppButton {
                    text: "取消"
                    onClicked: {
                        if (PopupContext)
                            PopupContext.dismiss()
                    }
                }
            }
        }
    }
}
