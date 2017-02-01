NULL =

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)/intl			\
	-I$(top_srcdir)/libgnomevfs		\
	$(OAF_CFLAGS)				\
	$(VFS_GNOMEUI_CFLAGS)			\
	$(VFS_CFLAGS) $(WERROR)			\
	-DMODULES_PATH=\"$(libdir)/vfs/modules\"

noinst_PROGRAMS =				\
	test-async				\
	test-async-cancel                       \
	test-async-directory			\
	test-channel				\
	test-directory				\
	test-directory-visit			\
	test-dirop				\
	test-escape				\
	test-find-directory			\
	test-info				\
	test-mime				\
	test-mime-handlers			\
	test-mime-handlers-set			\
	test-mime-info				\
	test-seek				\
	test-shell				\
	test-symlinks				\
	test-ssl				\
	test-sync				\
	test-sync-create			\
	test-sync-write				\
	test-unlink				\
	test-uri				\
	test-xfer				\
	test-callback				\
	test-module-selftest			\
	$(NULL)

test_files=		\
	test.input	\
	test.cmds	\
	test.output

if HAVE_LIBEFS
auto_tests=auto-test
else
auto_tests=
endif

# Set up the environment so the tests can find the back end and modules
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/libgnomevfs-pthread/.libs/ GNOME_VFS_MODULE_PATH=$(top_builddir)/modules/.libs GNOME_VFS_MODULE_CONFIG_PATH=$(top_srcdir)/modules
TESTS = test-async-cancel test-escape test-uri $(auto_tests)

libraries =						\
	$(top_builddir)/libgnomevfs/libgnomevfs.la	\
	$(VFS_GNOMEGNORBA_LIBS)				\
	$(GCONF_LIBS)					\
	$(LIBEFS_LIBS)

test_async_SOURCES = test-async.c
test_async_LDADD = $(libraries)

test_async_directory_SOURCES = test-async-directory.c
test_async_directory_LDADD = $(libraries)

test_channel_SOURCES = test-channel.c
test_channel_LDADD = $(libraries)

test_seek_SOURCES = test-seek.c
test_seek_LDADD = $(libraries)

test_shell_SOURCES = test-shell.c
test_shell_LDADD = $(libraries)

test_info_SOURCES = test-info.c
test_info_LDADD = $(libraries)

test_mime_SOURCES = test-mime.c
test_mime_LDADD = $(libraries)

test_mime_handlers_SOURCES = test-mime-handlers.c
test_mime_handlers_LDADD = $(libraries)

test_mime_handlers_set_SOURCES = test-mime-handlers-set.c
test_mime_handlers_set_LDADD = $(libraries)

test_xfer_SOURCES = test-xfer.c
test_xfer_LDADD = $(libraries)

test_directory_SOURCES = test-directory.c
test_directory_LDADD = $(libraries)

test_directory_visit_SOURCES = test-directory-visit.c
test_directory_visit_LDADD = $(libraries)

test_symlinks_SOURCES = test-symlinks.c
test_symlinks_LDADD = $(libraries)

test_ssl_SOURCES = test-ssl.c
test_ssl_LDADD = $(libraries)

test_sync_SOURCES = test-sync.c
test_sync_LDADD = $(libraries)

test_sync_write_SOURCES = test-sync-write.c
test_sync_write_LDADD = $(libraries)

test_sync_create_SOURCES = test-sync-create.c
test_sync_create_LDADD = $(libraries)

test_async_cancel_SOURCES = test-async-cancel.c
test_async_cancel_LDADD = $(libraries)

test_escape_SOURCES = test-escape.c
test_escape_LDADD = $(libraries)

test_uri_SOURCES = test-uri.c
test_uri_LDADD = $(libraries)

test_unlink_SOURCES = test-unlink.c
test_unlink_LDADD = $(libraries)

test_dirop_SOURCES = test-dirop.c
test_dirop_LDADD = $(libraries)

test_find_directory_SOURCES = test-find-directory.c
test_find_directory_LDADD = $(libraries)

test_mime_info_SOURCES = test-mime-info.c
test_mime_info_LDADD = $(libraries)

test_callback_SOURCES = test-callback.c
test_callback_LDADD = $(libraries)

test_module_selftest_SOURCES = test-module-selftest.c
test_module_selftest_LDADD = $(libraries)

EXTRA_DIST =					\
	$(test_files)				\
	vfs-run.in
