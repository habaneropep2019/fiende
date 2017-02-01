/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <gnome.h>
#include <libgnome/gnome-help.h>
#include <string.h>

#include "app.h"
#include "gtt.h"
#include "journal.h"
#include "menucmd.h"
#include "myoaf.h"
#include "prefs.h"
#include "timer.h"
#include "toolbar.h"


typedef struct _MyToggle MyToggle;
typedef struct _MyToolbar MyToolbar;

struct _MyToolbar {
	GtkToolbar *tbar;
	GtkTooltips *tt;
	GtkWidget *cut, *copy, *paste; /* to make them sensible
					  as needed */
	GtkWidget *journal_w;
	GtkWidget *prop_w;
	GtkWidget *timer_w;
	GnomeStock *timer;
	GtkWidget *calendar_w;
};

MyToolbar *mytbar = NULL;



static GtkWidget *
add_stock_button(GtkToolbar *tbar, const char *text, const char *tt_text,
		 const char *icon, GtkSignalFunc sigfunc)
{
	GtkWidget *w, *pixmap;

	pixmap = gnome_stock_pixmap_widget((GtkWidget *)window, icon);
	w = gtk_toolbar_append_item(tbar, text, tt_text, NULL, pixmap,
				    sigfunc, NULL);
	return w;
}


static GnomeStock *
add_toggle_button(GtkToolbar *tbar, char *text, char *tt_text,
		 char *icon, GtkSignalFunc sigfunc, GtkWidget **wptr)
{
	GtkWidget *w;

	w = gnome_stock_pixmap_widget((GtkWidget *)window, icon);
	(*wptr) = gtk_toolbar_append_item(tbar, text, tt_text, NULL, w,
					  sigfunc, NULL);
	return GNOME_STOCK(w);
}



void
toolbar_set_states(void)
{
	extern GttProject *cutted_project;
	GtkToolbarStyle tb_style;

	g_return_if_fail(mytbar != NULL);
	g_return_if_fail(mytbar->tbar != NULL);
	g_return_if_fail(GTK_IS_TOOLBAR(mytbar->tbar));

	if (mytbar->tt) {
		if (config_show_tb_tips)
			gtk_tooltips_enable(mytbar->tt);
		else
			gtk_tooltips_disable(mytbar->tt);
	}
#if 0
/* not done any more, use the focus project instead */
	if (mytbar->cut)
		gtk_widget_set_sensitive(mytbar->cut, (cur_proj != NULL));
	if (mytbar->copy)
		gtk_widget_set_sensitive(mytbar->copy, (cur_proj != NULL));
#endif
	if (mytbar->paste)
		gtk_widget_set_sensitive(mytbar->paste,
					 (cutted_project != NULL));
#if 0
/* not done any more, use the focus project instead */
	if (mytbar->prop_w)
		gtk_widget_set_sensitive(mytbar->prop_w, (cur_proj != NULL));
	if (mytbar->journal_w)
		gtk_widget_set_sensitive(mytbar->journal_w, (cur_proj != NULL));
#endif
	if (mytbar->timer)
		gnome_stock_set_icon(mytbar->timer,
				     (timer_is_running()) ?
				     GNOME_STOCK_PIXMAP_TIMER_STOP :
				     GNOME_STOCK_PIXMAP_TIMER);
#if 0
/* not done any more, use the focus project */
	if (mytbar->timer_w)
		gtk_widget_set_sensitive(GTK_WIDGET(mytbar->timer_w),
			(NULL != prev_proj) || (NULL != cur_proj));
#endif

	if ((config_show_tb_icons) && (config_show_tb_texts)) {
		tb_style = GTK_TOOLBAR_BOTH;
	} else if ((!config_show_tb_icons) && (config_show_tb_texts)) {
		tb_style = GTK_TOOLBAR_TEXT;
	} else {
		tb_style = GTK_TOOLBAR_ICONS;
	}
	gtk_toolbar_set_style(mytbar->tbar, tb_style);
}



static void
toolbar_help(GtkWidget *widget, gpointer data)
{

	GnomeHelpMenuEntry ref = {"gtt", "index.html"};
	gnome_help_display (NULL, &ref);
}



