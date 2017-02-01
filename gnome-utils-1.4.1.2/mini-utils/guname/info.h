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

#ifndef INFO_H
#define INFO_H

#include <gnome.h>

/* The basic information to display in the main dialog. */
typedef enum {
  si_distribution, /* Debian, Solaris, etc. */
  si_OS,     
  si_distribution_version, 
  si_OS_version,
  si_release,

  si_CPU_type,
  si_CPU_speed,

  si_host,
  si_domain,

  si_user,
  si_display,

  si_uptime,

  si_mem,          
  si_mem_swap,
  si_mem_total,     
  si_mem_total_free,

  end_system_info
} system_info;

extern const gchar * descriptions[];
extern const gchar * info[];

void load_system_info();

#endif


