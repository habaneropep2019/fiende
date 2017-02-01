/*   gnome-hint-properties: crapplet for gnome-hint
 *
 *   Copyright (C) 1999 Free Software Foundation
 *   Author: George Lebl <jirka@5z.com>
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
#include <capplet-widget.h>
#include <libgnomeui/gnome-window-icon.h>

static gboolean changing = FALSE;

static void
changed_cb(GtkWidget *button, GtkWidget *crapplet)
{
	if( ! changing)
		capplet_widget_state_changed(CAPPLET_WIDGET(crapplet), TRUE);
}

static void
cb_enable_cb(GtkWidget *button, GtkWidget *enable)
{
	gtk_widget_set_sensitive(enable, GTK_TOGGLE_BUTTON(button)->active);
}

static void
help(GtkWidget *crapplet)
{
  GnomeHelpMenuEntry help_entry= {"control-center",
  "session.html#STARTUP-HINTS"};
  gnome_help_display (NULL, &help_entry);
}

static gboolean hints_enabled = TRUE;
static gboolean display_fortune = FALSE;
static gboolean display_motd = FALSE;
static char *motdfile = NULL;

static GtkWidget *enable_box;
static GtkWidget *fortune_box;
static GtkWidget *motd_box;
static GtkWidget *motd_entry;

static void
loadup_vals(void)
{
	hints_enabled = gnome_config_get_bool("/Gnome/Login/RunHints=true");
	display_fortune = gnome_config_get_bool("/Gnome/Login/HintsAreFortune=false");
	display_motd = gnome_config_get_bool("/Gnome/Login/HintsAreMotd=false");
	g_free(motdfile);
	motdfile = gnome_config_get_string("/Gnome/Login/MotdFile=/etc/motd");
}

static void
try(GtkWidget *crapplet, gpointer data)
{
	gnome_config_set_bool("/Gnome/Login/RunHints",
			      GTK_TOGGLE_BUTTON(enable_box)->active);
	gnome_config_set_bool("/Gnome/Login/HintsAreFortune",
			      GTK_TOGGLE_BUTTON(fortune_box)->active);
	gnome_config_set_bool("/Gnome/Login/HintsAreMotd",
			      GTK_TOGGLE_BUTTON(motd_box)->active);
	gnome_config_set_string("/Gnome/Login/MotdFile",
				gtk_entry_get_text(GTK_ENTRY(motd_entry)));
	gnome_config_sync();
}

static void
revert(GtkWidget *crapplet, gpointer data)
{
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(enable_box),
				    hints_enabled);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(fortune_box),
				    display_fortune);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(motd_box),
				    display_motd);
	gtk_entry_set_text(GTK_ENTRY(motd_entry), motdfile);
	try(crapplet, data);
}

static void
setup_the_ui(GtkWidget *the_crapplet)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *hints_box;
	GtkWidget *motd_gentry, *motd_label;

	gtk_container_set_border_width(GTK_CONTAINER(the_crapplet), GNOME_PAD);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(the_crapplet), vbox);
	
	/*add the enable box*/
	enable_box = gtk_check_button_new_with_label(_("Enable login hints"));
	gtk_box_pack_start(GTK_BOX(vbox), enable_box, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(enable_box), "toggled",
			   GTK_SIGNAL_FUNC(changed_cb),
			   the_crapplet);

	/*add the hint box*/
	hints_box = gtk_radio_button_new_with_label(NULL, _("Display normal hints"));
	gtk_box_pack_start(GTK_BOX(vbox), hints_box, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(hints_box), "toggled",
			   GTK_SIGNAL_FUNC(changed_cb),
			   the_crapplet);

	/*add the fortune box*/
	fortune_box = gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(hints_box)),
		_("Display fortunes instead of hints"));
	gtk_box_pack_start(GTK_BOX(vbox), fortune_box, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(fortune_box), "toggled",
			   GTK_SIGNAL_FUNC(changed_cb),
			   the_crapplet);

	/*add the motd box*/
	motd_box = gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(hints_box)),
		_("Display message of the day instead of hints"));
	gtk_box_pack_start(GTK_BOX(vbox), motd_box, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(motd_box), "toggled",
			   GTK_SIGNAL_FUNC(changed_cb),
			   the_crapplet);

	/*the motd entry*/
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	motd_label = gtk_label_new(_("Message of the day file to use: ")),
	gtk_box_pack_start(GTK_BOX(hbox), motd_label, FALSE, FALSE, 0);
	motd_gentry = gnome_file_entry_new("motd", _("Browse"));
	gtk_box_pack_start(GTK_BOX(hbox), motd_gentry, TRUE, TRUE, 0);
	motd_entry = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(motd_gentry));
	gtk_signal_connect(GTK_OBJECT(motd_entry), "changed",
			   GTK_SIGNAL_FUNC(changed_cb),
			   the_crapplet);

	/*here we set up the enable disable thing*/
	gtk_signal_connect(GTK_OBJECT(enable_box), "toggled",
			   GTK_SIGNAL_FUNC(cb_enable_cb),
			   hints_box);
	gtk_signal_connect(GTK_OBJECT(enable_box), "toggled",
			   GTK_SIGNAL_FUNC(cb_enable_cb),
			   fortune_box);
	gtk_signal_connect(GTK_OBJECT(enable_box), "toggled",
			   GTK_SIGNAL_FUNC(cb_enable_cb),
			   motd_box);
	/*!!!we need to use different widgets for enable box and motd_box*/
	gtk_signal_connect(GTK_OBJECT(enable_box), "toggled",
			   GTK_SIGNAL_FUNC(cb_enable_cb),
			   hbox);
	gtk_signal_connect(GTK_OBJECT(motd_box), "toggled",
			   GTK_SIGNAL_FUNC(cb_enable_cb),
			   motd_gentry);
	gtk_signal_connect(GTK_OBJECT(motd_box), "toggled",
			   GTK_SIGNAL_FUNC(cb_enable_cb),
			   motd_label);

	/*setup defaults*/
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(enable_box),
				    hints_enabled);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(hints_box),
				    ! display_fortune &&
				    ! display_motd);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(fortune_box),
				    display_fortune);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(motd_box),
				    display_motd);
	gtk_entry_set_text(GTK_ENTRY(motd_entry), motdfile);

	/* Set up default sensitivities */
	gtk_widget_set_sensitive (hbox, hints_enabled);
	gtk_widget_set_sensitive (hints_box, hints_enabled);
	gtk_widget_set_sensitive (fortune_box, hints_enabled);
	gtk_widget_set_sensitive (motd_box, hints_enabled);

	gtk_widget_set_sensitive (motd_gentry, display_motd);
	gtk_widget_set_sensitive (motd_label, display_motd);

	/* Finished */
	gtk_widget_show_all(the_crapplet);
	gtk_signal_connect(GTK_OBJECT(the_crapplet), "try",
			   GTK_SIGNAL_FUNC(try), NULL);
	gtk_signal_connect(GTK_OBJECT(the_crapplet), "revert",
			   GTK_SIGNAL_FUNC(revert), NULL);
	gtk_signal_connect(GTK_OBJECT(the_crapplet), "ok",
			   GTK_SIGNAL_FUNC(try), NULL);
	gtk_signal_connect(GTK_OBJECT(the_crapplet), "cancel",
			   GTK_SIGNAL_FUNC(revert), NULL);
	/*gtk_signal_connect(GTK_OBJECT(the_crapplet), "help",
			   GTK_SIGNAL_FUNC(help), NULL);*/
}

int
main (int argc, char **argv)
{
	GtkWidget *the_crapplet;

	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	/* Make sure that we don't register changes yet */
	changing = TRUE;

	if(gnome_capplet_init("gnome-hint-properties", VERSION, argc,
			      argv, NULL, 0, NULL) < 0)
		return 1;
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR "/gnome-hint.png");
	loadup_vals();

	the_crapplet = capplet_widget_new();

	setup_the_ui(the_crapplet);

	/* start registering user changes */
	changing = FALSE;

	capplet_gtk_main();

	return 0;
}
