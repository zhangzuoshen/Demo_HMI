#ifndef APPINSTANCE_H
#define APPINSTANCE_H

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

    // 每个实例独立拥有
    AppContext *context = nullptr;
};

#endif // APPINSTANCE_H
