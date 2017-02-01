SUBDIRS = generate gnome

INCLUDES = $(PYTHON_INCLUDES) $(GNOME_INCLUDEDIR) \
	$(LIBGLADE_CFLAGS)

if WITH_APPLETS
appletmodule = _appletmodule.la
endif
if WITH_CAPPLET
cappletmodule = _cappletmodule.la
endif
if WITH_GLADEGNOME
gladegnomemodule = _gladegnomemodule.la
endif

if BUILD_GTKHTML
gtkhtmlmodule = _gtkhtmlmodule.la
gtkhtmlpython = gtkhtml.py
else
gtkhtmlmodule =
gtkhtmlpython =
endif

python_PYTHON = gettext.py $(gtkhtmlpython)

pyexec_LTLIBRARIES = _gnomemodule.la _gnomeuimodule.la \
	_zvtmodule.la _gtkxmhtmlmodule.la $(appletmodule) $(cappletmodule)\
	$(gladegnomemodule) $(gtkhtmlmodule)

_gnomemodule_la_LDFLAGS = -module -avoid-version
_gnomemodule_la_SOURCES = gnomemodule.c
_gnomemodule_la_LIBADD = $(GNOME_LIBDIR) $(GNOME_LIBS)

_gnomeuimodule_la_LDFLAGS = -module -avoid-version
_gnomeuimodule_la_SOURCES = gnomeuimodule.c
_gnomeuimodule_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS)

_zvtmodule_la_LDFLAGS = -module -avoid-version
_zvtmodule_la_SOURCES = zvtmodule.c
_zvtmodule_la_LIBADD = $(GNOME_LIBDIR) $(ZVT_LIBS) $(GDK_IMLIB_LIBS)

_gtkxmhtmlmodule_la_LDFLAGS = -module -avoid-version
_gtkxmhtmlmodule_la_SOURCES = gtkxmhtmlmodule.c
_gtkxmhtmlmodule_la_LIBADD = $(GNOME_LIBDIR) $(GTKXMHTML_LIBS)

_appletmodule_la_LDFLAGS = -module -avoid-version
_appletmodule_la_SOURCES = appletmodule.c
_appletmodule_la_LIBADD = $(GNOME_LIBDIR) $(GNOME_APPLETS_LIBS)


_cappletmodule_la_LDFLAGS = -module -avoid-version
_cappletmodule_la_SOURCES = cappletmodule.c
_cappletmodule_la_LIBADD = $(GNOME_LIBDIR) $(GNOME_CAPPLET_LIBS)

_gladegnomemodule_la_LDFLAGS = -module -avoid-version
_gladegnomemodule_la_SOURCES = gladegnomemodule.c
_gladegnomemodule_la_LIBADD = $(LIBGLADE_LIBS)

_gtkhtmlmodule_la_LDFLAGS = -module -avoid-version
_gtkhtmlmodule_la_SOURCES = gtkhtmlmodule.c
_gtkhtmlmodule_la_LIBADD = $(GTKHTML_LIBS)

GENERATE = \
  $(top_srcdir)/pygtk/generate/scmexpr.py \
  $(top_srcdir)/pygtk/generate/generate.py
GNOME_DEFS = \
  $(srcdir)/generate/gnome.defs       $(srcdir)/generate/gnomeui.defs \
  $(srcdir)/generate/zvt.defs         $(srcdir)/generate/xmhtml.defs \
  $(srcdir)/generate/gnome-enums.defs $(srcdir)/generate/gtkhtml.defs

GEN_FILES = \
  gnomemodule_defs.c     gnomemodule_impl.c \
  gnomeuimodule_defs.c   gnomeuimodule_impl.c \
  zvtmodule_defs.c       zvtmodule_impl.c \
  gtkxmhtmlmodule_defs.c gtkxmhtmlmodule_impl.c \
  appletmodule_defs.c    appletmodule_impl.c \
  cappletmodule_defs.c   cappletmodule_impl.c \
  gtkhtml_defs.c         gtkhtml_impl.c

$(GEN_FILES): $(srcdir)/mkgnome.py $(GENERATE) $(GNOME_DEFS)
	$(PYTHON) $(srcdir)/mkgnome.py $(srcdir)

distclean-local:
	rm -f $(GEN_FILES)

gnomemodule.lo: gnomemodule.c gnomemodule_defs.c gnomemodule_impl.c
gnomeuimodule.lo: gnomeuimodule.c gnomeuimodule_defs.c gnomeuimodule_impl.c
zvtmodule.lo: zvtmodule.c zvtmodule_defs.c zvtmodule_impl.c
gtkxmhtmlmodule.lo: gtkxmhtmlmodule.c gtkxmhtmlmodule_defs.c gtkxmhtmlmodule_impl.c
appletmodule.lo: appletmodule.c appletmodule_defs.c appletmodule_impl.c
cappletmodule.lo: cappletmodule.c cappletmodule_defs.c cappletmodule_impl.c
gtkhtmlmodule.lo: gtkhtmlmodule.c gtkhtml_defs.c gtkhtml_impl.c

EXTRA_DIST = \
	mkgnome.py \
	gtkhtml.py \
	examples/html_demo.py \
	examples/canvas.py \
	examples/stock_demo.py \
	examples/ted_demo.py \
	examples/uiinfo_demo.py \
	examples/zterm.py \
	examples/fifteen.py \
	examples/canvas-example.py \
	examples/clock-applet.py \
	examples/bookmarks-applet.py \
	examples/about.py \
	examples/calculator.py \
	examples/colorpicker.py \
	examples/dial.py \
	examples/gtkhtml_demo.py
