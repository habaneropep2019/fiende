/* GnomeCard - a graphical contact manager.
 *
 * dialog.c: This file is part of GnomeCard.
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

#include <config.h>
#include <gnome.h>

#include <fnmatch.h>

#include "card.h"
#include "canvas.h"
#include "dialog.h"
#include "gnomecard.h"
#include "columnhdrs.h"
#include "my.h"
#include "sort.h"
#include "list.h"
#include "deladdrlist.h"
#include "phonelist.h"
#include "world.xpm"
#include "misc.h"

typedef struct {
    GtkWidget *srcList;
    GtkWidget *destList;
    GtkWidget *addButton;
    GnomePropertyBox *box;

    GList     *allHdrs;
    GList     *selHdrs;
} ColumnHdrEditor;

typedef struct
{
	GtkWidget *entry, *sens, *back;
} GnomeCardFind;

static void gnomecard_prop_close(GtkWidget *widget, gpointer node);
static void gnomecard_take_from_name(GtkWidget *widget, gpointer data);
static void gnomecard_cancel(GtkWidget *widget, gpointer data);
static void gnomecard_setup_apply(GtkWidget *widget, int page);
static void gnomecard_find_card(GtkWidget *w, gpointer data);
static void gnomecard_save_call(GtkWidget *widget, gpointer data);
static int gnomecard_match_pattern(char *pattern, char *str, int sens);

/* handle when the user applies changes to the card editor propertybox */
static void gnomecard_prop_apply(GtkWidget *widget, int page)
{
	GnomeCardEditor *ce;
	GList *node;
	Card *crd;
	struct tm *tm;
	time_t tt;

	if (page != -1)
	  return;             /* ignore partial applies */
	
	ce = (GnomeCardEditor *) gtk_object_get_user_data(GTK_OBJECT(widget));
	crd = (Card *) ce->l->data;
	
	MY_FREE(crd->fname.str);
	MY_FREE(crd->name.family);
	MY_FREE(crd->name.given);
	MY_FREE(crd->name.additional);
	MY_FREE(crd->name.prefix);
	MY_FREE(crd->name.suffix);
	
	crd->fname.str       = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->fn)));
	crd->name.family     = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->fam)));
	crd->name.given      = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->given)));
	crd->name.additional = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->add)));
	crd->name.prefix     = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->pre)));
	crd->name.suffix     = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->suf)));

	tt = gnome_date_edit_get_date(GNOME_DATE_EDIT(ce->bday)); 
	tm = localtime(&tt);
	
	crd->bday.year       = tm->tm_year + 1900;
	crd->bday.month      = tm->tm_mon + 1;
	crd->bday.day        = tm->tm_mday;
	
	crd->timezn.sign  = (gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ce->tzh)) < 0)? -1: 1;
	crd->timezn.hours = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ce->tzh));
	crd->timezn.mins  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ce->tzm));
	
	crd->geopos.lon = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(ce->gplon));
	crd->geopos.lat = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(ce->gplat));
	
	MY_FREE(crd->title.str);
	MY_FREE(crd->role.str);
	MY_FREE(crd->org.name);
	MY_FREE(crd->org.unit1);
	MY_FREE(crd->org.unit2);
	MY_FREE(crd->org.unit3);
	MY_FREE(crd->org.unit4);

	crd->title.str = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->title)));
	crd->org.name  = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->orgn)));
#if 0
	/* NOT USED */
	crd->role.str  = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->role)));
	crd->org.unit1 = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->org1)));
	crd->org.unit2 = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->org2)));
	crd->org.unit3 = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->org3)));
	crd->org.unit4 = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->org4)));
#else
	crd->role.str  = MY_STRDUP("");
	crd->org.unit1 = MY_STRDUP("");
        crd->org.unit2 = MY_STRDUP("");
        crd->org.unit3 = MY_STRDUP("");
        crd->org.unit4 = MY_STRDUP("");
#endif	
	MY_FREE(crd->categories.str);
	MY_FREE(crd->comment.str);
	MY_FREE(crd->url.str);
	
	crd->categories.str = MY_STRDUP(gtk_entry_get_text(GTK_ENTRY(ce->categories)));
	crd->comment.str = gtk_editable_get_chars(GTK_EDITABLE(ce->comment), 
			 0, gtk_text_get_length(GTK_TEXT(ce->comment)));
	crd->url.str     = MY_STRDUP(gtk_entry_get_text
				     (GTK_ENTRY(ce->emaillist->url)));

	/* email list */
	/* remove old email list */
	{
		CardEMail *email;
		GList *node_tmp;
		
		for (node = crd->email.l; node; node = node_tmp) {
			node_tmp = node->next;
			email = (CardEMail *) node->data;
			g_free (email->data);
			g_free (email);
			g_list_free_1 (node);
		}
	}

	/* link to new email list */
	crd->email.l = emaillist_get_entries (ce->emaillist);
	
	
	/* delivery addresses */
	/* remove old delivery addresses list */
	{
		CardDelAddr *deladdr;
		GList *node_tmp;
		int i;
		
		for (node = crd->deladdr.l; node; node = node_tmp) {
			node_tmp = node->next;
			deladdr = (CardDelAddr *) node->data;
			
			for (i = 0; i < DELADDR_MAX; i++)
			  g_free (deladdr->data[i]);
			
			g_free (deladdr);
			g_list_free_1 (node);
		}
	}

	/* link to new address list */
	crd->deladdr.l = deladdrlist_get_entries (ce->deladdrlist);

	
	/* phone numbers */
	/* remove old phone list */
	{
		CardPhone *phone;
		GList *node_tmp;
		
		for (node = crd->phone.l; node; node = node_tmp) {
			node_tmp = node->next;
			phone = (CardPhone *) node->data;
			g_free (phone->data);
			g_free (phone);
			g_list_free_1 (node);
		}
	}

	/* link to new phone list */
	crd->phone.l = phonelist_get_entries (ce->phonelist);

        /* Update revision */
        tt = time(NULL);
        tm = gmtime(&tt);
        crd->rev.utc = 0;
        crd->rev.tm = *tm;
        crd->rev.prop.used = TRUE;
        
	/* key data */
        MY_FREE(crd->key.data);
	
	crd->key.data = gtk_editable_get_chars(GTK_EDITABLE(ce->key), 
						   0, gtk_text_get_length(GTK_TEXT(ce->key)));
	if (GTK_TOGGLE_BUTTON(ce->keypgp)->active)
	  crd->key.type = KEY_PGP;
	else
	  crd->key.type = KEY_X509;

