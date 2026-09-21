import QtQuick 2.12
import "../components"

BasePage {
    id: root

    backgroundColor: "#2E8B57"

    //==============================
    // 生命周期
    //==============================
    function onCreate() {
        console.log("[Home] Create")
    }

    function onReady() {
        console.log("[Home] Ready")
    }

    function onEnter() {
        console.log("[Home] Enter")
    }

    function onPause() {
        console.log("[Home] Pause")
    }

    function onResume() {
        console.log("[Home] Resume")
    }

    function onExit() {
        console.log("[Home] Exit")
    }

    function onDestroy() {
        console.log("[Home] Destroy")
    }

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

    //==============================
    // 调试信息（可删除）
    //==============================
    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 12

        width: 220
        height: 80

        radius: 4
        color: "#66000000"

        Column {
            anchors.centerIn: parent
            spacing: 2

            Text {
                text: "App: " + AppContext.appName
                color: "white"
                font.pixelSize: 14
            }

            Text {
                text: "ID: " + AppContext.appId
                color: "#CCCCCC"
                font.pixelSize: 12
            }

            Text {
                text: "Mode: " + AppContext.launchMode
                color: "#CCCCCC"
                font.pixelSize: 12
            }
        }
    }
}
