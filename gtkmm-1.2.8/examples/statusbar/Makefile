include ../examples.conf 

TARGETS = statusbar

all: $(TARGETS)

statusbar: statusbar.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

