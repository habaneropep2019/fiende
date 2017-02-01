include ../examples.conf

TARGETS = druid

all: $(TARGETS)

dialog_druid.lo : dialog_druid.cc
	$(CXXCOMPILE)
	
druid-window.lo : druid-window.cc
	$(CXXCOMPILE)

main.lo : main.cc
	$(CXXCOMPILE)

druid: main.lo druid-window.lo dialog_druid.lo
	$(CXXLINK) $^ $(LIBS)

clean:
	rm -Rf *.lo $(TARGETS) .libs
