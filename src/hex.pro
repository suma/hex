
TEMPLATE = subdirs
LANGUAGE = C++
EXTRAS = control app tests
CONFIG += x86_64 ordered

for(dir, EXTRAS) {
	exists($$dir) {
		SUBDIRS += $$dir
	}
}



