INCLUDES = -DG_LOG_DOMAIN=\"Gdk--\" \
           -I$(srcdir)/.. -I.. \
           @GTK_CFLAGS@

lib_LTLIBRARIES = libgdkmm.la

libgdkmm_la_SOURCES = \
	bitmap.cc    colormap.cc  font.cc      main.cc      window.cc \
	color.cc     drawable.cc  gc.cc        pixmap.cc    visual.cc \
        image.cc     types.cc     cursor.cc
libgdkmm_la_LDFLAGS = -version-info $(LIBGDKMM_SO_VERSION) -release $(GTKMM_RELEASE)

libgdkmmincludedir = $(includedir)/gdk--
libgdkmminclude_HEADERS = \
	bitmap.h      colormap.h    gc.h          main.h \
	color.h       drawable.h    pixmap.h \
	font.h        types.h       window.h      visual.h \
        image.h       list.h        cursor.h







