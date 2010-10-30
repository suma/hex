
TEMPLATE = app
CONFIG += qtestlib

mac {
	CONFIG -= app_bundle
}
win32 {
	CONFIG += console
}

target.path = ../
sources.files = ../hex.pro
sources.path = ../
INSTALLS += target sources

DEPENDPATH += .. \
    ../control \
    ../control/standard
INCLUDEPATH += .. \
    ../control \
    ../control/standard

# Input
SOURCES += test.cpp


