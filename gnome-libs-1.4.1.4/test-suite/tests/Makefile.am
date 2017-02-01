noinst_PROGRAMS = fileconvert metadata parse-path skel split trigs

INCLUDES =					\
	-I$(top_builddir) -I$(top_srcdir)	\
	$(SUPPORTINCS)				\
	$(GDK_IMLIB_CFLAGS)			\
	-I$(includedir)				\
	@GTK_CFLAGS@

LDADD = $(top_builddir)/libgnomeui/libgnomeui.la	\
	$(top_builddir)/libgnome/libgnome.la		\
	$(top_builddir)/libart_lgpl/libart_lgpl.la	\
	@LIBSUPPORT@ $(LIBGNOMEUI_LIBS) $(INTLLIBS)

EXTRA_DIST = fileconvert.in lynxdump.sh types.cvt trigs.in
