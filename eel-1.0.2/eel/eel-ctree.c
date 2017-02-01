/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball, Josh MacDonald, 
 * Copyright (C) 1997-1998 Jay Painter <jpaint@serv.net><jpaint@gimp.org>  
 *
 * EelCTree widget for GTK+
 * Copyright (C) 1998 Lars Hamann and Stefan Jeske
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


#include <config.h>
#include "eel-ctree.h"

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gdk/gdkx.h>
#include <gtk/gtkbindings.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtkmain.h>
#include <eel/eel-gdk-pixbuf-extensions.h>
#include <eel/eel-gdk-font-extensions.h>
#include <eel/eel-graphic-effects.h>
#include <eel/eel-gtk-extensions.h>
#include <stdlib.h>
#include <string.h>

#define PM_SIZE                    8
#define TAB_SIZE                   (PM_SIZE + 6)
#define CELL_SPACING               1
#define CLIST_OPTIMUM_SIZE         64
#define COLUMN_INSET               3
#define DRAG_WIDTH                 6

#define ROW_TOP_YPIXEL(clist, row) (((clist)->row_height * (row)) + \
				    (((row) + 1) * CELL_SPACING) + \
				    (clist)->voffset)
#define ROW_FROM_YPIXEL(clist, y)  (((y) - (clist)->voffset) / \
                                    ((clist)->row_height + CELL_SPACING))
#define COLUMN_LEFT_XPIXEL(clist, col)  ((clist)->column[(col)].area.x \
                                    + (clist)->hoffset)
#define COLUMN_LEFT(clist, column) ((clist)->column[(column)].area.x)

static inline gint
COLUMN_FROM_XPIXEL (EelCList * clist,
		    gint x)
{
  gint i, cx;

  for (i = 0; i < clist->columns; i++)
    if (clist->column[i].visible)
      {
	cx = clist->column[i].area.x + clist->hoffset;

	if (x >= (cx - (COLUMN_INSET + CELL_SPACING)) &&
	    x <= (cx + clist->column[i].area.width + COLUMN_INSET))
	  return i;
      }

  /* no match */
  return -1;
}

#define EEL_CLIST_CLASS_FW(_widget_) EEL_CLIST_CLASS (((GtkObject*) (_widget_))->klass)
#define CLIST_UNFROZEN(clist) eel_clist_check_unfrozen (clist)
#define CLIST_REFRESH(clist)    G_STMT_START { \
  if (CLIST_UNFROZEN (clist)) \
    EEL_CLIST_CLASS_FW (clist)->refresh (clist); \
} G_STMT_END


enum {
	ARG_0,
	ARG_N_COLUMNS,
	ARG_TREE_COLUMN,
	ARG_INDENT,
	ARG_SPACING,
	ARG_SHOW_STUB,
	ARG_LINE_STYLE
};


static void eel_ctree_class_init        (EelCTreeClass  *klass);
static void eel_ctree_init              (EelCTree       *ctree);
static void eel_ctree_set_arg		(GtkObject      *object,
					 GtkArg         *arg,
					 guint           arg_id);
static void eel_ctree_get_arg      	(GtkObject      *object,
					 GtkArg         *arg,
					 guint           arg_id);
static void eel_ctree_realize           (GtkWidget      *widget);
static void eel_ctree_unrealize         (GtkWidget      *widget);
static gint eel_ctree_button_press      (GtkWidget      *widget,
					 GdkEventButton *event);
static gboolean eel_ctree_event   	(GtkWidget      *widget,
					 GdkEvent 	*event,
					 gpointer 	user_data);
static void ctree_attach_styles         (EelCTree       *ctree,
					 EelCTreeNode   *node,
					 gpointer        data);
static void ctree_detach_styles         (EelCTree       *ctree,
					 EelCTreeNode   *node, 
					 gpointer        data);
static gint eel_ctree_draw_expander     (EelCTree       *ctree,
					      EelCTreeRow    *ctree_row,
					      GtkStyle       *style,
					      GdkRectangle   *clip_rectangle,
					      gint            x);
static gint eel_ctree_draw_lines        (EelCTree       *ctree,
					 EelCTreeRow    *ctree_row,
					 gint            row,
					 gint            column,
					 gint            state,
					 GdkRectangle   *clip_rectangle,
					 GdkRectangle   *cell_rectangle,
					 GdkRectangle   *crect,
					 GdkRectangle   *area,
					 GtkStyle       *style);
static void draw_row                    (EelCList       *clist,
					 GdkRectangle   *area,
					 gint            row,
					 EelCListRow    *clist_row);
static void draw_drag_highlight         (EelCList        *clist,
					 EelCListRow     *dest_row,
					 gint             dest_row_number,
					 EelCListDragPos  drag_pos);
static void tree_draw_node              (EelCTree      *ctree,
					 EelCTreeNode  *node);
static gboolean set_cell_contents       (EelCList      *clist,
					 EelCListRow   *clist_row,
					 gint           column,
					 EelCellType    type,
					 const gchar   *text,
					 guint8         spacing,
					 GdkPixbuf     *pixbuf);
static void set_node_info               (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 const gchar   *text,
					 guint8         spacing,
					 GdkPixbuf     *pixbuf_closed,
					 GdkPixbuf     *pixbuf_opened,
					 gboolean       is_leaf,
					 gboolean       expanded);
static EelCTreeRow *row_new             (EelCTree      *ctree);
static void row_delete                  (EelCTree      *ctree,
				 	 EelCTreeRow   *ctree_row);
static void tree_delete                 (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
					 gpointer       data);
static void tree_delete_row             (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
					 gpointer       data);
static void real_clear                  (EelCList      *clist);
static void tree_update_level           (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
					 gpointer       data);
static void tree_select                 (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
					 gpointer       data);
static void tree_unselect               (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
				         gpointer       data);
static void real_select_all             (EelCList      *clist);
static void real_unselect_all           (EelCList      *clist);
static void tree_expand                 (EelCTree      *ctree, 
					 EelCTreeNode  *node,
					 gpointer       data);
static void tree_collapse               (EelCTree      *ctree, 
					 EelCTreeNode  *node,
					 gpointer       data);
static void tree_collapse_to_depth      (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
					 gint           depth);
static void tree_toggle_expansion       (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 gpointer       data);
static void change_focus_row_expansion  (EelCTree      *ctree,
				         EelCTreeExpansionType expansion);
static void real_select_row             (EelCList      *clist,
					 gint           row,
					 gint           column,
					 GdkEvent      *event);
static void real_unselect_row           (EelCList      *clist,
					 gint           row,
					 gint           column,
					 GdkEvent      *event);
static void real_tree_select            (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 gint           column);
static void real_tree_unselect          (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 gint           column);
static void real_tree_expand            (EelCTree      *ctree,
					 EelCTreeNode  *node);
static void real_tree_collapse          (EelCTree      *ctree,
					 EelCTreeNode  *node);
static void real_tree_move              (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 EelCTreeNode  *new_parent, 
					 EelCTreeNode  *new_sibling);
static void real_row_move               (EelCList      *clist,
					 gint           source_row,
					 gint           dest_row);
static void real_tree_activate_row	(EelCTree	    *ctree,
					 EelCTreeNode  *node,
					 gint		     column);
static void eel_ctree_link              (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 EelCTreeNode  *parent,
					 EelCTreeNode  *sibling,
					 gboolean       update_focus_row);
static void eel_ctree_unlink            (EelCTree      *ctree, 
					 EelCTreeNode  *node,
					 gboolean       update_focus_row);
static EelCTreeNode * eel_ctree_last_visible (EelCTree     *ctree,
					      EelCTreeNode *node);
static gboolean ctree_is_hot_spot       (EelCTree      *ctree, 
					 EelCTreeNode  *node,
					 gint           row, 
					 gint           x, 
					 gint           y);
static void tree_sort                   (EelCTree      *ctree,
					 EelCTreeNode  *node,
					 gpointer       data);
static void fake_unselect_all           (EelCList      *clist,
					 gint           row);
static GList * selection_find           (EelCList      *clist,
					 gint           row_number,
					 GList         *row_list_element);
static void resync_selection            (EelCList      *clist,
					 GdkEvent      *event);
static void real_undo_selection         (EelCList      *clist);
static void select_row_recursive        (EelCTree      *ctree, 
					 EelCTreeNode  *node, 
					 gpointer       data);
static gint real_insert_row             (EelCList      *clist,
					 gint           row,
					 gchar         *text[]);
static void real_remove_row             (EelCList      *clist,
					 gint           row);
static void real_sort_list              (EelCList      *clist);
static void cell_size_request           (EelCList       *clist,
					 EelCListRow    *clist_row,
					 gint            column,
					 GtkRequisition *requisition);
static void column_auto_resize          (EelCList       *clist,
					 EelCListRow    *clist_row,
					 gint            column,
					 gint            old_width);
static void auto_resize_columns         (EelCList       *clist);


static gboolean check_drag               (EelCTree         *ctree,
			                  EelCTreeNode     *drag_source,
					  EelCTreeNode     *drag_target,
					  EelCListDragPos   insert_pos);
static void eel_ctree_drag_begin         (GtkWidget        *widget,
					  GdkDragContext   *context);
static gint eel_ctree_drag_motion        (GtkWidget        *widget,
					  GdkDragContext   *context,
					  gint              x,
					  gint              y,
					  guint             time);
static void eel_ctree_drag_data_received (GtkWidget        *widget,
					  GdkDragContext   *context,
					  gint              x,
					  gint              y,
					  GtkSelectionData *selection_data,
					  guint             info,
					  guint32           time);
static void remove_grab                  (EelCList         *clist);
static void drag_dest_cell               (EelCList         *clist,
					  gint              x,
					  gint              y,
					  EelCListDestInfo *dest_info);


enum
{
	TREE_SELECT_ROW,
	TREE_UNSELECT_ROW,
	TREE_EXPAND,
	TREE_COLLAPSE,
	TREE_MOVE,
	CHANGE_FOCUS_ROW_EXPANSION,
	TREE_ACTIVATE_ROW,
	LAST_SIGNAL
};

static EelCListClass *parent_class = NULL;
static GtkContainerClass *container_class = NULL;
static guint ctree_signals[LAST_SIGNAL] = {0};


GtkType
eel_ctree_get_type (void)
{
  static GtkType ctree_type = 0;

  if (!ctree_type)
    {
      static const GtkTypeInfo ctree_info =
      {
	"EelCTree",
	sizeof (EelCTree),
	sizeof (EelCTreeClass),
	(GtkClassInitFunc) eel_ctree_class_init,
	(GtkObjectInitFunc) eel_ctree_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      ctree_type = gtk_type_unique (EEL_TYPE_LIST, &ctree_info);
    }

  return ctree_type;
}

static void
eel_ctree_class_init (EelCTreeClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	EelCListClass *clist_class;
	GtkBindingSet *binding_set;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	container_class = (GtkContainerClass *) klass;
	clist_class = (EelCListClass *) klass;

	parent_class = gtk_type_class (EEL_TYPE_LIST);
	container_class = gtk_type_class (GTK_TYPE_CONTAINER);

	gtk_object_add_arg_type ("EelCTree::n_columns",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE | GTK_ARG_CONSTRUCT_ONLY,
				 ARG_N_COLUMNS);
	gtk_object_add_arg_type ("EelCTree::tree_column",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE | GTK_ARG_CONSTRUCT_ONLY,
				 ARG_TREE_COLUMN);
	gtk_object_add_arg_type ("EelCTree::indent",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_INDENT);
	gtk_object_add_arg_type ("EelCTree::spacing",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_SPACING);
	gtk_object_add_arg_type ("EelCTree::show_stub",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_SHOW_STUB);
	object_class->set_arg = eel_ctree_set_arg;
	object_class->get_arg = eel_ctree_get_arg;

	ctree_signals[TREE_SELECT_ROW] =
    gtk_signal_new ("tree_select_row",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass, tree_select_row),
		    gtk_marshal_NONE__POINTER_INT,
		    GTK_TYPE_NONE, 2, GTK_TYPE_CTREE_NODE, GTK_TYPE_INT);
  ctree_signals[TREE_UNSELECT_ROW] =
    gtk_signal_new ("tree_unselect_row",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass, tree_unselect_row),
		    gtk_marshal_NONE__POINTER_INT,
		    GTK_TYPE_NONE, 2, GTK_TYPE_CTREE_NODE, GTK_TYPE_INT);
  ctree_signals[TREE_EXPAND] =
    gtk_signal_new ("tree_expand",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass, tree_expand),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1, GTK_TYPE_CTREE_NODE);
  ctree_signals[TREE_COLLAPSE] =
    gtk_signal_new ("tree_collapse",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass, tree_collapse),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1, GTK_TYPE_CTREE_NODE);
  ctree_signals[TREE_MOVE] =
    gtk_signal_new ("tree_move",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass, tree_move),
		    gtk_marshal_NONE__POINTER_POINTER_POINTER,
		    GTK_TYPE_NONE, 3, GTK_TYPE_CTREE_NODE,
		    GTK_TYPE_CTREE_NODE, GTK_TYPE_CTREE_NODE);
  ctree_signals[CHANGE_FOCUS_ROW_EXPANSION] =
    gtk_signal_new ("change_focus_row_expansion",
		    GTK_RUN_LAST | GTK_RUN_ACTION,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass,
				       change_focus_row_expansion),
		    gtk_marshal_NONE__ENUM,
		    GTK_TYPE_NONE, 1, GTK_TYPE_CTREE_EXPANSION_TYPE);
  ctree_signals[TREE_ACTIVATE_ROW] =
    gtk_signal_new ("tree_activate_row",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (EelCTreeClass, tree_activate_row),
		    gtk_marshal_NONE__POINTER_INT,
		    GTK_TYPE_NONE, 2, GTK_TYPE_CTREE_NODE, GTK_TYPE_INT);

  gtk_object_class_add_signals (object_class, ctree_signals, LAST_SIGNAL);

  widget_class->realize = eel_ctree_realize;
  widget_class->unrealize = eel_ctree_unrealize;
  widget_class->button_press_event = eel_ctree_button_press;
  
  widget_class->drag_begin = eel_ctree_drag_begin;
  widget_class->drag_motion = eel_ctree_drag_motion;
  widget_class->drag_data_received = eel_ctree_drag_data_received;

  clist_class->select_row = real_select_row;
  clist_class->unselect_row = real_unselect_row;
  clist_class->row_move = real_row_move;
  clist_class->undo_selection = real_undo_selection;
  clist_class->resync_selection = resync_selection;
  clist_class->selection_find = selection_find;
  clist_class->click_column = NULL;
  clist_class->draw_row = draw_row;
  clist_class->draw_drag_highlight = draw_drag_highlight;
  clist_class->clear = real_clear;
  clist_class->select_all = real_select_all;
  clist_class->unselect_all = real_unselect_all;
  clist_class->fake_unselect_all = fake_unselect_all;
  clist_class->insert_row = real_insert_row;
  clist_class->remove_row = real_remove_row;
  clist_class->sort_list = real_sort_list;
  clist_class->set_cell_contents = set_cell_contents;
  clist_class->cell_size_request = cell_size_request;

  klass->tree_select_row = real_tree_select;
  klass->tree_unselect_row = real_tree_unselect;
  klass->tree_expand = real_tree_expand;
  klass->tree_collapse = real_tree_collapse;
  klass->tree_move = real_tree_move;
  klass->change_focus_row_expansion = change_focus_row_expansion;
  klass->tree_activate_row = real_tree_activate_row;

  binding_set = gtk_binding_set_by_class (klass);
  gtk_binding_entry_add_signal (binding_set,
				'+', GDK_SHIFT_MASK,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM, EEL_CTREE_EXPANSION_EXPAND);
  gtk_binding_entry_add_signal (binding_set,
				GDK_KP_Add, 0,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM, EEL_CTREE_EXPANSION_EXPAND);
  gtk_binding_entry_add_signal (binding_set,
				GDK_KP_Add, GDK_CONTROL_MASK,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM,
				EEL_CTREE_EXPANSION_EXPAND_RECURSIVE);
  gtk_binding_entry_add_signal (binding_set,
				'-', 0,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM, EEL_CTREE_EXPANSION_COLLAPSE);
  gtk_binding_entry_add_signal (binding_set,
				GDK_KP_Subtract, 0,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM, EEL_CTREE_EXPANSION_COLLAPSE);
  gtk_binding_entry_add_signal (binding_set,
				GDK_KP_Subtract, GDK_CONTROL_MASK,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM,
				EEL_CTREE_EXPANSION_COLLAPSE_RECURSIVE);
  gtk_binding_entry_add_signal (binding_set,
				'=', 0,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM, EEL_CTREE_EXPANSION_TOGGLE);
  gtk_binding_entry_add_signal (binding_set,
				GDK_KP_Multiply, 0,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM, EEL_CTREE_EXPANSION_TOGGLE);
  gtk_binding_entry_add_signal (binding_set,
				GDK_KP_Multiply, GDK_CONTROL_MASK,
				"change_focus_row_expansion", 1,
				GTK_TYPE_ENUM,
				EEL_CTREE_EXPANSION_TOGGLE_RECURSIVE);	
}

static void
eel_ctree_set_arg (GtkObject      *object,
		   GtkArg         *arg,
		   guint           arg_id)
{
  EelCTree *ctree;

  ctree = EEL_CTREE (object);

  switch (arg_id)
    {
    case ARG_N_COLUMNS: /* construct-only arg, only set when !GTK_CONSTRUCTED */
      if (ctree->tree_column)
	eel_ctree_construct (ctree,
			     MAX (1, GTK_VALUE_UINT (*arg)),
			     ctree->tree_column, NULL);
      else
	EEL_CLIST (ctree)->columns = MAX (1, GTK_VALUE_UINT (*arg));
      break;
    case ARG_TREE_COLUMN: /* construct-only arg, only set when !GTK_CONSTRUCTED */
      if (EEL_CLIST (ctree)->columns)
	eel_ctree_construct (ctree,
			     EEL_CLIST (ctree)->columns,
			     MAX (1, GTK_VALUE_UINT (*arg)),
			     NULL);
      else
	ctree->tree_column = MAX (1, GTK_VALUE_UINT (*arg));
      break;
    case ARG_INDENT:
      eel_ctree_set_indent (ctree, GTK_VALUE_UINT (*arg));
      break;
    case ARG_SPACING:
      eel_ctree_set_spacing (ctree, GTK_VALUE_UINT (*arg));
      break;
    case ARG_SHOW_STUB:
      eel_ctree_set_show_stub (ctree, GTK_VALUE_BOOL (*arg));
      break;
    case ARG_LINE_STYLE:
      eel_ctree_set_line_style (ctree, GTK_VALUE_ENUM (*arg));
      break;
    default:
      break;
    }
}

static void
eel_ctree_get_arg (GtkObject      *object,
		   GtkArg         *arg,
		   guint           arg_id)
{
  EelCTree *ctree;

  ctree = EEL_CTREE (object);

  switch (arg_id)
    {
    case ARG_N_COLUMNS:
      GTK_VALUE_UINT (*arg) = EEL_CLIST (ctree)->columns;
      break;
    case ARG_TREE_COLUMN:
      GTK_VALUE_UINT (*arg) = ctree->tree_column;
      break;
    case ARG_INDENT:
      GTK_VALUE_UINT (*arg) = ctree->tree_indent;
      break;
    case ARG_SPACING:
      GTK_VALUE_UINT (*arg) = ctree->tree_spacing;
      break;
    case ARG_SHOW_STUB:
      GTK_VALUE_BOOL (*arg) = ctree->show_stub;
      break;
    case ARG_LINE_STYLE:
      GTK_VALUE_ENUM (*arg) = ctree->line_style;
      break;
    default:
      arg->type = GTK_TYPE_INVALID;
      break;
    }
}

static void
eel_ctree_init (EelCTree *ctree)
{
	EelCList *clist;

	EEL_CLIST_SET_FLAG (ctree, CLIST_DRAW_DRAG_RECT);
	EEL_CLIST_SET_FLAG (ctree, CLIST_DRAW_DRAG_LINE);

	clist = EEL_CLIST (ctree);

	ctree->tree_indent    	= 20;
	ctree->tree_spacing   	= 5;
	ctree->tree_column    	= 0;
	ctree->line_style     	= EEL_CTREE_LINES_NONE;
	ctree->drag_compare   	= NULL;
	ctree->show_stub      	= TRUE;
	ctree->prelight_node	= NULL;
	
	clist->button_actions[0] |= EEL_BUTTON_EXPANDS;

	/* Some random GNOME luser wants trees to look `normal' */
	eel_list_set_alternate_row_colors (EEL_LIST (ctree), FALSE);
	eel_list_set_background_color_offsets (EEL_LIST (ctree),
						    G_STRUCT_OFFSET (GtkStyle,
								     base[GTK_STATE_NORMAL]), -1);

	gtk_signal_connect (GTK_OBJECT (ctree), "event",
			    GTK_SIGNAL_FUNC (eel_ctree_event), ctree);
}

static void
ctree_attach_styles (EelCTree     *ctree,
		     EelCTreeNode *node,
		     gpointer      data)
{
  EelCList *clist;
  gint i;

  clist = EEL_CLIST (ctree);

  if (EEL_CTREE_ROW (node)->row.style)
    EEL_CTREE_ROW (node)->row.style =
      gtk_style_attach (EEL_CTREE_ROW (node)->row.style, clist->clist_window);

  if (EEL_CTREE_ROW (node)->row.fg_set || EEL_CTREE_ROW (node)->row.bg_set)
    {
      GdkColormap *colormap;

      colormap = gtk_widget_get_colormap (GTK_WIDGET (ctree));
      if (EEL_CTREE_ROW (node)->row.fg_set)
	gdk_color_alloc (colormap, &(EEL_CTREE_ROW (node)->row.foreground));
      if (EEL_CTREE_ROW (node)->row.bg_set)
	gdk_color_alloc (colormap, &(EEL_CTREE_ROW (node)->row.background));
    }

  for (i = 0; i < clist->columns; i++)
    if  (EEL_CTREE_ROW (node)->row.cell[i].style)
      EEL_CTREE_ROW (node)->row.cell[i].style =
	gtk_style_attach (EEL_CTREE_ROW (node)->row.cell[i].style,
			  clist->clist_window);
}

static void
ctree_detach_styles (EelCTree     *ctree,
		     EelCTreeNode *node,
		     gpointer      data)
{
  EelCList *clist;
  gint i;

  clist = EEL_CLIST (ctree);

  if (EEL_CTREE_ROW (node)->row.style)
    gtk_style_detach (EEL_CTREE_ROW (node)->row.style);
  for (i = 0; i < clist->columns; i++)
    if  (EEL_CTREE_ROW (node)->row.cell[i].style)
      gtk_style_detach (EEL_CTREE_ROW (node)->row.cell[i].style);
}

