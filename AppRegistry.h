#ifndef APPREGISTRY_H
#define APPREGISTRY_H

#include <QObject>
#include <QMap>
#include <QStringList>

struct AppInfo
{
    enum LaunchMode
    {
        Standard,
        SingleTop,
        SingleTask
    };

    QString appId;
    QString name;
    QString entry;
    QString basePath;
    QString icon;

    int priority = 0;
    LaunchMode launchMode = Standard;
};

class AppRegistry : public QObject
{
    Q_OBJECT

public:
    explicit AppRegistry(QObject *parent = nullptr);

    bool loadApps(const QString &rootPath);

    bool contains(const QString &appId) const;

    AppInfo app(const QString &appId) const;

    QStringList appIds() const;

    QList<AppInfo> apps() const;

private:
    bool loadManifest(const QString &appDir);

private:
    QMap<QString, AppInfo> m_apps;
};

#endif
