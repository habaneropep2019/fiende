include ../examples.conf 

TARGETS = aspectframe

all: $(TARGETS)

aspectframe: aspectframe.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)
