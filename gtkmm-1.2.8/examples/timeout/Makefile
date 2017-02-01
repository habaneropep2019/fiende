include ../examples.conf 

TARGETS = timeout

all: $(TARGETS)

timeout : timeout.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

