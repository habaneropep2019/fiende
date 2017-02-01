include ../examples.conf 

TARGETS = packer

all: $(TARGETS)

packer: packer.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

