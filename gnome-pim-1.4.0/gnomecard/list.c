/* GnomeCard - a graphical contact manager.
 *
 * list.c: This file is part of GnomeCard.
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

/* Handle list view of avialable cards */
/* Michael Fulbright  <msf@redhat.com> */
/* Copyright (c) 1998 Red Hat Software, Inc */

#include <config.h>
#include <string.h>
#include <gnome.h>

#include "card.h"
#include "gnomecard.h"
#include "popup-menu.h"
#include "dialog.h"
#include "del.h"
#include "my.h"
#include "pix.h"
#include "sort.h"
#include "list.h"
#include "type_name.h"
#include "columnhdrs.h"
#include "dnd.h"

#define LIST_WIDTH 450
#define LIST_HEIGHT 400

static void gnomecard_list_selected(GtkCList *list, gint row, gint column,
				    GdkEventButton *event, gpointer data);
static void gnomecard_list_button_press(GtkCList *list, GdkEventButton *event, 
					gpointer data);

GtkTargetEntry drag_types [] = {
	{ "text/x-vcard",          0, TARGET_VCARD },
	{ "x-application/x-email", 0, TARGET_EMAIL },
	{ "text/plain",            0, TARGET_TEXT },
};

static void
gnomecard_list_button_press(GtkCList *list, GdkEventButton *event, 
			    gpointer data)
{
    static struct menu_item items[] = {
	{ N_("Edit this item..."), (GtkSignalFunc) gnomecard_edit_card, NULL,
	  TRUE },
	{ N_("Delete this item"), (GtkSignalFunc)gnomecard_delete_current_card,
	  NULL, TRUE } 
    };
    GList *tmp;
    gint  row, col;
    
    if (!event)
	return;
    
    if (event->button == 3) 
	if (gtk_clist_get_selection_info(list,event->x,event->y,&row,&col )) {
	    gnomecard_selected_row = row;
	    gtk_clist_select_row(list, row, 0);
	    tmp = g_list_nth(gnomecard_crds, row);
	    
	    if (!tmp) {
		g_message("Somehow selected non-existant card on row %d",row);
		return;
	    }

	    /* see if card if is being editted */
	    items[0].sensitive = !((Card *) tmp->data)->flag;
	    items[1].sensitive = !((Card *) tmp->data)->flag;

	    gnomecard_set_curr(tmp);
	    popup_menu (items, sizeof (items) / sizeof (items[0]), event);
	}
}

static void
gnomecard_list_selected(GtkCList *list, gint row, gint column,
			GdkEventButton *event, gpointer data)
{
    GList *tmp;

    gnomecard_selected_row = row;
    tmp = g_list_nth(gnomecard_crds, row);
    
    if (!tmp) {
	g_message("Somehow selected non-existant card on row %d",row);
	return;
    }

    if (!event) {
	gnomecard_set_curr(tmp);
    } else {
	switch (event->button) {
	  case 1:
	    if (event->type == GDK_2BUTTON_PRESS) {
		/* see if card if is being editted */
		if (!((Card *) gnomecard_curr_crd->data)->flag)
		    gnomecard_edit(tmp);
	    } else {
		gnomecard_set_curr(tmp);
	    }
	    break;
	    
	  default:
	    break;
	}
    }
}

static void
gnomecard_sort_by_col(GtkWidget *w, gint col, GtkWidget *list)
{
    GList *cols, *l;
    ColumnHeader *hdr;
/*    sort_func    func=NULL;*/

    cols = gtk_object_get_data(GTK_OBJECT(list), "ColumnHeaders");
    l = g_list_nth(cols, col);
    hdr = (ColumnHeader *)l->data;
    gnomecard_sort_card_list(hdr->coltype);

    /* this is a hack - need function to just rebuid list    */
    /* and maintain current selection in list w/o me knowing */
    /* about it here                                         */
    gnomecard_rebuild_list();
}


GList *
gnomecardCreateColTitles(GList *cols)
{
	gchar  *str;
	GList  *l, *p;
	
	l = NULL;
	for (p=cols; p; p=p->next) {
	    ColumnHeader *s;

	    s = (ColumnHeader *)p->data;
	    str = getColumnNameFromType(s->coltype);
	    if (!str)
		str = g_strdup("");
	    l = g_list_append(l, str);
	}

	return l;
}

