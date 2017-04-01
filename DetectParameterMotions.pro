#-------------------------------------------------
#
# Project created by QtCreator 2017-03-12T20:07:31
#
#-------------------------------------------------

QT       += multimedia multimediawidgets widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = DetectParameterMotions
TEMPLATE = app


SOURCES += main.cpp\
    videoplayer.cpp \
    videowidgetsurface.cpp \
    qcustomplot.cpp \
    myutility.cpp

HEADERS  += \
    videoplayer.h \
    videowidgetsurface.h \
    qcustomplot.h \
    myutility.h

target.path = $$[QT_INSTALL_PLUGINS]/multimediawidgets/videowidget

INSTALLS += target
