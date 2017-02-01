/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000 Jose M Celorio
 *
 * gpa-settings-private.h : here we include the functions that are not publicly
 *                          accessed
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


  
#ifndef __GPA_SETTINGS_PRIVATE_H__
#define __GPA_SETTINGS_PRIVATE_H__

BEGIN_GNOME_DECLS

#include "xml-utils.h"

typedef struct _GpaSettingsClass    GpaSettingsClass;
typedef struct _GpaValue            GpaValue;

struct _GpaValue {
	gchar *key;
	gchar *value;
};


struct _GpaSettingsClass
{
	GtkObjectClass parent_class;
};

struct _GpaSettings {
	GtkObject object;

	gboolean selected;

	gchar *name;             /* Can be "Default settings" or a name choosen by the user */
	gchar *command;          /* The command to print to, like "lpr" for example, this will go away */
	GList *selected_options; /* list of paths (gchar *) */
	GList *values;           /* A list of GpaValues */
	GpaPrinter *printer;     /* A pointer to get back to the parent of this struct */
};


gboolean gpa_settigns_load_default_paths_from_node (xmlNodePtr tree_, GpaModel *model);

GList * gpa_settings_list_load_from_node (xmlNodePtr tree,
								  GpaPrinter *printer);

xmlNodePtr gpa_settings_list_write (XmlParseContext *context,
				    GList *settings_list);



END_GNOME_DECLS

#endif /* __GPA_SETTINGS_PRIVATE_H__ */
