include ../examples.conf

TARGETS = clock

all: $(TARGETS)
	
clock-window.lo : clock-window.cc
	$(CXXCOMPILE)

main.lo : main.cc
	$(CXXCOMPILE)

clock: main.lo clock-window.lo
	$(CXXLINK) $^ $(LIBS)

clean:
	rm -Rf *.lo $(TARGETS) .libs
