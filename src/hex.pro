# #####################################################################
# Automatically generated by qmake (2.01a) ? 3 ? 6 16:25:31 2009
# #####################################################################
TEMPLATE = app
TARGET = hex
DEPENDPATH += . \
    control \
    control/standard
INCLUDEPATH += . \
    control \
    control/standard

# Input
HEADERS += editor.h \
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
    control/standard/scursor.h
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
    control/standard/scursor.cpp \
    qtc-gdbmacros/gdbmacros.cpp
CONFIG += x86_64
OTHER_FILES += 
