/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball, Josh MacDonald
 * Copyright (C) 1997-1998 Jay Painter <jpaint@serv.net><jpaint@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

/* 
 * Copy-pasted from GtkCList to add some missing overridability.
 */

#ifndef EEL_CLIST_H__
#define EEL_CLIST_H__

#include <gdk/gdk.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhscrollbar.h>
#include <gtk/gtkvscrollbar.h>
#include <gtk/gtkenums.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clist flags */
enum {
  EEL_CLIST_IN_DRAG             = 1 <<  0,
  EEL_CLIST_ROW_HEIGHT_SET      = 1 <<  1,
  EEL_CLIST_SHOW_TITLES         = 1 <<  2,
  EEL_CLIST_CHILD_HAS_FOCUS     = 1 <<  3,
  EEL_CLIST_ADD_MODE            = 1 <<  4,
  EEL_CLIST_AUTO_SORT           = 1 <<  5,
  EEL_CLIST_AUTO_RESIZE_BLOCKED = 1 <<  6,
  EEL_CLIST_REORDERABLE         = 1 <<  7,
  EEL_CLIST_USE_DRAG_ICONS      = 1 <<  8,
  EEL_CLIST_DRAW_DRAG_LINE      = 1 <<  9,
  EEL_CLIST_DRAW_DRAG_RECT      = 1 << 10
}; 

/* cell types */
/* Superset of GtkCellType enum defined in gtk-clist.h */
typedef enum
{
  EEL_CELL_EMPTY,
  EEL_CELL_TEXT,
  EEL_CELL_PIXBUF,   	/* new for Eel */
  EEL_CELL_PIXTEXT,        /* now uses pixbuf */
  EEL_CELL_WIDGET,
  EEL_CELL_PIXBUF_LIST,   	/* new for Eel */
  EEL_CELL_LINK_TEXT	/* new for Eel */
} EelCellType;

typedef enum
{
  EEL_CLIST_DRAG_NONE,
  EEL_CLIST_DRAG_BEFORE,
  EEL_CLIST_DRAG_INTO,
  EEL_CLIST_DRAG_AFTER
} EelCListDragPos;

typedef enum
{
  EEL_BUTTON_IGNORED = 0,
  EEL_BUTTON_SELECTS = 1 << 0,
  EEL_BUTTON_DRAGS   = 1 << 1,
  EEL_BUTTON_EXPANDS = 1 << 2
} EelButtonAction;

#define EEL_TYPE_CLIST            (eel_clist_get_type ())
#define EEL_CLIST(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_CLIST, EelCList))
#define EEL_CLIST_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_CLIST, EelCListClass))
#define EEL_IS_CLIST(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_CLIST))
#define EEL_IS_CLIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_CLIST))

#define EEL_CLIST_FLAGS(clist)             (EEL_CLIST (clist)->flags)
#define EEL_CLIST_SET_FLAG(clist,flag)     (EEL_CLIST_FLAGS (clist) |= (EEL_ ## flag))
#define EEL_CLIST_UNSET_FLAG(clist,flag)   (EEL_CLIST_FLAGS (clist) &= ~(EEL_ ## flag))

#define EEL_CLIST_IN_DRAG(clist)           (EEL_CLIST_FLAGS (clist) & EEL_CLIST_IN_DRAG)
#define EEL_CLIST_ROW_HEIGHT_SET(clist)    (EEL_CLIST_FLAGS (clist) & EEL_CLIST_ROW_HEIGHT_SET)
#define EEL_CLIST_SHOW_TITLES(clist)       (EEL_CLIST_FLAGS (clist) & EEL_CLIST_SHOW_TITLES)
#define EEL_CLIST_CHILD_HAS_FOCUS(clist)   (EEL_CLIST_FLAGS (clist) & EEL_CLIST_CHILD_HAS_FOCUS)
#define EEL_CLIST_ADD_MODE(clist)          (EEL_CLIST_FLAGS (clist) & EEL_CLIST_ADD_MODE)
#define EEL_CLIST_AUTO_SORT(clist)         (EEL_CLIST_FLAGS (clist) & EEL_CLIST_AUTO_SORT)
#define EEL_CLIST_AUTO_RESIZE_BLOCKED(clist) (EEL_CLIST_FLAGS (clist) & EEL_CLIST_AUTO_RESIZE_BLOCKED)
#define EEL_CLIST_REORDERABLE(clist)       (EEL_CLIST_FLAGS (clist) & EEL_CLIST_REORDERABLE)
#define EEL_CLIST_USE_DRAG_ICONS(clist)    (EEL_CLIST_FLAGS (clist) & EEL_CLIST_USE_DRAG_ICONS)
#define EEL_CLIST_DRAW_DRAG_LINE(clist)    (EEL_CLIST_FLAGS (clist) & EEL_CLIST_DRAW_DRAG_LINE)
#define EEL_CLIST_DRAW_DRAG_RECT(clist)    (EEL_CLIST_FLAGS (clist) & EEL_CLIST_DRAW_DRAG_RECT)

