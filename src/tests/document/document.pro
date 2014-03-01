QT += core gui testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets testlib

TEMPLATE = app



# Input
SOURCES += document.cpp


include(../tests.pri)



