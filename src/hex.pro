
TEMPLATE = subdirs
LANGUAGE = C++
EXTRAS = control app tests
CONFIG += x86_64 ordered precompile_header

for(dir, EXTRAS) {
	exists($$dir) {
		SUBDIRS += $$dir
	}
}

# Use Precompiled headers (PCH)
PRECOMPILED_HEADER = stable.h


OTHER_FILES += 

