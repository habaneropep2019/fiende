include ../examples.conf

TARGET = dialogs dialog

all: dialogs dialog

dialogs: dialogs.lo
	$(CXXLINK) $^ $(LIBS)

dialogs.lo : dialogs.cc
	$(CXXCOMPILE) 

dialog: dialog.lo
	$(CXXLINK) $^ $(LIBS)

dialog.lo : dialog.cc
	$(CXXCOMPILE) 


clean:  
	rm -Rf *.lo $(TARGET) .libs
