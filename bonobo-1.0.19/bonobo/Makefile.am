PLUGIN_DIR=$(libdir)/bonobo/plugin

INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DBONOBO_UIDIR=\""$(datadir)/gnome/ui/"\"	\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DBONOBO_LOCALEDIR=\""$(bonobolocaledir)"\"	\
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	$(BONOBO_INCLUDEDIR)				\
	$(GDK_PIXBUF_CFLAGS)				\
	$(ORBIT_CFLAGS)					\
	$(PROF_CFLAGS)

LDFLAGS += $(PROF_LDFLAGS)

lib_LTLIBRARIES = libbonobo.la libbonobox.la libbonobo-print.la

CORBA_SOURCE =			\
	Bonobo.h		\
	Bonobo-common.c		\
	Bonobo-stubs.c		\
	Bonobo-skels.c

idls =							\
	$(top_srcdir)/idl/Bonobo.idl			\
	$(top_srcdir)/idl/Bonobo_Advise.idl		\
	$(top_srcdir)/idl/Bonobo_Desktop.idl		\
	$(top_srcdir)/idl/Bonobo_Canvas.idl		\
	$(top_srcdir)/idl/Bonobo_ClientSite.idl		\
	$(top_srcdir)/idl/Bonobo_Context.idl		\
	$(top_srcdir)/idl/Bonobo_Control.idl		\
	$(top_srcdir)/idl/Bonobo_Embeddable.idl		\
	$(top_srcdir)/idl/Bonobo_ItemContainer.idl	\
	$(top_srcdir)/idl/Bonobo_Listener.idl		\
	$(top_srcdir)/idl/Bonobo_Moniker.idl		\
	$(top_srcdir)/idl/Bonobo_Persist.idl		\
	$(top_srcdir)/idl/Bonobo_Print.idl		\
	$(top_srcdir)/idl/Bonobo_Progressive.idl	\
	$(top_srcdir)/idl/Bonobo_Property.idl		\
	$(top_srcdir)/idl/Bonobo_Storage.idl		\
	$(top_srcdir)/idl/Bonobo_UI.idl			\
	$(top_srcdir)/idl/Bonobo_Unknown.idl		\
	$(top_srcdir)/idl/Bonobo_Zoomable.idl		\
	$(top_srcdir)/idl/Bonobo_Exception.idl

idl_flags = `$(GNOME_CONFIG) --cflags idl` -D__BONOBO_COMPILATION

$(CORBA_SOURCE): $(idls)
	$(ORBIT_IDL) $(top_srcdir)/idl/Bonobo.idl -I$(top_srcdir)/idl $(idl_flags)

libbonobo_la_LDFLAGS = \
	-version-info $(BONOBO_CURRENT):$(BONOBO_REVISION):$(BONOBO_AGE) \
	$(PROF_LDFLAGS) 

libbonobo_la_SOURCES =			\
	$(CORBA_SOURCE)			\
	bonobo-activation-context.c	\
	bonobo-arg.c			\
	bonobo-async.c			\
	bonobo-context.c		\
	bonobo-event-source.c		\
	bonobo-exception.c		\
	bonobo-object-directory.c	\
	bonobo-generic-factory.c	\
	bonobo-item-handler.c		\
	bonobo-listener.c		\
	bonobo-main.c			\
	bonobo-xobject.c		\
	bonobo-moniker.c		\
	bonobo-moniker-extender.c	\
	bonobo-moniker-simple.c		\
	bonobo-moniker-util.c		\
	bonobo-object.c			\
	bonobo-object-client.c		\
	bonobo-object-io.c		\
	bonobo-persist-file.c		\
	bonobo-persist-stream.c		\
	bonobo-persist.c		\
	bonobo-progressive.c		\
	bonobo-shlib-factory.c		\
	bonobo-storage-plugin.c		\
	bonobo-storage.c		\
	bonobo-stream.c			\
	bonobo-stream-client.c		\
	bonobo-stream-memory.c		\
	bonobo-transient.c		\
	bonobo-property.c		\
	bonobo-property-bag.c		\
	bonobo-property-bag-client.c	\
	bonobo-property-control.c	\
	bonobo-running-context.c

libbonoboincludedir = $(includedir)/gnome-1.0/bonobo

