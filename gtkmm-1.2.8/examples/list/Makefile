include ../examples.conf 

TARGETS = list

all: $(TARGETS)

list: list.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

