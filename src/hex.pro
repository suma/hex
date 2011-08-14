
TEMPLATE = subdirs
LANGUAGE = C++
EXTRAS = control app tests
CONFIG += x86_64 ordered debug

for(dir, EXTRAS) {
	exists($$dir) {
		SUBDIRS += $$dir
	}
}


app.depends = control
tests.depends = control

