include ../examples.conf

TARGET = menuapp

all: menuapp

menuapp: application.lo main.lo
	$(CXXLINK) $^ $(LIBS)

main.lo : main.cc
	$(CXXCOMPILE) 

application.lo : application.cc
	$(CXXCOMPILE)

clean:  
	rm -Rf *.lo $(TARGET) .libs
