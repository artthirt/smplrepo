QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    videodata.cpp \
    videoframe.cpp \
    websock.cpp

HEADERS += \
    mainwindow.h \
    videodata.h \
    videoframe.h \
    websock.h

FORMS += \
    mainwindow.ui \
    videoframe.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
