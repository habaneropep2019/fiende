include ../examples.conf

TARGETS = stock

all: $(TARGETS)

stock: stock.cc
	$(CXXBUILD)

clean:
	rm -Rf *.lo $(TARGETS) .libs
