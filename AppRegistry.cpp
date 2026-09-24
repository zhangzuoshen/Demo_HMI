#include "AppRegistry.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "Log.h"

AppRegistry::AppRegistry(QObject *parent)
    : QObject(parent)
{
}

bool AppRegistry::loadApps(const QString &resourceRoot)
{
    qCInfo(logRegistry) << "Scan resource path:" << resourceRoot;

    m_apps.clear();

    QDir root(resourceRoot);

    QFileInfoList dirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &dir, dirs)
    {
        QString manifest = dir.absoluteFilePath() + "/manifest.json";

        if (!QFile::exists(manifest))
        {
            qCDebug(logRegistry) << "Skip (no manifest):" << dir.fileName();

            continue;
        }

        loadManifest(dir.absoluteFilePath());
    }

    qCInfo(logRegistry) << "Total apps:" << m_apps.size();

    return !m_apps.isEmpty();
}

bool AppRegistry::loadManifest(const QString &appDir)
{
    QFile file(appDir + "/manifest.json");

    if (!file.open(QIODevice::ReadOnly))
    {
        qCWarning(logRegistry) << "Cannot open:" << file.fileName();

        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject())
    {
        qCWarning(logRegistry) << "Invalid manifest:" << file.fileName();

        return false;
    }

    QJsonObject obj = doc.object();

    AppInfo info;

    info.appId = obj["appId"].toString();
    info.name = obj["name"].toString();
    info.entry = obj["entry"].toString("App.qml");
    info.icon = obj["icon"].toString();

    info.priority = obj["priority"].toInt(0);
    info.keepAlive = obj["keepAlive"].toBool(false);

    info.launchMode = parseLaunchMode(obj["launchMode"].toString("standard"));

    info.basePath = appDir;

    if (info.appId.isEmpty())
    {
        qCWarning(logRegistry) << "Missing appId:" << appDir;

        return false;
    }

    m_apps.insert(info.appId, info);

    qCInfo(logRegistry) << "Register" << info.appId
                        << "priority:" << info.priority
                        << "launchMode:" << obj["launchMode"].toString()
                        << "keepAlive:" << info.keepAlive;

    return true;
}

AppInfo::LaunchMode AppRegistry::parseLaunchMode(const QString &mode) const
{
    if (mode == "singleTop")
        return AppInfo::SingleTop;

    if (mode == "singleTask")
        return AppInfo::SingleTask;

    return AppInfo::Standard;
}

bool AppRegistry::contains(const QString &appId) const
{
    return m_apps.contains(appId);
}

AppInfo AppRegistry::app(const QString &appId) const
{
    return m_apps.value(appId);
}

QList<AppInfo> AppRegistry::apps() const
{
    return m_apps.values();
}
