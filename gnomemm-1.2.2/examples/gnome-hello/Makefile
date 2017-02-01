include ../examples.conf

TARGET = gnome-hello

all: gnome-hello

gnome-hello: application.lo hello.lo
	$(CXXLINK) $^ $(LIBS)

hello.lo : hello.cc
	$(CXXCOMPILE) 

application.lo : application.cc
	$(CXXCOMPILE)

clean:  
	rm -Rf *.lo $(TARGET) .libs
