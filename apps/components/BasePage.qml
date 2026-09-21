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

    //==============================
    // 内部状态
    //==============================
    property string _cachedName:""
    property int _cachedInstance:0
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
    // 生命周期状态同步
    //==============================
    function syncState(state) {

        switch (state) {

        case AppState.Ready:
            console.log("[Page]", _cachedName,
                        "#" + _cachedInstance,
                        "Ready")

            root.onReady()
            break

        case AppState.Foreground:

            if (lastState === AppState.Background) {

                console.log("[Page]", _cachedName,
                            "#" + _cachedInstance,
                            "Resume")

                root.onResume()

            } else {

                console.log("[Page]", _cachedName,
                            "#" + _cachedInstance,
                            "Enter")

                root.onEnter()

            }

            break

        case AppState.Background:

            console.log("[Page]", _cachedName,
                        "#" + _cachedInstance,
                        "Pause")

            root.onPause()
            break

        case AppState.Covered:

            console.log("[Page]", _cachedName,
                        "#" + _cachedInstance,
                        "Covered")
            break

        case AppState.Suspended:

            console.log("[Page]", _cachedName,
                        "#" + _cachedInstance,
                        "Suspended")
            break

        case AppState.Destroyed:
            break
        }

        lastState = state
    }

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
    Component.onCompleted:{

        if (AppContext) {

            _cachedName = AppContext.appName
            _cachedInstance = AppContext.instanceId

        }

        console.log("[Page]",
                    _cachedName,
                    "#" + _cachedInstance,
                    "Create")

        root.onCreate()

        // 修复首次 Ready/Foreground 丢失
        if (AppContext)
            syncState(AppContext.state)
    }

    Component.onDestruction:{

        console.log("[Page]",
                    _cachedName,
                    "#" + _cachedInstance,
                    "Destroy")

        root.onDestroy()
    }

    //==============================
    // 监听 AppContext 状态
    //==============================
    Connections {
        target: AppContext

        onStateChanged: {

            if (AppContext)
                syncState(AppContext.state)
        }
    }
}
