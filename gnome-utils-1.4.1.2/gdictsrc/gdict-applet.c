/* $Id: gdict-applet.c,v 1.15.2.1 2001/10/03 03:38:06 jfleck Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict panel applet
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>
#include <applet-widget.h>

#include "gdict-app.h"
#include "gdict-about.h"
#include "gdict-pref.h"
#include "gdict-applet.h"
#include "gdict-pref-dialog.h"

gboolean gdict_applet_toggle = TRUE;

#define DOCKED_APPLET_WIDTH 74
#define FLOATING_APPLET_WIDTH 14
#define SHORT_APPLET_HEIGHT 22
#define TALL_APPLET_HEIGHT 44

/* Returns the ideal vertical applet size based on panel width and
 * orientation. */
static gint 
gdict_applet_determine_height(GDictApplet *applet)
{
	int ideal_panel_size = 0;
	PanelOrientType orientation = ORIENT_UP;

	ideal_panel_size = applet_widget_get_panel_pixel_size(
		APPLET_WIDGET(applet->applet_widget));
	orientation = applet_widget_get_panel_orient(
		APPLET_WIDGET(applet->applet_widget));

	if ((orientation == ORIENT_UP || orientation == ORIENT_DOWN) && 
	    ideal_panel_size < PIXEL_SIZE_STANDARD)

		return SHORT_APPLET_HEIGHT;
	else
		return TALL_APPLET_HEIGHT;
}

/* Returns the ideal horizontal applet size based on panel width and
 * orientation. */
static gint
gdict_applet_determine_width(GDictApplet *applet)
{
	if (applet->handlebox_widget != NULL && 
		GTK_HANDLE_BOX(applet->handlebox_widget)->child_detached)

		return FLOATING_APPLET_WIDTH;
	else
		return DOCKED_APPLET_WIDTH;
}

/* Configures and displays the applet as appropriate for the panel
 * width, orientation, and handlebox state. */
static void
gdict_applet_render (GDictApplet * applet)
{
	gint applet_height = 0;
	gint applet_width = 0;

	applet_height = gdict_applet_determine_height(applet);
	applet_width = gdict_applet_determine_width(applet);

	if (applet_height == TALL_APPLET_HEIGHT) {
		gtk_widget_show(applet->button_widget);
	}
	else {
		gtk_widget_hide(applet->button_widget);
	}

	gtk_container_set_border_width (GTK_CONTAINER(applet->vbox_widget), 2);
	gtk_widget_show(applet->vbox_widget);
	gtk_widget_show(applet->entry_widget);
	if (applet->handle) {
		g_return_if_fail(applet->handlebox_widget);
		gtk_handle_box_set_shadow_type(
			GTK_HANDLE_BOX(applet->handlebox_widget), GTK_SHADOW_IN);
		gtk_widget_set_usize(applet->handlebox_widget, applet_width, 
							 applet_height);
		gtk_widget_show(applet->handlebox_widget);
	}
	else {
		gtk_widget_set_usize(applet->vbox_widget, applet_width, applet_height);
	}
	gtk_widget_show (applet->applet_widget);
}

/* Signal handler that gets called when the user re-attaches the
 * handlebox's float window to its ghost. */
static gint
applet_attach_cb (GtkHandleBox * handlebox, AppletWidget *widget, 
				  gpointer data)
{
	gdict_applet_render((GDictApplet *) data);
	return FALSE;
}

/* Signal handler that gets called when the user detaches the
 * handlebox's float window. */
static gint
applet_detach_cb (GtkHandleBox * handlebox, AppletWidget *widget, 
				  gpointer data)
{
	gdict_applet_render((GDictApplet *) data);
	return FALSE;
}

/* Signal handler that gets called when the user changes the
 * orientation of his panel. */
static gint
applet_change_orient_cb (AppletWidget *widget, PanelOrientType orientation, 
						 gpointer data)
{
	GDictApplet * applet = (GDictApplet *) data;

	/* Note: Technically we could use the orientation parameter to
	 * resize the applet in a slightly more efficient fashion, but for
	 * now I decided to reuse gdict_applet_render for the sake of
	 * simplicity. */ 
	gdict_applet_render((GDictApplet *) applet);
	return FALSE;
}

/* Signal handler that gets called when the user changes the pixel
 * width of his panel. */
