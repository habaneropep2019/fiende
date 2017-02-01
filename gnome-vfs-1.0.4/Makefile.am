if PLATFORM_GNOME_2
SUBDIRS_GNOME=doc
else
SUBDIRS_GNOME=doc test
endif

SUBDIRS =					\
	libgnomevfs				\
	libgnomevfs-pthread			\
	idl					\
	modules					\
	man					\
	devel-docs				\
	$(SUBDIRS_GNOME)			\
	po


EXTRA_DIST =					\
	HACKING					\
	TODO					\
	autogen.sh				\
	gnome-vfs.spec				\
	vfsConf.sh.in				\
	xml-i18n-extract.in			\
	xml-i18n-merge.in			\
	xml-i18n-update.in			\
	gnome-vfs-config.in

bin_SCRIPTS = $(top_builddir)/gnome-vfs-config

confexecdir=$(libdir)
confexec_DATA =					\
	vfsConf.sh

## [The following is shamelessly copied from gnome-print.]

## We create `vfsConf.sh' and `gnome-vfs-config' here and
## not from configure because we want to get the paths expanded correctly.
## Macros like srcdir are given the value NONE in configure if the user doesn't
## specify them (this is an autoconf feature, not a bug).

vfsConf.sh: vfsConf.sh.in Makefile
	sed -e 's?\@VFS_LIBDIR\@?$(VFS_LIBDIR)?g' \
	    -e 's?\@VFS_INCLUDEDIR\@?$(VFS_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@VFS_LIBS\@?$(VFS_LIBS)?g' \
	      < $(srcdir)/vfsConf.sh.in > vfsConf.tmp \
	  && mv vfsConf.tmp vfsConf.sh

gnome-vfs-config: gnome-vfs-config.in Makefile
	sed -e 's?\@LIBDIR\@?$(libdir)?g' \
	    -e 's?\@VFS_LIBDIR\@?$(VFS_LIBDIR)?g' \
	    -e 's?\@VFS_INCLUDEDIR\@?$(VFS_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@VFS_LIBS\@?$(VFS_LIBS)?g' \
	      < $(srcdir)/gnome-vfs-config.in > gnome-vfs-config.tmp \
	  && mv gnome-vfs-config.tmp gnome-vfs-config && \
		chmod 755 gnome-vfs-config