#define EEL_CLIST_ROW(_glist_) ((EelCListRow *)((_glist_)->data))

/* pointer casting for cells */
#define EEL_CELL_TEXT(cell)     (((EelCellText *) &(cell)))
#define EEL_CELL_PIXBUF(cell)   (((EelCellPixbuf *) &(cell)))
#define EEL_CELL_PIXTEXT(cell)  (((EelCellPixText *) &(cell)))
#define EEL_CELL_WIDGET(cell)   (((EelCellWidget *) &(cell)))

typedef struct EelCList EelCList;
typedef struct EelCListClass EelCListClass;
typedef struct EelCListColumn EelCListColumn;
typedef struct EelCListRow EelCListRow;

typedef struct EelCell EelCell;
typedef struct EelCellText EelCellText;
typedef struct EelCellPixbuf EelCellPixbuf;
typedef struct EelCellPixText EelCellPixText;
typedef struct EelCellWidget EelCellWidget;

typedef gint (*EelCListCompareFunc) (EelCList     *clist,
				     gconstpointer ptr1,
				     gconstpointer ptr2);

typedef struct EelCListCellInfo EelCListCellInfo;
typedef struct EelCListDestInfo EelCListDestInfo;

struct EelCListCellInfo
{
  gint row;
  gint column;
};

struct EelCListDestInfo
{
  EelCListCellInfo cell;
  EelCListDragPos  insert_pos;
};

struct EelCList
{
  GtkContainer container;
  
  guint16 flags;
  
  /* mem chunks */
  GMemChunk *row_mem_chunk;
  GMemChunk *cell_mem_chunk;

  guint freeze_count;
  gboolean refresh_at_unfreeze_time;
  
  /* allocation rectangle after the conatiner_border_width
   * and the width of the shadow border */
  GdkRectangle internal_allocation;
  
  /* rows */
  gint rows;
  gint row_center_offset;
  gint row_height;
  GList *row_list;
  GList *row_list_end;
  
  /* columns */
  gint columns;
  GdkRectangle column_title_area;
  GdkWindow *title_window;
  
  /* dynamicly allocated array of column structures */
  EelCListColumn *column;
  
  /* the scrolling window and its height and width to
   * make things a little speedier */
  GdkWindow *clist_window;
  gint clist_window_width;
  gint clist_window_height;
  
  /* offsets for scrolling */
  gint hoffset;
  gint voffset;
  
  /* border shadow style */
  GtkShadowType shadow_type;
  
  /* the list's selection mode (gtkenums.h) */
  GtkSelectionMode selection_mode;
  
  /* list of selected rows */
  GList *selection;
  GList *selection_end;
  
  GList *undo_selection;
  GList *undo_unselection;
  gint undo_anchor;
  
  /* mouse buttons */
  guint8 button_actions[5];

  guint8 drag_button;

  /* dnd */
  EelCListCellInfo click_cell;

  /* scroll adjustments */
  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;
  
  /* xor GC for the vertical drag line */
  GdkGC *xor_gc;
  
  /* gc for drawing unselected cells */
  GdkGC *fg_gc;
  GdkGC *bg_gc;
  
  /* cursor used to indicate dragging */
  GdkCursor *cursor_drag;
  