/*	gnomecard_update_list(crd);
	gnomecard_scroll_list(ce->l);
*/
	gnomecard_sort_card_list_by_default();
	gnomecard_rebuild_list();
	gnomecard_update_canvas(crd);
	gnomecard_set_changed(TRUE);
}

/*
static void gnomecard_prop_close(GtkWidget *widget, gpointer node)
{
	((Card *) ((GList *) node)->data)->flag = FALSE;
	
	if ((GList *) node == gnomecard_curr_crd)
	  gnomecard_set_edit_del(TRUE);
}
*/
static void gnomecard_prop_close(GtkWidget *widget, gpointer data)
{
    GnomeCardEditor *ce;
    Card *card;
    GList *l;

    /* cleanup */
    ce = (GnomeCardEditor *) gtk_object_get_user_data(GTK_OBJECT(widget));
    deladdrlist_del_entries (ce->deladdrlist);
    phonelist_del_entries (ce->phonelist);

    card = (Card *) data;
    card->flag = FALSE;

    for (l = gnomecard_crds; l; l = l->next) {
	if (card == l->data)
	    break;
    }

    if (l == gnomecard_curr_crd)
	gnomecard_set_edit_del(TRUE);
}

static void gnomecard_take_from_name(GtkWidget *widget, gpointer data)
{
        GnomeCardEditor *ce;
	char *name;
	
	ce = (GnomeCardEditor *) data;
	
	name = gnomecard_join_name(gtk_entry_get_text(GTK_ENTRY(ce->pre)),
			 gtk_entry_get_text(GTK_ENTRY(ce->given)),
			 gtk_entry_get_text(GTK_ENTRY(ce->add)),
			 gtk_entry_get_text(GTK_ENTRY(ce->fam)),
			 gtk_entry_get_text(GTK_ENTRY(ce->suf)));
	
	gtk_entry_set_text(GTK_ENTRY(ce->fn), name);
	
	g_free(name);
}
	
