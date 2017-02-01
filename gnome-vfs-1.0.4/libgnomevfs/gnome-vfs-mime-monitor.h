/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   gnome-vfs-mime-monitor.h: Class for noticing changes in MIME data.
 
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
  
   Author: John Sullivan <sullivan@eazel.com>
*/

#ifndef GNOME_VFS_MIME_MONITOR_H
#define GNOME_VFS_MIME_MONITOR_H

#include <gtk/gtkobject.h>

typedef struct {
	GtkObject object;
} GnomeVFSMIMEMonitor;

/* Instead of a class declaration here, I will just document
 * the signals.
 *
 *	"data_changed", no parameters
 */

/* There's a single GnomeVFSMIMEMonitor object.
 * The only thing you need it for is to connect to its signals.
 */
GnomeVFSMIMEMonitor *           gnome_vfs_mime_monitor_get                          (void);

#endif /* GNOME_VFS_MIME_MONITOR_H */
