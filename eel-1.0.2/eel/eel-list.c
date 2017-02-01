/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-list.c: Enhanced version of GtkCList for Eel.

   Copyright (C) 1999, 2000 Free Software Foundation
   Copyright (C) 2000, 2001 Eazel, Inc.

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

   Authors: Federico Mena <federico@nuclecu.unam.mx>,
            Ettore Perazzoli <ettore@gnu.org>,
            John Sullivan <sullivan@eazel.com>,
	    Pavel Cisler <pavel@eazel.com>
 */

#include <config.h>
#include "eel-list.h"

#include "eel-dnd.h"
#include "eel-list-column-title.h"
#include <ctype.h>
#include <eel/eel-background.h>
#include <eel/eel-gdk-extensions.h>
#include <eel/eel-gdk-pixbuf-extensions.h>
#include <eel/eel-gdk-pixbuf-extensions.h>
#include <eel/eel-gdk-font-extensions.h>
#include <eel/eel-glib-extensions.h>
#include <eel/eel-graphic-effects.h>
#include <eel/eel-gtk-extensions.h>
#include <eel/eel-gtk-macros.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gtk/gtkbindings.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtkenums.h>
#include <gtk/gtkmain.h>
#include <string.h>

/* Timeout for making the row currently selected for keyboard operation visible.
 * Unlike in eel-icon-container, there appear to be no adverse effects from
 * making this 0.
 */
#define KEYBOARD_ROW_REVEAL_TIMEOUT 0

/* FIXME bugzilla.eazel.com 2573: This constant and much of the code surrounding its use was copied from
 * eel-icon-container; they should share code instead.
 */
#define CONTEXT_MENU_TIMEOUT_INTERVAL 500

#define NO_BUTTON		0
#define ACTION_BUTTON		1
#define CONTEXTUAL_MENU_BUTTON	3

#define CLIST_UNFROZEN(clist) eel_clist_check_unfrozen (clist)

struct EelListDetails
{
	/* Single click mode ? */
	gboolean single_click_mode;

	/* Anti-aliased mode ? */
	gboolean anti_aliased_mode;

	/* The anchor row for range selections */
	int anchor_row;

	/* Mouse information saved on button press */
	guint dnd_press_button;
	int dnd_press_x, dnd_press_y;
	int button_down_row;
	guint32 button_down_time;

	/* Timeout used to make a selected row fully visible after a short
	 * period of time. (The timeout is needed to make sure
	 * double-clicking still works, and to optimize holding down arrow key.)
	 */
	guint keyboard_row_reveal_timer_id;
	int keyboard_row_to_reveal;

	/* Typeahead state */
	char *type_select_pattern;
	gint64 last_typeselect_time;

	/* Signal IDs that we sometimes want to block. */
	guint select_row_signal_id;
	guint unselect_row_signal_id;

	/* Drag state */
	EelDragInfo *drag_info;
	gboolean drag_started;
	gboolean rejects_dropped_icons;
	
	guint context_menu_timeout_id;
	
	/* Delayed selection information */
	gboolean dnd_select_pending;
	guint dnd_select_pending_state;

	/* Targets for drag data */
	GtkTargetList *target_list; 
	
	EelCListRow *drag_prelight_row;
	
	GtkWidget *title;

	/* Rendering state */
	GdkGC *cell_lighter_background;
	GdkGC *cell_darker_background;
	GdkGC *cell_selected_lighter_background;
	GdkGC *cell_selected_darker_background;
	GdkGC *cell_divider_color;
	GdkGC *selection_light_color;
	GdkGC *selection_medium_color;
	GdkGC *selection_main_color;
	GdkGC *text_color;
	GdkGC *selected_text_color;
	GdkGC *link_text_color;

	/* Need RGB background values when compositing images */
	guint32 cell_lighter_background_rgb;
	guint32 cell_darker_background_rgb;
	guint32 cell_selected_lighter_background_rgb;
	guint32 cell_selected_darker_background_rgb;
	guint32 selection_light_color_rgb;
	guint32 selection_medium_color_rgb;
	guint32 selection_main_color_rgb;

	gboolean alternate_row_colors;
	gulong background_color_offset, selection_color_offset;
};

/* maximum amount of milliseconds the mouse button is allowed to stay down and still be considered a click */
#define MAX_CLICK_TIME 1500

/* horizontal space between images in a pixbuf list cell */
#define PIXBUF_LIST_SPACING	2

/* Some #defines stolen from gtkclist.c that we need for other stolen code. */

/* minimum allowed width of a column */
#define COLUMN_MIN_WIDTH 5

/* this defines the base grid spacing */
#define CELL_SPACING 1

/* added the horizontal space at the beginning and end of a row */
#define COLUMN_INSET 3

/* the width of the column resize windows */
#define DRAG_WIDTH  6

/* gives the left pixel of the given column in context of
 * the clist's hoffset */
#define COLUMN_LEFT_XPIXEL(clist, colnum)  ((clist)->column[(colnum)].area.x + \
					    (clist)->hoffset)

/* gives the top pixel of the given row in context of
 * the clist's voffset */
#define ROW_TOP_YPIXEL(clist, row) (((clist)->row_height * (row)) + \
				    (((row) + 1) * CELL_SPACING) + \
				    (clist)->voffset)

/* returns the row index from a y pixel location in the 
 * context of the clist's voffset */
#define ROW_FROM_YPIXEL(clist, y)  (((y) - (clist)->voffset) / \
				    ((clist)->row_height + CELL_SPACING))

/* returns the GList item for the nth row */
#define	ROW_ELEMENT(clist, row)	(((row) == (clist)->rows - 1) ? \
				 (clist)->row_list_end : \
				 g_list_nth ((clist)->row_list, (row)))

/* returns the total height of the list */
#define LIST_HEIGHT(clist)         (((clist)->row_height * ((clist)->rows)) + \
				    (CELL_SPACING * ((clist)->rows + 1)))

enum {
	CONTEXT_CLICK_SELECTION,
	CONTEXT_CLICK_BACKGROUND,
	ACTIVATE,
	SELECTION_CHANGED,
	SELECT_MATCHING_NAME,
	SELECT_PREVIOUS_NAME,
	SELECT_NEXT_NAME,
	HANDLE_DROPPED_ITEMS,
	HANDLE_DRAGGED_ITEMS,
	GET_DEFAULT_ACTION,
	GET_DRAG_PIXBUF,
	GET_SORT_COLUMN_INDEX,
	GET_CELL_TEXT,
	LAST_SIGNAL
};

static GtkTargetEntry eel_list_dnd_target_table[] = {
	{ EEL_ICON_DND_GNOME_ICON_LIST_TYPE, 0, EEL_ICON_DND_GNOME_ICON_LIST },
	{ EEL_ICON_DND_URI_LIST_TYPE, 0, EEL_ICON_DND_URI_LIST },
	{ EEL_ICON_DND_URL_TYPE, 0, EEL_ICON_DND_URL },
	{ EEL_ICON_DND_COLOR_TYPE, 0, EEL_ICON_DND_COLOR },
	{ EEL_ICON_DND_BGIMAGE_TYPE, 0, EEL_ICON_DND_BGIMAGE },
	{ EEL_ICON_DND_KEYWORD_TYPE, 0, EEL_ICON_DND_KEYWORD }
};

static GtkTargetList *eel_list_dnd_target_list = NULL;

static void     activate_row                       (EelList          *list,
						    int               row);
static int      get_cell_horizontal_start_position (EelCList         *clist,
						    EelCListRow      *row,
						    int               column_index,
						    int               content_width);
static void     eel_list_initialize_class          (EelListClass     *class);
static void     eel_list_initialize                (EelList          *list);
static void     eel_list_destroy                   (GtkObject        *object);
static int      eel_list_button_press              (GtkWidget        *widget,
						    GdkEventButton   *event);
static int      eel_list_button_release            (GtkWidget        *widget,
						    GdkEventButton   *event);
static int      eel_list_motion                    (GtkWidget        *widget,
						    GdkEventMotion   *event);
static void     eel_list_drag_end                  (GtkWidget        *widget,
						    GdkDragContext   *context);
static void     eel_list_drag_leave                (GtkWidget        *widget,
						    GdkDragContext   *context,
						    guint             time);
static gboolean eel_list_drag_motion               (GtkWidget        *widget,
						    GdkDragContext   *context,
						    int               x,
						    int               y,
						    guint             time);
static gboolean eel_list_drag_drop                 (GtkWidget        *widget,
						    GdkDragContext   *context,
						    int               x,
						    int               y,
						    guint             time);
static void     eel_list_drag_data_received        (GtkWidget        *widget,
						    GdkDragContext   *context,
						    int               x,
						    int               y,
						    GtkSelectionData *data,
						    guint             info,
						    guint             time);
static void     eel_list_clear_keyboard_focus      (EelList          *list);
static void     eel_list_draw_focus                (GtkWidget        *widget);
static int      eel_list_key_press                 (GtkWidget        *widget,
						    GdkEventKey      *event);
static void     eel_list_unselect_all              (EelCList         *clist);
static void     eel_list_select_all                (EelCList         *clist);
static void     schedule_keyboard_row_reveal       (EelList          *list,
						    int               row);
static void     unschedule_keyboard_row_reveal     (EelList          *list);
static void     emit_selection_changed             (EelList          *clist);
static void     eel_list_clear                     (EelCList         *clist);
static void     eel_list_draw                      (GtkWidget        *widget,
						    GdkRectangle     *area);
static int      eel_list_expose                    (GtkWidget        *widget,
						    GdkEventExpose   *event);
static void     draw_rows                          (EelCList         *clist,
						    GdkRectangle     *area);
static void     draw_row                           (EelCList         *list,
						    GdkRectangle     *area,
						    int               row_index,
						    EelCListRow      *row);
static void     draw_all                           (EelCList         *clist);
static void     eel_list_style_set                 (GtkWidget        *widget,
						    GtkStyle         *previous_style);
static void     eel_list_realize                   (GtkWidget        *widget);
static void     eel_list_unrealize                 (GtkWidget        *widget);
static gboolean eel_list_set_cell_contents         (EelCList         *clist,
						    EelCListRow      *row,
						    int               column_index,
						    EelCellType       type,
						    const gchar      *text,
						    guint8            spacing,
						    GdkPixbuf        *pixbuf);
static void     eel_list_size_request              (GtkWidget        *widget,
						    GtkRequisition   *requisition);
static void     eel_list_resize_column             (EelCList         *widget,
						    int               column_index,
						    int               width);
static void     eel_list_column_resize_track_start (GtkWidget        *widget,
						    int               column_index);
static void     eel_list_column_resize_track       (GtkWidget        *widget,
						    int               column_index);
static void     eel_list_column_resize_track_end   (GtkWidget        *widget,
						    int               column_index);
static gboolean row_set_selected                   (EelList          *list,
						    int               row_index,
						    EelCListRow      *row,
						    gboolean          select);
static gboolean select_row_unselect_others         (EelList          *list,
						    int               row_to_select);
static void     eel_list_flush_typeselect_state    (EelList          *container);
static int      insert_row                         (EelCList         *list,
						    int               row,
						    char             *text[]);
static void     eel_list_ensure_drag_data          (EelList          *list,
						    GdkDragContext   *context,
						    guint32           time);
static void     eel_list_start_auto_scroll         (EelList          *list);
static void     eel_list_stop_auto_scroll          (EelList          *list);
static void     unref_gcs                          (EelList          *list);


EEL_DEFINE_CLASS_BOILERPLATE (EelList, eel_list, EEL_TYPE_CLIST)

static guint list_signals[LAST_SIGNAL];

static GtkTargetEntry drag_types [] = {
	{ EEL_ICON_DND_GNOME_ICON_LIST_TYPE, 0, EEL_ICON_DND_GNOME_ICON_LIST },
	{ EEL_ICON_DND_URI_LIST_TYPE, 0, EEL_ICON_DND_URI_LIST },
	{ EEL_ICON_DND_URL_TYPE, 0, EEL_ICON_DND_URL }
};


