include ../examples.conf 

TARGETS = entry

all: $(TARGETS)

entry: entry.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

