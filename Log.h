#ifndef LOG_H
#define LOG_H

#include <QLoggingCategory>

// 日志分类
Q_DECLARE_LOGGING_CATEGORY(logMain)
Q_DECLARE_LOGGING_CATEGORY(logBootstrap)
Q_DECLARE_LOGGING_CATEGORY(logPageManager)
Q_DECLARE_LOGGING_CATEGORY(logScene)
Q_DECLARE_LOGGING_CATEGORY(logRegistry)
Q_DECLARE_LOGGING_CATEGORY(logQml)

// 初始化日志系统
void initLogSystem();

#endif
