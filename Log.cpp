#include "Log.h"

#include <QDateTime>
#include <QDebug>

// 分类定义
Q_LOGGING_CATEGORY(logMain, "MAIN")
Q_LOGGING_CATEGORY(logBootstrap, "Bootstrap")
Q_LOGGING_CATEGORY(logPageManager, "PageManager")
Q_LOGGING_CATEGORY(logScene, "Scene")
Q_LOGGING_CATEGORY(logRegistry, "AppRegistry")
Q_LOGGING_CATEGORY(logQml, "QML")

static void messageHandler(QtMsgType type, const QMessageLogContext &ctx,
                           const QString &msg)
{
    QString level;

    switch (type)
    {
    case QtDebugMsg: level = "DEBUG"; break;
    case QtInfoMsg: level = "INFO "; break;
    case QtWarningMsg: level = "WARN "; break;
    case QtCriticalMsg: level = "ERROR"; break;
    case QtFatalMsg: level = "FATAL"; break;
    }

    QString category = ctx.category;

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    fprintf(stdout, "[%s][%s][%s] %s\n", time.toUtf8().constData(),
            level.toUtf8().constData(), category.toUtf8().constData(),
            msg.toUtf8().constData());

    fflush(stdout);

    if (type == QtFatalMsg)
        abort();
}

void initLogSystem()
{
    qInstallMessageHandler(messageHandler);

    qSetMessagePattern("%{message}");

    qInfo() << "========== HMI Log Started ==========";
}