static void
eel_ctree_realize (GtkWidget *widget)
{
  EelCTree *ctree;
  EelCList *clist;
  GdkGCValues values;
  EelCTreeNode *node;
  EelCTreeNode *child;
  gint i;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (EEL_IS_CTREE (widget));

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  ctree = EEL_CTREE (widget);
  clist = EEL_CLIST (widget);

  node = EEL_CTREE_NODE (clist->row_list);
  for (i = 0; i < clist->rows; i++)
    {
      if (!EEL_CTREE_ROW (node)->is_leaf && !EEL_CTREE_ROW (node)->expanded)
	for (child = EEL_CTREE_ROW (node)->children; 
	     child != NULL;
	     child = EEL_CTREE_ROW (child)->sibling)
	  eel_ctree_pre_recursive (ctree, child, ctree_attach_styles, NULL);
      node = EEL_CTREE_NODE_NEXT (node);
    }

  values.foreground = widget->style->fg[GTK_STATE_NORMAL];
  values.background = widget->style->base[GTK_STATE_NORMAL];
  values.subwindow_mode = GDK_INCLUDE_INFERIORS;
  values.line_style = GDK_LINE_SOLID;
  ctree->lines_gc = gdk_gc_new_with_values (EEL_CLIST(widget)->clist_window, 
					    &values,
					    GDK_GC_FOREGROUND |
					    GDK_GC_BACKGROUND |
					    GDK_GC_SUBWINDOW |
					    GDK_GC_LINE_STYLE);

  if (ctree->line_style == EEL_CTREE_LINES_DOTTED)
    {
      gdk_gc_set_line_attributes (ctree->lines_gc, 1, 
				  GDK_LINE_ON_OFF_DASH, None, None);
      gdk_gc_set_dashes (ctree->lines_gc, 0, "\1\1", 2);
    }
}

#define	ROW_ELEMENT(clist, row)	(((row) == (clist)->rows - 1) ? \
				 (clist)->row_list_end : \
				 g_list_nth ((clist)->row_list, (row)))

static gint 
eel_ctree_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventMotion *motion;
	int press_row, press_column, row;
	EelCTree *tree;
	EelCTreeNode *node, *old_node;
	EelCTreeRow *ctree_row;
	EelCList *clist;
	gint x, y;
	GdkModifierType button;

	tree = EEL_CTREE (widget);
	clist = EEL_CLIST (widget);

	/* Do prelighting */ 
	if (event->type == GDK_MOTION_NOTIFY) {
		motion = (GdkEventMotion *) event;

		/* Get node that we are over */
		row = eel_clist_get_selection_info (clist, motion->x, motion->y, &press_row, &press_column);
		if (row <= 0) {
			return FALSE;
		}
		
		ctree_row = ROW_ELEMENT (clist, press_row)->data;
		if (ctree_row == NULL) {
			return FALSE;
		}
		
		node = eel_ctree_find_node_ptr (tree, ctree_row);
		if (node == NULL) {
			return FALSE;
		}

		/* Cancel prelighting if we have a button pressed */
		gdk_window_get_pointer (widget->window, &x, &y, &button);
		if ((button & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK | GDK_BUTTON4_MASK | GDK_BUTTON5_MASK)) != 0) {
			if (eel_ctree_is_hot_spot (tree, motion->x, motion->y)) {
				/* Handle moving in and out of hotspot while mouse is down */
				if (!ctree_row->in_hotspot) {
					ctree_row->in_hotspot = TRUE;
					eel_ctree_draw_node (tree, node);
				}
			} else {
				if (ctree_row->in_hotspot) {
					ctree_row->in_hotspot = FALSE;
					eel_ctree_draw_node (tree, node);
				}
			}

			/* Remove prelighting */
			if (tree->prelight_node != NULL) {
				old_node = tree->prelight_node;
				tree->prelight_node = NULL;
				eel_ctree_draw_node (tree, old_node);
			}
			return FALSE;
		}
						
		if (eel_ctree_is_hot_spot (tree, motion->x, motion->y)) {
			if (node != tree->prelight_node) {
				/* Redraw old prelit node and save and draw new highlight */
				old_node = tree->prelight_node;
				tree->prelight_node = node;
				eel_ctree_draw_node (tree, old_node);
				eel_ctree_draw_node (tree, tree->prelight_node);
			}				
		} else if (tree->prelight_node != NULL) {
			/* End prelighting of last expander */
			old_node = tree->prelight_node;
			tree->prelight_node = NULL;
			eel_ctree_draw_node (tree, old_node);
		}
	}
	
	return FALSE;
}

static void
eel_ctree_unrealize (GtkWidget *widget)
{
  EelCTree *ctree;
  EelCList *clist;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (EEL_IS_CTREE (widget));

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);

  ctree = EEL_CTREE (widget);
  clist = EEL_CLIST (widget);

  if (GTK_WIDGET_REALIZED (widget))
    {
      EelCTreeNode *node;
      EelCTreeNode *child;
      gint i;

      node = EEL_CTREE_NODE (clist->row_list);
      for (i = 0; i < clist->rows; i++)
	{
	  if (!EEL_CTREE_ROW (node)->is_leaf &&
	      !EEL_CTREE_ROW (node)->expanded)
	    for (child = EEL_CTREE_ROW (node)->children; 
		 child != NULL;
		 child = EEL_CTREE_ROW (child)->sibling)
	      eel_ctree_pre_recursive(ctree, child, ctree_detach_styles, NULL);
	  node = EEL_CTREE_NODE_NEXT (node);
	}
    }

  gdk_gc_destroy (ctree->lines_gc);
}

static gint
eel_ctree_button_press (GtkWidget *widget, GdkEventButton *event)
{
	EelCTree *ctree;
	EelCList *clist;
	gint button_actions;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (EEL_IS_CTREE (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	ctree = EEL_CTREE (widget);
	clist = EEL_CLIST (widget);

	button_actions = clist->button_actions[event->button - 1];

	if (button_actions == EEL_BUTTON_IGNORED) {
    		return FALSE;
    	}

	if (event->window == clist->clist_window)
	{
		EelCTreeNode *work;
		gint x;
		gint y;
		gint row;
		gint column;

		x = event->x;
		y = event->y;

		if (!eel_clist_get_selection_info (clist, x, y, &row, &column)) {
			return FALSE;
		}

      		work = EEL_CTREE_NODE (g_list_nth (clist->row_list, row));
	  
		if (button_actions & EEL_BUTTON_EXPANDS && 
		    (!EEL_CTREE_ROW (work)->is_leaf  &&
		     ctree_is_hot_spot (ctree, work, row, x, y)))
		{
	  		if (EEL_CTREE_ROW (work)->expanded) {
	    			eel_ctree_collapse (ctree, work);
	    		} else {
	    			eel_ctree_expand (ctree, work);
	    		}
	  		return FALSE;
		} else if (event->type == GDK_2BUTTON_PRESS) {
			/* double-click on a row = "activate" */
			gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_ACTIVATE_ROW],
					 work, column);
		}
    	}
    	
	return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, event);
}

static void
draw_drag_highlight (EelCList        *clist,
		     EelCListRow     *dest_row,
		     gint             dest_row_number,
		     EelCListDragPos  drag_pos)
{
  EelCTree *ctree;
  GdkPoint points[4];
  gint level;
  gint i;
  gint y = 0;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  ctree = EEL_CTREE (clist);

  level = ((EelCTreeRow *)(dest_row))->level;

  y = ROW_TOP_YPIXEL (clist, dest_row_number) - 1;

  switch (drag_pos)
    {
    case EEL_CLIST_DRAG_NONE:
      break;
    case EEL_CLIST_DRAG_AFTER:
      y += clist->row_height + 1;
    case EEL_CLIST_DRAG_BEFORE:
      
      if (clist->column[ctree->tree_column].visible)
	switch (clist->column[ctree->tree_column].justification)
	  {
	  case GTK_JUSTIFY_CENTER:
	  case GTK_JUSTIFY_FILL:
	  case GTK_JUSTIFY_LEFT:
	    if (ctree->tree_column > 0)
	      gdk_draw_line (clist->clist_window, clist->xor_gc, 
			     COLUMN_LEFT_XPIXEL(clist, 0), y,
			     COLUMN_LEFT_XPIXEL(clist, ctree->tree_column - 1)+
			     clist->column[ctree->tree_column - 1].area.width,
			     y);

	    gdk_draw_line (clist->clist_window, clist->xor_gc, 
			   COLUMN_LEFT_XPIXEL(clist, ctree->tree_column) + 
			   ctree->tree_indent * level -
			   (ctree->tree_indent - PM_SIZE) / 2, y,
			   GTK_WIDGET (ctree)->allocation.width, y);
	    break;
	  case GTK_JUSTIFY_RIGHT:
	    if (ctree->tree_column < clist->columns - 1)
	      gdk_draw_line (clist->clist_window, clist->xor_gc, 
			     COLUMN_LEFT_XPIXEL(clist, ctree->tree_column + 1),
			     y,
			     COLUMN_LEFT_XPIXEL(clist, clist->columns - 1) +
			     clist->column[clist->columns - 1].area.width, y);
      
	    gdk_draw_line (clist->clist_window, clist->xor_gc, 
			   0, y, COLUMN_LEFT_XPIXEL(clist, ctree->tree_column)
			   + clist->column[ctree->tree_column].area.width -
			   ctree->tree_indent * level +
			   (ctree->tree_indent - PM_SIZE) / 2, y);
	    break;
	  }
      else
	gdk_draw_line (clist->clist_window, clist->xor_gc, 
		       0, y, clist->clist_window_width, y);
      break;
    case EEL_CLIST_DRAG_INTO:
      y = ROW_TOP_YPIXEL (clist, dest_row_number) + clist->row_height;

      if (clist->column[ctree->tree_column].visible)
	switch (clist->column[ctree->tree_column].justification)
	  {
	  case GTK_JUSTIFY_CENTER:
	  case GTK_JUSTIFY_FILL:
	  case GTK_JUSTIFY_LEFT:
	    points[0].x =  COLUMN_LEFT_XPIXEL(clist, ctree->tree_column) + 
	      ctree->tree_indent * level - (ctree->tree_indent - PM_SIZE) / 2;
	    points[0].y = y;
	    points[3].x = points[0].x;
	    points[3].y = y - clist->row_height - 1;
	    points[1].x = clist->clist_window_width - 1;
	    points[1].y = points[0].y;
	    points[2].x = points[1].x;
	    points[2].y = points[3].y;

	    for (i = 0; i < 3; i++)
	      gdk_draw_line (clist->clist_window, clist->xor_gc,
			     points[i].x, points[i].y,
			     points[i+1].x, points[i+1].y);

	    if (ctree->tree_column > 0)
	      {
		points[0].x = COLUMN_LEFT_XPIXEL(clist,
						 ctree->tree_column - 1) +
		  clist->column[ctree->tree_column - 1].area.width ;
		points[0].y = y;
		points[3].x = points[0].x;
		points[3].y = y - clist->row_height - 1;
		points[1].x = 0;
		points[1].y = points[0].y;
		points[2].x = 0;
		points[2].y = points[3].y;

		for (i = 0; i < 3; i++)
		  gdk_draw_line (clist->clist_window, clist->xor_gc,
				 points[i].x, points[i].y, points[i+1].x, 
				 points[i+1].y);
	      }
	    break;
	  case GTK_JUSTIFY_RIGHT:
	    points[0].x =  COLUMN_LEFT_XPIXEL(clist, ctree->tree_column) - 
	      ctree->tree_indent * level + (ctree->tree_indent - PM_SIZE) / 2 +
	      clist->column[ctree->tree_column].area.width;
	    points[0].y = y;
	    points[3].x = points[0].x;
	    points[3].y = y - clist->row_height - 1;
	    points[1].x = 0;
	    points[1].y = points[0].y;
	    points[2].x = 0;
	    points[2].y = points[3].y;

	    for (i = 0; i < 3; i++)
	      gdk_draw_line (clist->clist_window, clist->xor_gc,
			     points[i].x, points[i].y,
			     points[i+1].x, points[i+1].y);

	    if (ctree->tree_column < clist->columns - 1)
	      {
		points[0].x = COLUMN_LEFT_XPIXEL(clist, ctree->tree_column +1);
		points[0].y = y;
		points[3].x = points[0].x;
		points[3].y = y - clist->row_height - 1;
		points[1].x = clist->clist_window_width - 1;
		points[1].y = points[0].y;
		points[2].x = points[1].x;
		points[2].y = points[3].y;

		for (i = 0; i < 3; i++)
		  gdk_draw_line (clist->clist_window, clist->xor_gc,
				 points[i].x, points[i].y,
				 points[i+1].x, points[i+1].y);
	      }
	    break;
	  }
      else
	gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
			    0, y - clist->row_height,
			    clist->clist_window_width - 1, clist->row_height);
      break;
    }
}

static EelCTreeRow *
eel_ctree_row_at (EelCTree *ctree, int y)
{
	int row_index, column_index;

	y -= (GTK_CONTAINER (ctree)->border_width +
		GTK_WIDGET (ctree)->style->klass->ythickness +
		EEL_CLIST (ctree)->column_title_area.height);
	
	if (!eel_clist_get_selection_info (EEL_CLIST (ctree), 10, y, &row_index, &column_index)) {
		return NULL;
	}
	
	return g_list_nth (EEL_CLIST (ctree)->row_list, row_index)->data;
}


static void
get_cell_rectangle (EelCList *clist, int row_index, int column_index, GdkRectangle *result)
{
	result->x = clist->column[column_index].area.x + clist->hoffset;
	result->y = ROW_TOP_YPIXEL (clist, row_index);
	result->width = clist->column[column_index].area.width;
	result->height = clist->row_height;
}


void 
eel_ctree_set_prelight (EelCTree      *ctree,
			     int                 y)
{
	EelCList *clist;
	EelCTreeRow *row, *last_row;

	g_return_if_fail (ctree != NULL);
	g_return_if_fail (EEL_IS_CTREE (ctree));

	clist = EEL_CLIST (ctree);

	row = NULL;

	if (y >= 0) { 
		row = eel_ctree_row_at (ctree, y);
	}
	
	if (row != ctree->dnd_prelighted_row) {
		last_row = ctree->dnd_prelighted_row;
		ctree->dnd_prelighted_row = row;

		{
			GdkRectangle rect;
			int row_index;
			/* Redraw old cell */
			if (last_row != NULL) {
				row_index = g_list_index (clist->row_list, last_row);
				get_cell_rectangle (clist, row_index, 0, &rect);
				gtk_widget_draw (GTK_WIDGET (clist), &rect);			
			}

			/* Draw new cell */
			if (ctree->dnd_prelighted_row != NULL) {
				row_index = g_list_index (clist->row_list, ctree->dnd_prelighted_row);
				get_cell_rectangle (clist, row_index, 0, &rect);
				gtk_widget_draw (GTK_WIDGET (clist), &rect);
			}
		}
	}
}

static gint
eel_ctree_draw_expander (EelCTree *ctree, EelCTreeRow *ctree_row, GtkStyle *style,
			      GdkRectangle *clip_rectangle, gint x)
{
	EelCList *clist;
	GdkPoint points[3];
	gint justification_factor;
	gint y;
	EelCTreeNode *node;

	clist = EEL_CLIST (ctree);
	if (clist->column[ctree->tree_column].justification == GTK_JUSTIFY_RIGHT)
		justification_factor = -1;
	else
		justification_factor = 1;

	y = (clip_rectangle->y + (clip_rectangle->height - PM_SIZE) / 2 - (clip_rectangle->height + 1) % 2);

  	if (ctree_row->is_leaf) {
		return x + justification_factor * (PM_SIZE + 3);	  
	}

	gdk_gc_set_clip_rectangle (style->fg_gc[GTK_STATE_NORMAL], clip_rectangle);
	gdk_gc_set_clip_rectangle (style->base_gc[GTK_STATE_NORMAL], clip_rectangle);

	if (ctree_row->expanded)
	{
  		points[0].x = x;
  		points[0].y = y + (PM_SIZE + 2) / 6;
  		points[1].x = points[0].x + justification_factor * (PM_SIZE + 2);
  		points[1].y = points[0].y;
  		points[2].x = (points[0].x + justification_factor * (PM_SIZE + 2) / 2);
  		points[2].y = y + 2 * (PM_SIZE + 2) / 3;
	} else {
  		points[0].x = x + justification_factor * ((PM_SIZE + 2) / 6 + 2);
  		points[0].y = y - 1;
  		points[1].x = points[0].x;
  		points[1].y = points[0].y + (PM_SIZE + 2);
  		points[2].x = (points[0].x + justification_factor * (2 * (PM_SIZE + 2) / 3 - 1));
  		points[2].y = points[0].y + (PM_SIZE + 2) / 2;
	}

	gdk_draw_polygon (clist->clist_window, style->base_gc[GTK_STATE_NORMAL], TRUE, points, 3);
	if (ctree_row->mouse_down) {
			gdk_draw_polygon (clist->clist_window, style->fg_gc[GTK_STATE_NORMAL], !ctree_row->in_hotspot, points, 3);
	} else {
		node = eel_ctree_find_node_ptr (ctree, ctree_row);
		if (node != NULL) {
			if (node == ctree->prelight_node) {
				/* Draw prelight state */
				gdk_draw_polygon (clist->clist_window, style->fg_gc[GTK_STATE_NORMAL], FALSE, points, 3);
			} else {
				gdk_draw_polygon (clist->clist_window, style->fg_gc[GTK_STATE_NORMAL], TRUE, points, 3);
			}
		}
	}
	
	x += justification_factor * (PM_SIZE + 3);

	gdk_gc_set_clip_rectangle (style->fg_gc[GTK_STATE_NORMAL], NULL);
	gdk_gc_set_clip_rectangle (style->base_gc[GTK_STATE_NORMAL], NULL);

	return x;
}

