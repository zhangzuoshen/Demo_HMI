#ifndef APPINSTANCE_H
#define APPINSTANCE_H

#include <QtGlobal>
#include "AppRegistry.h"

struct AppInstance
{
    quint64 instanceId = 0;
    bool firstLaunch = true;
    AppInfo info;
};

#endif
