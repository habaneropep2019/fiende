/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-drag.h - Common Drag & drop handling code shared by the icon container
   and the list view.

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

   Authors: Pavel Cisler <pavel@eazel.com>,
   	    Ettore Perazzoli <ettore@gnu.org>
*/

#ifndef EEL_DND_H
#define EEL_DND_H

/* FIXME: This should really be back in Nautilus, not here in Eel. */

#include <gtk/gtkdnd.h>

/* Drag & Drop target names. */
#define EEL_ICON_DND_GNOME_ICON_LIST_TYPE "x-special/gnome-icon-list"
#define EEL_ICON_DND_URI_LIST_TYPE        "text/uri-list"
#define EEL_ICON_DND_TEXT_TYPE            "text/plain"
#define EEL_ICON_DND_URL_TYPE	          "_NETSCAPE_URL"
#define EEL_ICON_DND_COLOR_TYPE           "application/x-color"
#define EEL_ICON_DND_BGIMAGE_TYPE         "property/bgimage"
#define EEL_ICON_DND_KEYWORD_TYPE         "property/keyword"

/* Item of the drag selection list */
typedef struct {
	char *uri;
	gboolean got_icon_position;
	int icon_x, icon_y;
	int icon_width, icon_height;
} EelDragSelectionItem;

/* Standard Drag & Drop types. */
typedef enum {
	EEL_ICON_DND_GNOME_ICON_LIST,
	EEL_ICON_DND_URI_LIST,
	EEL_ICON_DND_URL,
	EEL_ICON_DND_COLOR,
	EEL_ICON_DND_BGIMAGE,
	EEL_ICON_DND_KEYWORD,
	EEL_ICON_DND_TEXT
} EelIconDndTargetType;

/* drag&drop-related information. */
typedef struct {
	GtkTargetList *target_list;

	/* Stuff saved at "receive data" time needed later in the drag. */
	gboolean got_drop_data_type;
	EelIconDndTargetType data_type;
	GtkSelectionData *selection_data;

	/* Start of the drag, in world coordinates. */
	gdouble start_x, start_y;

	/* List of EelDragSelectionItems, representing items being dragged, or NULL
	 * if data about them has not been received from the source yet.
	 */
	GList *selection_list;

	/* Stipple for drawing icon shadows during DnD.  */
	GdkBitmap *stipple;

        /* has the drop occured ? */
        gboolean drop_occured;

	/* whether or not need to clean up the previous dnd data */
	gboolean need_to_destroy;

	/* autoscrolling during dragging */
	int auto_scroll_timeout_id;
	gboolean waiting_to_autoscroll;
	gint64 start_auto_scroll_in;

} EelDragInfo;

typedef void 		(* EelDragEachSelectedItemDataGet)	(const char *url, 
								 int x, int y, int w, int h, 
								 gpointer data);
typedef void 		(* EelDragEachSelectedItemIterator)	(EelDragEachSelectedItemDataGet iteratee, 
								 gpointer iterator_context, 
								 gpointer data);

void                   eel_drag_init                          (EelDragInfo                     *drag_info,
							       const GtkTargetEntry            *drag_types,
							       int                              drag_type_count,
							       GdkBitmap                       *stipple);
void                   eel_drag_finalize                      (EelDragInfo                     *drag_info);
EelDragSelectionItem  *eel_drag_selection_item_new            (void);
void                   eel_drag_destroy_selection_list        (GList                           *selection_list);
GList 		      *eel_drag_build_selection_list          (GtkSelectionData                *data);
gboolean               eel_drag_items_local                   (const char                      *target_uri,
							       const GList                     *selection_list);
gboolean               eel_drag_items_in_trash                (const GList                     *selection_list);
void                   eel_drag_default_drop_action_for_icons (GdkDragContext                  *context,
							       const char                      *target_uri,
							       const GList                     *items,
							       int                             *default_action,
							       int                             *non_default_action);
gboolean               eel_drag_drag_data_get                 (GtkWidget                       *widget,
							       GdkDragContext                  *context,
							       GtkSelectionData                *selection_data,
							       guint                            info,
							       guint32                          time,
							       gpointer                         container_context,
							       EelDragEachSelectedItemIterator  each_selected_item_iterator);
int                    eel_drag_modifier_based_action         (int                              default_action,
							       int                              non_default_action);
GdkDragAction          eel_drag_drop_action_ask               (GdkDragAction                    possible_actions);
gboolean               eel_drag_autoscroll_in_scroll_region   (GtkWidget                       *widget);
void                   eel_drag_autoscroll_calculate_delta    (GtkWidget                       *widget,
							       float                           *x_scroll_delta,
							       float                           *y_scroll_delta);
void                   eel_drag_autoscroll_start              (EelDragInfo                     *drag_info,
							       GtkWidget                       *widget,
							       GtkFunction                      callback,
							       gpointer                         user_data);
void                   eel_drag_autoscroll_stop               (EelDragInfo                     *drag_info);

#endif