  /* the current x-pixel location of the xor-drag line */
  gint x_drag;
  
  /* focus handling */
  gint focus_row;
  
  /* dragging the selection */
  gint anchor;
  GtkStateType anchor_state;
  gint drag_pos;
  gint htimer;
  gint vtimer;
  
  GtkSortType sort_type;
  EelCListCompareFunc compare;
  gint sort_column;
};

struct EelCListClass
{
  GtkContainerClass parent_class;
  
  void  (*set_scroll_adjustments) (EelCList       *clist,
				   GtkAdjustment  *hadjustment,
				   GtkAdjustment  *vadjustment);
  void   (*refresh)             (EelCList       *clist);
  void   (*select_row)          (EelCList       *clist,
				 gint            row,
				 gint            column,
				 GdkEvent       *event);
  void   (*unselect_row)        (EelCList       *clist,
				 gint            row,
				 gint            column,
				 GdkEvent       *event);
  void   (*row_move)            (EelCList       *clist,
				 gint            source_row,
				 gint            dest_row);
  void   (*click_column)        (EelCList       *clist,
				 gint            column);
  void   (*resize_column)       (EelCList       *clist,
				 gint            column,
                                 gint            width);
  void   (*toggle_focus_row)    (EelCList       *clist);
  void   (*select_all)          (EelCList       *clist);
  void   (*unselect_all)        (EelCList       *clist);
  void   (*undo_selection)      (EelCList       *clist);
  void   (*start_selection)     (EelCList       *clist);
  void   (*end_selection)       (EelCList       *clist);
  void   (*extend_selection)    (EelCList       *clist,
				 GtkScrollType   scroll_type,
				 gfloat          position,
				 gboolean        auto_start_selection);
  void   (*scroll_horizontal)   (EelCList       *clist,
				 GtkScrollType   scroll_type,
				 gfloat          position);
  void   (*scroll_vertical)     (EelCList       *clist,
				 GtkScrollType   scroll_type,
				 gfloat          position);
  void   (*toggle_add_mode)     (EelCList       *clist);
  void   (*abort_column_resize) (EelCList       *clist);
  void   (*resync_selection)    (EelCList       *clist,
				 GdkEvent       *event);
  GList* (*selection_find)      (EelCList       *clist,
				 gint            row_number,
				 GList          *row_list_element);
  void   (*draw_rows)           (EelCList       *clist,
				 GdkRectangle   *area);
  void   (*draw_row)            (EelCList       *clist,
				 GdkRectangle   *area,
				 gint            row,
				 EelCListRow    *clist_row);
  void   (*draw_all)            (EelCList       *clist);
  void   (*draw_drag_highlight) (EelCList        *clist,
				 EelCListRow     *target_row,
				 gint             target_row_number,
				 EelCListDragPos  drag_pos);
  void   (*clear)               (EelCList       *clist);
  void   (*fake_unselect_all)   (EelCList       *clist,
				 gint            row);
  void   (*sort_list)           (EelCList       *clist);
  gint   (*insert_row)          (EelCList       *clist,
				 gint            row,
				 gchar          *text[]);
  void   (*remove_row)          (EelCList       *clist,
				 gint            row);
  gboolean (*set_cell_contents) (EelCList       *clist,
				 EelCListRow    *clist_row,
				 gint            column,
				 EelCellType     type,
				 const gchar    *text,
				 guint8          spacing,
				 GdkPixbuf      *pixbuf);
  void   (*cell_size_request)   (EelCList       *clist,
				 EelCListRow    *clist_row,
				 gint            column,
				 GtkRequisition *requisition);

};

struct EelCListColumn
{
  gchar *title;
  GdkRectangle area;
  
  GtkWidget *button;
  GdkWindow *window;
  
  gint width;
  gint min_width;
  gint max_width;
  GtkJustification justification;
  
  guint visible        : 1;  
  guint width_set      : 1;
  guint resizeable     : 1;
  guint auto_resize    : 1;
  guint button_passive : 1;
};

struct EelCListRow
{
  EelCell *cell;
  GtkStateType state;
  
