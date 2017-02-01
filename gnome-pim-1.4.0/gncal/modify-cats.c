/* 
 * modify-cats.c
 * dialog for modifying categories for an item
 * part of gnome-cal 
 */


/*
** Copyright (C) 2000 Dirk-Jan C. Binnema <djcb@dds.nl>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/

#include "modify-cats.h"


static GtkWidget *list_selected, *list_available;

static void fill_stock_categories (GList **categories );
static void populate_list_boxes ( GList **category_list );
static gboolean already_in_categories ( GList *categories, gchar* cat );



static void cb_add_button_pressed (GtkWidget *button, gpointer data);
static void cb_remove_button_pressed (GtkWidget *button, gpointer data);
static void cb_select_list_selected ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data);
static void cb_unselect_list_selected ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data);
static void cb_select_list_available ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data);
static void cb_unselect_list_available ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data);

/*
 * contruct & show the 'modify categories' dialog
 */
gint
modify_cats_dialog ( GtkWidget *parent, GList **categories )
{
	GtkWidget *dialog;
	GtkWidget *commlist_box;
	GtkWidget *main_label;
	GtkWidget *list_label_box;
	GtkWidget *label_selected, *label_available;
	GtkWidget *button_box;
	GtkWidget *add_button, *remove_button;
	GtkWidget *add_icon, *remove_icon;
	GtkWidget *scrollwin_left, *scrollwin_right;
	GtkTooltips *tooltips;

	gchar* buttons[2];
	buttons[0] = _("Done") ;
	buttons[1] = NULL;
	
	dialog = gnome_dialog_newv ( _("Category selection"), (const gchar**)buttons);
	main_label = gtk_label_new ( _("Please select categories for the current item") );
	gtk_widget_show (main_label);
 
	list_label_box  = gtk_hbox_new ( FALSE, 0 );
	label_selected  = gtk_label_new ( _("Selected"));
	label_available = gtk_label_new ( _("Available"));
	gtk_box_pack_start_defaults ( GTK_BOX (list_label_box), label_selected  );
	gtk_box_pack_start_defaults ( GTK_BOX (list_label_box), label_available );
	gtk_widget_show (list_label_box );
	gtk_widget_show ( label_selected );
	gtk_widget_show ( label_available );
	
	commlist_box    = gtk_hbox_new ( FALSE, 5);
	scrollwin_left  = gtk_scrolled_window_new ( NULL, NULL );
	scrollwin_right = gtk_scrolled_window_new ( NULL, NULL );
       	list_selected   = gtk_clist_new (3);
	list_available  = gtk_clist_new (3);
	gtk_clist_set_selection_mode ( GTK_CLIST(list_selected), GTK_SELECTION_MULTIPLE);
	gtk_clist_set_selection_mode ( GTK_CLIST(list_available), GTK_SELECTION_MULTIPLE);

	button_box      = gtk_vbox_new ( FALSE, 0);
	add_icon        = gnome_stock_pixmap_widget ( NULL, GNOME_STOCK_BUTTON_PREV );
	add_button      = gtk_button_new();
	gtk_container_add (GTK_CONTAINER(add_button), add_icon);
	remove_icon     = gnome_stock_pixmap_widget ( NULL, GNOME_STOCK_BUTTON_NEXT );
	remove_button   = gtk_button_new();
	gtk_container_add (GTK_CONTAINER(remove_button), remove_icon);
       
	/* tooltips */
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip (GTK_TOOLTIPS(tooltips), add_button, 
			      _("Add a category"),
			      _("Add a category to the list of selected categories"));

	gtk_tooltips_set_tip (GTK_TOOLTIPS(tooltips), remove_button, 
			      _("Remove a category"),
			      _("Remove a category from  the list of selected categories"));

	gtk_signal_connect ( GTK_OBJECT (list_selected),  "select-row",     cb_select_list_selected,  NULL );
	gtk_signal_connect ( GTK_OBJECT (list_selected),  "unselect-row",   cb_unselect_list_selected, NULL );
	gtk_signal_connect ( GTK_OBJECT (list_available),  "select-row",    cb_select_list_available,  NULL );
	gtk_signal_connect ( GTK_OBJECT (list_available),  "unselect-row",  cb_unselect_list_available, NULL );
	
	gtk_signal_connect ( GTK_OBJECT (add_button), "pressed",    cb_add_button_pressed,    categories );
	gtk_signal_connect ( GTK_OBJECT (remove_button), "pressed", cb_remove_button_pressed, categories );

	gtk_box_pack_start (GTK_BOX(button_box), add_button, FALSE, FALSE, 0 );
	gtk_box_pack_start (GTK_BOX(button_box), remove_button, FALSE, FALSE, 0 );
	gtk_container_add  (GTK_CONTAINER(scrollwin_left), list_selected );
	gtk_container_add  (GTK_CONTAINER(scrollwin_right), list_available );
	gtk_widget_set_usize (GTK_WIDGET(scrollwin_left), 50, 100 );
	gtk_widget_set_usize (GTK_WIDGET(scrollwin_right), 50, 100 );
	
	gtk_box_pack_start_defaults (GTK_BOX(commlist_box), scrollwin_left);
	gtk_box_pack_start_defaults (GTK_BOX(commlist_box), button_box);
	gtk_box_pack_start_defaults (GTK_BOX(commlist_box), scrollwin_right);
	gtk_widget_show ( add_icon );
	gtk_widget_show ( remove_icon);
	gtk_widget_show ( add_button );
	gtk_widget_show ( remove_button );
	gtk_widget_show ( scrollwin_left );
	gtk_widget_show ( scrollwin_right );
	gtk_widget_show ( list_selected );
	gtk_widget_show ( list_available );
	gtk_widget_show ( button_box );
	gtk_widget_show ( commlist_box );
	
	gtk_box_pack_start_defaults ( GTK_BOX (GNOME_DIALOG(dialog)->vbox), main_label);
	gtk_box_pack_start_defaults ( GTK_BOX (GNOME_DIALOG(dialog)->vbox), list_label_box);
	gtk_box_pack_start_defaults ( GTK_BOX (GNOME_DIALOG(dialog)->vbox), commlist_box);
	
	fill_stock_categories ( categories );
	populate_list_boxes ( categories );

	gnome_dialog_set_parent ( GNOME_DIALOG(dialog), GTK_WINDOW(parent) );
	
	gtk_widget_set_usize ( GTK_WIDGET(dialog), 300, 250 );
	return gnome_dialog_run_and_close (GNOME_DIALOG(dialog));
}


