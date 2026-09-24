import QtQuick 2.12
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
