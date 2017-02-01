include ../examples.conf 

TARGETS = rulers

all: $(TARGETS)

rulers: rulers.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

