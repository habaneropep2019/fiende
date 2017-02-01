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


  
#ifndef __GPA_OPTIONS_PRIVATE_H__
#define __GPA_OPTIONS_PRIVATE_H__

BEGIN_GNOME_DECLS

#include "gpa-structs.h"
#include "xml-utils.h"

struct _GpaOptions
{
	gchar *name;  /* Name of the Options. Like : "Paper Size" */
	gchar *id;    /* Id. Like "PaperSize". The id is the internal tag
		       * used. Since the name can be translated */

	GList *code_fragments; /* List of GpaCodeFragments structs */

	gboolean defined;

	GpaOptionsGroup group;
	GpaContent      content;
	GpaOptionsType  options_type;

	GpaOption *parent;
	GList *children; /* List of "Option" elements */
	GpaModel *model;

	gchar *frame; /* The Frame that this options belong to in the config dialog
		       * it is a pointer to a string that lives inside a g_hash so
		       * it should not be freed. The string can be shared by other
		       * options that have the same frame name */
};


/* XML Writing */
xmlNodePtr gpa_options_write_paths_from_model (XmlParseContext *context,
					       GpaModel *model);
xmlNodePtr gpa_options_list_write (XmlParseContext *context,
				   GList *list_);
xmlNodePtr gpa_options_write (XmlParseContext *context,
			      GpaOptions *options);


/* XML Reading */
GpaOptions * gpa_options_new_from_node (GpaModel *model,
					xmlNodePtr tree_,
					GpaOption *parent);
GList *     gpa_options_list_new_from_node (xmlNodePtr tree_,
					    GpaModel *model);

/* Given a option type in a string (i.e. "PickOne") returns it's enumed value */
GpaOptionsType gpa_options_get_type_from_string (const gchar *text);


/* Basic "Options" object operations, either by individual objects
 * or by a list of them */
GpaOptions * gpa_options_new (GpaModel *model,
			      const gchar *name,
			      const gchar *id,
			      gint group);

void       gpa_options_free (GpaOptions *options);
void       gpa_options_list_free (GList *options_list);

gboolean   gpa_options_verify (GpaOptions *options,  const GpaModel *model);
gboolean   gpa_options_list_verify (const GpaModel *model);

gboolean   gpa_options_verify_with_settings (GpaOptions *options, GpaSettings *settings);
gboolean   gpa_options_list_verify_with_settings (const GpaModel *model,
						  GpaSettings *settings);

GpaOptions * gpa_options_copy (GpaOptions *options, GpaModel *model);
gboolean     gpa_options_list_copy (GpaPrinter *printer, GpaModel *model);

END_GNOME_DECLS

#endif /* __GPA_OPTIONS_PRIVATE_H__ */