static gint
eel_ctree_draw_lines (EelCTree     *ctree,
		      EelCTreeRow  *ctree_row,
		      gint          row,
		      gint          column,
		      gint          state,
		      GdkRectangle *clip_rectangle,
		      GdkRectangle *cell_rectangle,
		      GdkRectangle *crect,
		      GdkRectangle *area,
		      GtkStyle     *style)
{
  EelCList *clist;
  EelCTreeNode *node;
  EelCTreeNode *parent;
  GdkRectangle tree_rectangle;
  GdkRectangle tc_rectangle;
  GdkGC *bg_gc;
  gint offset;
  gint offset_x;
  gint offset_y;
  gint xcenter;
  gint ycenter;
  gint next_level;
  gint column_right;
  gint column_left;
  gint justify_right;
  gint justification_factor;
  
  clist = EEL_CLIST (ctree);
  ycenter = clip_rectangle->y + (clip_rectangle->height / 2);
  justify_right = (clist->column[column].justification == GTK_JUSTIFY_RIGHT);

  if (justify_right)
    {
      offset = (clip_rectangle->x + clip_rectangle->width - 1 -
		ctree->tree_indent * (ctree_row->level - 1));
      justification_factor = -1;
    }
  else
    {
      offset = clip_rectangle->x + ctree->tree_indent * (ctree_row->level - 1);
      justification_factor = 1;
    }

  switch (ctree->line_style)
    {
    case EEL_CTREE_LINES_NONE:
      break;
    case EEL_CTREE_LINES_TABBED:
      xcenter = offset + justification_factor * TAB_SIZE;

      column_right = (COLUMN_LEFT_XPIXEL (clist, ctree->tree_column) +
		      clist->column[ctree->tree_column].area.width +
		      COLUMN_INSET);
      column_left = (COLUMN_LEFT_XPIXEL (clist, ctree->tree_column) -
		     COLUMN_INSET - CELL_SPACING);

      if (area)
	{
	  tree_rectangle.y = crect->y;
	  tree_rectangle.height = crect->height;

	  if (justify_right)
	    {
	      tree_rectangle.x = xcenter;
	      tree_rectangle.width = column_right - xcenter;
	    }
	  else
	    {
	      tree_rectangle.x = column_left;
	      tree_rectangle.width = xcenter - column_left;
	    }

	  if (!gdk_rectangle_intersect (area, &tree_rectangle, &tc_rectangle))
	    {
	      offset += justification_factor * 3;
	      break;
	    }
	}

      gdk_gc_set_clip_rectangle (ctree->lines_gc, crect);

      next_level = ctree_row->level;

      if (!ctree_row->sibling || (ctree_row->children && ctree_row->expanded))
	{
	  node = eel_ctree_find_node_ptr (ctree, ctree_row);
	  if (EEL_CTREE_NODE_NEXT (node))
	    next_level = EEL_CTREE_ROW (EEL_CTREE_NODE_NEXT (node))->level;
	  else
	    next_level = 0;
	}

      if (ctree->tree_indent > 0)
	{
	  node = ctree_row->parent;
	  while (node)
	    {
	      xcenter -= (justification_factor * ctree->tree_indent);

	      if ((justify_right && xcenter < column_left) ||
		  (!justify_right && xcenter > column_right))
		{
		  node = EEL_CTREE_ROW (node)->parent;
		  continue;
		}

	      tree_rectangle.y = cell_rectangle->y;
	      tree_rectangle.height = cell_rectangle->height;
	      if (justify_right)
		{
		  tree_rectangle.x = MAX (xcenter - ctree->tree_indent + 1,
					  column_left);
		  tree_rectangle.width = MIN (xcenter - column_left,
					      ctree->tree_indent);
		}
	      else
		{
		  tree_rectangle.x = xcenter;
		  tree_rectangle.width = MIN (column_right - xcenter,
					      ctree->tree_indent);
		}

	      if (!area || gdk_rectangle_intersect (area, &tree_rectangle,
						    &tc_rectangle))
		{
		  eel_list_get_cell_style (EEL_LIST (clist),
						&EEL_CTREE_ROW (node)->row,
						state, row, column, NULL, NULL, &bg_gc, NULL);

		  if (bg_gc == clist->bg_gc)
		    gdk_gc_set_foreground
		      (clist->bg_gc, &EEL_CTREE_ROW (node)->row.background);

		  if (!area)
		    gdk_draw_rectangle (clist->clist_window, bg_gc, TRUE,
					tree_rectangle.x,
					tree_rectangle.y,
					tree_rectangle.width,
					tree_rectangle.height);
		  else 
		    gdk_draw_rectangle (clist->clist_window, bg_gc, TRUE,
					tc_rectangle.x,
					tc_rectangle.y,
					tc_rectangle.width,
					tc_rectangle.height);
		}
	      if (next_level > EEL_CTREE_ROW (node)->level)
		gdk_draw_line (clist->clist_window, ctree->lines_gc,
			       xcenter, crect->y,
			       xcenter, crect->y + crect->height);
	      else
		{
		  gint width;

		  offset_x = MIN (ctree->tree_indent, 2 * TAB_SIZE);
		  width = offset_x / 2 + offset_x % 2;

		  parent = EEL_CTREE_ROW (node)->parent;

		  tree_rectangle.y = ycenter;
		  tree_rectangle.height = (cell_rectangle->y - ycenter +
					   cell_rectangle->height);

		  if (justify_right)
		    {
		      tree_rectangle.x = MAX(xcenter + 1 - width, column_left);
		      tree_rectangle.width = MIN (xcenter + 1 - column_left,
						  width);
		    }
		  else
		    {
		      tree_rectangle.x = xcenter;
		      tree_rectangle.width = MIN (column_right - xcenter,
						  width);
		    }

		  if (!area ||
		      gdk_rectangle_intersect (area, &tree_rectangle,
					       &tc_rectangle))
		    {
		      if (parent)
			{
			  eel_list_get_cell_style (EEL_LIST (clist),
							&EEL_CTREE_ROW (parent)->row,
							state, row, column, NULL, NULL, &bg_gc, NULL);
			  if (bg_gc == clist->bg_gc)
			    gdk_gc_set_foreground
			      (clist->bg_gc,
			       &EEL_CTREE_ROW (parent)->row.background);
			}
		      else if (state == GTK_STATE_SELECTED)
			bg_gc = style->base_gc[state];
		      else
			bg_gc = GTK_WIDGET (clist)->style->base_gc[state];

		      if (!area)
			gdk_draw_rectangle (clist->clist_window, bg_gc, TRUE,
					    tree_rectangle.x,
					    tree_rectangle.y,
					    tree_rectangle.width,
					    tree_rectangle.height);
		      else
			gdk_draw_rectangle (clist->clist_window,
					    bg_gc, TRUE,
					    tc_rectangle.x,
					    tc_rectangle.y,
					    tc_rectangle.width,
					    tc_rectangle.height);
		    }

		  eel_list_get_cell_style (EEL_LIST (clist),
						&EEL_CTREE_ROW (node)->row,
						state, row, column, NULL, NULL, &bg_gc, NULL);
		  if (bg_gc == clist->bg_gc)
		    gdk_gc_set_foreground
		      (clist->bg_gc, &EEL_CTREE_ROW (node)->row.background);

		  gdk_gc_set_clip_rectangle (bg_gc, crect);
		  gdk_draw_arc (clist->clist_window, bg_gc, TRUE,
				xcenter - (justify_right * offset_x),
				cell_rectangle->y,
				offset_x, clist->row_height,
				(180 + (justify_right * 90)) * 64, 90 * 64);
		  gdk_gc_set_clip_rectangle (bg_gc, NULL);

		  gdk_draw_line (clist->clist_window, ctree->lines_gc, 
				 xcenter, cell_rectangle->y, xcenter, ycenter);

		  if (justify_right)
		    gdk_draw_arc (clist->clist_window, ctree->lines_gc, FALSE,
				  xcenter - offset_x, cell_rectangle->y,
				  offset_x, clist->row_height,
				  270 * 64, 90 * 64);
		  else
		    gdk_draw_arc (clist->clist_window, ctree->lines_gc, FALSE,
				  xcenter, cell_rectangle->y,
				  offset_x, clist->row_height,
				  180 * 64, 90 * 64);
		}
	      node = EEL_CTREE_ROW (node)->parent;
	    }
	}

      if (state != GTK_STATE_SELECTED)
	{
	  tree_rectangle.y = clip_rectangle->y;
	  tree_rectangle.height = clip_rectangle->height;
	  tree_rectangle.width = COLUMN_INSET + CELL_SPACING +
	    MIN (clist->column[ctree->tree_column].area.width + COLUMN_INSET,
		 TAB_SIZE);

	  if (justify_right)
	    tree_rectangle.x = MAX (xcenter + 1, column_left);
	  else
	    tree_rectangle.x = column_left;

	  if (!area)
	    gdk_draw_rectangle (clist->clist_window,
				GTK_WIDGET
				(ctree)->style->base_gc[GTK_STATE_NORMAL],
				TRUE,
				tree_rectangle.x,
				tree_rectangle.y,
				tree_rectangle.width,
				tree_rectangle.height);
	  else if (gdk_rectangle_intersect (area, &tree_rectangle,
					    &tc_rectangle))
	    gdk_draw_rectangle (clist->clist_window,
				GTK_WIDGET
				(ctree)->style->base_gc[GTK_STATE_NORMAL],
				TRUE,
				tc_rectangle.x,
				tc_rectangle.y,
				tc_rectangle.width,
				tc_rectangle.height);
	}

      xcenter = offset + (justification_factor * ctree->tree_indent / 2);

      eel_list_get_cell_style (EEL_LIST (clist),
				    &ctree_row->row, state, row, column,
				    NULL, NULL, &bg_gc, NULL);
      if (bg_gc == clist->bg_gc)
	gdk_gc_set_foreground (clist->bg_gc, &ctree_row->row.background);

      gdk_gc_set_clip_rectangle (bg_gc, crect);
      if (ctree_row->is_leaf)
	{
	  GdkPoint points[6];

	  points[0].x = offset + justification_factor * TAB_SIZE;
	  points[0].y = cell_rectangle->y;

	  points[1].x = points[0].x - justification_factor * 4;
	  points[1].y = points[0].y;

	  points[2].x = points[1].x - justification_factor * 2;
	  points[2].y = points[1].y + 3;

	  points[3].x = points[2].x;
	  points[3].y = points[2].y + clist->row_height - 5;

	  points[4].x = points[3].x + justification_factor * 2;
	  points[4].y = points[3].y + 3;

	  points[5].x = points[4].x + justification_factor * 4;
	  points[5].y = points[4].y;

	  gdk_draw_polygon (clist->clist_window, bg_gc, TRUE, points, 6);
	  gdk_draw_lines (clist->clist_window, ctree->lines_gc, points, 6);
	}
      else 
	{
	  gdk_draw_arc (clist->clist_window, bg_gc, TRUE,
			offset - (justify_right * 2 * TAB_SIZE),
			cell_rectangle->y,
			2 * TAB_SIZE, clist->row_height,
			(90 + (180 * justify_right)) * 64, 180 * 64);
	  gdk_draw_arc (clist->clist_window, ctree->lines_gc, FALSE,
			offset - (justify_right * 2 * TAB_SIZE),
			cell_rectangle->y,
			2 * TAB_SIZE, clist->row_height,
			(90 + (180 * justify_right)) * 64, 180 * 64);
	}
      gdk_gc_set_clip_rectangle (bg_gc, NULL);
      gdk_gc_set_clip_rectangle (ctree->lines_gc, NULL);

      offset += justification_factor * 3;
      break;
    default:
      xcenter = offset + justification_factor * PM_SIZE / 2;

      if (area)
	{
	  tree_rectangle.y = crect->y;
	  tree_rectangle.height = crect->height;

	  if (justify_right)
	    {
	      tree_rectangle.x = xcenter - PM_SIZE / 2 - 2;
	      tree_rectangle.width = (clip_rectangle->x +
				      clip_rectangle->width -tree_rectangle.x);
	    }
	  else
	    {
	      tree_rectangle.x = clip_rectangle->x + PM_SIZE / 2;
	      tree_rectangle.width = (xcenter + PM_SIZE / 2 + 2 -
				      clip_rectangle->x);
	    }

	  if (!gdk_rectangle_intersect (area, &tree_rectangle, &tc_rectangle))
	    break;
	}

      offset_x = 1;
      offset_y = 0;
      if (ctree->line_style == EEL_CTREE_LINES_DOTTED)
	{
	  offset_x += abs((clip_rectangle->x + clist->hoffset) % 2);
	  offset_y  = abs((cell_rectangle->y + clist->voffset) % 2);
	}

      clip_rectangle->y--;
      clip_rectangle->height++;
      gdk_gc_set_clip_rectangle (ctree->lines_gc, clip_rectangle);
      gdk_draw_line (clist->clist_window, ctree->lines_gc,
		     xcenter,
		     (ctree->show_stub || clist->row_list->data != ctree_row) ?
		     cell_rectangle->y + offset_y : ycenter,
		     xcenter,
		     (ctree_row->sibling) ? crect->y +crect->height : ycenter);

      gdk_draw_line (clist->clist_window, ctree->lines_gc,
		     xcenter + (justification_factor * offset_x), ycenter,
		     xcenter + (justification_factor * (PM_SIZE / 2 + 2)),
		     ycenter);

      node = ctree_row->parent;
      while (node)
	{
	  xcenter -= (justification_factor * ctree->tree_indent);

	  if (EEL_CTREE_ROW (node)->sibling)
	    gdk_draw_line (clist->clist_window, ctree->lines_gc, 
			   xcenter, cell_rectangle->y + offset_y,
			   xcenter, crect->y + crect->height);
	  node = EEL_CTREE_ROW (node)->parent;
	}
      gdk_gc_set_clip_rectangle (ctree->lines_gc, NULL);
      clip_rectangle->y++;
      clip_rectangle->height--;
      break;
    }
  return offset;
}

static void
draw_row (EelCList     *clist,
	  GdkRectangle *area,
	  gint          row,
	  EelCListRow  *clist_row)
{
  GtkWidget *widget;
  EelCTree  *ctree;
  GdkRectangle *rect;
  GdkRectangle *crect;
  GdkRectangle row_rectangle;
  GdkRectangle cell_rectangle; 
  GdkRectangle clip_rectangle;
  GdkRectangle intersect_rectangle;
  gint last_column;
  gint column_left = 0;
  gint column_right = 0;
  gint offset = 0;
  gint state;
  gint i;

  g_return_if_fail (clist != NULL);

  /* bail now if we arn't drawable yet */
  if (!GTK_WIDGET_DRAWABLE (clist) || row < 0 || row >= clist->rows)
    return;

  widget = GTK_WIDGET (clist);
  ctree  = EEL_CTREE  (clist);

  /* if the function is passed the pointer to the row instead of null,
   * it avoids this expensive lookup */
  if (!clist_row)
    clist_row = (g_list_nth (clist->row_list, row))->data;

  /* rectangle of the entire row */
  row_rectangle.x = 0;
  row_rectangle.y = ROW_TOP_YPIXEL (clist, row);
  row_rectangle.width = clist->clist_window_width;
  row_rectangle.height = clist->row_height;

  /* rectangle of the cell spacing above the row */
  cell_rectangle.x = 0;
  cell_rectangle.y = row_rectangle.y - CELL_SPACING;
  cell_rectangle.width = row_rectangle.width;
  cell_rectangle.height = CELL_SPACING;

  /* rectangle used to clip drawing operations, its y and height
   * positions only need to be set once, so we set them once here. 
   * the x and width are set withing the drawing loop below once per
   * column */
  clip_rectangle.y = row_rectangle.y;
  clip_rectangle.height = row_rectangle.height;

  if (clist_row->state == GTK_STATE_NORMAL)
    {
      if (clist_row->fg_set)
	gdk_gc_set_foreground (clist->fg_gc, &clist_row->foreground);
      if (clist_row->bg_set)
	gdk_gc_set_foreground (clist->bg_gc, &clist_row->background);
    }
  
  state = clist_row->state;

  gdk_gc_set_foreground (ctree->lines_gc,
			 &widget->style->fg[clist_row->state]);

  /* draw the cell borders */
  if (area)
    {
      rect = &intersect_rectangle;
      crect = &intersect_rectangle;

      if (gdk_rectangle_intersect (area, &cell_rectangle, crect))
	gdk_draw_rectangle (clist->clist_window,
			    widget->style->base_gc[GTK_STATE_ACTIVE], TRUE,
			    crect->x, crect->y, crect->width, crect->height);
    }
  else
    {
      rect = &clip_rectangle;
      crect = &cell_rectangle;

      gdk_draw_rectangle (clist->clist_window,
			  widget->style->base_gc[GTK_STATE_ACTIVE], TRUE,
			  crect->x, crect->y, crect->width, crect->height);
    }

  /* horizontal black lines */
  if (ctree->line_style == EEL_CTREE_LINES_TABBED)
    { 

      column_right = (COLUMN_LEFT_XPIXEL (clist, ctree->tree_column) +
		      clist->column[ctree->tree_column].area.width +
		      COLUMN_INSET);
      column_left = (COLUMN_LEFT_XPIXEL (clist, ctree->tree_column) -
		     COLUMN_INSET - (ctree->tree_column != 0) * CELL_SPACING);

      switch (clist->column[ctree->tree_column].justification)
	{
	case GTK_JUSTIFY_CENTER:
	case GTK_JUSTIFY_FILL:
	case GTK_JUSTIFY_LEFT:
	  offset = (column_left + ctree->tree_indent *
		    (((EelCTreeRow *)clist_row)->level - 1));

	  gdk_draw_line (clist->clist_window, ctree->lines_gc, 
			 MIN (offset + TAB_SIZE, column_right),
			 cell_rectangle.y,
			 clist->clist_window_width, cell_rectangle.y);
	  break;
	case GTK_JUSTIFY_RIGHT:
	  offset = (column_right - 1 - ctree->tree_indent *
		    (((EelCTreeRow *)clist_row)->level - 1));

	  gdk_draw_line (clist->clist_window, ctree->lines_gc,
			 -1, cell_rectangle.y,
			 MAX (offset - TAB_SIZE, column_left),
			 cell_rectangle.y);
	  break;
	}
    }

  /* the last row has to clear its bottom cell spacing too */
  if (clist_row == clist->row_list_end->data)
    {
      cell_rectangle.y += clist->row_height + CELL_SPACING;

      if (!area || gdk_rectangle_intersect (area, &cell_rectangle, crect))
	{
	  gdk_draw_rectangle (clist->clist_window,
			      widget->style->base_gc[GTK_STATE_ACTIVE], TRUE,
			      crect->x, crect->y, crect->width, crect->height);

	  /* horizontal black lines */
	  if (ctree->line_style == EEL_CTREE_LINES_TABBED)
	    { 
	      switch (clist->column[ctree->tree_column].justification)
		{
		case GTK_JUSTIFY_CENTER:
		case GTK_JUSTIFY_FILL:
		case GTK_JUSTIFY_LEFT:
		  gdk_draw_line (clist->clist_window, ctree->lines_gc, 
				 MIN (column_left + TAB_SIZE + COLUMN_INSET +
				      (((EelCTreeRow *)clist_row)->level > 1) *
				      MIN (ctree->tree_indent / 2, TAB_SIZE),
				      column_right),
				 cell_rectangle.y,
				 clist->clist_window_width, cell_rectangle.y);
		  break;
		case GTK_JUSTIFY_RIGHT:
		  gdk_draw_line (clist->clist_window, ctree->lines_gc, 
				 -1, cell_rectangle.y,
				 MAX (column_right - TAB_SIZE - 1 -
				      COLUMN_INSET -
				      (((EelCTreeRow *)clist_row)->level > 1) *
				      MIN (ctree->tree_indent / 2, TAB_SIZE),
				      column_left - 1), cell_rectangle.y);
		  break;
		}
	    }
	}
    }	  

  for (last_column = clist->columns - 1;
       last_column >= 0 && !clist->column[last_column].visible; last_column--)
    ;

  /* iterate and draw all the columns (row cells) and draw their contents */
  for (i = 0; i < clist->columns; i++)
    {
      GtkStyle *style;
      GdkGC *fg_gc; 
      GdkGC *bg_gc;
      guint bg_rgb;

      gint width;
      gint height;
      gint pixbuf_width;
      gint string_width;
      gint old_offset;
      gint row_center_offset;

      if (!clist->column[i].visible)
	continue;

      eel_list_get_cell_style (EEL_LIST (clist), clist_row, state, row, i,
				    &style, &fg_gc, &bg_gc, &bg_rgb);

      if (((EelCListRow *)ctree->dnd_prelighted_row) == clist_row) {
	      GdkFont *bold_font;

	      style = gtk_style_copy (style);
	      bold_font = eel_gdk_font_get_bold (style->font);
	      gdk_font_unref (style->font);
	      style->font = bold_font;
	      gdk_font_ref (style->font);
	      gtk_style_attach (style, clist->clist_window);
      } else {
	      gtk_style_ref (style);
      }


      /* calculate clipping region */
      clip_rectangle.x = clist->column[i].area.x + clist->hoffset;
      clip_rectangle.width = clist->column[i].area.width;

      cell_rectangle.x = clip_rectangle.x - COLUMN_INSET - CELL_SPACING;
      cell_rectangle.width = (clip_rectangle.width + 2 * COLUMN_INSET +
			      (1 + (i == last_column)) * CELL_SPACING);
      cell_rectangle.y = clip_rectangle.y;
      cell_rectangle.height = clip_rectangle.height;

      string_width = 0;
      pixbuf_width = 0;
      height = 0;

      if (area && !gdk_rectangle_intersect (area, &cell_rectangle,
					    &intersect_rectangle))
	{
	  if (i != ctree->tree_column)
	    continue;
	}
      else
	{
	  gdk_draw_rectangle (clist->clist_window, bg_gc, TRUE,
			      crect->x, crect->y, crect->width, crect->height);

	  /* calculate real width for column justification */
	  switch (clist_row->cell[i].type)
	    {
	    case EEL_CELL_TEXT:
	    case EEL_CELL_LINK_TEXT:
	      width = gdk_string_width
		(style->font, EEL_CELL_TEXT (clist_row->cell[i])->text);
	      break;
	    case EEL_CELL_PIXBUF:
	      pixbuf_width = gdk_pixbuf_get_width (EEL_CELL_PIXBUF (clist_row->cell[i])->pixbuf);
	      height = gdk_pixbuf_get_height (EEL_CELL_PIXBUF (clist_row->cell[i])->pixbuf);
	      width = pixbuf_width;
	      break;
	    case EEL_CELL_PIXTEXT:
	      if (EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf)
	        {
	          pixbuf_width = gdk_pixbuf_get_width (EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf);
	          height = gdk_pixbuf_get_height (EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf);
		}

	      width = pixbuf_width;

	      if (EEL_CELL_PIXTEXT (clist_row->cell[i])->text)
		{
		  string_width = gdk_string_width
		    (style->font, EEL_CELL_PIXTEXT (clist_row->cell[i])->text);
		  width += string_width;
		}

	      if (EEL_CELL_PIXTEXT (clist_row->cell[i])->text &&
		  EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf)
		width +=  EEL_CELL_PIXTEXT (clist_row->cell[i])->spacing;

	      if (i == ctree->tree_column)
		width += (ctree->tree_indent *
			  ((EelCTreeRow *)clist_row)->level);
	      break;
	    default:
	      continue;
	      break;
	    }

	  switch (clist->column[i].justification)
	    {
	    case GTK_JUSTIFY_LEFT:
	      offset = clip_rectangle.x + clist_row->cell[i].horizontal;
	      break;
	    case GTK_JUSTIFY_RIGHT:
	      offset = (clip_rectangle.x + clist_row->cell[i].horizontal +
			clip_rectangle.width - width);
	      break;
	    case GTK_JUSTIFY_CENTER:
	    case GTK_JUSTIFY_FILL:
	      offset = (clip_rectangle.x + clist_row->cell[i].horizontal +
			(clip_rectangle.width / 2) - (width / 2));
	      break;
	    };

	  if (i != ctree->tree_column)
	    {
	      offset += clist_row->cell[i].horizontal;
	      switch (clist_row->cell[i].type)
		{
		case EEL_CELL_PIXBUF:
			offset = eel_list_draw_cell_pixbuf (EEL_LIST (clist),
								 clist->clist_window, &cell_rectangle, fg_gc, bg_rgb,
								 EEL_CELL_PIXBUF (clist_row->cell[i])->pixbuf,
								 offset,
								 clip_rectangle.y + clist_row->cell[i].vertical +
								 (clip_rectangle.height - height) / 2);
		  break;
		case EEL_CELL_PIXTEXT:
			offset = eel_list_draw_cell_pixbuf (EEL_LIST (clist),
								 clist->clist_window, &clip_rectangle, fg_gc, bg_rgb,
								 EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf,
								 offset,
								 clip_rectangle.y + clist_row->cell[i].vertical +
								 (clip_rectangle.height - height) / 2);
			
		  offset += EEL_CELL_PIXTEXT (clist_row->cell[i])->spacing;
		case EEL_CELL_TEXT:
		  row_center_offset = ((clist->row_height -
					(style->font->ascent
					 + style->font->descent)) / 2
					 + style->font->ascent);

		  gdk_gc_set_clip_rectangle (fg_gc, &clip_rectangle);
		  gdk_draw_string
		    (clist->clist_window, style->font, fg_gc,
		     offset,
		     row_rectangle.y + row_center_offset +
		     clist_row->cell[i].vertical,
		     (clist_row->cell[i].type == EEL_CELL_PIXTEXT) ?
		     EEL_CELL_PIXTEXT (clist_row->cell[i])->text :
		     EEL_CELL_TEXT (clist_row->cell[i])->text);
		  gdk_gc_set_clip_rectangle (fg_gc, NULL);
		  break;
		default:
		  break;
		}
	      continue;
	    }
	}

      if (bg_gc == clist->bg_gc)
	gdk_gc_set_background (ctree->lines_gc, &clist_row->background);

      /* draw ctree->tree_column */
      cell_rectangle.y -= CELL_SPACING;
      cell_rectangle.height += CELL_SPACING;

      if (area && !gdk_rectangle_intersect (area, &cell_rectangle,
					    &intersect_rectangle))
	continue;

      /* draw lines */
      offset = eel_ctree_draw_lines (ctree, (EelCTreeRow *)clist_row, row, i,
					  state, &clip_rectangle, &cell_rectangle,
					  crect, area, style);

      /* draw expander */
      offset = eel_ctree_draw_expander (ctree, (EelCTreeRow *)clist_row,
					     style, &clip_rectangle, offset);

      if (clist->column[i].justification == GTK_JUSTIFY_RIGHT)
	offset -= ctree->tree_spacing;
      else
	offset += ctree->tree_spacing;

      if (clist->column[i].justification == GTK_JUSTIFY_RIGHT)
	offset -= (pixbuf_width + clist_row->cell[i].horizontal);
      else
	offset += clist_row->cell[i].horizontal;

      old_offset = offset;
      if (height > 0) {
	      GdkPixbuf *src_pixbuf, *dark_pixbuf;

	      if (((EelCListRow *)ctree->dnd_prelighted_row) == clist_row) {
		      
		      src_pixbuf = EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf;
		      
		      if (src_pixbuf != NULL) {
			      /* Create darkened pixbuf */			
			      dark_pixbuf = eel_create_darkened_pixbuf (src_pixbuf,
									     0.8 * 255,
									     0.8 * 255);
			      if (dark_pixbuf != NULL) {
				      offset = eel_list_draw_cell_pixbuf (EEL_LIST (clist),
									       clist->clist_window, &cell_rectangle, fg_gc, bg_rgb,
									       dark_pixbuf, offset,
									       clip_rectangle.y + clist_row->cell[i].vertical +
									       (clip_rectangle.height - height) / 2);

				      gdk_pixbuf_unref (dark_pixbuf);
			      }
		      }					
	      } else {		
		      offset = eel_list_draw_cell_pixbuf (EEL_LIST (clist),
							       clist->clist_window, &clip_rectangle, fg_gc, bg_rgb,
							       EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf,
							       offset,
							       clip_rectangle.y + clist_row->cell[i].vertical +
							       (clip_rectangle.height - height) / 2);


	      }
      }

      if (string_width)
	{ 
	  if (clist->column[i].justification == GTK_JUSTIFY_RIGHT)
	    {
	      offset = (old_offset - string_width);
	      if (EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf)
		offset -= EEL_CELL_PIXTEXT (clist_row->cell[i])->spacing;
	    }
	  else
	    {
	      if (EEL_CELL_PIXTEXT (clist_row->cell[i])->pixbuf)
		offset += EEL_CELL_PIXTEXT (clist_row->cell[i])->spacing;
	    }

	  row_center_offset = ((clist->row_height -
				(style->font->ascent
				 + style->font->descent)) / 2
			       + style->font->ascent);

	  gdk_gc_set_clip_rectangle (fg_gc, &clip_rectangle);
	  gdk_draw_string (clist->clist_window, style->font, fg_gc, offset,
			   row_rectangle.y + row_center_offset +
			   clist_row->cell[i].vertical,
			   EEL_CELL_PIXTEXT (clist_row->cell[i])->text);
	}
      gdk_gc_set_clip_rectangle (fg_gc, NULL);
      gtk_style_unref (style);
    }
  

  /* draw focus rectangle */
  if (clist->focus_row == row &&
      GTK_WIDGET_CAN_FOCUS (widget) && GTK_WIDGET_HAS_FOCUS (widget))
    {
      if (!area)
	gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
			    row_rectangle.x, row_rectangle.y,
			    row_rectangle.width - 1, row_rectangle.height - 1);
      else if (gdk_rectangle_intersect (area, &row_rectangle,
					&intersect_rectangle))
	{
	  gdk_gc_set_clip_rectangle (clist->xor_gc, &intersect_rectangle);
	  gdk_draw_rectangle (clist->clist_window, clist->xor_gc, FALSE,
			      row_rectangle.x, row_rectangle.y,
			      row_rectangle.width - 1,
			      row_rectangle.height - 1);
	  gdk_gc_set_clip_rectangle (clist->xor_gc, NULL);
	}
    }
}

