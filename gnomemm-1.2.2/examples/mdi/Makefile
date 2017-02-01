include ../examples.conf

TARGETS = mdi mdi2

all: $(TARGETS)

mdi-window.lo : mdi-window.cc
	$(CXXCOMPILE)

main.lo : main.cc
	$(CXXCOMPILE)

mdi2: main.lo mdi-window.lo
	$(CXXLINK) $^ $(LIBS)

mdi.lo: mdi.cc
	$(CXXCOMPILE)

mdi: mdi.lo 
	$(CXXLINK) $^ $(LIBS)

clean:
	rm -Rf *.lo $(TARGETS) .libs
