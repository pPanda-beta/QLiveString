#-------------------------------------------------
#
# Project created by QtCreator 2016-08-02T00:25:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QLiveString_1
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++1y
QMAKE_LFLAGS += -shared-libgcc

CONFIG += C++1y
QMAKE_LINK += -shared-libgcc


SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

CONFIG += mobility
MOBILITY = 

