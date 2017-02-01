/* $Id: gdict-app.c,v 1.17.2.1 2001/10/03 03:38:06 jfleck Exp $ */
/* -*- mode: c; style: k&r; c-basic-offset: 4 -*- */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict main window
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <gnome.h>

#include "dict.h"
#include "gdict-about.h"
#include "gdict-pref.h"
#include "gdict-applet.h"

#include "gdict-app.h"
#include "gdict-defbox.h"
#include "gdict-speller.h"

#ifdef HAVE_GNOME_PRINT
#  include <libgnomeprint/gnome-printer-dialog.h>
#  include <libgnomeprint/gnome-printer-profile.h>
#endif /* HAVE_GNOME_PRINT */

#define APPNAME "gdict"

GtkWidget *gdict_app;
GtkWidget *gdict_appbar;
GtkWidget *gnome_word_entry;
GtkWidget *word_entry;
GDictDefbox *defbox;
GDictSpeller *speller;
GtkWidget *pref_dialog;
GtkWidget *socket_error_dialog;

#ifdef HAVE_GNOME_PRINT
GnomePrinter *gdict_printer = NULL;
#endif /* HAVE_GNOME_PRINT */

dict_context_t *context;

static gint
socket_dialog_close_cb (GtkWidget *widget, gpointer data) 
{
    socket_error_dialog = NULL;
    return FALSE;
}

static void
socket_error_cb (GtkWidget *widget, gchar *message, gpointer data) 
{
    if (!socket_error_dialog) {
        socket_error_dialog = 
            gnome_error_dialog_parented (message, GTK_WINDOW (gdict_app));
        gtk_signal_connect (GTK_OBJECT (socket_error_dialog), "close",
                            GTK_SIGNAL_FUNC (socket_dialog_close_cb), NULL);
    }
}

/* gdict_init_context
 *
 * Initialises the context object with information on the current server
 *
 * Retrurns 0 on success, -1 if the server could not be found
 */

gint
gdict_init_context (void) 
{
    if (context) 
      dict_context_destroy (context);
    
    context = dict_context_new (gdict_pref.server, gdict_pref.port);
    context->command = dict_disconnect_command_new ();

    if (context->hostinfo)
	return 0;
    else
	return -1;
}

void
gdict_app_clear (void) 
{
    gchar *word;
    
    gdict_defbox_clear (defbox);
    
    /* Update entry */
    if ((word = gdict_defbox_get_word (defbox))) {
        gtk_entry_set_text (GTK_ENTRY(word_entry), word);
        gtk_editable_select_region (GTK_EDITABLE(word_entry), 0, strlen(word));
    }
}

static void
spell_lookup_start_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_clear_stack(GNOME_APPBAR(gdict_appbar));
    gnome_appbar_push(GNOME_APPBAR(gdict_appbar), _("Spell-checking..."));
    gnome_appbar_refresh(GNOME_APPBAR(gdict_appbar));
}

