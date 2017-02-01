noinst_PROGRAMS =		\
	test-any		\
	test-ui			\
	test-moniker		\
	test-properties-server	\
	test-properties-client	\
	test-storage		\
	test-toolbar		\
	selector_test		\
	test-event-source 	\
	test-sensitivity	\
	test-focus		\
	test-xobject		\
	test-reparent
#	test-container 		\

bonobo_libs =						\
	$(top_builddir)/bonobo/libbonobo.la		\
	$(BONOBOX_TEST_LIBS)				\
	$(INTLLIBS)

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

test_properties_server_SOURCES =			\
	test-properties-server.c

test_properties_server_LDADD =				\
	$(bonobo_libs)

test_properties_client_SOURCES =			\
	test-properties-client.c

test_properties_client_LDADD =				\
	$(bonobo_libs)

test_storage_SOURCES = test-storage.c

test_storage_LDADD = $(bonobo_libs)

test_any_SOURCES = test-any.c

test_any_LDADD =					\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

test_ui_SOURCES = test-ui.c
test_ui_LDADD =						\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(top_builddir)/bonobo/libbonobo-print.la

test_focus_SOURCES = test-focus.c
test_focus_LDADD =					\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

test_toolbar_SOURCES = test-toolbar.c
test_toolbar_LDADD =					\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(top_builddir)/bonobo/libbonobo-print.la

test_moniker_SOURCES = test-moniker.c

test_moniker_LDADD =					\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

#
# Old, super ugly, leaky and broken.
#
#test_container_SOURCES =				\
#	test-container.c
#
#test_container_LDADD =					\
#	$(bonobo_libs)					\
#	$(top_builddir)/bonobo/libbonobox.la

selector_test_SOURCES =					\
	selector_test.c

selector_test_LDADD =					\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

test_event_source_SOURCES = test-event-source.c

test_event_source_LDADD =				\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

test_sensitivity_SOURCES = test-sensitivity.c
test_sensitivity_LDADD =				\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

test_reparent_SOURCES = test-reparent.c
test_reparent_LDADD =				\
	$(bonobo_libs)					\
	$(top_builddir)/bonobo/libbonobox.la

test_xobject_SOURCES = \
	test-xobject.c

test_xobject_LDADD = \
	$(bonobo_libs)


TESTS_ENVIRONMENT =					\
	LD_LIBRARY_PATH=$(top_builddir)/bonobo/.libs/	\
	BONOBO_PLUGIN_PATH=$(top_builddir)/storage-modules/.libs

TESTS = test-storage test-any test-moniker test-event-source test-xobject