static gint
applet_change_pixel_size_cb (AppletWidget *widget, int size, gpointer data)
{
	GDictApplet * applet = (GDictApplet *) data;

	/* Note: Technically we could use the size parameter to
	 * resize the applet in a slightly more efficient fashion, but for
	 * now I decided to reuse this gdict_applet_render for the sake of
	 * simplicity. */ 
	gdict_applet_render((GDictApplet *) applet);
	return FALSE;
}

/* Signal handler for the "Help" menu item */
static void
cb_help (GtkWidget *w, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "gdict", "index.html" };
	gnome_help_display(NULL, &help_entry);
}

/* Signal handler for the "Show/Hide Definition Window" menu item */
static void
applet_show_hide_defbox_cb (AppletWidget *widget, gpointer data) 
{
    if (!GTK_WIDGET_VISIBLE (gdict_app))
        gtk_widget_show (gdict_app);
    else
        gtk_widget_hide (gdict_app);
}

/* Signal handler for "Spell Check" menu item */
static void
applet_spell_cb (AppletWidget *widget, gpointer data) 
{
    gchar *text;
	GDictApplet * applet = (GDictApplet *) data;

    text = gtk_entry_get_text (GTK_ENTRY (applet->entry_widget));
    gdict_spell (text, FALSE);
    gtk_entry_set_text (GTK_ENTRY (applet->entry_widget), "");
}

/* Signal handler to process mouse clicks on the applet */
static void
applet_clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    if ((ev == NULL) || (ev->button != 1) || (ev->type != GDK_2BUTTON_PRESS ))
        return;

    applet_show_hide_defbox_cb (APPLET_WIDGET (widget), data);
}

/* Signal handler for the "About" menu item */
static void
applet_about_cb (AppletWidget *widget, gpointer data)
{
    gdict_about();
}

/* Signal handler for the "Preferences" menu item */
static void
applet_pref_cb (AppletWidget *widget, gpointer data) 
{
    gdict_app_show_preferences ();
}

/* Signal handler for any event that should trigger a lookup */
static void
applet_lookup_cb (GtkWidget *widget, gpointer data)
{
	GDictApplet * applet = (GDictApplet *) data;

    gchar *text = gtk_entry_get_text(GTK_ENTRY(applet->entry_widget));
    if (*text == 0)
        return;
    g_strdown (text);
    if (!GTK_WIDGET_VISIBLE (gdict_app))
        gtk_widget_show (gdict_app);
    gtk_entry_set_text (GTK_ENTRY (applet->entry_widget), "");
    gtk_entry_set_text (GTK_ENTRY (word_entry), text);
    gtk_editable_select_region (GTK_EDITABLE (word_entry), 0, strlen (text));
    gdict_app_do_lookup (text);
}

/* Establishes explicit associations between wigets and events that
 * aren't enabled by default. */
static void 
gdict_applet_set_events (GDictApplet * applet)
{
    gtk_widget_set_events (applet->applet_widget, 
						   gtk_widget_get_events(applet->applet_widget) |
                           GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK );
}

/* Connects any signal handlers needed for the applet to function. */
static void
gdict_applet_connect_signals (GDictApplet * applet)
{
    gtk_signal_connect (GTK_OBJECT(applet->applet_widget), 
						"button_press_event",
                        GTK_SIGNAL_FUNC(applet_clicked_cb), 
						(gpointer) applet);
    gtk_signal_connect (GTK_OBJECT(applet->applet_widget), "destroy",
                        GTK_SIGNAL_FUNC(gtk_main_quit), 
						(gpointer) applet);
    gtk_signal_connect (GTK_OBJECT(applet->button_widget), "clicked",
						GTK_SIGNAL_FUNC(applet_lookup_cb), 
						(gpointer) applet);
    gtk_signal_connect (GTK_OBJECT(applet->entry_widget), "activate",
                        GTK_SIGNAL_FUNC(applet_lookup_cb), 
						(gpointer) applet);
	gtk_signal_connect (GTK_OBJECT(applet->applet_widget), "change_orient",
						GTK_SIGNAL_FUNC(applet_change_orient_cb),
						(gpointer) applet);
	gtk_signal_connect (GTK_OBJECT(applet->applet_widget), "change_pixel_size",
						GTK_SIGNAL_FUNC(applet_change_pixel_size_cb),
						(gpointer) applet);
	if (applet->handle) {
        gtk_signal_connect (GTK_OBJECT(applet->handlebox_widget), 
							"child_detached",
                            GTK_SIGNAL_FUNC(applet_detach_cb), 
							(gpointer) applet);
        gtk_signal_connect (GTK_OBJECT(applet->handlebox_widget), 
							"child_attached",
                            GTK_SIGNAL_FUNC(applet_attach_cb), 
							(gpointer) applet);
	}
}

