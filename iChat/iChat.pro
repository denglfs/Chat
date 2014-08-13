#-------------------------------------------------
#
# Project created by QtCreator 2014-08-07T12:35:28
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = iChat
TEMPLATE = app


SOURCES += \
    chat.cpp \
    main.cpp \
    mainwindow.cpp \
    receiver.cpp \
    sender.cpp

HEADERS  += \
    chat.h \
    mainwindow.h \
    receiver.h \
    sender.h \
    Structor.h

RESOURCES += \
    src.qrc

FORMS += \
    chat.ui \
    dialog.ui \
    mainwindow.ui \
    receiver.ui \
    sender.ui

OTHER_FILES +=
