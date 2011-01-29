

TEMPLATE = app
LANGUAGE = C++
TARGET = ../hex

DEPENDPATH += . ..
INCLUDEPATH += . \
	.. \
	../control \
	../control/standard



# Input
HEADERS += editor.h \
	mainform.h \
	writefileopration.h \
	ui_main.h

SOURCES += editor.cpp \
	mainform.cpp \
	writefileopration.cpp \
	main.cpp

FORMS = main.ui


unix {
	LIBS += ../libcontrol.a
}
win32 {
}


