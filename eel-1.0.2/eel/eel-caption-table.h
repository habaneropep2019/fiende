/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-caption-table.h - An easy way to do tables of aligned captions.

   Copyright (C) 1999, 2000 Eazel, Inc.

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_CAPTION_TABLE_H
#define EEL_CAPTION_TABLE_H

#include <gtk/gtktable.h>
#include <libgnome/gnome-defs.h>

/*
 * EelCaptionTable is a GtkTable sublass that allows you to painlessly
 * create tables of nicely aligned captions.
 */

BEGIN_GNOME_DECLS

#define EEL_TYPE_CAPTION_TABLE			(eel_caption_table_get_type ())
#define EEL_CAPTION_TABLE(obj)		(GTK_CHECK_CAST ((obj), EEL_TYPE_CAPTION_TABLE, EelCaptionTable))
#define EEL_CAPTION_TABLE_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_CAPTION_TABLE, EelCaptionTableClass))
#define EEL_IS_CAPTION_TABLE(obj)		(GTK_CHECK_TYPE ((obj), EEL_TYPE_CAPTION_TABLE))
#define EEL_IS_CAPTION_TABLE_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_CAPTION_TABLE))

typedef struct EelCaptionTable		EelCaptionTable;
typedef struct EelCaptionTableClass	EelCaptionTableClass;
typedef struct EelCaptionTableDetail	EelCaptionTableDetail;

struct EelCaptionTable
{
	GtkTable table;

	EelCaptionTableDetail *detail;
};

struct EelCaptionTableClass
{
	GtkTableClass parent_class;

	void (*activate) (GtkWidget *caption_table, guint active_entry);
};

GtkType    eel_caption_table_get_type           (void);
GtkWidget* eel_caption_table_new                (guint            num_rows);
void       eel_caption_table_set_row_info       (EelCaptionTable *caption_table,
						 guint            row,
						 const char      *label_text,
						 const char      *entry_text,
						 gboolean         entry_visibility,
						 gboolean         entry_readonly);
void       eel_caption_table_set_entry_text     (EelCaptionTable *caption_table,
						 guint            row,
						 const char      *entry_text);
void       eel_caption_table_set_entry_readonly (EelCaptionTable *caption_table,
						 guint            row,
						 gboolean         readonly);
void       eel_caption_table_entry_grab_focus   (EelCaptionTable *caption_table,
						 guint            row);
char*      eel_caption_table_get_entry_text     (EelCaptionTable *caption_table,
						 guint            row);
guint      eel_caption_table_get_num_rows       (EelCaptionTable *caption_table);
void       eel_caption_table_resize             (EelCaptionTable *caption_table,
						 guint            num_rows);

END_GNOME_DECLS

#endif /* EEL_CAPTION_TABLE_H */


