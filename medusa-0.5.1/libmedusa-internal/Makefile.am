NULL=

lib_LTLIBRARIES = libmedusa-index.la

DEFS = \
	$(NULL)

INCLUDES =\
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libmedusa-internal \
	-I$(top_srcdir)/libmedusa \
	$(GLIB_CFLAGS) \
	$(VFS_CFLAGS) \
	$(XML_CFLAGS) \
	$(WERROR) \
	$(NULL)

dependency_static_libs = \
	$(top_builddir)/medusa-idled/libmedusa-idle.la \
	$(NULL)

libmedusa_index_la_LIBADD = \
	$(dependency_static_libs) \
	$(NULL)

libmedusa_indexincludedir=$(includedir)/libmedusa-index

noinst_HEADERS = \
	medusa-byte.h \
	medusa-conf.h \
	medusa-db-update.h \
	medusa-debug.h \
	medusa-file-index-encoders.h \
	medusa-file-index-queries.h \
	medusa-file-index.h \
	medusa-file-information.h \
	medusa-hash.h \
	medusa-index-filenames.h \
	medusa-io-handler.h \
	medusa-keyword-set.h \
	medusa-lock.h \
	medusa-lock-file-paths.h \
	medusa-master-db-private.h \
	medusa-master-db.h \
	medusa-query-clauses.h \
	medusa-queue.h \
	medusa-query-optimizations.h \
	medusa-query-optimizations-private.h \
	medusa-rdb-fields.h \
	medusa-rdb-file.h \
	medusa-rdb-index.h \
	medusa-rdb-record.h \
	medusa-rdb-table-private.h \
	medusa-rdb-table.h \
	medusa-search-uri.h \
	medusa-string-list.h \
	medusa-test-conf.h \
	medusa-test.h \
	medusa-text-index-mime-module.h \
	medusa-text-index-plaintext-module.h \
	medusa-text-index-private.h \
	medusa-text-index-queries.h \
	medusa-text-index.h \
	medusa-tokenize.h \
	medusa-uri-list-encoders.h \
	medusa-uri-list-private.h \
	medusa-uri-list-queries.h \
	medusa-uri-list.h \
	medusa-versioned-file.h \
	$(NULL)



libmedusa_index_la_SOURCES = \
	medusa-byte.c \
	medusa-debug.c \
	medusa-file-index-encoders.c \
	medusa-file-index-queries.c \
	medusa-file-index.c \
	medusa-file-information.c \
	medusa-hash.c \
	medusa-io-handler.c \
	medusa-index-filenames.c \
	medusa-keyword-set.c \
	medusa-master-db.c \
	medusa-query-clauses.c \
	medusa-queue.c \
	medusa-query-optimizations.c \
	medusa-rdb-fields.c \
	medusa-rdb-file.c \
	medusa-rdb-table.c \
	medusa-search-uri.c \
	medusa-string-list.c \
	medusa-test.c \
	medusa-text-index-mime-module.c \
	medusa-text-index-plaintext-module.c \
	medusa-text-index-queries.c \
	medusa-text-index.c \
	medusa-tokenize.c \
	medusa-uri-list-encoders.c \
	medusa-uri-list-queries.c \
	medusa-uri-list.c \
	medusa-versioned-file.c \
	$(NULL)

