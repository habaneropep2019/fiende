/* GnomeCard - a graphical contact manager.
 *
 * gnomecard.c: This file is part of GnomeCard.
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
#include <libgnorba/gnorba.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgnomeui/gnome-window-icon.h>

#include <glib.h>
#include <stdio.h>
#include <orb/orbit.h>

#include "../libversit/vcc.h"
#include "card.h"
#include "canvas.h"
#include "del.h"
#include "dialog.h"
#include "gnomecard.h"
#include "init.h"
#include "my.h"
#include "sort.h"
#include "list.h"
#include "misc.h"

#include "columnhdrs.h"

extern CORBA_Object
impl_gnome_PIM_vCard_server__create(PortableServer_POA poa, 
                                   CORBA_Environment * ev);

#define NAME_COL_WIDTH 100
#define ORG_COL_WIDTH 100
#define PHONE_COL_WIDTH 100
#define EMAIL_COL_WIDTH 100

#define NONE  0
#define FNAME 1

#define IDENT 0
#define GEO 1
#define ORG 2
#define EXP 3

GnomeApp *gnomecard_window;

GtkCList *gnomecard_list=NULL;
GtkWidget *cardlist_scrollwin = NULL;

gint     gnomecard_selected_row=0;
GtkWidget *tb_next, *tb_prev, *tb_first, *tb_last;
GtkWidget *tb_edit, *tb_find, *tb_save, *tb_del;
GtkWidget *menu_next, *menu_prev, *menu_first, *menu_last;
GtkWidget *menu_edit, *menu_find, *menu_save, *menu_del;

GList *gnomecard_crds=NULL;
GList *gnomecard_curr_crd=NULL;

char *gnomecard_fname;
char *gnomecard_find_str;
gboolean gnomecard_find_sens;
gboolean gnomecard_find_back;
/* NOTE USED gint gnomecard_def_data; */

gboolean gnomecard_changed;
gboolean gnomecard_found;                 /* yeah... pretty messy. (fixme) */


static void gnomecard_toggle_card_view(GtkWidget *w, gpointer data);
static void gnomecard_set_next(gboolean state);
static void gnomecard_set_prev(gboolean state);
static gboolean gnomecard_cards_blocked(void);
static void gnomecard_new_card(GtkWidget *widget, gpointer data);
static void list_button_press(GtkWidget * widget, GdkEventButton *event);
void Exception (CORBA_Environment * ev);

void
Exception (CORBA_Environment * ev)
{
  switch (ev->_major)
    {
    case CORBA_SYSTEM_EXCEPTION:
      g_log ("Gnome Card Server", G_LOG_LEVEL_DEBUG, "CORBA system exception %s.\n",
	     CORBA_exception_id (ev));
      exit (1);
    case CORBA_USER_EXCEPTION:
      g_log ("Gnome Card Server", G_LOG_LEVEL_DEBUG, "CORBA user exception: %s.\n",
	     CORBA_exception_id (ev));
      exit (1);
    default:
      break;
    }
}

gchar
*gnomecard_join_name (char *pre, char *given, char *add, char *fam, char *suf)
{
    char *name;
    
    name = g_malloc(MY_STRLEN(given) + MY_STRLEN(add) + MY_STRLEN(fam) +
		    MY_STRLEN(pre) + MY_STRLEN(suf) + 5);
    
    *name = 0;
    if (pre && *pre) { strcpy(name, pre);   strcat(name, " "); }
    if (given && *given) { strcat(name, given); strcat(name, " "); }
    if (add && *add) { strcat(name, add);   strcat(name, " "); }
    if (fam && *fam) { strcat(name, fam);   strcat(name, " "); }
    if (suf && *suf)     
	strcat(name, suf);
    else
	if (*name)
	    name[strlen(name) - 1] = 0;
    
    return name;
}

void gnomecard_set_app_title (char *title) 
{
	gtk_window_set_title (GTK_WINDOW (gnomecard_window), title);
}

