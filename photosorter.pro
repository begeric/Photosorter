#-------------------------------------------------
#
# Project created by QtCreator 2016-11-23T20:47:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = photosorter
TEMPLATE = app

INCLUDEPATH += C:/MinGW/include/epeg
LIBS += -LC:/Qt/5.7/mingw53_32/lib -llibepeg -llibjpeg -llibexif.dll

SOURCES += main.cpp\
        mainwindow.cpp \
    imagesorter.cpp \
    listpixmapprefetcher.cpp

HEADERS  += mainwindow.h \
    clickableqgraphicsscene.h \
    imagesorter.h \
    listpixmapprefetcher.h

FORMS    += mainwindow.ui

#CONFIG += console