void
eel_ctree_draw_node (EelCTree *ctree, EelCTreeNode *node)
{
	if (ctree == NULL || node == NULL) {
		return;
	}
	
	tree_draw_node (ctree, node);
}

static void
tree_draw_node (EelCTree     *ctree, 
	        EelCTreeNode *node)
{
	EelCList *clist;
  
	clist = EEL_CLIST (ctree);

	if (CLIST_UNFROZEN (clist) && eel_ctree_is_viewable (ctree, node))
	{
		EelCTreeNode *work;
		gint num = 0;
      
		work = EEL_CTREE_NODE (clist->row_list);
		while (work && work != node)
		{
			work = EEL_CTREE_NODE_NEXT (work);
			num++;
		}
		
		if (work && eel_clist_row_is_visible (clist, num) != GTK_VISIBILITY_NONE) {
			EEL_CLIST_CLASS_FW (clist)->draw_row			
	  			(clist, NULL, num, EEL_CLIST_ROW ((GList *) node));
		}
	}
}

static EelCTreeNode *
eel_ctree_last_visible (EelCTree     *ctree,
			EelCTreeNode *node)
{
  EelCTreeNode *work;
  
  if (!node)
    return NULL;

  work = EEL_CTREE_ROW (node)->children;

  if (!work || !EEL_CTREE_ROW (node)->expanded)
    return node;

  while (EEL_CTREE_ROW (work)->sibling)
    work = EEL_CTREE_ROW (work)->sibling;

  return eel_ctree_last_visible (ctree, work);
}

static void
eel_ctree_link (EelCTree     *ctree,
		EelCTreeNode *node,
		EelCTreeNode *parent,
		EelCTreeNode *sibling,
		gboolean      update_focus_row)
{
  EelCList *clist;
  GList *list_end;
  GList *list;
  GList *work;
  gboolean visible = FALSE;
  gint rows = 0;
  
  if (sibling)
    g_return_if_fail (EEL_CTREE_ROW (sibling)->parent == parent);
  g_return_if_fail (node != NULL);
  g_return_if_fail (node != sibling);
  g_return_if_fail (node != parent);

  clist = EEL_CLIST (ctree);

  if (update_focus_row && clist->selection_mode == GTK_SELECTION_EXTENDED)
    {
      EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
      
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
    }

  for (rows = 1, list_end = (GList *)node; list_end->next;
       list_end = list_end->next)
    rows++;

  EEL_CTREE_ROW (node)->parent = parent;
  EEL_CTREE_ROW (node)->sibling = sibling;

  if (!parent || (parent && (eel_ctree_is_viewable (ctree, parent) &&
			     EEL_CTREE_ROW (parent)->expanded)))
    {
      visible = TRUE;
      clist->rows += rows;
    }

  if (parent)
    work = (GList *)(EEL_CTREE_ROW (parent)->children);
  else
    work = clist->row_list;

  if (sibling)
    {
      if (work != (GList *)sibling)
	{
	  while (EEL_CTREE_ROW (work)->sibling != sibling)
	    work = (GList *)(EEL_CTREE_ROW (work)->sibling);
	  EEL_CTREE_ROW (work)->sibling = node;
	}

      if (sibling == EEL_CTREE_NODE (clist->row_list))
	clist->row_list = (GList *) node;
      if (EEL_CTREE_NODE_PREV (sibling) &&
	  EEL_CTREE_NODE_NEXT (EEL_CTREE_NODE_PREV (sibling)) == sibling)
	{
	  list = (GList *)EEL_CTREE_NODE_PREV (sibling);
	  list->next = (GList *)node;
	}
      
      list = (GList *)node;
      list->prev = (GList *)EEL_CTREE_NODE_PREV (sibling);
      list_end->next = (GList *)sibling;
      list = (GList *)sibling;
      list->prev = list_end;
      if (parent && EEL_CTREE_ROW (parent)->children == sibling)
	EEL_CTREE_ROW (parent)->children = node;
    }
  else
    {
      if (work)
	{
	  /* find sibling */
	  while (EEL_CTREE_ROW (work)->sibling)
	    work = (GList *)(EEL_CTREE_ROW (work)->sibling);
	  EEL_CTREE_ROW (work)->sibling = node;
	  
	  /* find last visible child of sibling */
	  work = (GList *) eel_ctree_last_visible (ctree,
						   EEL_CTREE_NODE (work));
	  
	  list_end->next = work->next;
	  if (work->next)
	    list = work->next->prev = list_end;
	  work->next = (GList *)node;
	  list = (GList *)node;
	  list->prev = work;
	}
      else
	{
	  if (parent)
	    {
	      EEL_CTREE_ROW (parent)->children = node;
	      list = (GList *)node;
	      list->prev = (GList *)parent;
	      if (EEL_CTREE_ROW (parent)->expanded)
		{
		  list_end->next = (GList *)EEL_CTREE_NODE_NEXT (parent);
		  if (EEL_CTREE_NODE_NEXT(parent))
		    {
		      list = (GList *)EEL_CTREE_NODE_NEXT (parent);
		      list->prev = list_end;
		    }
		  list = (GList *)parent;
		  list->next = (GList *)node;
		}
	      else
		list_end->next = NULL;
	    }
	  else
	    {
	      clist->row_list = (GList *)node;
	      list = (GList *)node;
	      list->prev = NULL;
	      list_end->next = NULL;
	    }
	}
    }

  eel_ctree_pre_recursive (ctree, node, tree_update_level, NULL); 

  if (clist->row_list_end == NULL ||
      clist->row_list_end->next == (GList *)node)
    clist->row_list_end = list_end;

  if (visible && update_focus_row)
    {
      gint pos;
	  
      pos = g_list_position (clist->row_list, (GList *)node);
  
      if (pos <= clist->focus_row)
	{
	  clist->focus_row += rows;
	  clist->undo_anchor = clist->focus_row;
	}
    }
}

static void
eel_ctree_unlink (EelCTree     *ctree, 
		  EelCTreeNode *node,
                  gboolean      update_focus_row)
{
  EelCList *clist;
  gint rows;
  gint level;
  gint visible;
  EelCTreeNode *work;
  EelCTreeNode *parent;
  GList *list;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  clist = EEL_CLIST (ctree);
  
  if (update_focus_row && clist->selection_mode == GTK_SELECTION_EXTENDED)
    {
      EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
      
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
    }

  visible = eel_ctree_is_viewable (ctree, node);

  /* clist->row_list_end unlinked ? */
  if (visible &&
      (EEL_CTREE_NODE_NEXT (node) == NULL ||
       (EEL_CTREE_ROW (node)->children &&
	eel_ctree_is_ancestor (ctree, node,
				    EEL_CTREE_NODE (clist->row_list_end)))))
	  clist->row_list_end = (GList *) (EEL_CTREE_NODE_PREV (node));

  /* update list */
  rows = 0;
  level = EEL_CTREE_ROW (node)->level;
  work = EEL_CTREE_NODE_NEXT (node);
  while (work && EEL_CTREE_ROW (work)->level > level)
    {
      work = EEL_CTREE_NODE_NEXT (work);
      rows++;
    }

  if (visible)
    {
      clist->rows -= (rows + 1);

      if (update_focus_row)
	{
	  gint pos;
	  
	  pos = g_list_position (clist->row_list, (GList *)node);
	  if (pos + rows < clist->focus_row)
	    clist->focus_row -= (rows + 1);
	  else if (pos <= clist->focus_row)
	    {
	      if (!EEL_CTREE_ROW (node)->sibling)
		clist->focus_row = MAX (pos - 1, 0);
	      else
		clist->focus_row = pos;
	      
	      clist->focus_row = MIN (clist->focus_row, clist->rows - 1);
	    }
	  clist->undo_anchor = clist->focus_row;
	}
    }

  if (work)
    {
      list = (GList *)EEL_CTREE_NODE_PREV (work);
      list->next = NULL;
      list = (GList *)work;
      list->prev = (GList *)EEL_CTREE_NODE_PREV (node);
    }

  if (EEL_CTREE_NODE_PREV (node) &&
      EEL_CTREE_NODE_NEXT (EEL_CTREE_NODE_PREV (node)) == node)
    {
      list = (GList *)EEL_CTREE_NODE_PREV (node);
      list->next = (GList *)work;
    }

  /* update tree */
  parent = EEL_CTREE_ROW (node)->parent;
  if (parent)
    {
      if (EEL_CTREE_ROW (parent)->children == node)
	{
	  EEL_CTREE_ROW (parent)->children = EEL_CTREE_ROW (node)->sibling;
	  if (EEL_CTREE_ROW (parent)->is_leaf)
	    eel_ctree_collapse (ctree, parent);
	}
      else
	{
	  EelCTreeNode *sibling;

	  sibling = EEL_CTREE_ROW (parent)->children;
	  while (EEL_CTREE_ROW (sibling)->sibling != node)
	    sibling = EEL_CTREE_ROW (sibling)->sibling;
	  EEL_CTREE_ROW (sibling)->sibling = EEL_CTREE_ROW (node)->sibling;
	}
    }
  else
    {
      if (clist->row_list == (GList *)node)
	clist->row_list = (GList *) (EEL_CTREE_ROW (node)->sibling);
      else
	{
	  EelCTreeNode *sibling;

	  sibling = EEL_CTREE_NODE (clist->row_list);
	  while (EEL_CTREE_ROW (sibling)->sibling != node)
	    sibling = EEL_CTREE_ROW (sibling)->sibling;
	  EEL_CTREE_ROW (sibling)->sibling = EEL_CTREE_ROW (node)->sibling;
	}
    }
}

static void
real_row_move (EelCList *clist,
	       gint      source_row,
	       gint      dest_row)
{
  EelCTree *ctree;
  EelCTreeNode *node;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  if (EEL_CLIST_AUTO_SORT (clist))
    return;

  if (source_row < 0 || source_row >= clist->rows ||
      dest_row   < 0 || dest_row   >= clist->rows ||
      source_row == dest_row)
    return;

  ctree = EEL_CTREE (clist);
  node = EEL_CTREE_NODE (g_list_nth (clist->row_list, source_row));

  if (source_row < dest_row)
    {
      EelCTreeNode *work; 

      dest_row++;
      work = EEL_CTREE_ROW (node)->children;

      while (work && EEL_CTREE_ROW (work)->level > EEL_CTREE_ROW (node)->level)
	{
	  work = EEL_CTREE_NODE_NEXT (work);
	  dest_row++;
	}

      if (dest_row > clist->rows)
	dest_row = clist->rows;
    }

  if (dest_row < clist->rows)
    {
      EelCTreeNode *sibling;

      sibling = EEL_CTREE_NODE (g_list_nth (clist->row_list, dest_row));
      eel_ctree_move (ctree, node, EEL_CTREE_ROW (sibling)->parent, sibling);
    }
  else
    eel_ctree_move (ctree, node, NULL, NULL);
}

static void
real_tree_move (EelCTree     *ctree,
		EelCTreeNode *node,
		EelCTreeNode *new_parent, 
		EelCTreeNode *new_sibling)
{
  EelCList *clist;
  EelCTreeNode *work;
  gboolean visible = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (node != NULL);
  g_return_if_fail (!new_sibling || 
		    EEL_CTREE_ROW (new_sibling)->parent == new_parent);

  if (new_parent && EEL_CTREE_ROW (new_parent)->is_leaf)
    return;

  /* new_parent != child of child */
  for (work = new_parent; work; work = EEL_CTREE_ROW (work)->parent)
    if (work == node)
      return;

  clist = EEL_CLIST (ctree);

  visible = eel_ctree_is_viewable (ctree, node);

  if (clist->selection_mode == GTK_SELECTION_EXTENDED)
    {
      EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
      
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
    }

  if (EEL_CLIST_AUTO_SORT (clist))
    {
      if (new_parent == EEL_CTREE_ROW (node)->parent)
	return;
      
      if (new_parent)
	new_sibling = EEL_CTREE_ROW (new_parent)->children;
      else
	new_sibling = EEL_CTREE_NODE (clist->row_list);

      while (new_sibling && clist->compare
	     (clist, EEL_CTREE_ROW (node), EEL_CTREE_ROW (new_sibling)) > 0)
	new_sibling = EEL_CTREE_ROW (new_sibling)->sibling;
    }

  if (new_parent == EEL_CTREE_ROW (node)->parent && 
      new_sibling == EEL_CTREE_ROW (node)->sibling)
    return;

  eel_clist_freeze (clist);

  work = NULL;
  if (eel_ctree_is_viewable (ctree, node) ||
      eel_ctree_is_viewable (ctree, new_sibling))
    work = EEL_CTREE_NODE (g_list_nth (clist->row_list, clist->focus_row));
      
  eel_ctree_unlink (ctree, node, FALSE);
  eel_ctree_link (ctree, node, new_parent, new_sibling, FALSE);
  
  if (work)
    {
      while (work &&  !eel_ctree_is_viewable (ctree, work))
	work = EEL_CTREE_ROW (work)->parent;
      clist->focus_row = g_list_position (clist->row_list, (GList *)work);
      clist->undo_anchor = clist->focus_row;
      CLIST_REFRESH (clist);
    }

  if (clist->column[ctree->tree_column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist) &&
      (visible || eel_ctree_is_viewable (ctree, node)))
    eel_clist_set_column_width
      (clist, ctree->tree_column,
       eel_clist_optimal_column_width (clist, ctree->tree_column));

  eel_clist_thaw (clist);
}

static void
change_focus_row_expansion (EelCTree          *ctree,
			    EelCTreeExpansionType action)
{
  EelCList *clist;
  EelCTreeNode *node;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (ctree))
    return;
  
  if (!(node =
	EEL_CTREE_NODE (g_list_nth (clist->row_list, clist->focus_row))) ||
      EEL_CTREE_ROW (node)->is_leaf)
    return;

  switch (action)
    {
    case EEL_CTREE_EXPANSION_EXPAND:
      eel_ctree_expand (ctree, node);
      break;
    case EEL_CTREE_EXPANSION_EXPAND_RECURSIVE:
      eel_ctree_expand_recursive (ctree, node);
      break;
    case EEL_CTREE_EXPANSION_COLLAPSE:
      eel_ctree_collapse (ctree, node);
      break;
    case EEL_CTREE_EXPANSION_COLLAPSE_RECURSIVE:
      eel_ctree_collapse_recursive (ctree, node);
      break;
    case EEL_CTREE_EXPANSION_TOGGLE:
      eel_ctree_toggle_expansion (ctree, node);
      break;
    case EEL_CTREE_EXPANSION_TOGGLE_RECURSIVE:
      eel_ctree_toggle_expansion_recursive (ctree, node);
      break;
    }
}

