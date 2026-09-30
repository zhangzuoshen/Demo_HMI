import QtQuick
import HMI.Core 1.0

/*
 * 所有弹窗的基类：遮罩、居中、进出场动画、生命周期
 *
 * 三条约定（改这个文件时务必保留）：
 *
 * 1. 关闭不能立刻销毁。PopupContext.close() 只把状态置为 Closing，
 *    这里监听到后播退场动画，动画结束才调 notifyClosed()，
 *    由 C++ 真正销毁并发 popupResult。直接 delete 会吃掉动画。
 *
 * 2. rootItem 是全屏的 —— 遮罩要盖住整个下层，所以内容尺寸走
 *    contentWidth / contentHeight，不要直接设 width / height。
 *
 * 3. 非模态弹窗靠 MouseArea.enabled = PopupContext.modal 让点击穿透，
 *    不能把 rootItem 缩小成内容区大小。
 *
 * PopupContext 的 null 保护照项目约定保留。
 */
Item {
    id: root

    anchors.fill: parent

    opacity: 0

    property bool dim: PopupContext ? PopupContext.dim : true
    property bool closeOnDimClick: true
    property int animationDuration: 180

    property int contentWidth: 420
    property int contentHeight: 220

    default property alias content: contentArea.data

    //==============================
    // 遮罩
    //==============================
    Rectangle {
        anchors.fill: parent
        color: "#80000000"
        visible: root.dim
    }

    //==============================
    // 输入拦截
    //==============================
    MouseArea {
        anchors.fill: parent

        // 非模态时 enabled=false，点击穿透到下层页面
        enabled: PopupContext ? PopupContext.modal : false

        onClicked: {
            if (root.closeOnDimClick && PopupContext)
                PopupContext.dismiss()
        }
    }

    //==============================
    // 内容区（子类填充，自身居中）
    //==============================
    Item {
        id: contentArea

        anchors.centerIn: parent
        width: root.contentWidth
        height: root.contentHeight
    }

    //==============================
    // 入场
    //==============================
    Component.onCompleted: {
        console.log("[Popup] Create:", PopupContext ? PopupContext.popupId : "?")
        enterAnim.start()
    }

    //==============================
    // 状态监听：C++ 置 Closing 后播退场动画
    //==============================
    Connections {
        target: PopupContext

        function onStateChanged()
        {
            if (PopupContext && PopupContext.state === OverlayState.Closing)
                exitAnim.start()
        }
    }

    //==============================
    // 动画
    //==============================
    NumberAnimation {
        id: enterAnim

        target: root
        property: "opacity"
        from: 0
        to: 1
        duration: root.animationDuration
    }

    NumberAnimation {
        id: exitAnim

        target: root
        property: "opacity"
        from: 1
        to: 0
        duration: root.animationDuration

        onFinished: {
            // ★ 动画播完才允许销毁
            if (PopupContext)
                PopupContext.notifyClosed()
        }
    }
}
