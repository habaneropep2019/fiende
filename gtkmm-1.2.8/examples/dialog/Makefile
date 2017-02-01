include ../examples.conf 

TARGETS = dialog1 dialog2 dialog3

all: $(TARGETS)

dialog1: dialog1.cc
	$(CXXBUILD)

dialog2: dialog2.cc
	$(CXXBUILD)

dialog3: dialog3.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)
