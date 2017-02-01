NULL = 

bin_PROGRAMS = medusa-searchd 

medusa_searchd_SOURCES = \
	medusa-file-search-daemon.c \
	medusa-authenticate.c \
	medusa-file-search-parse-transmission.c \
	$(NULL)

noinst_HEADERS = \
	medusa-authenticate.h \
	medusa-file-search-parse-transmission.h \
	$(NULL)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libmedusa-internal \
	-I$(top_srcdir)/libmedusa \
	$(GLIB_CFLAGS) \
	$(VFS_CFLAGS) \
	$(NULL)

medusa_searchd_LDADD = \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(top_builddir)/libmedusa-internal/libmedusa-index.la \
	$(VFS_LIBS) \
	$(GLIB_LIBS) \
	$(GNOME_LIBS) \
	$(OAF_LIBS) \
	$(XML_LIBS) \
	$(NULL)
