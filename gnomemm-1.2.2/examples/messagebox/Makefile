include ../examples.conf

TARGETS = messagebox

all: $(TARGETS)
	
messagebox-window.lo : messagebox-window.cc
	$(CXXCOMPILE)

main.lo : main.cc
	$(CXXCOMPILE)

messagebox: main.lo messagebox-window.lo
	$(CXXLINK) $^ $(LIBS)

clean:
	rm -Rf *.lo $(TARGETS) .libs
