#ifndef APPINSTANCE_H
#define APPINSTANCE_H

#include <QtGlobal>

#include "AppRegistry.h"
#include "AppState.h"

class AppContext;

struct AppInstance
{
    quint64 instanceId = 0;

    bool firstLaunch = true;

    AppInfo info;

    AppState::State state = AppState::None;

    AppContext *context = nullptr;
};

#endif
