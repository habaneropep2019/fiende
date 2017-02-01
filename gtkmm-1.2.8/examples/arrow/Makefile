include ../examples.conf 

TARGETS = arrow direction

all: $(TARGETS)

arrow: arrow.cc
	$(CXXBUILD)

direction: direction.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)
