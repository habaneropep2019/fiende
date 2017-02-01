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

#ifndef __GNOME_PRINT_ADMIN_OPTIONS_H__
#define __GNOME_PRINT_ADMIN_OPTIONS_H__

BEGIN_GNOME_DECLS

typedef enum {
	GPA_OPTIONS_TYPE_PICKONE,
	GPA_OPTIONS_TYPE_PICKMANY,
	GPA_OPTIONS_TYPE_BOOLEAN,
	GPA_OPTIONS_TYPE_NUMERIC,
	GPA_OPTIONS_TYPE_DOUBLE,
	GPA_OPTIONS_TYPE_ERROR,
} GpaOptionsType;

typedef enum {
	GPA_GROUP_PAPER,
	GPA_GROUP_QUALITY,
	GPA_GROUP_INSTALLED,
	GPA_GROUP_SUBGROUP,
	GPA_GROUP_COMPRESSION,
	GPA_GROUP_PS,
	GPA_GROUP_ERROR,
} GpaOptionsGroup;

typedef enum {
	GPA_CONTENT_GENERIC,
	GPA_CONTENT_PAPER_SIZE,
	GPA_CONTENT_PAPER_MEDIA,
	GPA_CONTENT_PAPER_SOURCE,
	GPA_CONTENT_PAPER_ORIENTATION,
	GPA_CONTENT_RESOLUTION,
	GPA_CONTENT_RESOLUTION_MODE,
	GPA_CONTENT_ERROR
} GpaContent;

typedef enum {
	GPA_PICKONE_COMBO,
	GPA_PICKONE_RADIO,
	GPA_PICKONE_ERROR,
} GpaPickoneType;

#include "gpa-structs.h"

#define GPA_IS_OPTIONS(obj) (obj) /* For now check for NULL only */

extern gboolean   debug_turned_on;

/* Utility functions */
GpaOption *    gpa_options_get_selected_option (GpaSettings *settings,
						const GpaOptions *options,
						gboolean fail);

/* Access to the structure from the outside */
const gchar * gpa_options_get_name  (const GpaOptions *options);
const gchar * gpa_options_get_id    (const GpaOptions *options);
const gchar * gpa_options_get_frame (const GpaOptions *options);
      gchar * gpa_options_dup_id    (const GpaOptions *options);
      gchar * gpa_options_dup_path  (const GpaOptions *options);

 GList * gpa_options_get_children   (const GpaOptions *options);
gboolean gpa_options_have_children  (const GpaOptions *options);

const GpaOption * gpa_options_get_parent  (const GpaOptions *options);

GpaOptionsGroup gpa_options_get_group   (const GpaOptions *options);
GpaOptionsType  gpa_options_get_type    (const GpaOptions *options);
GpaPickoneType  gpa_options_get_pickone_type (const GpaOptions *options);
GpaContent      gpa_options_get_content (const GpaOptions *options);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_OPTIONS_H__ */



