include ../examples.conf 

TARGETS = frame

all: $(TARGETS)

frame: frame.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

