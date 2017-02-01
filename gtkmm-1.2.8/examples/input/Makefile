include ../examples.conf 

TARGETS = input

all: $(TARGETS)

input : input.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

