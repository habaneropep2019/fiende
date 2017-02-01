include ../examples.conf 

TARGETS = drawingarea

all: $(TARGETS)

drawingarea: drawingarea.cc
	$(CXXBUILD)

clean: 
	rm -Rf .libs *.o *.lo core $(TARGETS)
