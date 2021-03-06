#-------------------------------------------------
#
# Project created by QtCreator 2017-07-14T17:02:56
#
#-------------------------------------------------

QT       += core gui widgets opengl webenginewidgets xml network sql

TARGET = director_service
#TARGET = pic_display_service
TEMPLATE = lib

DEFINES += DS_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += precompile_header
PRECOMPILED_HEADER  = ds_global.h

SOURCES += \
        ds.cpp \
    hglwidget.cpp \
    hmainwidget.cpp \
    hdsctx.cpp \
    haudioplay.cpp \
    htitlebarwidget.cpp \
    htoolbarwidget.cpp \
    hrcloader.cpp \
    hmaintoolbar.cpp \
    hffmpeg.cpp \
    hchangecolorwidget.cpp \
    hnumselectwidget.cpp \
    qglwidgetimpl.cpp \
    hexprewidget.cpp \
    hnetwork.cpp \
    haddtextwidget.cpp \
    hsettingwidget.cpp \
    habstractitem.cpp \
    hoperatetarget.cpp \
    hcolorwidget.cpp \
    heffectwidget.cpp \
    hringbuffer.cpp \
    hlayout.cpp \
    hdsconf.cpp \
    hsaveinfo.cpp \
    hdsdb.cpp \
    hptzwidget.cpp

HEADERS += \
        ds.h \
        ds_global.h \ 
    hglwidget.h \
    hmainwidget.h \
    hdsctx.h \
    haudioplay.h \
    htitlebarwidget.h \
    htoolbarwidget.h \
    hrcloader.h \
    hmaintoolbar.h \
    hffmpeg.h \
    hchangecolorwidget.h \
    hnumselectwidget.h \
    qglwidgetimpl.h \
    hexprewidget.h \
    hnetwork.h \
    ds_def.h \
    haddtextwidget.h \
    hsettingwidget.h \
    habstractitem.h \
    hoperatetarget.h \
    hcolorwidget.h \
    qtheaders.h \
    heffectwidget.h \
    hringbuffer.h \
    hlayout.h \
    singleton.h \
    ds_version.h \
    hdsconf.h \
    hsaveinfo.h \
    hdsdb.h \
    list.h \
    somedef.h \
    hptzwidget.h

DISTFILES += \
    ../readme

unix {
    INCLUDEPATH += /usr/include/freetype2/
    LIBS += -lGLEW \
            -lfreetype \
            -lftgl \
            -lportaudio
}

win32 {
    DEFINES += GLEW_STATIC FTGL_LIBRARY_STATIC
    INCLUDEPATH += ../3rd/inc/ \
                   ../3rd/inc/freetype2/
    LIBS += -L../3rd/lib/x86/ \
            advapi32.lib \
            opengl32.lib \
            glu32.lib \
            libglew32.a \
            libfreetype2.a \
            libftgl.a \
            libportaudio.a \
            swscale.lib
}

debug {
    CONFIG += _DEBUG
}

