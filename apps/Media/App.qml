import QtQuick 2.12
import "../components"

BasePage {
    backgroundColor: "#404040"

    //==============================
    // 生命周期
    //==============================
    onPageCreate: {console.log("[Media] Create")}
    onPageReady: {console.log("[Media] Ready")}
    onPageEnter: {console.log("[Media] Enter")}
    onPagePause: {console.log("[Media] Pause")}
    onPageResume: {console.log("[Media] Resume")}
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
            text: "Instance #" + AppContext.instanceId
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
    }
}
