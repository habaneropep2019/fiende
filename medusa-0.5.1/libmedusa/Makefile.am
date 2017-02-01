NULL=

INCLUDES =\
	-I$(top_builddir) \
	$(VFS_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(XML_CFLAGS) \
	$(WERROR) \
	$(NULL)

libmedusaincludedir=$(includedir)/libmedusa

libmedusainclude_HEADERS=		\
	medusa-file-info-utilities.h	\
	medusa-index-progress.h		\
	medusa-index-service.h          \
	medusa-index-service-private.h  \
	medusa-indexed-search.h         \
	medusa-lock.h			\
	medusa-lock-file-paths.h	\
	medusa-log.h			\
	medusa-search-service.h 	\
	medusa-search-service-private.h \
	medusa-service-private.h        \
	medusa-stdio-extensions.h	\
	medusa-string.h			\
	medusa-system-state.h		\
	medusa-unindexed-search.h       \
	medusa-unsearched-locations.h	\
	medusa-utils.h			\
	$(NULL)

lib_LTLIBRARIES = libmedusa.la

libmedusa_la_SOURCES = \
	medusa-file-info-utilities.c	\
	medusa-index-progress.c 	\
	medusa-index-service.c          \
	medusa-indexed-search.c         \
	medusa-lock.c			\
	medusa-log.c			\
	medusa-search-service.c         \
	medusa-service-private.c        \
	medusa-stdio-extensions.c	\
	medusa-string.c			\
	medusa-system-state.c		\
	medusa-unindexed-search.c       \
	medusa-unsearched-locations.c	\
	medusa-utils.c			\
	$(NULL)


