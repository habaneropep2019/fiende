NULL = 

INCLUDES =					\
	-DGNOME_VFS_CONFDIR=\"$(sysconfdir)\"	\
	-DGNOME_VFS_DATADIR=\"$(datadir)\"	\
	-DGNOME_VFS_PREFIX=\"$(prefix)\"	\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)			\
	-I$(top_builddir)/intl			\
	-I$(top_builddir)/libgnomevfs		\
	$(GLIB_CFLAGS)				\
	$(ORBIT_CFLAGS)				\
	$(OAF_CFLAGS)				\
	$(XML_CFLAGS)				\
	$(GCONF_CFLAGS) 			\
	$(VFS_CFLAGS) $(WERROR)			\
        $(OPENSSL_CFLAGS)                       \
	-D_GNU_SOURCE				\
	-D_REENTRANT				\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-DLIBDIR=\"$(libdir)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DG_LOG_DOMAIN=\"GnomeVFS\"		\
	$(NULL)

lib_LTLIBRARIES = libgnomevfs.la

EXTRA_DIST =					\
	getdelim.c

libgnomevfs_la_LIBADD =				\
	$(GLIB_LIBS)				\
	$(ORBIT_LIBS)				\
	$(OAF_LIBS)				\
	$(XML_LIBS)				\
	$(GCONF_LIBS) 				\
	$(OPENSSL_LDFLAGS) 			\
	$(NULL)

libgnomevfsincludedir = $(includedir)/gnome-vfs-1.0/libgnomevfs

libgnomevfsinclude_HEADERS =			\
	gnome-vfs-application-registry.h	\
	gnome-vfs-async-ops.h			\
	gnome-vfs-backend.h			\
	gnome-vfs-cancellable-ops.h		\
	gnome-vfs-cancellation.h		\
	gnome-vfs-configuration.h		\
	gnome-vfs-context.h			\
	gnome-vfs-directory-filter.h		\
	gnome-vfs-directory.h			\
	gnome-vfs-file-info.h			\
	gnome-vfs-file-size.h			\
	gnome-vfs-find-directory.h		\
	gnome-vfs-handle.h			\
	gnome-vfs-inet-connection.h		\
	gnome-vfs-init.h			\
	gnome-vfs-iobuf.h			\
	gnome-vfs-messages.h			\
	gnome-vfs-method.h			\
	gnome-vfs-mime-handlers.h		\
	gnome-vfs-mime-info.h			\
	gnome-vfs-mime-magic.h			\
	gnome-vfs-mime-monitor.h		\
	gnome-vfs-mime-sniff-buffer.h		\
	gnome-vfs-mime.h			\
	gnome-vfs-module-callback-module-api.h	\
	gnome-vfs-module-callback.h		\
	gnome-vfs-module-shared.h		\
	gnome-vfs-module.h			\
	gnome-vfs-ops.h				\
	gnome-vfs-parse-ls.h			\
	gnome-vfs-private-utils.h		\
	gnome-vfs-private.h			\
	gnome-vfs-regexp-filter.h		\
	gnome-vfs-result.h			\
	gnome-vfs-seekable.h			\
	gnome-vfs-shellpattern-filter.h		\
	gnome-vfs-socket-buffer.h		\
	gnome-vfs-socket.h			\
	gnome-vfs-ssl-private.h			\
	gnome-vfs-ssl.h				\
	gnome-vfs-standard-callbacks.h		\
	gnome-vfs-transform.h			\
	gnome-vfs-types.h			\
	gnome-vfs-uri.h				\
	gnome-vfs-utils.h			\
	gnome-vfs-xfer.h			\
	gnome-vfs.h				\
	$(NULL)

libgnomevfs_la_SOURCES =			\
	fnmatch.c				\
	gnome-vfs-application-registry.c	\
	gnome-vfs-async-ops.c			\
	gnome-vfs-backend.c			\
	gnome-vfs-cancellable-ops.c		\
	gnome-vfs-cancellation.c		\
	gnome-vfs-configuration.c		\
	gnome-vfs-context.c			\
	gnome-vfs-directory-filter.c		\
	gnome-vfs-directory.c			\
	gnome-vfs-file-info.c			\
	gnome-vfs-find-directory.c		\
	gnome-vfs-handle.c			\
	gnome-vfs-i18n.c			\
	gnome-vfs-inet-connection.c		\
	gnome-vfs-init.c			\
	gnome-vfs-iobuf.c			\
	gnome-vfs-messages.c			\
	gnome-vfs-method.c			\
	gnome-vfs-mime-handlers.c		\
	gnome-vfs-mime-info.c			\
	gnome-vfs-mime-magic.c			\
	gnome-vfs-mime-monitor.c		\
	gnome-vfs-mime-sniff-buffer.c		\
	gnome-vfs-mime.c			\
	gnome-vfs-module-callback-module-api.c	\
	gnome-vfs-module-callback-private.c	\
	gnome-vfs-module-callback.c		\
	gnome-vfs-module-shared.c		\
	gnome-vfs-module.c			\
	gnome-vfs-ops.c				\
	gnome-vfs-parse-ls.c			\
	gnome-vfs-private-utils.c		\
	gnome-vfs-private.c			\
	gnome-vfs-process.c			\
	gnome-vfs-regexp-filter.c		\
	gnome-vfs-result.c			\
	gnome-vfs-seekable.c			\
	gnome-vfs-shellpattern-filter.c		\
	gnome-vfs-socket-buffer.c		\
	gnome-vfs-socket.c			\
	gnome-vfs-ssl.c				\
	gnome-vfs-transform.c			\
	gnome-vfs-uri.c				\
	gnome-vfs-utils.c			\
	gnome-vfs-xfer.c			\
	fnmatch.h				\
	gnome-vfs-mime-private.h		\
	gnome-vfs-mime-sniff-buffer-private.h	\
	gnome-vfs-module-callback-private.h	\
	gnome-vfs-process.h			\
	$(NULL)

noinst_HEADERS =				\
	gnome-vfs-backend-private.h

# Platform specific includes
libgnomevfsplatofrmincludedir = $(libdir)/gnome-vfs-1.0/include
libgnomevfsplatofrminclude_HEADERS =			\
	gnome-vfs-file-size.h
