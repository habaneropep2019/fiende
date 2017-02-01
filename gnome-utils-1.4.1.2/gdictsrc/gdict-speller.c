/* $Id: gdict-speller.c,v 1.4.2.1 2001/10/03 03:38:06 jfleck Exp $ */

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
#ifdef HAVE_GNOME_PRINT
#  include <libgnomeprint/gnome-print.h>
#  include <libgnomeprint/gnome-printer-dialog.h>
#  include <libgnomeprint/gnome-printer-profile.h>
#  include <math.h>
#endif

#include "gdict-speller.h"
#include "gdict-app.h"
#include "gdict-pref.h"

enum {
    WORD_LOOKUP_START_SIGNAL,
    WORD_LOOKUP_DONE_SIGNAL,
    WORD_NOT_FOUND_SIGNAL,
    SOCKET_ERROR_SIGNAL,
    LAST_SIGNAL
};

static gint gdict_speller_signals[LAST_SIGNAL] = { 0, 0, 0 };

static GnomeDialogClass *parent_class;

static void gdict_speller_init (GDictSpeller *speller);
static void gdict_speller_class_init (GDictSpellerClass *class);

static void speller_add_word  (GDictSpeller *speller, gchar *word);
static void speller_add_strat (GDictSpeller *speller, gchar *strat, gchar *desc);

static void spell_error_cb        (dict_command_t *command, DictStatusCode code, 
                                   gchar *message, gpointer data);
static void spell_word_status_cb  (dict_command_t *command, DictStatusCode code, 
                                   int num_found, gpointer data);
static void spell_strat_status_cb (dict_command_t *command, DictStatusCode code, 
                                   int num_found, gpointer data);
static void speller_set_strat_cb  (GtkWidget *widget, gpointer data);
static void spell_word_data_cb    (dict_command_t *command, dict_res_t *res,
                                   gpointer data);
static void spell_strat_data_cb   (dict_command_t *command, dict_res_t *res,
                                   gpointer data);
static void speller_spell_cb      (GtkWidget *widget, gpointer data);
static void speller_lookup_cb     (GtkWidget *widget, gpointer data);
static void speller_close_cb      (GtkWidget *widget, gpointer data);
static void word_select_cb        (GtkCList *clist, gint row, gint column,
                                   GdkEventButton *event, gpointer data);

/* gdict_speller_get_type
 *
 * Register the GDictSpeller type with Gtk's type system if necessary and
 * return the type identifier code
 */

guint 
gdict_speller_get_type (void) {
    static guint gdict_speller_type = 0;
    
    if (!gdict_speller_type) {
        GtkTypeInfo gdict_speller_info = {
            "GDictSpeller",
            sizeof (GDictSpeller),
            sizeof (GDictSpellerClass),
            (GtkClassInitFunc) gdict_speller_class_init,
            (GtkObjectInitFunc) gdict_speller_init,
            (GtkArgSetFunc) NULL,
            (GtkArgGetFunc) NULL
        };
        
        gdict_speller_type = 
            gtk_type_unique (gnome_dialog_get_type (), &gdict_speller_info);
    }
    
    return gdict_speller_type;
}

/* gdict_speller_init
 *
 * Initialises an instance of a GDictSpeller object
 */

