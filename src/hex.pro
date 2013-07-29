
TEMPLATE = subdirs
LANGUAGE = C++
EXTRAS = control app tests
CONFIG += x86_64 ordered debug

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

