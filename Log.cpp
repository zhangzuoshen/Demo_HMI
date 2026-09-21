#include "Log.h"

#include <QDateTime>
#include <QDebug>

// 分类定义
Q_LOGGING_CATEGORY(logAppRegistry, "HMI.AppRegistry")
Q_LOGGING_CATEGORY(logPageManager, "HMI.PageManager")
Q_LOGGING_CATEGORY(logSceneContainer, "HMI.SceneContainer")
Q_LOGGING_CATEGORY(logQml, "HMI.QML")

static void messageHandler(QtMsgType type,
                           const QMessageLogContext &context,
                           const QString &msg)
{
    Q_UNUSED(context)

    QString level;

    switch (type)
    {
    case QtDebugMsg:
        level = "DEBUG";
        break;

    case QtInfoMsg:
        level = "INFO ";
        break;

    case QtWarningMsg:
        level = "WARN ";
        break;

    case QtCriticalMsg:
        level = "ERROR";
        break;

    case QtFatalMsg:
        level = "FATAL";
        break;
    }

    QString category = context.category;

//    if (category.startsWith("HMI."))
//        category.remove(0,4);

    QString time =
        QDateTime::currentDateTime()
        .toString("hh:mm:ss.zzz");

    fprintf(stdout,
            "[%s][%s][%s] %s\n",
            time.toUtf8().constData(),
            level.toUtf8().constData(),
            category.toUtf8().constData(),
            msg.toUtf8().constData());

    fflush(stdout);

    if(type==QtFatalMsg)
        abort();
}

void initLogSystem()
{
    qInstallMessageHandler(messageHandler);

    qSetMessagePattern("%{message}");

    qInfo() << "========== HMI Log Started ==========";
}