/* Standard class initialization function */
static void
eel_list_initialize_class (EelListClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	EelCListClass *clist_class;
	EelListClass *list_class;

	GtkBindingSet *clist_binding_set;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	clist_class = (EelCListClass *) klass;
	list_class = (EelListClass *) klass;

	list_signals[CONTEXT_CLICK_SELECTION] =
		gtk_signal_new ("context_click_selection",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, context_click_selection),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_POINTER);
	list_signals[CONTEXT_CLICK_BACKGROUND] =
		gtk_signal_new ("context_click_background",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, context_click_background),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_POINTER);
	list_signals[ACTIVATE] =
		gtk_signal_new ("activate",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, activate),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_POINTER);
	list_signals[SELECTION_CHANGED] =
		gtk_signal_new ("selection_changed",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, selection_changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	list_signals[SELECT_MATCHING_NAME] =
		gtk_signal_new ("select_matching_name",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, select_matching_name),
				gtk_marshal_NONE__STRING,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_STRING, 0);
	list_signals[SELECT_PREVIOUS_NAME] =
		gtk_signal_new ("select_previous_name",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, select_previous_name),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	list_signals[SELECT_NEXT_NAME] =
		gtk_signal_new ("select_next_name",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, select_next_name),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	list_signals[HANDLE_DRAGGED_ITEMS] =
		gtk_signal_new ("handle_dragged_items",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, handle_dragged_items),
				eel_gtk_marshal_BOOL__INT_POINTER_INT_INT_UINT,
				GTK_TYPE_BOOL, 
				5,
				GTK_TYPE_INT,
				GTK_TYPE_POINTER,
				GTK_TYPE_INT,
				GTK_TYPE_INT,
				GTK_TYPE_UINT);
	list_signals[HANDLE_DROPPED_ITEMS] =
		gtk_signal_new ("handle_dropped_items",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, handle_dropped_items),
				eel_gtk_marshal_NONE__INT_POINTER_INT_INT_UINT,
				GTK_TYPE_NONE, 5,
				GTK_TYPE_INT,
				GTK_TYPE_POINTER,
				GTK_TYPE_INT,
				GTK_TYPE_INT,
				GTK_TYPE_UINT);
	list_signals[GET_DEFAULT_ACTION] =
		gtk_signal_new ("get_default_action",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, get_default_action),
				eel_gtk_marshal_NONE__POINTER_POINTER_POINTER_POINTER_INT_INT_UINT,
				GTK_TYPE_NONE, 7,
				GTK_TYPE_POINTER,
				GTK_TYPE_POINTER,
				GTK_TYPE_POINTER,
				GTK_TYPE_POINTER,
				GTK_TYPE_INT,
				GTK_TYPE_INT,
				GTK_TYPE_UINT);
	list_signals[GET_DRAG_PIXBUF] =
		gtk_signal_new ("get_drag_pixbuf",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, get_drag_pixbuf),
				eel_gtk_marshal_POINTER__INT,
				GTK_TYPE_POINTER, 1,
				GTK_TYPE_INT);
	list_signals[GET_SORT_COLUMN_INDEX] =
		gtk_signal_new ("get_sort_column_index",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, get_sort_column_index),
				eel_gtk_marshal_INT__NONE,
				GTK_TYPE_INT, 0);
	list_signals[GET_CELL_TEXT] =
		gtk_signal_new ("get_cell_text",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelListClass, get_cell_text),
				eel_gtk_marshal_POINTER__INT_INT_POINTER_POINTER,
				GTK_TYPE_POINTER, 4,
				GTK_TYPE_INT,
				GTK_TYPE_INT,
				GTK_TYPE_POINTER,
				GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, list_signals, LAST_SIGNAL);

	/* Turn off the GtkCList key bindings that we want unbound.
	 * We only need to do this for the keys that we don't handle
	 * in eel_list_key_press. These extra ones are turned off
	 * to avoid inappropriate GtkCList code and to standardize the
	 * keyboard behavior in Eel.
	 */
	clist_binding_set = gtk_binding_set_by_class (clist_class);

	/* Use Control-A for Select All, not Control-/ */
	gtk_binding_entry_clear (clist_binding_set, 
				 '/', 
				 GDK_CONTROL_MASK);
	/* Don't use Control-\ for Unselect All (maybe invent Eel 
	 * standard for this?) */
	gtk_binding_entry_clear (clist_binding_set, 
				 '\\', 
				 GDK_CONTROL_MASK);
	/* Hide GtkCList's weird extend-selection-from-keyboard stuff.
	 * Users can use control-navigation and control-space to create
	 * extended selections.
	 */
	gtk_binding_entry_clear (clist_binding_set, 
				 GDK_Shift_L, 
				 GDK_RELEASE_MASK | GDK_SHIFT_MASK);
	gtk_binding_entry_clear (clist_binding_set, 
				 GDK_Shift_R, 
				 GDK_RELEASE_MASK | GDK_SHIFT_MASK);
	gtk_binding_entry_clear (clist_binding_set, 
				 GDK_Shift_L, 
				 GDK_RELEASE_MASK | GDK_SHIFT_MASK | GDK_CONTROL_MASK);
	gtk_binding_entry_clear (clist_binding_set, 
				 GDK_Shift_R, 
				 GDK_RELEASE_MASK | GDK_SHIFT_MASK | GDK_CONTROL_MASK);

	list_class->column_resize_track_start = eel_list_column_resize_track_start;
	list_class->column_resize_track = eel_list_column_resize_track;
	list_class->column_resize_track_end = eel_list_column_resize_track_end;

	clist_class->clear = eel_list_clear;
	clist_class->draw_row = draw_row;
	clist_class->draw_rows = draw_rows;
	clist_class->draw_all = draw_all;
	clist_class->insert_row = insert_row;
  	clist_class->resize_column = eel_list_resize_column;
  	clist_class->set_cell_contents = eel_list_set_cell_contents;
  	clist_class->select_all = eel_list_select_all;
  	clist_class->unselect_all = eel_list_unselect_all;

	widget_class->button_press_event = eel_list_button_press;
	widget_class->button_release_event = eel_list_button_release;
	widget_class->motion_notify_event = eel_list_motion;

	widget_class->draw = eel_list_draw;
	widget_class->expose_event = eel_list_expose;
	widget_class->draw_focus = eel_list_draw_focus;
	widget_class->key_press_event = eel_list_key_press;
	widget_class->style_set = eel_list_style_set;
	widget_class->realize = eel_list_realize;
	widget_class->unrealize = eel_list_unrealize;
	widget_class->size_request = eel_list_size_request;

	object_class->destroy = eel_list_destroy;
}

static gboolean 
event_state_modifies_selection (guint event_state)
{
	return (event_state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) != 0;
}

void
eel_list_set_single_click_mode (EelList *list,
				     gboolean single_click_mode)
{
	list->details->single_click_mode = single_click_mode;
	gtk_widget_queue_draw (GTK_WIDGET (list));
}

void       
eel_list_set_sort_column (EelList *list,
			  int column)
{
  g_return_if_fail (EEL_IS_LIST (list));

  eel_clist_set_sort_column (EEL_CLIST (list), column);

  /* FIXME: Would be better to emit a SORT_COLUMN_CHANGED signal
   * here and have column title widget handle the signal by queueing
   * its own redraw.
   */
  eel_list_column_title_queue_draw (EEL_LIST_COLUMN_TITLE (list->details->title));
}

void       
eel_list_set_sort_type (EelList *list,
			GtkSortType sort_type)
{
  g_return_if_fail (EEL_IS_LIST (list));

  eel_clist_set_sort_type (EEL_CLIST (list), sort_type);

  /* FIXME: Would be better to emit a SORT_TYPE_CHANGED signal
   * here and have column title widget handle the signal by queueing
   * its own redraw.
   */
  eel_list_column_title_queue_draw (EEL_LIST_COLUMN_TITLE (list->details->title));
}

void
eel_list_set_anti_aliased_mode (EelList *list,
				     gboolean anti_aliased_mode)
{
	list->details->anti_aliased_mode = anti_aliased_mode;
	gtk_widget_queue_draw (GTK_WIDGET (list));
}

gboolean
eel_list_is_anti_aliased (EelList *list)
{
	return list->details->anti_aliased_mode;
}


void
eel_list_initialize_dnd (EelList *list)
{
	g_assert (list->details->drag_info == NULL);
	g_assert (!GTK_WIDGET_REALIZED (list));

	list->details->drag_info = g_new0 (EelDragInfo, 1);

	eel_drag_init (list->details->drag_info, drag_types,
			    EEL_N_ELEMENTS (drag_types), NULL);
	
	gtk_signal_connect (GTK_OBJECT (list), 
			    "drag_end", 
			    GTK_SIGNAL_FUNC (eel_list_drag_end), 
			    list);
	
	gtk_signal_connect (GTK_OBJECT (list), 
			    "drag_leave", 
			    GTK_SIGNAL_FUNC (eel_list_drag_leave), 
			    list);

	gtk_signal_connect (GTK_OBJECT (list),
			    "drag_motion", 
			    GTK_SIGNAL_FUNC (eel_list_drag_motion), 
			    list);

	gtk_signal_connect (GTK_OBJECT (list), 
			    "drag_drop", 
			    GTK_SIGNAL_FUNC (eel_list_drag_drop), 
			    list);

	gtk_signal_connect (GTK_OBJECT (list), 
			    "drag_data_received", 
			    GTK_SIGNAL_FUNC (eel_list_drag_data_received), 
			    list);


	/* Get ready to accept some dragged stuff. */
	gtk_drag_dest_set (GTK_WIDGET (list),
			   0,
			   eel_list_dnd_target_table,
			   EEL_N_ELEMENTS (eel_list_dnd_target_table),
			   GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK
			   | GDK_ACTION_ASK);

}

/* Standard object initialization function */
static void
eel_list_initialize (EelList *list)
{	
	list->details = g_new0 (EelListDetails, 1);
	list->details->anchor_row = -1;

	list->details->drag_prelight_row = NULL;

	list->details->alternate_row_colors = TRUE;
	list->details->background_color_offset = G_STRUCT_OFFSET (GtkStyle, bg[GTK_STATE_NORMAL]);
	list->details->selection_color_offset = G_STRUCT_OFFSET (GtkStyle, bg[GTK_STATE_SELECTED]);

	/* GtkCList does not specify pointer motion by default */
	gtk_widget_add_events (GTK_WIDGET (list), GDK_POINTER_MOTION_MASK);

	/* Emit "selection changed" signal when parent class changes selection */
	list->details->select_row_signal_id = gtk_signal_connect (GTK_OBJECT (list),
			    					  "select_row",
			    					  emit_selection_changed,
			    					  list);
	list->details->unselect_row_signal_id = gtk_signal_connect (GTK_OBJECT (list),
			    					    "unselect_row",
			    					    emit_selection_changed,
			    					    list);

	gtk_widget_push_composite_child ();
	list->details->title = GTK_WIDGET (eel_list_column_title_new());
	gtk_widget_pop_composite_child ();

	list->details->type_select_pattern = NULL;
	list->details->last_typeselect_time = G_GINT64_CONSTANT(0);
}

static void
eel_list_destroy (GtkObject *object)
{
	EelList *list;

	list = EEL_LIST (object);

	if (list->details->drag_info != NULL) {
		eel_drag_finalize (list->details->drag_info);
	}

	unschedule_keyboard_row_reveal (list);

	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));

	unref_gcs (list);

	g_free (list->details->type_select_pattern);

	/* Must do this after calling the parent, because GtkCList calls
	 * the clear method, which must have a valid details pointer.
	 */
	g_free (list->details);
}

static void
emit_selection_changed (EelList *list) 
{
	g_assert (EEL_IS_LIST (list));
	gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECTION_CHANGED]);
}

static void
activate_row_data_list (EelList *list, GList *activate_list)
{
	gtk_signal_emit (GTK_OBJECT (list),
			 list_signals[ACTIVATE],
			 activate_list);
}

static void
activate_selected_rows (EelList *list)
{
	GList *selection;

	selection = eel_list_get_selection (list);
	activate_row_data_list (list, selection);
	g_list_free (selection);
}

static void
activate_row (EelList *list, int row)
{
	GList *singleton_list;

	singleton_list = NULL;
	singleton_list = g_list_append (NULL, eel_clist_get_row_data (EEL_CLIST (list), row));
	activate_row_data_list (list, singleton_list);
	g_list_free (singleton_list);
}

gboolean
eel_list_is_row_selected (EelList *list, int row)
{
	EelCListRow *elem;

	g_return_val_if_fail (row >= 0, FALSE);
	g_return_val_if_fail (row < EEL_CLIST (list)->rows, FALSE);

	elem = g_list_nth (EEL_CLIST (list)->row_list, row)->data;

	return elem->state == GTK_STATE_SELECTED;
}

/* Selects the rows between the anchor to the specified row, inclusive.
 * Returns TRUE if selection changed.  */
static gboolean
select_range (EelList *list, int row)
{
	int min, max;
	int i;
	gboolean selection_changed;

	selection_changed = FALSE;

	if (list->details->anchor_row == -1) {
		list->details->anchor_row = row;
	}

	if (row < list->details->anchor_row) {
		min = row;
		max = list->details->anchor_row;
	} else {
		min = list->details->anchor_row;
		max = row;
	}

	for (i = min; i <= max; i++) {
		selection_changed |= row_set_selected (list, i, NULL, TRUE);
	}

	return selection_changed;
}

/* Handles row selection according to the specified modifier state */
static void
select_row_from_mouse (EelList *list, int row, guint state)
{
	int range, additive;
	gboolean should_select_row;
	gboolean selection_changed;

	selection_changed = FALSE;

	range = (state & GDK_SHIFT_MASK) != 0;
	additive = (state & GDK_CONTROL_MASK) != 0;

	eel_list_clear_keyboard_focus (list);

	if (!additive) {
		selection_changed |= select_row_unselect_others (list, -1);
	}

	if (range) {
		selection_changed |= select_range (list, row);
	} else {
		should_select_row = !additive || !eel_list_is_row_selected (list, row);
		selection_changed |= row_set_selected (list, row, NULL, should_select_row);
		list->details->anchor_row = row;
	}

	if (selection_changed) {
		emit_selection_changed (list);
	}
}

/* 
 * row_set_selected:
 * 
 * Select or unselect a row. Return TRUE if selection has changed. 
 * Does not emit the SELECTION_CHANGED signal; it's up to the caller
 * to handle that.
 *
 * @list: The EelList in question.
 * @row: index of row number to select or unselect.
 * @row: EelCListRow pointer for given list. Passing this avoids
 * expensive lookup. If it's NULL, it will be looked up in this function.
 * @select: TRUE if row should be selected, FALSE otherwise.
 * 
 * Return Value: TRUE if selection has changed, FALSE otherwise.
 */
static gboolean
row_set_selected (EelList *list, int row_index, EelCListRow *row, gboolean select)
{
	g_assert (row_index >= 0 && row_index < EEL_CLIST (list)->rows);

	if (row == NULL) {
		row = ROW_ELEMENT (EEL_CLIST (list), row_index)->data;
	}

	if (select == (row->state == GTK_STATE_SELECTED)) {
		return FALSE;
	}

	/* Block signal handlers so we can make sure the selection-changed
	 * signal gets sent only once.
	 */
	gtk_signal_handler_block (GTK_OBJECT(list), 
				  list->details->select_row_signal_id);
	gtk_signal_handler_block (GTK_OBJECT(list), 
				  list->details->unselect_row_signal_id);
	
	if (select) {
		eel_clist_select_row (EEL_CLIST (list), row_index, -1);
	} else {
		eel_clist_unselect_row (EEL_CLIST (list), row_index, -1);
	}

	gtk_signal_handler_unblock (GTK_OBJECT(list), 
				    list->details->select_row_signal_id);
	gtk_signal_handler_unblock (GTK_OBJECT(list), 
				    list->details->unselect_row_signal_id);

	return TRUE;
}

/**
 * select_row_unselect_others:
 * 
 * Change the selected rows as necessary such that only
 * the given row remains selected.
 * 
 * @list: The EelList in question.
 * @row_to_select: The row number to leave selected. Use -1 to leave
 * no row selected.
 * 
 * Return value: TRUE if the selection changed; FALSE otherwise.
 */
static gboolean
select_row_unselect_others (EelList *list, int row_to_select)
{
	GList *p;
	int row_index;
	gboolean selection_changed;

	g_return_val_if_fail (EEL_IS_LIST (list), FALSE);

	selection_changed = FALSE;
	for (p = EEL_CLIST (list)->row_list, row_index = 0; p != NULL; p = p->next, ++row_index) {
		selection_changed |= row_set_selected (list, row_index, p->data, row_index == row_to_select);
	}

	return selection_changed;
}

static void
eel_list_unselect_all (EelCList *clist)
{
	g_return_if_fail (EEL_IS_LIST (clist));

	if (select_row_unselect_others (EEL_LIST (clist), -1)) {
		emit_selection_changed (EEL_LIST (clist));
	}
}

static void
eel_list_select_all (EelCList *clist)
{
	GList *p;
	int row_index;
	gboolean selection_changed;

	g_return_if_fail (EEL_IS_LIST (clist));

	selection_changed = FALSE;
	for (p = clist->row_list, row_index = 0; p != NULL; p = p->next, ++row_index) {
		selection_changed |= row_set_selected (EEL_LIST (clist), row_index, p->data, TRUE);
	}

	if (selection_changed) {
		emit_selection_changed (EEL_LIST (clist));
	}
}

typedef struct {
	EelList 	*list;
	GdkEventButton	*event;
} ContextMenuParameters;

static ContextMenuParameters *
context_menu_parameters_new (EelList *list,
			     GdkEventButton *event)
{
	ContextMenuParameters *parameters;

	parameters = g_new (ContextMenuParameters, 1);
	parameters->list = list;
	parameters->event = (GdkEventButton *)(gdk_event_copy ((GdkEvent *)event));

	return parameters;
}			     

static void
context_menu_parameters_free (ContextMenuParameters *parameters)
{
	gdk_event_free ((GdkEvent *)parameters->event);
	g_free (parameters);
}

