
TEMPLATE = subdirs
QT += testlib
SUBDIRS = document

mac {
	CONFIG -= app_bundle
}
win32 {
	CONFIG += console
}


unix {
	LIBS += ../libcontrol.a
}

