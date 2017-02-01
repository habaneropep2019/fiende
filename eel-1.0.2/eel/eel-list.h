/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-list.h: Enhanced version of GtkCList for Eel.

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

#ifndef EEL_LIST_H
#define EEL_LIST_H

#include <libgnome/gnome-defs.h>
#include <eel/eel-clist.h>

/* This class was originally derived from the GtkFList class in gmc.
 */

/* It is sad that we have to do this. GtkCList's behavior is so broken that we
 * have to override all the event handlers and implement our own selection
 * behavior. Sigh. -Federico
 */

/* pointer casting for cells */
#define EEL_CELL_PIXBUF_LIST(cell)	((EelCellPixbufList *) &(cell))
/* no #define for EEL_CELL_LINK_TEXT, use GTK_CELL_TEXT instead */

/* returns the GList item for the nth row */
#define	ROW_ELEMENT(clist, row)	(((row) == (clist)->rows - 1) ? \
				 (clist)->row_list_end : \
				 g_list_nth ((clist)->row_list, (row)))

typedef struct EelCellPixbufList EelCellPixbufList;
/* no struct for EelCellLinkText, use GtkCellText instead */

/* Since the info in each cell must fit in the GtkCell struct that CList defines,
 * we disguise ours in the GtkCellWidget format, with our pixbufs list where
 * the widget would be.
 */
struct EelCellPixbufList
{
	EelCellType type;
	
	gint16 vertical;
	gint16 horizontal;
	
	GtkStyle *style;
	
	GList *pixbufs; /* list of GdkPixbuf * */
};

#define EEL_TYPE_LIST            (eel_list_get_type ())
#define EEL_LIST(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_LIST, EelList))
#define EEL_LIST_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_LIST, EelListClass))
#define EEL_IS_LIST(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_LIST))
#define EEL_IS_LIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_LIST))

typedef struct EelList EelList;
typedef struct EelListClass EelListClass;
typedef struct EelListDetails EelListDetails;

struct EelList {
	EelCList clist;
	EelListDetails *details;
};

struct EelListClass {
	EelCListClass parent_class;

	/* Signal: invoke the popup menu for selected items */
	void (* context_click_selection) (EelList *list, 
					  GdkEventButton *event);

	/* Signal: invoke the popup menu for empty areas */
	void (* context_click_background) (EelList *list,
					   GdkEventButton *event);

	/* Signal: announce that one or more items have been activated. */
	void (* activate) (EelList *list, GList *row_data_list);

	/* Signal: selection has changed */
	void (* selection_changed) (EelList *list);

	/* column resize tracking calls */
	void      (* column_resize_track_start) (GtkWidget *widget, int column);
	void      (* column_resize_track)       (GtkWidget *widget, int column);
	void      (* column_resize_track_end)   (GtkWidget *widget, int column);
	void      (* select_matching_name)      (GtkWidget *widget, const char *);
	void      (* select_previous_name)      (GtkWidget *widget);
	void      (* select_next_name)          (GtkWidget *widget);
	GdkPixbuf (* get_drag_pixbuf)           (EelList *list, int row_index);
	int       (* get_sort_column_index)     (EelList *list);
	char     *(* get_cell_text)		(GtkWidget *widget, int column_index, int cell_width,
						 EelCListRow *row, GdkFont *font);

	/* dnd handling. defer the semantics of dnd to the application side, not eel-list */
	gboolean  (* handle_dragged_items)      (GtkWidget     *widget,
						 int            action,
						 GList         *drop_data,
						 int            x,
						 int            y,
						 guint          info);
	void      (* handle_dropped_items)      (GtkWidget     *widget,
						 int            action,
						 GList         *drop_data,
						 int            x,
						 int            y,
						 guint          info);
	void      (* get_default_action)        (GtkWidget     *widget,
						 int           *default_action,
						 int           *non_default_action,
						 GdkDragContext *context,
						 GList         *drop_data,
						 int            x,
						 int            y,
						 guint          info);

};

typedef gboolean (* EelEachRowFunction) (EelCListRow *, int, gpointer);

GtkType      eel_list_get_type                     (void);
GtkWidget *  eel_list_new_with_titles              (int                  columns,
						    const char * const  *titles);
void         eel_list_initialize_dnd               (EelList             *list);
GList *      eel_list_get_selection                (EelList             *list);
void         eel_list_set_selection                (EelList             *list,
						    GList               *selection);
void         eel_list_reveal_row                   (EelList             *list,
						    int                  row);
gboolean     eel_list_is_row_selected              (EelList             *list,
						    int                  row);
void         eel_list_get_cell_rectangle           (EelList             *clist,
						    int                  row_index,
						    int                  column_index,
						    GdkRectangle        *result);
void         eel_list_set_pixbuf_list              (EelList             *list,
						    gint                 row,
						    gint                 column,
						    GList               *pixbufs);
void         eel_list_set_pixbuf                   (EelList             *list,
						    int                  row_index,
						    int                  column_index,
						    GdkPixbuf           *pixbuf);
GdkPixbuf   *eel_list_get_pixbuf                   (EelList             *list,
						    int                  row_index,
						    int                  column_index);
void         eel_list_mark_cell_as_link            (EelList             *list,
						    gint                 row,
						    gint                 column);
void         eel_list_set_single_click_mode        (EelList             *list,
						    gboolean             single_click_mode);
void         eel_list_select_row                   (EelList             *list,
						    int                  row);
EelCListRow *eel_list_row_at                       (EelList             *list,
						    int                  y);
int          eel_list_get_first_selected_row       (EelList             *list);
int          eel_list_get_last_selected_row        (EelList             *list);
void         eel_list_each_selected_row            (EelList             *list,
						    EelEachRowFunction   function,
						    gpointer             data);
gboolean     eel_list_rejects_dropped_icons        (EelList             *list);
void         eel_list_set_rejects_dropped_icons    (EelList             *list,
						    gboolean             new_value);
void         eel_list_set_drag_prelight_row        (EelList             *list,
						    int                  y);
void         eel_list_get_initial_drag_offset      (EelList             *list,
						    int                 *x,
						    int                 *y);
void         eel_list_set_anti_aliased_mode        (EelList             *list,
						    gboolean             anti_aliased_mode);
gboolean     eel_list_is_anti_aliased              (EelList             *list);
int          eel_list_draw_cell_pixbuf             (EelList             *list,
						    GdkWindow           *window,
						    GdkRectangle        *clip_rectangle,
						    GdkGC               *fg_gc,
						    guint32              bg_rgb,
						    GdkPixbuf           *pixbuf,
						    int                  x,
						    int                  y);
void         eel_list_get_cell_style               (EelList             *list,
						    EelCListRow         *row,
						    int                  state,
						    int                  row_index,
						    int                  column_index,
						    GtkStyle           **style,
						    GdkGC              **fg_gc,
						    GdkGC              **bg_gc,
						    guint32             *bg_rgb);
void         eel_list_set_alternate_row_colors     (EelList             *list,
						    gboolean             state);
void         eel_list_set_background_color_offsets (EelList             *list,
						    long                 background_offset,
						    long                 selection_offset);
void         eel_list_set_sort_type 		   (EelList    		*list,
						    GtkSortType 	 sort_type);
void         eel_list_set_sort_column 		   (EelList    		*list,
						    int 	 	 column);

#endif /* EEL_LIST_H */





