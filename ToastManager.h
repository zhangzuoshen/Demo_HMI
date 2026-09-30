#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QVariantMap>

#include "OverlayState.h"
#include "ToastContext.h"
#include "ToastRegistry.h"

class PageManager;
class QQmlContext;
class QQmlEngine;
class QQuickItem;
class QTimer;
class ToastContainer;

/**
 * @brief 单条 Toast 的运行时状态
 *
 * 与 Dialog 的 PopupInstance 不同：这里直接持有 QML 对象。
 * Toast 是同步创建的，不需要 Incubator，也就不需要单独的 View 类。
 */
struct ToastEntry
{
    quint64 toastId = 0;

    QString key;

    ToastInfo info;

    OverlayState::State state = OverlayState::None;

    // 宿主页面实例；0 = 系统级，不随页面销毁
    quint64 ownerInstanceId = 0;

    QPointer<ToastContext> context;

    QQmlContext *qmlContext = nullptr;

    QQuickItem *item = nullptr;

    // duration > 0 时存在；sticky 为 nullptr
    QTimer *timer = nullptr;
};

/**
 * @brief Toast 管理器（核心）
 *
 * 与 PopupManager 的三个本质差别：
 * 1. **同步创建**：Toast 必须立即可见，异步孵化最快也要下一帧，
 *    体感就是"点了没反应"。所以不走 Incubator。
 * 2. **队列模型**：Toast 是"队列 + 可视窗口"，不是栈。多条并存是常态。
 * 3. **无结果回传**：show() 返回的 toastId 只用于后续 dismiss，没有回调语义。
 */
class ToastManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    // Active 满位时的溢出策略
    enum Overflow
    {
        Queue,        // 排队等待（默认）
        DropOldest    // 踢掉优先级最低且最老的
    };

    explicit ToastManager(ToastRegistry *registry, QQmlEngine *engine,
                          PageManager *pages, QObject *parent = nullptr);
    ~ToastManager() override;

    Q_INVOKABLE quint64 show(const QString &key,
                             const QVariantMap &args = QVariantMap());

    Q_INVOKABLE void dismiss(quint64 toastId);

    // 按 key 批量关（如 USB 拔出）
    Q_INVOKABLE void dismissKey(const QString &key);

    Q_INVOKABLE void dismissAll();

    int count() const;

    int maxVisible() const;
    void setMaxVisible(int max);

    Overflow overflow() const;
    void setOverflow(Overflow overflow);

    // ToastContainer 挂载时回调（由它自己调用）
    void setContainer(ToastContainer *container);

public slots:
    void onOwnerDestroyed(quint64 instanceId);

signals:
    void toastCreated(quint64 toastId);

    void toastDestroyed(quint64 toastId);

    void actionTriggered(quint64 toastId, const QString &name,
                         const QVariant &data);

    void countChanged();

private:
    bool activate(ToastEntry &entry);

    void beginClose(quint64 toastId);

    void finalizeClose(quint64 toastId);

    // Active 有空位时从 waiting 补位
    void pumpQueue();

    void releaseEntry(ToastEntry &entry);

    int indexOfActive(quint64 toastId) const;
    int indexOfWaiting(quint64 toastId) const;

    int indexOfKey(const QList<ToastEntry> &list, const QString &key) const;

    // 满位抢占：找出 priority 严格低于自己、且其中最低最老的
    int findPreemptVictim(int priority) const;

    // dropOldest：优先级最低且最老的
    int findDropVictim() const;

    void changeState(ToastEntry &entry, OverlayState::State state);

private:
    ToastRegistry *m_registry = nullptr;

    QQmlEngine *m_engine = nullptr;

    QPointer<PageManager> m_pages;

    ToastContainer *m_container = nullptr;

    QList<ToastEntry> m_active;

    QList<ToastEntry> m_waiting;

    quint64 m_nextToastId = 1;

    int m_maxVisible = 3;

    Overflow m_overflow = Queue;

    // 退场动画兜底超时（毫秒）
    int m_closeTimeoutMs = 3000;
};

#endif // TOASTMANAGER_H
