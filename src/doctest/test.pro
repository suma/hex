######################################################################
# Automatically generated by qmake (2.01a) ? 2? 2 00:53:34 2010
######################################################################

QT += testlib
CONFIG += console
TARGET = test
DEPENDPATH += . ../control
INCLUDEPATH += . ../control

mac {
	CONFIG -= app_bundle
}

win32 {
	CONFIG += console
}

unix {
	LIBS += ../libcontrol.a
}

HEADERS += \
	rectest.h

SOURCES += \
	doctest.cpp


CONFIG += x86_64
