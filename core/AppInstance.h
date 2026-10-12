#ifndef APPINSTANCE_H
#define APPINSTANCE_H

#include <QPointer>
#include <QtGlobal>

#include "AppRegistry.h"
#include "AppState.h"

class AppContext;

/**
 * @brief 运行时应用实例
 *
 * AppInfo   : Manifest中的静态信息
 * AppContext: 每个实例独立拥有的QML上下文
 * AppState  : 当前运行状态
 */
struct AppInstance
{
    quint64 instanceId = 0;

    bool firstLaunch = true;

    // 页面是否等待首次Ready
    bool waitingForReady = false;

    AppInfo info;

    AppState::State state = AppState::None;

    // 每个实例独立拥有。
    // AppInstance 按值拷贝、多处持有，故用 QPointer：
    // AppContext 一旦销毁，所有副本自动置空，不会留下悬空指针。
    QPointer<AppContext> context;
};

#endif // APPINSTANCE_H
