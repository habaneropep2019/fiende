/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball, Josh MacDonald
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

#ifndef EEL_CTREE_H
#define EEL_CTREE_H

#include <eel/eel-list.h>

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#define EEL_TYPE_CTREE            		(eel_ctree_get_type ())
#define EEL_CTREE(obj)            		(GTK_CHECK_CAST ((obj), EEL_TYPE_CTREE, EelCTree))
#define EEL_CTREE_CLASS(klass)    		(GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_CTREE, EelCTreeClass))
#define EEL_IS_CTREE(obj)         		(GTK_CHECK_TYPE ((obj), EEL_TYPE_CTREE))
#define EEL_IS_CTREE_CLASS(klass) 		(GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_CTREE))

#define EEL_CTREE_ROW(_node_) 		((EelCTreeRow *)(((GList *)(_node_))->data))
#define EEL_CTREE_NODE(_node_) 		((EelCTreeNode *)((_node_)))
#define EEL_CTREE_NODE_NEXT(_nnode_) 	((EelCTreeNode *)(((GList *)(_nnode_))->next))
#define EEL_CTREE_NODE_PREV(_pnode_) 	((EelCTreeNode *)(((GList *)(_pnode_))->prev))
#define EEL_CTREE_FUNC(_func_) 		((EelCTreeFunc)(_func_))

typedef enum
{
	EEL_CTREE_POS_BEFORE,
	EEL_CTREE_POS_AS_CHILD,
	EEL_CTREE_POS_AFTER
} EelCTreePos;

typedef enum
{
	EEL_CTREE_LINES_NONE,
	EEL_CTREE_LINES_SOLID,
	EEL_CTREE_LINES_DOTTED,
	EEL_CTREE_LINES_TABBED
} EelCTreeLineStyle;

typedef enum
{
	EEL_CTREE_EXPANDER_TRIANGLE
} EelCTreeExpanderStyle;

typedef enum
{
	EEL_CTREE_EXPANSION_EXPAND,
	EEL_CTREE_EXPANSION_EXPAND_RECURSIVE,
	EEL_CTREE_EXPANSION_COLLAPSE,
	EEL_CTREE_EXPANSION_COLLAPSE_RECURSIVE,
	EEL_CTREE_EXPANSION_TOGGLE,
	EEL_CTREE_EXPANSION_TOGGLE_RECURSIVE
} EelCTreeExpansionType;

typedef struct _EelCTree      EelCTree;
typedef struct _EelCTreeClass EelCTreeClass;
typedef struct _EelCTreeRow   EelCTreeRow;
typedef struct _EelCTreeNode  EelCTreeNode;

typedef void (*EelCTreeFunc) 	(EelCTree     *ctree,
			      		 EelCTreeNode *node,
			      		 gpointer      data);

typedef gboolean (*EelCTreeGNodeFunc) (EelCTree     *ctree,
					    guint         depth,
					    GNode        *gnode,
					    EelCTreeNode *cnode,
					    gpointer      data);

typedef gboolean (*EelCTreeCompareDragFunc) (EelCTree     *ctree,
						  EelCTreeNode *source_node,
						  EelCTreeNode *new_parent,
						  EelCTreeNode *new_sibling);

struct _EelCTree
{
	EelList list;
	
	GdkGC *lines_gc;
	
	gint tree_indent;
	gint tree_spacing;
	gint tree_column;
	
	guint line_style     : 2;
	guint show_stub      : 1;
	
	EelCTreeNode *prelight_node;

	EelCTreeRow *dnd_prelighted_row;

	EelCTreeCompareDragFunc drag_compare;
};

struct _EelCTreeClass
{
  EelListClass parent_class;
  
  void (*tree_select_row)   (EelCTree     *ctree,
			     EelCTreeNode *row,
			     gint          column);
  void (*tree_unselect_row) (EelCTree     *ctree,
			     EelCTreeNode *row,
			     gint          column);
  void (*tree_expand)       (EelCTree     *ctree,
			     EelCTreeNode *node);
  void (*tree_collapse)     (EelCTree     *ctree,
			     EelCTreeNode *node);
  void (*tree_move)         (EelCTree     *ctree,
			     EelCTreeNode *node,
			     EelCTreeNode *new_parent,
			     EelCTreeNode *new_sibling);
  void (*change_focus_row_expansion) (EelCTree *ctree,
				      EelCTreeExpansionType action);
  void (*tree_activate_row) (EelCTree     *ctree,
			     EelCTreeNode *row,
			     gint		column);
};