void 
populate_list_boxes ( GList **categories )
{
	gint row_id = 0;
	
	Category* cursor_item;
	gchar*  dummy[] = { NULL, NULL, NULL}; 
	GList *cursor = g_list_first(*categories);

	gtk_clist_clear (GTK_CLIST(list_available));
	gtk_clist_clear (GTK_CLIST(list_selected));
	
	while ( cursor ) {
		
		cursor_item = (Category*) cursor->data;
	
		if ( cursor_item->is_available ) {
			
			row_id = gtk_clist_append ( GTK_CLIST(list_available), dummy );
			gtk_clist_set_text ( GTK_CLIST(list_available), row_id, 2, cursor_item->name);
			gtk_clist_set_row_data ( GTK_CLIST(list_available), row_id, (gpointer) cursor_item );
	
		} else {

			row_id = gtk_clist_append ( GTK_CLIST(list_selected), dummy );
			gtk_clist_set_text ( GTK_CLIST(list_selected), row_id, 2, cursor_item->name);
			gtk_clist_set_row_data ( GTK_CLIST(list_selected), row_id, (gpointer) cursor_item );
		}

		cursor = g_list_next ( cursor );
	}
}


void
cb_add_button_pressed ( GtkWidget* button, gpointer category_list )
{
        GList  *list  = *((GList**) category_list); 
	Category* cursor_item;

	while ( list ) {
		
		cursor_item = (Category*) list->data;
		
		if ( (TRUE == cursor_item -> is_available ) 
		    && ( TRUE == cursor_item -> selected )) {
	
			cursor_item -> is_available = FALSE;
			cursor_item -> selected     = FALSE;
	
		}
		list = g_list_next ( list );
	}
	
	populate_list_boxes ( category_list );
}


void
cb_remove_button_pressed ( GtkWidget* button, gpointer category_list )
{
	GList  *list = *((GList**) category_list);  
	Category* cursor_item;

	while ( list ) {
		
		cursor_item = (Category*) list->data;
		
		if ( (FALSE == cursor_item -> is_available ) 
		    && ( TRUE == cursor_item -> selected )) {
	
			cursor_item -> is_available = TRUE;
			cursor_item -> selected     = FALSE;
		}

		list = g_list_next (list);
	}

	populate_list_boxes ( category_list );
}



void 
cb_select_list_selected ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	Category* cat = (Category*) gtk_clist_get_row_data (GTK_CLIST(list_selected), row);
	cat -> selected = TRUE;
}



void 
cb_unselect_list_selected ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	Category* cat = (Category*) gtk_clist_get_row_data (GTK_CLIST(list_selected), row);
	cat -> selected = FALSE;
}


void 
cb_select_list_available ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	Category* cat = (Category*) gtk_clist_get_row_data (GTK_CLIST(list_available), row);
	cat -> selected = TRUE;
}



void 
cb_unselect_list_available ( GtkCList *clist, gint row, gint column, GdkEventButton *event, gpointer data)
{
	Category* cat = (Category*) gtk_clist_get_row_data (GTK_CLIST(list_available), row);
	cat -> selected = FALSE;
}



GList*
ico_categories_as_categories ( GList *ico_categories )
{
	GList *ico_cursor = g_list_first(ico_categories);
	GList *categories  = NULL;
	Category* new_cat;

	while ( ico_cursor ) {

		new_cat = g_new (Category, 1);
		new_cat -> name = (gchar*)ico_cursor->data;
		new_cat -> name_end = NULL;
		new_cat -> is_available = FALSE;
		new_cat -> selected     = FALSE;

		categories = g_list_append (categories, new_cat);

		ico_cursor = g_list_next (ico_cursor);
	}


	return g_list_first (categories);
}


