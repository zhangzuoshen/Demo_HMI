#ifndef POPUPINSTANCE_H
#define POPUPINSTANCE_H

#include <QPointer>
#include <QVariantMap>
#include <QtGlobal>

#include "OverlayState.h"
#include "PopupRegistry.h"

class PopupContext;

/**
 * @brief 运行时弹窗实例
 *
 * 与 AppInstance 同构：按值拷贝、多处持有（Manager 的栈、View 各一份）。
 * context 用 QPointer —— 对象一旦销毁，所有副本自动置空，不会留下悬空指针。
 */
struct PopupInstance
{
    // 运行时唯一，每次 open() 递增
    quint64 popupId = 0;

    // "media/trackInfo"
    QString key;

    PopupInfo info;

    OverlayState::State state = OverlayState::None;

    // 宿主页面实例；0 = 系统级弹窗，不随页面销毁
    quint64 ownerInstanceId = 0;

    QVariantMap args;

    QPointer<PopupContext> context;
};

#endif // POPUPINSTANCE_H