void
gnomecard_set_changed(gboolean val)
{
    gnomecard_changed = val;
    gtk_widget_set_sensitive(tb_save, val);
    gtk_widget_set_sensitive(menu_save, val);
}

static void
gnomecard_toggle_card_view(GtkWidget *w, gpointer data)
{
    if (GTK_CHECK_MENU_ITEM(w)->active)
	gtk_widget_hide(gnomecard_canvas);
    else
	gtk_widget_show(gnomecard_canvas);
}

static void
gnomecard_set_next(gboolean state)
{
    gtk_widget_set_sensitive(tb_next, state);
    gtk_widget_set_sensitive(menu_next, state);
    gtk_widget_set_sensitive(tb_last, state);
    gtk_widget_set_sensitive(menu_last, state);
}

static void
gnomecard_set_prev(gboolean state)
{
    gtk_widget_set_sensitive(tb_prev, state);
    gtk_widget_set_sensitive(menu_prev, state);
    gtk_widget_set_sensitive(tb_first, state);
    gtk_widget_set_sensitive(menu_first, state);
}

static void
list_button_press(GtkWidget * widget, GdkEventButton *event)
{
}


/* NOT USED
extern void gnomecard_set_add(gboolean state)
{
	int i;
	
	for (i = 2; add_menu[i].type != GNOME_APP_UI_ENDOFINFO; i++)
		if (add_menu[i].type == GNOME_APP_UI_ITEM)
			gtk_widget_set_sensitive(GTK_WIDGET(add_menu[i].widget), state);
}
*/

void
gnomecard_set_edit_del(gboolean state)
{
    gtk_widget_set_sensitive(tb_edit, state);
    gtk_widget_set_sensitive(menu_edit, state);
    gtk_widget_set_sensitive(tb_del, state);
    gtk_widget_set_sensitive(menu_del, state);
    gtk_widget_set_sensitive(tb_find, state);
    gtk_widget_set_sensitive(menu_find, state);
}

void
gnomecard_set_curr(GList *node)
{
    gnomecard_curr_crd = node;
    
    if (gnomecard_curr_crd) {
	gnomecard_update_canvas(gnomecard_curr_crd->data);
	
	if (!((Card *) gnomecard_curr_crd->data)->flag) {
	    gnomecard_set_edit_del(TRUE);
	    /*gnomecard_set_add(TRUE);*/
	} else { 
	    gnomecard_set_edit_del(FALSE);
	}
	
	if (gnomecard_curr_crd->next)
	    gnomecard_set_next(TRUE);
	else
	    gnomecard_set_next(FALSE);
	
	if (gnomecard_curr_crd->prev)
	    gnomecard_set_prev(TRUE);
	else
	    gnomecard_set_prev(FALSE);
	
    } else {
	gnomecard_clear_canvas();
	
	gnomecard_set_edit_del(FALSE);
	/*gnomecard_set_add(FALSE);*/
	
	gnomecard_set_next(FALSE);
	gnomecard_set_prev(FALSE);
    }
}

