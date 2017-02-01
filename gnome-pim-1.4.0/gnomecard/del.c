/* GnomeCard - a graphical contact manager.
 *
 * del.c: This file is part of GnomeCard.
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

#include <gnome.h>

#include "card.h"
#include "cardtypes.h"
#include "del.h"
#include "gnomecard.h"
#include "my.h"
#include "list.h"


/* New Code */

/* widget and data are ignored, just there for now so we can be */
/* a direct signal callback if necessary                        */
void
gnomecard_delete_current_card(GtkWidget *widget, gpointer data)
{
	GList *tmp, *curr;
	
	g_return_if_fail (gnomecard_curr_crd != NULL);
	
	curr = gnomecard_curr_crd;
	if (curr->next)
	  tmp = curr->next;
	else
	  tmp = curr->prev;
	
	gnomecard_list_remove_card(curr->data);
	
	card_free(curr->data);
	gnomecard_crds = g_list_remove_link(gnomecard_crds, curr);
	g_list_free(curr);
	
	if (tmp) 
	  gnomecard_scroll_list(tmp);
	else
	  gnomecard_set_curr(NULL);
	
	gnomecard_set_changed (TRUE);
}








/*********** OLD CODE *****************************************/
#if 0
static void del_prop(GtkCTreeNode *node, gpointer data)
{
	CardProperty *prop = data;
	
	card_prop_free(*prop);
	
        g_message(" in del_prop - did not remove from list");
/*
	gtk_ctree_remove_node(gnomecard_tree, node);
	gtk_ctree_select(gnomecard_tree, 
			 ((Card *) gnomecard_curr_crd->data)->prop.user_data);
*/
}

static void del_str_prop(GtkCTreeNode *node, gpointer data)
{
	MY_FREE(((CardStrProperty *) data)->str);
	del_prop(node, data);
}

static void del_card(GtkCTreeNode *node, gpointer data)
{
	GList *tmp;
	
	node = 0; /* avoid warnings */
	data = 0;
	
	if (gnomecard_curr_crd->next)
	  tmp = gnomecard_curr_crd->next;
	else
	  tmp = gnomecard_curr_crd->prev;
	
	card_free(gnomecard_curr_crd->data);
	gnomecard_crds = g_list_remove_link(gnomecard_crds, gnomecard_curr_crd);
/*
  gtk_ctree_remove_node(gnomecard_tree, ((Card *) gnomecard_curr_crd->data)->prop.user_data); */
	g_message("in del_card - did not remove node");
	g_list_free(gnomecard_curr_crd);
	
	if (tmp) {
	    /*gnomecard_scroll_tree(tmp); */
	    g_message("in del_card - did not scroll");
	} else
	  gnomecard_set_curr(NULL);
}

static void del_name(GtkCTreeNode *node, gpointer data)
{
	CardName *name = data;
	
	MY_FREE(name->family); card_prop_free(name->prop);
	MY_FREE(name->given);
	MY_FREE(name->additional);
	MY_FREE(name->prefix);
	MY_FREE(name->suffix);

	g_message("In del_name - did not remove node");
/*	
	gtk_ctree_remove_node(gnomecard_tree, node);
	gtk_ctree_select(gnomecard_tree, 
			 ((Card *) gnomecard_curr_crd->data)->prop.user_data);
*/
}

static void del_deladdr_list(GtkCTreeNode *node, gpointer data)
{
}

static void del_deladdr(GtkCTreeNode *node, gpointer data)
{
}

static void del_dellabel_list(GtkCTreeNode *node, gpointer data)
{
}

static void del_dellabel(GtkCTreeNode *node, gpointer data)
{
}

static void del_phone_list(GtkCTreeNode *node, gpointer data)
{
}

static void del_phone(GtkCTreeNode *node, gpointer data)
{
}

