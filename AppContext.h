#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>

#include "AppState.h"

struct AppInstance;

class AppContext : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString appId READ appId CONSTANT)
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString entry READ entry CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(QString launchMode READ launchMode CONSTANT)

    Q_PROPERTY(quint64 instanceId READ instanceId CONSTANT)
    Q_PROPERTY(bool firstLaunch READ firstLaunch CONSTANT)

    Q_PROPERTY(AppState::State state
               READ state
               NOTIFY stateChanged)

public:

    explicit AppContext(QObject *parent=nullptr);

    void initialize(const AppInstance &instance);

    QString appId() const;
    QString appName() const;
    QString entry() const;
    QString icon() const;

    int priority() const;

    QString launchMode() const;

    quint64 instanceId() const;

    bool firstLaunch() const;

    AppState::State state() const;

    void setState(AppState::State state);

signals:

    void stateChanged();

private:

    QString m_appId;
    QString m_appName;
    QString m_entry;
    QString m_icon;

    QString m_launchMode;

    int m_priority = 0;

    quint64 m_instanceId = 0;

    bool m_firstLaunch = true;

    AppState::State m_state = AppState::None;
};

#endif