static gboolean
show_context_menu_callback (void *cast_to_parameters)
{
	ContextMenuParameters *parameters;

	parameters = (ContextMenuParameters *)cast_to_parameters;

	g_assert (EEL_IS_LIST (parameters->list));

	/* FIXME bugzilla.eazel.com 2574: 
	 * Need to handle case where button has already been released,
	 * a la EelIconContainer code?
	 */

	gtk_timeout_remove (parameters->list->details->context_menu_timeout_id);

	/* Context menu applies to all selected items. The only
	 * odd case is if this click deselected the item under
	 * the mouse, but at least the behavior is consistent.
	 */
	gtk_signal_emit (GTK_OBJECT (parameters->list),
			 list_signals[CONTEXT_CLICK_SELECTION],
			 parameters->event);

	context_menu_parameters_free (parameters);

	return TRUE;
}

static char *
get_cell_text (EelCList *clist, int column_index, int cell_width,
	EelCListRow *row, GdkFont *font)
{
	char *result;
	
	result = NULL;
	gtk_signal_emit_by_name (GTK_OBJECT (clist), "get_cell_text",
		column_index, cell_width, row, font, &result);
	
	return result;
}

static GdkRectangle
eel_list_get_cell_hit_rectangle (EelList *list, int row_index, int column_index)
{
	EelCList *clist;
	EelCListRow *row;
	GdkRectangle result;
	GtkStyle *style;
	GdkGC *fg_gc;
	GdkGC *bg_gc;
	guint32 bg_rgb;
	int width, pixbuf_width, height;
	char *text;
	GList *p;

	clist = EEL_CLIST (list);
	row = ROW_ELEMENT (clist, row_index)->data;
	g_assert (row != NULL);
	
	eel_list_get_cell_rectangle (EEL_LIST (clist), row_index, column_index, &result);

	eel_list_get_cell_style (EEL_LIST(clist), row, row->state, row_index, 
				 column_index, &style, &fg_gc, &bg_gc, &bg_rgb);

	width = 0;
	
	switch ((EelCellType) row->cell[column_index].type) {
	case EEL_CELL_TEXT:
	case EEL_CELL_LINK_TEXT:
		text = get_cell_text (clist, column_index, clist->column[column_index].area.width,
			row, style->font);
		if (text != NULL) {
			width = gdk_string_width (style->font, text);
		}
		break;
	case EEL_CELL_PIXBUF:
		width = gdk_pixbuf_get_width (EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf);
		height = gdk_pixbuf_get_height (EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf);
		break;
	case EEL_CELL_PIXTEXT:
		pixbuf_width = gdk_pixbuf_get_width (EEL_CELL_PIXTEXT (row->cell[column_index])->pixbuf);
		height = gdk_pixbuf_get_height (EEL_CELL_PIXTEXT (row->cell[column_index])->pixbuf);
		text = get_cell_text (clist, column_index,
			clist->column[column_index].area.width - pixbuf_width
				- EEL_CELL_PIXTEXT (row->cell[column_index])->spacing,
			row, style->font);
		width = pixbuf_width +
 			EEL_CELL_PIXTEXT (row->cell[column_index])->spacing +
			text == NULL ? 0 : gdk_string_width (style->font, text);
		break;
	case EEL_CELL_PIXBUF_LIST:
		for (p = EEL_CELL_PIXBUF_LIST (row->cell[column_index])->pixbufs; 
			p != NULL; p = p->next) {
			if (width != 0) {
				width += PIXBUF_LIST_SPACING;
			}
			width += gdk_pixbuf_get_width (p->data);
		}
		break;
	case EEL_CELL_EMPTY:
	case EEL_CELL_WIDGET:
		return result;
	}

	result.x = get_cell_horizontal_start_position (clist, row, column_index, width);
	result.width = width;
	
	return result;
}


static int
eel_list_item_hit (EelList *list, int x, int y)
{
	EelCList *clist;
	int row_index, column_index;
	GdkRectangle column_item_rectangle;

	clist = EEL_CLIST (list);

	if (!eel_clist_get_selection_info (clist, x, y, &row_index, &column_index))
		// didn't hit any populated row
		return -1;
	
	column_item_rectangle = eel_list_get_cell_hit_rectangle (list, row_index,
		column_index);
	
	if (!eel_rectangle_contains (&column_item_rectangle, x, y))
		// didn't hit any active part of a cell
		return -1;

	return row_index;
}


/* Our handler for button_press events.  We override all of GtkCList's broken
 * behavior.
 */
static int
eel_list_button_press (GtkWidget *widget, GdkEventButton *event)
{
	EelList *list;
	EelCList *clist;
	int row_index;
	int retval;

	g_return_val_if_fail (EEL_IS_LIST (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	list = EEL_LIST (widget);
	clist = EEL_CLIST (widget);
	retval = FALSE;

	if (!GTK_WIDGET_HAS_FOCUS (widget)) {
		gtk_widget_grab_focus (widget);
	}


	/* Forget the typeahead state. */
	eel_list_flush_typeselect_state (list);

	if (event->window != clist->clist_window)
		return EEL_CALL_PARENT_WITH_RETURN_VALUE
			(GTK_WIDGET_CLASS, button_press_event, (widget, event));

	row_index = eel_list_item_hit (list, event->x, event->y);
	list->details->button_down_time = event->time;
	list->details->drag_started = FALSE;

	list->details->button_down_row = -1;
		
	switch (event->type) {
	case GDK_BUTTON_PRESS:
	
		if (event->button == CONTEXTUAL_MENU_BUTTON && row_index < 0) {
			gtk_signal_emit (GTK_OBJECT (list),
					 list_signals[CONTEXT_CLICK_BACKGROUND],
					 event);

			retval = TRUE;
		} else if (event->button == ACTION_BUTTON || event->button == CONTEXTUAL_MENU_BUTTON) {
			if (row_index >= 0) {

				if (event->button == CONTEXTUAL_MENU_BUTTON) {
					/* after a timeout we will decide if this is a
					 * context menu click or a drag start
					 */
					list->details->context_menu_timeout_id = gtk_timeout_add (
						CONTEXT_MENU_TIMEOUT_INTERVAL, 
						show_context_menu_callback, 				
						context_menu_parameters_new (list, event));
				}

				/* Save the clicked row_index for DnD and single-click activate */
				
				list->details->button_down_row = row_index;

				/* Save the mouse info for DnD */

				list->details->dnd_press_button = event->button;
				list->details->dnd_press_x = event->x;
				list->details->dnd_press_y = event->y;
	
				/* Handle selection */

				if ((eel_list_is_row_selected (list, row_index)
				     && !event_state_modifies_selection (event->state))
				    || ((event->state & GDK_CONTROL_MASK)
					&& !(event->state & GDK_SHIFT_MASK))) {
					/* don't change selection just yet, wait for 
					 * possible drag
					 */
					list->details->dnd_select_pending = TRUE;
					list->details->dnd_select_pending_state = event->state;
				}

				if (!list->details->dnd_select_pending) {
					select_row_from_mouse (list, row_index, event->state);
				}
			} else {
				eel_clist_unselect_all (clist);
			}

			retval = TRUE;
		}

		break;

	case GDK_2BUTTON_PRESS:
		if (event->button == ACTION_BUTTON) {
			list->details->dnd_select_pending = FALSE;
			list->details->dnd_select_pending_state = 0;

			if (row_index >= 0 && !list->details->single_click_mode) {
				/* We'll just eat the 2nd click if in single-click mode. */
				activate_selected_rows (list);
			}

			retval = TRUE;
			break;
		}

	default:
		break;
	}

	return retval;
}

/* Our handler for button_release events.  We override all of GtkCList's broken
 * behavior.
 */
static int
eel_list_button_release (GtkWidget *widget, GdkEventButton *event)
{
	EelList *list;
	EelCList *clist;
	EelCListRow *row;
	gboolean drag_started, on_row;
	int row_index, column_index;
	int elapsed_time;
	GdkRectangle hit_rectangle;

	g_return_val_if_fail (EEL_IS_LIST (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	list = EEL_LIST (widget);
	clist = EEL_CLIST (widget);

	if (event->window != clist->clist_window
	    && event->button != list->details->dnd_press_button)
		return EEL_CALL_PARENT_WITH_RETURN_VALUE
			(GTK_WIDGET_CLASS, button_release_event, (widget, event));

	drag_started = list->details->drag_started;

	list->details->dnd_press_button = NO_BUTTON;
	list->details->dnd_press_x = 0;
	list->details->dnd_press_y = 0;
	list->details->drag_started = FALSE;

	/* Clean up after abortive drag-and-drop attempt (since user can't
	 * reorder list view items, releasing mouse in list view cancels
	 * drag-and-drop possibility). 
	 */
	if (list->details->dnd_select_pending) {
		/* If clicked on a selected item, don't change selection 
		 * (unless perhaps if modifiers were used)
		 */
		if (!eel_list_is_row_selected (list, list->details->button_down_row) 
		    || event_state_modifies_selection (list->details->dnd_select_pending_state)) {
			select_row_from_mouse (list,
					       list->details->button_down_row,
					       list->details->dnd_select_pending_state);
		}
		
		list->details->dnd_select_pending = FALSE;
		list->details->dnd_select_pending_state = 0;
	}
	
	if (event->button == CONTEXTUAL_MENU_BUTTON && !drag_started) {
		/* Right click, drag never happened, immediately show context menu */
		gtk_timeout_remove (list->details->context_menu_timeout_id);
		gtk_signal_emit (GTK_OBJECT (list),
				 list_signals[CONTEXT_CLICK_SELECTION],
				 event);
		return TRUE;
	}

	/* Activate on single click if not extending selection, mouse
	 * hasn't moved to a different row, not too much time has
	 * passed, and this is a link-type cell.
	 */

	if (event->button != ACTION_BUTTON
	    || !list->details->single_click_mode
	    || event_state_modifies_selection (event->state)) {
		return FALSE;
	}

	elapsed_time = event->time - list->details->button_down_time;
	if (elapsed_time > MAX_CLICK_TIME) {
		return FALSE;
	}

	on_row = eel_clist_get_selection_info (clist, event->x, event->y,
					       &row_index, &column_index);
	if (!on_row ||  list->details->button_down_row != row_index) {
		return FALSE;
	}

	row = ROW_ELEMENT (clist, row_index)->data;
	if (row->cell[column_index].type != EEL_CELL_LINK_TEXT) {
		return FALSE;
	}

	/* One final test. Check whether the click was in the
	 * horizontal bounds of the displayed text.
	 */
	hit_rectangle = eel_list_get_cell_hit_rectangle (list, row_index, column_index);
	if (!eel_rectangle_contains (&hit_rectangle, event->x, event->y)) {
		return FALSE;
	}

	/* Note that we activate only the clicked-on item, not all
	 * selected items. This is because the UI feedback makes it
	 * clear that you're clicking on a link to activate that link,
	 * rather than activating the whole selection.
	 */
	activate_row (list, row_index);
	return TRUE;
}

static void
eel_list_clear_keyboard_focus (EelList *list)
{
	if (EEL_CLIST (list)->focus_row >= 0) {
		gtk_widget_draw_focus (GTK_WIDGET (list));
	}

	EEL_CLIST (list)->focus_row = -1;
}

static void
eel_list_set_keyboard_focus (EelList *list, int row_index)
{
	g_assert (row_index >= 0 && row_index < EEL_CLIST (list)->rows);

	if (row_index == EEL_CLIST (list)->focus_row) {
		return;
	}

	eel_list_clear_keyboard_focus (list);

	EEL_CLIST (list)->focus_row = row_index;

	gtk_widget_draw_focus (GTK_WIDGET (list));
}

static void
eel_list_keyboard_move_to (EelList *list, int row_index, GdkEventKey *event)
{
	EelCList *clist;

	g_assert (EEL_IS_LIST (list));
	g_assert (row_index >= 0 || row_index < EEL_CLIST (list)->rows);

	clist = EEL_CLIST (list);

	if (event != NULL && (event->state & GDK_CONTROL_MASK) != 0) {
		/* Move the keyboard focus. */
		eel_list_set_keyboard_focus (list, row_index);
	} else {
		/* Select row_index and get rid of special keyboard focus. */
		eel_list_clear_keyboard_focus (list);
		if (select_row_unselect_others (list, row_index)) {
			emit_selection_changed (list);
		}
	}

	schedule_keyboard_row_reveal (list, row_index);
}

void
eel_list_select_row (EelList *list, int row_index)
{
	g_assert (EEL_IS_LIST (list));
	g_assert (row_index >= 0);

	if (row_index >= EEL_CLIST (list)->rows)
		row_index = EEL_CLIST (list)->rows - 1;

	eel_list_keyboard_move_to (list, row_index, NULL);
}

static gboolean
keyboard_row_reveal_timeout_callback (gpointer data)
{
	EelList *list;
	int row_index;

	GDK_THREADS_ENTER ();

	list = EEL_LIST (data);
	row_index = list->details->keyboard_row_to_reveal;

	if (row_index >= 0 && row_index < EEL_CLIST (list)->rows) {	
		/* Only reveal the icon if it's still the keyboard
		 * focus or if it's still selected. Someone originally
		 * thought we should cancel this reveal if the user
		 * manages to sneak a direct scroll in before the
		 * timeout fires, but we later realized this wouldn't
		 * actually be an improvement (see bugzilla.eazel.com
		 * 612).
		 */
		if (row_index == EEL_CLIST (list)->focus_row
		    || eel_list_is_row_selected (list, row_index)) {
			eel_list_reveal_row (list, row_index);
		}
		list->details->keyboard_row_reveal_timer_id = 0;
	}

	GDK_THREADS_LEAVE ();

	return FALSE;
}

static void
unschedule_keyboard_row_reveal (EelList *list) 
{
	if (list->details->keyboard_row_reveal_timer_id != 0) {
		gtk_timeout_remove (list->details->keyboard_row_reveal_timer_id);
	}
}

static void
schedule_keyboard_row_reveal (EelList *list, int row_index)
{
	unschedule_keyboard_row_reveal (list);

	list->details->keyboard_row_to_reveal = row_index;
	list->details->keyboard_row_reveal_timer_id
		= gtk_timeout_add (KEYBOARD_ROW_REVEAL_TIMEOUT,
				   keyboard_row_reveal_timeout_callback,
				   list);
}

void
eel_list_reveal_row (EelList *list, int row_index)
{
	EelCList *clist;

	g_return_if_fail (EEL_IS_LIST (list));
	g_return_if_fail (row_index >= 0 && row_index < EEL_CLIST (list) ->rows);
	
	clist = EEL_CLIST (list);
	
	if (ROW_TOP_YPIXEL (clist, row_index) + clist->row_height >
      		      clist->clist_window_height) {
		eel_clist_moveto (clist, row_index, -1, 1, 0);
     	} else if (ROW_TOP_YPIXEL (clist, row_index) < 0) {
		eel_clist_moveto (clist, row_index, -1, 0, 0);
     	}
}

static void
eel_list_keyboard_navigation_key_press (EelList *list, GdkEventKey *event,
			          	     GtkScrollType scroll_type, gboolean jump_to_end)
{
	EelCList *clist;
	int start_row;
	int destination_row;
	int rows_per_page;

	g_assert (EEL_IS_LIST (list));

	clist = EEL_CLIST (list);
	
	if (scroll_type == GTK_SCROLL_JUMP) {
		destination_row = (jump_to_end ?
				   clist->rows - 1 :
				   0);
	} else {
		/* Choose the row to start with.
		 * If we have a keyboard focus, start with it.
		 * If there's a selection, use the selected row farthest toward the end.
		 */

		if (clist->focus_row >= 0) {
			start_row = clist->focus_row;
		} else {
			start_row = (scroll_type == GTK_SCROLL_STEP_FORWARD 
					|| scroll_type == GTK_SCROLL_PAGE_FORWARD ?
				     eel_list_get_last_selected_row (list) :
				     eel_list_get_first_selected_row (list));
		}

		/* If there's no row to start with, select the row farthest toward the end.
		 * If there is a row to start with, select the next row in the arrow direction.
		 */
		if (start_row < 0) {
			destination_row = (scroll_type == GTK_SCROLL_STEP_FORWARD 
						|| scroll_type == GTK_SCROLL_PAGE_FORWARD 
					   ? 0 : clist->rows - 1);
		} else if (scroll_type == GTK_SCROLL_STEP_FORWARD) {
			destination_row = MIN (clist->rows - 1, start_row + 1);
		} else if (scroll_type == GTK_SCROLL_STEP_BACKWARD) {
			destination_row = MAX (0, start_row - 1);
		} else {
			g_assert (scroll_type == GTK_SCROLL_PAGE_FORWARD || GTK_SCROLL_PAGE_BACKWARD);
			rows_per_page = (2 * clist->clist_window_height -
					 clist->row_height - CELL_SPACING) /
					(2 * (clist->row_height + CELL_SPACING));
			
			if (scroll_type == GTK_SCROLL_PAGE_FORWARD) {
				destination_row = MIN (clist->rows - 1, 
						       start_row + rows_per_page);
			} else {
				destination_row = MAX (0,
						       start_row - rows_per_page);
			}
		}
	}

	eel_list_keyboard_move_to (list, destination_row, event);
}			   

static void
eel_list_keyboard_home (EelList *list, GdkEventKey *event)
{
	/* Home selects the first row.
	 * Control-Home sets the keyboard focus to the first row.
	 */
	eel_list_keyboard_navigation_key_press (list, event, GTK_SCROLL_JUMP, FALSE); 
}

static void
eel_list_keyboard_end (EelList *list, GdkEventKey *event)
{
	/* End selects the last row.
	 * Control-End sets the keyboard focus to the last row.
	 */
	eel_list_keyboard_navigation_key_press (list, event, GTK_SCROLL_JUMP, TRUE); 
}

static void
eel_list_keyboard_up (EelList *list, GdkEventKey *event)
{
	/* Up selects the next higher row.
	 * Control-Up sets the keyboard focus to the next higher icon.
	 */
	eel_list_keyboard_navigation_key_press (list, event, GTK_SCROLL_STEP_BACKWARD, FALSE); 
}

static void
eel_list_keyboard_down (EelList *list, GdkEventKey *event)
{
	/* Down selects the next lower row.
	 * Control-Down sets the keyboard focus to the next lower icon.
	 */
	eel_list_keyboard_navigation_key_press (list, event, GTK_SCROLL_STEP_FORWARD, FALSE); 
}

static void
eel_list_keyboard_page_up (EelList *list, GdkEventKey *event)
{
	/* Page Up selects a row one screenful higher.
	 * Control-Page Up sets the keyboard focus to the row one screenful higher.
	 */
	eel_list_keyboard_navigation_key_press (list, event, GTK_SCROLL_PAGE_BACKWARD, FALSE); 
}

static void
eel_list_keyboard_page_down (EelList *list, GdkEventKey *event)
{
	/* Page Down selects a row one screenful lower.
	 * Control-Page Down sets the keyboard focus to the row one screenful lower.
	 */
	eel_list_keyboard_navigation_key_press (list, event, GTK_SCROLL_PAGE_FORWARD, FALSE); 
}

static void
eel_list_keyboard_space (EelList *list, GdkEventKey *event)
{
	if (event->state & GDK_CONTROL_MASK) {
		gtk_signal_emit_by_name (GTK_OBJECT (list), "toggle_focus_row");
	}
}

static void
eel_list_activate_selected_items (EelList *list)
{
	int row_index;

	for (row_index = 0; row_index < EEL_CLIST (list)->rows; ++row_index) {
		if (eel_list_is_row_selected (list, row_index)) {
			activate_row (list, row_index);
		}
	}
}

static void
eel_list_flush_typeselect_state (EelList *list)
{
	g_free (list->details->type_select_pattern);
	list->details->type_select_pattern = NULL;
	list->details->last_typeselect_time = G_GINT64_CONSTANT(0);
}

enum {
	EEL_TYPESELECT_FLUSH_DELAY = 1000000
	/* After this time the current typeselect buffer will be
	 * thrown away and the new pressed character will be made
	 * the the start of a new pattern.
	 */
};

static gboolean
eel_list_handle_typeahead (EelList *list, const char *key_string)
{
	char *new_pattern;
	gint64 now;
	gint64 time_delta;
	int key_string_length;
	int index;

	g_assert (key_string != NULL);
	g_assert (strlen (key_string) < 5);

	key_string_length = strlen (key_string);

	if (key_string_length == 0) {
		/* can be an empty string if the modifier was held down, etc. */
		return FALSE;
	}

	/* only handle if printable keys typed */
	for (index = 0; index < key_string_length; index++) {
		if (!isprint (key_string[index])) {
			return FALSE;
		}
	}

	/* find out how long since last character was typed */
	now = eel_get_system_time();
	time_delta = now - list->details->last_typeselect_time;
	if (time_delta < 0 || time_delta > EEL_TYPESELECT_FLUSH_DELAY) {
		/* the typeselect state is too old, start with a fresh one */
		g_free (list->details->type_select_pattern);
		list->details->type_select_pattern = NULL;
	}

	if (list->details->type_select_pattern != NULL) {
		new_pattern = g_strconcat
			(list->details->type_select_pattern,
			 key_string, NULL);
		g_free (list->details->type_select_pattern);
	} else {
		new_pattern = g_strdup (key_string);
	}

	list->details->type_select_pattern = new_pattern;
	list->details->last_typeselect_time = now;
	
	gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECT_MATCHING_NAME], new_pattern);

	return TRUE;
}

static int
eel_list_key_press (GtkWidget *widget,
		 	 GdkEventKey *event)
{
	EelList *list;

	list = EEL_LIST (widget);

	switch (event->keyval) {
	case GDK_Home:
		eel_list_keyboard_home (list, event);
		break;
	case GDK_End:
		eel_list_keyboard_end (list, event);
		break;
	case GDK_Page_Up:
		eel_list_keyboard_page_up (list, event);
		break;
	case GDK_Page_Down:
		eel_list_keyboard_page_down (list, event);
		break;
	case GDK_Up:
		eel_list_keyboard_up (list, event);
		break;
	case GDK_Down:
		eel_list_keyboard_down (list, event);
		break;
	case GDK_space:
		eel_list_keyboard_space (list, event);
		break;
	case GDK_Return:
		eel_list_activate_selected_items (list);
		break;
	case GDK_Tab:
	case GDK_ISO_Left_Tab:
		if ((event->state & GDK_SHIFT_MASK) == 0) {
			gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECT_PREVIOUS_NAME]);
		} else {
			gtk_signal_emit (GTK_OBJECT (list), list_signals[SELECT_NEXT_NAME]);
		}
		break;
	default:
		/* Don't use Control or Alt keys for type-selecting, because they
		 * might be used for menus.
		 */
		if ((event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) == 0 &&
		     eel_list_handle_typeahead (list, event->string)) {
			return TRUE;
		}

		return EEL_CALL_PARENT_WITH_RETURN_VALUE
			(GTK_WIDGET_CLASS, key_press_event, (widget, event));
	}

	return TRUE;
}

