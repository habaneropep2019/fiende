include ../examples.conf 

TARGETS = tree

all: $(TARGETS)

tree: tree.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

