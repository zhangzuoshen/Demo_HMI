#include "AppRegistry.h"
#include "Log.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

AppRegistry::AppRegistry(QObject *parent)
    : QObject(parent)
{
}

bool AppRegistry::loadApps(const QString &rootPath)
{
    qCInfo(logAppRegistry) << "Scan resource path:" << rootPath;

    QDir root(rootPath);

    int count = 0;

    const auto dirs = root.entryInfoList(
                QDir::Dirs | QDir::NoDotAndDotDot);

    for(const QFileInfo &dirInfo : dirs)
    {
        QString manifest = dirInfo.absoluteFilePath() + "/manifest.json";

        if(!QFile::exists(manifest))
        {
            qCDebug(logAppRegistry)
                    << "Skip (no manifest):"
                    << dirInfo.fileName();
            continue;
        }

        if(loadManifest(dirInfo.absoluteFilePath()))
            count++;
    }

    qCInfo(logAppRegistry) << "Total apps:" << count;

    return count > 0;
}

bool AppRegistry::loadManifest(const QString &appDir)
{
    QString manifestPath =
            appDir + "/manifest.json";

    QFile file(manifestPath);

    if(!file.open(QIODevice::ReadOnly))
    {
        qCWarning(logAppRegistry)
                << "Cannot open:"
                << manifestPath;
        return false;
    }

    auto doc =
            QJsonDocument::fromJson(file.readAll());

    if(doc.isNull())
    {
        qCCritical(logAppRegistry)
                << "Invalid json:"
                << manifestPath;
        return false;
    }

    auto obj = doc.object();

    AppInfo info;

    info.appId = obj.value("appId").toString();
    info.name = obj.value("name").toString();
    info.entry = obj.value("entry").toString();
    info.icon = obj.value("icon").toString();
    info.priority = obj.value("priority").toInt(0);
    info.basePath = appDir;

    QString mode =
            obj.value("launchMode")
            .toString("standard");

    if(mode == "singleTask")
        info.launchMode = AppInfo::SingleTask;
    else if(mode == "singleTop")
        info.launchMode = AppInfo::SingleTop;
    else
        info.launchMode = AppInfo::Standard;

    if(info.appId.isEmpty())
    {
        qCCritical(logAppRegistry)
                << "Missing appId:"
                << manifestPath;
        return false;
    }

    if(m_apps.contains(info.appId))
    {
        qCWarning(logAppRegistry)
                << "Duplicate appId:"
                << info.appId;
        return false;
    }

    m_apps.insert(info.appId, info);

    qCInfo(logAppRegistry)
            << "Register"
            << info.appId
            << "priority:" << info.priority
            << "launchMode:" << mode;

    return true;
}

bool AppRegistry::contains(const QString &appId) const
{
    return m_apps.contains(appId);
}

AppInfo AppRegistry::app(const QString &appId) const
{
    return m_apps.value(appId);
}

QStringList AppRegistry::appIds() const
{
    return m_apps.keys();
}

QList<AppInfo> AppRegistry::apps() const
{
    auto list = m_apps.values();

    std::sort(list.begin(),
              list.end(),
              [](const AppInfo &a,
                 const AppInfo &b)
              {
                  return a.priority > b.priority;
              });

    return list;
}