void
gnomecardFreeColTitles(GList *col)
{
    g_list_free(col);
}


GList *
gnomecardCreateColValues(Card *crd, GList *cols)
{
	gchar *str;
	GList *l, *p;
	GList *phone_list, *email_list;
	ColumnHeader *hdr;
	
	l = NULL;
	email_list = crd->email.l;
	phone_list = crd->phone.l;
	
	for (p = cols; p; p = p->next) {
		str = NULL;
		hdr = (ColumnHeader *) p->data;
		
		switch (hdr->coltype) {
		 case COLTYPE_EMAIL:
			if (email_list) {
				str = g_strdup
				  (((CardEMail *) email_list->data)->data);
				email_list = email_list->next;
			}
			break;
			
		 case COLTYPE_PHONE:
			if (phone_list) {
				str = g_strdup
				  (((CardPhone *) phone_list->data)->data);
				phone_list = phone_list->next;
			}
			break;
			
		 default:
			str = getValFromColumnHdr(crd, hdr);
		}
		
		if (!str) {
			if (hdr->coltype == COLTYPE_CARDNAME)
			  str = g_strdup (_("No Card Name"));
			else
			  str = g_strdup("");
		}
		l = g_list_append(l, str);
	}

	return l;
}

void
gnomecardFreeColValues(GList *col)
{
    g_list_foreach(col, (GFunc)g_free, NULL);
    g_list_free(col);
}

void
gnomecard_update_list(Card *crd)
{
    gint   row;
    gint   col;
    GList  *rowtxt, *l, *cols;

    cols = gtk_object_get_data(GTK_OBJECT(gnomecard_list), "ColumnHeaders");
    if (!cols) {
	g_message("gnomecard_add_card_to_list: col hdrs not defined");
	return;
    }
    row = gtk_clist_find_row_from_data( gnomecard_list, crd);
    
    if (row < 0) {
	g_message("gnomecard_update_list: row < 0");
	return;
    }

    rowtxt = gnomecardCreateColValues(crd, cols);
    for (col=0, l=rowtxt; !l; l=l->next)
	gtk_clist_set_text(gnomecard_list, row, col, l->data);

    gtk_clist_set_row_data(gnomecard_list, row, crd);
    gnomecardFreeColValues(rowtxt);
}


/* redraw list based on gnomecard_crd structure               */
/* used after a sort, for example, has changed order of cards */
void
gnomecard_rebuild_list(void)
{
    GList *c;
    Card  *curcard=NULL;
    gint  currow;

    if (gnomecard_list->selection) {
	currow = GPOINTER_TO_INT(gnomecard_list->selection->data);
	if (currow >= 0)
	    curcard = (Card *)gtk_clist_get_row_data(gnomecard_list, currow);
    }

    gtk_clist_freeze(gnomecard_list);
    gtk_clist_clear(gnomecard_list);
    for (c = gnomecard_crds; c; c = c->next)
	gnomecard_add_card_to_list((Card *) c->data);
    gtk_clist_thaw(gnomecard_list);
    if (curcard) {
	GList *l;
	l = g_list_find(gnomecard_crds, curcard);
	if (l)
	    gnomecard_scroll_list(l);
	else 
	    gnomecard_scroll_list(gnomecard_crds);
    } else {
	gnomecard_scroll_list(gnomecard_crds);
    }
}

void
gnomecard_scroll_list(GList *node)
{
    gint row;
    GList *tmp;
	
    if (! node)
	  return;
	
    row = gtk_clist_find_row_from_data(gnomecard_list,(Card *) node->data);

    if (gtk_clist_row_is_visible(gnomecard_list, row) != GTK_VISIBILITY_FULL)
	gtk_clist_moveto(gnomecard_list, row, 0, 0.5, 0.0);

    gtk_clist_select_row(gnomecard_list, row, 0);
    tmp = g_list_nth(gnomecard_crds, row);

    if (!tmp) {
	g_message("Somehow selected non-existant card");
	return;
    }

    gnomecard_set_curr(tmp);
}