/* Sets up the applet's context menu */
static void
gdict_applet_populate_menu (GDictApplet * applet)
{
    applet_widget_register_stock_callback 
		(APPLET_WIDGET (applet->applet_widget), "about",
		 GNOME_STOCK_MENU_ABOUT, _("About..."),
		 applet_about_cb, NULL);
    applet_widget_register_stock_callback
        (APPLET_WIDGET (applet->applet_widget), "help",
		 GNOME_STOCK_PIXMAP_HELP, _("Help"),
		 (AppletCallbackFunc) cb_help,
		 NULL);
    applet_widget_register_stock_callback 
		(APPLET_WIDGET (applet->applet_widget), "preferences",
		 GNOME_STOCK_MENU_PREF, _("Preferences..."),
		 applet_pref_cb, NULL);
    applet_widget_register_callback
		(APPLET_WIDGET (applet->applet_widget), "show_hide_window", 
		 _("Show/Hide Definition Window..."), applet_show_hide_defbox_cb,
		 NULL);
    applet_widget_register_callback
		(APPLET_WIDGET (applet->applet_widget), "spell_check", 
		 _("Spell Check..."), applet_spell_cb, (gpointer) applet);
}

/* Constructs an applet object, containing all its necessary widgets */
static GDictApplet * 
gdict_applet_new ()
{
	GDictApplet *applet = NULL;

	applet = g_malloc (sizeof(GDictApplet));

    if ((applet->applet_widget = applet_widget_new("gdict")) == NULL)
        g_error(_("Cannot create applet!\n"));

    applet->vbox_widget = gtk_vbox_new(FALSE, 2);
    applet->entry_widget = gtk_entry_new();
    applet->button_widget = gtk_button_new_with_label(_("Look up"));

    applet->handle = gdict_pref.applet_handle;

    if (gdict_applet_determine_height (applet) != TALL_APPLET_HEIGHT)
	    applet->handle = TRUE;

    if (applet->handle) {
        applet->handlebox_widget = gtk_handle_box_new();
    } else {
		applet->handlebox_widget = NULL;
    }

	return applet;
}

/* Deletes the GDictApplet structure returned by gdict_applet_new */
void gdict_applet_delete (GDictApplet * applet)
{
	g_return_if_fail (applet);
	/* Can I safely assume that GTK will clean up my widgets for me? */
	g_free (applet);
}

/* Packs and adds widgets inside container widgets, as needed for the
 * applet.  When this call is finished, the interface for the applet
 * should be all set up and ready to display using gdict_applet_render. */
static void
gdict_applet_pack_widgets (GDictApplet * applet)
{
    gtk_box_pack_end(GTK_BOX(applet->vbox_widget), applet->button_widget, 
					 TRUE, TRUE, 0);

    if (applet->handle) {
        gtk_container_add(GTK_CONTAINER(applet->handlebox_widget), 
						  applet->vbox_widget);
        applet_widget_add(APPLET_WIDGET(applet->applet_widget), 
						  applet->handlebox_widget);
	}
	else {
        applet_widget_add(APPLET_WIDGET(applet->applet_widget), 
						  applet->vbox_widget);
	}

    /* We want to allow pasting into the input box so we pack it after */
    /* applet_widdget_add has bound the middle button -- thanks to webcontrol applet! :-) */
    gtk_box_pack_end(GTK_BOX(applet->vbox_widget), applet->entry_widget, TRUE, 
					 TRUE, 0);
}

/* Creates and displays the applet.  Returns a structure containing
 * the state of the applet.  After applet_widget_gtk_main exits, this
 * structure may be safely deleted using gdict_applet_delete */
GDictApplet * gdict_applet_create (void)
{
	GDictApplet *applet = NULL;

#if 0
    GtkWidget *frame_in, *frame_out;
#endif

    g_assert(gdict_applet_toggle);

	applet = gdict_applet_new();

	gdict_applet_set_events (applet);
	gdict_applet_connect_signals (applet);
	gdict_applet_populate_menu (applet);
	gdict_applet_pack_widgets (applet);
	gdict_applet_render (applet);

	return applet;
}