libbonoboinclude_HEADERS =			\
	Bonobo.h				\
	bonobo-arg.h				\
	bonobo-async.h				\
	bonobo-canvas-component.h		\
	bonobo-canvas-item.h			\
	bonobo-client-site.h			\
	bonobo-control-frame.h			\
	bonobo-control.h			\
	bonobo-context.h			\
	bonobo-desktop-window.h			\
	bonobo-embeddable.h			\
	bonobo-event-source.h			\
	bonobo-exception.h			\
	bonobo-generic-factory.h		\
	bonobo-item-container.h			\
	bonobo-item-handler.h			\
	bonobo-listener.h			\
	bonobo-main.h				\
	bonobo-xobject.h			\
	bonobo-moniker.h			\
	bonobo-moniker-extender.h		\
	bonobo-moniker-simple.h			\
	bonobo-moniker-util.h			\
	bonobo-object-client.h			\
	bonobo-object-directory.h		\
	bonobo-object-io.h			\
	bonobo-object.h				\
	bonobo-persist-file.h			\
	bonobo-persist-stream.h			\
	bonobo-persist.h			\
	bonobo-plug.h				\
	bonobo-progressive.h			\
	bonobo-property-bag.h			\
	bonobo-property-bag-client.h		\
	bonobo-property-bag-xml.h		\
	bonobo-property-control.h		\
	bonobo-property.h			\
	bonobo-selector.h			\
	bonobo-selector-widget.h		\
	bonobo-shlib-factory.h			\
	bonobo-socket.h				\
	bonobo-storage-plugin.h			\
	bonobo-storage.h			\
	bonobo-stream-client.h			\
	bonobo-stream-memory.h			\
	bonobo-stream.h				\
	bonobo-transient.h			\
	bonobo-ui-component.h            	\
	bonobo-ui-container.h            	\
	bonobo-ui-config-widget.h		\
	bonobo-ui-engine.h	            	\
	bonobo-ui-node.h                 	\
	bonobo-ui-sync.h			\
	bonobo-ui-toolbar-button-item.h  	\
	bonobo-ui-toolbar-icon.h         	\
	bonobo-ui-toolbar-item.h		\
	bonobo-ui-toolbar-toggle-button-item.h	\
	bonobo-ui-util.h			\
	bonobo-view-frame.h			\
	bonobo-view.h				\
	bonobo-widget.h 			\
	bonobo-win.h				\
	bonobo-wrapper.h			\
	bonobo-zoomable.h			\
	bonobo-zoomable-frame.h			

noinst_HEADERS =				\
	bonobo-activation-context.h		\
	bonobo-ui-engine-config.h		\
	bonobo-ui-engine-private.h		\
	bonobo-ui-icon.h			\
	bonobo-ui-icon-modify.h			\
	bonobo-ui-pixmap-cache.h		\
	bonobo-ui-state-cache.h			\
	bonobo-ui-sync-menu.h			\
	bonobo-ui-sync-keys.h			\
	bonobo-ui-sync-status.h			\
	bonobo-ui-sync-toolbar.h		\
	bonobo-ui-toolbar.h			\
	bonobo-ui-toolbar-control-item.h  	\
	bonobo-ui-toolbar-popup-item.h		\
	bonobo-ui-toolbar-separator-item.h	\
	bonobo-ui-xml.h				\
	bonobo-running-context.h		

libbonobox_la_LDFLAGS = \
	-version-info $(BONOBO_CURRENT):$(BONOBO_REVISION):$(BONOBO_AGE)

libbonobox_la_SOURCES =				\
	bonobo-canvas-component.c		\
	bonobo-canvas-item.c			\
	bonobo-client-site.c			\
	bonobo-control-frame.c			\
	bonobo-control.c			\
	bonobo-desktop-window.c			\
	bonobo-embeddable.c			\
	bonobo-item-container.c			\
	bonobo-plug.c				\
	bonobo-property-bag-xml.c		\
	bonobo-selector.c			\
	bonobo-selector-widget.c		\
	bonobo-socket.c				\
	bonobo-ui-component.c            	\
	bonobo-ui-config-widget.c		\
	bonobo-ui-container.c            	\
	bonobo-ui-engine.c			\
	bonobo-ui-engine-config.c		\
	bonobo-ui-icon.c			\
	bonobo-ui-icon-modify.c			\
	bonobo-ui-node.c                 	\
	bonobo-ui-pixmap-cache.c		\
	bonobo-ui-state-cache.c			\
	bonobo-ui-sync.c			\
	bonobo-ui-sync-menu.c			\
	bonobo-ui-sync-keys.c			\
	bonobo-ui-sync-status.c			\
	bonobo-ui-sync-toolbar.c		\
	bonobo-ui-toolbar-button-item.c  	\
	bonobo-ui-toolbar-control-item.c  	\
	bonobo-ui-toolbar-icon.c         	\
	bonobo-ui-toolbar-item.c		\
	bonobo-ui-toolbar-popup-item.c		\
	bonobo-ui-toolbar-separator-item.c	\
	bonobo-ui-toolbar-toggle-button-item.c	\
	bonobo-ui-toolbar.c			\
	bonobo-ui-util.c			\
	bonobo-ui-xml.c				\
	bonobo-view-frame.c			\
	bonobo-view.c				\
	bonobo-widget.c				\
	bonobo-win.c				\
	bonobo-wrapper.c			\
	bonobo-zoomable.c			\
	bonobo-zoomable-frame.c

libbonobo_print_la_LDFLAGS = \
	-version-info $(BONOBO_CURRENT):$(BONOBO_REVISION):$(BONOBO_AGE)

libbonobo_print_la_SOURCES =		\
	bonobo-print.c			\
	bonobo-print-client.c

libbonobo_printincludedir = $(includedir)/gnome-1.0/bonobo

libbonobo_printinclude_HEADERS =		\
	Bonobo.h			\
	bonobo-print.h		\
	bonobo-print-client.h

EXTRA_DIST = bonobo-insert-component.xpm

BUILT_SOURCES=$(CORBA_SOURCE)
CLEANFILES += $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
