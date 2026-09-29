#ifndef POPUPREGISTRY_H
#define POPUPREGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "OverlayPriority.h"

/**
 * @brief Manifest中的静态弹窗信息
 *
 * 与 AppInfo 同构，但多了弹窗专有的字段（模态、遮罩、冲突策略）。
 */
struct PopupInfo
{
    enum Kind
    {
        Modal,    // 拦截输入
        Modeless  // 点击穿透
    };

    /**
     * @brief 与已显示弹窗冲突时本弹窗采取的策略
     *
     * Stack  ：不管优先级，直接叠加（默认）
     * Preempt：关闭所有优先级严格低于自己的，然后自己显示
     * Reject ：已有优先级 >= 自己的，则放弃显示（open() 返回 0）
     */
    enum Conflict
    {
        Stack,
        Preempt,
        Reject
    };

    QString popupId;    // "trackInfo"
    QString ownerAppId; // "media"
    QString name;
    QString entry;      // "TrackInfo.qml"
    QString basePath;   // ":/apps/Media/popups/TrackInfo"

    Kind kind = Modal;

    bool dim = true;
    bool closeOnDimClick = true;
    bool closeOnBackKey = true;

    int priority = OverlayPriority::Normal;

    Conflict conflict = Stack;
};

/**
 * @brief 弹窗注册表
 *
 * 扫 <root>/*/popups/*/manifest.json，与 AppRegistry 同构但完全独立。
 * key 为全限定名 "<ownerAppId>/<popupId>"，如 "media/trackInfo"。
 * 简写（"trackInfo"）仅在全局唯一时可解析，否则拒绝 —— 避免以后新增一个
 * 同名弹窗就悄悄改变解析结果。
 */
class PopupRegistry : public QObject
{
    Q_OBJECT

public:
    explicit PopupRegistry(QObject *parent = nullptr);

    bool loadPopups(const QString &resourceRoot);

    bool contains(const QString &key) const;

    PopupInfo popup(const QString &key) const;

    QList<PopupInfo> popups() const;

private:
    bool loadManifest(const QString &ownerAppId, const QString &dir);

    QString readOwnerAppId(const QString &appManifest) const;

    PopupInfo::Kind parseKind(const QString &kind) const;

    PopupInfo::Conflict parseConflict(const QString &conflict) const;

    // 解析 key：优先全限定名，其次唯一简写
    QString resolve(const QString &key) const;

private:
    QMap<QString, PopupInfo> m_popups;

    // 简写 → 全限定名；值为空表示该简写有歧义，不可用
    QMap<QString, QString> m_shortcuts;
};

#endif // POPUPREGISTRY_H
