/*   property-ui.c: Property page for configuring look and feel
 *
 *   Copyright (C) 1998 Free Software Foundation
 *   Author: Havoc Pennington <hp@pobox.com>
 *           Jonathan Blandford <jrb@redhat.com>
 *
 *   Ported to capplet by Martin Baulig.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <locale.h>
#include "capplet-widget.h"

static GtkWidget *capplet;
static gboolean changing = FALSE;

typedef gint (*get_func) (void);
typedef void (*set_func) (gint);

/* we do app-options first */
typedef struct WidgetLayout {
	const gchar **label;
	GtkWidget *widget;
/*	gint ( * get_func ) (void);
	void ( * set_func ) (gint);*/
	gpointer get_func;
	gpointer set_func;
	gchar *config_path;
	gint initial_val;
	gint cur_val; /* option menu only */
} WidgetLayout;

typedef struct CappletLayout {
	WidgetLayout *layout;
	gchar *title;
} CappletLayout;

static gchar const *toolbar_detachable_name[] = {
	N_("Can detach and move toolbars"),
	NULL
};

static gchar const *toolbar_relief_name[] = {
	N_("Toolbars have relieved border"),
	NULL
};

static gchar const *toolbar_relief_btn_name[] = {
	N_("Toolbar buttons have relieved border"),
	NULL
};

static gchar const *toolbar_lines_name[] = {
	N_("Toolbars have line separators"),
	NULL
};

static gchar const *toolbar_labels_name[] = {
	N_("Toolbars have text labels"),
	NULL
};

static WidgetLayout toolbar_options[] = {
	{toolbar_detachable_name, NULL,
	 gnome_preferences_get_toolbar_detachable,
	 gnome_preferences_set_toolbar_detachable,
	 NULL},
	{toolbar_relief_name, NULL,
	 gnome_preferences_get_toolbar_relief,
	 gnome_preferences_set_toolbar_relief,
	 NULL},
	{toolbar_relief_btn_name, NULL,
	 gnome_preferences_get_toolbar_relief_btn,
	 gnome_preferences_set_toolbar_relief_btn,
	 NULL},
	{toolbar_lines_name, NULL,
	 gnome_preferences_get_toolbar_lines,
	 gnome_preferences_set_toolbar_lines,
	 NULL},
	{toolbar_labels_name, NULL,
	 gnome_preferences_get_toolbar_labels,
	 gnome_preferences_set_toolbar_labels,
	 NULL},
	{NULL, NULL, NULL, NULL, NULL}
};

static gchar const *menubar_detachable_name [] = {
	N_("Can detach and move menus"),
	NULL
};

static gchar const *menubar_relief_name [] = {
	N_("Menus have relieved border"),
	NULL
};
static gchar const *menus_have_tearoff_name [] = {
	N_("Submenus can be torn off"),
	NULL
};
static gchar const *menus_have_icons_name [] = {
	N_("Menu items have icons"),
	NULL
};
static WidgetLayout menu_options[] = {
	{menubar_detachable_name, NULL,
	 gnome_preferences_get_menubar_detachable,
	 gnome_preferences_set_menubar_detachable,
	 NULL},
	{menubar_relief_name, NULL,
	 gnome_preferences_get_menubar_relief,
	 gnome_preferences_set_menubar_relief,
	 NULL},
	{menus_have_tearoff_name, NULL,
	 gnome_preferences_get_menus_have_tearoff,
	 gnome_preferences_set_menus_have_tearoff,
	 NULL},
	{menus_have_icons_name, NULL,
	 gnome_preferences_get_menus_have_icons,
	 gnome_preferences_set_menus_have_icons,
	 NULL},
	{NULL, NULL, NULL, NULL, NULL}
};

static gchar const *statusbar_interactive_name [] = {
	N_("Statusbar is interactive when possible"),
	NULL
};

static gchar const *statusbar_meter_on_right_name [] = {
	N_("Statusbar progress meter is on the right"),
	NULL
};

static WidgetLayout status_options[] = {
	{statusbar_interactive_name, NULL,
	 gnome_preferences_get_statusbar_interactive,
	 gnome_preferences_set_statusbar_interactive,
	 NULL},
	{statusbar_meter_on_right_name, NULL,
	 gnome_preferences_get_statusbar_meter_on_right,
	 gnome_preferences_set_statusbar_meter_on_right,
	 NULL},
	{NULL, NULL, NULL, NULL, NULL}
};