static guint32
eel_gdk_set_shifted_foreground_gc_color (GdkGC *gc, guint32 color, float shift_by)
{
	guint32 shifted_color;

	shifted_color = eel_rgb_shift_color (color, shift_by);
	gdk_rgb_gc_set_foreground (gc, shifted_color);

	return shifted_color;
}

static GdkGC *
eel_gdk_gc_copy (GdkGC *source, GdkWindow *window)
{
	GdkGC *result;

	result = gdk_gc_new (window);
	gdk_gc_copy (result, source);

	/* reset some properties to be on the safe side */
	gdk_gc_set_function (result, GDK_COPY);
	gdk_gc_set_fill (result, GDK_SOLID);
	gdk_gc_set_clip_origin (result, 0, 0);
	gdk_gc_set_clip_mask (result, NULL);

	return result;
}

static void
eel_list_setup_style_colors (EelList *list)
{
	guint32 style_background_color;
	guint32 selection_background_color;
	GdkColor text_color;

	gdk_rgb_init();

	style_background_color = eel_gdk_color_to_rgb
		(G_STRUCT_MEMBER_P (GTK_WIDGET (list)->style,
				    list->details->background_color_offset));
	selection_background_color = eel_gdk_color_to_rgb
		(G_STRUCT_MEMBER_P (GTK_WIDGET (list)->style,
				    list->details->selection_color_offset));

	list->details->cell_lighter_background_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->cell_lighter_background, 
							    style_background_color, 1);

	list->details->cell_darker_background_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->cell_darker_background, 
							    style_background_color, 1.03);

	list->details->cell_selected_lighter_background_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->cell_selected_lighter_background, 
							    style_background_color, 1.04);

	list->details->cell_selected_darker_background_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->cell_selected_darker_background, 
							    style_background_color, 1.06);

	eel_gdk_set_shifted_foreground_gc_color (list->details->cell_divider_color, 
						      style_background_color, 1.1);

	list->details->selection_main_color_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->selection_main_color, 
							    selection_background_color, 1);
		
	list->details->selection_medium_color_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->selection_medium_color, 
							    selection_background_color, 0.7);

	list->details->selection_light_color_rgb
	    = eel_gdk_set_shifted_foreground_gc_color (list->details->selection_light_color, 
							    selection_background_color, 0.5);

	text_color = GTK_WIDGET (list)->style->fg[GTK_STATE_NORMAL];
	eel_gdk_gc_choose_foreground_color (list->details->text_color, &text_color,
						 &GTK_WIDGET (list)->style->bg[GTK_STATE_NORMAL]);

	text_color = GTK_WIDGET (list)->style->fg[GTK_STATE_SELECTED];
	eel_gdk_gc_choose_foreground_color (list->details->selected_text_color, &text_color,
						 &GTK_WIDGET (list)->style->bg[GTK_STATE_SELECTED]);

	text_color.red = 0;
	text_color.green = 0;
	text_color.blue = 65535;
	eel_gdk_gc_choose_foreground_color (list->details->link_text_color, &text_color,
						 &GTK_WIDGET (list)->style->bg[GTK_STATE_NORMAL]);
}

static void
unref_a_gc (GdkGC **gc)
{
	if (*gc != NULL) {
		gdk_gc_unref (*gc);
		*gc = NULL;
	}
}

static void
unref_gcs (EelList *list)
{
	g_return_if_fail (EEL_IS_LIST (list));

	unref_a_gc (&list->details->cell_lighter_background);
	unref_a_gc (&list->details->cell_darker_background);
	unref_a_gc (&list->details->cell_selected_lighter_background);
	unref_a_gc (&list->details->cell_selected_darker_background);
	unref_a_gc (&list->details->cell_divider_color);
	unref_a_gc (&list->details->selection_light_color);
	unref_a_gc (&list->details->selection_medium_color);
	unref_a_gc (&list->details->selection_main_color);
	unref_a_gc (&list->details->text_color);
	unref_a_gc (&list->details->selected_text_color);
	unref_a_gc (&list->details->link_text_color);
}

static void
make_gcs_and_colors (EelList *list)
{
	GtkWidget *widget;

	g_return_if_fail (EEL_IS_LIST (list));
	g_return_if_fail (GTK_IS_WIDGET (list));

	widget = GTK_WIDGET (list);

	/* First unref old gcs */
	unref_gcs (list);

	/* now setup new ones */
	list->details->cell_lighter_background = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_NORMAL], widget->window);
	list->details->cell_darker_background = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_NORMAL], widget->window);
	list->details->cell_selected_lighter_background = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_NORMAL], widget->window);
	list->details->cell_selected_darker_background = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_NORMAL], widget->window);
	list->details->cell_divider_color = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_NORMAL], widget->window);
	list->details->selection_light_color = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_SELECTED], widget->window);
	list->details->selection_medium_color = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_SELECTED], widget->window);
	list->details->selection_main_color = eel_gdk_gc_copy (
		widget->style->bg_gc[GTK_STATE_SELECTED], widget->window);

	list->details->text_color = eel_gdk_gc_copy (
		widget->style->fg_gc[GTK_STATE_NORMAL], widget->window);
	list->details->selected_text_color = eel_gdk_gc_copy (
		widget->style->fg_gc[GTK_STATE_SELECTED], widget->window);
	list->details->link_text_color = eel_gdk_gc_copy (
		widget->style->fg_gc[GTK_STATE_NORMAL], widget->window);

	eel_list_setup_style_colors (list);
}

static void
eel_list_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
	EelList *list;

	g_return_if_fail (EEL_IS_LIST (widget));

	list = EEL_LIST (widget);

	EEL_CALL_PARENT (GTK_WIDGET_CLASS, style_set, (widget, previous_style));

	if (GTK_WIDGET_REALIZED (widget)) {
		make_gcs_and_colors (list);
	}
}

static void
eel_list_realize (GtkWidget *widget)
{
	EelList *list;
	EelCList *clist;
	GtkWindow *window;

	g_return_if_fail (EEL_IS_LIST (widget));

	list = EEL_LIST (widget);
	clist = EEL_CLIST (widget);

	clist->column[0].button = list->details->title;

	EEL_CALL_PARENT (GTK_WIDGET_CLASS, realize, (widget));

	make_gcs_and_colors (list);

	if (list->details->title) {
		gtk_widget_set_parent_window (list->details->title, clist->title_window);
		gtk_widget_set_parent (list->details->title, GTK_WIDGET (clist));
		gtk_widget_show (list->details->title);
	}

	/* make us the focused widget */
        g_assert (GTK_IS_WINDOW (gtk_widget_get_toplevel (widget)));
        window = GTK_WINDOW (gtk_widget_get_toplevel (widget));
	gtk_window_set_focus (window, widget);
}

static void
eel_list_unrealize (GtkWidget *widget)
{
	GtkWindow *window;
        window = GTK_WINDOW (gtk_widget_get_toplevel (widget));
	gtk_window_set_focus (window, NULL);

	/* unref all the gcs we've created */
	unref_gcs (EEL_LIST (widget));

	EEL_CALL_PARENT (GTK_WIDGET_CLASS, unrealize, (widget));
}

