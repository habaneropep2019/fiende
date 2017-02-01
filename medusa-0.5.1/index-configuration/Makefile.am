NULL=


sbin_PROGRAMS = medusa-test-indexing-status

medusa_test_indexing_status_SOURCES = \
	medusa-test-indexing-status.c \
	$(NULL)

INCLUDES = \
	$(GLIB_CFLAGS) \
	$(GNOME_CFLAGS) \
	$(NULL)	

medusa_test_indexing_status_LDADD = \
	$(GLIB_LIBS) \
	$(GNOME_LIBS) \
	$(VFS_LIBS) \
	$(XML_LIBS) \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(NULL)


file_indexdir = $(sysconfdir)/medusa

file_index_DATA = \
	file-index-stoplist \
	mount-type-stoplist \
	$(NULL)


medusacronconfdir=$(sysconfdir)/cron.daily

medusacronconf_DATA = \
	medusa.cron \
	$(NULL)


medusaidledconfdir=$(sysconfdir)/profile.d

medusaidledconf_DATA = \
	medusa-idled.sh \
	medusa-idled.csh \
	$(NULL)

medusainitdir=$(prefix)/tmp

medusainit_DATA = \
	medusa-init \
	$(NULL)

# XXX There must be some better way than this to get automake to install
#     an empty directory.  I'm doing this cargo-cult style.

dummydir = $(localstatedir)/medusa

dummy_DATA = \
	.dummy \
	$(NULL)

medusa_conf_DATA = \
	medusa.conf \
	$(NULL)

medusa_confdir = $(sysconfdir)/medusa

EXTRA_DIST = 			\
	$(file_index_DATA) 	\
	$(medusacronconf_DATA) 	\
	$(medusaidledconf_DATA) \
	$(medusainit_DATA)	\
	$(dummy_DATA)		\
	$(medusa_conf_DATA)     \
	$(NULL)


