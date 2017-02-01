NULL =

noinst_PROGRAMS = test-text-index test-index-service test-queue test-file-indexer test-versioned-file test-search-method test-system-state

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libmedusa-internal \
	-I$(top_srcdir)/libmedusa \
	$(VFS_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NULL)

test_text_index_SOURCES = \
	medusa-text-index-test.c

test_index_service_SOURCES = \
	medusa-index-service-test.c

test_queue_SOURCES = \
	medusa-queue-test.c

test_file_indexer_SOURCES = \
	test-file-indexer.c

test_system_state_SOURCES = \
	test-system-state.c

test_versioned_file_SOURCES = \
	medusa-versioned-file-test.c 

test_search_method_SOURCES = \
	search-method-test.c

LDADD = \
	$(top_builddir)/libmedusa-internal/libmedusa-index.la \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(VFS_LIBS) \
	$(GLIB_LIBS) \
	$(GNOME_LIBS) \
	$(GTK_LIBS) \
	$(XML_LIBS) \
	$(NULL)
