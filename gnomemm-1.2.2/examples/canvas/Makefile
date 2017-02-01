include ../examples.conf

TARGET = canvas canvas_events canvas_affine

all: canvas canvas_events canvas_affine

canvas: canvas.lo
	$(CXXLINK) $^ $(LIBS)

canvas.lo : canvas.cc
	$(CXXCOMPILE) 


canvas_events: canvas_events.lo
	$(CXXLINK) $^ $(LIBS)

canvas_events.lo : canvas_events.cc
	$(CXXCOMPILE) 


canvas_affine: canvas_affine.lo
	$(CXXLINK) $^ $(LIBS)

canvas_affine.lo : canvas_affine.cc
	$(CXXCOMPILE) 

	
clean:  
	rm -Rf *.lo $(TARGET) .libs