/* returns a pointer to the (still hidden) GtkToolbar */
GtkWidget *
build_toolbar(void)
{
	if (mytbar) return GTK_WIDGET(mytbar->tbar);
	mytbar = g_malloc0(sizeof(MyToolbar));
	mytbar->tbar = GTK_TOOLBAR(gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,
						   GTK_TOOLBAR_ICONS));
	mytbar->tt = mytbar->tbar->tooltips;

	if (config_show_tb_new) {
		/* Note to translators: just skip the `[GTT]' */
		add_stock_button(mytbar->tbar, gtt_gettext(_("[GTT]New")),
				 _("Create a New Project..."),
				 GNOME_STOCK_PIXMAP_NEW,
				 (GtkSignalFunc)new_project);
		gtk_toolbar_append_space(mytbar->tbar);
	}
	if (config_show_tb_file) {
		add_stock_button(mytbar->tbar, _("Reload"),
				 _("Reload Configuration File"),
				 GNOME_STOCK_PIXMAP_OPEN,
				 (GtkSignalFunc)init_project_list);
		add_stock_button(mytbar->tbar, _("Save"),
				 _("Save Configuration File"),
				 GNOME_STOCK_PIXMAP_SAVE,
				 (GtkSignalFunc)save_project_list);
		gtk_toolbar_append_space(mytbar->tbar);
	}
	if (config_show_tb_ccp) {
		mytbar->cut = add_stock_button(mytbar->tbar, _("Cut"),
					       _("Cut Selected Project"),
					       GNOME_STOCK_PIXMAP_CUT,
					       (GtkSignalFunc)cut_project);
		mytbar->copy = add_stock_button(mytbar->tbar, _("Copy"),
						_("Copy Selected Project"),
						GNOME_STOCK_PIXMAP_COPY,
						(GtkSignalFunc)copy_project);
		mytbar->paste = add_stock_button(mytbar->tbar, _("Paste"),
						 _("Paste Project"),
						 GNOME_STOCK_PIXMAP_PASTE,
						 (GtkSignalFunc)paste_project);
		gtk_toolbar_append_space(mytbar->tbar);
	}
	if (config_show_tb_journal) {
		mytbar->journal_w = add_stock_button(mytbar->tbar, 
				 _("Journal"),
				 _("View and Edit Timestamp Logs"),
				 GNOME_STOCK_PIXMAP_BOOK_OPEN,
				 (GtkSignalFunc)edit_journal);
	}
	if (config_show_tb_prop)
		mytbar->prop_w = add_stock_button(mytbar->tbar, _("Props"),
				  _("Edit Project Properties..."),
				  GNOME_STOCK_PIXMAP_PROPERTIES,
				  (GtkSignalFunc)menu_properties);
	if (config_show_tb_timer)
		mytbar->timer = add_toggle_button(mytbar->tbar, _("Timer"),
				  _("Start/Stop Timer"),
				  GNOME_STOCK_PIXMAP_TIMER,
				  (GtkSignalFunc)menu_toggle_timer,
				  &(mytbar->timer_w));
	if (config_show_tb_calendar) {
		mytbar->calendar_w = add_stock_button(mytbar->tbar, 
				 _("Calendar"),
				 _("View Calendar"),
				 GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST,
				 (GtkSignalFunc)edit_calendar);
	}
	if (((config_show_tb_timer)    || 
	     (config_show_tb_journal)  ||    
	     (config_show_tb_calendar)  ||    
	     (config_show_tb_prop)     ) &&
	    ((config_show_tb_pref) || 
	     (config_show_tb_help) ||
	     (config_show_tb_exit)))
		gtk_toolbar_append_space(mytbar->tbar);
	if (config_show_tb_pref)
		add_stock_button(mytbar->tbar, _("Prefs"),
				 _("Edit Preferences..."),
				 GNOME_STOCK_PIXMAP_PREFERENCES,
				 (GtkSignalFunc)menu_options);
	if (config_show_tb_help) {
		add_stock_button(mytbar->tbar, _("Manual"), 
				 _("User's Guide and Manual"),
				 GNOME_STOCK_PIXMAP_HELP,
				 (GtkSignalFunc)toolbar_help);
	}
	if (config_show_tb_exit) {
		add_stock_button(mytbar->tbar, _("Quit"), _("Quit GTimeTracker"),
				 GNOME_STOCK_PIXMAP_QUIT,
				 (GtkSignalFunc)quit_app);
	}

	if (gnome_preferences_get_toolbar_lines()) {
		gtk_toolbar_set_space_style(GTK_TOOLBAR(mytbar->tbar),
					    GTK_TOOLBAR_SPACE_LINE);
		gtk_toolbar_set_space_size(GTK_TOOLBAR(mytbar->tbar),
					   GNOME_PAD * 2);
	} else
		gtk_toolbar_set_space_size(GTK_TOOLBAR(mytbar->tbar),
					   GNOME_PAD);

	if (!gnome_preferences_get_toolbar_relief_btn())
		gtk_toolbar_set_button_relief(GTK_TOOLBAR(mytbar->tbar),
					      GTK_RELIEF_NONE);

	/* gtt handles icons/labels style on its own
	if (!gnome_preferences_get_toolbar_labels())
		gtk_toolbar_set_style(GTK_TOOLBAR(mytbar->tbar),
				      GTK_TOOLBAR_ICONS);
	*/

	return GTK_WIDGET(mytbar->tbar);
}



/* TODO: I have to completely rebuild the toolbar, when I want to add or
   remove items. There should be a better way now */
void
update_toolbar_sections(void)
{
	GtkWidget *tb;
	GtkWidget *w;

	if (!window) return;
	if (!mytbar) return;

	w = GTK_WIDGET(mytbar->tbar)->parent;
	if (w) {
		gtk_container_remove(GTK_CONTAINER(w),
				     GTK_WIDGET(mytbar->tbar));
	}

	g_free(mytbar);
	mytbar = NULL;
	tb = build_toolbar();
	gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(mytbar->tbar));
	gtk_widget_show(GTK_WIDGET(tb));
}


