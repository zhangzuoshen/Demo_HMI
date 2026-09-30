import QtQuick
import HMI.Core 1.0

/*
 * 所有 Toast 的基类
 *
 * 与 BasePopup.qml 的三条关键差异（改这个文件时务必保留）：
 *
 * 1. rootItem **不是全屏** —— Toast 尺寸由内容决定，只占一小块，
 *    其余区域天然可点击穿透。所以这里没有遮罩，也没有 MouseArea。
 *
 * 2. 滑入滑出走 transform: Translate 的 y 偏移，**不直接改 item.y**。
 *    item.y 由 ToastContainer::relayout() 独占 —— 布局与动画都改 y 会打架。
 *
 * 3. 关闭同样要等退场动画：ToastContext.state 变 Closing 后播动画，
 *    onFinished 才调 notifyClosed()，由 C++ 真正销毁。
 *
 * ToastContext 的 null 保护照项目约定保留。
 */
Item {
    id: root

    opacity: 0

    property int animationDuration: 150

    // 滑入偏移，由 transform 消费；不要拿它去改 y
    property int slideOffset: 24

    default property alias content: contentArea.data

    transform: Translate {
        y: root.slideOffset
    }

    //==============================
    // 背景
    //==============================
    Rectangle {
        id: contentRect

        anchors.fill: parent
        radius: 6
        color: "#E6000000"
    }

    //==============================
    // 内容区（子类填充）
    //==============================
    Item {
        id: contentArea

        anchors.fill: parent
    }

    //==============================
    // 入场
    //==============================
    Component.onCompleted: {
        console.log("[Toast] Create:", ToastContext ? ToastContext.toastId : "?")
        enterAnim.start()
    }

    //==============================
    // 状态监听：C++ 置 Closing 后播退场动画
    //==============================
    Connections {
        target: ToastContext

        function onStateChanged()
        {
            if (ToastContext && ToastContext.state === OverlayState.Closing)
                exitAnim.start()
        }
    }

    //==============================
    // 动画
    //==============================
    ParallelAnimation {
        id: enterAnim

        NumberAnimation {
            target: root
            property: "opacity"
            from: 0
            to: 1
            duration: root.animationDuration
        }

        NumberAnimation {
            target: root
            property: "slideOffset"
            from: 24
            to: 0
            duration: root.animationDuration
        }
    }

    ParallelAnimation {
        id: exitAnim

        NumberAnimation {
            target: root
            property: "opacity"
            to: 0
            duration: root.animationDuration
        }

        NumberAnimation {
            target: root
            property: "slideOffset"
            to: 24
            duration: root.animationDuration
        }

        onFinished: {
            // ★ 动画播完才允许销毁
            if (ToastContext)
                ToastContext.notifyClosed()
        }
    }
}