/* NOT USED 
static gchar *
gnomecard_first_phone_str(GList *phone)
{
	gchar *ret;
	
	if (! phone)
	  return NULL;
	
	ret = ((CardPhone *) phone->data)->data;
	
	for ( ; phone; phone = phone->next)
	  if (((CardPhone *) phone->data)->type & PHONE_PREF)	
	    ret = ((CardPhone *) phone->data)->data;
	
	return ret;
}
*/

/* NOT USED
void
gnomecard_list_set_node_info(Card *crd)
{
    g_message("in gnomecard_list_set_node_info - not implemented");
}
*/

/* NOT USED
static gint
gnomecard_next_addr_type(gint type, gint start)
{
    gint j;
    
    for (j = start; j < 6; j++)
	if (type & (1 << j))
	    return j;
    
    return j;
}*/

/* NOT USED
void
gnomecard_add_card_sections_to_list(Card *crd)
{
    g_message("gnomecard_add_card_sections_to_list not implemented");
}
*/
void
gnomecard_list_remove_card(Card *crd)
{
    gint row;

    row = gtk_clist_find_row_from_data( gnomecard_list, crd);
    if (row < 0)
	g_message("list_remove_card: couldnt find data %p",crd);
    gtk_clist_freeze(gnomecard_list);
    gtk_clist_remove(gnomecard_list, row);
    gtk_clist_thaw(gnomecard_list);
}

void
gnomecard_add_card_to_list(Card *crd)
{
	gint   row;
	gint   col;
	gint   ncols;
        gchar  **tmp;
	GList  *rowtxt, *l, *cols;

	cols = gtk_object_get_data(GTK_OBJECT(gnomecard_list), "ColumnHeaders");
	if (!cols) {
	    g_message("gnomecard_add_card_to_list: col hdrs not defined");
	    return;
	}

	ncols = g_list_length(cols);
	tmp = (gchar **) g_new0(gchar *, ncols+1);
	for (col=0; col<ncols; col++)
	    tmp[col] = "";
	tmp[ncols] = NULL;
	row = gtk_clist_append(gnomecard_list, tmp);

	rowtxt = gnomecardCreateColValues(crd, cols);
	for (col=0, l=rowtxt; l; l=l->next, col++)
	    gtk_clist_set_text(gnomecard_list, row, col, l->data);
	    
	gtk_clist_set_row_data(gnomecard_list, row, crd);
	gnomecardFreeColValues(rowtxt);

}

void
gnomecard_list_set_sorted_pos(Card *crd)
{
    g_message("gnomecard_tree_set_sorted_pos not implemented");
}


/* clear card list */
void
gnomecardClearCardListDisplay(GtkWidget *list)
{
    gtk_clist_clear(GTK_CLIST(list));
}

/*
 * Called back when the drag has been accepted.  Render
 * the information
 */
static void
drag_data_get (GtkWidget        *widget,
	       GdkDragContext   *context,
	       GtkSelectionData *selection_data,
	       guint            info,
	       guint32          time)
{
	Card *card = gnomecard_curr_crd->data;
	char *data = NULL;
	
	switch (info){
	case TARGET_TEXT: 
		data = card_to_string (card);
		break;
		
	case TARGET_EMAIL: {
	        char *txt = "";
		
		if (card->email.l)
		  txt = ((CardEMail *) card->email.l->data)->data;
		
		data = g_strdup_printf ("%s <%s>", card->fname.str, txt);
		}
		
		break;
		
	case TARGET_VCARD:
		data = card_to_vobj_string (card);
		break;
	}

	if (!data)
		return;
	
	gtk_selection_data_set (selection_data, selection_data->target, 8,
				data, strlen (data)+1);

	g_free (data);
}

void
configure_source_card_dnd (GtkWidget *widget)
{
	gtk_drag_source_set (widget, GDK_BUTTON1_MASK,
			     drag_types, 3,
			     GDK_ACTION_COPY | GDK_ACTION_DEFAULT);
	
	gtk_signal_connect (GTK_OBJECT (widget), "drag_data_get",
			    GTK_SIGNAL_FUNC (drag_data_get), NULL);
}

