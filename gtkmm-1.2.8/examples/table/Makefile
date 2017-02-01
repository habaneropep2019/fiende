include ../examples.conf 

TARGETS = table

all: $(TARGETS)

table: table.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)

