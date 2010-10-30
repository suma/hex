
TEMPLATE = app
LANGUAGE = C++
TARGET = hex
EXTRAS = tests
CONFIG += x86_64 precompile_header

for(dir, EXTRAS) {
	exists($$dir) {
		SUBDIRS += $$dir
	}
}

# Use Precompiled headers (PCH)
PRECOMPILED_HEADER = stable.h


DEPENDPATH += . \
    control \
    control/standard
INCLUDEPATH += . \
    control \
    control/standard

# Input
HEADERS += \
	stable.h \
	editor.h \
    control/color.h \
    control/control.h \
    control/cursor.h \
    control/document.h \
    control/document_i.h \
    control/highlight.h \
    control/keyboard.h \
    control/qfragmentmap_p.h \
    control/standard.h \
    control/view.h \
    control/standard/addressview.h \
    control/standard/hexview.h \
	control/standard/textview.h \
	control/standard/hexcursor.h \
	control/standard/textcursor.h \
	control/standard/cursorutil.h \
	control/standard/textdecodehelper.h
SOURCES += editor.cpp \
    main.cpp \
    control/control.cpp \
    control/cursor.cpp \
    control/document.cpp \
    control/document_i.cpp \
    control/highlight.cpp \
    control/qfragmentmap.cpp \
    control/view.cpp \
    control/standard/addressview.cpp \
    control/standard/hexview.cpp \
	control/standard/textview.cpp \
	control/standard/hexcursor.cpp \
	control/standard/textcursor.cpp \
	control/standard/textdecodehelper.cpp \
	build/qtc-gdbmacros/gdbmacros.cpp


OTHER_FILES += 

