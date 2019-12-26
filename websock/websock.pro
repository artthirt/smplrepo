TARGET = websock
TEMPLATE = app

DESTDIR = $$OUT_PWD/../bin

QT += core gui widgets network multimedia opengl

INCLUDEPATH += $$PWD

AV = $$PWD/ffmpeg

OPENCL = $$PWD/3rdparty/include/opencl/1.2/

HEADERS += mainwindow.h \
    camerastream.h \
    cl_main_object.h \
    common.h \
    DataStream.h \
    dialogsetfilename.h \
    dialogtcphost.h \
    jpegenc.h \
    tcpserver.h \
    tcpsocket.h \
    testsender.h \
    utils.h \
    videodata.h \
    videoframe.h \
    videosurface.h \
    websock.h

SOURCES += main.cpp \
            mainwindow.cpp \
    camerastream.cpp \
    cl_main_object.cpp \
    common.cpp \
    DataStream.cpp \
    dialogsetfilename.cpp \
    dialogtcphost.cpp \
    jpegenc.cpp \
    main.cpp \
    tcpserver.cpp \
    tcpsocket.cpp \
    testsender.cpp \
    videodata.cpp \
    videoframe.cpp \
    videosurface.cpp \
    websock.cpp

INCLUDEPATH += $$AV/include \
               $$OPENCL

LIBS += -L$$AV/bin -lavcodec -lavformat -lavutil -lWS2_32  -L$$DESTDIR -ljpeg

CONFIG(debug, debug|release){
    DST = debug
}else{
    DST = release
}

UI_DIR = tmp/$$DST/ui
RCC_DIR = tmp/$$DST/rcc
OBJECTS_DIR = tmp/$$DST/obj
MOC_DIR = tmp/$$DST/moc

RESOURCES += \
    resource.qrc

FORMS += \
    dialogsetfilename.ui \
    dialogtcphost.ui \
    mainwindow.ui \
    videoframe.ui