struct _EelCTreeRow
{
	EelCListRow row;
  
	EelCTreeNode *parent;
	EelCTreeNode *sibling;
	EelCTreeNode *children;
  
	GdkPixbuf *pixbuf_closed;
	GdkPixbuf *pixbuf_opened;
  
	guint16 level;
  
	guint is_leaf  : 1;
	guint expanded : 1;

	gboolean mouse_down;
	gboolean in_hotspot;

};

struct _EelCTreeNode {
  GList list;
};


/***********************************************************
 *           Creation, insertion, deletion                 *
 ***********************************************************/

GtkType       eel_ctree_get_type                    (void);
void          eel_ctree_construct                   (EelCTree                 *ctree,
						     gint                      columns,
						     gint                      tree_column,
						     gchar                    *titles[]);
GtkWidget *   eel_ctree_new_with_titles             (gint                      columns,
						     gint                      tree_column,
						     gchar                    *titles[]);
GtkWidget *   eel_ctree_new                         (gint                      columns,
						     gint                      tree_column);
EelCTreeNode *eel_ctree_insert_node                 (EelCTree                 *ctree,
						     EelCTreeNode             *parent,
						     EelCTreeNode             *sibling,
						     gchar                    *text[],
						     guint8                    spacing,
						     GdkPixbuf                *pixbuf_closed,
						     GdkPixbuf                *pixbuf_opened,
						     gboolean                  is_leaf,
						     gboolean                  expanded);
void          eel_ctree_remove_node                 (EelCTree                 *ctree,
						     EelCTreeNode             *node);
EelCTreeNode *eel_ctree_insert_gnode                (EelCTree                 *ctree,
						     EelCTreeNode             *parent,
						     EelCTreeNode             *sibling,
						     GNode                    *gnode,
						     EelCTreeGNodeFunc         func,
						     gpointer                  data);
GNode *       eel_ctree_export_to_gnode             (EelCTree                 *ctree,
						     GNode                    *parent,
						     GNode                    *sibling,
						     EelCTreeNode             *node,
						     EelCTreeGNodeFunc         func,
						     gpointer                  data);

/***********************************************************
 *  Generic recursive functions, querying / finding tree   *
 *  information                                            *
 ***********************************************************/
void          eel_ctree_post_recursive              (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     EelCTreeFunc              func,
						     gpointer                  data);
void          eel_ctree_post_recursive_to_depth     (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      depth,
						     EelCTreeFunc              func,
						     gpointer                  data);
void          eel_ctree_pre_recursive               (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     EelCTreeFunc              func,
						     gpointer                  data);
void          eel_ctree_pre_recursive_to_depth      (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      depth,
						     EelCTreeFunc              func,
						     gpointer                  data);
gboolean      eel_ctree_is_viewable                 (EelCTree                 *ctree,
						     EelCTreeNode             *node);
EelCTreeNode *eel_ctree_last                        (EelCTree                 *ctree,
						     EelCTreeNode             *node);
EelCTreeNode *eel_ctree_find_node_ptr               (EelCTree                 *ctree,
						     EelCTreeRow              *ctree_row);
EelCTreeNode *eel_ctree_node_nth                    (EelCTree                 *ctree,
						     int                       row);
gboolean      eel_ctree_find                        (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     EelCTreeNode             *child);
gboolean      eel_ctree_is_ancestor                 (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     EelCTreeNode             *child);
EelCTreeNode *eel_ctree_find_by_row_data            (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gpointer                  data);
/* returns a GList of all EelCTreeNodes with row->data == data. */
GList *       eel_ctree_find_all_by_row_data        (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gpointer                  data);
EelCTreeNode *eel_ctree_find_by_row_data_custom     (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gpointer                  data,
						     GCompareFunc              func);
/* returns a GList of all EelCTreeNodes with row->data == data. */
GList *       eel_ctree_find_all_by_row_data_custom (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gpointer                  data,
						     GCompareFunc              func);
gboolean      eel_ctree_is_hot_spot                 (EelCTree                 *ctree,
						     gint                      x,
						     gint                      y);

/***********************************************************
 *   Tree signals : move, expand, collapse, (un)select     *
 ***********************************************************/
void          eel_ctree_move                        (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     EelCTreeNode             *new_parent,
						     EelCTreeNode             *new_sibling);