  GdkColor foreground;
  GdkColor background;
  
  GtkStyle *style;

  gpointer data;
  GtkDestroyNotify destroy;
  
  guint fg_set     : 1;
  guint bg_set     : 1;
  guint selectable : 1;
};

/* Cell Structures */
struct EelCellText
{
  EelCellType type;
  
  gint16 vertical;
  gint16 horizontal;
  
  GtkStyle *style;

  gchar *text;
};

struct EelCellPixbuf
{
  EelCellType type;
  
  gint16 vertical;
  gint16 horizontal;
  
  GtkStyle *style;

  GdkPixbuf *pixbuf;
};

struct EelCellPixText
{
  EelCellType type;
  
  gint16 vertical;
  gint16 horizontal;
  
  GtkStyle *style;

  gchar *text;
  guint8 spacing;
  GdkPixbuf *pixbuf;
};

struct EelCellWidget
{
  EelCellType type;
  
  gint16 vertical;
  gint16 horizontal;
  
  GtkStyle *style;

  GtkWidget *widget;
};

struct EelCell
{
  EelCellType type;
  
  gint16 vertical;
  gint16 horizontal;
  
  GtkStyle *style;

  union {
    gchar *text;
    
    struct {
      GdkPixbuf *pixbuf;
    } pb;
    
    struct {
      gchar *text;
      guint8 spacing;
      GdkPixbuf *pixbuf;
    } pt;
    
    GtkWidget *widget;
  } u;
};

GtkType        eel_clist_get_type                 (void);

/* constructors useful for gtk-- wrappers */
void           eel_clist_construct                (EelCList             *clist,
						   gint                  columns,
						   gchar                *titles[]);

/* create a new EelCList */
GtkWidget*     eel_clist_new                      (gint                  columns);
GtkWidget*     eel_clist_new_with_titles          (gint                  columns,
						   gchar                *titles[]);

/* set adjustments of clist */
void           eel_clist_set_hadjustment          (EelCList             *clist,
						   GtkAdjustment        *adjustment);
void           eel_clist_set_vadjustment          (EelCList             *clist,
						   GtkAdjustment        *adjustment);

/* get adjustments of clist */
GtkAdjustment* eel_clist_get_hadjustment          (EelCList             *clist);
GtkAdjustment* eel_clist_get_vadjustment          (EelCList             *clist);

/* set the border style of the clist */
void           eel_clist_set_shadow_type          (EelCList             *clist,
						   GtkShadowType         type);

/* set the clist's selection mode */
void           eel_clist_set_selection_mode       (EelCList             *clist,
						   GtkSelectionMode      mode);

/* enable clists reorder ability */
void           eel_clist_set_reorderable          (EelCList             *clist,
						   gboolean              reorderable);
void           eel_clist_set_use_drag_icons       (EelCList             *clist,
						   gboolean              use_icons);
void           eel_clist_set_button_actions       (EelCList             *clist,
						   guint                 button,
						   guint8                button_actions);

/* freeze all visual updates of the list, and then thaw the list after
 * you have made a number of changes and the updates wil occure in a
 * more efficent mannor than if you made them on a unfrozen list
 */
void           eel_clist_freeze                   (EelCList             *clist);
void           eel_clist_thaw                     (EelCList             *clist);

/* show and hide the column title buttons */
void           eel_clist_column_titles_show       (EelCList             *clist);
void           eel_clist_column_titles_hide       (EelCList             *clist);

/* set the column title to be a active title (responds to button presses, 
 * prelights, and grabs keyboard focus), or passive where it acts as just
 * a title
 */
void           eel_clist_column_title_active      (EelCList             *clist,
						   gint                  column);
void           eel_clist_column_title_passive     (EelCList             *clist,
						   gint                  column);
void           eel_clist_column_titles_active     (EelCList             *clist);
void           eel_clist_column_titles_passive    (EelCList             *clist);

/* set the title in the column title button */
void           eel_clist_set_column_title         (EelCList             *clist,
						   gint                  column,
						   const gchar          *title);

/* returns the title of column. Returns NULL if title is not set */
gchar *        eel_clist_get_column_title         (EelCList             *clist,
						   gint                  column);

