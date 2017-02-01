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

#ifndef MOREINFO_H
#define MOREINFO_H

typedef enum {
  fs_description,
  fs_numbers,
  fs_percent_full,

  end_filesystem_info
} filesystem_info;

/* List of arrays of filesystem info */
extern GList * filesystems;

typedef enum {
  mem_total,
  mem_used,
  mem_free,
  mem_shared,
  mem_buffer,
  mem_cached,
  mem_user,
  
  mem_swap_total,
  mem_swap_used,
  mem_swap_free,

  end_memory_info
} memory_info;

extern gchar ** memory;
extern gchar ** memory_descriptions;

void load_moreinfo();

void display_moreinfo();

#endif
