include ../examples.conf

TARGETS = dock

all: $(TARGETS)

dock-window.lo : dock-window.cc
	$(CXXCOMPILE)

main.lo : main.cc
	$(CXXCOMPILE)

dock: main.lo dock-window.lo
	$(CXXLINK) $^ $(LIBS)

clean:
	rm -Rf *.lo $(TARGETS) .libs