extern void gnomecard_edit(GList *node)
{
        static GnomeHelpMenuEntry help_entry = { NULL, "edit" };
	GnomePropertyBox *box;
	GnomeCardEditor *ce;
	GtkWidget *hbox, *hbox2, *vbox, *frame, *table;
	GtkWidget *label, *entry, *align, *align2, *pix;
	GtkWidget *radio1, *radio2, *button;
	GtkObject *adj;
	GtkWidget *nametable;
	Card *crd;
	time_t tmp_time;

	help_entry.name = gnome_app_id;

	ce = g_new0(GnomeCardEditor, 1);
	ce->l = node;
	crd = (Card *) node->data;
	
	/* Set flag and disable Delete and Edit. */
	crd->flag = TRUE;
	gnomecard_set_edit_del(FALSE);
	/*gnomecard_set_add(TRUE);*/
	
	box = GNOME_PROPERTY_BOX(gnome_property_box_new());
	gtk_object_set_user_data(GTK_OBJECT(box), ce);
	gtk_window_set_wmclass(GTK_WINDOW(box), "gnomecard",
			       "gnomecard");
	gtk_signal_connect(GTK_OBJECT(box), "apply",
			   (GtkSignalFunc)gnomecard_prop_apply, NULL);
/*	gtk_signal_connect(GTK_OBJECT(box), "destroy",
			   (GtkSignalFunc)gnomecard_prop_close, node);
*/
/* changed to use card ptr, not pointer to list entry containing card */
	gtk_signal_connect(GTK_OBJECT(box), "destroy",
			   (GtkSignalFunc)gnomecard_prop_close, crd);

	gtk_signal_connect(GTK_OBJECT(box), "help",
			   GTK_SIGNAL_FUNC (gnome_help_pbox_display),
			   &help_entry);

	/* Identity notebook page*/
	vbox = my_gtk_vbox_new();
	label = gtk_label_new(_("Identity"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), vbox, label);
	
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	label = gtk_label_new(_("File As:"));
	ce->fn = entry = my_gtk_entry_new(0, crd->fname.str);
	my_connect(entry, "changed", box, &crd->fname.prop, PROP_FNAME);
	button = gtk_button_new_with_label(_("Take from Name"));
 	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
				  GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(box));
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(gnomecard_take_from_name),
			   ce);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	/* create name frame */
	frame = gtk_frame_new(_("Name"));	
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, FALSE, 0);

	/* pack all name fields into a table */
	nametable = my_gtk_table_new(1, 4);
	gtk_container_add(GTK_CONTAINER(frame), nametable);

	/* first name */
	label = gtk_label_new(_("First:"));
	ce->given = entry = my_gtk_entry_new(0, crd->name.given);
	my_connect(entry, "changed", box, &crd->name.prop, PROP_NAME);
	align = gtk_alignment_new(0.0, 0.0, 0, 0);
        gtk_container_add (GTK_CONTAINER (align), entry);
	gtk_table_attach(GTK_TABLE(nametable), label, 0, 1, 0, 1,
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 
			 0, 0);
	gtk_table_attach(GTK_TABLE(nametable), align, 1, 2, 0, 1, 
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 
			 GNOME_PAD_SMALL, GNOME_PAD_SMALL);

	label = gtk_label_new(_("Middle:"));
	ce->add = entry = my_gtk_entry_new(0, crd->name.additional);
	my_connect(entry, "changed", box, &crd->name.prop, PROP_NAME);
	gtk_table_attach(GTK_TABLE(nametable), label, 2, 3, 0, 1,
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 
			 0, 0);
	gtk_table_attach(GTK_TABLE(nametable), entry, 3, 4, 0, 1, 
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 
			 GNOME_PAD_SMALL, GNOME_PAD_SMALL);

	label = gtk_label_new(_("Last:"));
	ce->fam = entry = my_gtk_entry_new(0, crd->name.family);
	align = gtk_alignment_new(0.0, 0.0, 0, 0);
        gtk_container_add (GTK_CONTAINER (align), entry);
	my_connect(entry, "changed", box, &crd->name.prop, PROP_NAME);
	gtk_table_attach(GTK_TABLE(nametable), label, /*4, 5, 0, 1, */
			                              0, 1, 1, 2, 
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 
			 0, 0);
	gtk_table_attach(GTK_TABLE(nametable), align, /*5, 6, 0, 1,*/
			                              1, 4 , 1, 2,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			 GNOME_PAD_SMALL, GNOME_PAD_SMALL);

	label = gtk_label_new(_("Prefix:"));
	ce->pre = entry = my_gtk_entry_new(5, crd->name.prefix);
	align = gtk_alignment_new(0.0, 0.0, 0, 0);
        gtk_container_add (GTK_CONTAINER (align), entry);
	my_connect(entry, "changed", box, &crd->name.prop, PROP_NAME);
	gtk_table_attach(GTK_TABLE(nametable), label, 0, 1, 2, 3,
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(nametable), align, 1, 2, 2, 3, 
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 
			 GNOME_PAD_SMALL, 0);

	label = gtk_label_new(_("Suffix:"));
	ce->suf = entry = my_gtk_entry_new(5, crd->name.suffix);
	align = gtk_alignment_new(0.0, 0.0, 0, 0); 
        gtk_container_add (GTK_CONTAINER (align), entry);
	my_connect(entry, "changed", box, &crd->name.prop, PROP_NAME);
        gtk_table_attach(GTK_TABLE(nametable), label, 2, 3, 2, 3,
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(nametable), align, 3, 4, 2, 3, 
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			 GNOME_PAD_SMALL, 0);

	/* organization and internet info share same line */
	/* add organization to identity notetab */
	hbox2 = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);

	frame = gtk_frame_new(_("Organization"));
	gtk_box_pack_start(GTK_BOX(hbox2), frame, TRUE, TRUE, 0);

	table = my_gtk_table_new(2, 2);
	gtk_container_add(GTK_CONTAINER(frame), table);

	label = gtk_label_new(_("Name:"));
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	ce->orgn = entry = my_gtk_entry_new(0, crd->org.name);
	my_connect(entry, "changed", box, &crd->org.prop, PROP_ORG);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 
			 0, 0);
	gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 0, 1,
			 GTK_FILL | GTK_SHRINK | GTK_EXPAND, GTK_FILL | GTK_SHRINK,
			 GNOME_PAD_SMALL, GNOME_PAD_SMALL);

	label = gtk_label_new(_("Title:"));
	ce->title = entry = my_gtk_entry_new(0, crd->title.str);
	my_connect(entry, "changed", box, &crd->title.prop, PROP_TITLE);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
			 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 
			 0, 0);
	gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 1, 2,
			 GTK_FILL | GTK_SHRINK | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 
			 GNOME_PAD_SMALL, GNOME_PAD_SMALL);

	/* email page */
        ce->emaillist = emaillist_create_edit_page(crd, box);
	label = gtk_label_new(_("Network"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), 
				 ce->emaillist->the_vbox, label);

	/* delivery address page */
	ce->deladdrlist = deladdrlist_create_edit_page(crd, box);
	label = gtk_label_new(_("Addresses"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), 
				 ce->deladdrlist->the_vbox, label);

	/* phone number page */
        ce->phonelist = phonelist_create_edit_page(crd, box);
	label = gtk_label_new(_("Phone"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), 
				 ce->phonelist->the_vbox, label);

	/* Birth date */
	
	vbox = my_gtk_vbox_new();
	gtk_box_pack_start(GTK_BOX(hbox2), vbox, FALSE, FALSE, 0);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new(_("Birthdate:"));
	
	if (crd->bday.prop.used) {
		struct tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		
		tm.tm_year = crd->bday.year - 1900;
		tm.tm_mon  = crd->bday.month - 1;
		tm.tm_mday = crd->bday.day;
		
		tmp_time = mktime(&tm);
	} else
	  tmp_time = time(NULL);
	
	ce->bday = entry = gnome_date_edit_new(tmp_time, FALSE, FALSE);
	my_connect(GNOME_DATE_EDIT(entry)->calendar, "day_selected",
		   box, &crd->bday.prop, PROP_BDAY);
	my_connect(GNOME_DATE_EDIT(entry)->date_entry, "changed",
		   box, &crd->bday.prop, PROP_BDAY);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);

	/* Geographical */
	align2 = gtk_alignment_new(0.5, 0.5, 0, 0);
	hbox2 = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_container_add (GTK_CONTAINER (align2), hbox2);
	label = gtk_label_new(_("Geographical"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), align2, label);
	
	pix = gnome_pixmap_new_from_xpm_d (world_xpm);
	gtk_box_pack_start(GTK_BOX(hbox2), pix, FALSE, FALSE, 0);
	
	vbox = my_gtk_vbox_new();
	gtk_box_pack_start(GTK_BOX(hbox2), vbox, FALSE, FALSE, 0);
	frame = gtk_frame_new(_("Time Zone"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width(GTK_CONTAINER(hbox), GNOME_PAD_SMALL);
	align = gtk_alignment_new(0.5, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (align), hbox);
	gtk_container_add(GTK_CONTAINER(frame), align);
	label = gtk_label_new(_("hrs."));
	adj = gtk_adjustment_new(crd->timezn.prop.used? 
				 crd->timezn.sign * crd->timezn.hours : 0,
				 -12, 12, 1, 1, 3);
	ce->tzh = entry = my_gtk_spin_button_new(GTK_ADJUSTMENT(adj), 3);
	my_connect(adj, "value_changed", box, &crd->timezn.prop, PROP_TIMEZN);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	label = gtk_label_new(_("mins."));
	adj = gtk_adjustment_new(crd->timezn.prop.used? crd->timezn.mins : 0,
				 0, 59, 1, 1, 10);
	ce->tzm = entry = my_gtk_spin_button_new(GTK_ADJUSTMENT(adj), 2);
	my_connect(adj, "value_changed", box, &crd->timezn.prop, PROP_TIMEZN);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		
	frame = gtk_frame_new(_("Geographic Position"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width(GTK_CONTAINER(hbox), GNOME_PAD_SMALL);
	align = gtk_alignment_new(0.5, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (align), hbox);
	gtk_container_add(GTK_CONTAINER(frame), align);
	label = gtk_label_new(_("lat, "));
	adj = gtk_adjustment_new(crd->geopos.prop.used? crd->geopos.lat : 0,
				 -90, 90, .01, 1, 1);
	ce->gplat = entry = my_gtk_spin_button_new(GTK_ADJUSTMENT(adj), 5);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(entry), 2);
	my_connect(adj, "value_changed", box, &crd->geopos.prop, PROP_GEOPOS);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	label = gtk_label_new(_("lon."));
	adj = gtk_adjustment_new(crd->geopos.prop.used? crd->geopos.lon : 0,
				 -180, 180, .01, 1, 1);
	ce->gplon = entry = my_gtk_spin_button_new(GTK_ADJUSTMENT(adj), 6);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(entry), 2);
	my_connect(adj, "value_changed", box, &crd->geopos.prop, PROP_GEOPOS);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	/* Explanatory */
	
	vbox = my_gtk_vbox_new();
	label = gtk_label_new(_("Explanatory"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), vbox, label);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	label = gtk_label_new(_("Categories:"));
	ce->categories = entry = my_gtk_entry_new(0, crd->categories.str);
	my_connect(entry, "changed", box, &crd->categories.prop, PROP_CATEGORIES);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new(_("Comment:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	ce->comment = entry = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(entry), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
	gtk_widget_realize(entry);
	if (crd->comment.prop.used) {
		int pos = 0;
		gtk_editable_insert_text(GTK_EDITABLE(entry), crd->comment.str,
					 strlen(crd->comment.str), &pos);
	}
	gtk_widget_set_usize (entry, 0, (entry->style->font->ascent +
					 entry->style->font->descent) * 4);
	my_connect(entry, "changed", box, &crd->comment.prop, PROP_COMMENT);
	
/* URL is above now 
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	label = gtk_label_new(_("URL:"));
	ce->url = entry = my_gtk_entry_new(0, crd->url.str);
	my_connect(entry, "changed", box, &crd->url.prop, PROP_URL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
*/

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, GNOME_PAD);
/*	label = gtk_label_new(_("Last Revision:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	label = gtk_label_new(_("The last revision goes here."));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);*/
	
	/* Security */
	
	vbox = my_gtk_vbox_new();
	label = gtk_label_new(_("Security"));
	gtk_notebook_append_page(GTK_NOTEBOOK(box->notebook), vbox, label);

	label = gtk_label_new(_("Public Key:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	ce->key = entry = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(entry), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
	gtk_widget_realize(entry);
	if (crd->key.prop.used) {
		int pos = 0;
		gtk_editable_insert_text(GTK_EDITABLE(entry), crd->key.data,
					 strlen(crd->key.data), &pos);
	}
	gtk_widget_set_usize (entry, 0, (entry->style->font->ascent +
					 entry->style->font->descent) * 6);
	my_connect(entry, "changed", box, &crd->key.prop, PROP_KEY);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	ce->keypgp = radio1 = gtk_radio_button_new_with_label(NULL, _("PGP"));
	if (crd->key.prop.used && crd->key.type != KEY_PGP)
	  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(radio1), 0);
	my_connect(radio1, "toggled", box, &crd->key.prop, PROP_KEY);
	gtk_box_pack_start(GTK_BOX(hbox), radio1, FALSE, FALSE, 0);
	radio2 = gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(radio1)),
		_("X509"));
	if (crd->key.prop.used && crd->key.type == KEY_X509)
	  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(radio2), 1);
	my_connect(radio2, "toggled", box, &crd->key.prop, PROP_KEY);
	gtk_box_pack_start(GTK_BOX(hbox), radio2, FALSE, FALSE, 0);

	gtk_widget_show_all(GTK_WIDGET(box));
}