static CappletLayout app_options[] = {
	{menu_options, N_("Menu Options")},
	{status_options, N_("Statusbar Options")},
	{toolbar_options, N_("Toolbar Options")},
	{NULL, NULL}
};

/* Dialog options */
static gchar const *buttonbox_style_names[] = {
	N_("Dialog buttons"),
	N_("Default value"),
	N_("Spread buttons out"),
	N_("Put buttons on edges"),
	N_("Left-justify buttons"),
	N_("Right-justify buttons"),
	NULL
};

static gchar const *dialog_icons_name [] = {
	N_("Dialog buttons have icons"),
	NULL
};

static gchar const *statusbar_dialog_name [] = {
	N_("Use statusbar instead of dialog when possible"),
	NULL
};

static WidgetLayout dialog_layout [] = {
	{ buttonbox_style_names, NULL,
	  gnome_preferences_get_button_layout,
	  gnome_preferences_set_button_layout,
	  NULL},
	{dialog_icons_name, NULL,
	 NULL,
	 NULL,
	 "/Gnome/Icons/ButtonUseIcons=true"},
	{statusbar_dialog_name, NULL,
	 gnome_preferences_get_statusbar_dialog,
	 gnome_preferences_set_statusbar_dialog,
	 NULL},
	{NULL, NULL, NULL, NULL, NULL}
};


static const gchar *dialog_positions_names [] = {
	N_("Dialog position"),
	N_("Let window manager decide"),
	N_("Center of the screen"),
	N_("At the mouse pointer"),
	NULL
};

static const gchar *dialog_types_names [] = {
	N_("Dialog hints"),
	N_("Dialogs are like other windows"),
	N_("Dialogs are treated specially by window manager"),
	NULL
};

static const gchar *dialog_centered_name [] = {
	N_("Place dialogs over application window when possible"),
	NULL
};

static WidgetLayout dialog_behavior [] = {
	{dialog_positions_names, NULL,
	 gnome_preferences_get_dialog_position,
	 gnome_preferences_set_dialog_position,
	 NULL},
	{dialog_types_names, NULL,
	 gnome_preferences_get_dialog_type,
	 gnome_preferences_set_dialog_type,
	 NULL},
	{dialog_centered_name, NULL,
	 gnome_preferences_get_dialog_centered,
	 gnome_preferences_set_dialog_centered,
	 NULL},
	{NULL, NULL, NULL, NULL, NULL}
};

static CappletLayout dialog_options[] = {
	{dialog_layout, N_("Dialog Layout")},
	{dialog_behavior, N_("Dialog Behavior")},
	{NULL, NULL}
};

/* looking for better descriptions here... */
static const gchar *mdi_mode_names[] = {
	N_("Default MDI Mode"),
	N_("Notebook"),
	N_("Toplevel"),
	N_("Modal"),
	NULL
};

static const gchar *tab_position_names[] = {
	N_("MDI notebook tab position"),
	N_("Left"),
	N_("Right"),
	N_("Top"),
	N_("Bottom"),
	NULL
};
static WidgetLayout gnome_mdi_options [] = {
	{mdi_mode_names, NULL,
	 gnome_preferences_get_mdi_mode,
	 gnome_preferences_set_mdi_mode,
	 NULL},
	{tab_position_names, NULL,
	 gnome_preferences_get_mdi_tab_pos,
	 gnome_preferences_set_mdi_tab_pos,
	 NULL},
	{NULL, NULL, NULL, NULL, NULL}
};
static CappletLayout mdi_options[] = {
	{gnome_mdi_options, N_("GNOME MDI Options")},
	{NULL, NULL}
};


static void
checkbutton_cb (GtkWidget *button,
		WidgetLayout *layout)

