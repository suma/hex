
TEMPLATE = lib
LANGUAGE = C++
TARGET = ../control
CONFIG += precompile_header staticlib

PRECOMPILED_HEADER = ../stable.h

DEPENDPATH += . \
    control \
    control/standard
INCLUDEPATH += .. . \
    control \
    control/standard

include(./control.pri)

