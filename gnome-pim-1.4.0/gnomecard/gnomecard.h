/* GnomeCard - a graphical contact manager.
 *
 * gnomecard.h: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
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
 */

#ifndef __GNOMECARD
#define __GNOMECARD

#include <gnome.h>

#define PHONE 1
#define EMAIL 2

#define TREE_SPACING 16

/*extern GtkCTree  *gnomecard_tree; */
extern GtkCList *gnomecard_list;
extern GtkWidget *cardlist_scrollwin;
extern gint gnomecard_selected_row;

extern GList *gnomecard_crds;
extern GList *gnomecard_curr_crd;

extern GtkCTreeNode *gnomecard_selected_node;

extern char *gnomecard_fname;
extern char *gnomecard_find_str;
extern gboolean gnomecard_find_sens;
extern gboolean gnomecard_find_back;
extern gint gnomecard_def_data;

gchar *gnomecard_join_name (char *pre, char *given, char *add, 
				  char *fam, char *suf);
void gnomecard_set_add(gboolean state);
void gnomecard_set_app_title (char *title);
void gnomecard_set_changed(gboolean val);
void gnomecard_set_edit_del(gboolean state);
void gnomecard_set_curr(GList *node);
gint  gnomecard_destroy_cards(void);

void gnomecard_first_card( GtkWidget *widget, gpointer data);
void gnomecard_prev_card( GtkWidget *widget, gpointer data);
void gnomecard_next_card( GtkWidget *widget, gpointer data);
void gnomecard_last_card( GtkWidget *widget, gpointer data);
void gnomecard_quit( GtkWidget *widget, gpointer data);
gint gnomecard_delete( GtkWidget *widget, GdkEvent *e, gpointer data);
void gnomecard_spawn_new( GtkWidget *widget, gpointer data);
void gnomecard_sort_by_fname( GtkWidget *widget, gpointer data);
void gnomecard_init(void);
#endif
