NULL =
SUBDIRS = extfs

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/libgnomevfs		\
	-I$(top_builddir)/libgnomevfs		\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)/intl			\
	$(GLIB_CFLAGS)				\
	$(XML_CFLAGS)	 			\
	$(LIBEFS_CFLAGS)			\
	$(GCONF_CFLAGS)				\
	$(VFS_GNOME_CFLAGS)			\
	$(VFS_CFLAGS) $(WERROR)			\
	-D_GNU_SOURCE				\
	-D_REENTRANT				\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DPREFIX=\"$(prefix)\"

EXTRA_DIST =					\
	default-modules.conf			\
	cdda-module.conf			\
	ssl-modules.conf			\
	$(libnfs_la_SRCLIST)			\
	$(libssh_la_SRCLIST)

###  Module setup

if HAVE_CDDA
CDDA_LTLIBS = libcdda.la
else
CDDA_LTLIBS =
endif

module_flags = -export_dynamic -avoid-version
modulesdir = $(libdir)/vfs/modules

modules_LTLIBRARIES =				\
	$(CDDA_LTLIBS)				\
	libextfs.la				\
	libfile.la				\
	libvfs-test.la				\
	$(HTTP_LTLIBS)				\
	libbzip2.la				\
	libgzip.la				\
	libftp.la				\
	libnntp.la				\
	libvfs-pipe.la				\
	libvfs-translate.la			\
	libdesktop.la				\
	$(NULL)

# Disabled for now
#	libssh.la
#	libnfs.la

modulesconfdir=$(sysconfdir)/vfs/modules
if HAVE_CDDA
CDDA_CONF = cdda-module.conf
endif
if HAVE_SSL
SSL_CONF = ssl-modules.conf
endif
modulesconf_DATA = default-modules.conf $(CDDA_CONF) $(SSL_CONF)

###  `cdda' method

libcdda_la_SOURCES = \
	cdda-method.c 				\
	cdda-method.h				\
	cdda-cddb.c 				\
	cdda-cddb.h				\
	cdda-cdrom-extensions.h

libcdda_la_LDFLAGS = $(module_flags)
libcdda_la_LIBADD = -L/gnome/lib -lcdda_paranoia -lcdda_interface

###  `extfs' method

libextfs_la_SOURCES = 				\
	extfs-method.c 				\
	extfs-method.h
libextfs_la_LDFLAGS = $(module_flags)

###  `file' method

libfile_la_SOURCES = 				\
	file-method.c 				\
	file-method.h
libfile_la_LDFLAGS = $(module_flags)

###  `desktop' method

libdesktop_la_SOURCES = 				\
	desktop-method.c
libdesktop_la_LDFLAGS = $(module_flags)

###  `ssh' method

libssh_la_SRCLIST = 				\
	ssh-method.c 				\
	ssh-method.h
#libssh_la_LDFLAGS = $(module_flags)

###  `test' method

libvfs_test_la_SOURCES =			\
	test-method.c				
libvfs_test_la_LIBADD = $(XML_LIBS)

### `gzip' method

libgzip_la_SOURCES = gzip-method.c
libgzip_la_LDFLAGS = $(module_flags)
libgzip_la_LIBADD = -lz

### `bzip2' method

libbzip2_la_SOURCES = bzip2-method.c
libbzip2_la_LDFLAGS = $(module_flags)
libbzip2_la_LIBADD = -lbz2

### `ftp' method

libftp_la_SOURCES = 				\
	ftp-method.c				\
	ftp-method.h

### `nntp' method

libnntp_la_SOURCES = 				\
	nntp-method.c 				\
	nntp-method.h
libfile_la_LDFLAGS = $(module_flags)

### `nfs' method

libnfs_la_SRCLIST = 				\
	nfs-method.c				\
	nfs-method.h				\
	nfs-method_mount_xdr.c			\
	nfs-method_nfs_prot_xdr.c		\
	nfs-method_mount.h			\
	nfs-method_nfs_prot.h
#libnfs_la_LDFLAGS = $(module_flags)

### `http' method

HTTP_LTLIBS = libhttp.la

libhttp_la_SOURCES =				\
	http-method.c				\
	http-method.h				\
	http-authn.c				\
	http-authn.h				\
	http-cache.c				\
	http-cache.h				\
	$(NULL)

libhttp_la_LDFLAGS = $(module_flags)
libhttp_la_LIBADD  = $(GLIB_LIBS) $(XML_LIBS) $(GCONF_LIBS)

libvfs_pipe_la_SOURCES =			\
	pipe-method.c				\
	pipe-method.h
libvfs_pipe_la_LDFLAGS = $(module_flags)
libvfs_pipe_la_LIBADD  = $(GLIB_LIBS)

libvfs_translate_la_SOURCES =			\
	translate-method.c
libvfs_translate_la_LDFLAGS = $(module_flags)
libvfs_translate_la_LIBADD  = $(GLIB_LIBS)

#if HAVE_LIBEFS
#vefsmoduledir = $(modulesdir)
#vefsmodule_LTLIBRARIES = libvefs.la
#
#libvefs_la_SOURCES =	\
#	efs-method.c	\
#        efs-method.h
#libvefs_la_LIBADD = $(LIBEFS_LIBS) $(GLIB_LIBS)
#endif