/* this is here just temporarily */
static int
list_requisition_width (EelCList *clist) 
{
	int width = CELL_SPACING;
	int i;

	for (i = clist->columns - 1; i >= 0; i--) {
		if (!clist->column[i].visible)
			continue;

		if (clist->column[i].width_set)
			width += clist->column[i].width + CELL_SPACING + (2 * COLUMN_INSET);
		else if (EEL_CLIST_SHOW_TITLES(clist) && clist->column[i].button)
			width += clist->column[i].button->requisition.width;
	}

	return width;
}


static void
eel_list_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	/* stolen from gtk_clist 
	 * make sure the proper title ammount is allocated for the column
	 * title view --  this would not otherwise be done because 
	 * EelList depends the buttons being there when doing a size calculation
	 */
	EelList *list;
	EelCList *clist;

	g_return_if_fail (EEL_IS_LIST (widget));
	g_return_if_fail (requisition != NULL);

	clist = EEL_CLIST (widget);
	list = EEL_LIST (widget);

	requisition->width = 0;
	requisition->height = 0;

	/* compute the size of the column title (title) area */
	clist->column_title_area.height = 0;
	if (EEL_CLIST_SHOW_TITLES(clist) && list->details->title) {
		GtkRequisition child_requisition;
		
		gtk_widget_size_request (list->details->title,
					 &child_requisition);

		child_requisition.height = 20;
			/* for now */

		clist->column_title_area.height =
			MAX (clist->column_title_area.height,
			     child_requisition.height);
	}

	requisition->width += (widget->style->klass->xthickness +
			       GTK_CONTAINER (widget)->border_width) * 2;
	requisition->height += (clist->column_title_area.height +
				(widget->style->klass->ythickness +
				GTK_CONTAINER (widget)->border_width) * 2);


	requisition->width += list_requisition_width (clist);
	requisition->height += LIST_HEIGHT (clist);
}

static int
new_column_width (EelCList *clist, int column_index,  int *x)
{
	int xthickness = GTK_WIDGET (clist)->style->klass->xthickness;
	int width;
	int cx;
	int dx;
	int last_column;

	/* first translate the x position from widget->window
	 * to clist->clist_window */
	cx = *x - xthickness;

	for (last_column = clist->columns - 1;
		last_column >= 0 && !clist->column[last_column].visible; last_column--);

	/* calculate new column width making sure it doesn't end up
	 * less than the minimum width */
	dx = (COLUMN_LEFT_XPIXEL (clist, column_index) + COLUMN_INSET +
		(column_index < last_column) * CELL_SPACING);
	width = cx - dx;

	if (width < MAX (COLUMN_MIN_WIDTH, clist->column[column_index].min_width)) {
		width = MAX (COLUMN_MIN_WIDTH, clist->column[column_index].min_width);
		cx = dx + width;
		*x = cx + xthickness;
	} else if (clist->column[column_index].max_width >= COLUMN_MIN_WIDTH &&
	   width > clist->column[column_index].max_width) {
		width = clist->column[column_index].max_width;
		cx = dx + clist->column[column_index].max_width;
		*x = cx + xthickness;
    	}

	if (cx < 0 || cx > clist->clist_window_width)
		*x = -1;

	return width;
}

static void
size_allocate_columns (EelCList *clist, gboolean  block_resize)
{
	int xoffset = CELL_SPACING + COLUMN_INSET;
	int last_column;
	int i;

	/* find last visible column and calculate correct column width */
	for (last_column = clist->columns - 1;
	     last_column >= 0 && !clist->column[last_column].visible; last_column--)
		;

	if (last_column < 0)
		return;

	for (i = 0; i <= last_column; i++)  {
		if (!clist->column[i].visible)
			continue;

		clist->column[i].area.x = xoffset;
		if (clist->column[i].width_set) {
			if (!block_resize && EEL_CLIST_SHOW_TITLES(clist) &&
				clist->column[i].auto_resize && clist->column[i].button) {
				int width;

				width = (clist->column[i].button->requisition.width -
					(CELL_SPACING + (2 * COLUMN_INSET)));

				if (width > clist->column[i].width)
					eel_clist_set_column_width (clist, i, width);
			}

			clist->column[i].area.width = clist->column[i].width;
			xoffset += clist->column[i].width + CELL_SPACING + (2 * COLUMN_INSET);
		} else if (EEL_CLIST_SHOW_TITLES(clist) && clist->column[i].button) {
			clist->column[i].area.width =
				clist->column[i].button->requisition.width -
				(CELL_SPACING + (2 * COLUMN_INSET));
			xoffset += clist->column[i].button->requisition.width;
		}
	}

	clist->column[last_column].area.width 
		+= MAX (0, clist->clist_window_width + COLUMN_INSET - xoffset);
}

static void
size_allocate_title_buttons (EelCList *clist)
{
	GtkAllocation button_allocation;
	int last_column;
	int last_button = 0;
	int i;

	button_allocation.x = clist->hoffset;
	button_allocation.y = 0;
	button_allocation.width = 0;
	button_allocation.height = clist->column_title_area.height;

	/* find last visible column */
	for (last_column = clist->columns - 1; last_column >= 0; last_column--)
		if (clist->column[last_column].visible)
			break;

	for (i = 0; i < last_column; i++) {
		if (!clist->column[i].visible) {
			last_button = i + 1;
			gdk_window_hide (clist->column[i].window);
			continue;
		}

		button_allocation.width += (clist->column[i].area.width +
				  	    CELL_SPACING + 2 * COLUMN_INSET);

		if (!clist->column[i + 1].button) {
			gdk_window_hide (clist->column[i].window);
			continue;
		}

		gtk_widget_size_allocate (clist->column[last_button].button,
					  &button_allocation); 
		button_allocation.x += button_allocation.width;
		button_allocation.width = 0;

		last_button = i + 1;
	}

	button_allocation.width += (clist->column[last_column].area.width +
				    2 * (CELL_SPACING + COLUMN_INSET));
	gtk_widget_size_allocate (clist->column[last_button].button,
				  &button_allocation);

}

static void
eel_list_draw_focus (GtkWidget *widget)
{
	GdkGCValues saved_values;
	EelCList *clist;

	g_return_if_fail (EEL_IS_LIST (widget));

	if (!GTK_WIDGET_DRAWABLE (widget) || !GTK_WIDGET_CAN_FOCUS (widget)) {
  		return;
  	}

	clist = EEL_CLIST (widget);
	if (clist->focus_row < 0) {
		return;
	}

  	gdk_gc_get_values (clist->xor_gc, &saved_values);

  	gdk_gc_set_stipple (clist->xor_gc, eel_stipple_bitmap ());
  	gdk_gc_set_fill (clist->xor_gc, GDK_STIPPLED);

    	gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
		0, ROW_TOP_YPIXEL(clist, clist->focus_row),
		clist->clist_window_width - 1,
		clist->row_height - 1);
	/* Resetting the stipple to the saved value causes death
	 * deep in Bonobo X handling, believe it or not. Fortunately
	 * we don't need to.
	 */
  	gdk_gc_set_fill (clist->xor_gc, saved_values.fill);
}

static int
selected_column_index (EelList *list)
{
	int column;

	column = 2;
	gtk_signal_emit_by_name (GTK_OBJECT (list), "get_sort_column_index", &column);
	return column;
}

static void
get_column_background (EelList *list, GdkGC **selected, GdkGC **plain)
{
	*plain = list->details->cell_lighter_background;
	*selected = list->details->cell_selected_lighter_background;
}

void
eel_list_get_cell_style (EelList *list, EelCListRow *row,
			      int state, int row_index, int column_index, GtkStyle **style,
			      GdkGC **fg_gc, GdkGC **bg_gc, guint32 *bg_rgb)
{
	gboolean lighter_row;

	lighter_row = (list->details->alternate_row_colors
		       ? (row_index % 2) != 0 : TRUE);

	if (style) {
		*style = GTK_WIDGET (list)->style;
	}

	if (state == GTK_STATE_SELECTED) {
		if (fg_gc != NULL) {
			*fg_gc = GTK_WIDGET (list)->style->fg_gc[state];
		}
		if (bg_gc != NULL) {
			if (column_index == selected_column_index (list)) {
				*bg_gc = list->details->selection_medium_color;
			} else  {
				*bg_gc = list->details->selection_light_color;
			}
		}
		if (bg_rgb != NULL) {
			if (column_index == selected_column_index (list)) {
				*bg_rgb = list->details->selection_medium_color_rgb;
			} else  {
				*bg_rgb = list->details->selection_light_color_rgb;
			}
		}


		return;
	}

	if (fg_gc != NULL) {
		*fg_gc = GTK_WIDGET (list)->style->fg_gc[state];
	}

	if (bg_gc != NULL) {
		if (column_index == selected_column_index (list)) {
			if (lighter_row) {
				*bg_gc = list->details->cell_selected_lighter_background;
			} else {
				*bg_gc = list->details->cell_selected_darker_background;
			}
		} else {
			if (lighter_row) {
				*bg_gc = list->details->cell_lighter_background;
			} else {
				*bg_gc = list->details->cell_darker_background;
			}
		}
	}
	if (bg_rgb != NULL) {
		if (column_index == selected_column_index (list)) {
			if (lighter_row) {
				*bg_rgb = list->details->cell_selected_lighter_background_rgb;
			} else {
				*bg_rgb = list->details->cell_selected_darker_background_rgb;
			}
		} else {
			if (lighter_row) {
				*bg_rgb = list->details->cell_lighter_background_rgb;
			} else {
				*bg_rgb = list->details->cell_darker_background_rgb;
			}
		}
	}
}

void
eel_list_set_alternate_row_colors (EelList *list,
					gboolean state)
{
	list->details->alternate_row_colors = state;
}

void
eel_list_set_background_color_offsets (EelList *list,
					    long background_offset,
					    long selection_offset)
{
	g_return_if_fail (background_offset < 0
			  || (gulong) background_offset < sizeof (GtkStyle));
	g_return_if_fail (selection_offset < 0
			  || (gulong) selection_offset < sizeof (GtkStyle));

	if (background_offset >= 0) {
		list->details->background_color_offset = background_offset;
	}
	if (selection_offset >= 0) {
		list->details->selection_color_offset = selection_offset;
	}
}

int
eel_list_draw_cell_pixbuf (EelList *list, GdkWindow *window,
				GdkRectangle *clip_rectangle,
				GdkGC *fg_gc, guint32 bg_rgb,
				GdkPixbuf *pixbuf, int x, int y)
{
	GdkRectangle image_rectangle;
	GdkRectangle intersect_rectangle;
	GdkPixbuf *composited;

	image_rectangle.width = gdk_pixbuf_get_width (pixbuf);
	image_rectangle.height = gdk_pixbuf_get_height (pixbuf);
	image_rectangle.x = x;
	image_rectangle.y = y;

	if (!gdk_rectangle_intersect (clip_rectangle, &image_rectangle, &intersect_rectangle)) {
		return x;
	}

	if (list && eel_list_is_anti_aliased (list)) {
		/* Composite a version of the pixbuf with the background color */
		composited = gdk_pixbuf_composite_color_simple (pixbuf,
								image_rectangle.width,
								image_rectangle.height,
								GDK_INTERP_BILINEAR,
								255, 64,
								bg_rgb, bg_rgb);
		if (composited == NULL) {
			return x;
		}

		gdk_pixbuf_render_to_drawable (composited, window, fg_gc,
					       intersect_rectangle.x - x,
					       intersect_rectangle.y - y, 
					       image_rectangle.x, image_rectangle.y, 
					       intersect_rectangle.width,
					       intersect_rectangle.height,
					       GDK_RGB_DITHER_MAX, 0, 0);

		gdk_pixbuf_unref (composited);

	} else {

		gdk_pixbuf_render_to_drawable_alpha (pixbuf, window,
						     intersect_rectangle.x - x,
						     intersect_rectangle.y - y, 
						     image_rectangle.x, image_rectangle.y, 
						     intersect_rectangle.width,
						     intersect_rectangle.height,
						     GDK_PIXBUF_ALPHA_BILEVEL,
						     EEL_STANDARD_ALPHA_THRESHHOLD,
						     GDK_RGB_DITHER_MAX, 0, 0);
	}

	return x + intersect_rectangle.width;
}

/**
 * get_cell_horizontal_start_position:
 * 
 * Get the leftmost x value at which the contents of this cell are painted.
 * 
 * @clist: The list in question.
 * @row: The row data structure for the target cell.
 * @column_index: The column of the target cell.
 * @content_width: The already-computed width of the cell contents.
 * 
 * Return value: x value at which the contents of this cell are painted.
 */
static int
get_cell_horizontal_start_position (EelCList *clist, EelCListRow *row, int column_index, int content_width)
{
	int initial_offset;

	initial_offset = clist->column[column_index].area.x + 
			 clist->hoffset + 
			 row->cell[column_index].horizontal;
	
	switch (clist->column[column_index].justification) {
		case GTK_JUSTIFY_LEFT:
			return initial_offset;
		case GTK_JUSTIFY_RIGHT:
			return initial_offset + clist->column[column_index].area.width - content_width;
		case GTK_JUSTIFY_CENTER:
		case GTK_JUSTIFY_FILL:
		default:
			return initial_offset + (clist->column[column_index].area.width - content_width)/2;
	}
} 

static int
last_column_index (EelCList *clist)
{
	int result;
	for (result = clist->columns - 1;
		result >= 0 && !clist->column[result].visible; 
		result--) {
	}

	return result;
}

void
eel_list_get_cell_rectangle (EelList *list, int row_index, int column_index, GdkRectangle *result)
{
	EelCList *clist;
	
	g_return_if_fail (EEL_IS_LIST (list));

	clist = EEL_CLIST (list);
	result->x = clist->column[column_index].area.x + clist->hoffset;
	result->y = ROW_TOP_YPIXEL (clist, row_index);
	result->width = clist->column[column_index].area.width;
	result->height = clist->row_height;
}

static void
get_cell_greater_rectangle (GdkRectangle *cell_rect, GdkRectangle *result, 
	gboolean last_column)
{
	*result = *cell_rect;
	result->x -= COLUMN_INSET + CELL_SPACING;
	result->width += 2 * COLUMN_INSET + CELL_SPACING;
	if (last_column) {
		result->width += CELL_SPACING;
	}
}

