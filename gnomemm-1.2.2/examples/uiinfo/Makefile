include ../examples.conf

TARGETS = uiinfo1 uiinfo2 radio

all: $(TARGETS)

uiinfo1: uiinfo1.cc
	$(CXXBUILD)

uiinfo2: uiinfo2.cc
	$(CXXBUILD)

radio: radio.cc
	$(CXXBUILD)

#animator.lo : animator.cc
#	$(CXXCOMPILE)

clean:  
	rm -Rf *.lo $(TARGETS) .libs
