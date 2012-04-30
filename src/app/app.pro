

TEMPLATE = app
LANGUAGE = C++
TARGET = ../hex
CONFIG += precompile_header

PRECOMPILED_HEADER = ../stable.h

DEPENDPATH += . ..
INCLUDEPATH += . \
	.. \
	../control \
	../control/standard



# Input
HEADERS += \
	mainform.h \
	writefileopration.h \
	hexapp.h \
	ui_main.h

SOURCES += \
	mainform.cpp \
	writefileopration.cpp \
	hexapp.cpp \
	main.cpp

FORMS = main.ui


unix {
	LIBS += ../libcontrol.a
}
win32 {
}


