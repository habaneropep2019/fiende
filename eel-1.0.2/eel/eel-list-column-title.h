/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-list-column-title.h: List column title widget for interacting with list columns

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
  
   Authors: Pavel Cisler <pavel@eazel.com>

*/

#ifndef __EEL_LIST_COLUMN_TITLE__
#define __EEL_LIST_COLUMN_TITLE__

#include <gdk/gdktypes.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkbin.h>
#include <gtk/gtkenums.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EEL_TYPE_LIST_COLUMN_TITLE	\
	(eel_list_column_title_get_type ())
#define EEL_LIST_COLUMN_TITLE(obj)	\
	(GTK_CHECK_CAST ((obj), EEL_TYPE_LIST_COLUMN_TITLE, EelListColumnTitle))
#define EEL_LIST_COLUMN_TITLE_CLASS(klass)	\
	(GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_LIST_COLUMN_TITLE, EelListColumnTitleClass))
#define EEL_IS_LIST_COLUMN_TITLE(obj) \
	(GTK_CHECK_TYPE ((obj), EEL_TYPE_LIST_COLUMN_TITLE))
#define EEL_IS_LIST_COLUMN_TITLE_CLASS(klass) \
	(GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_LIST_COLUMN_TITLE))

typedef struct EelListColumnTitle		EelListColumnTitle;
typedef struct EelListColumnTitleClass  	EelListColumnTitleClass;
typedef struct EelListColumnTitleDetails	EelListColumnTitleDetails;


struct EelListColumnTitle
{
	GtkBin bin;
	EelListColumnTitleDetails *details;
};

struct EelListColumnTitleClass
{
	GtkBinClass parent_class;
};

GtkType                          eel_list_column_title_get_type      (void);
EelListColumnTitle	        *eel_list_column_title_new           (void);
void				 eel_list_column_title_queue_draw    (EelListColumnTitle *title);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __EEL_LIST_COLUMN_TITLE__ */
