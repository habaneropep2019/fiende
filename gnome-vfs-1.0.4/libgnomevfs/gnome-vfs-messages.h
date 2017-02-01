/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-messages.h - Status message reporting for GNOME Virtual File
   System.

   Copyright (C) 1999 Free Software Foundation

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

   Author: Havoc Pennington <hp@redhat.com> */

#ifndef _GNOME_VFS_MESSAGES_H
#define _GNOME_VFS_MESSAGES_H

/* The GnomeVFSMessageCallbacks object is intended to be used
 * internally by the handle and async handle to manage lists of status
 * callbacks.  Although it may make sense to have
 * handle_get_message_callbacks() and then let people use this API
 * directly.  */

/* FIXME bugzilla.eazel.com 1143
   should we have a priority enum of some kind, and pass it to
   the status callback? Netscape doesn't distinguish any of the status
   messages but you probably could if you wanted. */

/* FIXME bugzilla.eazel.com 1141
   this isn't thread safe (well, one thread per
   GnomeVFSMessageCallbacks, there's no static data) - can multiple
   threads be using the same file handle? */

#include <glib.h>

#include <libgnomevfs/gnome-vfs.h>

typedef struct GnomeVFSMessageCallbacks GnomeVFSMessageCallbacks;

/* Used to report user-friendly status messages you might want to display. */
typedef void (* GnomeVFSStatusCallback) (const gchar  *message,
					 gpointer      callback_data);

GnomeVFSMessageCallbacks*
     gnome_vfs_message_callbacks_new  (void);

void gnome_vfs_message_callbacks_destroy
                                      (GnomeVFSMessageCallbacks *cbs);

guint gnome_vfs_message_callbacks_add  (GnomeVFSMessageCallbacks *cbs,
                                       GnomeVFSStatusCallback    callback,
                                       gpointer                  user_data);

guint gnome_vfs_message_callbacks_add_full
                                      (GnomeVFSMessageCallbacks *cbs,
                                       GnomeVFSStatusCallback    callback,
                                       gpointer                  user_data,
                                       GDestroyNotify            notify);

void gnome_vfs_message_callbacks_remove
                                      (GnomeVFSMessageCallbacks *cbs,
				       guint num);
				       
void gnome_vfs_message_callbacks_remove_by_func
                                      (GnomeVFSMessageCallbacks *cbs,
                                       GnomeVFSStatusCallback    callback);

void gnome_vfs_message_callbacks_remove_by_data
                                      (GnomeVFSMessageCallbacks *cbs,
                                       gpointer                  user_data);

void gnome_vfs_message_callbacks_remove_by_func_and_data
                                      (GnomeVFSMessageCallbacks *cbs,
                                       GnomeVFSStatusCallback    callback,
                                       gpointer                  user_data);

void gnome_vfs_message_callbacks_emit (GnomeVFSMessageCallbacks *cbs,
                                       const gchar              *message);

#endif /* _GNOME_VFS_MESSAGES_H */