extern gboolean gnomecard_save(void)
{
	GList *l;
	FILE *fp;
	char *bak_fname, *bak_fname2, *cmd, *real_fname;
	struct stat s;
	char i;
	
	if (! *gnomecard_fname)
	  real_fname = misc_tilde_expand ("~/.gnome/GnomeCard.gcrd");
	else
	  real_fname = gnomecard_fname;
	
	bak_fname = g_malloc (strlen (real_fname) + 3);
	
	for (i = '0'; i <= '9'; i++) {
		
		bak_fname2 = g_strdup (bak_fname);
		sprintf (bak_fname, "%s~%c", real_fname, i);
		
		if (stat (bak_fname, &s) == -1 && errno == ENOENT) {
			g_free (bak_fname2);
			break;
		}
		
		if (i != '0') {
			cmd = g_malloc (strlen (bak_fname) + 
					strlen (bak_fname2) + 8);
		
			sprintf (cmd, "mv -f %s %s", bak_fname, bak_fname2);
			
			system (cmd);
			g_free (cmd);
			g_free (bak_fname2);
		}
	}
	
	cmd = g_malloc (strlen (bak_fname) + strlen (real_fname) + 8);
	sprintf (cmd, "cp -f %s %s", real_fname, bak_fname);
	system (cmd);
	
	g_free (cmd);
	g_free (bak_fname);

	fp = fopen (real_fname, "w");
	if (! fp) {
		if (! *gnomecard_fname)
		  g_free (real_fname);
		
		return FALSE;
	}
	
	for (l = gnomecard_crds; l; l = l->next)
	  card_save((Card *) l->data, fp);
	fclose(fp);
	
	gnomecard_set_changed(FALSE);
	
	if (! *gnomecard_fname)
	  g_free (real_fname);
		
	return TRUE;
}

static gboolean
gnomecard_cards_blocked(void)
{
	GList *l;

	for (l = gnomecard_crds; l; l = l->next)
	  if (((Card *) l->data)->flag)
	    return TRUE;
	
	return FALSE;
}

/* Returns TRUE if the cards were destroyed. FALSE if canceled */
extern int gnomecard_destroy_cards(void)
{
    GList *l;
    
    if (gnomecard_cards_blocked()) {
	GtkWidget *w;
	
	w = gnome_message_box_new(_("There are cards which are currently being modified.\nFinish any pending modifications and try again."),
				  GNOME_MESSAGE_BOX_ERROR,
				  GNOME_STOCK_BUTTON_OK, NULL);
	GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
	gtk_widget_show(w);
	
	return FALSE;
    }
    
    if (gnomecard_changed) {
	GtkWidget *w;
	char *msg;
	char *real_fname;
	
	if (! *gnomecard_fname)
	  real_fname = _("The default file");
	else
	  real_fname = gnomecard_fname;
	    
	msg= g_strconcat (real_fname, _(" changed. Save?"), NULL);
	
	w = gnome_message_box_new(msg,
				  GNOME_MESSAGE_BOX_QUESTION,
				  GNOME_STOCK_BUTTON_YES,
				  GNOME_STOCK_BUTTON_NO,
				  GNOME_STOCK_BUTTON_CANCEL, NULL);
	GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
	gtk_widget_show(w);
	
	gtk_window_set_modal(GTK_WINDOW(w),TRUE);
	switch(gnome_dialog_run(GNOME_DIALOG(w))) {
	  case -1:
	  case 2:
	    return FALSE;
	  case 1:
	    break;
	  case 0:
	    gnomecard_save();
	}
	
	g_free (msg);
    }
    
    for (l = gnomecard_crds; l; l = l->next)
	card_free (l->data);
    
    gnomecardClearCardListDisplay(GTK_WIDGET(gnomecard_list));
    g_list_free(gnomecard_crds);
    gnomecard_crds = NULL;
    
    gnomecard_set_curr(NULL);
    gnomecard_set_changed(FALSE);
    
    return TRUE;
}

static void
gnomecard_new_card(GtkWidget *widget, gpointer data)
{
	Card *crd;
	GList *last;
	
	crd = card_new();
	gnomecard_add_card_to_list(crd);
	gnomecard_crds = g_list_append(gnomecard_crds, crd);
	
	last = g_list_last(gnomecard_crds);
	gnomecard_edit(last);
	gnomecard_scroll_list(last);
	gnomecard_set_changed(TRUE);
}

void gnomecard_first_card(GtkWidget *widget, gpointer data)
{
	gnomecard_scroll_list(g_list_first(gnomecard_crds));
}

void gnomecard_prev_card(GtkWidget *widget, gpointer data)
{
	if (gnomecard_curr_crd->prev)
	  gnomecard_scroll_list(gnomecard_curr_crd->prev);
}

