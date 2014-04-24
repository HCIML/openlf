#-------------------------------------------------
#
# Project created by QtCreator 2014-04-23T15:07:06
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ViewLF
TEMPLATE = app


SOURCES += main.cpp\
        viewer.cpp

LIBS = -L./../../build/lib -llibopenlf
LIBS += -L/usr/local/lib
LIBS += -L/usr/lib/X86_64-linux-gnu

LIBS += -lhdf5
LIBS += -lhdf5_cpp
LIBS += -lhdf5_hl
LIBS += -lhdf5_hl_cpp

LIBS += -lboost_iostreams
LIBS += -lboost_context
LIBS += -lboost_filesystem
LIBS += -lboost_system

LIBS += -lvigraimpex

HEADERS  += viewer.h
INCLUDEPATH += ./../../includes

FORMS    += viewer.ui