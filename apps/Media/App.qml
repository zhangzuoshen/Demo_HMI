import QtQuick
import "../components"

BasePage {
    backgroundColor: "#404040"

    //==============================
    // 生命周期
    //==============================
    onPageCreate: {console.log("[Media] Create")}
    onPageReady: {
        var args = AppContext.takeLaunchArgs()
        console.log("[Media] Ready:", args.times)
    }
    onPageEnter: {
        var args = AppContext.takeLaunchArgs()
        console.log("[Media] Enter:", args.times)
    }
    onPagePause:{
        var args = AppContext.takeLaunchArgs()
        console.log("[Media] Pause:", args.times)
    }
    onPageResume: {
        var args = AppContext.takeLaunchArgs()
        console.log("[Media] Resume:", args.times)
    }
    onPageDestroy: {console.log("[Media] Destroy")}

    Column {
        anchors.centerIn: parent
        spacing: 20

        TextInput {
            text: "Now Playing"
            color: "white"
            font.pixelSize: 26
        }

        Text {
            text: "Instance #" + _cachedInstance
            color: "#DDDDDD"
            font.pixelSize: 16
        }

        AppButton {
            text: "Open Settings"
            onClicked: {
                PageManager.launch("settings")
            }
        }

        AppButton {
            text: "Back Home"
            onClicked: {
                PageManager.launch("home")
            }
        }

        //==============================
        // 弹窗示例
        //==============================
        AppButton {
            text: "Track Info (modal)"
            onClicked: {
                openPopup("media/trackInfo",
                          { "title": "夜曲", "artist": "周杰伦" },
                          function(result) {
                              console.log("[Media] trackInfo result:", result)
                          })
            }
        }

        AppButton {
            text: "Thermal (preempt)"
            onClicked: {
                openPopup("system/thermalWarning",
                          { "message": "设备温度 92°C，即将自动关机" },
                          function(result) {
                              console.log("[Media] thermal result:", result)
                          })
            }
        }

        AppButton {
            text: "Hint (modeless)"
            onClicked: {
                openPopup("system/hintTip", { "text": "已添加到收藏" })
            }
        }
    }
}
