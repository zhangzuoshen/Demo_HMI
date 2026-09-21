#ifndef APPSTATE_H
#define APPSTATE_H

#include <QObject>

class AppState
{
    Q_GADGET

public:
    enum State
    {
        None = 0,
        Created,
        Ready,
        Foreground,
        Background,
        Covered,
        Suspended,
        Destroyed
    };
    Q_ENUM(State)
};

#endif