static void
draw_cell (EelCList *clist, GdkRectangle *area, int row_index, int column_index, 
	   EelCListRow *row)
{
	GtkStyle *style;
	GdkGC *fg_gc;
	GdkGC *bg_gc;
	GdkGC *text_gc;
	guint32 bg_rgb;

	GList *p;

	int width;
	int height;
	int pixbuf_width;
	int offset = 0;
	int baseline;
	int row_center_offset;
	char *text;

	GdkRectangle cell_rectangle;
	GdkRectangle erase_rectangle;
	GdkRectangle intersect_rectangle;
	
	if (!clist->column[column_index].visible) {
		return;
	}

	eel_list_get_cell_style (EEL_LIST(clist), row, row->state, row_index, 
				      column_index, &style, &fg_gc, &bg_gc, &bg_rgb);
	eel_list_get_cell_rectangle (EEL_LIST (clist), row_index, column_index, &cell_rectangle);
	get_cell_greater_rectangle (&cell_rectangle, &erase_rectangle, 
		column_index == last_column_index (clist));

	/* do we have anything do draw? */
	if (area && !gdk_rectangle_intersect (area, &erase_rectangle, &intersect_rectangle)) {
		return;
	}

	gdk_draw_rectangle (clist->clist_window, bg_gc, TRUE,
		  erase_rectangle.x, erase_rectangle.y, 
		  erase_rectangle.width, erase_rectangle.height);

	/* calculate real width for column justification */
	width = 0;
	height = 0;
	text = NULL;
	
	switch ((EelCellType) row->cell[column_index].type) {
	case EEL_CELL_TEXT:
	case EEL_CELL_LINK_TEXT:
		text = get_cell_text (clist, column_index, clist->column[column_index].area.width,
			row, style->font);
		if (text != NULL) {
			width = gdk_string_width (style->font, text);
		}
		break;
	case EEL_CELL_PIXBUF:
		width = gdk_pixbuf_get_width (EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf);
		height = gdk_pixbuf_get_height (EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf);
		break;
	case EEL_CELL_PIXTEXT:
		pixbuf_width = gdk_pixbuf_get_width (EEL_CELL_PIXTEXT (row->cell[column_index])->pixbuf);
		height = gdk_pixbuf_get_height (EEL_CELL_PIXTEXT (row->cell[column_index])->pixbuf);
		text = get_cell_text (clist, column_index,
			clist->column[column_index].area.width - pixbuf_width
				- EEL_CELL_PIXTEXT (row->cell[column_index])->spacing,
			row, style->font);
		width = pixbuf_width +
 			EEL_CELL_PIXTEXT (row->cell[column_index])->spacing +
			text == NULL ? 0 : gdk_string_width (style->font, text);
		break;
	case EEL_CELL_PIXBUF_LIST:
		for (p = EEL_CELL_PIXBUF_LIST (row->cell[column_index])->pixbufs; 
			p != NULL; p = p->next) {
			if (width != 0) {
				width += PIXBUF_LIST_SPACING;
			}
			width += gdk_pixbuf_get_width (p->data);
		}
		break;
	case EEL_CELL_EMPTY:
	case EEL_CELL_WIDGET:
		return;
	}

	offset = get_cell_horizontal_start_position (clist, row, column_index, width);

	/* Draw Text and/or Pixbuf */
	switch ((EelCellType) row->cell[column_index].type) {
	case EEL_CELL_PIXBUF: {
		EelList *list = EEL_LIST (clist);
		GdkPixbuf *src_pixbuf, *dark_pixbuf;
		
		if (list->details->drag_prelight_row == row) {
			
			src_pixbuf = EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf;
			
			if (src_pixbuf != NULL) {
				/* Create darkened pixbuf */
				dark_pixbuf = eel_create_darkened_pixbuf (src_pixbuf,
									       0.8 * 255,
									       0.8 * 255);
				if (dark_pixbuf != NULL) {
					eel_list_draw_cell_pixbuf (EEL_LIST (clist), clist->clist_window, &cell_rectangle, fg_gc, bg_rgb,
									dark_pixbuf, offset,
									cell_rectangle.y + row->cell[column_index].vertical +
									(cell_rectangle.height - height) / 2);
					
					gdk_pixbuf_unref (dark_pixbuf);
				}
			}					
		} else {		
			eel_list_draw_cell_pixbuf (EEL_LIST (clist), clist->clist_window, &cell_rectangle, fg_gc, bg_rgb,
							EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf,
							offset,
							cell_rectangle.y + row->cell[column_index].vertical +
							(cell_rectangle.height - height) / 2);
		}
		break;
	}

	case EEL_CELL_PIXTEXT:
		offset = eel_list_draw_cell_pixbuf (EEL_LIST (clist), clist->clist_window, &cell_rectangle, fg_gc, bg_rgb,
							 EEL_CELL_PIXTEXT (row->cell[column_index])->pixbuf,
							 offset,
							 cell_rectangle.y + row->cell[column_index].vertical+
							 (cell_rectangle.height - height) / 2);
		offset += EEL_CELL_PIXTEXT (row->cell[column_index])->spacing;
		/* fall through */
	case EEL_CELL_TEXT:
	case EEL_CELL_LINK_TEXT:
		if (text == NULL) {
			break;
		}
		
		if (style != GTK_WIDGET (clist)->style) {
			row_center_offset = (((clist->row_height - style->font->ascent -
				style->font->descent - 1) / 2) + 1.5 +
				style->font->ascent);
		} else {
			row_center_offset = clist->row_center_offset;
		}
		baseline = cell_rectangle.y + row_center_offset + row->cell[column_index].vertical;

		if (row->state != GTK_STATE_NORMAL) {
			text_gc = EEL_LIST (clist)->details->selected_text_color;
		} else if ((EelCellType)row->cell[column_index].type == EEL_CELL_LINK_TEXT
			   && EEL_LIST (clist)->details->single_click_mode) {
			/* For link text cells, draw with blue link-like color and use underline. */
			text_gc = EEL_LIST (clist)->details->link_text_color;
		} else {
			text_gc = EEL_LIST (clist)->details->text_color;
		}

		gdk_gc_set_clip_rectangle (text_gc, &cell_rectangle);

		gdk_draw_string (clist->clist_window, style->font, text_gc,
				 offset,
				 baseline,
				 text);

		if ((EelCellType)row->cell[column_index].type == EEL_CELL_LINK_TEXT
			&& EEL_LIST (clist)->details->single_click_mode) {
			gdk_draw_line (clist->clist_window, text_gc,
				       offset, baseline + 1,
				       offset + width, baseline + 1);
		}

		gdk_gc_set_clip_rectangle (text_gc, NULL);
		break;
			
	case EEL_CELL_PIXBUF_LIST: {
		guint pixbuf_width;
		guint ellipsis_width;

		ellipsis_width = gdk_string_width (style->font, "...");
	  
		for (p = EEL_CELL_PIXBUF_LIST (row->cell[column_index])->pixbufs; p != NULL; p = p->next) {
			pixbuf_width = gdk_pixbuf_get_width (p->data);

			if ((p->next != NULL && (int) (pixbuf_width + ellipsis_width) >= 
		  		cell_rectangle.x + cell_rectangle.width - offset) 
		  			|| ((int) pixbuf_width >= cell_rectangle.x + cell_rectangle.width - offset)) {
				/* Not enough room for this icon & ellipsis, just draw ellipsis. */
			
				gdk_draw_string (clist->clist_window, style->font, fg_gc,
						 offset,
						 cell_rectangle.y + cell_rectangle.height/2,
						 "...");

				break;
			}

			height = gdk_pixbuf_get_height (p->data);

	  		offset = eel_list_draw_cell_pixbuf (EEL_LIST (clist), clist->clist_window,
								 &cell_rectangle,
								 fg_gc, bg_rgb, p->data,
								 offset,
								 cell_rectangle.y + row->cell[column_index].vertical +
								 (cell_rectangle.height - height) / 2);

			offset += PIXBUF_LIST_SPACING;
		}
		break;
	}
	case EEL_CELL_EMPTY:
	case EEL_CELL_WIDGET:
		break;
	}
	
	g_free (text);
}

static void
draw_row (EelCList *clist, GdkRectangle *area, int row_index, EelCListRow *row)
{
	GtkWidget *widget;
	GdkRectangle row_rectangle;
	GdkRectangle extended_row_rectangle;
	GdkRectangle intersect_rectangle;
	int colum_index;

	g_return_if_fail (clist != NULL);

	/* bail now if we arn't drawable yet */
	if (!GTK_WIDGET_DRAWABLE (clist) || row_index < 0 || row_index >= clist->rows) {
		return;
	}

	widget = GTK_WIDGET (clist);

	/* if the function is passed the pointer to the row instead of null,
	 * it avoids this expensive lookup 
	 */
	if (!row) {
		row = ROW_ELEMENT (clist, row_index)->data;
	}

	/* rectangle of the entire row */
	row_rectangle.x = 0;
	row_rectangle.y = ROW_TOP_YPIXEL (clist, row_index);
	row_rectangle.width = clist->clist_window_width;
	row_rectangle.height = clist->row_height;

	/* rectangle of the entire row including spacing above and below the row */
	extended_row_rectangle.x = 0;
	extended_row_rectangle.y = row_rectangle.y - CELL_SPACING;
	extended_row_rectangle.width = row_rectangle.width;
	extended_row_rectangle.height = row_rectangle.height + CELL_SPACING;

	if (row->state == GTK_STATE_NORMAL) {
		if (row->fg_set) {
			gdk_gc_set_foreground (clist->fg_gc, &row->foreground);
		}
		if (row->bg_set) {
			gdk_gc_set_foreground (clist->bg_gc, &row->background);
		}
	}

	intersect_rectangle = extended_row_rectangle;
	/* check if we have something to draw */
	if (area && !gdk_rectangle_intersect (area, &extended_row_rectangle, &intersect_rectangle)) {
		return;
	}


	/* iterate and draw all the columns (row cells) and draw their contents */
	for (colum_index = 0; colum_index < clist->columns; colum_index++) {
		draw_cell (clist, area, row_index, colum_index, row);
	}

	/* draw the row spacer */
	gdk_draw_rectangle (clist->clist_window,
			    EEL_LIST (clist)->details->cell_divider_color,
			    TRUE,
			    intersect_rectangle.x,
			    extended_row_rectangle.y,
			    intersect_rectangle.width,
			    CELL_SPACING);
	gdk_draw_rectangle (clist->clist_window,
			    EEL_LIST (clist)->details->cell_divider_color,
			    TRUE,
			    intersect_rectangle.x,
			    row_rectangle.y + row_rectangle.height,
			    intersect_rectangle.width,
			    CELL_SPACING);

	/* draw focus rectangle */
	if (clist->focus_row == row_index 
		&& GTK_WIDGET_CAN_FOCUS (widget) && GTK_WIDGET_HAS_FOCUS (widget)) {
		if (!area) {
			gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
					    row_rectangle.x, row_rectangle.y,
					    row_rectangle.width - 1, row_rectangle.height - 1);
		} else if (gdk_rectangle_intersect (area, &row_rectangle, &intersect_rectangle)) {
			gdk_gc_set_clip_rectangle (clist->xor_gc, &intersect_rectangle);
			gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
					      row_rectangle.x, row_rectangle.y,
					      row_rectangle.width - 1,
					      row_rectangle.height - 1);
			gdk_gc_set_clip_rectangle (clist->xor_gc, NULL);
		}
	}
}

static void
rectangle_intersect (const GdkRectangle *source1, const GdkRectangle *source2, GdkRectangle *result)
{
	/* convenience call that returns result with defined values even if sources are disjucnt */
	if (!gdk_rectangle_intersect ((GdkRectangle *)source1, (GdkRectangle *)source2, result)) {
		result->width = 0;
		result->height = 0;
	}
}

static void
eel_list_clear_from_row (EelList *list, int row_index,
	GdkRectangle *area)
{
	EelCList *clist;
	GdkRectangle clip_area, tmp;
	GdkRectangle first_column_plain_rectangle, selected_column_rectangle, 
		second_column_plain_rectangle;
	GdkGC *selected_column_gc;
	GdkGC *plain_column_gc;

	g_assert (EEL_IS_LIST (list));
	g_assert (area);

	clist = EEL_CLIST (list);

	/* calculate the area we need to erase */
	clip_area = *area;
	clip_area.y = ROW_TOP_YPIXEL (clist, row_index);
	if (clip_area.y < 0) {
		clip_area.y = 0;
	}
	if (clip_area.y - area->y < area->height) {
		clip_area.height = area->height - (clip_area.y - area->y); 
	} else {
		clip_area.height = 0;
	}
	
	if (clip_area.height <= 0) {
		/* nothing visible to erase */
		return;
	}

	/* calculate the rectangle for the selected column */
	eel_list_get_cell_rectangle (list, 0, selected_column_index (list), &tmp);
	get_cell_greater_rectangle (&tmp, &tmp, 
		selected_column_index (list) == last_column_index (clist));
	tmp.y = clip_area.y;
	tmp.height = clip_area.height;

	rectangle_intersect (&clip_area, &tmp, &selected_column_rectangle);

	/* calculate the first rectangle */
	tmp = clip_area;
	if (selected_column_rectangle.x > tmp.x) {
		tmp.width = selected_column_rectangle.x - tmp.x;
	} else {
		tmp.width = selected_column_rectangle.x - tmp.x;
	}
	rectangle_intersect (&clip_area, &tmp, &first_column_plain_rectangle);


	/* calculate the last rectangle */
	tmp = clip_area;
	tmp.x = selected_column_rectangle.x + selected_column_rectangle.width;
	rectangle_intersect (&clip_area, &tmp, &second_column_plain_rectangle);

	/* get the colors for drawing */
	get_column_background (list, &selected_column_gc, &plain_column_gc);

	/* draw the first column if non-empty */
	if (first_column_plain_rectangle.width > 0) {
		gdk_draw_rectangle (clist->clist_window, plain_column_gc, TRUE,
			  first_column_plain_rectangle.x, first_column_plain_rectangle.y, 
			  first_column_plain_rectangle.width, first_column_plain_rectangle.height);
	}
	/* draw the selected column if non-empty */
	if (selected_column_rectangle.width > 0) {
		gdk_draw_rectangle (clist->clist_window, selected_column_gc, TRUE,
			  selected_column_rectangle.x, selected_column_rectangle.y, 
			  selected_column_rectangle.width, selected_column_rectangle.height);
	}
	/* draw the last column if non-empty */
	if (second_column_plain_rectangle.width > 0) {
		gdk_draw_rectangle (clist->clist_window, plain_column_gc, TRUE,
			  second_column_plain_rectangle.x, second_column_plain_rectangle.y, 
			  second_column_plain_rectangle.width, second_column_plain_rectangle.height);
	}
}

static void
draw_rows (EelCList *clist, GdkRectangle *area)
{
	GList *list;
	int row_index;
	int first_row;
	int last_row;

	g_assert (area != NULL);
	
	if (clist->row_height == 0 || !GTK_WIDGET_DRAWABLE (clist)) {
		return;
	}

	first_row = ROW_FROM_YPIXEL (clist, area->y);
	last_row = ROW_FROM_YPIXEL (clist, area->y + area->height);

	/* this is a small special case which exposes the bottom cell line
	 * on the last row -- it might go away if I change the wall the cell
	 * spacings are drawn
	 */
	if (clist->rows == first_row) {
		first_row--;
	}

	list = ROW_ELEMENT (clist, first_row);
	for (row_index = first_row; row_index <= last_row ; row_index++) {
		if (list == NULL) {
			break;
		}

		EEL_CALL_METHOD (EEL_CLIST_CLASS, clist, 
				      draw_row, (clist, area, row_index, list->data));
		list = list->next;
	}

	eel_list_clear_from_row (EEL_LIST (clist), 
		row_index, area);
}

static void
draw_all (EelCList *clist)
{
	GdkRectangle area;
	area.x = 0;
	area.y = 0;
	area.width = clist->clist_window_width;
	area.height = clist->clist_window_height;
	EEL_CALL_METHOD (EEL_CLIST_CLASS, clist, draw_rows, (clist, &area));
}

