
TEMPLATE = subdirs
LANGUAGE = C++
EXTRAS = control app tests
CONFIG += x86_64 ordered debug c++14
QMAKE_CXXFLAGS += -std=c++14

for(dir, EXTRAS) {
	exists($$dir) {
		SUBDIRS += $$dir
	}
}

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}


app.depends = control
tests.depends = control

