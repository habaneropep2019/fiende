include ../examples.conf

TARGETS = dial_test

all: $(TARGETS)

dial_test: gtkdial.lo dial_test.lo
	$(CXXLINK) dial_test.o gtkdial.o $(LIBS)

dial_test.lo: dial_test.cc gtkdial.h
	$(CXXCOMPILE) -c $<

gtkdial.lo: gtkdial.cc gtkdial.h
	$(CXXCOMPILE) -c $<

clean:
	rm -Rf .libs *.lo core dial_test 

