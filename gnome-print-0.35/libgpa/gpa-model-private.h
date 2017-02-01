/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000 Jose M Celorio
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *
 */
  
#ifndef __GPA_MODEL_PRIVATE_H__
#define __GPA_MODEL_PRIVATE_H__

BEGIN_GNOME_DECLS

struct _GpaModel {

	GtkObject object;
	
	gchar *name;
	gchar *id;
	const GpaVendor *vendor;

	GList *code_fragments;   /* List of GpaCodeFragments*/
	GList *backend_list;     /* List of GpaBackends */
	GList *options_list;     /* List of GpaOptions */
	GList *default_settings; /* List of paths (gchar *) */

	GHashTable *model_info;
	GList *default_values;
};


GpaModel * gpa_model_new (void);

END_GNOME_DECLS

#endif /* __GPA_MODEL_PRIVATE_H__ */