{
	if (changing)
		return;
	capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
static void
option_menu_cb (GtkWidget *menu,
		gpointer data)

{
	WidgetLayout *wlay;
	if (changing)
		return;
	wlay = gtk_object_get_data (GTK_OBJECT (menu), "WidgetLayout");
	wlay->cur_val = GPOINTER_TO_INT (data);
	capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}

static GtkWidget *
make_widget (WidgetLayout *wlay)
{
	GtkWidget *button;
	GtkWidget *option_menu;
	GtkWidget *menu;
	GtkWidget *menuitem;
	GtkWidget *hbox;
	GtkWidget *label;
	const gchar **text;
	gint i, def = 0;

	/* what type of widget are we? */
	if (wlay->label[1] == NULL) {
		/* we're a check_button */
		button = gtk_check_button_new_with_label (_(*(wlay->label)));
		wlay->widget = button;
		if (wlay->config_path == NULL) {
			wlay->initial_val = ((get_func)wlay->get_func) ();
			wlay->cur_val = ((get_func)wlay->get_func) ();
		} else {
			wlay->initial_val = gnome_config_get_bool (wlay->config_path);
			wlay->cur_val = gnome_config_get_bool (wlay->config_path);
		}
		gtk_signal_connect (GTK_OBJECT (button), "toggled",
				    GTK_SIGNAL_FUNC (checkbutton_cb),
				    wlay);
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button),
					    wlay->initial_val);
		return button;
	} else {
		option_menu = gtk_option_menu_new();
		menu = gtk_menu_new();
		wlay->widget = option_menu;

		text = wlay->label;
		wlay->initial_val = ((get_func) wlay->get_func) ();
		wlay->cur_val = ((get_func) wlay->get_func) ();
		for (i = 1; text[i] != NULL; i++) {
			menuitem = gtk_menu_item_new_with_label(_(text[i]));
			gtk_widget_show (menuitem);
			gtk_menu_append (GTK_MENU (menu), menuitem);
			gtk_object_set_data (GTK_OBJECT (menuitem), "WidgetLayout", wlay);
			gtk_signal_connect ( GTK_OBJECT (menuitem), "activate",
					     GTK_SIGNAL_FUNC(option_menu_cb),
					     GINT_TO_POINTER (i-1));
			if ( i == wlay->initial_val) {
				def = i;
			}


		}

		gtk_option_menu_set_menu (GTK_OPTION_MENU(option_menu), menu );

		gtk_option_menu_set_history (GTK_OPTION_MENU(option_menu), def );

		hbox = gtk_hbox_new (FALSE, 0);
		label = gtk_label_new(_(wlay->label[0]));
		gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, GNOME_PAD );
		/* FIXME: option menu width should be just wide enough to display
		   widest menu item.  Ideally the option menu code would handle this
		   for us.  Also, the button is not tall enough -- descenders seem
		   to get clipped on my display.  */
		gtk_box_pack_start ( GTK_BOX(hbox), option_menu, TRUE, FALSE, 0 );
		return hbox;
	}

}

static GtkWidget *
make_app_defaults (CappletLayout *cap_lay)
{
	GtkWidget *vbox_left, *vbox_right, *vbox2;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *widget;
	gint i, j;

	hbox = gtk_hbox_new(FALSE, GNOME_PAD);
	vbox_left = gtk_vbox_new (FALSE, GNOME_PAD);
	vbox_right = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (hbox), vbox_left, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox_right, TRUE, TRUE, 0);

	for (i = 0; cap_lay[i].title != NULL; i++) {
		frame = gtk_frame_new (_(cap_lay[i].title));
		if (!(i > 1))
			gtk_box_pack_start (GTK_BOX (vbox_left), frame, FALSE, FALSE, 0);
		else
			gtk_box_pack_start (GTK_BOX (vbox_right), frame, FALSE, FALSE, 0);
		vbox2 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
		gtk_container_set_border_width (GTK_CONTAINER (vbox2), GNOME_PAD_SMALL);
		gtk_container_add (GTK_CONTAINER (frame), vbox2);
		for (j = 0; cap_lay[i].layout[j].label != NULL; j++) {
			widget = make_widget (&(cap_lay[i].layout[j]));
			gtk_box_pack_start (GTK_BOX (vbox2), widget, FALSE, FALSE, 0);
		}
	}
	gtk_widget_show_all(hbox);
	return hbox;
}
static void
try_app (CappletLayout *cap_layout)
{
	gint i, j;

	for (i = 0; cap_layout[i].title != NULL; i++) {
		for (j = 0; cap_layout[i].layout[j].label != NULL; j++) {
			if (cap_layout[i].layout[j].label[1] == NULL) { /* We are a toggle-button */
				if (cap_layout[i].layout[j].config_path) {
					gnome_config_set_bool (cap_layout[i].layout[j].config_path,
							       GTK_TOGGLE_BUTTON (cap_layout[i].layout[j].widget)->active);
				} else {
					((set_func)cap_layout[i].layout[j].set_func) (GTK_TOGGLE_BUTTON (cap_layout[i].layout[j].widget)->active);
				}
			} else {
				((set_func)cap_layout[i].layout[j].set_func) (cap_layout[i].layout[j].cur_val);
			}
		}
	}
	/* This does a gnome_config_sync for us */
	gnome_preferences_save();
}
static void
revert_app (CappletLayout *cap_layout)
{
	gint i, j;

	changing = TRUE;
	for (i = 0; cap_layout[i].title != NULL; i++) {
		for (j = 0; cap_layout[i].layout[j].label != NULL; j++) {
			if (cap_layout[i].layout[j].label[1] == NULL) { /* We are a toggle-button */
				if (cap_layout[i].layout[j].config_path) {
					gnome_config_set_bool (cap_layout[i].layout[j].config_path,
							       cap_layout[i].layout[j].initial_val);
				} else {
					((set_func)cap_layout[i].layout[j].set_func) (cap_layout[i].layout[j].initial_val);
				}
				gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (cap_layout[i].layout[j].widget),
							     cap_layout[i].layout[j].initial_val);
			} else {
				((set_func)cap_layout[i].layout[j].set_func) (cap_layout[i].layout[j].initial_val);
				gtk_option_menu_set_history (GTK_OPTION_MENU (cap_layout[i].layout[j].widget), cap_layout[i].layout[j].initial_val);

			}
		}
	}
	changing = FALSE;
	/* This does a gnome_config_sync for us */
	gnome_preferences_save();
}

