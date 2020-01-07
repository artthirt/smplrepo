TARGET = jpeg
TEMPLATE = lib

DESTDIR = $$OUT_PWD/../bin

INCLUDEPATH += $$PWD

SOURCES = jcapimin.c\
        jmemmgr.c\
        jmemansi.c\
        jcomapi.c\
        jcmarker.c\
        jutils.c\
        jerror.c\
        jcparam.c\
        jcapistd.c\
        jcinit.c jcmaster.c jccolor.c jcsample.c \
        jcprepct.c jcdctmgr.c jcphuff.c jchuff.c \
        jccoefct.c jcmainct.c jdct.h jfdctint.c \
        jfdctflt.c jfdctfst.c

HEADERS += \
    jconfig.h \
    jpeglib.h \
    jinclude.h \
    jversion.h \
    jmorecfg.h

CONFIG(debug, debug|release){
    DST = debug
}else{
    DST = release
}

UI_DIR = tmp/$$DST/ui
RCC_DIR = tmp/$$DST/rcc
OBJECTS_DIR = tmp/$$DST/obj
MOC_DIR = tmp/$$DST/moc
