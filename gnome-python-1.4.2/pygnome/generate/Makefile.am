defsdir = $(datadir)/pygtk/1.2/defs
defs_DATA = gnome.defs gnomeui.defs gnome-enums.defs zvt.defs xmhtml.defs \
 applet.defs capplet.defs gtkhtml.defs

EXTRA_DIST = $(defs_DATA)

IGNORE_FILES = \
  gnome.ignore gnomeui.ignore zvt.ignore xmhtml.ignore gtkhtml.ignore

all: $(IGNORE_FILES)

distclean-local:
	rm -f $(IGNORE_FILES)

gnome.ignore: $(srcdir)/../gnomemodule.c
	echo "# ignore list for gnomemodule" > gnome.ignore
	echo "# (automatically generated)" >> gnome.ignore
	sed -n 's/^.*_wrap_\([a-zA-Z0-9_]*\),.*$$/\1/p' \
		< $(srcdir)/../gnomemodule.c >> gnome.ignore

gnomeui.ignore: $(srcdir)/../gnomeuimodule.c
	echo "# ignore list for gnomeuimodule" > gnomeui.ignore
	echo "# (automatically generated)" >> gnomeui.ignore
	sed -n 's/^.*_wrap_\([a-zA-Z0-9_]*\),.*$$/\1/p' \
		< $(srcdir)/../gnomeuimodule.c >> gnomeui.ignore

zvt.ignore: $(srcdir)/../zvtmodule.c
	echo "# ignore list for zvtmodule" > zvt.ignore
	echo "# (automatically generated)" >> zvt.ignore
	sed -n 's/^.*_wrap_\([a-zA-Z0-9_]*\),.*$$/\1/p' \
		< $(srcdir)/../zvtmodule.c >> zvt.ignore

xmhtml.ignore: $(srcdir)/../gtkxmhtmlmodule.c
	echo "# ignore list for xmhtmlmodule" > xmhtml.ignore
	echo "# (automatically generated)" >> xmhtml.ignore
	sed -n 's/^.*_wrap_\([a-zA-Z0-9_]*\),.*$$/\1/p' \
		< $(srcdir)/../gtkxmhtmlmodule.c >> xmhtml.ignore

gtkhtml.ignore: $(srcdir)/../gtkhtmlmodule.c
	echo "# ignore list for gtkhtmlmodule" > gtkhtml.ignore
	echo "# (automatically generated)" >> gtkhtml.ignore
	sed -n 's/^.*_wrap_\([a-zA-Z_]*\),.*$$/\1/p' \
	  < $(srcdir)/../gtkhtmlmodule.c >> gtkhtml.ignore

