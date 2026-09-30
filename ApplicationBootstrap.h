#ifndef APPLICATIONBOOTSTRAP_H
#define APPLICATIONBOOTSTRAP_H

class QQmlApplicationEngine;
class PageManager;
class PopupManager;
class ToastManager;
class AppRegistry;

class ApplicationBootstrap
{
public:
    static bool initialize(QQmlApplicationEngine &engine, AppRegistry &registry,
                           PageManager &pageManager,
                           PopupManager &popupManager,
                           ToastManager &toastManager);
};

#endif