static void del_email_list(GtkCTreeNode *node, gpointer data)
{
	CardList *email_list;
	GList *email;


	g_message("del_email_list not implemented yet");
#if 0	
	email_list = data;
	card_prop_free(email_list->prop);
	
	while ((email = email_list->l)) {
	    /*
		gtk_ctree_remove_node(gnomecard_tree, 
				      ((CardEMail *) email->data)->prop.user_data);
	    */
	    g_message("in del_email_list - did not remove node");

		email_list->l = g_list_remove_link(email_list->l, email);
		g_free(((CardEMail *) email->data)->data);
		g_free(email->data);
		g_list_free(email);
	}

	g_message("in del_email_list - did not remove node and select");
/*	
	gtk_ctree_remove_node(gnomecard_tree, node);
	gtk_ctree_select(gnomecard_tree, 
			 ((Card *) gnomecard_curr_crd->data)->prop.user_data);
*/
#endif
}

static void del_email(GtkCTreeNode *node, gpointer data)
{
	CardList *email_list;
	GList *email;

	g_message("del_email not implemented yet");
#if 0	
	email_list = & ((Card *) gnomecard_curr_crd->data)->email;
	for (email = email_list->l; email; email = email->next)
	  if (email->data == data)
	    break;
	
	if (email) {
		GList *curr = NULL;
		
		if (email->next)
		  curr = email->next;
		else if (email->prev)
		  curr = email->prev;

/*		
		if (curr)
		  gtk_ctree_select(gnomecard_tree, 
				   ((CardEMail *) curr->data)->prop.user_data);
		else
		  gtk_ctree_select(gnomecard_tree, 
				   ((Card *) gnomecard_curr_crd->data)->prop.user_data);
*/
		g_message("in del_email - did not select");
		email_list->l = g_list_remove_link(email_list->l, email);
		g_free(((CardEMail *) email->data)->data);
		g_free(email->data);
		g_list_free(email);
		
/*		gtk_ctree_remove_node(gnomecard_tree, node); */
		g_message("in del_email - did not remove node");
		
		if (email_list->l && !email_list->l->next) {

/*			gnomecard_update_tree(gnomecard_curr_crd->data); */
		    g_message("in del_email, did not update tree");
/*			gtk_ctree_collapse_recursive(gnomecard_tree,
						     ((Card *) gnomecard_curr_crd->data)->prop.user_data);
			gtk_ctree_move(gnomecard_tree, ((CardEMail *) email->data)->prop.user_data,
				       ((Card *) gnomecard_curr_crd->data)->prop.user_data,
				       email_list->prop.user_data);
			gtk_ctree_remove_node(gnomecard_tree, email_list->prop.user_data);*/
		}
	}
#endif
}

static void del_org(GtkCTreeNode *node, gpointer data)
{
	CardOrg *org = data;
	
	MY_FREE(org->name); card_prop_free(org->prop);
	MY_FREE(org->unit1);
	MY_FREE(org->unit2);
	MY_FREE(org->unit3);
	MY_FREE(org->unit4);

/*	
	gtk_ctree_remove_node(gnomecard_tree, node);
	gtk_ctree_select(gnomecard_tree, 
			 ((Card *) gnomecard_curr_crd->data)->prop.user_data);
*/
	g_message("in del_org, did not remove node and select");
}

#if 0
typedef void (*DelFunc) (GtkCTreeNode *, gpointer);

extern void gnomecard_del(GtkWidget *widget, gpointer data)
{
	CardProperty *prop;
	DelFunc del_func[] = {
		NULL, del_card, del_card, del_name, NULL, del_prop,
		del_deladdr_list, del_deladdr, del_dellabel_list, 
		del_dellabel, del_phone_list, del_phone, del_email_list,
		del_email, del_str_prop, del_prop, del_prop, del_str_prop, 
		del_str_prop, NULL, NULL, del_org, del_str_prop, NULL, NULL, 
		del_str_prop, del_str_prop, NULL, NULL };

/*	
	prop = gtk_ctree_node_get_row_data(gnomecard_tree, gnomecard_selected_node);
*/
	g_message("in gnomecard_del - did not get row data for prop");
/*
	if (del_func[prop->type])
	  (*del_func[prop->type]) (gnomecard_selected_node, prop);
*/
	g_message("in gnomecard_del - did not delete selected node");
	
	gnomecard_set_changed(TRUE);
}
#endif


#endif
