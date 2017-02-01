/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-async-job-map.h 

   Copyright (C) 2001 Eazel Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Pavel Cisler <pavel@eazel.com> */

#ifndef GNOME_VFS_ASYNC_JOB_MAP_H
#define GNOME_VFS_ASYNC_JOB_MAP_H

#include "gnome-vfs.h"
#include "gnome-vfs-job.h"


/* async job map calls */
void		 gnome_vfs_async_job_map_init		  	(void);
void	     	 gnome_vfs_async_job_map_shutdown	  	(void);
gboolean     	 gnome_vfs_async_job_completed 	  	  	(GnomeVFSAsyncHandle		*handle);
void 		 gnome_vfs_async_job_map_add_job  	  	(GnomeVFSJob			*job);
void 		 gnome_vfs_async_job_map_remove_job  	  	(GnomeVFSJob			*job);
GnomeVFSJob	*gnome_vfs_async_job_map_get_job	  	(const GnomeVFSAsyncHandle	*handle);

void		 gnome_vfs_async_job_map_assert_locked	  	(void);
void		 gnome_vfs_async_job_map_lock	  	  	(void);
void		 gnome_vfs_async_job_map_unlock	  	  	(void);

/* async job callback map calls */
void		 gnome_vfs_async_job_callback_valid		(guint				 callback_id,
								 gboolean			*valid,
								 gboolean			*cancelled);
gboolean	 gnome_vfs_async_job_add_callback		(GnomeVFSJob			*job,
							  	 GnomeVFSNotifyResult		*notify_result);
void		 gnome_vfs_async_job_remove_callback		(guint			 	 callback_id);
void		 gnome_vfs_async_job_cancel_job_and_callbacks	(GnomeVFSAsyncHandle		*job_handle,
								 GnomeVFSJob 			*job);

#endif