/* set a widget instead of a title for the column title button */
void           eel_clist_set_column_widget        (EelCList             *clist,
						   gint                  column,
						   GtkWidget            *widget);

/* returns the column widget */
GtkWidget *    eel_clist_get_column_widget        (EelCList             *clist,
						   gint                  column);

/* set the justification on a column */
void           eel_clist_set_column_justification (EelCList             *clist,
						   gint                  column,
						   GtkJustification      justification);

/* set visibility of a column */
void           eel_clist_set_column_visibility    (EelCList             *clist,
						   gint                  column,
						   gboolean              visible);

/* enable/disable column resize operations by mouse */
void           eel_clist_set_column_resizeable    (EelCList             *clist,
						   gint                  column,
						   gboolean              resizeable);

/* resize column automatically to its optimal width */
void           eel_clist_set_column_auto_resize   (EelCList             *clist,
						   gint                  column,
						   gboolean              auto_resize);
gint           eel_clist_columns_autosize         (EelCList             *clist);

/* return the optimal column width, i.e. maximum of all cell widths */
gint           eel_clist_optimal_column_width     (EelCList             *clist,
						   gint                  column);

/* set the pixel width of a column; this is a necessary step in
 * creating a CList because otherwise the column width is chozen from
 * the width of the column title, which will never be right
 */
void           eel_clist_set_column_width         (EelCList             *clist,
						   gint                  column,
						   gint                  width);

/* set column minimum/maximum width. min/max_width < 0 => no restriction */
void           eel_clist_set_column_min_width     (EelCList             *clist,
						   gint                  column,
						   gint                  min_width);
void           eel_clist_set_column_max_width     (EelCList             *clist,
						   gint                  column,
						   gint                  max_width);

/* change the height of the rows, the default (height=0) is
 * the hight of the current font.
 */
void           eel_clist_set_row_height           (EelCList             *clist,
						   guint                 height);

/* scroll the viewing area of the list to the given column and row;
 * row_align and col_align are between 0-1 representing the location the
 * row should appear on the screnn, 0.0 being top or left, 1.0 being
 * bottom or right; if row or column is -1 then then there is no change
 */
void           eel_clist_moveto                   (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   gfloat                row_align,
						   gfloat                col_align);

/* returns whether the row is visible */
GtkVisibility  eel_clist_row_is_visible           (EelCList             *clist,
						   gint                  row);

/* returns the cell type */
EelCellType    eel_clist_get_cell_type            (EelCList             *clist,
						   gint                  row,
						   gint                  column);

/* sets a given cell's text, replacing its current contents */
void           eel_clist_set_text                 (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   const gchar          *text);

/* for the "get" functions, any of the return pointer can be
 * NULL if you are not interested
 */
gint           eel_clist_get_text                 (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   gchar               **text);

/* sets a given cell's pixbuf, replacing its current contents */
void           eel_clist_set_pixbuf               (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   GdkPixbuf            *pixbuf);
gint           eel_clist_get_pixbuf               (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   GdkPixbuf           **pixbuf);

/* sets a given cell's pixbuf and text, replacing its current contents */
void           eel_clist_set_pixtext              (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   const gchar          *text,
						   guint8                spacing,
						   GdkPixbuf            *pixbuf);
gint           eel_clist_get_pixtext              (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   gchar               **text,
						   guint8               *spacing,
						   GdkPixbuf           **pixbuf);

/* sets the foreground color of a row, the color must already
 * be allocated
 */
void           eel_clist_set_foreground           (EelCList             *clist,
						   gint                  row,
						   GdkColor             *color);

/* sets the background color of a row, the color must already
 * be allocated
 */
void           eel_clist_set_background           (EelCList             *clist,
						   gint                  row,
						   GdkColor             *color);

/* set / get cell styles */
void           eel_clist_set_cell_style           (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   GtkStyle             *style);
GtkStyle *     eel_clist_get_cell_style           (EelCList             *clist,
						   gint                  row,
						   gint                  column);
void           eel_clist_set_row_style            (EelCList             *clist,
						   gint                  row,
						   GtkStyle             *style);
