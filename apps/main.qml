import QtQuick 2.12
import QtQuick.Window 2.12
import HMI.Core 1.0

Window {
    id: root

    visible: true
    width: 800
    height: 480
    color: "#000000"
    title: "Demo HMI"

    /*
     * Scene 层级（由下到上）
     * 1. AppScene      ：普通应用(Home/Media/Settings)
     * 2. OverlayScene  ：CarPlay、Camera 等抢占层
     * 3. PopupScene    ：Dialog、Toast
     * 4. DebugOverlay  ：调试信息
     */

    //==============================
    // App Scene
    //==============================
    Item {
        anchors.fill: parent
        focus: true
        //==============================
        // 全局返回键
        //==============================
        Keys.onReleased: {
            switch(event.key)
            {
            case Qt.Key_Back:
            case Qt.Key_Escape:
                PageManager.back()
                event.accepted = true
                break
            }
        }

        SceneContainer {
            id: appScene
            anchors.fill: parent
            pageManager: PageManager
            z: 0
        }
    }

    //==============================
    // Overlay Scene（预留）
    //==============================
    Item {
        id: overlayScene
        anchors.fill: parent
        z: 100
        visible: false
    }

    //==============================
    // Popup Scene（预留）
    //==============================
    Item {
        id: popupScene
        anchors.fill: parent
        z: 200
    }

    //==============================
    // 调试信息
    //==============================
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        width: 220
        height: 64
        radius: 4
        color: "#66000000"
        z: 999

        Column {
            anchors.centerIn: parent
            spacing: 2

            Text {
                text: "App : " + PageManager.currentAppId
                color: "white"
                font.pixelSize: 14
            }

            Text {
                text: Qt.application.version
                color: "#CCCCCC"
                font.pixelSize: 12
            }
        }
    }

    //==============================
    // 启动日志
    //==============================
    Component.onCompleted: {
        console.log("[Main] Window Ready")
    }
}
