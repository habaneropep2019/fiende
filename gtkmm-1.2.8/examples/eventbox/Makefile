include ../examples.conf 

TARGETS = eventbox

all: $(TARGETS)

eventbox: eventbox.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

