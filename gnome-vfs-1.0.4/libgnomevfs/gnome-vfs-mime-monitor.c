/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   gnome-vfs-mime-monitor.c: Class for noticing changes in MIME data.
 
   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Authors: John Sullivan <sullivan@eazel.com>,
*/

#include <config.h>
#include "gnome-vfs-mime-monitor.h"

#include "gnome-vfs-mime-private.h"

#include <gtk/gtksignal.h>

#define GNOME_VFS_MIME_MONITOR(obj) \
	GTK_CHECK_CAST (obj, gnome_vfs_mime_monitor_get_type (), GnomeVFSMIMEMonitor)

typedef struct {
	GtkObjectClass parent_class;
} GnomeVFSMIMEMonitorClass;

enum {
	DATA_CHANGED,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL];

static GtkType    gnome_vfs_mime_monitor_get_type         (void);
static void       gnome_vfs_mime_monitor_initialize_class (GnomeVFSMIMEMonitorClass *class);
static void       gnome_vfs_mime_monitor_initialize       (GnomeVFSMIMEMonitor      *monitor);

static gpointer parent_class;

static GtkType
gnome_vfs_mime_monitor_get_type (void)
{
	GtkType parent_type;
	static GtkType type;

	if (type == 0) {
		static GtkTypeInfo info = {
			"GnomeVFSMIMEMonitor",
			sizeof (GnomeVFSMIMEMonitor),
			sizeof (GnomeVFSMIMEMonitorClass),
			(GtkClassInitFunc)gnome_vfs_mime_monitor_initialize_class,
			(GtkObjectInitFunc)gnome_vfs_mime_monitor_initialize,
			NULL,
			NULL,
			NULL
		};

		parent_type = GTK_TYPE_OBJECT;
		type = gtk_type_unique (parent_type, &info);
		parent_class = gtk_type_class (parent_type);
	}

	return type;
}

static GnomeVFSMIMEMonitor *global_mime_monitor = NULL;

/* Return a pointer to the single global monitor. */
GnomeVFSMIMEMonitor *
gnome_vfs_mime_monitor_get (void)
{
        if (global_mime_monitor == NULL) {
		global_mime_monitor = GNOME_VFS_MIME_MONITOR
			(gtk_object_new (gnome_vfs_mime_monitor_get_type (), NULL));
        }
        return global_mime_monitor;
}

static void
gnome_vfs_mime_monitor_initialize (GnomeVFSMIMEMonitor *monitor)
{
}

static void
gnome_vfs_mime_monitor_initialize_class (GnomeVFSMIMEMonitorClass *class)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (class);

	signals[DATA_CHANGED]
		= gtk_signal_new ("data_changed",
				  GTK_RUN_LAST,
#if GNOME_PLATFORM_VERSION < 1095000
				  object_class->type,
#else
				  G_OBJECT_CLASS_TYPE (G_OBJECT (object_class)),
#endif
				  0,
				  gtk_marshal_NONE__NONE,
				  GTK_TYPE_NONE, 0);

#if GNOME_PLATFORM_VERSION < 1095000
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
#endif
}

void
gnome_vfs_mime_monitor_emit_data_changed (GnomeVFSMIMEMonitor *monitor)
{
	gtk_signal_emit (GTK_OBJECT (monitor),
			 signals[DATA_CHANGED]);	
}
