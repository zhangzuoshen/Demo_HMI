#include "PopupRegistry.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include "Log.h"

PopupRegistry::PopupRegistry(QObject *parent)
    : QObject(parent)
{
}

bool PopupRegistry::loadPopups(const QString &resourceRoot)
{
    qCInfo(logRegistry) << "Scan popups:" << resourceRoot;

    m_popups.clear();
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

        // ownerAppId 取自 App 自己的 manifest，
        // 这样目录名与 appId 不一致也不会出错
        QString ownerAppId = readOwnerAppId(appManifest);

        if (ownerAppId.isEmpty())
        {
            qCWarning(logRegistry) << "Missing appId:" << appManifest;

            continue;
        }

        QDir popupDir(dir.absoluteFilePath() + "/popups");

        if (!popupDir.exists())
            continue;

        const QFileInfoList popups =
            popupDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo &popup : popups)
            loadManifest(ownerAppId, popup.absoluteFilePath());
    }

    qCInfo(logRegistry) << "Total popups:" << m_popups.size();

    return !m_popups.isEmpty();
}

QString PopupRegistry::readOwnerAppId(const QString &appManifest) const
{
    QFile file(appManifest);

    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    if (!doc.isObject())
        return QString();

    return doc.object()["appId"].toString();
}

bool PopupRegistry::loadManifest(const QString &ownerAppId, const QString &dir)
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

    PopupInfo info;

    info.popupId = obj["popupId"].toString();
    info.name = obj["name"].toString();
    info.entry = obj["entry"].toString("Popup.qml");
    info.ownerAppId = ownerAppId;
    info.basePath = dir;

    info.kind = parseKind(obj["kind"].toString("modal"));
    info.dim = obj["dim"].toBool(true);
    info.closeOnDimClick = obj["closeOnDimClick"].toBool(true);
    info.closeOnBackKey = obj["closeOnBackKey"].toBool(true);

    info.priority = obj["priority"].toInt(OverlayPriority::Normal);
    info.conflict = parseConflict(obj["conflict"].toString("stack"));

    if (info.popupId.isEmpty())
    {
        qCWarning(logRegistry) << "Missing popupId:" << dir;

        return false;
    }

    QString key = ownerAppId + "/" + info.popupId;

    // 简写唯一性：出现同名则置空，禁止再用简写解析
    if (m_shortcuts.contains(info.popupId))
    {
        qCWarning(logRegistry) << "Ambiguous shortcut, disabled:" << info.popupId;

        m_shortcuts.insert(info.popupId, QString());
    }
    else
    {
        m_shortcuts.insert(info.popupId, key);
    }

    m_popups.insert(key, info);

    qCInfo(logRegistry) << "Register popup" << key
                        << "kind:" << obj["kind"].toString("modal")
                        << "priority:" << OverlayPriority::toString(info.priority)
                        << "conflict:" << obj["conflict"].toString("stack");

    return true;
}

PopupInfo::Kind PopupRegistry::parseKind(const QString &kind) const
{
    if (kind == "modeless")
        return PopupInfo::Modeless;

    return PopupInfo::Modal;
}

PopupInfo::Conflict PopupRegistry::parseConflict(const QString &conflict) const
{
    if (conflict == "preempt")
        return PopupInfo::Preempt;

    if (conflict == "reject")
        return PopupInfo::Reject;

    return PopupInfo::Stack;
}

QString PopupRegistry::resolve(const QString &key) const
{
    if (m_popups.contains(key))
        return key;

    // 值为空 = 简写有歧义，拒绝解析
    return m_shortcuts.value(key);
}

bool PopupRegistry::contains(const QString &key) const
{
    return m_popups.contains(resolve(key));
}

PopupInfo PopupRegistry::popup(const QString &key) const
{
    return m_popups.value(resolve(key));
}

QList<PopupInfo> PopupRegistry::popups() const
{
    return m_popups.values();
}
