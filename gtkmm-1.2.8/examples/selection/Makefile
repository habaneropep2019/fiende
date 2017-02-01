
CXX = $(shell awk -F% '/@CXX@/ {print $$3}' ../../config.status )

all: gettargets setselection

gettargets: gettargets.cc
	$(CXX) `gtkmm-config --cflags` $< -o $@ `gtkmm-config --libs`

setselection: setselection.cc
	$(CXX) `gtkmm-config --cflags` $< -o $@ `gtkmm-config --libs`

clean:
	rm -f *.o gettargets setselection

