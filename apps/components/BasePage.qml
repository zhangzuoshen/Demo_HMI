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
    property string _cachedName: ""
    property int _cachedInstance: 0
    property int lastState:AppState.None

    //==============================
    // 生命周期（Qt Signal）
    //==============================
    signal pageCreate()
    signal pageReady()
    signal pageEnter()
    signal pagePause()
    signal pageResume()
    signal pageCovered()
    signal pageSuspend()
    signal pageDestroy()

    signal windowAttached()
    signal windowDetached()
    signal windowVisible()


    //==============================
    // 生命周期状态同步
    //==============================
    function syncState(state)
    {
        var previous = lastState
        lastState = state

        switch(state)
        {
        case AppState.Ready:
            console.log("[Page]",_cachedName,"#"+_cachedInstance,"Ready")
            pageReady()
            break

        case AppState.Foreground:

            if(previous===AppState.Background)
            {
                console.log("[Page]",_cachedName,"#"+_cachedInstance,"Resume")
                pageResume()
            }
            else
            {
                console.log("[Page]",_cachedName,"#"+_cachedInstance,"Enter")
                pageEnter()
            }
            break

        case AppState.Background:
            console.log("[Page]",_cachedName,"#"+_cachedInstance,"Pause")
            pagePause()
            break

        case AppState.Covered:
            console.log("[Page]",_cachedName,"#"+_cachedInstance,"Covered")
            pageCovered()
            break

        case AppState.Suspended:
            console.log("[Page]",_cachedName,"#"+_cachedInstance,"Suspended")
            pageSuspend()
            break
        }
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
        if(AppContext)
        {
            _cachedName=AppContext.appName
            _cachedInstance=AppContext.instanceId
        }

        console.log("[Page]",_cachedName,"#"+_cachedInstance,"Create")

        pageCreate()

        if(AppContext)
            syncState(AppContext.state)
    }

    //==========================
    // 页面销毁
    //==========================
    Component.onDestruction:{
        console.log("[Page]",_cachedName,"#"+_cachedInstance,"Destroy")
        pageDestroy()
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
