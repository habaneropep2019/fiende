CXXFLAGS += $(shell sigc-config --cflags)
LDFLAGS += $(shell sigc-config --libs)
SRC = $(wildcard *.cc)
EXE = $(basename $(SRC))

.PHONY:	all clean

all:	$(EXE)

clean:
	$(RM) $(EXE)
