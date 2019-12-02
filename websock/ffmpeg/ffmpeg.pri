INCLUDEPATH += $$PWD/include

win32{
    LIBS += -L$$PWD/bin/ -lavcodec -lavformat -lavutil
}else{
    LIBS += -L/usr/local -L$$PWD/bin/ -l:libavcodec.so.57 -l:libavformat.so.57 -l:libavutil.so.55
}