static void
gnomecard_cancel(GtkWidget *widget, gpointer data)
{
	void *p;

	if ((p = gtk_object_get_user_data(GTK_OBJECT(widget))) != NULL)
		g_free(p);
	
	gtk_widget_destroy(widget);
}

static void
gnomecard_setup_apply(GtkWidget *widget, int page)
{
    ColumnType *hdrs, *p;
    ColumnHdrEditor *edit;
    GList *l, *cols;
    gint  ncol, i;

    if (page != -1)
	return;             /* ignore partial applies */
    
    
    edit = (ColumnHdrEditor *) gtk_object_get_user_data(GTK_OBJECT(widget));
    ncol = g_list_length(edit->selHdrs);

    if (ncol < 1) {
	/* force at least one column! */
	hdrs = g_new0(ColumnType, 2);
	hdrs[0] = COLTYPE_CARDNAME;
	hdrs[1] = COLTYPE_END;
    } else {
	hdrs = g_new0(ColumnType, ncol+1);
	for (p=hdrs, l=edit->selHdrs; l; l=l->next, p++)
	    *p = GPOINTER_TO_INT(l->data);
	*p = COLTYPE_END;
    }

    /* free old column headers */
    /* FIXME - need to do this through functions elsewhere */
    cols = gtk_object_get_data(GTK_OBJECT(gnomecard_list), "ColumnHeaders");
    if (cols)
	g_list_free(cols);

    gtk_widget_destroy(GTK_WIDGET(gnomecard_list));

    gnomecard_list = GTK_CLIST(gnomecardCreateCardListDisplay(hdrs));
    gtk_container_add(GTK_CONTAINER(cardlist_scrollwin), 
		      GTK_WIDGET(gnomecard_list));

    gnomecard_sort_card_list_by_default();
    gnomecard_rebuild_list();

    /* save settings via gnome_config */
    gnome_config_clean_section("/GnomeCard/CardDisplay");
    gnome_config_set_int("/GnomeCard/CardDisplay/ncols", ncol);
    for (i=0; i<ncol; i++) {
	gchar path[200];

	snprintf(path, sizeof(path), "/GnomeCard/CardDisplay/Column%0d", i);
	gnome_config_set_string(path, getColumnTypeNameFromType(hdrs[i]));
    }
    gnome_config_sync();
}


static void
colsetup_list_selected(GtkCList *list, gint row, gint column,
			GdkEventButton *event, gpointer data)
{
    gint coltype;
    ColumnHdrEditor *edit;

    edit = (ColumnHdrEditor *)data;

    coltype = GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(edit->srcList), row));

    g_message("User selected %s", getColumnNameFromType(coltype));

    if (coltype != COLTYPE_EMAIL &&
	coltype != COLTYPE_PHONE &&
	g_list_find(edit->selHdrs, GINT_TO_POINTER(coltype)))
	gtk_widget_set_sensitive(edit->addButton, 0);
    else
	gtk_widget_set_sensitive(edit->addButton, 1);
}