void gnomecard_next_card(GtkWidget *widget, gpointer data)
{
	if (gnomecard_curr_crd->next)
	  gnomecard_scroll_list(gnomecard_curr_crd->next);
}

void gnomecard_last_card(GtkWidget *widget, gpointer data)
{
	gnomecard_scroll_list(g_list_last(gnomecard_crds));
}

void gnomecard_quit(GtkWidget *widget, gpointer data)
{
	if (gnomecard_destroy_cards ()) {
		int x, y;
	
		x = GTK_WIDGET (gnomecard_window)->allocation.width;
		y = GTK_WIDGET (gnomecard_window)->allocation.height;
		gnome_config_set_int("/GnomeCard/CardDisplay/width", x);
		gnome_config_set_int("/GnomeCard/CardDisplay/height", y);
		gnome_config_sync ();
		
		gtk_widget_destroy(GTK_WIDGET (gnomecard_window));
		gtk_main_quit ();
	}
}

gint gnomecard_delete(GtkWidget *w, GdkEvent *e, gpointer data)
{
	if (gnomecard_destroy_cards()) {
		gtk_widget_destroy(GTK_WIDGET (gnomecard_window));
		gtk_main_quit();
	}
	  
	return TRUE;
}

void gnomecard_spawn_new(GtkWidget *widget, gpointer data)
{
	GtkWidget *w;
	int pid;
	char *text[] = { "gnomecard", NULL };
	
	pid = fork();
	if (pid == 0) { /* child */
/* gnomecard: error in loading shared libraries
 * /gnome/lib/libgnome.so.0: undefined symbol: stat */
		if (execvp("gnomecard", text) == -1) { 
			w = gnome_message_box_new(_("A new Gnomecard could not be spawned. Maybe it is not in your path."),
																GNOME_MESSAGE_BOX_ERROR,
																GNOME_STOCK_BUTTON_OK, NULL);
			GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
			gtk_widget_show(w);
		}
		exit (1);
	}
}

GnomeUIInfo filemenu[] = {

	GNOMEUIINFO_MENU_NEW_ITEM(N_("New"), N_("Create a new card file"),
				  gnomecard_destroy_cards, NULL),

	GNOMEUIINFO_MENU_OPEN_ITEM(gnomecard_open, NULL),

	{GNOME_APP_UI_ITEM, N_("Open _Default"),
	 N_("Open the default file"),
	 gnomecard_open_default, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN, 0, 0, NULL},
	
	GNOMEUIINFO_MENU_SAVE_ITEM(gnomecard_save, NULL),

	GNOMEUIINFO_MENU_SAVE_AS_ITEM(gnomecard_save_as, NULL),

	{GNOME_APP_UI_ITEM, N_("_Append..."),
	 N_("Add the contents of another card file"),
	 gnomecard_append, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN, 0, 0, NULL},

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_EXIT_ITEM(gnomecard_quit, NULL),

	GNOMEUIINFO_END
};

