import QtQuick
import HMI.Apps

// ============================================================================
// StatusBar：常驻顶部状态栏 + 下拉「负一屏」面板
//
// 层级：在 main.qml 中由 statusBarScene(z:250) 承载。
//   - 状态栏常驻可见（盖在 App/Overlay/Popup 之上，Toast 之下）
//   - 从状态栏「下拉」展开面板，「上推」或点遮罩收起；点状态栏切换
//
// 控件全部自绘，不依赖 QtQuick.Controls，便于跨平台部署。
// ============================================================================

Item {
    id: root
    anchors.fill: parent

    //==============================
    // 配置
    //==============================
    property int barHeight: 34
    property real shadeHeight: Math.min(parent.height * 0.82, 600)
    property bool expanded: false

    // 面板纵向位置：-shadeHeight 收起（藏在屏幕上方）.. 0 完全展开
    property real panelY: -shadeHeight

    // 亮度（演示用，仅本地状态）
    property int brightness: 70

    //==============================
    // 遮罩：展开后点空白收起
    //==============================
    Rectangle {
        id: scrim
        anchors.fill: parent
        color: "#000000"
        opacity: 0
        visible: opacity > 0.001
        z: 0

        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    //==============================
    // 下拉面板（负一屏主体）
    //==============================
    Rectangle {
        id: shade
        width: parent.width
        height: root.shadeHeight
        y: root.panelY
        color: "#242424"
        z: 1

        Column {
            anchors.fill: parent
            anchors.topMargin: root.barHeight + 12
            anchors.bottomMargin: 16
            padding: 20
            spacing: 18

            //===== 大时钟 + 日期 =====
            Item {
                width: parent.width - 40
                height: 86

                Text {
                    id: bigClock
                    text: Qt.formatDateTime(new Date(), "hh:mm")
                    color: "white"
                    font.pixelSize: 64
                    font.bold: true
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    Timer {
                        interval: 1000
                        repeat: true
                        running: true
                        onTriggered: {
                            bigClock.text = Qt.formatDateTime(new Date(), "hh:mm")
                            dateText.text = Qt.formatDate(new Date(), "yyyy年M月d日 dddd")
                        }
                    }
                }

                Text {
                    id: dateText
                    text: Qt.formatDate(new Date(), "yyyy年M月d日 dddd")
                    color: "#BBBBBB"
                    font.pixelSize: 18
                    anchors.left: parent.left
                    anchors.top: bigClock.bottom
                    anchors.topMargin: 2
                }
            }

            //===== 快捷开关 =====
            Row {
                width: parent.width - 40
                spacing: 14

                Repeater {
                    model: [
                        { label: "WLAN" },
                        { label: "蓝牙" },
                        { label: "飞行" },
                        { label: "免打扰" }
                    ]
                    Rectangle {
                        width: 72; height: 72; radius: 12
                        color: toggled ? "#4FA3FF" : "#3A3A3A"
                        property bool toggled: false

                        Text {
                            text: modelData.label
                            color: toggled ? "white" : "#CCCCCC"
                            font.pixelSize: 14
                            anchors.centerIn: parent
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: parent.toggled = !parent.toggled
                        }
                    }
                }
            }

            //===== 亮度调节（简易滑块） =====
            Item {
                width: parent.width - 40
                height: 40

                Text {
                    text: "亮度"
                    color: "#CCCCCC"
                    font.pixelSize: 16
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    id: brightTrack
                    x: 90
                    width: parent.width - 90 - 60
                    height: 6
                    radius: 3
                    color: "#555555"
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        width: parent.width * (brightness / 100)
                        height: parent.height
                        radius: 3
                        color: "#4FA3FF"
                    }
                    Rectangle {
                        width: 18; height: 18; radius: 9
                        color: "white"
                        x: (parent.width - 18) * (brightness / 100)
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    MouseArea {
                        anchors.fill: parent
                        onPositionChanged: {
                            if (pressed) {
                                var ratio = Math.max(0, Math.min(1, mouseX / parent.width))
                                brightness = Math.round(ratio * 100)
                            }
                        }
                    }
                }

                Text {
                    text: brightness + "%"
                    color: "#CCCCCC"
                    font.pixelSize: 16
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            //===== 通知列表 =====
            Column {
                width: parent.width - 40
                spacing: 10

                Text {
                    text: "通知"
                    color: "#FFFFFF"
                    font.pixelSize: 18
                    font.bold: true
                }

                Repeater {
                    model: [
                        { title: "系统更新", body: "新版本 v1.2.0 已就绪", time: "刚刚" },
                        { title: "USB 设备", body: "U盘 已安全弹出", time: "5 分钟前" },
                        { title: "媒体", body: "当前播放：未知音轨", time: "12 分钟前" }
                    ]
                    Rectangle {
                        width: parent.width
                        height: 64
                        radius: 8
                        color: "#333333"

                        Text {
                            text: modelData.title
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            x: 14; y: 10
                        }
                        Text {
                            text: modelData.body
                            color: "#BBBBBB"
                            font.pixelSize: 14
                            x: 14; y: 34
                        }
                        Text {
                            text: modelData.time
                            color: "#888888"
                            font.pixelSize: 12
                            anchors.right: parent.right
                            anchors.rightMargin: 12
                            anchors.top: parent.top
                            anchors.topMargin: 10
                        }
                    }
                }
            }
        }
    }

    //==============================
    // 常驻顶部状态栏
    //==============================
    Rectangle {
        id: bar
        width: parent.width
        height: root.barHeight
        color: "#141414"
        z: 2

        // 时间（左）
        Text {
            id: barClock
            text: Qt.formatDateTime(new Date(), "hh:mm")
            color: "white"
            font.pixelSize: 15
            anchors.left: parent.left
            anchors.leftMargin: 14
            anchors.verticalCenter: parent.verticalCenter

            Timer {
                interval: 1000
                repeat: true
                running: true
                onTriggered: barClock.text = Qt.formatDateTime(new Date(), "hh:mm")
            }
        }

        // 状态图标（右）
        Row {
            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Text { text: "WLAN"; color: "#CCCCCC"; font.pixelSize: 13 }
            Text { text: "BT";   color: "#CCCCCC"; font.pixelSize: 13 }

            // 信号格（自绘）
            Row {
                spacing: 2
                anchors.verticalCenter: parent.verticalCenter
                Repeater {
                    model: 4
                    Rectangle {
                        width: 4; height: 6 + index * 4
                        radius: 1
                        color: index < 3 ? "#CCCCCC" : "#555555"
                    }
                }
            }

            Text { text: "100%"; color: "#CCCCCC"; font.pixelSize: 13 }
        }

        // 下拉把手提示（自绘，避免字体依赖）
        Rectangle {
            width: 28; height: 4; radius: 2
            color: "#666666"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        // 手势：下拉展开 / 上推收起 / 点击切换
        MouseArea {
            anchors.fill: parent
            property real startMouseY: 0
            property real startPanelY: 0
            property bool dragged: false

            onPressed: {
                startMouseY = mouseY
                startPanelY = root.panelY
                dragged = false
            }
            onPositionChanged: {
                if (pressed) {
                    var d = mouseY - startMouseY
                    if (Math.abs(d) > 6) dragged = true
                    var t = startPanelY + d
                    root.panelY = Math.max(-root.shadeHeight, Math.min(0, t))
                }
            }
            onReleased: {
                if (dragged) {
                    if (root.panelY > -root.shadeHeight * 0.5) root.open()
                    else root.close()
                }
            }
            onClicked: {
                if (!dragged) root.toggle()
            }
        }
    }

    //==============================
    // 控制逻辑
    //==============================
    function open()  { expanded = true;  animTo(0) }
    function close() { expanded = false; animTo(-shadeHeight) }
    function toggle(){
        if (expanded) {
            close()
        } else {
            open()
        }
    }

    function animTo(targetY) {
        panelAnim.from = root.panelY
        panelAnim.to = targetY
        panelAnim.running = true
    }

    NumberAnimation {
        id: panelAnim
        target: root
        property: "panelY"
        duration: 220
        easing.type: Easing.OutCubic
    }

    // 遮罩透明度随面板位置：完全展开=1，收起=0
    Binding {
        target: scrim
        property: "opacity"
        value: root.shadeHeight > 0
               ? 1.0 - (-root.panelY) / root.shadeHeight
               : 0
    }

    Component.onCompleted: console.log("[StatusBar] ready")
}
