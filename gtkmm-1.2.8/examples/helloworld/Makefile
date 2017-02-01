include ../examples.conf 

TARGETS = helloworld

all: $(TARGETS)

helloworld: helloworld.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

