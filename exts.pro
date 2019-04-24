QMAKE_CC = gcc-8 -std=c11

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        common.c \
        main.c

HEADERS += \
    common.h
