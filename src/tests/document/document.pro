QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets testlib

TEMPLATE = app
CONFIG += qtestlib



# Input
SOURCES += document.cpp


include(../tests.pri)