/* add selected row in src list to dest list */
static void
colsetup_addpress(GtkWidget *widget, gpointer data)
{
	ColumnHdrEditor *edit;
	gint coltype;
	
	edit = (ColumnHdrEditor *)data;
	if (GTK_CLIST(edit->srcList)->selection) {
		gint row, row2;
		const gchar *rowtxt[2];
		
		row = GPOINTER_TO_INT
		  (GTK_CLIST(edit->srcList)->selection->data);
		
		g_message("adding row # %d",row);
		
		coltype = GPOINTER_TO_INT
		  (gtk_clist_get_row_data(GTK_CLIST(edit->srcList), row));
		g_message("coltype is %d",coltype);
		
		/* add to end of the clist, selected list if not already selected */
		if (coltype == COLTYPE_EMAIL ||
		    coltype == COLTYPE_PHONE ||
		    !g_list_find(edit->selHdrs, GINT_TO_POINTER(coltype))) {
			edit->selHdrs = g_list_append
			  (edit->selHdrs,(GINT_TO_POINTER(coltype)));
			
			rowtxt[0] = getColumnNameFromType(coltype);
			rowtxt[1] = NULL;
			row2 = gtk_clist_append
			  (GTK_CLIST(edit->destList), rowtxt);
			gtk_clist_set_row_data(GTK_CLIST(edit->destList),
					       row2, GINT_TO_POINTER(coltype));
			
			/* disable add button */
			if (coltype != COLTYPE_EMAIL &&
			    coltype != COLTYPE_PHONE)
			  gtk_widget_set_sensitive(edit->addButton, 0);
			
			/* notify property box we've made a change */
			gnome_property_box_changed(edit->box);
		}
		
	}
}

static void
colsetup_delpress(GtkWidget *widget, gpointer data)
{
    ColumnHdrEditor *edit;
    gint row, coltype;
    gint row2, coltype2;

    edit = (ColumnHdrEditor *)data;
    if (GTK_CLIST(edit->destList)->selection) {

	row = GPOINTER_TO_INT(GTK_CLIST(edit->destList)->selection->data);
	coltype = GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(edit->destList), row));
	
	/* del from the clist */
	edit->selHdrs=g_list_remove(edit->selHdrs, GINT_TO_POINTER(coltype));
	gtk_clist_remove(GTK_CLIST(edit->destList), row);

	/* tell propertybox we have made a change */
	gnome_property_box_changed(edit->box);

	/* enable add button if required */
	if (GTK_CLIST(edit->srcList)->selection) {
	    
	    row2 = GPOINTER_TO_INT(GTK_CLIST(edit->srcList)->selection->data);
	    coltype2 = GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(edit->srcList), row2));

	    g_message("coltype coltype2 %d %d",coltype, coltype2);
	    if (coltype2 == coltype)
		gtk_widget_set_sensitive(edit->addButton, 1);
	}
    }
}

static void
colsetup_uppress(GtkWidget *widget, gpointer data)
{
    ColumnHdrEditor *edit;
    gint coltype;

    edit = (ColumnHdrEditor *)data;
    if (GTK_CLIST(edit->destList)->selection) {
	gint row;
	GList *l;
	GList *prevnode, *curnode;

	row = GPOINTER_TO_INT(GTK_CLIST(edit->destList)->selection->data);
	if (row == 0)
	    return;

	coltype = GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(edit->destList), row));

	/* move selected item up one */
	if ((curnode=g_list_find(edit->selHdrs, GINT_TO_POINTER(coltype)))) {
	    prevnode = curnode->prev;
	    /* split into cases so my head doesnt hurt */
	    if (prevnode) {
		if (prevnode->prev)
		    prevnode->prev->next = curnode;
		else
		    edit->selHdrs = curnode;
		if (curnode->next)
		    curnode->next->prev = prevnode;
		l = prevnode->prev;
		prevnode->prev = curnode;
		curnode->prev = l;
		l = curnode->next;
		curnode->next = prevnode;
		prevnode->next = l;
	    } else {
		/* do nothing */  
	    }

	    /* move rows around in clist too */
	    gtk_clist_swap_rows(GTK_CLIST(edit->destList), row, row-1);
	    gtk_clist_select_row(GTK_CLIST(edit->destList), row-1, 0);

	    /* notify property box we've made a change */
	    gnome_property_box_changed(edit->box);
	}
    }
}

static void
colsetup_dnpress(GtkWidget *widget, gpointer data)
{
    ColumnHdrEditor *edit;
    gint coltype;

    edit = (ColumnHdrEditor *)data;
    if (GTK_CLIST(edit->destList)->selection) {
	gint row;
	GList *l;
	GList *curnode, *nextnode;

	row = GPOINTER_TO_INT(GTK_CLIST(edit->destList)->selection->data);
	if (row == GTK_CLIST(edit->destList)->rows-1)
	    return;

	coltype = GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(edit->destList), row));

	/* move selected item up one */
	if ((curnode=g_list_find(edit->selHdrs, GINT_TO_POINTER(coltype)))) {
	    nextnode = curnode->next;
	    /* split into cases so my head doesnt hurt */
	    if (nextnode) {
		if (nextnode->next)
		    nextnode->next->prev = curnode;

		if (curnode->prev)
		    curnode->prev->next = nextnode;
		else
		    edit->selHdrs = nextnode;

		l = nextnode->next;
		nextnode->next = curnode;
		curnode->next = l;
		l = curnode->prev;
		curnode->prev = nextnode;
		nextnode->prev = l;
	    } else {
		/* do nothing */  
	    }

	    /* move rows around in clist too */
	    gtk_clist_swap_rows(GTK_CLIST(edit->destList), row, row+1);
	    gtk_clist_select_row(GTK_CLIST(edit->destList), row+1, 0);

	    /* notify property box we've made a change */
	    gnome_property_box_changed(edit->box);
	}
    }
}

