include ../examples.conf 

TARGETS = notebook

all: $(TARGETS)

notebook: notebook.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

