import QtQuick 2.12
import "../components"

BasePage {
    backgroundColor: "#8B4513"

    //==============================
    // 生命周期
    //==============================
    function onCreate() {
        console.log("[Settings] Create")
    }

    function onEnter() {
        console.log("[Settings] Enter")
    }

    function onExit() {
        console.log("[Settings] Exit")
    }

    function onDestroy() {
        console.log("[Settings] Destroy")
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Settings Page"
            color: "white"
            font.pixelSize: 24
        }

        AppButton {
            text: "Open Settings Again"
            onClicked: {
                PageManager.launchApp("settings")
            }
        }

        AppButton {
            text: "Back"
            onClicked: {
                PageManager.back()
            }
        }
    }
}