static void
try (GtkWidget *capplet, gpointer data)
{
	switch (CAPPLET_WIDGET (capplet)->capid) {
	case 0:
		try_app (dialog_options);
		break;
	case 1:
		try_app (app_options);
		break;
	case 2:
		try_app (mdi_options);
		break;

	}
	gnome_preferences_save ();
}
static void
revert (GtkWidget *capplet, gpointer data)
{
	switch (CAPPLET_WIDGET (capplet)->capid) {
	case 0:
		revert_app (dialog_options);
		break;
	case 1:
		revert_app (app_options);
		break;
	case 2:
		revert_app (mdi_options);
		break;
	}
	gnome_preferences_save ();
}

static void
help (GtkWidget *capplet)
{
  gchar *helpfile;
  GnomeHelpMenuEntry help_entry= {"control-center",NULL};

  switch(CAPPLET_WIDGET(capplet)->capid) {
  case 0: /* Dialogs */
  default:
    helpfile = "gccui.html#GCCDIALOGS";
    break;
  case 1: /* Application */
    helpfile = "gccui.html#GCCAPPDEFAULTS";
    break;
  case 2: /* MDI */
    helpfile = "gccui.html#GCCMDI";
    break;
  }

  help_entry.path = helpfile;

  gnome_help_display (NULL, &help_entry);
}

static void
ui_setup (GtkWidget *old, GtkWidget *the_capplet)
{
	capplet = the_capplet;

	gtk_signal_connect (GTK_OBJECT(the_capplet), "new_multi_capplet",
			    GTK_SIGNAL_FUNC(ui_setup), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (the_capplet), GNOME_PAD);

	switch (CAPPLET_WIDGET(the_capplet)->capid) {
	case 1: /* Application */
		gtk_container_add (GTK_CONTAINER (the_capplet),
				   make_app_defaults (app_options));
		break;
	case 2: /* MDI */
		gtk_container_add (GTK_CONTAINER (the_capplet),
				   make_app_defaults (mdi_options));
		break;
	default:
	case 0: /* Dialogs */
		gtk_container_add (GTK_CONTAINER (the_capplet),
				   make_app_defaults (dialog_options));
		break;
	}

	/* Finished */
	gtk_widget_show (the_capplet);
	gtk_signal_connect (GTK_OBJECT (the_capplet), "try",
			    GTK_SIGNAL_FUNC (try), NULL);
	gtk_signal_connect (GTK_OBJECT (the_capplet), "revert",
			    GTK_SIGNAL_FUNC (revert), NULL);
	gtk_signal_connect (GTK_OBJECT (the_capplet), "ok",
			    GTK_SIGNAL_FUNC (try), NULL);
	gtk_signal_connect (GTK_OBJECT (the_capplet), "cancel",
			    GTK_SIGNAL_FUNC (revert), NULL);
	gtk_signal_connect (GTK_OBJECT (the_capplet), "help",
			    GTK_SIGNAL_FUNC (help), NULL);
}

int
main (int argc, char **argv)
{
	GtkWidget *the_capplet;

	setlocale(LC_ALL, "");
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
	changing = TRUE;
	if (gnome_capplet_init ("ui-properties", VERSION, argc,
				argv, NULL, 0, NULL) < 0)
		return 1;

	the_capplet = capplet_widget_new();
	gnome_preferences_load();
	ui_setup (NULL, the_capplet);
	changing = FALSE;
	capplet_gtk_main ();

	return 0;
}
