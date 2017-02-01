NULL =

modulesdir = $(libdir)/vfs/modules

modules_LTLIBRARIES =				\
	libvfs-search.la	                \
	$(NULL)

libvfs_search_la_SOURCES = \
	search-method.c \
	$(NULL)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libmedusa \
	$(GNOME_CFLAGS) \
	$(VFS_CFLAGS) \
	$(NULL)

libvfs_search_la_LIBADD = \
	-L$(top_builddir)/libmedusa/.libs -lmedusa \
	$(VFS_LIBS) \
	$(GLIB_LIBS) \
	$(NULL)

libvfs_search_la_LDFLAGS = -export_dynamic -avoid-version

modulesconfdir=$(sysconfdir)/vfs/modules
modulesconf_DATA = search-methods.conf

EXTRA_DIST=search-methods.conf
