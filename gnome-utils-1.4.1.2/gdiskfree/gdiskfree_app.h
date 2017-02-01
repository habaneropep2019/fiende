/* -*- Mode: C -*-
 * $Id: gdiskfree_app.h,v 1.3 2000/06/10 15:56:11 gregm Exp $
 *
 * GDiskFree -- A disk free space toy (df on steriods).
 * Copyright 1998,1999 Gregory McLean
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,  59 Temple Place - Suite 330, Cambridge, MA 02139, USA.
 *
 */ 
#ifndef __GDISKFREE_APP_H__
#define __GDISKFREE_APP_H__

#include <gnome.h>

typedef struct _GDiskFreeApp    GDiskFreeApp;
typedef struct _GDiskFreeDisk   GDiskFreeDisk;
struct _GDiskFreeApp {
  GtkWidget    *app;

  GtkWidget    *dial_box;
  GList        *drive_frame;    /* List of drives (widgets) in the window */
  GList        *drives;
};
struct _GDiskFreeDisk {
  GtkWidget    *dial;
  GtkWidget    *mount_label;
  GtkWidget    *size_label;
  gchar        *drive;
  gchar        *mount_point;
};
/****************************************************************************
 *
 **/
GDiskFreeApp       *gdiskfree_app_new          (const gchar    *geo);
void               gdiskfree_app_close         (GDiskFreeApp   *app);
void               gdiskfree_app_add_disk      (GDiskFreeApp   *app,
						const gchar    *disk,
						const gchar    *mount_point,
						const gchar    *disk_size);
void               gdiskfree_app_change_orient (GDiskFreeApp   *app,
						GtkOrientation orient);
gboolean           gdiskfree_update            (GDiskFreeApp   *app);
#endif
/* EOF */




