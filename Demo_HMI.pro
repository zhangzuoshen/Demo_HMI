QT += quick

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    AppContext.h \
    AppInstance.h \
    AppRegistry.h \
    AppState.h \
    ApplicationBootstrap.h \
    Log.h \
    PageIncubator.h \
    PageManager.h \
    PageView.h \
    SceneContainer.h \
    WindowState.h

SOURCES += \
        AppContext.cpp \
        AppInstance.cpp \
        AppRegistry.cpp \
        ApplicationBootstrap.cpp \
        Log.cpp \
        PageIncubator.cpp \
        PageManager.cpp \
        PageView.cpp \
        SceneContainer.cpp \
        main.cpp

RESOURCES += \
    apps/apps.qrc

QMAKE_CXXFLAGS += -Wno-deprecated-copy

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