static void
eel_list_draw (GtkWidget *widget, GdkRectangle *area)
{
	EelCList *clist;
	EelList *list;
	
	g_assert (EEL_IS_LIST (widget));
	g_assert (area != NULL);

	clist = EEL_CLIST (widget);
	list = EEL_LIST (widget);

	eel_list_setup_style_colors (EEL_LIST (widget));

	if (GTK_WIDGET_DRAWABLE (widget)) {
		int border_width;
		border_width = GTK_CONTAINER (widget)->border_width;
		gdk_window_clear_area (widget->window,
				area->x - border_width, 
				area->y - border_width,
				area->width, area->height);

		gtk_draw_shadow (widget->style, widget->window,
				GTK_STATE_NORMAL, clist->shadow_type,
				0, 0, 
				clist->clist_window_width +
					(2 * widget->style->klass->xthickness),
				clist->clist_window_height +
					(2 * widget->style->klass->ythickness) +
				clist->column_title_area.height);

		EEL_CALL_METHOD (EEL_CLIST_CLASS, clist, draw_rows, (clist, area));

		/* Draw the title if it exists */
		if (list->details->title) {
			GdkRectangle draw_area;

			if (gtk_widget_intersect (list->details->title,
						  area, &draw_area)) {
				gtk_widget_draw (list->details->title, &draw_area);
			}
		}
	}
}

static int
eel_list_expose (GtkWidget *widget, GdkEventExpose *event)
{
	EelCList *clist;
	
	g_assert (EEL_IS_LIST (widget));

	clist = EEL_CLIST (widget);

	eel_list_setup_style_colors (EEL_LIST (widget));

	if (GTK_WIDGET_DRAWABLE (widget)) {

		gtk_draw_shadow (widget->style, widget->window,
				GTK_STATE_NORMAL, clist->shadow_type,
				0, 0, 
				clist->clist_window_width +
					(2 * widget->style->klass->xthickness),
				clist->clist_window_height +
					(2 * widget->style->klass->ythickness) +
				clist->column_title_area.height);

		EEL_CALL_METHOD (EEL_CLIST_CLASS, clist, draw_rows, 
				      (clist, &event->area));
	}

	return FALSE;
}

static void 
eel_list_resize_column (EelCList *clist, int column_index, int width)
{
	/* override resize column to invalidate the title */
	EelList *list;

	list = EEL_LIST (clist);

	gtk_widget_queue_draw (list->details->title);
		
	EEL_CALL_PARENT (EEL_CLIST_CLASS, resize_column, (clist, column_index, width));
}


/**
 * eel_list_mark_cell_as_link:
 * 
 * Mark a text cell as a link cell. Link cells are drawn differently,
 * and activate rather than select on single-click. The cell must
 * be a text cell (not a pixbuf cell or one of the other types).
 * 
 * @list: The EelList in question.
 * @column_index: The column of the desired cell.
 * @row: The row of the desired cell.
 */
void
eel_list_mark_cell_as_link (EelList *list,
				 int row_index,
				 int column_index)
{
	EelCListRow *row;
	EelCList *clist;

	g_return_if_fail (EEL_IS_LIST (list));

	clist = EEL_CLIST (list);

	g_return_if_fail (row_index >= 0 && row_index < clist->rows);
	g_return_if_fail (column_index >= 0 && column_index < clist->columns);
	
	row = ROW_ELEMENT (clist, row_index)->data;

	/* 
	 * We only support changing text cells to links. Maybe someday
	 * we'll support pixbuf or pixtext link cells too. 
	 */
	g_return_if_fail ((EelCellType)row->cell[column_index].type == EEL_CELL_TEXT);

	row->cell[column_index].type = EEL_CELL_LINK_TEXT;
}				


static gboolean
eel_list_set_cell_contents (EelCList    *clist,
		   		 EelCListRow *row,
		   		 int         column_index,
		   		 EelCellType  type,
		   		 const gchar *text,
		   		 guint8       spacing,
		   		 GdkPixbuf   *pixbuf)
{
	gboolean result;

	/* 
	 * Note that we don't do the auto_resize bracketing here that's done
	 * in the parent class. It would require copying over huge additional
	 * chunks of code. We might decide we need that someday, but the
	 * chances seem larger that we'll switch away from CList first.
	 */

	/* Clean up old data, which parent class doesn't know about. */
	if ((EelCellType)row->cell[column_index].type == EEL_CELL_PIXBUF_LIST) {
		eel_gdk_pixbuf_list_free (EEL_CELL_PIXBUF_LIST (row->cell[column_index])->pixbufs);
	}

	result = EEL_CALL_PARENT_WITH_RETURN_VALUE
		(EEL_CLIST_CLASS, set_cell_contents,
		 (clist, row, column_index, type, text, spacing, pixbuf));

	if ((EelCellType)type == EEL_CELL_PIXBUF_LIST) {
		row->cell[column_index].type = EEL_CELL_PIXBUF_LIST;
		/* Hideously, we concealed our list of pixbufs in the pixbuf parameter. */
	  	EEL_CELL_PIXBUF_LIST (row->cell[column_index])->pixbufs = (GList *)pixbuf;
	}

	return result;
}

static void
set_list_cell (EelList *list,
	       int row_index, int column_index,
	       EelCellType type,
	       gpointer data)
{
	EelCList    *clist;
	EelCListRow *row;

	g_return_if_fail (EEL_IS_LIST (list));

	clist = EEL_CLIST (list);

	if (row_index < 0 || row_index >= clist->rows) {
		return;
	}
	
	if (column_index < 0 || column_index >= clist->columns) {
		return;
	}

	row = ROW_ELEMENT (clist, row_index)->data;

	/*
	 * We have to go through the set_cell_contents bottleneck, which only
	 * allows expected parameter types. Since our pixbuf is not an
	 * expected parameter type, we have to sneak it in by casting it into
	 * one of the expected parameters.
	 */
	if (EEL_CALL_METHOD_WITH_RETURN_VALUE
	    (EEL_CLIST_CLASS, clist, set_cell_contents, 
	     (clist, row, column_index, type, NULL, 0, data))) {
		/* redraw the list if it's not frozen */
		if (CLIST_UNFROZEN (clist) 
		    && eel_clist_row_is_visible (clist, row_index) != GTK_VISIBILITY_NONE) {
			EEL_CALL_METHOD (EEL_CLIST_CLASS, clist, draw_row, 
					      (clist, NULL, row_index, row));
		}
	}
}

static gpointer
get_list_cell (EelList *list,
	       int row_index, int column_index,
	       EelCellType type)
{
	EelCList    *clist;
	EelCListRow *row;

	g_return_val_if_fail (EEL_IS_LIST (list), NULL);

	clist = EEL_CLIST (list);

	if (row_index < 0 || row_index >= clist->rows) {
		return NULL;
	}
	
	if (column_index < 0 || column_index >= clist->columns) {
		return NULL;
	}

	row = ROW_ELEMENT (clist, row_index)->data;

	if (row->cell[column_index].type == type) {
		return EEL_CELL_PIXBUF (row->cell[column_index])->pixbuf;
	}

	return NULL;
}

/**
 * eel_list_set_pixbuf_list:
 * 
 * Set the contents of a cell to a list of similarly-sized GdkPixbufs.
 * 
 * @list: The EelList in question.
 * @row: The row of the target cell.
 * @column_index: The column of the target cell.
 * @pixbufs: A GList of GdkPixbufs.
 */
void 	   
eel_list_set_pixbuf_list (EelList *list,
			       int row_index,
			       int column_index,
			       GList *pixbufs)
{
	set_list_cell (list, row_index, column_index,
		       EEL_CELL_PIXBUF_LIST, pixbufs);
}

/**
 * eel_list_set_pixbuf:
 * 
 * Similar to eel_list_set_pixbuf_list, but with a single pixbuf.
 * 
 * @list: The EelList in question.
 * @row: The row of the target cell.
 * @column_index: The column of the target cell.
 * @pixbuf: A GdkPixbuf.
 */
void 	   
eel_list_set_pixbuf (EelList *list,
			  int row_index,
			  int column_index,
			  GdkPixbuf *pixbuf)
{
	set_list_cell (list, row_index, column_index,
		       EEL_CELL_PIXBUF, pixbuf);
}

/**
 * eel_list_get_pixbuf:
 * 
 * Return the pixbuf stored in the specified position, or a null pointer
 * if the cell isn't a pixbuf.
 * 
 * @list: The EelList in question.
 * @row: The row of the target cell.
 * @column_index: The column of the target cell.
 */
GdkPixbuf *
eel_list_get_pixbuf (EelList *list,
			  int row_index,
			  int column_index)
{
	return get_list_cell (list, row_index, column_index,
			      EEL_CELL_PIXBUF);
}

static void
eel_list_track_new_column_width (EelCList *clist, int column_index, int new_width)
{
	EelList *list;
	GdkRectangle area;

	list = EEL_LIST (clist);

	/* pin new_width to min and max values */
	if (new_width < MAX (COLUMN_MIN_WIDTH, clist->column[column_index].min_width))
		new_width = MAX (COLUMN_MIN_WIDTH, clist->column[column_index].min_width);
	if (clist->column[column_index].max_width >= 0 &&
	    new_width > clist->column[column_index].max_width)
		new_width = clist->column[column_index].max_width;

	/* check to see if the pinned value is still different */
	if (clist->column[column_index].width == new_width)
		return;

	/* set the new width */
	clist->column[column_index].width = new_width;
	clist->column[column_index].width_set = TRUE;

	size_allocate_columns (clist, TRUE);
	size_allocate_title_buttons (clist);

	/* redraw the invalid columns */
	if (CLIST_UNFROZEN (clist)) {
		area = clist->column_title_area;
		area.x = clist->column[column_index].area.x;
		area.height += clist->clist_window_height;

		EEL_CALL_METHOD (EEL_CLIST_CLASS, clist, draw_rows, (clist, &area));
	}
}

static void
eel_list_drag_start (GtkWidget *widget, GdkEventMotion *event)
{
	EelList *list;
	GdkDragContext *context;
	GdkPixbuf *pixbuf;

	g_return_if_fail (EEL_IS_LIST (widget));
	list = EEL_LIST (widget);

	if (list->details->drag_info == NULL) {
		return;
	}

	list->details->drag_started = TRUE;
	list->details->dnd_select_pending = FALSE;
	/* reinit from last dnd if there was one */
	list->details->drag_info->got_drop_data_type = FALSE;
	eel_drag_destroy_selection_list (list->details->drag_info->selection_list);
	list->details->drag_info->selection_list = NULL;
	
	context = gtk_drag_begin (widget, list->details->drag_info->target_list,
				  list->details->dnd_press_button == CONTEXTUAL_MENU_BUTTON
				  ? GDK_ACTION_ASK
				  : GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_ASK,
				  list->details->dnd_press_button,
				  (GdkEvent *) event);

	pixbuf = NULL;
	gtk_signal_emit (GTK_OBJECT (list), list_signals[GET_DRAG_PIXBUF], 
			 list->details->button_down_row, &pixbuf);

	if (pixbuf != NULL) {
		/* FIXME: We can do better than 10,10. */
		eel_drag_set_icon_pixbuf (context, pixbuf, 10, 10);
		
		gdk_pixbuf_unref (pixbuf);
	}
}

/* Our handler for motion_notify events.  We override all of GtkCList's broken
 * behavior.
 */
