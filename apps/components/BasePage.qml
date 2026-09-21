import QtQuick 2.12

import HMI.Core 1.0

Item {
    id: root

    anchors.fill: parent

    //==============================
    // 可配置属性
    //==============================
    property color backgroundColor: "#202020"
    property bool showTitleBar: true
    property int titleBarHeight: 50

    // 内容区域
    default property alias content: contentArea.data

    property int lastState:AppState.None

    //==============================
    // 生命周期（供子页面重写）
    //==============================
    function onCreate() {}
    function onReady() {}
    function onEnter() {}
    function onPause() {}
    function onResume() {}
    function onExit() {}
    function onDestroy() {}
    function onNewIntent() {}

    //==============================
    // 背景
    //==============================
    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    //==============================
    // 标题栏
    //==============================
    Rectangle {
        id: titleBar

        visible: root.showTitleBar
        anchors.top: parent.top
        width: parent.width
        height: root.titleBarHeight
        color: "#303030"

        Text {
            anchors.centerIn: parent
            text: AppContext.appName
            color: "white"
            font.pixelSize: 24
            font.bold: true
        }
    }

    //==============================
    // 内容区域
    //==============================
    Item {
        id: contentArea

        anchors {
            top: root.showTitleBar ? titleBar.bottom : parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }

    //==============================
    // 页面创建
    //==============================
    Component.onCompleted: {

        console.log(
                    "[Page]",
                    AppContext.appName,
                    "#" + AppContext.instanceId,
                    "Create")

        root.onCreate()
    }

    Component.onDestruction: {

        console.log(
                    "[Page]",
                    AppContext.appName,
                    "#" + AppContext.instanceId,
                    "Destroy")

        root.onDestroy()
    }

    //==============================
    // 生命周期监听
    //==============================
    Connections {
        target: AppContext
        onStateChanged:{
            switch(AppContext.state){
            case AppState.Ready:
                root.onReady()
                break
            case AppState.Foreground:
                if(lastState===AppState.Background)
                    root.onResume()
                else
                    root.onEnter()
                break
            case AppState.Background:
                root.onPause()
                break
            }
            lastState=AppContext.state
        }
    }

}
