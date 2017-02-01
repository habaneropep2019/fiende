/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* eel-types.h - 

   Copyright (C) 2000 Eazel, Inc.

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

   Authors: Maciej Stachowiak <mjs@eazel.com>
*/


#include <config.h>
#include <gtk/gtktypeutils.h>

#include <eel/eel.h>

#include "eel-type-builtins-vars.c"
#include "eel-type-builtins-evals.c"

void
eel_type_init() {
	int i;
	
	static struct {
		const gchar * type_name;
		GtkType *type_id;
		GtkType parent;
		const GtkEnumValue *values;
	} builtin_info[EEL_TYPE_NUM_BUILTINS + 1] = {
		
#include "eel-type-builtins-ids.c"
		
		{ NULL }
	};
	
	for (i = 0; i < EEL_TYPE_NUM_BUILTINS; i++) {
		GtkType type_id = GTK_TYPE_INVALID;
		g_assert (builtin_info[i].type_name != NULL);

		if (builtin_info[i].parent == GTK_TYPE_ENUM) {
			type_id = gtk_type_register_enum (builtin_info[i].type_name, 
							  (GtkEnumValue *)builtin_info[i].values);
		} else if (builtin_info[i].parent == GTK_TYPE_FLAGS) {
			type_id = gtk_type_register_flags (builtin_info[i].type_name, 
							   (GtkFlagValue *)builtin_info[i].values);
		}

		g_assert (type_id != GTK_TYPE_INVALID);
		(*builtin_info[i].type_id) = type_id;
	}
}

