
TEMPLATE = lib
LANGUAGE = C++
TARGET = ../control
CONFIG += precompile_header 
PRECOMPILED_HEADER = ../stable.h

DEPENDPATH += . \
    control \
    control/standard
INCLUDEPATH += .. . \
    control \
    control/standard

# Input
HEADERS += \
    color.h \
    control.h \
    cursor.h \
    document.h \
    document_i.h \
    highlight.h \
    keyboard.h \
    qfragmentmap_p.h \
    standard.h \
    view.h \
    standard/addressview.h \
    standard/hexview.h \
	standard/textview.h \
	standard/hexcursor.h \
	standard/textcursor.h \
	standard/cursorutil.h \
	standard/textdecodehelper.h
SOURCES += \
    control.cpp \
    cursor.cpp \
    document.cpp \
    document_i.cpp \
    highlight.cpp \
    qfragmentmap.cpp \
    view.cpp \
    standard/addressview.cpp \
    standard/hexview.cpp \
	standard/textview.cpp \
	standard/hexcursor.cpp \
	standard/textcursor.cpp \
	standard/textdecodehelper.cpp


OTHER_FILES += 

