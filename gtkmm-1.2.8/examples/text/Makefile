include ../examples.conf 

TARGETS = text

all: $(TARGETS)

text: text.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