static void 
gdict_speller_init (GDictSpeller *speller) {
    GtkWidget *label, *scrolled_win;
    
    speller->context = NULL;
    speller->get_strat_cmd = NULL;
    speller->spell_cmd = NULL;
    speller->strat = gdict_pref.dfl_strat;
    
    speller->table = GTK_TABLE (gtk_table_new (2, 3, FALSE));
    gtk_table_set_row_spacings (speller->table, 5);
    gtk_table_set_col_spacings (speller->table, 5);
    
    label = gtk_label_new (_("Word:"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_table_attach_defaults (speller->table, label, 0, 1, 0, 1);
    
    speller->word_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_table_attach_defaults (speller->table, 
		               GTK_WIDGET (speller->word_entry), 1, 2, 0, 1);
    
    label = gtk_label_new (_("Search Strategy:"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_table_attach_defaults (speller->table, label, 0, 1, 1, 2);
    
    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                    GTK_POLICY_NEVER,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize (scrolled_win, 400, 250);
    
    gtk_table_attach (speller->table, scrolled_win, 0, 2, 2, 3, 
                      GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 5);
    
    speller->word_sel = GTK_CLIST (gtk_clist_new (1));
    gtk_clist_column_titles_hide (speller->word_sel);
    gtk_clist_set_selection_mode (speller->word_sel, GTK_SELECTION_SINGLE);
    gtk_signal_connect (GTK_OBJECT (speller->word_sel), "select-row",
                        GTK_SIGNAL_FUNC (word_select_cb), speller);
    gtk_container_add (GTK_CONTAINER (scrolled_win), 
                       GTK_WIDGET (speller->word_sel));
    
    gnome_dialog_append_buttons (GNOME_DIALOG (speller),
                                 _("Find Words..."),
                                 _("Look up Word..."),
                                 GNOME_STOCK_BUTTON_CLOSE,
                                 NULL);
    
    gnome_dialog_set_default (GNOME_DIALOG (speller), 0);
    
    gnome_dialog_button_connect (GNOME_DIALOG (speller), 0,
                        GTK_SIGNAL_FUNC (speller_spell_cb), speller);
    gnome_dialog_button_connect (GNOME_DIALOG (speller), 1,
                        GTK_SIGNAL_FUNC (speller_lookup_cb), speller);
    gnome_dialog_button_connect (GNOME_DIALOG (speller), 2,
                        GTK_SIGNAL_FUNC (speller_close_cb), speller);
    gtk_window_set_title (GTK_WINDOW (speller), _("Spell checker"));
    
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (speller)->vbox),
                        GTK_WIDGET (speller->table), 5, TRUE, TRUE);
    gtk_widget_show_all (GTK_WIDGET (speller->table));
}

/* gdict_speller_class_init
 *
 * Initialises a structure describing the GDictSpeller class; sets up signals
 * for speller events in the Gtk signal management system
 */

static void 
gdict_speller_class_init (GDictSpellerClass *class) {
    GtkObjectClass *object_class;
    
    object_class = GTK_OBJECT_CLASS (class);

    gdict_speller_signals[WORD_LOOKUP_START_SIGNAL] =
        gtk_signal_new ("word_lookup_start", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, word_lookup_start),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_speller_signals[WORD_LOOKUP_DONE_SIGNAL] =
        gtk_signal_new ("word_lookup_done", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, word_lookup_done),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_speller_signals[WORD_NOT_FOUND_SIGNAL] =
        gtk_signal_new ("word_not_found", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, word_not_found),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_speller_signals[SOCKET_ERROR_SIGNAL] =
        gtk_signal_new ("socket_error", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictSpellerClass, socket_error),
                        gtk_marshal_NONE__STRING, GTK_TYPE_NONE, 1,
                        GTK_TYPE_STRING);
    
    gtk_object_class_add_signals (object_class, gdict_speller_signals,
                                  LAST_SIGNAL);
    
    parent_class = gtk_type_class (gnome_dialog_get_type ());

    class->word_lookup_done = NULL;
    class->word_not_found = NULL;

    object_class->destroy = (void (*) (GtkObject *)) gdict_speller_destroy;
}

/* gdict_speller_new
 *
 * Creates a new GDictSpeller object
 */

GtkWidget *
gdict_speller_new (dict_context_t *context) {
    GDictSpeller *speller;
    
    g_return_val_if_fail (context != NULL, NULL);
    
    speller = GDICT_SPELLER (gtk_type_new (gdict_speller_get_type ()));
    speller->context = context;
    
    gdict_speller_reset_strat (speller);
    
    return GTK_WIDGET (speller);
}

/* gdict_speller_destroy
 *
 * Destroys a speller dialog
 */

void
gdict_speller_destroy (GDictSpeller *speller) {
    g_free (speller->database);
    dict_command_destroy (speller->get_strat_cmd);
    dict_command_destroy (speller->spell_cmd);
    GTK_OBJECT_CLASS (parent_class)->destroy (GTK_OBJECT (speller));
}

/* gdict_speller_lookup
 *
 * Sends the command to the server to commence looking up matches for a word
 * of a word and sets the callbacks so that the definition will be displayed
 * in this speller
 *
 * Returns 0 on success and -1 on command invocation error
 */

gint 
gdict_speller_lookup (GDictSpeller *speller, gchar *text) {
    g_return_val_if_fail (speller != NULL, -1);
    g_return_val_if_fail (IS_GDICT_SPELLER (speller), -1);
    g_return_val_if_fail (text != NULL, -1);
    
    while (isspace (*text)) text++;
    
    if (*text == '\0')
        return 0;
    
    gtk_signal_emit (GTK_OBJECT (speller), 
                     gdict_speller_signals[WORD_LOOKUP_START_SIGNAL]);
    
    gdict_speller_clear (speller);
    
    gtk_entry_set_text (speller->word_entry, text);
    gtk_editable_select_region (GTK_EDITABLE (speller->word_entry), 
                                0, strlen (text));
    
    if (speller->database) g_free (speller->database);
    
    speller->database = g_strdup (gdict_pref.database);
    
    speller->spell_cmd = 
        dict_match_command_new (speller->database, speller->strat, text);
    speller->spell_cmd->error_notify_cb = spell_error_cb;
    speller->spell_cmd->status_notify_cb = spell_word_status_cb;
    speller->spell_cmd->data_notify_cb = spell_word_data_cb;
    speller->spell_cmd->user_data = speller;
    
    if (dict_command_invoke (speller->spell_cmd, speller->context) < 0)
      return -1;
    
    return 0;
}

/* gdict_speller_clear
 *
 * Clears the text in a speller and eliminates the current command structure
 */

void 
gdict_speller_clear (GDictSpeller *speller) {
    g_return_if_fail (speller != NULL);
    g_return_if_fail (IS_GDICT_SPELLER (speller));
    
    gtk_entry_set_text (speller->word_entry, "");
    gtk_clist_clear (speller->word_sel);
    speller->current_word = NULL;
    
    if (speller->spell_cmd) {
        dict_command_destroy (speller->spell_cmd);
        speller->spell_cmd = NULL;
    }
}

/* gdict_speller_reset
 *
 * Reinvokes the search with a new context and preferences
 */

void
gdict_speller_reset (GDictSpeller *speller, dict_context_t *context) {
    gchar *word;
    
    /* If we have a new context, we cannot trust that the search strategy
     * we were using will still be present
     */
    
    if (context != speller->context) {
        speller->context = context;
        speller->strat = gdict_pref.dfl_strat;
        gdict_speller_reset_strat (speller);
    }
    
    /* Re-invoke current query only if there is a new database specified */
    
    if (context != speller->context ||
        strcmp (speller->database, gdict_pref.database))
    {
        if (speller->spell_cmd) {
            word = g_strdup (speller->spell_cmd->search_term);
            dict_command_destroy (speller->spell_cmd);
            speller->spell_cmd = NULL;
            gdict_speller_lookup (speller, word);
            g_free (word);
        }
    }
}

/* gdict_speller_get_word
 *
 * Returns the word defined in the speller, if any
 */

gchar *
gdict_speller_get_word (GDictSpeller *speller) {
    g_return_val_if_fail (speller != NULL, NULL);
    g_return_val_if_fail (IS_GDICT_SPELLER (speller), NULL);
    
    return speller->spell_cmd ? speller->spell_cmd->search_term : NULL;
}

/* gdict_speller_reset_strat
 *
 * Resets the list of strategies
 */

void
gdict_speller_reset_strat (GDictSpeller *speller) {
    GtkWidget *error_label, *alignment;

    if (speller->get_strat_cmd)
	dict_command_destroy (speller->get_strat_cmd);
    
    if (speller->strat_sel) {
        gtk_option_menu_remove_menu (speller->strat_sel);
        gtk_widget_destroy (GTK_WIDGET (speller->strat_sel));
        gtk_widget_show_all (GTK_WIDGET (speller->table));
        speller->strat_sel = NULL;
    }
    
    speller->strat_list = GTK_MENU (gtk_menu_new ());
    
    speller->get_strat_cmd = dict_show_strat_command_new ();
    speller->get_strat_cmd->error_notify_cb = spell_error_cb;
    speller->get_strat_cmd->data_notify_cb = spell_strat_data_cb;
    speller->get_strat_cmd->status_notify_cb = spell_strat_status_cb;
    speller->get_strat_cmd->user_data = speller;
    speller->strat_idx = 0;
    
    if (dict_command_invoke (speller->get_strat_cmd,
			     speller->context) == -1) 
    {
	/* Could not look up search strategies, so just display a
	 * label; FIXME: Memory leak
	 */
	error_label = gtk_label_new (_("Cannot connect to server"));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (error_label));
        gtk_table_attach_defaults (speller->table, alignment, 1, 2, 1, 2);
        gtk_widget_show_all (GTK_WIDGET (speller->table));
    }
}

