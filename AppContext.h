#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>
#include <QtGlobal>
#include "AppInstance.h"

class AppContext : public QObject
{
    Q_OBJECT

    // 页面状态
    Q_PROPERTY(State state
               READ state
               NOTIFY stateChanged)

    Q_PROPERTY(bool isForeground
               READ isForeground
               NOTIFY stateChanged)

    Q_PROPERTY(bool isActive
               READ isActive
               NOTIFY stateChanged)

    // 运行时实例信息
    Q_PROPERTY(quint64 instanceId READ instanceId CONSTANT)
    Q_PROPERTY(bool firstLaunch READ firstLaunch CONSTANT)

    // Manifest 信息
    Q_PROPERTY(QString appId READ appId CONSTANT)
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString entry READ entry CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(QString launchMode READ launchMode CONSTANT)

public:
    enum State
    {
        Created,
        Ready,
        Foreground,
        Background,
        Destroyed
    };
    Q_ENUM(State)

    explicit AppContext(const AppInstance &instance,
                        QObject *parent = nullptr);

    State state() const;
    bool isForeground() const;
    bool isActive() const;
    void setState(State state);

    quint64 instanceId() const;
    bool firstLaunch() const;

    QString appId() const;
    QString appName() const;
    QString entry() const;
    QString icon() const;

    int priority() const;
    QString launchMode() const;

signals:
    void stateChanged();

private:
    State m_state=Created;
    AppInstance m_instance;
    QString m_launchModeString;
};

#endif // APPCONTEXT_H