static void
spell_lookup_done_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("Spell check done"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
spell_not_found_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("No matches found"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static gint
spell_close_cb (GtkWidget *widget, gpointer data) 
{
    speller = NULL;
    return FALSE;
}

void
gdict_open_speller (void) 
{
    if (!speller) {
        speller = GDICT_SPELLER (gdict_speller_new (context));
        gnome_dialog_set_parent (GNOME_DIALOG (speller), 
                                 GTK_WINDOW (gdict_app));
        
        if (!speller) return;

        gtk_signal_connect (GTK_OBJECT (speller), "word_lookup_start",
                            GTK_SIGNAL_FUNC (spell_lookup_start_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "word_lookup_done",
                            GTK_SIGNAL_FUNC (spell_lookup_done_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "word_not_found",
                            GTK_SIGNAL_FUNC (spell_not_found_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "socket_error",
                            GTK_SIGNAL_FUNC (socket_error_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (speller), "close",
                            GTK_SIGNAL_FUNC (spell_close_cb), NULL);
    }
    
    gtk_widget_show (GTK_WIDGET (speller));
}


gint
gdict_spell (gchar *text, gboolean pattern) 
{
    g_return_val_if_fail(text != NULL, 0);

    gdict_open_speller ();

    if (!speller) return -1;

    if (pattern) 
	speller->strat = "re";
    else
	speller->strat = gdict_pref.dfl_strat;

    if (gdict_speller_lookup (speller, text) == -1) return -1;

    return 0;
}

static gboolean
is_pattern (gchar *text) 
{
    if (strpbrk (text, "*|{}()[]"))
	return TRUE;
    else
	return FALSE;
}

void
gdict_app_do_lookup (gchar *text) 
{
    gint retval;

    if (gdict_pref.smart && is_pattern (text)) {
	retval = gdict_spell (text, TRUE);
    }
    else {
	retval = gdict_defbox_lookup (defbox, text);
	if (!retval) gtk_widget_show (gdict_app);
    }

    if (retval) {
	gdict_not_online ();
    }
}

static void
lookup_entry (void) 
{
    gchar *text = gtk_entry_get_text (GTK_ENTRY (word_entry));
    g_strdown(text);
    gdict_app_do_lookup (text);
}

static void
lookup_defbox (void) 
{
    gchar *text = NULL;
    gtk_signal_emit_by_name (GTK_OBJECT (defbox), "copy_clipboard");
    gtk_entry_set_text (GTK_ENTRY (word_entry), "");
    gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "paste_clipboard");
    text = gtk_entry_get_text (GTK_ENTRY (word_entry));
    g_strdown (text);
    gnome_entry_append_history (GNOME_ENTRY (gnome_word_entry), 1, text);
    gdict_app_do_lookup (text);
}

static void
lookup_any (void) 
{
    if (GTK_WIDGET_HAS_FOCUS (word_entry))
	lookup_entry ();
    else if (GTK_WIDGET_HAS_FOCUS (defbox))
	lookup_defbox ();
}

static void
def_lookup_start_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_clear_stack(GNOME_APPBAR(gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("Looking up word..."));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
def_lookup_done_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("Lookup done"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
def_not_found_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("No matches found"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
    
    if (gdict_pref.smart) {
        gdict_spell (gdict_defbox_get_word (defbox), FALSE);
    }
}

static void
def_substr_not_found_cb (GtkWidget *widget, gpointer data) 
{
    gnome_appbar_pop (GNOME_APPBAR (gdict_appbar));
    gnome_appbar_push (GNOME_APPBAR (gdict_appbar), _("String not found"));
    gnome_appbar_refresh (GNOME_APPBAR (gdict_appbar));
}

static void
lookup_cb (GtkWidget *menuitem, gpointer user_data) 
{
    lookup_any();
}

static void
spell_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gchar *text;

    text = gtk_entry_get_text (GTK_ENTRY (word_entry));
    if (gdict_spell (text, FALSE) < 0)
	gdict_not_online ();
}

#ifdef HAVE_GNOME_PRINT

static void
print_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gdict_defbox_print (defbox);
}

static void
print_setup_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gdict_printer = gnome_printer_dialog_new_modal();
}

#endif /* HAVE_GNOME_PRINT */

static void
close_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gtk_widget_hide(gdict_app);
}

static void
exit_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gtk_main_quit();
}

static void
cut_cb (GtkWidget *button, gpointer user_data) 
{
    gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "cut_clipboard");
}

static void
copy_cb (GtkWidget *menuitem, gpointer user_data) 
{
    if (GTK_WIDGET_HAS_FOCUS (defbox))
        gtk_signal_emit_by_name (GTK_OBJECT (defbox), "copy_clipboard");
    else if (GTK_WIDGET_HAS_FOCUS (word_entry))
        gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "copy_clipboard");
}

static void
paste_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gtk_signal_emit_by_name (GTK_OBJECT (word_entry), "paste_clipboard");
}

static void
clear_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gtk_editable_select_region (GTK_EDITABLE (defbox), 0, 0);
}

static void
select_all_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gtk_editable_select_region (GTK_EDITABLE (defbox), 0, 
                                gtk_text_get_length (GTK_TEXT (defbox)));
}

static gchar *find_text;

static void
prompt_response_cb (GnomeAppBar *ab) 
{
    /* fprintf(stderr, "Response\n"); */
    if (ab->prompt) {
        g_free(find_text);
        find_text = gnome_appbar_get_response(ab);
        if (find_text)
            gdict_defbox_find (defbox, find_text, TRUE);
        gnome_appbar_clear_prompt(GNOME_APPBAR(gdict_appbar));
    }
}

static void
find_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    if (!defbox->def_cmd)
        return;

    gnome_appbar_set_prompt (GNOME_APPBAR (gdict_appbar), _("Find:"), TRUE);
}

static void
find_again_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    if (!defbox->def_cmd)
        return;
    if (find_text)
        gdict_defbox_find (defbox, find_text, FALSE);
}

static gint
pref_dialog_close_cb (GtkWidget *widget, gpointer user_data) 
{
    pref_dialog = NULL;
    return FALSE;
}