/* spell_error_cb
 *
 * Callback invoked when there was an error in the last query
 */

static void
spell_error_cb (dict_command_t *command, DictStatusCode code,
                gchar *message, gpointer data)
{
    GtkWindow *speller;
    gchar *string;
    
    speller = GTK_WINDOW (data);
    
    if (code != DICT_SOCKET_ERROR) {
        string = g_strdup_printf (_("Error invoking query: %s"), message);
        gnome_error_dialog_parented (string, speller);
        
        if (command->cmd == C_MATCH)
          gtk_signal_emit (GTK_OBJECT (speller), 
                           gdict_speller_signals[WORD_LOOKUP_DONE_SIGNAL]);
    }
    else {
        gtk_signal_emit (GTK_OBJECT (speller),
                         gdict_speller_signals[SOCKET_ERROR_SIGNAL], message);
    }
}

/* speller_add_word
 *
 * Adds a word to the word list
 */

static void 
speller_add_word (GDictSpeller *speller, gchar *word) {
    gtk_clist_append (speller->word_sel, &word);
    
    if (!speller->current_word) speller->current_word = word;
}

/* speller_set_strat_cb
 *
 * Sets the current search strategy to the one indicated
 */

static void
speller_set_strat_cb (GtkWidget *widget, gpointer data) {
    GDictSpeller *speller;
    gchar *strat;
    
    speller = GDICT_SPELLER (data);
    strat = gtk_object_get_data (GTK_OBJECT (widget), "strat_name");
    speller->strat = strat;
}