GnomeUIInfo gomenu[] = {
	{GNOME_APP_UI_ITEM, N_("First"), N_("First card"),
	 gnomecard_first_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardFirstMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Prev"), N_("Previous card"),
	 gnomecard_prev_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Next"), N_("Next card"),
	 gnomecard_next_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Last"), N_("Last card"),
	 gnomecard_last_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardLastMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};
	
GnomeUIInfo sortradios[] = {
	
	{GNOME_APP_UI_ITEM, N_("By Card Name"), "",
	 gnomecard_sort_cards, (gpointer) COLTYPE_CARDNAME, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("By Name"), "",
	 gnomecard_sort_cards, (gpointer) COLTYPE_FULLNAME, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("By Last Name"), "",
	 gnomecard_sort_cards, (gpointer) COLTYPE_LASTNAME, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("By E-mail"), "",
	 gnomecard_sort_cards, (gpointer) COLTYPE_EMAIL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("By Organization"), "",
	 gnomecard_sort_cards, (gpointer) COLTYPE_ORG, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo sortmenu[] = {
	
	GNOMEUIINFO_RADIOLIST(sortradios),

	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo editmenu[] = {
	{GNOME_APP_UI_ITEM, N_("Add"), N_("Create new card"),
	 gnomecard_new_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardNewMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Modify"), N_("Edit card"),
	 gnomecard_edit_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardEditMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Delete"), N_("Erase card"),
	 gnomecard_delete_current_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_TRASH, 0, 0, NULL},
	
	{GNOME_APP_UI_SEPARATOR},
	
	{GNOME_APP_UI_SUBTREE, N_("Go"), N_("Change current card"),
	 gomenu, NULL, NULL, GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_SEPARATOR},
	
	{GNOME_APP_UI_ITEM, N_("Find"), N_("Search card"),
	 gnomecard_find_card_call, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardFindMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_SUBTREE, N_("Sort"), N_("Set sort criteria"),
	 sortmenu, NULL, NULL, GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

/* NOT USED
GnomeUIInfo addmenu[] = {
	{GNOME_APP_UI_ITEM, N_("Card"), N_("Create new card"),
	 gnomecard_new_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardNewMenu", 0, 0, NULL},

	{GNOME_APP_UI_SEPARATOR},
	
	{GNOME_APP_UI_ITEM, N_("E-mail"), N_("Add Electronic Address"),
	 gnomecard_add_email_call, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardEMailMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Phone"), N_("Add Telephone Number"),
	 gnomecard_add_phone_call, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardPhoneMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Address"), N_("Add Delivery Address"),
	 gnomecard_add_deladdr_call, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardAddrMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Address Label"), N_("Add Delivery Address Label"),
	 gnomecard_add_dellabel_call, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardAddrMenu", 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};
*/
GnomeUIInfo viewmenu[] = {
	{GNOME_APP_UI_TOGGLEITEM, N_("Card"), N_("Toggle Card View"),
	 gnomecard_toggle_card_view, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

#ifdef B4MSF	
	{GNOME_APP_UI_TOGGLEITEM, N_("Tree"), N_("Toggle Tree View"),
	 gnomecard_toggle_tree_view, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
#endif
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo settingsmenu[] = {
		GNOMEUIINFO_MENU_PREFERENCES_ITEM(gnomecard_setup, NULL),
		GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
		GNOMEUIINFO_HELP("gnomecard"),
		GNOMEUIINFO_MENU_ABOUT_ITEM(gnomecard_about, NULL),
		GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
        GNOMEUIINFO_MENU_FILE_TREE(filemenu),

	GNOMEUIINFO_MENU_EDIT_TREE(editmenu),

	GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),

	GNOMEUIINFO_MENU_HELP_TREE(helpmenu),

/*	{GNOME_APP_UI_SUBTREE, N_("Add"), NULL, addmenu, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
*/
/*	{GNOME_APP_UI_SUBTREE, N_("View"), NULL, viewmenu, NULL, NULL,
	GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},*/

	GNOMEUIINFO_END
};

GnomeUIInfo toolbar[] = {
	{GNOME_APP_UI_ITEM, N_("Open"), N_("Open file"), 
		gnomecard_open, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_OPEN, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("Save"), N_("Save changes"), 
		gnomecard_save, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_SAVE, 0, 0, NULL},

	{GNOME_APP_UI_SEPARATOR},
	
	{GNOME_APP_UI_ITEM, N_("Add"), N_("Create new card"),
	 gnomecard_new_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardNew", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Modify"), N_("Edit card"),
	 gnomecard_edit_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardEdit", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Del"), N_("Delete card"),
	 gnomecard_delete_current_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_TRASH, 0, 0, NULL},
	
	{GNOME_APP_UI_SEPARATOR},
	
	{GNOME_APP_UI_ITEM, N_("First"), N_("First card"),
	 gnomecard_first_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardFirst", 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Prev"), N_("Previous card"),
	 gnomecard_prev_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_BACK, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Next"), N_("Next card"),
	 gnomecard_next_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_FORWARD, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("Last"), N_("Last card"),
	 gnomecard_last_card, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardLast", 0, 0, NULL},
	
	{GNOME_APP_UI_SEPARATOR},
	
	{GNOME_APP_UI_ITEM, N_("Find"), N_("Search card"),
	 gnomecard_find_card_call, NULL, NULL,
	GNOME_APP_PIXMAP_STOCK, "GnomeCardFind", 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

void gnomecard_sort_by_fname(GtkWidget *w, gpointer data)
{
	gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(sortradios[0].widget), 
				      TRUE);
}


void gnomecard_init(void)
{
	GtkWidget *scrollwin;
        GnomeAppBar *appbar;
	GtkWidget *canvas, *hbox;
	gchar *s;
	gint ncol, i, x, y;

	/* hard coded column headers */
	ColumnType defaulthdrs[] = {COLTYPE_CARDNAME, COLTYPE_EMAIL, 
				    COLTYPE_ORG, COLTYPE_END};
	ColumnType *hdrs, *p;

	gnomecard_init_stock();
	gnomecard_init_pixes();
	
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gnomecard.png");
	gnomecard_window = GNOME_APP(gnome_app_new("GnomeCard", 
						   "GnomeCard: Default"));
	x = gnome_config_get_int("/GnomeCard/CardDisplay/width=0");
	y = gnome_config_get_int("/GnomeCard/CardDisplay/height=0");
	gtk_widget_set_usize (GTK_WIDGET (gnomecard_window), x, y);

	appbar = GNOME_APPBAR(gnome_appbar_new(FALSE, TRUE,
					       GNOME_PREFERENCES_USER));
	gnome_app_set_statusbar(GNOME_APP(gnomecard_window),
				GTK_WIDGET(appbar));

	gtk_window_set_wmclass(GTK_WINDOW(gnomecard_window), "GnomeCard",
			       "GnomeCard");
	gtk_window_set_policy(GTK_WINDOW(gnomecard_window), TRUE, TRUE, FALSE);
			      
	gtk_signal_connect(GTK_OBJECT(gnomecard_window), "delete_event",
			   GTK_SIGNAL_FUNC(gnomecard_delete), NULL);
	
	hbox = gtk_hbox_new(FALSE, 0);
/*	gtk_paned_set_gutter_size (GTK_PANED (hpaned), GNOME_PAD_BIG);
	gtk_widget_show (hpaned);*/
	
	gnome_app_create_menus(GNOME_APP(gnomecard_window), mainmenu);

	gnome_app_install_menu_hints(GNOME_APP(gnomecard_window), mainmenu);
	gnome_app_create_toolbar(GNOME_APP(gnomecard_window), toolbar);

	gnome_app_set_contents(GNOME_APP(gnomecard_window), hbox);
/*	gnome_app_set_contents(GNOME_APP(gnomecard_window), hpaned);*/

	cardlist_scrollwin = scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
				   GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);

	/* card display layout */
	ncol=gnome_config_get_int("/GnomeCard/CardDisplay/ncols=0");
	s = gnome_config_get_string("/GnomeCard/CardDisplay/SortType=COLTYPE_CARDNAME");
	gnomecard_sort_col = getColumnTypeFromTypeName(s);
	if (gnomecard_sort_col == COLTYPE_END)
		gnomecard_sort_col = COLTYPE_CARDNAME;

	if (!ncol) {
	    gnomecard_list = GTK_CLIST(gnomecardCreateCardListDisplay(defaulthdrs));
	} else {
	    gchar path[200];

	    
	    hdrs = g_new0(ColumnType, ncol+1);
	    for (p=hdrs, i=0; i<ncol; i++) {
		snprintf(path, sizeof(path),
			 "/GnomeCard/CardDisplay/Column%0d=", i);
		s=gnome_config_get_string(path);
		
		if (s && *s) {
		    *p++ = getColumnTypeFromTypeName(s);
		}
	    }
	    *p = COLTYPE_END;
	    gnomecard_list = GTK_CLIST(gnomecardCreateCardListDisplay(hdrs));
	}

	gtk_container_add(GTK_CONTAINER(scrollwin), GTK_WIDGET(gnomecard_list));
	gtk_signal_connect(GTK_OBJECT(scrollwin),
			  "button_press_event",
			  GTK_SIGNAL_FUNC(list_button_press),
			  NULL);
/*	gtk_widget_set_usize (scrollwin, 200, 0);*/
	gtk_widget_show(scrollwin);
	
	gtk_box_pack_start(GTK_BOX(hbox), cardlist_scrollwin, TRUE, TRUE, 0);
/*	gtk_paned_pack1 (GTK_PANED (hpaned), scrollwin, TRUE, FALSE);*/

	/* add canvas */
	gtk_widget_push_visual(gdk_imlib_get_visual());
	gtk_widget_push_colormap(gdk_imlib_get_colormap());
	
	canvas = gnomecard_canvas_new();
	scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
				   GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(scrollwin), canvas);
	gtk_widget_show(canvas);
	gtk_widget_show(scrollwin);
	gtk_box_pack_start(GTK_BOX(hbox), scrollwin, TRUE, TRUE, 0);
/*	gtk_paned_pack2 (GTK_PANED (hpaned), scrollwin, TRUE, TRUE);*/

	/* no cards yet */
	gnomecard_crds = NULL;

	/* FIXME ? - ugly mechanism to be able to enable/disable buttons on */
	/*           toolbar                                                */
	tb_save = toolbar[1].widget;
	tb_edit = toolbar[4].widget;
	tb_del  = toolbar[5].widget;
	tb_first = toolbar[7].widget;
	tb_prev = toolbar[8].widget;
	tb_next = toolbar[9].widget;
	tb_last = toolbar[10].widget;
	tb_find = toolbar[12].widget;
	
	menu_save = filemenu[3].widget;
	menu_edit = editmenu[1].widget;
	menu_del  = editmenu[2].widget;
	menu_find = editmenu[6].widget;
	menu_first = gomenu[0].widget;
	menu_prev = gomenu[1].widget;
	menu_next = gomenu[2].widget;
	menu_last = gomenu[3].widget;
	
	gnomecard_init_defaults();
	gnomecard_set_changed(FALSE);
	
	if (! gnomecard_open_file(gnomecard_fname))
          gnomecard_set_curr(NULL);

	gtk_widget_show(GTK_WIDGET (gnomecard_window));
}

int main (int argc, char *argv[])
{
	CORBA_ORB orb;
	CORBA_Environment ev;
	CORBA_Object server;
	CORBA_Object name_server;
	PortableServer_POA poa;
	PortableServer_POAManager pm;

        bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	CORBA_exception_init(&ev);
	orb = gnome_CORBA_init("gnomecard", VERSION, &argc, argv, GNORBA_INIT_SERVER_FUNC, &ev);
	Exception (&ev);

	poa = CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
	Exception (&ev);

	server = impl_gnome_PIM_vCard_server__create(poa, &ev);
	Exception (&ev);

	pm = PortableServer_POA__get_the_POAManager (poa, &ev);
	Exception (&ev);

	PortableServer_POAManager_activate (pm, &ev);
	Exception (&ev);

	name_server = gnome_name_service_get ();
	goad_server_register (name_server,
                              server,
                              "gnomecard",
                              "object",
			      &ev);

	gnomecard_init();
	gtk_main();
	return 0;
}