static void 
real_tree_expand (EelCTree     *ctree,
		  EelCTreeNode *node)
{
  EelCList *clist;
  EelCTreeNode *work;
  GtkRequisition requisition;
  gboolean visible;
  gint level;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (!node || EEL_CTREE_ROW (node)->expanded || EEL_CTREE_ROW (node)->is_leaf)
    return;

  clist = EEL_CLIST (ctree);
  
  EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);

  EEL_CTREE_ROW (node)->expanded = TRUE;
  level = EEL_CTREE_ROW (node)->level;

  visible = eel_ctree_is_viewable (ctree, node);
  /* get cell width if tree_column is auto resized */
  if (visible && clist->column[ctree->tree_column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    EEL_CLIST_CLASS_FW (clist)->cell_size_request
      (clist, &EEL_CTREE_ROW (node)->row, ctree->tree_column, &requisition);

  /* unref/unset closed pixbuf */
  if (EEL_CELL_PIXTEXT 
      (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf)
    {
      gdk_pixbuf_unref
	(EEL_CELL_PIXTEXT
	 (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf);
      
      EEL_CELL_PIXTEXT
	(EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf = NULL;
    }

  /* set/ref opened pixbuf */
  if (EEL_CTREE_ROW (node)->pixbuf_opened)
    {
      EEL_CELL_PIXTEXT
	(EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf = 
	gdk_pixbuf_ref (EEL_CTREE_ROW (node)->pixbuf_opened);
    }


  work = EEL_CTREE_ROW (node)->children;
  if (work)
    {
      GList *list = (GList *)work;
      gint *cell_width = NULL;
      gint tmp = 0;
      gint row;
      gint i;
      
      if (visible && !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
	{
	  cell_width = g_new0 (gint, clist->columns);
	  if (clist->column[ctree->tree_column].auto_resize)
	      cell_width[ctree->tree_column] = requisition.width;

	  while (work)
	    {
	      /* search maximum cell widths of auto_resize columns */
	      for (i = 0; i < clist->columns; i++)
		if (clist->column[i].auto_resize)
		  {
		    EEL_CLIST_CLASS_FW (clist)->cell_size_request
		      (clist, &EEL_CTREE_ROW (work)->row, i, &requisition);
		    cell_width[i] = MAX (requisition.width, cell_width[i]);
		  }

	      list = (GList *)work;
	      work = EEL_CTREE_NODE_NEXT (work);
	      tmp++;
	    }
	}
      else
	while (work)
	  {
	    list = (GList *)work;
	    work = EEL_CTREE_NODE_NEXT (work);
	    tmp++;
	  }

      list->next = (GList *)EEL_CTREE_NODE_NEXT (node);

      if (EEL_CTREE_NODE_NEXT (node))
	{
	  GList *tmp_list;

	  tmp_list = (GList *)EEL_CTREE_NODE_NEXT (node);
	  tmp_list->prev = list;
	}
      else
	clist->row_list_end = list;

      list = (GList *)node;
      list->next = (GList *)(EEL_CTREE_ROW (node)->children);

      if (visible)
	{
	  /* resize auto_resize columns if needed */
	  for (i = 0; i < clist->columns; i++)
	    if (clist->column[i].auto_resize &&
		cell_width[i] > clist->column[i].width)
	      eel_clist_set_column_width (clist, i, cell_width[i]);
	  g_free (cell_width);

	  /* update focus_row position */
	  row = g_list_position (clist->row_list, (GList *)node);
	  if (row < clist->focus_row)
	    clist->focus_row += tmp;

	  clist->rows += tmp;
	  CLIST_REFRESH (clist);
	}
    }
  else if (visible && clist->column[ctree->tree_column].auto_resize)
    /* resize tree_column if needed */
    column_auto_resize (clist, &EEL_CTREE_ROW (node)->row, ctree->tree_column,
			requisition.width);

  tree_draw_node (ctree, node);
}

static void 
real_tree_collapse (EelCTree     *ctree,
		    EelCTreeNode *node)
{
  EelCList *clist;
  EelCTreeNode *work;
  GtkRequisition requisition;
  gboolean visible;
  gint level;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (!node || !EEL_CTREE_ROW (node)->expanded ||
      EEL_CTREE_ROW (node)->is_leaf)
    return;

  clist = EEL_CLIST (ctree);

  EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
  
  EEL_CTREE_ROW (node)->expanded = FALSE;
  level = EEL_CTREE_ROW (node)->level;

  visible = eel_ctree_is_viewable (ctree, node);
  /* get cell width if tree_column is auto resized */
  if (visible && clist->column[ctree->tree_column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    EEL_CLIST_CLASS_FW (clist)->cell_size_request
      (clist, &EEL_CTREE_ROW (node)->row, ctree->tree_column, &requisition);

  /* unref/unset opened pixbuf */
  if (EEL_CELL_PIXTEXT 
      (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf)
    {
      gdk_pixbuf_unref
	(EEL_CELL_PIXTEXT
	 (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf);
      
      EEL_CELL_PIXTEXT
	(EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf = NULL;
    }

  /* set/ref closed pixbuf */
  if (EEL_CTREE_ROW (node)->pixbuf_closed)
    {
      EEL_CELL_PIXTEXT 
	(EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf = 
	gdk_pixbuf_ref (EEL_CTREE_ROW (node)->pixbuf_closed);
    }

  work = EEL_CTREE_ROW (node)->children;
  if (work)
    {
      gint tmp = 0;
      gint row;
      GList *list;

      while (work && EEL_CTREE_ROW (work)->level > level)
	{
	  work = EEL_CTREE_NODE_NEXT (work);
	  tmp++;
	}

      if (work)
	{
	  list = (GList *)node;
	  list->next = (GList *)work;
	  list = (GList *)EEL_CTREE_NODE_PREV (work);
	  list->next = NULL;
	  list = (GList *)work;
	  list->prev = (GList *)node;
	}
      else
	{
	  list = (GList *)node;
	  list->next = NULL;
	  clist->row_list_end = (GList *)node;
	}

      if (visible)
	{
	  /* resize auto_resize columns if needed */
	  auto_resize_columns (clist);

	  row = g_list_position (clist->row_list, (GList *)node);
	  if (row < clist->focus_row)
	    clist->focus_row -= tmp;
	  clist->rows -= tmp;
	  CLIST_REFRESH (clist);
	}
    }
  else if (visible && clist->column[ctree->tree_column].auto_resize &&
	   !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    /* resize tree_column if needed */
    column_auto_resize (clist, &EEL_CTREE_ROW (node)->row, ctree->tree_column,
			requisition.width);
    
  tree_draw_node (ctree, node);
}

static void
column_auto_resize (EelCList    *clist,
		    EelCListRow *clist_row,
		    gint         column,
		    gint         old_width)
{
  /* resize column if needed for auto_resize */
  GtkRequisition requisition;

  if (!clist->column[column].auto_resize ||
      EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    return;

  if (clist_row)
    EEL_CLIST_CLASS_FW (clist)->cell_size_request (clist, clist_row,
						   column, &requisition);
  else
    requisition.width = 0;

  if (requisition.width > clist->column[column].width)
    eel_clist_set_column_width (clist, column, requisition.width);
  else if (requisition.width < old_width &&
	   old_width == clist->column[column].width)
    {
      GList *list;
      gint new_width;

      /* run a "eel_clist_optimal_column_width" but break, if
       * the column doesn't shrink */
      if (EEL_CLIST_SHOW_TITLES (clist) && clist->column[column].button)
	new_width = (clist->column[column].button->requisition.width -
		     (CELL_SPACING + (2 * COLUMN_INSET)));
      else
	new_width = 0;

      for (list = clist->row_list; list; list = list->next)
	{
	  EEL_CLIST_CLASS_FW (clist)->cell_size_request
	    (clist, EEL_CLIST_ROW (list), column, &requisition);
	  new_width = MAX (new_width, requisition.width);
	  if (new_width == clist->column[column].width)
	    break;
	}
      if (new_width < clist->column[column].width)
	eel_clist_set_column_width (clist, column, new_width);
    }
}

static void
auto_resize_columns (EelCList *clist)
{
  gint i;

  if (EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    return;

  for (i = 0; i < clist->columns; i++)
    column_auto_resize (clist, NULL, i, clist->column[i].width);
}

static void
cell_size_request (EelCList       *clist,
		   EelCListRow    *clist_row,
		   gint            column,
		   GtkRequisition *requisition)
{
	EelCTree *ctree;
	GtkStyle *style;
	gint width;
	gint height;

	g_return_if_fail (clist != NULL);
	g_return_if_fail (EEL_IS_CTREE (clist));
	g_return_if_fail (requisition != NULL);

	ctree = EEL_CTREE (clist);

	eel_list_get_cell_style (EEL_LIST (clist), clist_row,
				      GTK_STATE_NORMAL, 0, column,
				      &style, NULL, NULL, NULL);

	switch (clist_row->cell[column].type)
	{
		case EEL_CELL_TEXT:
		case EEL_CELL_LINK_TEXT:
			requisition->width =
				gdk_string_width (style->font, EEL_CELL_TEXT (clist_row->cell[column])->text);
      			requisition->height = style->font->ascent + style->font->descent;
      			break;
      			
		case EEL_CELL_PIXTEXT:
			if (EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf) {
				width = gdk_pixbuf_get_width (EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf);
				height = gdk_pixbuf_get_height (EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf);
				width += EEL_CELL_PIXTEXT (clist_row->cell[column])->spacing;
			} else {
				width = height = 0;
			}
				  
			requisition->width = width + gdk_string_width (style->font,
								       EEL_CELL_TEXT (clist_row->cell[column])->text);
      			requisition->height = MAX (style->font->ascent + style->font->descent, height);

			if (column == ctree->tree_column) {
				requisition->width += (ctree->tree_spacing + ctree->tree_indent *
				 		      (((EelCTreeRow *) clist_row)->level - 1));
				requisition->width += PM_SIZE + 3;
			}

			if (ctree->line_style == EEL_CTREE_LINES_TABBED) {
				requisition->width += 3;
			}		
      			break;
      			
		case EEL_CELL_PIXBUF:
			width = gdk_pixbuf_get_width (EEL_CELL_PIXBUF (clist_row->cell[column])->pixbuf);
			height = gdk_pixbuf_get_height (EEL_CELL_PIXBUF (clist_row->cell[column])->pixbuf);
			requisition->width = width;
			requisition->height = height;
			break;
			
		default:
			requisition->width  = 0;
			requisition->height = 0;
			break;
	}

	requisition->width  += clist_row->cell[column].horizontal;
	requisition->height += clist_row->cell[column].vertical;
}

static gboolean
set_cell_contents (EelCList    *clist,
		   EelCListRow *clist_row,
		   gint         column,
		   EelCellType  type,
		   const gchar *text,
		   guint8       spacing,
		   GdkPixbuf   *pixbuf)
{
  gboolean visible = FALSE;
  EelCTree *ctree;
  GtkRequisition requisition;

  g_return_val_if_fail (EEL_IS_CTREE (clist), FALSE);
  g_return_val_if_fail (clist_row != NULL, FALSE);

  ctree = EEL_CTREE (clist);

  if (type == clist_row->cell[column].type)
    {
      switch (type)
	{
	case EEL_CELL_EMPTY:
	  return FALSE;
	case EEL_CELL_TEXT:
	case EEL_CELL_LINK_TEXT:
	  if (EEL_CELL_TEXT (clist_row->cell[column])->text == NULL)
	    {
	      if (text == NULL)
		return FALSE;
	    }
	  else
	    {
	      if (text != NULL && strcmp (EEL_CELL_TEXT (clist_row->cell[column])->text, text) == 0)
		return FALSE;
	    }
	  break;
	case EEL_CELL_PIXBUF:
	  if (pixbuf == EEL_CELL_PIXBUF (clist_row->cell[column])->pixbuf)
	    return FALSE;
	  break;
	case EEL_CELL_PIXTEXT:
	  if (pixbuf == EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf)
	    {
	      if (EEL_CELL_PIXTEXT (clist_row->cell[column])->text == NULL)
		{
		  if (text == NULL)
		    return FALSE;
		}
	      else
		{
		  if (text != NULL && strcmp (EEL_CELL_PIXTEXT (clist_row->cell[column])->text, text) == 0)
		    return FALSE;
		}
	    }
	  break;
	case EEL_CELL_WIDGET:
	  /* unimplemented */
	  break;
	case EEL_CELL_PIXBUF_LIST:
	  /* handled at the higher level */
	  break;
	}
    }

  if (clist->column[column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    {
      EelCTreeNode *parent;

      parent = ((EelCTreeRow *)clist_row)->parent;
      if (!parent || (parent && EEL_CTREE_ROW (parent)->expanded &&
		      eel_ctree_is_viewable (ctree, parent)))
	{
	  visible = TRUE;
	  EEL_CLIST_CLASS_FW (clist)->cell_size_request (clist, clist_row,
							 column, &requisition);
	}
    }

  switch (clist_row->cell[column].type)
    {
    case EEL_CELL_EMPTY:
      break;
    case EEL_CELL_TEXT:
    case EEL_CELL_LINK_TEXT:
      g_free (EEL_CELL_TEXT (clist_row->cell[column])->text);
      break;
    case EEL_CELL_PIXBUF:
      gdk_pixbuf_unref (EEL_CELL_PIXBUF (clist_row->cell[column])->pixbuf);
      break;
    case EEL_CELL_PIXTEXT:
      if (EEL_CELL_PIXTEXT (clist_row->cell[column])->text)
	g_free (EEL_CELL_PIXTEXT (clist_row->cell[column])->text);
      if (EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf)
	{
	  gdk_pixbuf_unref
	    (EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf);
	}
      break;
    case EEL_CELL_WIDGET:
      /* unimplimented */
      break;
      
    default:
      break;
    }

  clist_row->cell[column].type = EEL_CELL_EMPTY;
  if (column == ctree->tree_column && type != EEL_CELL_EMPTY)
    type = EEL_CELL_PIXTEXT;

  switch (type)
    {
    case EEL_CELL_TEXT:
    case EEL_CELL_LINK_TEXT:
      if (text)
	{
	  clist_row->cell[column].type = EEL_CELL_TEXT;
	  EEL_CELL_TEXT (clist_row->cell[column])->text = g_strdup (text);
	}
      break;
    case EEL_CELL_PIXBUF:
      if (pixbuf)
	{
	  clist_row->cell[column].type = EEL_CELL_PIXBUF;
	  EEL_CELL_PIXBUF (clist_row->cell[column])->pixbuf = gdk_pixbuf_ref (pixbuf);
	}
      break;
    case EEL_CELL_PIXTEXT:
      if (column == ctree->tree_column)
	{
	  if (pixbuf)
	    pixbuf = gdk_pixbuf_ref (pixbuf);
	  clist_row->cell[column].type = EEL_CELL_PIXTEXT;
	  EEL_CELL_PIXTEXT (clist_row->cell[column])->text = g_strdup (text);
	  EEL_CELL_PIXTEXT (clist_row->cell[column])->spacing = spacing;
	  EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf = pixbuf;
	}
      else if (text && pixbuf)
	{
	  clist_row->cell[column].type = EEL_CELL_PIXTEXT;
	  EEL_CELL_PIXTEXT (clist_row->cell[column])->text = g_strdup (text);
	  EEL_CELL_PIXTEXT (clist_row->cell[column])->spacing = spacing;
	  EEL_CELL_PIXTEXT (clist_row->cell[column])->pixbuf = gdk_pixbuf_ref (pixbuf);
	}
      break;
    default:
      break;
    }
  
  if (visible && clist->column[column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    column_auto_resize (clist, clist_row, column, requisition.width);

  return TRUE;
}

static void 
set_node_info (EelCTree     *ctree,
	       EelCTreeNode *node,
	       const gchar  *text,
	       guint8        spacing,
	       GdkPixbuf    *pixbuf_closed,
	       GdkPixbuf    *pixbuf_opened,
	       gboolean      is_leaf,
	       gboolean      expanded)
{
  if (EEL_CTREE_ROW (node)->pixbuf_opened)
    {
      gdk_pixbuf_unref (EEL_CTREE_ROW (node)->pixbuf_opened);
    }
  if (EEL_CTREE_ROW (node)->pixbuf_closed)
    {
      gdk_pixbuf_unref (EEL_CTREE_ROW (node)->pixbuf_closed);
    }

  EEL_CTREE_ROW (node)->pixbuf_opened = NULL;
  EEL_CTREE_ROW (node)->pixbuf_closed = NULL;

  if (pixbuf_closed)
    {
      EEL_CTREE_ROW (node)->pixbuf_closed = gdk_pixbuf_ref (pixbuf_closed);
    }
  if (pixbuf_opened)
    {
      EEL_CTREE_ROW (node)->pixbuf_opened = gdk_pixbuf_ref (pixbuf_opened);
    }

  EEL_CTREE_ROW (node)->is_leaf  = is_leaf;
  EEL_CTREE_ROW (node)->expanded = (is_leaf) ? FALSE : expanded;

  if (EEL_CTREE_ROW (node)->expanded)
    eel_ctree_node_set_pixtext (ctree, node, ctree->tree_column,
				text, spacing, pixbuf_opened);
  else 
    eel_ctree_node_set_pixtext (ctree, node, ctree->tree_column,
				text, spacing, pixbuf_closed);
}

static void
tree_delete (EelCTree     *ctree, 
	     EelCTreeNode *node, 
	     gpointer      data)
{
  tree_unselect (ctree,  node, NULL);
  row_delete (ctree, EEL_CTREE_ROW (node));
  g_list_free_1 ((GList *)node);
}

static void
tree_delete_row (EelCTree     *ctree, 
		 EelCTreeNode *node, 
		 gpointer      data)
{
  row_delete (ctree, EEL_CTREE_ROW (node));
  g_list_free_1 ((GList *)node);
}

static void
tree_update_level (EelCTree     *ctree, 
		   EelCTreeNode *node, 
		   gpointer      data)
{
  if (!node)
    return;

  if (EEL_CTREE_ROW (node)->parent)
      EEL_CTREE_ROW (node)->level = 
	EEL_CTREE_ROW (EEL_CTREE_ROW (node)->parent)->level + 1;
  else
      EEL_CTREE_ROW (node)->level = 1;
}

static void
tree_select (EelCTree     *ctree, 
	     EelCTreeNode *node, 
	     gpointer      data)
{
  if (node && EEL_CTREE_ROW (node)->row.state != GTK_STATE_SELECTED &&
      EEL_CTREE_ROW (node)->row.selectable)
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_SELECT_ROW],
		     node, -1);
}

static void
tree_unselect (EelCTree     *ctree, 
	       EelCTreeNode *node, 
	       gpointer      data)
{
  if (node && EEL_CTREE_ROW (node)->row.state == GTK_STATE_SELECTED)
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_UNSELECT_ROW], 
		     node, -1);
}

static void
tree_expand (EelCTree     *ctree, 
	     EelCTreeNode *node, 
	     gpointer      data)
{
  if (node && !EEL_CTREE_ROW (node)->expanded)
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_EXPAND], node);
}

static void
tree_collapse (EelCTree     *ctree, 
	       EelCTreeNode *node, 
	       gpointer      data)
{
  if (node && EEL_CTREE_ROW (node)->expanded)
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_COLLAPSE], node);
}

static void
tree_collapse_to_depth (EelCTree     *ctree, 
			EelCTreeNode *node, 
			gint          depth)
{
  if (node && EEL_CTREE_ROW (node)->level == depth)
    eel_ctree_collapse_recursive (ctree, node);
}

static void
tree_toggle_expansion (EelCTree     *ctree,
		       EelCTreeNode *node,
		       gpointer      data)
{
  if (!node)
    return;

  if (EEL_CTREE_ROW (node)->expanded)
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_COLLAPSE], node);
  else
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_EXPAND], node);
}

static EelCTreeRow *
row_new (EelCTree *ctree)
{
	EelCList *clist;
	EelCTreeRow *ctree_row;
	int i;

	clist = EEL_CLIST (ctree);
	ctree_row = g_chunk_new (EelCTreeRow, clist->row_mem_chunk);
	ctree_row->row.cell = g_chunk_new (EelCell, clist->cell_mem_chunk);

	for (i = 0; i < clist->columns; i++) {
		ctree_row->row.cell[i].type = EEL_CELL_EMPTY;
		ctree_row->row.cell[i].vertical = 0;
		ctree_row->row.cell[i].horizontal = 0;
		ctree_row->row.cell[i].style = NULL;
	}

	EEL_CELL_PIXTEXT (ctree_row->row.cell[ctree->tree_column])->text = NULL;

	ctree_row->row.fg_set     = FALSE;
	ctree_row->row.bg_set     = FALSE;
	ctree_row->row.style      = NULL;
	ctree_row->row.selectable = TRUE;
	ctree_row->row.state      = GTK_STATE_NORMAL;
	ctree_row->row.data       = NULL;
	ctree_row->row.destroy    = NULL;

	ctree_row->level         = 0;
	ctree_row->expanded      = FALSE;
	ctree_row->parent        = NULL;
	ctree_row->sibling       = NULL;
	ctree_row->children      = NULL;
	ctree_row->pixbuf_closed = NULL;
	ctree_row->pixbuf_opened = NULL;
	ctree_row->mouse_down    = FALSE;
	ctree_row->in_hotspot    = FALSE;
	
	return ctree_row;
}

static void
row_delete (EelCTree    *ctree,
	    EelCTreeRow *ctree_row)
{
  EelCList *clist;
  gint i;

  clist = EEL_CLIST (ctree);

  for (i = 0; i < clist->columns; i++)
    {
      EEL_CLIST_CLASS_FW (clist)->set_cell_contents
	(clist, &(ctree_row->row), i, EEL_CELL_EMPTY, NULL, 0, NULL);
      if (ctree_row->row.cell[i].style)
	{
	  if (GTK_WIDGET_REALIZED (ctree))
	    gtk_style_detach (ctree_row->row.cell[i].style);
	  gtk_style_unref (ctree_row->row.cell[i].style);
	}
    }

  if (ctree_row->row.style)
    {
      if (GTK_WIDGET_REALIZED (ctree))
	gtk_style_detach (ctree_row->row.style);
      gtk_style_unref (ctree_row->row.style);
    }

  if (ctree_row->pixbuf_closed)
    {
      gdk_pixbuf_unref (ctree_row->pixbuf_closed);
    }

  if (ctree_row->pixbuf_opened)
    {
      gdk_pixbuf_unref (ctree_row->pixbuf_opened);
    }

  if (ctree_row->row.destroy)
    {
      GtkDestroyNotify dnotify = ctree_row->row.destroy;
      gpointer ddata = ctree_row->row.data;

      ctree_row->row.destroy = NULL;
      ctree_row->row.data = NULL;

      dnotify (ddata);
    }

  g_mem_chunk_free (clist->cell_mem_chunk, ctree_row->row.cell);
  g_mem_chunk_free (clist->row_mem_chunk, ctree_row);
}

static void
real_tree_activate_row (EelCTree *ctree,
			EelCTreeNode *node,
			gint column)
{
	g_return_if_fail (ctree != NULL);
	g_return_if_fail (EEL_IS_CTREE (ctree));

	/* don't care */
}

static void
real_select_row (EelCList *clist,
		 gint      row,
		 gint      column,
		 GdkEvent *event)
{
  GList *node;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));
  
  if ((node = g_list_nth (clist->row_list, row)) &&
      EEL_CTREE_ROW (node)->row.selectable)
    gtk_signal_emit (GTK_OBJECT (clist), ctree_signals[TREE_SELECT_ROW],
		     node, column);
}

static void
real_unselect_row (EelCList *clist,
		   gint      row,
		   gint      column,
		   GdkEvent *event)
{
  GList *node;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  if ((node = g_list_nth (clist->row_list, row)))
    gtk_signal_emit (GTK_OBJECT (clist), ctree_signals[TREE_UNSELECT_ROW],
		     node, column);
}

static void
real_tree_select (EelCTree     *ctree,
		  EelCTreeNode *node,
		  gint          column)
{
  EelCList *clist;
  GList *list;
  EelCTreeNode *sel_row;
  gboolean node_selected;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (!node || EEL_CTREE_ROW (node)->row.state == GTK_STATE_SELECTED ||
      !EEL_CTREE_ROW (node)->row.selectable)
    return;

  clist = EEL_CLIST (ctree);

  switch (clist->selection_mode)
    {
    case GTK_SELECTION_SINGLE:
    case GTK_SELECTION_BROWSE:

      node_selected = FALSE;
      list = clist->selection;

      while (list)
	{
	  sel_row = list->data;
	  list = list->next;
	  
	  if (node == sel_row)
	    node_selected = TRUE;
	  else
	    gtk_signal_emit (GTK_OBJECT (ctree),
			     ctree_signals[TREE_UNSELECT_ROW], sel_row, column);
	}

      if (node_selected)
	return;

    default:
      break;
    }

  EEL_CTREE_ROW (node)->row.state = GTK_STATE_SELECTED;

  if (!clist->selection)
    {
      clist->selection = g_list_append (clist->selection, node);
      clist->selection_end = clist->selection;
    }
  else
    clist->selection_end = g_list_append (clist->selection_end, node)->next;

  tree_draw_node (ctree, node);
}

static void
real_tree_unselect (EelCTree     *ctree,
		    EelCTreeNode *node,
		    gint          column)
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (!node || EEL_CTREE_ROW (node)->row.state != GTK_STATE_SELECTED)
    return;

  clist = EEL_CLIST (ctree);

  if (clist->selection_end && clist->selection_end->data == node)
    clist->selection_end = clist->selection_end->prev;

  clist->selection = g_list_remove (clist->selection, node);
  
  EEL_CTREE_ROW (node)->row.state = GTK_STATE_NORMAL;

  tree_draw_node (ctree, node);
}

static void
select_row_recursive (EelCTree     *ctree, 
		      EelCTreeNode *node, 
		      gpointer      data)
{
  if (!node || EEL_CTREE_ROW (node)->row.state == GTK_STATE_SELECTED ||
      !EEL_CTREE_ROW (node)->row.selectable)
    return;

  EEL_CLIST (ctree)->undo_unselection = 
    g_list_prepend (EEL_CLIST (ctree)->undo_unselection, node);
  eel_ctree_select (ctree, node);
}

static void
real_select_all (EelCList *clist)
{
  EelCTree *ctree;
  EelCTreeNode *node;
  
  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  ctree = EEL_CTREE (clist);

  switch (clist->selection_mode)
    {
    case GTK_SELECTION_SINGLE:
    case GTK_SELECTION_BROWSE:
      return;

    case GTK_SELECTION_EXTENDED:
      eel_clist_freeze (clist);

      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
	  
      clist->anchor_state = GTK_STATE_SELECTED;
      clist->anchor = -1;
      clist->drag_pos = -1;
      clist->undo_anchor = clist->focus_row;

      for (node = EEL_CTREE_NODE (clist->row_list); node;
	   node = EEL_CTREE_NODE_NEXT (node))
	eel_ctree_pre_recursive (ctree, node, select_row_recursive, NULL);

      CLIST_REFRESH (clist);
      eel_clist_thaw (clist);
      break;

    case GTK_SELECTION_MULTIPLE:
      eel_ctree_select_recursive (ctree, NULL);
      break;

    default:
      /* do nothing */
      break;
    }
}

static void
real_unselect_all (EelCList *clist)
{
  EelCTree *ctree;
  EelCTreeNode *node;
  GList *list;
 
  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));
  
  ctree = EEL_CTREE (clist);

  switch (clist->selection_mode)
    {
    case GTK_SELECTION_BROWSE:
      if (clist->focus_row >= 0)
	{
	  eel_ctree_select
	    (ctree,
	     EEL_CTREE_NODE (g_list_nth (clist->row_list, clist->focus_row)));
	  return;
	}
      break;

    case GTK_SELECTION_EXTENDED:
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;

      clist->anchor = -1;
      clist->drag_pos = -1;
      clist->undo_anchor = clist->focus_row;
      break;

    default:
      break;
    }

  list = clist->selection;

  while (list)
    {
      node = list->data;
      list = list->next;
      eel_ctree_unselect (ctree, node);
    }
}

static gboolean
ctree_is_hot_spot (EelCTree     *ctree, 
		   EelCTreeNode *node,
		   gint          row, 
		   gint          x, 
		   gint          y)
{
	EelCTreeRow *tree_row;
	EelCList *clist;
	EelCellPixText *cell;
	gint xl;
	gint yu;
  
	g_return_val_if_fail (ctree != NULL, FALSE);
	g_return_val_if_fail (EEL_IS_CTREE (ctree), FALSE);
	g_return_val_if_fail (node != NULL, FALSE);

	clist = EEL_CLIST (ctree);

	if (!clist->column[ctree->tree_column].visible) {
		return FALSE;
	}

	tree_row = EEL_CTREE_ROW (node);

	cell = EEL_CELL_PIXTEXT(tree_row->row.cell[ctree->tree_column]);

	yu = (ROW_TOP_YPIXEL (clist, row) + (clist->row_height - PM_SIZE) / 2 - (clist->row_height - 1) % 2);

	if (clist->column[ctree->tree_column].justification == GTK_JUSTIFY_RIGHT) {
		xl = (clist->column[ctree->tree_column].area.x + 
	  	      clist->column[ctree->tree_column].area.width - 1 + clist->hoffset -
	  	     (tree_row->level - 1) * ctree->tree_indent - PM_SIZE -
	  	     (ctree->line_style == EEL_CTREE_LINES_TABBED) * 3);
	} else {
    		xl = (clist->column[ctree->tree_column].area.x + clist->hoffset +
	  	     (tree_row->level - 1) * ctree->tree_indent +
	  	     (ctree->line_style == EEL_CTREE_LINES_TABBED) * 3);
	}

	return (x >= xl - 3 && x <= xl + 3 + PM_SIZE && y >= yu - 3 && y <= yu + PM_SIZE + 3);
}

/***********************************************************
 ***********************************************************
 ***                  Public interface                   ***
 ***********************************************************
 ***********************************************************/


/***********************************************************
 *           Creation, insertion, deletion                 *
 ***********************************************************/

void
eel_ctree_construct (EelCTree    *ctree,
		     gint         columns, 
		     gint         tree_column,
		     gchar       *titles[])
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (GTK_OBJECT_CONSTRUCTED (ctree) == FALSE);

  clist = EEL_CLIST (ctree);

  clist->row_mem_chunk = g_mem_chunk_new ("ctree row mem chunk",
					  sizeof (EelCTreeRow),
					  sizeof (EelCTreeRow)
					  * CLIST_OPTIMUM_SIZE, 
					  G_ALLOC_AND_FREE);

  clist->cell_mem_chunk = g_mem_chunk_new ("ctree cell mem chunk",
					   sizeof (EelCell) * columns,
					   sizeof (EelCell) * columns
					   * CLIST_OPTIMUM_SIZE, 
					   G_ALLOC_AND_FREE);

  ctree->tree_column = tree_column;

  eel_clist_construct (clist, columns, titles);
}

GtkWidget *
eel_ctree_new_with_titles (gint         columns, 
			   gint         tree_column,
			   gchar       *titles[])
{
	GtkWidget *widget;

	g_return_val_if_fail (columns > 0, NULL);
	g_return_val_if_fail (tree_column >= 0 && tree_column < columns, NULL);

	widget = GTK_WIDGET (gtk_type_new (EEL_TYPE_CTREE));
	eel_ctree_construct (EEL_CTREE (widget), columns, tree_column, titles);

	return widget;
}

GtkWidget *
eel_ctree_new (gint columns, 
	       gint tree_column)
{
  return eel_ctree_new_with_titles (columns, tree_column, NULL);
}

static gint
real_insert_row (EelCList *clist,
		 gint      row,
		 gchar    *text[])
{
  EelCTreeNode *parent = NULL;
  EelCTreeNode *sibling;
  EelCTreeNode *node;

  g_return_val_if_fail (clist != NULL, -1);
  g_return_val_if_fail (EEL_IS_CTREE (clist), -1);

  sibling = EEL_CTREE_NODE (g_list_nth (clist->row_list, row));
  if (sibling)
    parent = EEL_CTREE_ROW (sibling)->parent;

  node = eel_ctree_insert_node (EEL_CTREE (clist), parent, sibling, text, 5,
				     NULL, NULL, TRUE, FALSE);

  if (EEL_CLIST_AUTO_SORT (clist) || !sibling)
    return g_list_position (clist->row_list, (GList *) node);
  
  return row;
}

EelCTreeNode * 
eel_ctree_insert_node (EelCTree     *ctree,
		       EelCTreeNode *parent, 
		       EelCTreeNode *sibling,
		       gchar        *text[],
		       guint8        spacing,
		       GdkPixbuf    *pixbuf_closed,
		       GdkPixbuf    *pixbuf_opened,
		       gboolean      is_leaf,
		       gboolean      expanded)
{
	EelCList *clist;
	EelCTreeRow *new_row;
	EelCTreeNode *node;
	GList *list;
	gint i;

	g_return_val_if_fail (ctree != NULL, NULL);
	g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);
	if (sibling) {
    		g_return_val_if_fail (EEL_CTREE_ROW (sibling)->parent == parent, NULL);
    	}

	if (parent && EEL_CTREE_ROW (parent)->is_leaf) {
    		return NULL;
    	}

	clist = EEL_CLIST (ctree);

	/* create the row */
	new_row = row_new (ctree);
	list = g_list_alloc ();
	list->data = new_row;
	node = EEL_CTREE_NODE (list);

  if (text)
    for (i = 0; i < clist->columns; i++)
      if (text[i] && i != ctree->tree_column)
	EEL_CLIST_CLASS_FW (clist)->set_cell_contents
	  (clist, &(new_row->row), i, EEL_CELL_TEXT, text[i], 0, NULL);

  set_node_info (ctree, node, text ?
		 text[ctree->tree_column] : NULL, spacing, pixbuf_closed,
		 pixbuf_opened, is_leaf, expanded);

  /* sorted insertion */
  if (EEL_CLIST_AUTO_SORT (clist))
    {
      if (parent)
	sibling = EEL_CTREE_ROW (parent)->children;
      else
	sibling = EEL_CTREE_NODE (clist->row_list);

      while (sibling && clist->compare
	     (clist, EEL_CTREE_ROW (node), EEL_CTREE_ROW (sibling)) > 0)
	sibling = EEL_CTREE_ROW (sibling)->sibling;

    }

  eel_ctree_link (ctree, node, parent, sibling, TRUE);

  if (text && !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist) &&
      eel_ctree_is_viewable (ctree, node))
    {
      for (i = 0; i < clist->columns; i++)
	if (clist->column[i].auto_resize)
	  column_auto_resize (clist, &(new_row->row), i, 0);
    }

  if (clist->rows == 1)
    {
      clist->focus_row = 0;
      if (clist->selection_mode == GTK_SELECTION_BROWSE)
	eel_ctree_select (ctree, node);
    }


  CLIST_REFRESH (clist);

  return node;
}