static void
pref_dialog_apply_cb (GtkWidget *widget, gpointer user_data) 
{
    gdict_defbox_reset (defbox, context);
    if (speller) gdict_speller_reset (speller, context);
}

void
gdict_not_online () 
{
    GtkWidget *w;
    gchar *s;
    
    s = g_strdup_printf (_("Unable to perform requested operation, either because the server\n"
			   "you are using is down or because you are not connected to the\n"
			   "Internet. If you are not already connected to the Internet, please\n" 
    			   "do so and try again."));
    w = gnome_error_dialog (s);
    gnome_dialog_run_and_close (GNOME_DIALOG (w));
    g_free (s);
}

void
gdict_app_show_preferences (void) 
{
    if (!pref_dialog) {
        pref_dialog = gdict_pref_dialog_new (context);
        gtk_signal_connect (GTK_OBJECT (pref_dialog), "apply_changes",
                            GTK_SIGNAL_FUNC (pref_dialog_apply_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (pref_dialog), "socket_error",
                            GTK_SIGNAL_FUNC (socket_error_cb), NULL);
        gtk_signal_connect (GTK_OBJECT (pref_dialog), "close",
                            GTK_SIGNAL_FUNC (pref_dialog_close_cb),
			    NULL);
    }

    gtk_widget_show (pref_dialog);
}

static void
preferences_cb (GtkWidget *menuitem, gpointer user_data) 
{
    gdict_app_show_preferences ();
}

static void
about_cb (GtkMenuItem *menuitem, gpointer user_data) 
{
    gdict_about();
}

static GnomeUIInfo file_menu_uiinfo[] = {
    GNOMEUIINFO_ITEM_STOCK (N_("Look up"),
			    N_("Look up word in dictionary"),
			    lookup_cb, GNOME_STOCK_MENU_SEARCH),
    GNOMEUIINFO_ITEM_STOCK (N_("Spell"), 
			    N_("Check word spelling"), 
			    spell_cb, GNOME_STOCK_MENU_SPELLCHECK),
#ifdef HAVE_GNOME_PRINT
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_PRINT_ITEM (print_cb, NULL),
    GNOMEUIINFO_MENU_PRINT_SETUP_ITEM (print_setup_cb, NULL),
#endif /* HAVE_GNOME_PRINT */
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_EXIT_ITEM (exit_cb, NULL),
    GNOMEUIINFO_END
};

#ifdef HAVE_GNOME_PRINT
#define EXIT_FILE_MENU_ITEM 6
#else
#define EXIT_FILE_MENU_ITEM 3
#endif

static GnomeUIInfo applet_close_item = 
  GNOMEUIINFO_MENU_CLOSE_ITEM (close_cb, NULL);

static GnomeUIInfo edit_menu_uiinfo[] = {
    GNOMEUIINFO_MENU_CUT_ITEM (cut_cb, NULL),
    GNOMEUIINFO_MENU_COPY_ITEM (copy_cb, NULL),
    GNOMEUIINFO_MENU_PASTE_ITEM (paste_cb, NULL),
    GNOMEUIINFO_MENU_CLEAR_ITEM (clear_cb, NULL),
    GNOMEUIINFO_MENU_SELECT_ALL_ITEM (select_all_cb, NULL),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_FIND_ITEM (find_cb, NULL),
    GNOMEUIINFO_MENU_FIND_AGAIN_ITEM (find_again_cb, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu_uiinfo[] = {
    GNOMEUIINFO_MENU_PREFERENCES_ITEM (preferences_cb, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo help_menu_uiinfo[] = {
    GNOMEUIINFO_HELP(APPNAME),
    GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo menubar_uiinfo[] = {
    GNOMEUIINFO_MENU_FILE_TREE (file_menu_uiinfo),
    GNOMEUIINFO_MENU_EDIT_TREE (edit_menu_uiinfo),
    GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu_uiinfo),
    GNOMEUIINFO_MENU_HELP_TREE (help_menu_uiinfo),
    GNOMEUIINFO_END
};

static GnomeUIInfo toolbar_uiinfo[] = {
    GNOMEUIINFO_ITEM_STOCK (N_("Look up"), 
			    N_("Look up word in dictionary"), 
			    lookup_cb, GNOME_STOCK_PIXMAP_SEARCH),
    GNOMEUIINFO_ITEM_STOCK (N_("Spell"), 
			    N_("Check word spelling"), 
			    spell_cb, GNOME_STOCK_PIXMAP_SPELLCHECK),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK (N_("Cut"), 
			    N_("Cut to Clipboard"), 
			    cut_cb, GNOME_STOCK_PIXMAP_CUT),
    GNOMEUIINFO_ITEM_STOCK (N_("Copy"), 
			    N_("Copy to Clipboard"), 
			    copy_cb, GNOME_STOCK_PIXMAP_COPY),
    GNOMEUIINFO_ITEM_STOCK (N_("Paste"), 
			    N_("Paste Clipboard Contents"), 
			    paste_cb, GNOME_STOCK_PIXMAP_PASTE),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK (N_("Settings"), 
			  N_("Server to use, default search strategy, fonts"), 
			    preferences_cb, GNOME_STOCK_PIXMAP_PREFERENCES),
    GNOMEUIINFO_END
};

GtkWidget *gdict_app_create (void) {
    GtkWidget *dock;
    GtkWidget *vbox, *hbox;
    GtkAdjustment *vscrolladj;
    GtkWidget *defbox_vscrollbar;
    GtkTooltips *tooltips;
  
    tooltips = gtk_tooltips_new ();
  
    gdict_app = gnome_app_new ("GDict", "GDict");
    gtk_widget_set_usize (gdict_app, 600, 330);
  
    dock = GNOME_APP (gdict_app)->dock;
    gtk_widget_show (dock);

    if (gdict_applet_toggle)  /* quick hack */
        memcpy (&file_menu_uiinfo[EXIT_FILE_MENU_ITEM],
		&applet_close_item, sizeof(GnomeUIInfo));

    gnome_app_create_menus (GNOME_APP (gdict_app), menubar_uiinfo);
    gnome_app_create_toolbar (GNOME_APP (gdict_app), toolbar_uiinfo);
    
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gnome_app_set_contents (GNOME_APP (gdict_app), vbox);

    gnome_word_entry = gnome_entry_new ("Word entry");
    gtk_widget_show (gnome_word_entry);
    gtk_box_pack_start (GTK_BOX (vbox), gnome_word_entry, FALSE, FALSE, 0);
    GTK_WIDGET_SET_FLAGS (gnome_word_entry, GTK_CAN_FOCUS);
    gtk_widget_grab_focus (gnome_word_entry);
    gnome_entry_set_max_saved (GNOME_ENTRY (gnome_word_entry), 50);
    word_entry = gnome_entry_gtk_entry(GNOME_ENTRY (gnome_word_entry));
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

    vscrolladj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 0, 0, 0, 0));
    defbox_vscrollbar = gtk_vscrollbar_new (vscrolladj);
    gtk_widget_show (defbox_vscrollbar);
  
    defbox = GDICT_DEFBOX (gdict_defbox_new ());
    gtk_signal_connect (GTK_OBJECT (defbox), "word_lookup_start",
                        GTK_SIGNAL_FUNC (def_lookup_start_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "word_lookup_done",
                        GTK_SIGNAL_FUNC (def_lookup_done_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "word_not_found",
                        GTK_SIGNAL_FUNC (def_not_found_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "substr_not_found",
                        GTK_SIGNAL_FUNC (def_substr_not_found_cb), defbox);
    gtk_signal_connect (GTK_OBJECT (defbox), "socket_error",
                        GTK_SIGNAL_FUNC (socket_error_cb), defbox);
  
    gtk_text_set_adjustments (GTK_TEXT (defbox), NULL, vscrolladj);
    gtk_widget_show (GTK_WIDGET (defbox));
    gtk_tooltips_set_tip (tooltips, GTK_WIDGET (defbox), 
                          _("Word definition"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (defbox), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), defbox_vscrollbar, FALSE, TRUE, 0);

    gdict_appbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_ALWAYS);
    gtk_widget_show (gdict_appbar);
    gnome_app_set_statusbar (GNOME_APP (gdict_app), gdict_appbar);
    
    if (!gdict_applet_toggle)
      gtk_signal_connect (GTK_OBJECT (gdict_app), "delete_event",
                          GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    else
      gtk_signal_connect (GTK_OBJECT (gdict_app), "delete_event",
                          GTK_SIGNAL_FUNC (gtk_widget_hide), NULL);
    
    gtk_signal_connect (GTK_OBJECT (word_entry), "activate",
                        GTK_SIGNAL_FUNC(lookup_cb), NULL);
    gtk_signal_connect (GTK_OBJECT (gdict_appbar), "user_response",
                        GTK_SIGNAL_FUNC(prompt_response_cb), NULL);
    
    return gdict_app;
}