/* speller_add_strat
 *
 * Adds a search strategy to the search strategy list
 */

static void
speller_add_strat (GDictSpeller *speller, gchar *strat, gchar *desc) {
    GtkWidget *menu_item;
    
    menu_item = gtk_menu_item_new_with_label (desc);
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", 
                        GTK_SIGNAL_FUNC (speller_set_strat_cb), speller);
    gtk_object_set_data (GTK_OBJECT (menu_item), "strat_name", strat);
    gtk_widget_show (menu_item);
    gtk_menu_append (speller->strat_list, menu_item);
    
    if (!strcmp (speller->strat, strat))
      gtk_menu_set_active (speller->strat_list, speller->strat_idx);
    speller->strat_idx++;
}

/* spell_word_data_cb
 *
 * Callback used when a new word has arrived over the link
 */

static void 
spell_word_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictSpeller *speller;
    
    speller = GDICT_SPELLER (data);
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;
    speller_add_word (speller, res->desc);
}

/* spell_strat_data_cb
 *
 * Callback used when a new strategy definition has arrived over the link
 */

static void
spell_strat_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictSpeller *speller;
    
    speller = GDICT_SPELLER (data);
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;
    speller_add_strat (speller, res->name, res->desc);
}

/* spell_word_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
spell_word_status_cb (dict_command_t *command, DictStatusCode code, 
                      int num_found, gpointer data)
{
    GDictSpeller *speller;
    
    speller = GDICT_SPELLER (data);

    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;
    
    if (code == DICT_STATUS_OK)
      gtk_signal_emit (GTK_OBJECT (speller), 
                       gdict_speller_signals[WORD_LOOKUP_DONE_SIGNAL]);
    else if (code == DICT_STATUS_NO_MATCH)
      gtk_signal_emit (GTK_OBJECT (speller), 
                       gdict_speller_signals[WORD_NOT_FOUND_SIGNAL]);
}

/* spell_strat_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
spell_strat_status_cb (dict_command_t *command, DictStatusCode code, 
                       int num_found, gpointer data)
{
    GDictSpeller *speller;
    GtkWidget *alignment;
    
    speller = GDICT_SPELLER (data);
    
    if (!GTK_WIDGET_VISIBLE (GTK_WIDGET (speller))) return;

    if (code == DICT_STATUS_OK) {
        speller->strat_sel = GTK_OPTION_MENU (gtk_option_menu_new ());
        gtk_option_menu_set_menu (speller->strat_sel, 
                                  GTK_WIDGET (speller->strat_list));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (speller->strat_sel));
        gtk_table_attach_defaults (speller->table, alignment, 1, 2, 1, 2);
        gtk_widget_show_all (GTK_WIDGET (speller->table));
    }
}

/* speller_spell_cb
 *
 * Looks up the currently selected word
 */