EelCTreeNode *
eel_ctree_insert_gnode (EelCTree          *ctree,
			EelCTreeNode      *parent,
			EelCTreeNode      *sibling,
			GNode             *gnode,
			EelCTreeGNodeFunc  func,
			gpointer           data)
{
  EelCList *clist;
  EelCTreeNode *cnode = NULL;
  EelCTreeNode *child = NULL;
  EelCTreeNode *new_child;
  GList *list;
  GNode *work;
  guint depth = 1;

  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);
  g_return_val_if_fail (gnode != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);
  if (sibling)
    g_return_val_if_fail (EEL_CTREE_ROW (sibling)->parent == parent, NULL);
  
  clist = EEL_CLIST (ctree);

  if (parent)
    depth = EEL_CTREE_ROW (parent)->level + 1;

  list = g_list_alloc ();
  list->data = row_new (ctree);
  cnode = EEL_CTREE_NODE (list);

  eel_clist_freeze (clist);

  set_node_info (ctree, cnode, "", 0, NULL, NULL, TRUE, FALSE);

  if (!func (ctree, depth, gnode, cnode, data))
    {
      tree_delete_row (ctree, cnode, NULL);
      return NULL;
    }

  if (EEL_CLIST_AUTO_SORT (clist))
    {
      if (parent)
	sibling = EEL_CTREE_ROW (parent)->children;
      else
	sibling = EEL_CTREE_NODE (clist->row_list);

      while (sibling && clist->compare
	     (clist, EEL_CTREE_ROW (cnode), EEL_CTREE_ROW (sibling)) > 0)
	sibling = EEL_CTREE_ROW (sibling)->sibling;
    }

  eel_ctree_link (ctree, cnode, parent, sibling, TRUE);

  for (work = g_node_last_child (gnode); work; work = work->prev)
    {
      new_child = eel_ctree_insert_gnode (ctree, cnode, child,
					       work, func, data);
      if (new_child)
	child = new_child;
    }	
  
  CLIST_REFRESH (clist);
  eel_clist_thaw (clist);

  return cnode;
}

GNode *
eel_ctree_export_to_gnode (EelCTree          *ctree,
			   GNode             *parent,
			   GNode             *sibling,
			   EelCTreeNode      *node,
			   EelCTreeGNodeFunc  func,
			   gpointer           data)
{
  EelCTreeNode *work;
  GNode *gnode;
  gint depth;

  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);
  g_return_val_if_fail (node != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);
  if (sibling)
    {
      g_return_val_if_fail (parent != NULL, NULL);
      g_return_val_if_fail (sibling->parent == parent, NULL);
    }

  gnode = g_node_new (NULL);
  depth = g_node_depth (parent) + 1;
  
  if (!func (ctree, depth, gnode, node, data))
    {
      g_node_destroy (gnode);
      return NULL;
    }

  if (parent)
    g_node_insert_before (parent, sibling, gnode);

  if (!EEL_CTREE_ROW (node)->is_leaf)
    {
      GNode *new_sibling = NULL;

      for (work = EEL_CTREE_ROW (node)->children; work;
	   work = EEL_CTREE_ROW (work)->sibling)
	new_sibling = eel_ctree_export_to_gnode (ctree, gnode, new_sibling,
						 work, func, data);

      g_node_reverse_children (gnode);
    }

  return gnode;
}
  
static void
real_remove_row (EelCList *clist,
		 gint      row)
{
  EelCTreeNode *node;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  node = EEL_CTREE_NODE (g_list_nth (clist->row_list, row));

  if (node)
    eel_ctree_remove_node (EEL_CTREE (clist), node);
}

void
eel_ctree_remove_node (EelCTree     *ctree, 
		       EelCTreeNode *node)
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  eel_clist_freeze (clist);

  if (node)
    {
      gboolean visible;

      visible = eel_ctree_is_viewable (ctree, node);
      eel_ctree_unlink (ctree, node, TRUE);
      eel_ctree_post_recursive (ctree, node, EEL_CTREE_FUNC (tree_delete),
				NULL);
      if (clist->selection_mode == GTK_SELECTION_BROWSE && !clist->selection &&
	  clist->focus_row >= 0)
	eel_clist_select_row (clist, clist->focus_row, -1);

      auto_resize_columns (clist);
    }
  else
    eel_clist_clear (clist);

  CLIST_REFRESH (clist);
  eel_clist_thaw (clist);
}

static void
real_clear (EelCList *clist)
{
  EelCTree *ctree;
  EelCTreeNode *work;
  EelCTreeNode *ptr;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  ctree = EEL_CTREE (clist);

  /* remove all rows */
  work = EEL_CTREE_NODE (clist->row_list);
  clist->row_list = NULL;
  clist->row_list_end = NULL;

  EEL_CLIST_SET_FLAG (clist, CLIST_AUTO_RESIZE_BLOCKED);
  while (work)
    {
      ptr = work;
      work = EEL_CTREE_ROW (work)->sibling;
      eel_ctree_post_recursive (ctree, ptr, EEL_CTREE_FUNC (tree_delete_row), 
				NULL);
    }
  EEL_CLIST_UNSET_FLAG (clist, CLIST_AUTO_RESIZE_BLOCKED);

  parent_class->clear (clist);
}


/***********************************************************
 *  Generic recursive functions, querying / finding tree   *
 *  information                                            *
 ***********************************************************/


void
eel_ctree_post_recursive (EelCTree     *ctree, 
			  EelCTreeNode *node,
			  EelCTreeFunc  func,
			  gpointer      data)
{
  EelCTreeNode *work;
  EelCTreeNode *tmp;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (func != NULL);

  if (node)
    work = EEL_CTREE_ROW (node)->children;
  else
    work = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (work)
    {
      tmp = EEL_CTREE_ROW (work)->sibling;
      eel_ctree_post_recursive (ctree, work, func, data);
      work = tmp;
    }

  if (node)
    func (ctree, node, data);
}

void
eel_ctree_post_recursive_to_depth (EelCTree     *ctree, 
				   EelCTreeNode *node,
				   gint          depth,
				   EelCTreeFunc  func,
				   gpointer      data)
{
  EelCTreeNode *work;
  EelCTreeNode *tmp;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (func != NULL);

  if (depth < 0)
    {
      eel_ctree_post_recursive (ctree, node, func, data);
      return;
    }

  if (node)
    work = EEL_CTREE_ROW (node)->children;
  else
    work = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  if (work && EEL_CTREE_ROW (work)->level <= depth)
    {
      while (work)
	{
	  tmp = EEL_CTREE_ROW (work)->sibling;
	  eel_ctree_post_recursive_to_depth (ctree, work, depth, func, data);
	  work = tmp;
	}
    }

  if (node && EEL_CTREE_ROW (node)->level <= depth)
    func (ctree, node, data);
}

void
eel_ctree_pre_recursive (EelCTree     *ctree, 
			 EelCTreeNode *node,
			 EelCTreeFunc  func,
			 gpointer      data)
{
  EelCTreeNode *work;
  EelCTreeNode *tmp;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (func != NULL);

  if (node)
    {
      work = EEL_CTREE_ROW (node)->children;
      func (ctree, node, data);
    }
  else
    work = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (work)
    {
      tmp = EEL_CTREE_ROW (work)->sibling;
      eel_ctree_pre_recursive (ctree, work, func, data);
      work = tmp;
    }
}

void
eel_ctree_pre_recursive_to_depth (EelCTree     *ctree, 
				  EelCTreeNode *node,
				  gint          depth, 
				  EelCTreeFunc  func,
				  gpointer      data)
{
  EelCTreeNode *work;
  EelCTreeNode *tmp;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (func != NULL);

  if (depth < 0)
    {
      eel_ctree_pre_recursive (ctree, node, func, data);
      return;
    }

  if (node)
    {
      work = EEL_CTREE_ROW (node)->children;
      if (EEL_CTREE_ROW (node)->level <= depth)
	func (ctree, node, data);
    }
  else
    work = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  if (work && EEL_CTREE_ROW (work)->level <= depth)
    {
      while (work)
	{
	  tmp = EEL_CTREE_ROW (work)->sibling;
	  eel_ctree_pre_recursive_to_depth (ctree, work, depth, func, data);
	  work = tmp;
	}
    }
}

gboolean
eel_ctree_is_viewable (EelCTree     *ctree, 
		       EelCTreeNode *node)
{ 
  EelCTreeRow *work;

  g_return_val_if_fail (ctree != NULL, FALSE);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), FALSE);
  g_return_val_if_fail (node != NULL, FALSE);

  work = EEL_CTREE_ROW (node);

  while (work->parent && EEL_CTREE_ROW (work->parent)->expanded)
    work = EEL_CTREE_ROW (work->parent);

  if (!work->parent)
    return TRUE;

  return FALSE;
}

EelCTreeNode * 
eel_ctree_last (EelCTree     *ctree,
		     EelCTreeNode *node)
{
  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);

  if (!node) 
    return NULL;

  while (EEL_CTREE_ROW (node)->sibling)
    node = EEL_CTREE_ROW (node)->sibling;
  
  if (EEL_CTREE_ROW (node)->children)
    return eel_ctree_last (ctree, EEL_CTREE_ROW (node)->children);
  
  return node;
}

EelCTreeNode *
eel_ctree_find_node_ptr (EelCTree    *ctree,
			 EelCTreeRow *ctree_row)
{
  EelCTreeNode *node;
  
  g_return_val_if_fail (ctree != NULL, FALSE);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), FALSE);
  g_return_val_if_fail (ctree_row != NULL, FALSE);
  
  if (ctree_row->parent)
    node = EEL_CTREE_ROW (ctree_row->parent)->children;
  else
    node = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (EEL_CTREE_ROW (node) != ctree_row)
    node = EEL_CTREE_ROW (node)->sibling;
  
  return node;
}

EelCTreeNode *
eel_ctree_node_nth (EelCTree *ctree,
			 int     row)
{
  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);

  if ((row < 0) || (row >= EEL_CLIST(ctree)->rows))
    return NULL;
 
  return EEL_CTREE_NODE (g_list_nth (EEL_CLIST (ctree)->row_list, row));
}

gboolean
eel_ctree_find (EelCTree     *ctree,
		     EelCTreeNode *node,
		     EelCTreeNode *child)
{
  if (!child)
    return FALSE;

  if (!node)
    node = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (node)
    {
      if (node == child) 
	return TRUE;
      if (EEL_CTREE_ROW (node)->children)
	{
	  if (eel_ctree_find (ctree, EEL_CTREE_ROW (node)->children, child))
	    return TRUE;
	}
      node = EEL_CTREE_ROW (node)->sibling;
    }
  return FALSE;
}

gboolean
eel_ctree_is_ancestor (EelCTree     *ctree,
		       EelCTreeNode *node,
		       EelCTreeNode *child)
{
  g_return_val_if_fail (EEL_IS_CTREE (ctree), FALSE);
  g_return_val_if_fail (node != NULL, FALSE);

  if (EEL_CTREE_ROW (node)->children)
    return eel_ctree_find (ctree, EEL_CTREE_ROW (node)->children, child);

  return FALSE;
}

EelCTreeNode *
eel_ctree_find_by_row_data (EelCTree     *ctree,
			    EelCTreeNode *node,
			    gpointer      data)
{
  EelCTreeNode *work;
  
  if (!node)
    node = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);
  
  while (node)
    {
      if (EEL_CTREE_ROW (node)->row.data == data) 
	return node;
      if (EEL_CTREE_ROW (node)->children &&
	  (work = eel_ctree_find_by_row_data 
	   (ctree, EEL_CTREE_ROW (node)->children, data)))
	return work;
      node = EEL_CTREE_ROW (node)->sibling;
    }
  return NULL;
}

GList *
eel_ctree_find_all_by_row_data (EelCTree     *ctree,
				EelCTreeNode *node,
				gpointer      data)
{
  GList *list = NULL;

  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);

  /* if node == NULL then look in the whole tree */
  if (!node)
    node = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (node)
    {
      if (EEL_CTREE_ROW (node)->row.data == data)
        list = g_list_append (list, node);

      if (EEL_CTREE_ROW (node)->children)
        {
	  GList *sub_list;

          sub_list = eel_ctree_find_all_by_row_data (ctree,
							  EEL_CTREE_ROW
							  (node)->children,
							  data);
          list = g_list_concat (list, sub_list);
        }
      node = EEL_CTREE_ROW (node)->sibling;
    }
  return list;
}

EelCTreeNode *
eel_ctree_find_by_row_data_custom (EelCTree     *ctree,
				   EelCTreeNode *node,
				   gpointer      data,
				   GCompareFunc  func)
{
  EelCTreeNode *work;

  g_return_val_if_fail (func != NULL, NULL);

  if (!node)
    node = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (node)
    {
      if (!func (EEL_CTREE_ROW (node)->row.data, data))
	return node;
      if (EEL_CTREE_ROW (node)->children &&
	  (work = eel_ctree_find_by_row_data_custom
	   (ctree, EEL_CTREE_ROW (node)->children, data, func)))
	return work;
      node = EEL_CTREE_ROW (node)->sibling;
    }
  return NULL;
}

GList *
eel_ctree_find_all_by_row_data_custom (EelCTree     *ctree,
				       EelCTreeNode *node,
				       gpointer      data,
				       GCompareFunc  func)
{
  GList *list = NULL;

  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);
  g_return_val_if_fail (func != NULL, NULL);

  /* if node == NULL then look in the whole tree */
  if (!node)
    node = EEL_CTREE_NODE (EEL_CLIST (ctree)->row_list);

  while (node)
    {
      if (!func (EEL_CTREE_ROW (node)->row.data, data))
        list = g_list_append (list, node);

      if (EEL_CTREE_ROW (node)->children)
        {
	  GList *sub_list;

          sub_list = eel_ctree_find_all_by_row_data_custom (ctree,
							    EEL_CTREE_ROW
							    (node)->children,
							    data,
							    func);
          list = g_list_concat (list, sub_list);
        }
      node = EEL_CTREE_ROW (node)->sibling;
    }
  return list;
}

gboolean
eel_ctree_is_hot_spot (EelCTree *ctree, 
		       	    gint      x, 
		       	    gint      y)
{
	EelCTreeNode *node;
	gint column;
	gint row;
  
	g_return_val_if_fail (ctree != NULL, FALSE);
	g_return_val_if_fail (EEL_IS_CTREE (ctree), FALSE);

	if (eel_clist_get_selection_info (EEL_CLIST (ctree), x, y, &row, &column)) {
		if ((node = EEL_CTREE_NODE(g_list_nth (EEL_CLIST (ctree)->row_list, row)))) {
			return ctree_is_hot_spot (ctree, node, row, x, y);
		}
	}
	
	return FALSE;
}


/***********************************************************
 *   Tree signals : move, expand, collapse, (un)select     *
 ***********************************************************/


void
eel_ctree_move (EelCTree     *ctree,
		EelCTreeNode *node,
		EelCTreeNode *new_parent, 
		EelCTreeNode *new_sibling)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);
  
  gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_MOVE], node,
		   new_parent, new_sibling);
}

void
eel_ctree_expand (EelCTree     *ctree,
		  EelCTreeNode *node)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);
  
  if (EEL_CTREE_ROW (node)->is_leaf)
    return;

  gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_EXPAND], node);
}

void 
eel_ctree_expand_recursive (EelCTree     *ctree,
			    EelCTreeNode *node)
{
  EelCList *clist;
  gboolean thaw = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  if (node && EEL_CTREE_ROW (node)->is_leaf)
    return;

  if (CLIST_UNFROZEN (clist) && (!node || eel_ctree_is_viewable (ctree, node)))
    {
      eel_clist_freeze (clist);
      thaw = TRUE;
    }

  eel_ctree_post_recursive (ctree, node, EEL_CTREE_FUNC (tree_expand), NULL);

  CLIST_REFRESH (clist);
  if (thaw)
    eel_clist_thaw (clist);
}

void 
eel_ctree_expand_to_depth (EelCTree     *ctree,
			   EelCTreeNode *node,
			   gint          depth)
{
  EelCList *clist;
  gboolean thaw = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  if (node && EEL_CTREE_ROW (node)->is_leaf)
    return;

  if (CLIST_UNFROZEN (clist) && (!node || eel_ctree_is_viewable (ctree, node)))
    {
      eel_clist_freeze (clist);
      thaw = TRUE;
    }

  eel_ctree_post_recursive_to_depth (ctree, node, depth,
				     EEL_CTREE_FUNC (tree_expand), NULL);

  CLIST_REFRESH (clist);
  if (thaw)
    eel_clist_thaw (clist);
}

void
eel_ctree_collapse (EelCTree     *ctree,
		    EelCTreeNode *node)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);
  
  if (EEL_CTREE_ROW (node)->is_leaf)
    return;

  gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_COLLAPSE], node);
}

void 
eel_ctree_collapse_recursive (EelCTree     *ctree,
			      EelCTreeNode *node)
{
  EelCList *clist;
  gboolean thaw = FALSE;
  gint i;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (node && EEL_CTREE_ROW (node)->is_leaf)
    return;

  clist = EEL_CLIST (ctree);

  if (CLIST_UNFROZEN (clist) && (!node || eel_ctree_is_viewable (ctree, node)))
    {
      eel_clist_freeze (clist);
      thaw = TRUE;
    }

  EEL_CLIST_SET_FLAG (clist, CLIST_AUTO_RESIZE_BLOCKED);
  eel_ctree_post_recursive (ctree, node, EEL_CTREE_FUNC (tree_collapse), NULL);
  EEL_CLIST_UNSET_FLAG (clist, CLIST_AUTO_RESIZE_BLOCKED);
  for (i = 0; i < clist->columns; i++)
    if (clist->column[i].auto_resize)
      eel_clist_set_column_width (clist, i,
				  eel_clist_optimal_column_width (clist, i));

  CLIST_REFRESH (clist);
  if (thaw)
    eel_clist_thaw (clist);
}

void 
eel_ctree_collapse_to_depth (EelCTree     *ctree,
			     EelCTreeNode *node,
			     gint          depth)
{
  EelCList *clist;
  gboolean thaw = FALSE;
  gint i;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (node && EEL_CTREE_ROW (node)->is_leaf)
    return;

  clist = EEL_CLIST (ctree);

  if (CLIST_UNFROZEN (clist) && (!node || eel_ctree_is_viewable (ctree, node)))
    {
      eel_clist_freeze (clist);
      thaw = TRUE;
    }

  EEL_CLIST_SET_FLAG (clist, CLIST_AUTO_RESIZE_BLOCKED);
  eel_ctree_post_recursive_to_depth (ctree, node, depth,
				     EEL_CTREE_FUNC (tree_collapse_to_depth),
				     GINT_TO_POINTER (depth));
  EEL_CLIST_UNSET_FLAG (clist, CLIST_AUTO_RESIZE_BLOCKED);
  for (i = 0; i < clist->columns; i++)
    if (clist->column[i].auto_resize)
      eel_clist_set_column_width (clist, i,
				  eel_clist_optimal_column_width (clist, i));

  CLIST_REFRESH (clist);
  if (thaw)
    eel_clist_thaw (clist);
}

