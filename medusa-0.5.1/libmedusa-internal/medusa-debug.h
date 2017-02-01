/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   medusa-debug.h: Medusa debugging aids.
 
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
  
   Author: Darin Adler <darin@eazel.com>
*/

#ifndef MEDUSA_DEBUG_H
#define MEDUSA_DEBUG_H

#include <glib.h>

#ifdef G_DISABLE_ASSERT

#define medusa_assert_computed_str(str_expr, expected_str)

#else /* !G_DISABLE_ASSERT */

gboolean medusa_str_equal_with_free                          (char       *eat_this,
								const char *not_this);

#define medusa_assert_computed_str(str_expr, expected_str) \
	g_assert (medusa_str_equal_with_free ((str_expr), (expected_str)))

#endif /* !G_DISABLE_ASSERT */

void     medusa_stop_in_debugger                             (void);
void     medusa_make_warnings_and_criticals_stop_in_debugger (const char *first_domain,
								...);
int      medusa_get_available_file_descriptor_count          (void);

#endif /* MEDUSA_DEBUG_H */