void
gnomecard_setup(GtkWidget *widget, gpointer data)
{
        static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
	GtkWidget *hbox, *vbox, *vbox2, *frame;
	GtkWidget *label, *h, *v;

	GtkWidget *scrollwin;
	GtkWidget *delButton;
	GtkWidget *upButton;
	GtkWidget *dnButton;
	GtkWidget *align;
	GList     *l, *cols;
	gint      i;

	ColumnHdrEditor  *edit;
	ColumnHeader     *hdr;

	help_entry.name = gnome_app_id;

	edit = g_new0(ColumnHdrEditor, 1);
	edit->box = GNOME_PROPERTY_BOX(gnome_property_box_new());
	gtk_object_set_user_data(GTK_OBJECT(edit->box), edit);
	gtk_window_set_wmclass(GTK_WINDOW(edit->box), "gnomecard",
			       "gnomecard");
	gtk_signal_connect(GTK_OBJECT(edit->box), "apply",
			   (GtkSignalFunc)gnomecard_setup_apply, NULL);

	gtk_signal_connect(GTK_OBJECT (edit->box), "help",
			   GTK_SIGNAL_FUNC (gnome_help_pbox_display),
			   &help_entry);

	vbox = my_gtk_vbox_new();
	label = gtk_label_new(_("Layout"));
	gtk_notebook_append_page(GTK_NOTEBOOK(edit->box->notebook),
				 vbox, label);
	
	frame = gtk_frame_new(_("Column Display"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, FALSE, GNOME_PAD_SMALL);
	
	v = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), v);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(v), hbox, TRUE, FALSE, GNOME_PAD_SMALL);

	/* make list of possible columns to add */
	frame = gtk_frame_new(_("Possible Columns"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, GNOME_PAD_SMALL);
	h = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), h);
	v = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(h), v, TRUE, TRUE, GNOME_PAD_SMALL);
	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(v), scrollwin, TRUE, TRUE, GNOME_PAD_SMALL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	edit->srcList = gtk_clist_new(1);
	gtk_container_add(GTK_CONTAINER(scrollwin), edit->srcList);
	gtk_clist_set_selection_mode(GTK_CLIST(edit->srcList),
				     GTK_SELECTION_BROWSE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(edit->srcList), 0, TRUE);
	gtk_widget_set_usize (GTK_WIDGET(edit->srcList), 0, 180);

	/* add add/remove buttons in center */
	align = gtk_alignment_new(0.5, 0.5, 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), align, TRUE, FALSE, GNOME_PAD_SMALL);
	vbox2 = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(align), vbox2);
	
	edit->addButton = gnome_pixmap_button (gnome_stock_pixmap_widget 
	   (vbox2, GNOME_STOCK_PIXMAP_ADD), _("Add"));
	delButton = gnome_pixmap_button (gnome_stock_pixmap_widget 
	   (vbox2, GNOME_STOCK_PIXMAP_REMOVE), _("Remove"));
	gtk_box_pack_start(GTK_BOX(vbox2), 
			   edit->addButton, FALSE, FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox2), 
			   delButton, FALSE, FALSE, GNOME_PAD_SMALL);

	/* make list of currently displayed column types */
	frame = gtk_frame_new(_("Displayed Columns"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, GNOME_PAD_SMALL);
	h = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), h);
	v = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(h), v, TRUE, TRUE, GNOME_PAD_SMALL);
	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(v), scrollwin, TRUE, TRUE, GNOME_PAD_SMALL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	edit->destList = gtk_clist_new(1);
	gtk_container_add(GTK_CONTAINER(scrollwin), edit->destList);
	gtk_clist_set_selection_mode(GTK_CLIST(edit->destList),
				     GTK_SELECTION_BROWSE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(edit->destList), 0, TRUE);
	
	h = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(v), h, FALSE, FALSE, GNOME_PAD_SMALL);
	upButton = gtk_button_new ();
	gtk_container_add (GTK_CONTAINER (upButton),
			   gnome_stock_pixmap_widget
			   (vbox2, GNOME_STOCK_PIXMAP_UP));
	gtk_box_pack_start(GTK_BOX(h), upButton, FALSE, FALSE, GNOME_PAD_SMALL);
	dnButton = gtk_button_new ();
	gtk_container_add (GTK_CONTAINER (dnButton),
			   gnome_stock_pixmap_widget
			   (vbox2, GNOME_STOCK_PIXMAP_DOWN));
	gtk_box_pack_start(GTK_BOX(h), dnButton, FALSE, FALSE, GNOME_PAD_SMALL);

	
	/* fill in the two lists */
	edit->allHdrs = getAllColumnHdrs();
	for (l=edit->allHdrs, i=0; l; l=l->next, i++) {
	    const gchar *rowtxt[2];

	    rowtxt[0] = _(((ColumnHeader *)(l->data))->colname);
	    rowtxt[1] = NULL;

	    gtk_clist_append(GTK_CLIST(edit->srcList), rowtxt);
	    gtk_clist_set_row_data(GTK_CLIST(edit->srcList), i, 
				   GINT_TO_POINTER(((ColumnHeader *)
						    (l->data))->coltype));
	}

	/* prime with current headers */
	/* FIXME - need helper function to get this info */
	cols = gtk_object_get_data(GTK_OBJECT(gnomecard_list),
				   "ColumnHeaders");
	edit->selHdrs = NULL;
	for (l=cols; l; l=l->next) {
	    const gchar *rowtxt[2];
	    gint  row;

	    hdr = (ColumnHeader *)l->data;
	    edit->selHdrs = g_list_append(edit->selHdrs, 
					  GINT_TO_POINTER(hdr->coltype));

	    rowtxt[0] = getColumnNameFromType(hdr->coltype);
	    rowtxt[1] = NULL;
	    row=gtk_clist_append(GTK_CLIST(edit->destList), rowtxt);
	    gtk_clist_set_row_data(GTK_CLIST(edit->destList),
				   row, GINT_TO_POINTER(hdr->coltype));
	}

	gtk_signal_connect(GTK_OBJECT(edit->addButton), "clicked",
			   GTK_SIGNAL_FUNC(colsetup_addpress), edit);
	gtk_signal_connect(GTK_OBJECT(delButton), "clicked",
			   GTK_SIGNAL_FUNC(colsetup_delpress), edit);
	gtk_signal_connect(GTK_OBJECT(upButton), "clicked",
			   GTK_SIGNAL_FUNC(colsetup_uppress), edit);
	gtk_signal_connect(GTK_OBJECT(dnButton), "clicked",
			   GTK_SIGNAL_FUNC(colsetup_dnpress), edit);
	gtk_signal_connect(GTK_OBJECT(edit->srcList), "select_row",
			   GTK_SIGNAL_FUNC(colsetup_list_selected), edit);
	gtk_widget_show_all(GTK_WIDGET(edit->box));
}

