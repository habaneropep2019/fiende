include ../examples.conf 

TARGETS = helloworld2

all: $(TARGETS)

helloworld2: helloworld2.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

