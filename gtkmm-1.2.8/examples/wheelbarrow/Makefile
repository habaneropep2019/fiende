include ../examples.conf 

TARGETS = wheelbarrow

all: $(TARGETS)

wheelbarrow: wheelbarrow.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

