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


  
#ifndef __GPA_OPTION_PRIVATE_H__
#define __GPA_OPTION_PRIVATE_H__

BEGIN_GNOME_DECLS

#include "xml-utils.h"

struct _GpaOption
{
	gchar *name;     /* The name of the option. Like "Letter", "Upper Tray" */
	gchar *id;       /* The id. "Letter","UpperTray". This is are tranlated, names are */

	GpaContent content;

	GList *constraints; /* A list of GpaConstraints for this option */
	GHashTable *values;
	GpaOptions *parent; /* A pointer to it's GpaOptions struct */
	GpaOptions *children; /* Each option can have children, NULL if no children
			       * Like : Resolution-1720x1720-Softweave
			       * Resolition-1720x1720-MicroWeave
			       * Resolition-3600x1720-MicroWeave
			       */
};

/* XML writing & reading */
xmlNodePtr gpa_option_write  (XmlParseContext *context, GpaOption *option);

GpaOption * gpa_option_new_from_node (xmlNodePtr tree_,
				      GpaOptions *parent,
				      GList *loaded_option_list);

/* Paths */
    gchar * gpa_option_dup_path (GpaOption *option);
GpaOption * gpa_option_get_from_options_list_and_path (GList *options_list,
						       const gchar *path_);
GpaOption * gpa_option_get_from_settings_and_path (const GpaSettings *settings,
						   const gchar *path_);
GpaOption * gpa_option_get_from_printer_and_path  (const GpaPrinter *printer,
						   const gchar *path_);
GpaOption * gpa_option_get_from_model_and_path  (const GpaModel *model,
						 const gchar *path_);


END_GNOME_DECLS

#endif /* __GPA_OPTION_PRIVATE_H__ */

