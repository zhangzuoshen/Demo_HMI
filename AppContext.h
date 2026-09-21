#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>

#include "AppInstance.h"

class AppContext : public QObject
{
    Q_OBJECT

    // Manifest
    Q_PROPERTY(QString appId READ appId CONSTANT)
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString entry READ entry CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)

    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(QString launchMode READ launchMode CONSTANT)

    // Runtime
    Q_PROPERTY(quint64 instanceId READ instanceId CONSTANT)
    Q_PROPERTY(bool firstLaunch READ firstLaunch CONSTANT)

    Q_PROPERTY(AppState::State state
               READ state
               NOTIFY stateChanged)

public:

    explicit AppContext(
            const AppInstance &instance,
            QObject *parent=nullptr);

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

    AppInstance m_instance;

    QString m_launchModeString;

    AppState::State m_state;
};

#endif
