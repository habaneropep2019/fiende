AUTOMAKE_OPTIONS = 1.3 # required version of automake.

SUBDIRS=popt libIDL src test

bin_SCRIPTS=orbit-config

EXTRA_DIST=ORBit.spec.in ORBit.m4 ORBit.pc.in

dist-hook: ORBit.spec
	mkdir $(distdir)/docs
	cp -pr $(srcdir)/docs/* $(distdir)/docs
	rm -rf $(distdir)/docs/CVS $(distdir)/docs/idl/CVS
	cp ORBit.spec $(distdir)

rpms: distcheck
	rpm -ta $(top_builddir)/@PACKAGE@-@VERSION@.tar.gz

m4datadir=$(datadir)/aclocal
m4data_DATA=ORBit.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= ORBit.pc
