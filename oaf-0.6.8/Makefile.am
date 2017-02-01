NULL = 

SUBDIRS=		\
	po 		\
	intl 		\
	docs 		\
	idl 		\
	liboaf 		\
	api-docs	\
	oafd 		\
	utils 		\
	test		\
	$(NULL)

bin_SCRIPTS=oaf-config

EXTRA_DIST=					\
	oaf-config.in				\
	oaf.m4					\
	oafConf.sh.in				\
	oafConf.sh				\
	oaf.spec.in				\
	oaf-config.xml				\
	oaf-config.xml.sample			\
	oaf.spec				\
	xml-i18n-extract.in			\
	xml-i18n-merge.in			\
	xml-i18n-update.in			\
	AUTHORS					\
	COPYING					\
	ChangeLog				\
	NEWS					\
	README					\
	$(NULL)

oafconfigdir=$(sysconfdir)/oaf
oafconfig_DATA = 		\
	oaf-config.xml		\
	oaf-config.xml.sample	\
	$(NULL)

confexecdir = $(libdir)
confexec_DATA = oafConf.sh

m4datadir=$(datadir)/aclocal
m4data_DATA=oaf.m4

#dist-hook: oaf.spec
#	cp oaf.spec $(distdir)
