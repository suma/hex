
INCLUDEPATH = ../../

mac {
	CONFIG -= app_bundle
}

win32 {
	CONFIG += console
}

unix {
	LIBS += ../../libcontrol.a
}
