NULL=

noinst_LTLIBRARIES = libmedusa-idle.la 
bin_PROGRAMS = medusa-idled


libmedusa_idle_la_SOURCES = \
	medusa-idled-client.c \
	$(NULL)	

libmedusa_idle_la_LDFLAGS = -static

libmedusa_idleincludedir = $(includedir)/libmedusa

libmedusa_idleinclude_HEADERS = \
	medusa-idled-client.h \
	$(NULL)

medusa_idled_SOURCES = \
	medusa-idled.c \
	x-screensaver-code.c \
	$(NULL)


noinst_HEADERS = \
	x-screensaver-code.h \
	medusa-idled-private.h \
	$(NULL)

INCLUDES = \
	-I$(top_srcdir) \
	$(X_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NULL)	

medusa_idled_LDADD = \
	$(GLIB_LIBS) \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(XML_LIBS) \
	$(VFS_LIBS) \
	$(X_LIBS) \
	$(GTK_LIBS) \
	$(DB_LIB) \
	-lXt \
	$(NULL)


