import QtQuick 2.12

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
                    "Ready")

        root.onCreate()
        root.onReady()
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

        target: PageManager

        onAppEntered: {

            if(instanceId === AppContext.instanceId)
            {
                console.log(
                            "[Page]",
                            AppContext.appName,
                            "#" + AppContext.instanceId,
                            "Enter")

                root.onEnter()
            }

        }

        onAppPaused: {

            if(instanceId === AppContext.instanceId)
            {
                console.log(
                            "[Page]",
                            AppContext.appName,
                            "#" + AppContext.instanceId,
                            "Pause")

                root.onPause()
            }

        }

        onAppResumed: {

            if(instanceId === AppContext.instanceId)
            {
                console.log(
                            "[Page]",
                            AppContext.appName,
                            "#" + AppContext.instanceId,
                            "Resume")

                root.onResume()
            }

        }

        onAppExited: {

            if(instanceId === AppContext.instanceId)
            {
                console.log(
                            "[Page]",
                            AppContext.appName,
                            "#" + AppContext.instanceId,
                            "Exit")

                root.onExit()
            }

        }

        onAppNewIntent: {

            if(instanceId === AppContext.instanceId)
            {
                console.log(
                            "[Page]",
                            AppContext.appName,
                            "#" + AppContext.instanceId,
                            "NewIntent")

                root.onNewIntent()
            }

        }

    }

}
