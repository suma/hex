

TEMPLATE = app
LANGUAGE = C++
TARGET = ../hex

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


