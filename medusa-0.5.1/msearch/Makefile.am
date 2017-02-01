NULL = 

bin_PROGRAMS = msearch

msearch_SOURCES = \
	medusa-command-line-search.c \
	$(NULL)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libmedusa-internal \
	-I$(top_srcdir)/libmedusa \
	$(GLIB_CFLAGS) \
	$(VFS_CFLAGS) \
	$(XML_CFLAGS) \
	$(NULL)

msearch_LDADD = \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(top_builddir)/libmedusa-internal/libmedusa-index.la \
	$(VFS_LIBS) \
	$(GLIB_LIBS) \
	$(XML_LIBS) \
	$(NULL)