void
eel_ctree_toggle_expansion (EelCTree     *ctree,
			    EelCTreeNode *node)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);
  
  if (EEL_CTREE_ROW (node)->is_leaf)
    return;

  tree_toggle_expansion (ctree, node, NULL);
}

void 
eel_ctree_toggle_expansion_recursive (EelCTree     *ctree,
				      EelCTreeNode *node)
{
  EelCList *clist;
  gboolean thaw = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  
  if (node && EEL_CTREE_ROW (node)->is_leaf)
    return;

  clist = EEL_CLIST (ctree);

  if (CLIST_UNFROZEN (clist) && (!node || eel_ctree_is_viewable (ctree, node)))
    {
      eel_clist_freeze (clist);
      thaw = TRUE;
    }
  
  eel_ctree_post_recursive (ctree, node,
			    EEL_CTREE_FUNC (tree_toggle_expansion), NULL);

  CLIST_REFRESH (clist);
  if (thaw)
    eel_clist_thaw (clist);
}

void
eel_ctree_select (EelCTree     *ctree, 
		  EelCTreeNode *node)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  if (EEL_CTREE_ROW (node)->row.selectable)
    gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_SELECT_ROW],
		     node, -1);
}

void
eel_ctree_unselect (EelCTree     *ctree, 
		    EelCTreeNode *node)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  gtk_signal_emit (GTK_OBJECT (ctree), ctree_signals[TREE_UNSELECT_ROW],
		   node, -1);
}

void
eel_ctree_select_recursive (EelCTree     *ctree, 
			    EelCTreeNode *node)
{
  eel_ctree_real_select_recursive (ctree, node, TRUE);
}

void
eel_ctree_unselect_recursive (EelCTree     *ctree, 
			      EelCTreeNode *node)
{
  eel_ctree_real_select_recursive (ctree, node, FALSE);
}

void
eel_ctree_real_select_recursive (EelCTree     *ctree, 
				 EelCTreeNode *node, 
				 gint          state)
{
  EelCList *clist;
  gboolean thaw = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  if ((state && 
       (clist->selection_mode ==  GTK_SELECTION_BROWSE ||
	clist->selection_mode == GTK_SELECTION_SINGLE)) ||
      (!state && clist->selection_mode ==  GTK_SELECTION_BROWSE))
    return;

  if (CLIST_UNFROZEN (clist) && (!node || eel_ctree_is_viewable (ctree, node)))
    {
      eel_clist_freeze (clist);
      thaw = TRUE;
    }

  if (clist->selection_mode == GTK_SELECTION_EXTENDED)
    {
      EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
      
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
    }

  if (state)
    eel_ctree_post_recursive (ctree, node,
			      EEL_CTREE_FUNC (tree_select), NULL);
  else 
    eel_ctree_post_recursive (ctree, node,
			      EEL_CTREE_FUNC (tree_unselect), NULL);
  
  CLIST_REFRESH (clist);
  if (thaw)
    eel_clist_thaw (clist);
}


/***********************************************************
 *           Analogons of EelCList functions               *
 ***********************************************************/


void 
eel_ctree_node_set_text (EelCTree     *ctree,
			      EelCTreeNode *node,
			      gint          column,
			      const gchar  *text)
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return;
  
  clist = EEL_CLIST (ctree);

  if (EEL_CLIST_CLASS_FW (clist)->set_cell_contents
      (clist, &(EEL_CTREE_ROW(node)->row), column, EEL_CELL_TEXT,
       text, 0, NULL))
	  tree_draw_node (ctree, node);
}

void 
eel_ctree_node_set_pixbuf (EelCTree     *ctree,
			   EelCTreeNode *node,
			   gint          column,
			   GdkPixbuf    *pixbuf)
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);
  g_return_if_fail (pixbuf != NULL);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return;

  clist = EEL_CLIST (ctree);

  if (EEL_CLIST_CLASS_FW (clist)->set_cell_contents
      (clist, &(EEL_CTREE_ROW (node)->row), column, EEL_CELL_PIXBUF,
       NULL, 0, pixbuf))
	  tree_draw_node (ctree, node);
}

void 
eel_ctree_node_set_pixtext (EelCTree     *ctree,
			    EelCTreeNode *node,
			    gint          column,
			    const gchar  *text,
			    guint8        spacing,
			    GdkPixbuf    *pixbuf)
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);
  if (column != ctree->tree_column)
    g_return_if_fail (pixbuf != NULL);
  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return;

  clist = EEL_CLIST (ctree);

  if (EEL_CLIST_CLASS_FW (clist)->set_cell_contents
      (clist, &(EEL_CTREE_ROW (node)->row), column, EEL_CELL_PIXTEXT,
       text, spacing, pixbuf))
	  tree_draw_node (ctree, node);
}

void 
eel_ctree_set_node_info (EelCTree     *ctree,
			 EelCTreeNode *node,
			 const gchar  *text,
			 guint8        spacing,
			 GdkPixbuf    *pixbuf_closed,
			 GdkPixbuf    *pixbuf_opened,
			 gboolean      is_leaf,
			 gboolean      expanded)
{
  gboolean old_leaf;
  gboolean old_expanded;
 
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  old_leaf = EEL_CTREE_ROW (node)->is_leaf;
  old_expanded = EEL_CTREE_ROW (node)->expanded;

  if (is_leaf && EEL_CTREE_ROW (node)->children)
    {
      EelCTreeNode *work;
      EelCTreeNode *ptr;
      
      work = EEL_CTREE_ROW (node)->children;
      while (work)
	{
	  ptr = work;
	  work = EEL_CTREE_ROW(work)->sibling;
	  eel_ctree_remove_node (ctree, ptr);
	}
    }

  set_node_info (ctree, node, text, spacing, pixbuf_closed,
		 pixbuf_opened, is_leaf, expanded);

  if (!is_leaf && !old_leaf)
    {
      EEL_CTREE_ROW (node)->expanded = old_expanded;
      if (expanded && !old_expanded)
	eel_ctree_expand (ctree, node);
      else if (!expanded && old_expanded)
	eel_ctree_collapse (ctree, node);
    }

  EEL_CTREE_ROW (node)->expanded = (is_leaf) ? FALSE : expanded;
  
  if (EEL_CLIST_AUTO_SORT (EEL_CLIST (ctree))
      && EEL_CTREE_ROW (node)->parent != NULL)
    {
      eel_ctree_sort_single_node (ctree, EEL_CTREE_ROW (node)->parent);
    }
}

void
eel_ctree_node_set_shift (EelCTree     *ctree,
			  EelCTreeNode *node,
			  gint          column,
			  gint          vertical,
			  gint          horizontal)
{
  EelCList *clist;
  GtkRequisition requisition;
  gboolean visible = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return;

  clist = EEL_CLIST (ctree);

  if (clist->column[column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    {
      visible = eel_ctree_is_viewable (ctree, node);
      if (visible)
	EEL_CLIST_CLASS_FW (clist)->cell_size_request
	  (clist, &EEL_CTREE_ROW (node)->row, column, &requisition);
    }

  EEL_CTREE_ROW (node)->row.cell[column].vertical   = vertical;
  EEL_CTREE_ROW (node)->row.cell[column].horizontal = horizontal;

  if (visible)
    column_auto_resize (clist, &EEL_CTREE_ROW (node)->row,
			column, requisition.width);

  tree_draw_node (ctree, node);
}

static void
remove_grab (EelCList *clist)
{
  if (gdk_pointer_is_grabbed () && GTK_WIDGET_HAS_GRAB (clist))
    {
      gtk_grab_remove (GTK_WIDGET (clist));
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }

  if (clist->htimer)
    {
      gtk_timeout_remove (clist->htimer);
      clist->htimer = 0;
    }

  if (clist->vtimer)
    {
      gtk_timeout_remove (clist->vtimer);
      clist->vtimer = 0;
    }
}

void
eel_ctree_node_set_selectable (EelCTree     *ctree,
			       EelCTreeNode *node,
			       gboolean      selectable)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  if (selectable == EEL_CTREE_ROW (node)->row.selectable)
    return;

  EEL_CTREE_ROW (node)->row.selectable = selectable;

  if (!selectable && EEL_CTREE_ROW (node)->row.state == GTK_STATE_SELECTED)
    {
      EelCList *clist;

      clist = EEL_CLIST (ctree);

      if (clist->anchor >= 0 &&
	  clist->selection_mode == GTK_SELECTION_EXTENDED)
	{
	  clist->drag_button = 0;
	  remove_grab (clist);

	  EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
	}
      eel_ctree_unselect (ctree, node);
    }      
}

gboolean
eel_ctree_node_get_selectable (EelCTree     *ctree,
			       EelCTreeNode *node)
{
  g_return_val_if_fail (node != NULL, FALSE);

  return EEL_CTREE_ROW (node)->row.selectable;
}

EelCellType 
eel_ctree_node_get_cell_type (EelCTree     *ctree,
			      EelCTreeNode *node,
			      gint          column)
{
  g_return_val_if_fail (ctree != NULL, -1);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), -1);
  g_return_val_if_fail (node != NULL, -1);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return -1;

  return EEL_CTREE_ROW (node)->row.cell[column].type;
}

gint
eel_ctree_node_get_text (EelCTree      *ctree,
			 EelCTreeNode  *node,
			 gint           column,
			 gchar        **text)
{
  g_return_val_if_fail (ctree != NULL, 0);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), 0);
  g_return_val_if_fail (node != NULL, 0);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return 0;

  if (EEL_CTREE_ROW (node)->row.cell[column].type != EEL_CELL_TEXT
      && EEL_CTREE_ROW (node)->row.cell[column].type != EEL_CELL_LINK_TEXT)
    return 0;

  if (text)
    *text = EEL_CELL_TEXT (EEL_CTREE_ROW (node)->row.cell[column])->text;

  return 1;
}

gint
eel_ctree_node_get_pixbuf (EelCTree     *ctree,
			   EelCTreeNode *node,
			   gint          column,
			   GdkPixbuf   **pixbuf)
{
  g_return_val_if_fail (ctree != NULL, 0);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), 0);
  g_return_val_if_fail (node != NULL, 0);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return 0;

  if (EEL_CTREE_ROW (node)->row.cell[column].type != EEL_CELL_PIXBUF)
    return 0;

  if (pixbuf)
    *pixbuf = EEL_CELL_PIXBUF (EEL_CTREE_ROW(node)->row.cell[column])->pixbuf;

  return 1;
}

gint
eel_ctree_node_get_pixtext (EelCTree      *ctree,
			    EelCTreeNode  *node,
			    gint           column,
			    gchar        **text,
			    guint8        *spacing,
			    GdkPixbuf    **pixbuf)
{
  g_return_val_if_fail (ctree != NULL, 0);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), 0);
  g_return_val_if_fail (node != NULL, 0);
  
  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return 0;
  
  if (EEL_CTREE_ROW (node)->row.cell[column].type != EEL_CELL_PIXTEXT)
    return 0;
  
  if (text)
    *text = EEL_CELL_PIXTEXT (EEL_CTREE_ROW (node)->row.cell[column])->text;
  if (spacing)
    *spacing = EEL_CELL_PIXTEXT (EEL_CTREE_ROW 
				 (node)->row.cell[column])->spacing;
  if (pixbuf)
    *pixbuf = EEL_CELL_PIXTEXT (EEL_CTREE_ROW 
				(node)->row.cell[column])->pixbuf;
  
  return 1;
}

gint
eel_ctree_get_node_info (EelCTree      *ctree,
			 EelCTreeNode  *node,
			 gchar        **text,
			 guint8        *spacing,
			 GdkPixbuf    **pixbuf_closed,
			 GdkPixbuf    **pixbuf_opened,
			 gboolean      *is_leaf,
			 gboolean      *expanded)
{
  g_return_val_if_fail (ctree != NULL, 0);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), 0);
  g_return_val_if_fail (node != NULL, 0);
  
  if (text)
    *text = EEL_CELL_PIXTEXT 
      (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->text;
  if (spacing)
    *spacing = EEL_CELL_PIXTEXT 
      (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->spacing;
  if (pixbuf_closed)
    *pixbuf_closed = EEL_CTREE_ROW (node)->pixbuf_closed;
  if (pixbuf_opened)
    *pixbuf_opened = EEL_CTREE_ROW (node)->pixbuf_opened;
  if (is_leaf)
    *is_leaf = EEL_CTREE_ROW (node)->is_leaf;
  if (expanded)
    *expanded = EEL_CTREE_ROW (node)->expanded;
  
  return 1;
}

void
eel_ctree_node_set_cell_style (EelCTree     *ctree,
			       EelCTreeNode *node,
			       gint          column,
			       GtkStyle     *style)
{
  EelCList *clist;
  GtkRequisition requisition;
  gboolean visible = FALSE;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  clist = EEL_CLIST (ctree);

  if (column < 0 || column >= clist->columns)
    return;

  if (EEL_CTREE_ROW (node)->row.cell[column].style == style)
    return;

  if (clist->column[column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    {
      visible = eel_ctree_is_viewable (ctree, node);
      if (visible)
	EEL_CLIST_CLASS_FW (clist)->cell_size_request
	  (clist, &EEL_CTREE_ROW (node)->row, column, &requisition);
    }

  if (EEL_CTREE_ROW (node)->row.cell[column].style)
    {
      if (GTK_WIDGET_REALIZED (ctree))
        gtk_style_detach (EEL_CTREE_ROW (node)->row.cell[column].style);
      gtk_style_unref (EEL_CTREE_ROW (node)->row.cell[column].style);
    }

  EEL_CTREE_ROW (node)->row.cell[column].style = style;

  if (EEL_CTREE_ROW (node)->row.cell[column].style)
    {
      gtk_style_ref (EEL_CTREE_ROW (node)->row.cell[column].style);
      
      if (GTK_WIDGET_REALIZED (ctree))
        EEL_CTREE_ROW (node)->row.cell[column].style =
	  gtk_style_attach (EEL_CTREE_ROW (node)->row.cell[column].style,
			    clist->clist_window);
    }

  if (visible)
    column_auto_resize (clist, &EEL_CTREE_ROW (node)->row, column,
			requisition.width);

  tree_draw_node (ctree, node);
}

GtkStyle *
eel_ctree_node_get_cell_style (EelCTree     *ctree,
			       EelCTreeNode *node,
			       gint          column)
{
  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);
  g_return_val_if_fail (node != NULL, NULL);

  if (column < 0 || column >= EEL_CLIST (ctree)->columns)
    return NULL;

  return EEL_CTREE_ROW (node)->row.cell[column].style;
}

void
eel_ctree_node_set_row_style (EelCTree     *ctree,
			      EelCTreeNode *node,
			      GtkStyle     *style)
{
  EelCList *clist;
  GtkRequisition requisition;
  gboolean visible;
  gint *old_width = NULL;
  gint i;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  clist = EEL_CLIST (ctree);

  if (EEL_CTREE_ROW (node)->row.style == style)
    return;
  
  visible = eel_ctree_is_viewable (ctree, node);
  if (visible && !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    {
      old_width = g_new (gint, clist->columns);
      for (i = 0; i < clist->columns; i++)
	if (clist->column[i].auto_resize)
	  {
	    EEL_CLIST_CLASS_FW (clist)->cell_size_request
	      (clist, &EEL_CTREE_ROW (node)->row, i, &requisition);
	    old_width[i] = requisition.width;
	  }
    }

  if (EEL_CTREE_ROW (node)->row.style)
    {
      if (GTK_WIDGET_REALIZED (ctree))
        gtk_style_detach (EEL_CTREE_ROW (node)->row.style);
      gtk_style_unref (EEL_CTREE_ROW (node)->row.style);
    }

  EEL_CTREE_ROW (node)->row.style = style;

  if (EEL_CTREE_ROW (node)->row.style)
    {
      gtk_style_ref (EEL_CTREE_ROW (node)->row.style);
      
      if (GTK_WIDGET_REALIZED (ctree))
        EEL_CTREE_ROW (node)->row.style =
	  gtk_style_attach (EEL_CTREE_ROW (node)->row.style,
			    clist->clist_window);
    }

  if (visible && !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    {
      for (i = 0; i < clist->columns; i++)
	if (clist->column[i].auto_resize)
	  column_auto_resize (clist, &EEL_CTREE_ROW (node)->row, i,
			      old_width[i]);
      g_free (old_width);
    }
  tree_draw_node (ctree, node);
}

GtkStyle *
eel_ctree_node_get_row_style (EelCTree     *ctree,
			      EelCTreeNode *node)
{
  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);
  g_return_val_if_fail (node != NULL, NULL);

  return EEL_CTREE_ROW (node)->row.style;
}

void
eel_ctree_node_set_foreground (EelCTree     *ctree,
			       EelCTreeNode *node,
			       GdkColor     *color)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  if (color)
    {
      EEL_CTREE_ROW (node)->row.foreground = *color;
      EEL_CTREE_ROW (node)->row.fg_set = TRUE;
      if (GTK_WIDGET_REALIZED (ctree))
	gdk_color_alloc (gtk_widget_get_colormap (GTK_WIDGET (ctree)),
			 &EEL_CTREE_ROW (node)->row.foreground);
    }
  else
    EEL_CTREE_ROW (node)->row.fg_set = FALSE;

  tree_draw_node (ctree, node);
}

void
eel_ctree_node_set_background (EelCTree     *ctree,
			       EelCTreeNode *node,
			       GdkColor     *color)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  if (color)
    {
      EEL_CTREE_ROW (node)->row.background = *color;
      EEL_CTREE_ROW (node)->row.bg_set = TRUE;
      if (GTK_WIDGET_REALIZED (ctree))
	gdk_color_alloc (gtk_widget_get_colormap (GTK_WIDGET (ctree)),
			 &EEL_CTREE_ROW (node)->row.background);
    }
  else
    EEL_CTREE_ROW (node)->row.bg_set = FALSE;

  tree_draw_node (ctree, node);
}

void
eel_ctree_node_set_row_data (EelCTree     *ctree,
			     EelCTreeNode *node,
			     gpointer      data)
{
  eel_ctree_node_set_row_data_full (ctree, node, data, NULL);
}

void
eel_ctree_node_set_row_data_full (EelCTree         *ctree,
				  EelCTreeNode     *node,
				  gpointer          data,
				  GtkDestroyNotify  destroy)
{
  GtkDestroyNotify dnotify;
  gpointer ddata;
  
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (node != NULL);

  dnotify = EEL_CTREE_ROW (node)->row.destroy;
  ddata = EEL_CTREE_ROW (node)->row.data;
  
  EEL_CTREE_ROW (node)->row.data = data;
  EEL_CTREE_ROW (node)->row.destroy = destroy;

  if (dnotify)
    dnotify (ddata);
}

gpointer
eel_ctree_node_get_row_data (EelCTree     *ctree,
			     EelCTreeNode *node)
{
  g_return_val_if_fail (ctree != NULL, NULL);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), NULL);

  return node ? EEL_CTREE_ROW (node)->row.data : NULL;
}

void
eel_ctree_node_moveto (EelCTree     *ctree,
		       EelCTreeNode *node,
		       gint          column,
		       gfloat        row_align,
		       gfloat        col_align)
{
  gint row = -1;
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  while (node && !eel_ctree_is_viewable (ctree, node))
    node = EEL_CTREE_ROW (node)->parent;

  if (node)
    row = g_list_position (clist->row_list, (GList *)node);
  
  eel_clist_moveto (clist, row, column, row_align, col_align);
}

GtkVisibility eel_ctree_node_is_visible (EelCTree     *ctree,
                                         EelCTreeNode *node)
{
  gint row;
  
  g_return_val_if_fail (ctree != NULL, 0);
  g_return_val_if_fail (node != NULL, 0);
  
  row = g_list_position (EEL_CLIST (ctree)->row_list, (GList*) node);
  return eel_clist_row_is_visible (EEL_CLIST (ctree), row);
}


/***********************************************************
 *             EelCTree specific functions                 *
 ***********************************************************/