static void
speller_spell_cb (GtkWidget *widget, gpointer data) {
    GDictSpeller *speller;
    gchar *text;
    
    g_return_if_fail (data != NULL);
    
    speller = GDICT_SPELLER (data);
    text = gtk_entry_get_text (speller->word_entry);
    gtk_editable_select_region (GTK_EDITABLE (speller->word_entry), 0,
                                strlen (text));
    gdict_speller_lookup (speller, text);
}

/* speller_lookup_cb
 *
 * Looks up the currently selected word
 */

static void
speller_lookup_cb (GtkWidget *widget, gpointer data) {
    GDictSpeller *speller;
    
    g_return_if_fail (data != NULL);
    
    speller = GDICT_SPELLER (data);
    
    if (!speller->current_word) return;
    
    gtk_entry_set_text (GTK_ENTRY (word_entry), speller->current_word);
    gtk_editable_select_region (GTK_EDITABLE (word_entry), 0,
                                strlen (speller->current_word));
    gdict_app_do_lookup (speller->current_word);
}

/* speller_close_cb
 *
 * Closes the speller dialog
 */

static void
speller_close_cb (GtkWidget *widget, gpointer data) {
    GDictSpeller *speller;
    
    g_return_if_fail (data != NULL);
    
    speller = GDICT_SPELLER (data);
    gnome_dialog_close (GNOME_DIALOG (speller));
}

/* word_select_cb
 *
 * Callback invoked when a word is selected in the speller
 */

static void
word_select_cb (GtkCList *clist, gint row, gint column, GdkEventButton *event,
                gpointer data)
{
    GDictSpeller *speller;
    GList *current_row;
    
    if (event->button == 1) {
        speller = GDICT_SPELLER (data);
        current_row = g_list_nth (speller->spell_cmd->res_list, row);
        speller->current_word = ((dict_res_t *) current_row->data)->desc;
    }
}
