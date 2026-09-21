import QtQuick 2.12
import QtQuick.Controls 2.12
import "../components"

BasePage {
    id: root

    backgroundColor: "#2E8B57"

    //==============================
    // 生命周期
    //==============================
    function onCreate(){console.log("[Home] Create")}
    function onReady(){console.log("[Home] Ready")}
    function onEnter(){console.log("[Home] Enter")}
    function onPause(){console.log("[Home] Pause")}
    function onResume(){console.log("[Home] Resume")}
    function onDestroy(){console.log("[Home] Destroy")}

    //==============================
    // 页面内容
    //==============================
    Column {
        anchors.centerIn: parent
        spacing: 24

        Text {
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

        Slider {

        }

        AppButton {
            text: "Open Media"

            onClicked: {
                PageManager.launchApp("media")
            }
        }

        AppButton {
            text: "Open Settings"

            onClicked: {
                PageManager.launchApp("settings")
            }
        }
    }

}
