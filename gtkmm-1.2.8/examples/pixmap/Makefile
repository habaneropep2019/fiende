include ../examples.conf 

TARGETS = pixmap

all: $(TARGETS)

pixmap: pixmap.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

