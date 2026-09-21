#ifndef LOG_H
#define LOG_H

#include <QLoggingCategory>

// 日志分类
Q_DECLARE_LOGGING_CATEGORY(logAppRegistry)
Q_DECLARE_LOGGING_CATEGORY(logPageManager)
Q_DECLARE_LOGGING_CATEGORY(logSceneContainer)
Q_DECLARE_LOGGING_CATEGORY(logQml)

// 初始化日志系统
void initLogSystem();

#endif
