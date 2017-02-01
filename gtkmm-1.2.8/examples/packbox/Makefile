include ../examples.conf 

TARGETS = packbox

all: $(TARGETS)

packbox: packbox.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

