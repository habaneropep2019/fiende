# Process this file with automake to produce Makefile.in.

INCLUDES = \
	-I$(top_srcdir)/libs \
        $(XML_CFLAGS)

LDADD = \
        $(XML_LIBS) \
	$(top_builddir)/libs/libscrollkeeper.la

bin_PROGRAMS = scrollkeeper-preinstall scrollkeeper-update scrollkeeper-install scrollkeeper-uninstall scrollkeeper-get-cl 

scrollkeeper_preinstall_SOURCES = \
	preinstall.c

scrollkeeper_update_SOURCES = \
	update.c

scrollkeeper_install_SOURCES = \
	install.c

scrollkeeper_uninstall_SOURCES = \
	uninstall.c

scrollkeeper_get_cl_SOURCES = \
	get-cl.c