extern void gnomecard_edit_card(GtkWidget *widget, gpointer data)
{
    if (gnomecard_curr_crd)
	gnomecard_edit(gnomecard_curr_crd);
}

extern gboolean gnomecard_append_file(char *fname)
{
	GList *crds;
	char *real_fname;
	char *app_title;
	char *fmt;
	
	if (! *fname)
	  real_fname = misc_tilde_expand ("~/.gnome/GnomeCard.gcrd");
	else
	  real_fname = fname;
	
	if (!(crds = card_load(gnomecard_crds, real_fname))) {
		if (! *fname)
		  g_free (real_fname);
		
		return FALSE;
	}
	
	if (! *fname) {
		g_free (real_fname);
		real_fname = _("Default");
	}
	
	fmt = _("GnomeCard: %s");
	app_title = g_malloc (strlen (fmt) + strlen (real_fname));
	sprintf (app_title, fmt, real_fname);
	gnomecard_set_app_title (app_title);
	g_free (app_title);
	
	gnomecard_crds = crds;
	gnomecard_sort_card_list_by_default();
	gnomecard_rebuild_list();
	gnomecard_scroll_list(gnomecard_crds);
	gnomecard_set_curr(gnomecard_crds);
	return TRUE;
}

static void gnomecard_send_error(char *fname, char *action)
{
	GtkWidget *w;
	char *message, *fmat;
	char *real_fname;
	
	if (! *fname)
	  real_fname = _("the default file");
	else
	  real_fname = fname;
	
	fmat = _("Error %s %s.");
	
	message = g_malloc (strlen (fmat) + strlen (action) + 
			    strlen (real_fname));
	sprintf (message, fmat, action, real_fname);
	w = gnome_message_box_new(message,
				  GNOME_MESSAGE_BOX_ERROR,
				  GNOME_STOCK_BUTTON_OK, NULL);
	GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
	gtk_widget_show(w);
	g_free (message);
}

static void gnomecard_send_read_error (char *fname)
{
	gnomecard_send_error (fname, "reading");
}

static void gnomecard_send_write_error (char *fname)
{
	gnomecard_send_error (fname, "writing");
}

static void gnomecard_append_call(GtkWidget *widget, gpointer data)
{
	char *fname;
	
	fname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget));
	
	if (gnomecard_append_file(fname)) {
			gnomecard_set_changed(TRUE);
			gtk_widget_destroy(widget);
	} else
	  gnomecard_send_read_error(fname);
}

extern void gnomecard_append(GtkWidget *widget, gpointer data)
{
	GtkWidget *fsel;
	
	fsel = gtk_file_selection_new(_("Append GnomeCard File"));
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(fsel));
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fsel)->ok_button),
			   "clicked", GTK_SIGNAL_FUNC(gnomecard_append_call),
			   GTK_OBJECT(fsel));
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fsel)->cancel_button),
			   "clicked", GTK_SIGNAL_FUNC(gnomecard_cancel),
			   GTK_OBJECT(fsel));
	gtk_widget_show(fsel);
}

extern gboolean gnomecard_open_file(char *fname)
{
	if (! gnomecard_destroy_cards())
	  return FALSE;
	
	if (gnomecard_append_file(fname))
	  return TRUE;
	
	gnomecard_send_read_error(fname);
	return FALSE;
}

static void gnomecard_open_call(GtkWidget *widget, gpointer data)
{
	char *fname;
	
	fname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget));
	
	if (gnomecard_open_file(fname)) {
		g_free(gnomecard_fname);
		gnomecard_fname = g_strdup(fname);
		gtk_widget_destroy(widget);
		gnome_config_set_string("/GnomeCard/file/open",  gnomecard_fname);
		gnome_config_sync();
	}
}

extern void gnomecard_open(GtkWidget *widget, gpointer data)
{
	GtkWidget *fsel;
	
	fsel = gtk_file_selection_new(_("Open GnomeCard File"));
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(fsel));
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fsel)->ok_button),
			   "clicked", GTK_SIGNAL_FUNC(gnomecard_open_call),
			   GTK_OBJECT(fsel));
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fsel)->cancel_button),
			   "clicked", GTK_SIGNAL_FUNC(gnomecard_cancel),
			   GTK_OBJECT(fsel));
	gtk_widget_show(fsel);
}

extern void gnomecard_open_default (GtkWidget *widget, gpointer data)
{
	if (gnomecard_open_file("")) {
		g_free(gnomecard_fname);
		gnomecard_fname = g_strdup("");
		gnome_config_set_string("/GnomeCard/file/open",  gnomecard_fname);
		gnome_config_sync();
	}
}

static void gnomecard_save_call(GtkWidget *widget, gpointer data)
{
	g_free(gnomecard_fname);
	gnomecard_fname = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget)));
	gtk_widget_destroy(widget);
	
	if (gnomecard_save() == -1)
	  gnomecard_send_write_error (gnomecard_fname);
}

extern void gnomecard_save_as(GtkWidget *widget, gpointer data)
{
	GtkWidget *fsel;
	
	fsel = gtk_file_selection_new(_("Save GnomeCard File As..."));
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(fsel));
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fsel)->ok_button),
			   "clicked", GTK_SIGNAL_FUNC(gnomecard_save_call),
			   GTK_OBJECT(fsel));
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fsel)->cancel_button),
			   "clicked", GTK_SIGNAL_FUNC(gnomecard_cancel),
			   GTK_OBJECT(fsel));
	gtk_widget_show(fsel);
}

extern void gnomecard_about(GtkWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors[] = { "arturo@nuclecu.unam.mx", 
				   "drmike@redhat.com",
				   NULL };
	
	about = gnome_about_new (_("GnomeCard"), VERSION,
				 "(C) 1997-2000 the Free Software Foundation",
				 authors,
				 _("Electronic Business Card Manager"),
				 NULL);
	gtk_widget_show (about);
}

static int gnomecard_match_pattern(char *pattern, char *str, int sens)
{
	char *txt;
	int found = 0;
	
	if (! str)
		return 0;
	
	if (sens)
		txt = str;
	else
		g_strup(txt = g_strdup(str));
	
	if (fnmatch(pattern, txt, 0) == 0) {
		found = 1;
	}
	
	if (! sens)
		g_free(txt);
	
	return found;
}

