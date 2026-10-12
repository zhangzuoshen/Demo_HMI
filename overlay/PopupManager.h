#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QVariant>
#include <QVariantMap>

#include "OverlayPriority.h"
#include "PopupInstance.h"
#include "PopupRegistry.h"

class PageManager;

/**
 * @brief 弹窗管理器（核心）
 *
 * 与 PageManager 的关系：**同构不同类**。
 * 弹窗是"多层叠加 + 结果回传 + 归属宿主页面"，语义与页面栈完全不同，
 * 硬塞进 AppInstance / AppState 会给页面系统加一堆特例分支。
 *
 * 对外契约：
 * - open() 返回的是运行时 popupId，**不是结果**（此时 QML 还没创建完）
 * - 结果通过 popupResult(popupId, result) 信号回传
 * - open() 返回 0 = 失败或被 Reject 策略拒绝，此时**不会有** popupResult
 */
class PopupManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    Q_PROPERTY(bool hasModal READ hasModal NOTIFY countChanged)

public:
    explicit PopupManager(PopupRegistry *registry, PageManager *pages,
                          QObject *parent = nullptr);
    ~PopupManager() override;

    Q_INVOKABLE quint64 open(const QString &key,
                             const QVariantMap &args = QVariantMap());

    Q_INVOKABLE void close(quint64 popupId,
                           const QVariant &result = QVariant());

    // 关闭视觉最上层（z 序最高），不是最后入栈的那个
    Q_INVOKABLE void closeTop(const QVariant &result = QVariant());

    Q_INVOKABLE void closeAll();

    int count() const;

    bool hasModal() const;

    // 当前最高优先级，QML 侧可用于预判能否弹得出来
    int topPriority() const;

public slots:
    // PopupContainer 挂载完成后回调
    void popupReady(quint64 popupId);

    // 宿主页面销毁 → 关掉它弹出的所有弹窗
    void onOwnerDestroyed(quint64 instanceId);

signals:
    void popupCreated(const PopupInstance &instance);

    void popupDestroyed(quint64 popupId);

    void popupResult(quint64 popupId, const QVariant &result);

    // 被更高优先级的弹窗顶掉（popupResult 照发，result 为 null）
    void popupPreempted(quint64 popupId);

    // 被 Reject 策略拒绝，不会有 popupResult
    void popupRejected(const QString &key, int priority);

    void countChanged();

private:
    PopupInstance createInstance(const PopupInfo &info, const QVariantMap &args);

    void changeState(PopupInstance &instance, OverlayState::State state);

    bool admit(const PopupInfo &info) const;

    void preemptClose(quint64 popupId);

    // 置 Closing，等 QML 播退场动画
    void beginClose(quint64 popupId, const QVariant &result);

    // 真正销毁 + 发结果
    void finalizeClose(quint64 popupId);

    // z 序最高者（closeTop 用）
    quint64 visualTopId() const;

    bool hasClosing() const;

    void updateCoveredState();

    void releaseContext(PopupInstance &instance);

    int indexOf(quint64 popupId) const;

    PopupInstance *findInstance(quint64 popupId);

private:
    PopupRegistry *m_registry = nullptr;

    QPointer<PageManager> m_pages;

    // 栈底 → 栈顶；z 序另算（按优先级，见 PopupContainer::restack）
    QList<PopupInstance> m_stack;

    // 关闭结果暂存：context 可能先于 finalizeClose 被销毁
    QHash<quint64, QVariant> m_results;

    quint64 m_nextPopupId = 1;

    // 下层页面是否已置 Covered，避免多层弹窗重复下发
    bool m_covered = false;

    // 退场动画兜底超时（毫秒）
    int m_closeTimeoutMs = 3000;
};

#endif // POPUPMANAGER_H
