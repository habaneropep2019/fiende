/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   guname: System information dialog.
 *
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com> except marquee code.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef LIST_H
#define LIST_H

#ifdef HAVE_LIBGTOP_SYSINFO

#include <glibtop/sysinfo.h>

void fill_clist_from_glibtop_entry (GtkCList * list,
                                    glibtop_entry * entry,
                                    gint * width, gint * height);

#endif

void fill_clist(GtkCList * clist,
                const gchar ** col1, const gchar ** col2, 
                gint numitems, gint * width, gint * height);

GtkWidget * create_clist(const gchar * titles[]);

#endif