static void gnomecard_find_card(GtkWidget *w, gpointer data)
{
	GnomeCardFind *p;
	GList *l, *k;
	Card *crd;
	char *pattern, *crd_text[16];
	int i, wrapped, found, sens, back;
	
	p = (GnomeCardFind *) data;

	found = 0;
	wrapped = 0;

	if (GTK_TOGGLE_BUTTON(p->back)->active)
		gnomecard_find_back = back = 1;
	else
		gnomecard_find_back = back = 0;
	
	MY_FREE(gnomecard_find_str);
	gnomecard_find_str = g_strdup(gtk_entry_get_text(GTK_ENTRY(p->entry)));
	pattern = g_malloc(strlen(gnomecard_find_str) + 3);
	sprintf(pattern, "*%s*", gnomecard_find_str);

	if (GTK_TOGGLE_BUTTON(p->sens)->active)
		gnomecard_find_sens = sens = 1;
	else {
		gnomecard_find_sens = sens = 0;
		g_strup(pattern);
	}
	
	l = gnomecard_curr_crd;
	
	while (l) {
	    if (wrapped != 1)
		l = (back)? l->prev : l->next;
	    else
		wrapped ++;
	    
	    if (l) {
		crd = l->data;
		crd_text[0] = crd->fname.str;
		crd_text[1] = crd->name.family;
		crd_text[2] = crd->name.given;
		crd_text[3] = crd->name.additional;
		crd_text[4] = crd->name.prefix;
		crd_text[5] = crd->name.suffix;
		crd_text[6] = crd->title.str;
		crd_text[7] = crd->role.str;
		crd_text[8] = crd->categories.str;
		crd_text[9] = crd->comment.str;
		crd_text[10] = crd->url.str;
		crd_text[11] = crd->org.name;
		crd_text[12] = crd->org.unit1;
		crd_text[13] = crd->org.unit2;
		crd_text[14] = crd->org.unit3;
		crd_text[15] = crd->org.unit4;
		
		for (i = 0; i < sizeof(crd_text)/sizeof(crd_text[0]) && !found; i++)
		    if (gnomecard_match_pattern(pattern, crd_text[i], sens))
			found = 1;
		
		for (k = crd->phone.l; k && !found; k = k->next)
		    if (gnomecard_match_pattern(pattern, 
						((CardPhone *) k->data)->data,
						sens))
			found = 1;
		
		for (k = crd->email.l; k && !found; k = k->next)
		    if (gnomecard_match_pattern(pattern, 
						((CardEMail *) k->data)->data,
						sens))
			found = 1;
		
		for (k = crd->dellabel.l; k && !found; k = k->next)
		    if (gnomecard_match_pattern(pattern, 
						((CardDelLabel *)k->data)->data, sens))
			found = 1;

		for (k = crd->deladdr.l; k && !found; k = k->next)
		    for (i = 0; i < DELADDR_MAX; i++)
			if (gnomecard_match_pattern(pattern, 
						    ((CardDelAddr *) k->data)->data[i], sens))
			    found = 1;
		
		if (found) {
		    gnomecard_scroll_list(l);
		    break;
		}
		
	    }	else if (wrapped) {
		GtkWidget *w;
		
		w = gnome_message_box_new(_("No matching record found."),
					  GNOME_MESSAGE_BOX_ERROR,
					  GNOME_STOCK_BUTTON_OK, NULL);
		GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
		gnome_dialog_button_connect_object(GNOME_DIALOG(w),	0,
						   GTK_SIGNAL_FUNC(gnomecard_cancel),
						   GTK_OBJECT(w));	
		gtk_widget_show(w);
		break;
	    } else {
		GtkWidget *w;
		char msg[128], *str1, *str2;
		
		str1 = (back)? _("first") : _("last");
		str2 = (back)? _("last") : _("first");
		snprintf(msg, 128, _("Reached %s record.\nContinue from the %s one?"),
			 str1, str2);
		w = gnome_message_box_new(msg,
					  GNOME_MESSAGE_BOX_QUESTION,
					  GNOME_STOCK_BUTTON_OK,
					  GNOME_STOCK_BUTTON_CANCEL, NULL);
		GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
		gtk_widget_show(w);
		
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
		switch(gnome_dialog_run(GNOME_DIALOG(w))) {
		  case -1:
		  case 1:
		    l = NULL;
		    break;
		  case 0:
		    l = (back)? g_list_last(gnomecard_crds) : gnomecard_crds;
		    wrapped = 1;
		}
	    }
	}
	
	g_free(pattern);
	
	gnome_config_set_bool("/GnomeCard/find/sens",  gnomecard_find_sens);
	gnome_config_set_bool("/GnomeCard/find/back",  gnomecard_find_back);
	gnome_config_set_string("/GnomeCard/find/str",  gnomecard_find_str);
	gnome_config_sync();
}

extern void gnomecard_find_card_call(GtkWidget *widget, gpointer data)
{
	GtkWidget *w, *hbox, *check;
	GnomeCardFind *p;

	p = g_malloc(sizeof(GnomeCardFind));
	w = gnome_dialog_new(_("Find Card"), _("Find"),
			     GNOME_STOCK_BUTTON_CLOSE, NULL);
	gtk_object_set_user_data(GTK_OBJECT(w), p);
	gnome_dialog_button_connect(GNOME_DIALOG(w), 0,
				    GTK_SIGNAL_FUNC(gnomecard_find_card), p);
	gnome_dialog_button_connect_object(GNOME_DIALOG(w),	1,
					   GTK_SIGNAL_FUNC(gnomecard_cancel),
					   GTK_OBJECT(w));	

	p->entry = my_hbox_entry(GNOME_DIALOG(w)->vbox, _("Find:"), gnomecard_find_str);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(w)->vbox), hbox, FALSE, FALSE, 0);
	p->sens = check = gtk_check_button_new_with_label(_("Case Sensitive"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), gnomecard_find_sens);
	gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
	p->back = check = gtk_check_button_new_with_label(_("Find Backwards"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), gnomecard_find_back);
	gtk_box_pack_end(GTK_BOX(hbox), check, FALSE, FALSE, 0);
	
	gtk_widget_show_all(GNOME_DIALOG(w)->vbox);
	gtk_widget_show(w);
}

