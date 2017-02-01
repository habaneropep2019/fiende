/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-mime-private.h
 *
 * Copyright (C) 2000 Eazel, Inc
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#ifndef GNOME_VFS_MIME_PRIVATE_H
#define GNOME_VFS_MIME_PRIVATE_H

#include <libgnomevfs/gnome-vfs-mime-monitor.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

void gnome_vfs_mime_info_shutdown 	      (void);
void gnome_vfs_mime_monitor_emit_data_changed (GnomeVFSMIMEMonitor *monitor);

typedef struct FileDateTracker FileDateTracker;

FileDateTracker	*gnome_vfs_file_date_tracker_new			(void);
void		 gnome_vfs_file_date_tracker_free			(FileDateTracker *tracker);
void		 gnome_vfs_file_date_tracker_start_tracking_file	(FileDateTracker *tracker, 
							 		 const char *local_file_path);
gboolean	 gnome_vfs_file_date_tracker_date_has_changed 		(FileDateTracker *tracker);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* GNOME_VFS_MIME_PRIVATE_H */
