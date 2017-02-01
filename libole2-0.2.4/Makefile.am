AUTOMAKE_OPTIONS = gnu 1.4

bin_SCRIPTS = libole2-config

SUBDIRS = libole2 test doc

libole2aclocaldir = $(datadir)/aclocal
libole2aclocal_DATA = libole2.m4 gnome-libole2.m4

EXTRA_DIST = autogen.sh libole2.m4 libole2Conf.sh.in gnome-libole2.m4 libole2.spec libole2.spec.in

## Put `exec' in the name because this should be installed by
## `install-exec', not `install-data'.
confexecdir=$(libdir)
confexec_DATA = libole2Conf.sh

