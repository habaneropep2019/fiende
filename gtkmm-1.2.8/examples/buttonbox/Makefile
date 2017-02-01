include ../examples.conf 

TARGETS = buttonbox

all: $(TARGETS)

buttonbox: buttonbox.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

