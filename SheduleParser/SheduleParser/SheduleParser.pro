QT += core network sql
QT -= gui

TARGET = SheduleParser
CONFIG += console c++11
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    databaseprovider.cpp \
    parser.cpp \
    scheduler.cpp \
    webloader.cpp

HEADERS += \
    databaseprovider.h \
    parser.h \
    scheduler.h \
    webloader.h

