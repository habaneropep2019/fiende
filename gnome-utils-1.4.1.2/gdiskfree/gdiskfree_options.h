/* -*- Mode: C -*-
 * $Id: gdiskfree_options.h,v 1.4 2001/08/12 16:36:06 gregm Exp $
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
#ifndef __GDISKFREE_OPTIONS_H__
#define __GDISKFREE_OPTIONS_H__
#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <glibtop/mountlist.h>
#include "gdiskfree_app.h"

typedef struct _GDiskFreeOptions          GDiskFreeOptions;
typedef struct _Disk                      Disk;
struct _GDiskFreeOptions {
  gint             update_interval;    /* How often to poll */
  /* FIXME:
   * These should be collaped down to a bitfield, for easier addtions and
   * cleaner code to parse around em.
   */
  gboolean         sync_required;      /* Invoke sync before gathering stats */
  gboolean         show_mount;         /* Show mountpoints */
  gboolean         show_size;          /* Show total size of device*/
  gboolean         show_remain;        /* Show remainging space on device */
  GtkOrientation   orientation;        /* Horizontal/Vertical */
  GList            *excluded;          /* Exclude these devices */
};
/****************************************************************************
 * Functions
 **/
void          gdiskfree_option_init             (void);
void          gdiskfree_option_save             (void);
GtkWidget     *gdiskfree_option_dialog          (GDiskFreeApp *app);

#endif
/* EOF */