void
eel_ctree_set_indent (EelCTree *ctree, 
                      gint      indent)
{
  EelCList *clist;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (indent >= 0);

  if (indent == ctree->tree_indent)
    return;

  clist = EEL_CLIST (ctree);
  ctree->tree_indent = indent;

  if (clist->column[ctree->tree_column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    eel_clist_set_column_width
      (clist, ctree->tree_column,
       eel_clist_optimal_column_width (clist, ctree->tree_column));
  else
    CLIST_REFRESH (clist);
}

void
eel_ctree_set_spacing (EelCTree *ctree, 
		       gint      spacing)
{
  EelCList *clist;
  gint old_spacing;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));
  g_return_if_fail (spacing >= 0);

  if (spacing == ctree->tree_spacing)
    return;

  clist = EEL_CLIST (ctree);

  old_spacing = ctree->tree_spacing;
  ctree->tree_spacing = spacing;

  if (clist->column[ctree->tree_column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    eel_clist_set_column_width (clist, ctree->tree_column,
				clist->column[ctree->tree_column].width +
				spacing - old_spacing);
  else
    CLIST_REFRESH (clist);
}

void
eel_ctree_set_show_stub (EelCTree *ctree, 
			 gboolean  show_stub)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  show_stub = show_stub != FALSE;

  if (show_stub != ctree->show_stub)
    {
      EelCList *clist;

      clist = EEL_CLIST (ctree);
      ctree->show_stub = show_stub;

      if (CLIST_UNFROZEN (clist) && clist->rows &&
	  eel_clist_row_is_visible (clist, 0) != GTK_VISIBILITY_NONE)
	EEL_CLIST_CLASS_FW (clist)->draw_row
	  (clist, NULL, 0, EEL_CLIST_ROW (clist->row_list));
    }
}

void 
eel_ctree_set_line_style (EelCTree          *ctree, 
			  EelCTreeLineStyle  line_style)
{
  EelCList *clist;
  EelCTreeLineStyle old_style;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  if (line_style == ctree->line_style)
    return;

  clist = EEL_CLIST (ctree);

  old_style = ctree->line_style;
  ctree->line_style = line_style;

  if (clist->column[ctree->tree_column].auto_resize &&
      !EEL_CLIST_AUTO_RESIZE_BLOCKED (clist))
    {
      if (old_style == EEL_CTREE_LINES_TABBED)
	eel_clist_set_column_width
	  (clist, ctree->tree_column,
	   clist->column[ctree->tree_column].width - 3);
      else if (line_style == EEL_CTREE_LINES_TABBED)
	eel_clist_set_column_width
	  (clist, ctree->tree_column,
	   clist->column[ctree->tree_column].width + 3);
    }

  if (GTK_WIDGET_REALIZED (ctree))
    {
      switch (line_style)
	{
	case EEL_CTREE_LINES_SOLID:
	  if (GTK_WIDGET_REALIZED (ctree))
	    gdk_gc_set_line_attributes (ctree->lines_gc, 1, GDK_LINE_SOLID, 
					None, None);
	  break;
	case EEL_CTREE_LINES_DOTTED:
	  if (GTK_WIDGET_REALIZED (ctree))
	    gdk_gc_set_line_attributes (ctree->lines_gc, 1, 
					GDK_LINE_ON_OFF_DASH, None, None);
	  gdk_gc_set_dashes (ctree->lines_gc, 0, "\1\1", 2);
	  break;
	case EEL_CTREE_LINES_TABBED:
	  if (GTK_WIDGET_REALIZED (ctree))
	    gdk_gc_set_line_attributes (ctree->lines_gc, 1, GDK_LINE_SOLID, 
					None, None);
	  break;
	case EEL_CTREE_LINES_NONE:
	  break;
	default:
	  return;
	}
      CLIST_REFRESH (clist);
    }
}

/***********************************************************
 *             Tree sorting functions                      *
 ***********************************************************/


static void
tree_sort (EelCTree     *ctree,
	   EelCTreeNode *node,
	   gpointer      data)
{
  EelCTreeNode *list_start;
  EelCTreeNode *cmp;
  EelCTreeNode *work;
  EelCList *clist;

  clist = EEL_CLIST (ctree);

  if (node)
    list_start = EEL_CTREE_ROW (node)->children;
  else
    list_start = EEL_CTREE_NODE (clist->row_list);

  while (list_start)
    {
      cmp = list_start;
      work = EEL_CTREE_ROW (cmp)->sibling;
      while (work)
	{
	  if (clist->sort_type == GTK_SORT_ASCENDING)
	    {
	      if (clist->compare 
		  (clist, EEL_CTREE_ROW (work), EEL_CTREE_ROW (cmp)) < 0)
		cmp = work;
	    }
	  else
	    {
	      if (clist->compare 
		  (clist, EEL_CTREE_ROW (work), EEL_CTREE_ROW (cmp)) > 0)
		cmp = work;
	    }
	  work = EEL_CTREE_ROW (work)->sibling;
	}
      if (cmp == list_start)
	list_start = EEL_CTREE_ROW (cmp)->sibling;
      else
	{
	  eel_ctree_unlink (ctree, cmp, FALSE);
	  eel_ctree_link (ctree, cmp, node, list_start, FALSE);
	}
    }
}

void
eel_ctree_sort_recursive (EelCTree     *ctree, 
			  EelCTreeNode *node)
{
  EelCList *clist;
  EelCTreeNode *focus_node = NULL;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  eel_clist_freeze (clist);

  if (clist->selection_mode == GTK_SELECTION_EXTENDED)
    {
      EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
      
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
    }

  if (!node || (node && eel_ctree_is_viewable (ctree, node)))
    focus_node =
      EEL_CTREE_NODE (g_list_nth (clist->row_list, clist->focus_row));
      
  eel_ctree_post_recursive (ctree, node, EEL_CTREE_FUNC (tree_sort), NULL);

  if (!node)
    tree_sort (ctree, NULL, NULL);

  if (focus_node)
    {
      clist->focus_row = g_list_position (clist->row_list,(GList *)focus_node);
      clist->undo_anchor = clist->focus_row;
    }

  CLIST_REFRESH (clist);
  eel_clist_thaw (clist);
}

static void
real_sort_list (EelCList *clist)
{
  eel_ctree_sort_recursive (EEL_CTREE (clist), NULL);
}

void
eel_ctree_sort_node (EelCTree     *ctree, 
		     EelCTreeNode *node)
{
  EelCList *clist;
  EelCTreeNode *focus_node = NULL;

  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  clist = EEL_CLIST (ctree);

  eel_clist_freeze (clist);

  if (clist->selection_mode == GTK_SELECTION_EXTENDED)
    {
      EEL_CLIST_CLASS_FW (clist)->resync_selection (clist, NULL);
      
      g_list_free (clist->undo_selection);
      g_list_free (clist->undo_unselection);
      clist->undo_selection = NULL;
      clist->undo_unselection = NULL;
    }

  if (!node || (node && eel_ctree_is_viewable (ctree, node)))
    focus_node = EEL_CTREE_NODE
      (g_list_nth (clist->row_list, clist->focus_row));

  tree_sort (ctree, node, NULL);

  if (focus_node)
    {
      clist->focus_row = g_list_position (clist->row_list,(GList *)focus_node);
      clist->undo_anchor = clist->focus_row;
    }

  CLIST_REFRESH (clist);
  eel_clist_thaw (clist);
}

void
eel_ctree_sort_single_node (EelCTree *ctree,
				 EelCTreeNode *node)
{
  EelCTreeNode *sibling, *parent;
  EelCList *clist;

  clist = EEL_CLIST (ctree);

  eel_clist_freeze (clist);

  if (EEL_CLIST_AUTO_SORT (clist))
    {
      parent = EEL_CTREE_ROW (node)->parent;

      if (parent)
	sibling = EEL_CTREE_ROW (parent)->children;
      else
	sibling = EEL_CTREE_NODE (clist->row_list);

      while (sibling
	     && (sibling == node || clist->compare (clist, EEL_CTREE_ROW (node), EEL_CTREE_ROW (sibling)) > 0))
	sibling = EEL_CTREE_ROW (sibling)->sibling;

      eel_ctree_unlink (ctree, node, TRUE);
      eel_ctree_link (ctree, node, parent, sibling, TRUE);
    }

  CLIST_REFRESH (clist);
  eel_clist_thaw (clist);
}

/************************************************************************/

static void
fake_unselect_all (EelCList *clist,
		   gint      row)
{
  GList *list;
  GList *focus_node = NULL;

  if (row >= 0 && (focus_node = g_list_nth (clist->row_list, row)))
    {
      if (EEL_CTREE_ROW (focus_node)->row.state == GTK_STATE_NORMAL &&
	  EEL_CTREE_ROW (focus_node)->row.selectable)
	{
	  EEL_CTREE_ROW (focus_node)->row.state = GTK_STATE_SELECTED;
	  
	  if (CLIST_UNFROZEN (clist) &&
	      eel_clist_row_is_visible (clist, row) != GTK_VISIBILITY_NONE)
	    EEL_CLIST_CLASS_FW (clist)->draw_row (clist, NULL, row,
						  EEL_CLIST_ROW (focus_node));
	}  
    }

  clist->undo_selection = clist->selection;
  clist->selection = NULL;
  clist->selection_end = NULL;
  
  for (list = clist->undo_selection; list; list = list->next)
    {
      if (list->data == focus_node)
	continue;

      EEL_CTREE_ROW ((GList *)(list->data))->row.state = GTK_STATE_NORMAL;
      tree_draw_node (EEL_CTREE (clist), EEL_CTREE_NODE (list->data));
    }
}

static GList *
selection_find (EelCList *clist,
		gint      row_number,
		GList    *row_list_element)
{
  return g_list_find (clist->selection, row_list_element);
}

static void
resync_selection (EelCList *clist, GdkEvent *event)
{
  EelCTree *ctree;
  GList *list;
  EelCTreeNode *node;
  gint i;
  gint e;
  gint row;
  gboolean unselect;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  if (clist->selection_mode != GTK_SELECTION_EXTENDED)
    return;

  if (clist->anchor < 0 || clist->drag_pos < 0)
    return;

  ctree = EEL_CTREE (clist);
  
  clist->freeze_count++;

  i = MIN (clist->anchor, clist->drag_pos);
  e = MAX (clist->anchor, clist->drag_pos);

  if (clist->undo_selection)
    {
      list = clist->selection;
      clist->selection = clist->undo_selection;
      clist->selection_end = g_list_last (clist->selection);
      clist->undo_selection = list;
      list = clist->selection;

      while (list)
	{
	  node = list->data;
	  list = list->next;
	  
	  unselect = TRUE;

	  if (eel_ctree_is_viewable (ctree, node))
	    {
	      row = g_list_position (clist->row_list, (GList *)node);
	      if (row >= i && row <= e)
		unselect = FALSE;
	    }
	  if (unselect && EEL_CTREE_ROW (node)->row.selectable)
	    {
	      EEL_CTREE_ROW (node)->row.state = GTK_STATE_SELECTED;
	      eel_ctree_unselect (ctree, node);
	      clist->undo_selection = g_list_prepend (clist->undo_selection,
						      node);
	    }
	}
    }    

  if (clist->anchor < clist->drag_pos)
    {
      for (node = EEL_CTREE_NODE (g_list_nth (clist->row_list, i)); i <= e;
	   i++, node = EEL_CTREE_NODE_NEXT (node))
	if (EEL_CTREE_ROW (node)->row.selectable)
	  {
	    if (g_list_find (clist->selection, node))
	      {
		if (EEL_CTREE_ROW (node)->row.state == GTK_STATE_NORMAL)
		  {
		    EEL_CTREE_ROW (node)->row.state = GTK_STATE_SELECTED;
		    eel_ctree_unselect (ctree, node);
		    clist->undo_selection =
		      g_list_prepend (clist->undo_selection, node);
		  }
	      }
	    else if (EEL_CTREE_ROW (node)->row.state == GTK_STATE_SELECTED)
	      {
		EEL_CTREE_ROW (node)->row.state = GTK_STATE_NORMAL;
		clist->undo_unselection =
		  g_list_prepend (clist->undo_unselection, node);
	      }
	  }
    }
  else
    {
      for (node = EEL_CTREE_NODE (g_list_nth (clist->row_list, e)); i <= e;
	   e--, node = EEL_CTREE_NODE_PREV (node))
	if (EEL_CTREE_ROW (node)->row.selectable)
	  {
	    if (g_list_find (clist->selection, node))
	      {
		if (EEL_CTREE_ROW (node)->row.state == GTK_STATE_NORMAL)
		  {
		    EEL_CTREE_ROW (node)->row.state = GTK_STATE_SELECTED;
		    eel_ctree_unselect (ctree, node);
		    clist->undo_selection =
		      g_list_prepend (clist->undo_selection, node);
		  }
	      }
	    else if (EEL_CTREE_ROW (node)->row.state == GTK_STATE_SELECTED)
	      {
		EEL_CTREE_ROW (node)->row.state = GTK_STATE_NORMAL;
		clist->undo_unselection =
		  g_list_prepend (clist->undo_unselection, node);
	      }
	  }
    }

  clist->undo_unselection = g_list_reverse (clist->undo_unselection);
  for (list = clist->undo_unselection; list; list = list->next)
    eel_ctree_select (ctree, list->data);

  clist->anchor = -1;
  clist->drag_pos = -1;

  if (!CLIST_UNFROZEN (clist))
    clist->freeze_count--;
}

static void
real_undo_selection (EelCList *clist)
{
  EelCTree *ctree;
  GList *work;

  g_return_if_fail (clist != NULL);
  g_return_if_fail (EEL_IS_CTREE (clist));

  if (clist->selection_mode != GTK_SELECTION_EXTENDED)
    return;

  if (!(clist->undo_selection || clist->undo_unselection))
    {
      eel_clist_unselect_all (clist);
      return;
    }

  ctree = EEL_CTREE (clist);

  for (work = clist->undo_selection; work; work = work->next)
    if (EEL_CTREE_ROW (work->data)->row.selectable)
      eel_ctree_select (ctree, EEL_CTREE_NODE (work->data));

  for (work = clist->undo_unselection; work; work = work->next)
    if (EEL_CTREE_ROW (work->data)->row.selectable)
      eel_ctree_unselect (ctree, EEL_CTREE_NODE (work->data));

  if (GTK_WIDGET_HAS_FOCUS (clist) && clist->focus_row != clist->undo_anchor)
    {
      gtk_widget_draw_focus (GTK_WIDGET (clist));
      clist->focus_row = clist->undo_anchor;
      gtk_widget_draw_focus (GTK_WIDGET (clist));
    }
  else
    clist->focus_row = clist->undo_anchor;
  
  clist->undo_anchor = -1;
 
  g_list_free (clist->undo_selection);
  g_list_free (clist->undo_unselection);
  clist->undo_selection = NULL;
  clist->undo_unselection = NULL;

  if (ROW_TOP_YPIXEL (clist, clist->focus_row) + clist->row_height >
      clist->clist_window_height)
    eel_clist_moveto (clist, clist->focus_row, -1, 1, 0);
  else if (ROW_TOP_YPIXEL (clist, clist->focus_row) < 0)
    eel_clist_moveto (clist, clist->focus_row, -1, 0, 0);

}

void
eel_ctree_set_drag_compare_func (EelCTree                *ctree,
				 EelCTreeCompareDragFunc  cmp_func)
{
  g_return_if_fail (ctree != NULL);
  g_return_if_fail (EEL_IS_CTREE (ctree));

  ctree->drag_compare = cmp_func;
}

static gboolean
check_drag (EelCTree        *ctree,
	    EelCTreeNode    *drag_source,
	    EelCTreeNode    *drag_target,
	    EelCListDragPos  insert_pos)
{
  g_return_val_if_fail (ctree != NULL, FALSE);
  g_return_val_if_fail (EEL_IS_CTREE (ctree), FALSE);

  if (drag_source && drag_source != drag_target &&
      (!EEL_CTREE_ROW (drag_source)->children ||
       !eel_ctree_is_ancestor (ctree, drag_source, drag_target)))
    {
      switch (insert_pos)
	{
	case EEL_CLIST_DRAG_NONE:
	  return FALSE;
	case EEL_CLIST_DRAG_AFTER:
	  if (EEL_CTREE_ROW (drag_target)->sibling != drag_source)
	    return (!ctree->drag_compare ||
		    ctree->drag_compare (ctree,
					 drag_source,
					 EEL_CTREE_ROW (drag_target)->parent,
					 EEL_CTREE_ROW(drag_target)->sibling));
	  break;
	case EEL_CLIST_DRAG_BEFORE:
	  if (EEL_CTREE_ROW (drag_source)->sibling != drag_target)
	    return (!ctree->drag_compare ||
		    ctree->drag_compare (ctree,
					 drag_source,
					 EEL_CTREE_ROW (drag_target)->parent,
					 drag_target));
	  break;
	case EEL_CLIST_DRAG_INTO:
	  if (!EEL_CTREE_ROW (drag_target)->is_leaf &&
	      EEL_CTREE_ROW (drag_target)->children != drag_source)
	    return (!ctree->drag_compare ||
		    ctree->drag_compare (ctree,
					 drag_source,
					 drag_target,
					 EEL_CTREE_ROW (drag_target)->children));
	  break;
	}
    }
  return FALSE;
}



/************************************/
static void
drag_dest_info_destroy (gpointer data)
{
  EelCListDestInfo *info = data;

  g_free (info);
}

static void
drag_dest_cell (EelCList         *clist,
		gint              x,
		gint              y,
		EelCListDestInfo *dest_info)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (clist);

  dest_info->insert_pos = EEL_CLIST_DRAG_NONE;

  y -= (GTK_CONTAINER (widget)->border_width +
	widget->style->klass->ythickness + clist->column_title_area.height);
  dest_info->cell.row = ROW_FROM_YPIXEL (clist, y);

  if (dest_info->cell.row >= clist->rows)
    {
      dest_info->cell.row = clist->rows - 1;
      y = ROW_TOP_YPIXEL (clist, dest_info->cell.row) + clist->row_height;
    }
  if (dest_info->cell.row < -1)
    dest_info->cell.row = -1;

  x -= GTK_CONTAINER (widget)->border_width + widget->style->klass->xthickness;
  dest_info->cell.column = COLUMN_FROM_XPIXEL (clist, x);

  if (dest_info->cell.row >= 0)
    {
      gint y_delta;
      gint h = 0;

      y_delta = y - ROW_TOP_YPIXEL (clist, dest_info->cell.row);
      
      if (EEL_CLIST_DRAW_DRAG_RECT(clist) &&
	  !EEL_CTREE_ROW (g_list_nth (clist->row_list,
				      dest_info->cell.row))->is_leaf)
	{
	  dest_info->insert_pos = EEL_CLIST_DRAG_INTO;
	  h = clist->row_height / 4;
	}
      else if (EEL_CLIST_DRAW_DRAG_LINE(clist))
	{
	  dest_info->insert_pos = EEL_CLIST_DRAG_BEFORE;
	  h = clist->row_height / 2;
	}

      if (EEL_CLIST_DRAW_DRAG_LINE(clist))
	{
	  if (y_delta < h)
	    dest_info->insert_pos = EEL_CLIST_DRAG_BEFORE;
	  else if (clist->row_height - y_delta < h)
	    dest_info->insert_pos = EEL_CLIST_DRAG_AFTER;
	}
    }
}

static void
eel_ctree_drag_begin (GtkWidget	     *widget,
		      GdkDragContext *context)
{
  EelCList *clist;
  EelCTree *ctree;
  gboolean use_icons;
  GdkPixbuf *pixbuf;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (EEL_IS_CTREE (widget));
  g_return_if_fail (context != NULL);

  clist = EEL_CLIST (widget);
  ctree = EEL_CTREE (widget);

  use_icons = EEL_CLIST_USE_DRAG_ICONS (clist);
  EEL_CLIST_UNSET_FLAG (clist, CLIST_USE_DRAG_ICONS);
  GTK_WIDGET_CLASS (parent_class)->drag_begin (widget, context);

  if (use_icons)
    {
      EelCTreeNode *node;

      EEL_CLIST_SET_FLAG (clist, CLIST_USE_DRAG_ICONS);
      node = EEL_CTREE_NODE (g_list_nth (clist->row_list,
					 clist->click_cell.row));
      if (node)
	{
	  pixbuf = EEL_CELL_PIXTEXT
		  (EEL_CTREE_ROW (node)->row.cell[ctree->tree_column])->pixbuf;
	  if (pixbuf)
	    {
	      eel_drag_set_icon_pixbuf (context, pixbuf, -2, -2);
	      return;
	    }
	}
      gtk_drag_set_icon_default (context);
    }
}

static gint
eel_ctree_drag_motion (GtkWidget      *widget,
		       GdkDragContext *context,
		       gint            x,
		       gint            y,
		       guint           time)
{
  EelCList *clist;
  EelCTree *ctree;
  EelCListDestInfo new_info;
  EelCListDestInfo *dest_info;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (EEL_IS_CTREE (widget), FALSE);

  clist = EEL_CLIST (widget);
  ctree = EEL_CTREE (widget);

  dest_info = g_dataset_get_data (context, "eel-clist-drag-dest");

  if (!dest_info)
    {
      dest_info = g_new (EelCListDestInfo, 1);
	  
      dest_info->cell.row    = -1;
      dest_info->cell.column = -1;
      dest_info->insert_pos  = EEL_CLIST_DRAG_NONE;

      g_dataset_set_data_full (context, "eel-clist-drag-dest", dest_info,
			       drag_dest_info_destroy);
    }

  drag_dest_cell (clist, x, y, &new_info);

  if (EEL_CLIST_REORDERABLE (clist))
    {
      GList *list;
      GdkAtom atom = gdk_atom_intern ("eel-clist-drag-reorder", FALSE);

      list = context->targets;
      while (list)
	{
	  if (atom == GPOINTER_TO_UINT (list->data))
	    break;
	  list = list->next;
	}

      if (list)
	{
	  EelCTreeNode *drag_source;
	  EelCTreeNode *drag_target;

	  drag_source = EEL_CTREE_NODE (g_list_nth (clist->row_list,
						    clist->click_cell.row));
	  drag_target = EEL_CTREE_NODE (g_list_nth (clist->row_list,
						    new_info.cell.row));

	  if (gtk_drag_get_source_widget (context) != widget ||
	      !check_drag (ctree, drag_source, drag_target,
			   new_info.insert_pos))
	    {
	      if (dest_info->cell.row < 0)
		{
		  gdk_drag_status (context, GDK_ACTION_DEFAULT, time);
		  return FALSE;
		}
	      return TRUE;
	    }

	  if (new_info.cell.row != dest_info->cell.row ||
	      (new_info.cell.row == dest_info->cell.row &&
	       dest_info->insert_pos != new_info.insert_pos))
	    {
	      if (dest_info->cell.row >= 0)
		EEL_CLIST_CLASS_FW (clist)->draw_drag_highlight
		  (clist,
		   g_list_nth (clist->row_list, dest_info->cell.row)->data,
		   dest_info->cell.row, dest_info->insert_pos);

	      dest_info->insert_pos  = new_info.insert_pos;
	      dest_info->cell.row    = new_info.cell.row;
	      dest_info->cell.column = new_info.cell.column;

	      EEL_CLIST_CLASS_FW (clist)->draw_drag_highlight
		(clist,
		 g_list_nth (clist->row_list, dest_info->cell.row)->data,
		 dest_info->cell.row, dest_info->insert_pos);

	      gdk_drag_status (context, context->suggested_action, time);
	    }
	  return TRUE;
	}
    }

  dest_info->insert_pos  = new_info.insert_pos;
  dest_info->cell.row    = new_info.cell.row;
  dest_info->cell.column = new_info.cell.column;
  return TRUE;
}

static void
eel_ctree_drag_data_received (GtkWidget        *widget,
			      GdkDragContext   *context,
			      gint              x,
			      gint              y,
			      GtkSelectionData *selection_data,
			      guint             info,
			      guint32           time)
{
  EelCTree *ctree;
  EelCList *clist;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (EEL_IS_CTREE (widget));
  g_return_if_fail (context != NULL);
  g_return_if_fail (selection_data != NULL);

  ctree = EEL_CTREE (widget);
  clist = EEL_CLIST (widget);

  if (EEL_CLIST_REORDERABLE (clist) &&
      gtk_drag_get_source_widget (context) == widget &&
      selection_data->target ==
      gdk_atom_intern ("eel-clist-drag-reorder", FALSE) &&
      selection_data->format == GTK_TYPE_POINTER &&
      selection_data->length == sizeof (EelCListCellInfo))
    {
      EelCListCellInfo *source_info;

      source_info = (EelCListCellInfo *)(selection_data->data);
      if (source_info)
	{
	  EelCListDestInfo dest_info;
	  EelCTreeNode *source_node;
	  EelCTreeNode *dest_node;

	  drag_dest_cell (clist, x, y, &dest_info);
	  
	  source_node = EEL_CTREE_NODE (g_list_nth (clist->row_list,
						    source_info->row));
	  dest_node = EEL_CTREE_NODE (g_list_nth (clist->row_list,
						  dest_info.cell.row));

	  if (!source_node || !dest_node)
	    return;

	  switch (dest_info.insert_pos)
	    {
	    case EEL_CLIST_DRAG_NONE:
	      break;
	    case EEL_CLIST_DRAG_INTO:
	      if (check_drag (ctree, source_node, dest_node,
			      dest_info.insert_pos))
		eel_ctree_move (ctree, source_node, dest_node,
				EEL_CTREE_ROW (dest_node)->children);
	      g_dataset_remove_data (context, "eel-clist-drag-dest");
	      break;
	    case EEL_CLIST_DRAG_BEFORE:
	      if (check_drag (ctree, source_node, dest_node,
			      dest_info.insert_pos))
		eel_ctree_move (ctree, source_node,
				EEL_CTREE_ROW (dest_node)->parent, dest_node);
	      g_dataset_remove_data (context, "eel-clist-drag-dest");
	      break;
	    case EEL_CLIST_DRAG_AFTER:
	      if (check_drag (ctree, source_node, dest_node,
			      dest_info.insert_pos))
		eel_ctree_move (ctree, source_node,
				EEL_CTREE_ROW (dest_node)->parent, 
				EEL_CTREE_ROW (dest_node)->sibling);
	      g_dataset_remove_data (context, "eel-clist-drag-dest");
	      break;
	    }
	}
    }
}
