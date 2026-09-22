import QtQuick 2.12
import "../components"

BasePage {
    id: root

    backgroundColor: "#2E8B57"

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
            text: "Instance #" + AppContext.instanceId
            color: "#DDDDDD"
            font.pixelSize: 16
        }

        AppButton {
            text: "Open Media"

            onClicked: {
                PageManager.launch("media")
            }
        }

        AppButton {
            text: "Open Settings"

            onClicked: {
                PageManager.launch("settings")
            }
        }
    }

}
