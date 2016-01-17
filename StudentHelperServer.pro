#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T14:49:43
#
#-------------------------------------------------

QT       += core network gui printsupport sql
#QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StudentHelperServer
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp \
    studenthelperserver.cpp \
    shquery.cpp \
    filetreeitem.cpp \
    studenthelpercontent.cpp \
#        mainwindow.cpp

#HEADERS  += mainwindow.h

#FORMS    += mainwindow.ui

HEADERS += \
    studenthelperserver.h \
    shquery.h \
    filetreeitem.h \
    studenthelpercommon.h \
    studenthelpercontent.h \

SUBDIRS += \
    ../StudentHelperClient/StudentHelperClient.pro

RESOURCES += \
    images.qrc