GtkStyle *     eel_clist_get_row_style            (EelCList             *clist,
						   gint                  row);

/* this sets a horizontal and vertical shift for drawing
 * the contents of a cell; it can be positive or negitive;
 * this is particulary useful for indenting items in a column
 */
void           eel_clist_set_shift                (EelCList             *clist,
						   gint                  row,
						   gint                  column,
						   gint                  vertical,
						   gint                  horizontal);

/* set/get selectable flag of a single row */
void           eel_clist_set_selectable           (EelCList             *clist,
						   gint                  row,
						   gboolean              selectable);
gboolean       eel_clist_get_selectable           (EelCList             *clist,
						   gint                  row);

/* prepend/append returns the index of the row you just added,
 * making it easier to append and modify a row
 */
gint           eel_clist_prepend                  (EelCList             *clist,
						   gchar                *text[]);
gint           eel_clist_append                   (EelCList             *clist,
						   gchar                *text[]);

/* inserts a row at index row and returns the row where it was
 * actually inserted (may be different from "row" in auto_sort mode)
 */
gint           eel_clist_insert                   (EelCList             *clist,
						   gint                  row,
						   gchar                *text[]);

/* removes row at index row */
void           eel_clist_remove                   (EelCList             *clist,
						   gint                  row);

/* sets a arbitrary data pointer for a given row */
void           eel_clist_set_row_data             (EelCList             *clist,
						   gint                  row,
						   gpointer              data);

/* sets a data pointer for a given row with destroy notification */
void           eel_clist_set_row_data_full        (EelCList             *clist,
						   gint                  row,
						   gpointer              data,
						   GtkDestroyNotify      destroy);

/* returns the data set for a row */
gpointer       eel_clist_get_row_data             (EelCList             *clist,
						   gint                  row);

/* givin a data pointer, find the first (and hopefully only!)
 * row that points to that data, or -1 if none do
 */
gint           eel_clist_find_row_from_data       (EelCList             *clist,
						   gpointer              data);

/* force selection of a row */
void           eel_clist_select_row               (EelCList             *clist,
						   gint                  row,
						   gint                  column);

/* force unselection of a row */
void           eel_clist_unselect_row             (EelCList             *clist,
						   gint                  row,
						   gint                  column);

/* undo the last select/unselect operation */
void           eel_clist_undo_selection           (EelCList             *clist);

/* clear the entire list -- this is much faster than removing
 * each item with eel_clist_remove
 */
void           eel_clist_clear                    (EelCList             *clist);

/* return the row column corresponding to the x and y coordinates,
 * the returned values are only valid if the x and y coordinates
 * are respectively to a window == clist->clist_window
 */
gint           eel_clist_get_selection_info       (EelCList             *clist,
						   gint                  x,
						   gint                  y,
						   gint                 *row,
						   gint                 *column);

/* in multiple or extended mode, select all rows */
void           eel_clist_select_all               (EelCList             *clist);

/* in all modes except browse mode, deselect all rows */
void           eel_clist_unselect_all             (EelCList             *clist);

/* swap the position of two rows */
void           eel_clist_swap_rows                (EelCList             *clist,
						   gint                  row1,
						   gint                  row2);

/* move row from source_row position to dest_row position */
void           eel_clist_row_move                 (EelCList             *clist,
						   gint                  source_row,
						   gint                  dest_row);

/* sets a compare function different to the default */
void           eel_clist_set_compare_func         (EelCList             *clist,
						   EelCListCompareFunc   cmp_func);

/* the column to sort by */
void           eel_clist_set_sort_column          (EelCList             *clist,
						   gint                  column);

/* how to sort : ascending or descending */
void           eel_clist_set_sort_type            (EelCList             *clist,
						   GtkSortType           sort_type);

/* sort the list with the current compare function */
void           eel_clist_sort                     (EelCList             *clist);

/* Automatically sort upon insertion */
void           eel_clist_set_auto_sort            (EelCList             *clist,
						   gboolean              auto_sort);
gboolean       eel_clist_check_unfrozen           (EelCList             *clist);


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* EEL_CLIST_H__ */
