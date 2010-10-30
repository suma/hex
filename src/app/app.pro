
TEMPLATE = app
LANGUAGE = C++
TARGET = ../hex
CONFIG += precompile_header 

DEPENDPATH += . ..
INCLUDEPATH += . \
	../control \
	../control/standard

LIBS += -L../ -lcontrol


# Input
HEADERS += editor.h
SOURCES += editor.cpp \
	main.cpp

