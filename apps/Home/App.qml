import QtQuick
import "../components"

BasePage {
    id: root

    backgroundColor: "#2E8B57"

    // sticky Toast 的 id，用于拔出时 dismiss
    property int _usbToast: 0

    //==============================
    // 生命周期
    //==============================
    onPageCreate: {console.log("[Home] Create")}
    onPageReady: {console.log("[Home] Ready")}
    onPageEnter: {console.log("[Home] Enter")}
    onPagePause: {console.log("[Home] Pause")}
    onPageResume: {console.log("[Home] Resume")}
    onPageDestroy: {console.log("[Home] Destroy")}

    //==============================
    // 页面内容
    //==============================
    Column {
        anchors.centerIn: parent
        spacing: 24

        TextInput {
            text: "Welcome Home"
            color: "white"
            font.pixelSize: 30
            font.bold: true
        }

        Text {
            text: "Instance #" + _cachedInstance
            color: "#DDDDDD"
            font.pixelSize: 16
        }

        AppButton {
            text: "Open Media"

            onClicked: {
                var t = new Date().toTimeString()
                PageManager.launch("media", {times: t})
                console.log("times:", t)
            }
        }

        AppButton {
            text: "Open Settings"

            onClicked: {
                PageManager.launch("settings")
            }
        }
    }

    //==============================
    // Toast 示例
    //==============================
    // 横向排列：上面的 Column 已接近一屏，竖着加会溢出
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        spacing: 12

        AppButton {
            text: "Toast"
            onClicked: {
                ToastManager.show("media/playFailed",
                                  { "message": "无法解码该音轨",
                                    "retryable": true })
            }
        }

        AppButton {
            text: "Collapse"
            onClicked: {
                ToastManager.show("media/volumeChanged",
                                  { "value": Math.floor(Math.random() * 100) })
            }
        }

        AppButton {
            text: _usbToast ? "USB 断开" : "USB 连接"
            onClicked: {
                if (_usbToast)
                {
                    ToastManager.dismiss(_usbToast)
                    _usbToast = 0
                }
                else
                {
                    _usbToast = ToastManager.show("system/usbConnected",
                                                  { "device": "U盘" })
                }
            }
        }

        AppButton {
            text: "Preempt"
            onClicked: {
                ToastManager.show("system/lowStorage", { "free": "120MB" })
            }
        }
    }

    //==============================
    // Toast 上的 action（如"重试"）
    //==============================
    Connections {
        target: ToastManager

        function onActionTriggered(toastId, name, data)
        {
            console.log("[Home] toast action:", toastId, name, data)
        }
    }
}
