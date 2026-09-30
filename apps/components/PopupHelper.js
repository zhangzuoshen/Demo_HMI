//========================================================================
// 弹窗调用辅助
//
// 作用域（Qt 文档 qtqml-javascript-resources.html）：
//   未声明 .pragma library 的 JS 资源属于 "code-behind"，与 import 它的
//   QML 组件共享作用域（所以这里可以直接访问上下文属性 PopupManager），
//   但**每个 import 处各一份独立副本**。
//
// ⚠ 因此本文件只能在 BasePage.qml 里 import 一次，业务页面调继承来的
//   openPopup()。不要自己 import 后再 PopupHelper.open() —— 那样注册与
//   派发会落在两个不同副本上，callback 永远收不到（已实测确认）。
//
//   若将来要允许任意文件 import，须改为 .pragma library（全局单副本），
//   但那样访问不到上下文属性 PopupManager（实测 ReferenceError），
//   需要额外注入 manager 实例（init(PopupManager)）。
//========================================================================

var _callbacks = ({})

function open(key, args, callback)
{
    var pid = PopupManager.open(key, args)

    // ★ 返回 0 = key 不存在，或被 Reject 策略拒绝。
    //   这两种情况都不会有 popupResult，此处不回调 callback 就会永久滞留
    if (pid === 0)
    {
        console.warn("[Popup] rejected:", key)

        if (callback)
            callback(null)

        return 0
    }

    if (callback)
        _callbacks[pid] = callback

    return pid
}

function dispatch(popupId, result)
{
    var cb = _callbacks[popupId]

    if (cb)
    {
        delete _callbacks[popupId]
        cb(result)
    }
}
