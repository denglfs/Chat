#-------------------------------------------------
#
# Project created by QtCreator 2014-08-12T20:23:29
#
#-------------------------------------------------

QT       += core gui
QT      +=  network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    thread.cpp

HEADERS  += mainwindow.h \
    Structor.h \
    thread.h

FORMS    += mainwindow.ui