/* create a clist based on the column headings */
GtkWidget *
gnomecardCreateCardListDisplay(ColumnType *hdrs)
{
    GtkCList *cardlist;
    GList *cols, *titles, *l;
    gint  i;

    cols = buildColumnHeaders(hdrs);

    cardlist = GTK_CLIST(gtk_clist_new(numColumnHeaders(cols)));
    configure_source_card_dnd (GTK_WIDGET (cardlist));
    gtk_object_set_data(GTK_OBJECT(cardlist), "ColumnHeaders", cols);
    
    titles = gnomecardCreateColTitles(cols);

    for (l=titles, i=0; l; l=l->next, i++) {
	gtk_clist_set_column_title(cardlist, i, (gchar *)l->data);

	/* need to set widths based on user settings probably */
	gtk_clist_set_column_auto_resize(cardlist, i, TRUE);
    }
    
    gtk_clist_column_titles_show(cardlist);
    gnomecardFreeColTitles(titles);
    
/* 	gtk_widget_realize(GTK_WIDGET(gnomecard_list)); */
/*
  gdk_gc_get_values(gnomecard_tree->lines_gc, &crd_tree_values);
*/
#if 0
    gtk_clist_set_column_width(GTK_CLIST(gnomecard_list), 0, NAME_COL_WIDTH);
    gtk_clist_set_column_width(GTK_CLIST(gnomecard_list), 1, ORG_COL_WIDTH);
    gtk_clist_set_column_width(GTK_CLIST(gnomecard_list), 2, PHONE_COL_WIDTH);
    gtk_clist_set_column_width(GTK_CLIST(gnomecard_list), 3, EMAIL_COL_WIDTH);
#endif
    gtk_signal_connect(GTK_OBJECT(cardlist), "select_row",
		       GTK_SIGNAL_FUNC(gnomecard_list_selected), NULL);
    
    gtk_signal_connect(GTK_OBJECT(cardlist),
		       "click_column", 
		       GTK_SIGNAL_FUNC(gnomecard_sort_by_col),
		       (gpointer) cardlist);

    /*
     * Configure the Drag and drop
     */

/* NOT USED (yet)
   gtk_signal_connect(GTK_OBJECT(gnomecard_list), 
   "unselect_row",
   GTK_SIGNAL_FUNC(gnomecard_list_unselected), NULL);
*/
    
/* NOT USED - will add when I finallize the column header code
   gtk_signal_connect(GTK_OBJECT(GTK_CLIST(gnomecard_tree)->column->button),
   "clicked", GTK_SIGNAL_FUNC(gnomecard_sort_by_fname),
   (gpointer) NULL);
*/
    gtk_clist_set_selection_mode(cardlist, GTK_SELECTION_BROWSE);
    
    gtk_signal_connect(GTK_OBJECT(cardlist), "button_press_event",
		       GTK_SIGNAL_FUNC(gnomecard_list_button_press), NULL);
    
/*	omenu = gtk_option_menu_new();
	gtk_clist_set_column_widget(GTK_CLIST(gnomecard_tree), 1, omenu);
	menu = gtk_menu_new();
	for (i = 0; sort_type_name[i]; i++) {
	item = gtk_menu_item_new_with_label(sort_type_name[i]);
	gtk_object_set_user_data(GTK_OBJECT(item), (gpointer) i);
	gtk_menu_append(GTK_MENU(menu), item);
	gtk_widget_show(item);
	}
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu), menu);
	gtk_widget_show(omenu);*/
    
/*	
	crd_tree_sel_col = g_new (GdkColor, 1);
	crd_tree_sel_col->red = crd_tree_sel_col->green = 
	crd_tree_sel_col->blue = 50000;
	gdk_color_alloc(gtk_widget_get_colormap (GTK_WIDGET (gnomecard_tree)),
	crd_tree_sel_col);
	
	crd_tree_usel_col = g_new (GdkColor, 1);
	crd_tree_usel_col->red = crd_tree_usel_col->green = 
	crd_tree_usel_col->blue = 60000;
	gdk_color_alloc(gtk_widget_get_colormap (GTK_WIDGET (gnomecard_tree)),
	crd_tree_usel_col);
*/
/* & (GTK_WIDGET(crd_tree)->style->fg[GTK_STATE_NORMAL]);*/
    
    gtk_clist_column_titles_active(cardlist);
    gtk_widget_show(GTK_WIDGET(cardlist));
    
    return GTK_WIDGET(cardlist);
}