void          eel_ctree_expand                      (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_expand_recursive            (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_expand_to_depth             (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      depth);
void          eel_ctree_collapse                    (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_collapse_recursive          (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_collapse_to_depth           (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      depth);
void          eel_ctree_toggle_expansion            (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_toggle_expansion_recursive  (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_select                      (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_select_recursive            (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_unselect                    (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_unselect_recursive          (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_real_select_recursive       (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      state);
void          eel_ctree_draw_node                   (EelCTree                 *ctree,
						     EelCTreeNode             *node);

/***********************************************************
 *           Analogons of GtkCList functions               *
 ***********************************************************/
void          eel_ctree_node_set_text               (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     const gchar              *text);
void          eel_ctree_node_set_pixbuf             (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     GdkPixbuf                *pixbuf);
void          eel_ctree_node_set_pixtext            (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     const gchar              *text,
						     guint8                    spacing,
						     GdkPixbuf                *pixbuf);
void          eel_ctree_set_node_info               (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     const gchar              *text,
						     guint8                    spacing,
						     GdkPixbuf                *pixbuf_closed,
						     GdkPixbuf                *pixbuf_opened,
						     gboolean                  is_leaf,
						     gboolean                  expanded);
void          eel_ctree_node_set_shift              (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     gint                      vertical,
						     gint                      horizontal);
void          eel_ctree_node_set_selectable         (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gboolean                  selectable);
gboolean      eel_ctree_node_get_selectable         (EelCTree                 *ctree,
						     EelCTreeNode             *node);
EelCellType   eel_ctree_node_get_cell_type          (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column);
gint          eel_ctree_node_get_text               (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     gchar                   **text);
gint          eel_ctree_node_get_pixbuf             (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     GdkPixbuf               **pixbuf);
gint          eel_ctree_node_get_pixtext            (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     gchar                   **text,
						     guint8                   *spacing,
						     GdkPixbuf               **pixbuf);
gint          eel_ctree_get_node_info               (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gchar                   **text,
						     guint8                   *spacing,
						     GdkPixbuf               **pixbuf_closed,
						     GdkPixbuf               **pixbuf_opened,
						     gboolean                 *is_leaf,
						     gboolean                 *expanded);
void          eel_ctree_set_prelight                (EelCTree                 *ctree,
						     int                       y);
void          eel_ctree_node_set_row_style          (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     GtkStyle                 *style);
GtkStyle *    eel_ctree_node_get_row_style          (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_node_set_cell_style         (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     GtkStyle                 *style);
GtkStyle *    eel_ctree_node_get_cell_style         (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column);
void          eel_ctree_node_set_foreground         (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     GdkColor                 *color);
void          eel_ctree_node_set_background         (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     GdkColor                 *color);
void          eel_ctree_node_set_row_data           (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gpointer                  data);
void          eel_ctree_node_set_row_data_full      (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gpointer                  data,
						     GtkDestroyNotify          destroy);
gpointer      eel_ctree_node_get_row_data           (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_node_moveto                 (EelCTree                 *ctree,
						     EelCTreeNode             *node,
						     gint                      column,
						     gfloat                    row_align,
						     gfloat                    col_align);
GtkVisibility eel_ctree_node_is_visible             (EelCTree                 *ctree,
						     EelCTreeNode             *node);

/***********************************************************
 *             EelCTree specific functions            *
 ***********************************************************/
void          eel_ctree_set_indent                  (EelCTree                 *ctree,
						     gint                      indent);
void          eel_ctree_set_spacing                 (EelCTree                 *ctree,
						     gint                      spacing);
void          eel_ctree_set_show_stub               (EelCTree                 *ctree,
						     gboolean                  show_stub);
void          eel_ctree_set_line_style              (EelCTree                 *ctree,
						     EelCTreeLineStyle         line_style);
void          eel_ctree_set_drag_compare_func       (EelCTree                 *ctree,
						     EelCTreeCompareDragFunc   cmp_func);

/***********************************************************
 *             Tree sorting functions                      *
 ***********************************************************/
void          eel_ctree_sort_node                   (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_sort_single_node            (EelCTree                 *ctree,
						     EelCTreeNode             *node);
void          eel_ctree_sort_recursive              (EelCTree                 *ctree,
						     EelCTreeNode             *node);
#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* EEL_CTREE_H */
