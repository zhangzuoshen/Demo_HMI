import QtQuick 2.12
import "../components"

BasePage {
    backgroundColor: "#404040"

    //==============================
    // 生命周期
    //==============================
    function onCreate(){console.log("[Media] Create")}
    function onReady(){console.log("[Media] Ready")}
    function onEnter(){console.log("[Media] Enter")}
    function onPause(){console.log("[Media] Pause")}
    function onResume(){console.log("[Media] Resume")}
    function onDestroy(){console.log("[Media] Destroy")}

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Now Playing"
            color: "white"
            font.pixelSize: 26
        }

        AppButton {
            text: "Open Settings"
            onClicked: {
                PageManager.launchApp("settings")
            }
        }

        AppButton {
            text: "Back Home"
            onClicked: {
                PageManager.launchApp("home")
            }
        }
    }
}