static int
eel_list_motion (GtkWidget *widget, GdkEventMotion *event)
{
	EelList *list;
	EelCList *clist;

	g_return_val_if_fail (EEL_IS_LIST (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	list = EEL_LIST (widget);
	clist = EEL_CLIST (widget);

	if (event->window != clist->clist_window) {
		return EEL_CALL_PARENT_WITH_RETURN_VALUE
			(GTK_WIDGET_CLASS, motion_notify_event, (widget, event));
	}
	
	if (!((list->details->dnd_press_button == ACTION_BUTTON && (event->state & GDK_BUTTON1_MASK))
	    || (list->details->dnd_press_button == CONTEXTUAL_MENU_BUTTON && (event->state & GDK_BUTTON3_MASK))))
		return FALSE;

	/* This is the same threshold value that is used in gtkdnd.c */

	if (MAX (abs (list->details->dnd_press_x - event->x),
		 abs (list->details->dnd_press_y - event->y)) <= 3) {
		return FALSE;
	}


	if (list->details->button_down_row < 0) {
		/* We didn't hit a row, just blank space */
		return FALSE;
	}

	g_assert (list->details->button_down_row < clist->rows);
	if (!list->details->drag_started) {
		if (list->details->dnd_press_button == CONTEXTUAL_MENU_BUTTON) {
			gtk_timeout_remove (list->details->context_menu_timeout_id);
		}
		eel_list_drag_start (widget, event);
	}
	
	return TRUE;
}

void 
eel_list_column_resize_track_start (GtkWidget *widget, int column_index)
{
	EelCList *clist;

	g_return_if_fail (EEL_IS_LIST (widget));

	clist = EEL_CLIST (widget);
	clist->drag_pos = column_index;
}

void 
eel_list_column_resize_track (GtkWidget *widget, int column_index)
{
	EelCList *clist;
	int x;

	g_return_if_fail (EEL_IS_LIST (widget));

	clist = EEL_CLIST (widget);

	gtk_widget_get_pointer (widget, &x, NULL);
	eel_list_track_new_column_width (clist, column_index, 
		new_column_width (clist, column_index, &x));
}

void 
eel_list_column_resize_track_end (GtkWidget *widget, int column_index)
{
	EelCList *clist;

	g_return_if_fail (EEL_IS_LIST (widget));

	clist = EEL_CLIST (widget);
	clist->drag_pos = -1;
}

/* FIXME bugzilla.eazel.com 7445: Needs to become a shared function */
static void
get_data_on_first_target_we_support (GtkWidget *widget, GdkDragContext *context, guint32 time)
{
	GList *target;

	if (eel_list_dnd_target_list == NULL)
		eel_list_dnd_target_list = gtk_target_list_new (eel_list_dnd_target_table,
								     EEL_N_ELEMENTS (eel_list_dnd_target_table));

	for (target = context->targets; target != NULL; target = target->next) {
		guint dummy_info;
		GdkAtom target_atom = GPOINTER_TO_UINT (target->data);

		if (gtk_target_list_find (eel_list_dnd_target_list, 
					  target_atom,
					  &dummy_info)) {
			gtk_drag_get_data (GTK_WIDGET (widget), context,
					   target_atom,
					   time);
			break;
		}
	}
}


static void
eel_list_ensure_drag_data (EelList *list,
				GdkDragContext *context,
				guint32 time)
{
	if (!list->details->drag_info->got_drop_data_type) {
		get_data_on_first_target_we_support (GTK_WIDGET (list), context, time);
	}
}

static void
eel_list_drag_end (GtkWidget *widget, GdkDragContext *context)
{
	EelList *list;
	EelDragInfo *drag_info;

	list = EEL_LIST (widget);
	drag_info = list->details->drag_info;

	drag_info->got_drop_data_type = FALSE;
	eel_drag_destroy_selection_list (list->details->drag_info->selection_list);
	list->details->drag_info->selection_list = NULL;

}

static void
eel_list_drag_leave (GtkWidget *widget, GdkDragContext *context, guint time)
{
	EelList *list;
	EelDragInfo *drag_info;

	list = EEL_LIST (widget);
	drag_info = list->details->drag_info;

	eel_list_stop_auto_scroll (EEL_LIST (list));

	eel_list_set_drag_prelight_row (list, -1);
}

gboolean
eel_list_rejects_dropped_icons (EelList *list)
{
	return list->details->rejects_dropped_icons;
}

void
eel_list_set_rejects_dropped_icons (EelList *list, gboolean new_value)
{
	list->details->rejects_dropped_icons = new_value;
}

static void
eel_list_get_drop_action (EelList *list, 
			       GdkDragContext *context,
			       int x, int y,
			       int *default_action,
			       int *non_default_action)
{
	EelDragInfo *drag_info;

	drag_info = EEL_LIST (list)->details->drag_info;

	/* FIXME bugzilla.eazel.com 2569: Too much code copied from eel-icon-dnd.c.
	 * Need to share more.
	 */

	if (!drag_info->got_drop_data_type) {
		/* drag_data_received didn't get called yet */
		return;
	}

	/* get those actions from a subclass of this object */
	gtk_signal_emit (GTK_OBJECT (list), 
			 list_signals[GET_DEFAULT_ACTION],
			 default_action, 
			 non_default_action,  
			 context,
			 drag_info->selection_list,
			 x, y, 
			 drag_info->data_type);

}			       


static void
eel_list_real_scroll (EelList *list, float delta_x, float delta_y)
{
	GtkAdjustment *hadj, *vadj;

	hadj = eel_clist_get_hadjustment (EEL_CLIST (list));
	vadj = eel_clist_get_vadjustment (EEL_CLIST (list));

	eel_gtk_adjustment_set_value (hadj, hadj->value + (int)delta_x);
	eel_gtk_adjustment_set_value (vadj, vadj->value + (int)delta_y);

}

static int
auto_scroll_timeout_callback (gpointer data)
{
	EelList *list;
	EelDragInfo *drag_info;
	GtkWidget *widget;
	float x_scroll_delta, y_scroll_delta;

	g_assert (EEL_IS_LIST (data));
	widget = GTK_WIDGET (data);
	list = EEL_LIST (widget);
	drag_info = list->details->drag_info;

	if (drag_info->waiting_to_autoscroll
	    && drag_info->start_auto_scroll_in > eel_get_system_time()) {
		/* not yet */
		return TRUE;
	}

	drag_info->waiting_to_autoscroll = FALSE;

	eel_drag_autoscroll_calculate_delta (widget, &x_scroll_delta, &y_scroll_delta);

	eel_list_real_scroll (list, x_scroll_delta, y_scroll_delta);

	return TRUE;
}

static void
eel_list_start_auto_scroll (EelList *list)
{
	g_assert (EEL_IS_LIST (list));

	eel_drag_autoscroll_start (list->details->drag_info,
					GTK_WIDGET (list),
					auto_scroll_timeout_callback,
					list);
}

static void
eel_list_stop_auto_scroll (EelList *list)
{
	g_assert (EEL_IS_LIST (list));

	eel_drag_autoscroll_stop (list->details->drag_info);
}

static void
eel_list_prelight_if_necessary (EelList *list, GdkDragContext *context,
				     int x, int y, guint time)
{
	gboolean is_prelight_necessary;
	
	/* should we prelight the current row ? */
	gtk_signal_emit (GTK_OBJECT (list), 
			 list_signals[HANDLE_DRAGGED_ITEMS],
			 context->action, 
			 list->details->drag_info->selection_list,
			 x, y, 
			 list->details->drag_info->data_type, 
		 	 &is_prelight_necessary);

	if (is_prelight_necessary) {
		eel_list_set_drag_prelight_row (list, y);
	} else {
		eel_list_set_drag_prelight_row (list, -1);
	}
}


static gboolean
eel_list_drag_motion (GtkWidget *widget, GdkDragContext *context,
			   int x, int y, guint time)
{
	EelList *list;
	int default_action, non_default_action, resulting_action;

	list = EEL_LIST (widget);

	eel_list_ensure_drag_data (list, context,  time);

	eel_list_start_auto_scroll (EEL_LIST (widget));

	default_action = 0; 
	non_default_action = 0;

	eel_list_get_drop_action (list, context, x, y, &default_action, &non_default_action);
	resulting_action = eel_drag_modifier_based_action (default_action, non_default_action);

	gdk_drag_status (context, resulting_action, time);

	eel_list_prelight_if_necessary (list, context, x, y, time);

	return TRUE;
}

static gboolean
eel_list_drag_drop (GtkWidget *widget, GdkDragContext *context,
			 int x, int y, guint time)
{
	EelList *list;
	
	list = EEL_LIST (widget);

	/* make sure that drag_data_received is going to be called
	   after this event and will do the actual actions */
	list->details->drag_info->drop_occured = TRUE;
	get_data_on_first_target_we_support (widget, context, time);

	return FALSE;
}

static void
eel_list_receive_dropped_icons (EelList *list,
				     int action,
				     GtkSelectionData *data,
				     int x, int y, guint info)
{
	EelDragInfo *drag_info;
	GList *selected_items;

	g_assert (EEL_IS_LIST (list));
	drag_info = list->details->drag_info;

	/* Put selection list in local variable and NULL the global one
	 * so it doesn't get munged in a modal popup-menu event loop
	 * in the handle_dropped_item handler.
	 */
	selected_items = drag_info->selection_list;
	drag_info->selection_list = NULL;
	gtk_signal_emit (GTK_OBJECT (list), list_signals[HANDLE_DROPPED_ITEMS],
			 action, selected_items, x, y, info);
	eel_drag_destroy_selection_list (selected_items);
}

static void
eel_list_receive_dropped_keyword (EelList *list,
				       int action,
				       GtkSelectionData *data,
				       int x, int y,
				       guint info)
{
	GList *emblems;

	emblems = g_list_prepend (NULL, (char *)data->data);

	gtk_signal_emit (GTK_OBJECT (list), 
			 list_signals[HANDLE_DROPPED_ITEMS],
			 action, emblems, x, y, info);

	g_list_free (emblems);
}



static void
eel_list_drag_data_received (GtkWidget *widget, GdkDragContext *context,
				  int x, int y, GtkSelectionData *data,
				  guint info, guint time)
{
	EelList *list;
	EelDragInfo *drag_info;

	list = EEL_LIST (widget);
	drag_info = list->details->drag_info;


	if (!drag_info->got_drop_data_type) {

		drag_info->data_type = info;
		drag_info->got_drop_data_type = TRUE;
		drag_info->selection_data = data;


		switch (info) {
		case EEL_ICON_DND_GNOME_ICON_LIST:
			drag_info->selection_list = eel_drag_build_selection_list (data);
			break;
		case EEL_ICON_DND_URI_LIST:
			drag_info->selection_list = eel_drag_build_selection_list (data);
			break;
		case EEL_ICON_DND_COLOR:
			break;
		case EEL_ICON_DND_BGIMAGE:	
			break;
		case EEL_ICON_DND_KEYWORD:	
			break;
		default:
			break;
		}
	}

	if (drag_info->drop_occured) {

		switch (info) {
		case EEL_ICON_DND_GNOME_ICON_LIST:
			eel_list_receive_dropped_icons
				(EEL_LIST (list),
				 context->action, data, x, y, info);
			gtk_drag_finish (context, TRUE, FALSE, time);
			break;
		case EEL_ICON_DND_URI_LIST:
			eel_list_receive_dropped_icons
				(EEL_LIST (list),
				 context->action, data, x, y, info);
			gtk_drag_finish (context, TRUE, FALSE, time);
			break;
		case EEL_ICON_DND_COLOR:
			eel_background_receive_dropped_color
				(eel_get_widget_background (widget),
				 widget, x, y, data);
			eel_list_setup_style_colors (EEL_LIST (list));
			gtk_drag_finish (context, TRUE, FALSE, time);
			break;
		case EEL_ICON_DND_BGIMAGE:
			eel_background_receive_dropped_background_image
				(eel_get_widget_background (widget),
				 (char *)data->data);
			gtk_drag_finish (context, TRUE, FALSE, time);
			break;
		case EEL_ICON_DND_KEYWORD:
			eel_list_receive_dropped_keyword
				(EEL_LIST (list),
				 context->action, data, x, y, info);
			gtk_drag_finish (context, TRUE, FALSE, time);
			break;
		default:
			gtk_drag_finish (context, FALSE, FALSE, time);
			break;
		}


		drag_info->drop_occured = FALSE;
		drag_info->got_drop_data_type = FALSE;
	}
}



/* Our handler for the clear signal of the clist.  We have to reset the anchor
  * to null.
 */
static void
eel_list_clear (EelCList *clist)
{
	EelList *list;

	g_return_if_fail (EEL_IS_LIST (clist));

	list = EEL_LIST (clist);
	list->details->anchor_row = -1;

	EEL_CALL_PARENT (EEL_CLIST_CLASS, clear, (clist));
}


/**
 * eel_list_new_with_titles:
 * @columns: The number of columns in the list
 * @titles: The titles for the columns
 * 
 * Return value: The newly-created file list.
 **/
GtkWidget *
eel_list_new_with_titles (int columns, const char * const *titles)
{
	EelList *list;

	list = EEL_LIST (gtk_type_new (eel_list_get_type ()));
	eel_clist_construct (EEL_CLIST (list), columns, NULL);
	if (titles) {
		EelCList *clist;
		int index;

		clist = EEL_CLIST(list);
		
		for (index = 0; index < columns; index++) {
  			clist->column[index].title = g_strdup (titles[index]);
  		}
    	}

	eel_clist_set_selection_mode (EEL_CLIST (list),
				      GTK_SELECTION_MULTIPLE);

	return GTK_WIDGET (list);
}

EelCListRow *
eel_list_row_at (EelList *list, int y)
{
	EelCList *clist;
	int row_index, column_index;

	clist = EEL_CLIST (list);
	y -= (GTK_CONTAINER (list)->border_width +
		GTK_WIDGET (list)->style->klass->ythickness +
		clist->column_title_area.height);
	
	if (!eel_clist_get_selection_info (clist, 10, y, &row_index, &column_index)) {
		return NULL;
	}
	
	return g_list_nth (clist->row_list, row_index)->data;
}

GList *
eel_list_get_selection (EelList *list)
{
	GList *retval;
	GList *p;

	g_return_val_if_fail (EEL_IS_LIST (list), NULL);

	retval = NULL;
	for (p = EEL_CLIST (list)->row_list; p != NULL; p = p->next) {
		EelCListRow *row;

		row = p->data;
		if (row->state == GTK_STATE_SELECTED)
			retval = g_list_prepend (retval, row->data);
	}

	return retval;
}

void
eel_list_set_selection (EelList *list, GList *selection)
{
	gboolean selection_changed;
	GHashTable *hash;
	GList *p;
	int i;
	EelCListRow *row;

	g_return_if_fail (EEL_IS_LIST (list));

	selection_changed = FALSE;

	hash = g_hash_table_new (NULL, NULL);
	for (p = selection; p != NULL; p = p->next) {
		g_hash_table_insert (hash, p->data, p->data);
	}

	for (p = EEL_CLIST (list)->row_list, i = 0; p != NULL; p = p->next, i++) {
		row = p->data;
		selection_changed |= row_set_selected (list, i, row, g_hash_table_lookup (hash, row->data) != NULL);
	}
	
	g_hash_table_destroy (hash);

	if (selection_changed) {
		emit_selection_changed (list);
	}
}

void
eel_list_each_selected_row (EelList *list, EelEachRowFunction function,
	gpointer data)
{
	EelCListRow *row;
	GList *p;
	int row_index;

	g_assert (EEL_IS_LIST (list));

	for (p = EEL_CLIST (list)->row_list, row_index = 0; p != NULL; p = p->next, row_index++) {
		row = p->data;
		if (row->state != GTK_STATE_SELECTED) 
			continue;

		if (!function(row, row_index, data))
			return;
	}
}

/**
 * eel_list_get_first_selected_row:
 * 
 * Get the index of the first selected row, or -1 if no rows are selected.
 * @list: Any EelList
 **/
int
eel_list_get_first_selected_row (EelList *list)
{
	EelCListRow *row;
	GList *p;
	int row_index;

	g_return_val_if_fail (EEL_IS_LIST (list), -1);

	for (p = EEL_CLIST (list)->row_list, row_index = 0; 
	     p != NULL; 
	     p = p->next, ++row_index) {
		row = p->data;
		if (row->state == GTK_STATE_SELECTED) 
			return row_index;
	}

	return -1;
}

/**
 * eel_list_get_last_selected_row:
 * 
 * Get the index of the last selected row, or -1 if no rows are selected.
 * @list: Any GtkCList
 **/
int 
eel_list_get_last_selected_row (EelList *list)
{
	EelCListRow *row;
	GList *p;
	int row_index;

	g_return_val_if_fail (EEL_IS_LIST (list), -1);

	for (p = EEL_CLIST (list)->row_list_end, row_index = EEL_CLIST (list)->rows - 1; p != NULL; p = p->prev, --row_index) {
		row = p->data;
		if (row->state == GTK_STATE_SELECTED) {
			return row_index;	
		}
	}

	return -1;
}

/* Workaround for a bug in GtkCList's insert_row.
 * It sets the focus row to 0 if there is exactly one row,
 * even if there was no focus on entry.
 * Although this works for focus, there may still be a problem
 * with selection.
 */
static int
insert_row (EelCList *list, int row_index, char *text[])
{
	gboolean had_focus;
	int result;

	had_focus = list->focus_row != -1;

	result = EEL_CALL_PARENT_WITH_RETURN_VALUE
		(EEL_CLIST_CLASS, insert_row, (list, row_index, text));

	if (!had_focus) {
		list->focus_row = -1;
	}

	return result;
}


void 	     
eel_list_set_drag_prelight_row (EelList *list, int y)
{
	EelCList *clist;
	EelCListRow *row, *last_row;
	GdkRectangle rect;
	int row_index;
	
	clist = EEL_CLIST (list);

	row = NULL;
	
	if (y >= 0) { 
		row = eel_list_row_at (list, y);
	}

	if (row != list->details->drag_prelight_row) {
		last_row = list->details->drag_prelight_row;
		list->details->drag_prelight_row = row;
		
		/* Redraw old cell */
		if (last_row != NULL) {
			row_index = g_list_index (clist->row_list, last_row);
			eel_list_get_cell_rectangle (list, row_index, 0, &rect);
			gtk_widget_draw (GTK_WIDGET (list), &rect);			
		}
		
		/* Draw new cell */
		if (list->details->drag_prelight_row != NULL) {
			row_index = g_list_index (clist->row_list, list->details->drag_prelight_row);
			eel_list_get_cell_rectangle (list, row_index, 0, &rect);
			gtk_widget_draw (GTK_WIDGET (list), &rect);			
		}
	}
}

void 
eel_list_get_initial_drag_offset (EelList *list, int *x, int *y)
{
	*x = list->details->dnd_press_x;
	*y = list->details->dnd_press_y;
}

