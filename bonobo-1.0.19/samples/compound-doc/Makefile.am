
SUBDIRS = container bonobo-hello

bin_PROGRAMS = bonobo-sample-paint bonobo-sample-canvas-item

GPRINT_LIBS = $(top_builddir)/bonobo/libbonobo-print.la

INCLUDES =						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

bonobo_sample_paint_SOURCES =	\
	paint-component-simple.c

bonobo_sample_paint_LDADD =				\
	$(top_builddir)/bonobo/libbonobo.la		\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(top_builddir)/bonobo/libbonobo-print.la	\
	$(BONOBOX_TEST_LIBS)				\
	$(INTLLIBS)

bonobo_sample_canvas_item_SOURCES =	\
	sample-canvas-item.c

bonobo_sample_canvas_item_LDADD = 			\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)			\
	$(INTLLIBS)

oafdir = $(datadir)/oaf
oaf_in_files = Bonobo_Sample_Paint.oaf.in	\
	       Bonobo_Sample_CanvasItem.oaf.in

oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST = $(oaf_in_files) $(oaf_DATA)

