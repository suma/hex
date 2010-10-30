

TEMPLATE = app
LANGUAGE = C++
TARGET = ../hex

DEPENDPATH += . ..
INCLUDEPATH += . \
	.. \
	../control \
	../control/standard



# Input
HEADERS += editor.h
SOURCES += editor.cpp \
	main.cpp



unix {
	LIBS += ../libcontrol.a
}
win32 {
}