GList*
categories_as_ico_categories ( GList *categories )
{
	GList *cursor = g_list_first(categories);
        GList* ico_categories = NULL;
	Category * this_cat;
	
	while ( cursor ) {
		
		this_cat = (Category*) cursor->data;

		if ( FALSE == this_cat -> is_available ) 
			ico_categories = g_list_append ( ico_categories, this_cat->name ); 
	       
				
		cursor = g_list_next ( cursor );
	}

	return g_list_first (ico_categories);
}



/*
 * print non-available (ie. selected) categories
 * must be freed by caller
 */
gchar*
categories_as_string ( GList *categories )
{
	GList *cursor;
	Category* cursor_item;
	gchar *sep = ", ", *cat_string;
	gboolean first_cat = TRUE;
	gint size = 1;

	if ( NULL == categories ) {
		
	        cat_string = g_new0 ( gchar, 1 );
		sprintf ( cat_string, "%s", "" );
		
		return cat_string;
	}

	/* determine length */
	cursor = categories;
	while ( cursor ) {
	
		cursor_item = (Category*) cursor->data;

		if ( FALSE == cursor_item -> is_available ) 
			size += strlen (cursor_item->name) + strlen(sep);

		cursor = g_list_next (cursor);
	}

	cat_string = g_new0 ( gchar, size );

	/* fill string */
	cursor = categories;
	while ( cursor ) {
		
		cursor_item = (Category*) cursor->data;

		if ( FALSE == cursor_item -> is_available ) {
			sprintf ( cat_string, "%s%s%s", cat_string, ((first_cat)?"":sep), cursor_item->name);
			first_cat = FALSE;
		}
		
		cursor = g_list_next (cursor);
	}
		
	return cat_string;
}


/*
 *  must be freed by caller
 */
gchar*
ico_categories_as_string ( GList *ico_categories )
{
	GList *cursor;
	gchar *sep = ", ", *cat_string;
	gboolean first_cat = TRUE;
	gint size = 1;

	if ( NULL == ico_categories ) {
		
	        cat_string = g_new0 ( gchar, 1 );
		sprintf ( cat_string, "%s", "" );
		
		return cat_string;
	}

	/* determine length */
	for ( cursor = ico_categories; cursor; cursor=g_list_next(cursor) )
	
		size += strlen ( (gchar*) cursor->data ) + strlen(sep);
	       
	
	cat_string = g_new0 ( gchar, size );

	/* fill string */
	for ( cursor = ico_categories; cursor; cursor=g_list_next(cursor) ) {
		
		sprintf ( cat_string, "%s%s%s", cat_string, (first_cat)?"":sep, (gchar*) cursor->data );
		first_cat = FALSE;
	}		
	return cat_string;
}


	
void
fill_stock_categories ( GList **categories )
{
	/*  stock categories to attach to todo items */
	static Category stock_category[] = {
		{ N_("Church"),		NULL, TRUE,  FALSE },
		{ N_("Friends"),	NULL, TRUE,  FALSE },
		{ N_("Guadec"),		NULL, TRUE,  FALSE },
		{ N_("Hacking"),	NULL, TRUE,  FALSE },
		{ N_("Hobby"),		NULL, TRUE,  FALSE },
		{ N_("Housekeeping"),	NULL, TRUE,  FALSE },
		{ N_("Love"),		NULL, TRUE,  FALSE },
		{ N_("Meeting"),	NULL, TRUE,  FALSE },
		{ N_("Money"),		NULL, TRUE,  FALSE },
		{ N_("Music"),		NULL, TRUE,  FALSE },
		{ N_("Paperwork"),      NULL, TRUE,  FALSE },
		{ N_("Relatives"),      NULL, TRUE,  FALSE },
		{ N_("School"),		NULL, TRUE,  FALSE },
		{ N_("Shopping"),	NULL, TRUE,  FALSE },
		{ N_("Travel"),		NULL, TRUE,  FALSE },
		{ N_("Work"),		NULL, TRUE,  FALSE },
		{ NULL }
	};

	Category* new_cat;
	gint index = 0;

	GList *cursor = *categories;

	while ( stock_category[index].name ) {

		if ( ! already_in_categories ( cursor, stock_category[index].name )) {
			
			new_cat = g_new ( Category, 1 );
			new_cat->name = stock_category[index].name;
			new_cat->name_end = NULL;
			new_cat->is_available = TRUE;
			new_cat->selected = FALSE;
			
			*categories = g_list_append (*categories, new_cat );
		}

		index++;
	}
}

		     



gboolean
already_in_categories ( GList *categories, gchar* cat ) 
{

	GList *cursor = categories;
	gboolean found = FALSE;
	Category *cursor_item;
	
	if ( NULL == cursor )
		return FALSE;

	while ( cursor ) {

		cursor_item = (Category*) cursor->data;

		if ( 0 == strcmp ( cat, cursor_item->name  )) {
			found = TRUE;
			break;
		}

		cursor = g_list_next ( cursor );
	}

	return found;
}



  












