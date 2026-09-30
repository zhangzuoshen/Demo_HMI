#ifndef TOASTREGISTRY_H
#define TOASTREGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "OverlayPriority.h"

class QQmlComponent;
class QQmlEngine;

/**
 * @brief Toast 的静态信息
 *
 * 与 PopupInfo 同构，但字段完全不同 —— 这正是不复用 PopupInfo 的原因：
 * duration / duplicate / position 对 Dialog 毫无意义，
 * 而 modal / dim / conflict 对 Toast 也毫无意义。
 */
struct ToastInfo
{
    // 堆叠锚点
    enum Position
    {
        Top,
        Bottom,
        Center
    };

    /**
     * @brief 同 key 重复请求的处理
     *
     * Parallel ：并存（默认）
     * Collapse ：重置计时并更新 args，只保留一条（音量连按）
     * Replace  ：关掉旧的，插入新的（播放失败，第二条覆盖第一条）
     * Reject   ：已有则忽略（防刷屏）
     */
    enum Duplicate
    {
        Parallel,
        Collapse,
        Replace,
        Reject
    };

    QString toastId;
    QString ownerAppId;
    QString name;
    QString entry;
    QString basePath;      // ":/apps/Media/toasts/PlayFailed"

    int duration = 3000;   // 0 = sticky，不自动消失
    Position position = Bottom;

    // 与 Dialog 共用同一量级（OverlayPriority），跨系统可比
    int priority = OverlayPriority::Normal;

    // 满位时是否顶掉严格低于自己的 Toast
    bool preempt = false;

    Duplicate duplicate = Parallel;

    bool preload = false;
};

/**
 * @brief Toast 注册表
 *
 * 扫 <app 目录>/toasts/<toast 目录>/manifest.json，与 PopupRegistry 同构但独立。
 * key 为 "<ownerAppId>/<toastId>"，简写仅在全局唯一时可解析。
 */
class ToastRegistry : public QObject
{
    Q_OBJECT

public:
    explicit ToastRegistry(QObject *parent = nullptr);

    bool loadToasts(const QString &resourceRoot);

    bool contains(const QString &key) const;

    ToastInfo toast(const QString &key) const;

    QList<ToastInfo> toasts() const;

    //==============================
    // preload
    //==============================
    // 提前构造 QQmlComponent 完成编译，消除首次 show 的卡顿。
    // 一个 component 可以反复 create()，缓存是安全的。
    void preloadAll(QQmlEngine *engine);

    QQmlComponent *cachedComponent(const QString &key) const;

private:
    bool loadManifest(const QString &ownerAppId, const QString &dir);

    QString readOwnerAppId(const QString &appManifest) const;

    ToastInfo::Position parsePosition(const QString &position) const;

    ToastInfo::Duplicate parseDuplicate(const QString &duplicate) const;

    QString resolve(const QString &key) const;

private:
    QMap<QString, ToastInfo> m_toasts;

    // 简写 → 全限定名；值为空表示该简写有歧义
    QMap<QString, QString> m_shortcuts;

    // preload 缓存，以 registry 为 parent，进程生命周期内复用
    QMap<QString, QQmlComponent *> m_components;
};

#endif // TOASTREGISTRY_H
