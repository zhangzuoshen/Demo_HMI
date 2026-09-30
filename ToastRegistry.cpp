#include "ToastRegistry.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlComponent>
#include <QQmlEngine>

#include "Log.h"

ToastRegistry::ToastRegistry(QObject *parent)
    : QObject(parent)
{
}

bool ToastRegistry::loadToasts(const QString &resourceRoot)
{
    qCInfo(logRegistry) << "Scan toasts:" << resourceRoot;

    m_toasts.clear();
    m_shortcuts.clear();

    QDir root(resourceRoot);

    if (!root.exists())
    {
        qCWarning(logRegistry) << "Resource root not found:" << resourceRoot;

        return false;
    }

    const QFileInfoList dirs =
        root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &dir : dirs)
    {
        QString appManifest = dir.absoluteFilePath() + "/manifest.json";

        if (!QFile::exists(appManifest))
            continue;

        // ownerAppId 取自 App 自己的 manifest，目录名与 appId 不一致也没关系
        QString ownerAppId = readOwnerAppId(appManifest);

        if (ownerAppId.isEmpty())
        {
            qCWarning(logRegistry) << "Missing appId:" << appManifest;

            continue;
        }

        QDir toastDir(dir.absoluteFilePath() + "/toasts");

        if (!toastDir.exists())
            continue;

        const QFileInfoList toasts =
            toastDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo &toast : toasts)
            loadManifest(ownerAppId, toast.absoluteFilePath());
    }

    qCInfo(logRegistry) << "Total toasts:" << m_toasts.size();

    return !m_toasts.isEmpty();
}

QString ToastRegistry::readOwnerAppId(const QString &appManifest) const
{
    QFile file(appManifest);

    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject())
        return QString();

    return doc.object()["appId"].toString();
}

bool ToastRegistry::loadManifest(const QString &ownerAppId, const QString &dir)
{
    QFile file(dir + "/manifest.json");

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(logRegistry) << "Skip (no manifest):" << dir;

        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject())
    {
        qCWarning(logRegistry) << "Invalid manifest:" << file.fileName();

        return false;
    }

    QJsonObject obj = doc.object();

    ToastInfo info;

    info.toastId = obj["toastId"].toString();
    info.name = obj["name"].toString();
    info.entry = obj["entry"].toString("Toast.qml");
    info.ownerAppId = ownerAppId;
    info.basePath = dir;

    info.duration = obj["duration"].toInt(3000);
    info.position = parsePosition(obj["position"].toString("bottom"));
    info.priority = obj["priority"].toInt(OverlayPriority::Normal);
    info.preempt = obj["preempt"].toBool(false);
    info.duplicate = parseDuplicate(obj["duplicate"].toString("parallel"));
    info.preload = obj["preload"].toBool(false);

    if (info.toastId.isEmpty())
    {
        qCWarning(logRegistry) << "Missing toastId:" << dir;

        return false;
    }

    QString key = ownerAppId + "/" + info.toastId;

    if (m_shortcuts.contains(info.toastId))
    {
        qCWarning(logRegistry) << "Ambiguous shortcut, disabled:"
                               << info.toastId;

        m_shortcuts.insert(info.toastId, QString());
    }
    else
    {
        m_shortcuts.insert(info.toastId, key);
    }

    m_toasts.insert(key, info);

    qCInfo(logRegistry) << "Register toast" << key
                        << "duration:" << info.duration
                        << "priority:" << OverlayPriority::toString(info.priority)
                        << "duplicate:" << obj["duplicate"].toString("parallel");

    return true;
}

ToastInfo::Position ToastRegistry::parsePosition(const QString &position) const
{
    if (position == "top")
        return ToastInfo::Top;

    if (position == "center")
        return ToastInfo::Center;

    return ToastInfo::Bottom;
}

ToastInfo::Duplicate ToastRegistry::parseDuplicate(
    const QString &duplicate) const
{
    if (duplicate == "collapse")
        return ToastInfo::Collapse;

    if (duplicate == "replace")
        return ToastInfo::Replace;

    if (duplicate == "reject")
        return ToastInfo::Reject;

    return ToastInfo::Parallel;
}

void ToastRegistry::preloadAll(QQmlEngine *engine)
{
    if (!engine)
        return;

    for (auto it = m_toasts.constBegin(); it != m_toasts.constEnd(); ++it)
    {
        const ToastInfo &info = it.value();

        if (!info.preload)
            continue;

        QString path = info.basePath;

        path.replace(":/", "qrc:/");

        auto *comp = new QQmlComponent(
            engine, QUrl(path + "/" + info.entry), this);

        if (comp->isError())
        {
            qCWarning(logRegistry) << "Preload failed:" << it.key()
                                   << comp->errors();

            comp->deleteLater();

            continue;
        }

        m_components.insert(it.key(), comp);

        qCInfo(logRegistry) << "Preloaded:" << it.key();
    }
}

QQmlComponent *ToastRegistry::cachedComponent(const QString &key) const
{
    return m_components.value(resolve(key), nullptr);
}

QString ToastRegistry::resolve(const QString &key) const
{
    if (m_toasts.contains(key))
        return key;

    return m_shortcuts.value(key);
}

bool ToastRegistry::contains(const QString &key) const
{
    return m_toasts.contains(resolve(key));
}

ToastInfo ToastRegistry::toast(const QString &key) const
{
    return m_toasts.value(resolve(key));
}

QList<ToastInfo> ToastRegistry::toasts() const
{
    return m_toasts.values();
}
