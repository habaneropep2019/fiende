include ../examples.conf 

TARGETS = ctree 

all: $(TARGETS)

ctree: ctree.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)
