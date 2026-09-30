import QtQuick
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
            text: _cachedName
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

        function onStateChanged()
        {
            if (AppContext)
                syncState(AppContext.state)
        }
    }

    //==============================
    // 弹窗辅助
    //==============================
    // 没有拆成独立的 PopupHelper.js：.pragma library 的脚本是共享实例，
    // 访问不到调用方的 QML 上下文；.js 若不声明 pragma 则每个 import
    // 处各一份，回调注册与派发会落在不同副本上。放在基类里每个页面
    // 一份，业务页面经 QML 作用域链直接调用（如 Media/App.qml）。
    property var _popupCallbacks: ({})

    function openPopup(key, args, callback)
    {
        var pid = PopupManager.open(key, args)

        // ★ 返回 0 = key 不存在，或被 Reject 策略拒绝。
        //   这两种情况都不会有 popupResult，此处不回调就会永久滞留
        if (pid === 0)
        {
            console.warn("[Popup] rejected:", key)

            if (callback)
                callback(null)

            return 0
        }

        if (callback)
            _popupCallbacks[pid] = callback

        return pid
    }

    Connections {
        target: PopupManager

        function onPopupResult(popupId, result)
        {
            var cb = _popupCallbacks[popupId]

            if (cb)
            {
                delete _popupCallbacks[popupId]
                cb(result)
            }
        }
    }
}
