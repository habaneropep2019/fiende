LDFLAGS		= `gnome-config --libs panelmm`
CPPFLAGS	= `gnome-config --cflags panelmm`

SRCS		= hello.cc
BINS		= hello
DIST_FILES	= $(SRCS) Makefile

all: $(BINS)

clean:
	rm -f $(BINS)

distdir:
	rm -rf $(distdir)
	mkdir -p $(distdir)
	cp -f $(DIST_FILES) $(distdir)
