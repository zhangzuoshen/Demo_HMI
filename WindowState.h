#ifndef WINDOWSTATE_H
#define WINDOWSTATE_H

#include <QObject>

class WindowState
{
    Q_GADGET

public:
    enum State
    {
        Detached,
        Attached,
        Visible,
        Transition
    };
    Q_ENUM(State)

    static QString toString(State s)
    {
        switch(s)
        {
        case Detached: return "Detached";
        case Attached: return "Attached";
        case Visible: return "Visible";
        case Transition: return "Transition";
        }

        return "Unknown";
    }
};

#endif
