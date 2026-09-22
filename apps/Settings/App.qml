import QtQuick 2.12
import "../components"

BasePage {
    backgroundColor: "#8B4513"

    //==============================
    // 生命周期
    //==============================
    onPageCreate: {console.log("[Settings] Create")}
    onPageReady: {console.log("[Settings] Ready")}
    onPageEnter: {console.log("[Settings] Enter")}
    onPagePause: {console.log("[Settings] Pause")}
    onPageResume: {console.log("[Settings] Resume")}
    onPageDestroy: {console.log("[Settings] Destroy")}

    HeavyTestPage {

    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        TextInput {
            text: "Settings Page"
            color: "white"
            font.pixelSize: 24
        }

        Text {
            text: "Instance #" + AppContext.instanceId
            color: "#DDDDDD"
            font.pixelSize: 16
        }

        AppButton {
            text: "Open Settings Again"
            onClicked: {
                PageManager.launch("settings")
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
