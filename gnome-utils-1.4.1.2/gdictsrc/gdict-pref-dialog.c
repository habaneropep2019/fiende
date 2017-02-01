/* $Id: gdict-pref-dialog.c,v 1.10 2001/06/22 22:15:42 kmaraas Exp $ */

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

#include "gdict-pref-dialog.h"
#include "gdict-app.h"
#include "gdict-applet.h"

enum {
    APPLY_CHANGES_SIGNAL,
    SOCKET_ERROR_SIGNAL,
    LAST_SIGNAL
};

static gchar *typeface_sel_names[NUM_TYPEFACES] = {
    N_("Headword:"),
    N_("Sub-number:"),
    N_("Pronunciation:"),
    N_("Etymology:"),
    N_("Part of speech:"),
    N_("Example:"),
    N_("Main body:"),
    N_("Cross-reference:")
};

#define NUM_TYPEFACE_ROWS 4
#define NUM_TYPEFACE_COLS 2

static gint gdict_pref_dialog_signals[LAST_SIGNAL] = { 0 };

static void gdict_pref_dialog_init (GDictPrefDialog *pref_dialog);
static void gdict_pref_dialog_class_init (GDictPrefDialogClass *class);

static void pref_dialog_add_db      (GDictPrefDialog *pref_dialog, 
                                     gchar *db, gchar *desc);
static void pref_dialog_add_strat   (GDictPrefDialog *pref_dialog, 
                                     gchar *strat, gchar *desc);
static void pref_dialog_reset_db    (GDictPrefDialog *pref_dialog);
static void pref_dialog_reset_strat (GDictPrefDialog *pref_dialog);
static gint pref_dialog_update_pref (GDictPrefDialog *pref_dialog,
                                     gboolean save_prefs);

static void pref_set_strat_cb       (GtkWidget *widget, gpointer data);
static void pref_set_db_cb          (GtkWidget *widget, gpointer data);

static void pref_error_cb           (dict_command_t *command, 
                                     DictStatusCode code, 
                                     gchar *message, gpointer data);
static void pref_status_cb          (dict_command_t *command, 
                                     DictStatusCode code, 
                                     int num_found, gpointer data);
static void pref_data_cb            (dict_command_t *command, dict_res_t *res,
                                     gpointer data);

static void pref_dialog_ok_cb       (GtkWidget *widget, gpointer data);
static void pref_dialog_apply_cb    (GtkWidget *widget, gpointer data);
static void pref_dialog_close_cb    (GtkWidget *widget, gpointer data);

/* gdict_pref_dialog_get_type
 *
 * Register the GDictPrefDialog type with Gtk's type system if necessary and
 * return the type identifier code
 */

guint 
gdict_pref_dialog_get_type (void) {
    static guint gdict_pref_dialog_type = 0;
    
    if (!gdict_pref_dialog_type) {
        GtkTypeInfo gdict_pref_dialog_info = {
            "GDictPrefDialog",
            sizeof (GDictPrefDialog),
            sizeof (GDictPrefDialogClass),
            (GtkClassInitFunc) gdict_pref_dialog_class_init,
            (GtkObjectInitFunc) gdict_pref_dialog_init,
            (GtkArgSetFunc) NULL,
            (GtkArgGetFunc) NULL
        };
        
        gdict_pref_dialog_type = 
            gtk_type_unique (gnome_dialog_get_type (), &gdict_pref_dialog_info);
    }
    
    return gdict_pref_dialog_type;
}

/* gdict_pref_dialog_init
 *
 * Initialises an instance of a GDictPrefDialog object
 */

static void 
gdict_pref_dialog_init (GDictPrefDialog *pref_dialog) {
    GtkWidget *label, *alignment;
    GtkWidget *vbox, *table, *table1;
    GtkWidget *notebook;
    gchar *port_str;
    gint i;
    
    pref_dialog->context = NULL;
    pref_dialog->get_db_cmd = NULL;
    pref_dialog->get_strat_cmd = NULL;
    
    pref_dialog->database = gdict_pref.database;
    pref_dialog->dfl_strat = gdict_pref.dfl_strat;
    
    gtk_window_set_title (GTK_WINDOW (pref_dialog), _("Preferences"));
    gtk_window_set_policy (GTK_WINDOW (pref_dialog), FALSE, FALSE, FALSE);

    notebook = gtk_notebook_new ();
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (pref_dialog)->vbox), notebook, 
                        TRUE, TRUE, 0);
    
    pref_dialog->table = GTK_TABLE (gtk_table_new (5, 2, FALSE));
    gtk_container_add (GTK_CONTAINER (notebook), 
                       GTK_WIDGET (pref_dialog->table));
    gtk_container_set_border_width (GTK_CONTAINER (pref_dialog->table), 8);
    gtk_table_set_row_spacings (pref_dialog->table, 6);
    gtk_table_set_col_spacings (pref_dialog->table, 6);

    alignment = gtk_alignment_new(0, 0.5, 0, 1);
    gtk_table_attach_defaults (pref_dialog->table, alignment, 0, 2, 2, 3);

    pref_dialog->smart_lookup_btn = 
        GTK_CHECK_BUTTON (gtk_check_button_new_with_label (_("Smart lookup")));
    gtk_container_add (GTK_CONTAINER (alignment), 
                       GTK_WIDGET (pref_dialog->smart_lookup_btn));

    pref_dialog->server_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_table_attach_defaults (pref_dialog->table, 
                               GTK_WIDGET (pref_dialog->server_entry), 
                               1, 2, 0, 1);
    
    label = gtk_label_new (_("Server:"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, label, 0, 1, 0, 1);

    label = gtk_label_new (_("Port:"));
    gtk_table_attach_defaults (pref_dialog->table, label, 0, 1, 1, 2);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);

    pref_dialog->port_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_table_attach_defaults (pref_dialog->table, 
                               GTK_WIDGET (pref_dialog->port_entry), 
                               1, 2, 1, 2);
    
    label = gtk_label_new (_("Database:"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, label, 0, 1, 3, 4);
    pref_dialog->db_list = GTK_MENU (gtk_menu_new ());
    gtk_widget_show (GTK_WIDGET (pref_dialog->db_list));
    
    label = gtk_label_new (_("Default strategy:"));
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
    gtk_table_attach_defaults (pref_dialog->table, label, 0, 1, 4, 5);
    pref_dialog->strat_list = GTK_MENU (gtk_menu_new ());
    gtk_widget_show (GTK_WIDGET (pref_dialog->strat_list));
    
    label = gtk_label_new (_("Server"));
    gtk_notebook_set_tab_label
	(GTK_NOTEBOOK (notebook), 
	 gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), 
	 label);
    
    gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
    
    table1 = gtk_table_new (1, 1, FALSE);
    alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
    table = gtk_table_new (NUM_TYPEFACE_ROWS + 2, NUM_TYPEFACE_COLS, FALSE);
    gtk_table_attach (GTK_TABLE (table1), table, 0, 1, 0, 1,
                      0, 0, 10, 10);
    gtk_container_add (GTK_CONTAINER (alignment), table1);
    gtk_container_add (GTK_CONTAINER (notebook), alignment);
    
    gtk_table_set_row_spacings (GTK_TABLE (table), 4);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);
    
    for (i = 0; i < NUM_TYPEFACES; i++) {
        label = gtk_label_new (_(typeface_sel_names[i]));
        gtk_table_attach (GTK_TABLE (table), label, 
                          (3 * (i / NUM_TYPEFACE_ROWS)), 
                          (3 * (i / NUM_TYPEFACE_ROWS)) + 1,
                          (i % NUM_TYPEFACE_ROWS), 
                          (i % NUM_TYPEFACE_ROWS) + 1, GTK_FILL, 0, 5, 0);
        gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
        
        pref_dialog->font_pickers[i] = 
            GNOME_FONT_PICKER (gnome_font_picker_new ());
        gtk_table_attach (GTK_TABLE (table), 
                          GTK_WIDGET (pref_dialog->font_pickers[i]),
                          (3 * (i / NUM_TYPEFACE_ROWS)) + 1, 
                          (3 * (i / NUM_TYPEFACE_ROWS)) + 2,
                          (i % NUM_TYPEFACE_ROWS), 
                          (i % NUM_TYPEFACE_ROWS) + 1, 0, GTK_FILL, 0, 0);
        
        pref_dialog->color_pickers[i] = 
            GNOME_COLOR_PICKER (gnome_color_picker_new ());
        gtk_table_attach (GTK_TABLE (table), 
                          GTK_WIDGET (pref_dialog->color_pickers[i]),
                          (3 * (i / NUM_TYPEFACE_ROWS)) + 2, 
                          (3 * (i / NUM_TYPEFACE_ROWS)) + 3,
                          (i % NUM_TYPEFACE_ROWS), 
                          (i % NUM_TYPEFACE_ROWS) + 1, 0, GTK_FILL, 0, 0);
    }
    
    gtk_widget_show_all (table1);
    
    label = gtk_label_new (_("Fonts"));
    gtk_notebook_set_tab_label
	(GTK_NOTEBOOK (notebook), 
	 gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), 
	 label);
    
#if 0
    if (gdict_applet_toggle) {
        vbox = gtk_vbox_new (FALSE, 2);
        gtk_container_add (GTK_CONTAINER (notebook), vbox);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
        
        alignment = gtk_alignment_new (0, 0.5, 0, 1);
        gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);

        pref_dialog->applet_handle_btn = 
            GTK_CHECK_BUTTON (gtk_check_button_new_with_label
			      (_("Display handle bar")));
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (pref_dialog->applet_handle_btn));
        
        label = gtk_label_new (_("You have to restart GDict for these\nsettings to become active."));
        gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
        gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
        gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
        
        gtk_widget_show_all (vbox);
        
        label = gtk_label_new (_("Applet"));
        gtk_notebook_set_tab_label
	    (GTK_NOTEBOOK (notebook), 
	     gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 2), 
	     label);
    }
#endif

    gnome_dialog_append_button (GNOME_DIALOG (pref_dialog),
				GNOME_STOCK_BUTTON_OK);
    gnome_dialog_append_button (GNOME_DIALOG (pref_dialog),
				GNOME_STOCK_BUTTON_APPLY);
    gnome_dialog_append_button (GNOME_DIALOG (pref_dialog),
				GNOME_STOCK_BUTTON_CANCEL);

    gnome_dialog_button_connect (GNOME_DIALOG (pref_dialog), 0,
                                 GTK_SIGNAL_FUNC (pref_dialog_ok_cb),
                                 pref_dialog);
    gnome_dialog_button_connect (GNOME_DIALOG (pref_dialog), 1,
                                 GTK_SIGNAL_FUNC (pref_dialog_apply_cb),
                                 pref_dialog);
    gnome_dialog_button_connect (GNOME_DIALOG (pref_dialog), 2,
                                 GTK_SIGNAL_FUNC (pref_dialog_close_cb),
                                 pref_dialog);
    
    gnome_dialog_set_default (GNOME_DIALOG (pref_dialog), 1);
    
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->server_entry),
			gdict_pref.server);
    port_str = g_strdup_printf("%d", (int) gdict_pref.port);
    gtk_entry_set_text (GTK_ENTRY (pref_dialog->port_entry), port_str);
    g_free(port_str);
    gtk_toggle_button_set_active
	(GTK_TOGGLE_BUTTON (pref_dialog->smart_lookup_btn), gdict_pref.smart);

#if 0
    if (gdict_applet_toggle)
        gtk_toggle_button_set_active
	    (GTK_TOGGLE_BUTTON(pref_dialog->applet_handle_btn),
	     gdict_pref.applet_handle);
#endif

    for (i = 0; i < NUM_TYPEFACES; i++) {
        gnome_font_picker_set_font_name (pref_dialog->font_pickers[i], 
                                         gdict_pref.typefaces[i].font_name);
        gnome_color_picker_set_i16 (pref_dialog->color_pickers[i], 
                                    gdict_pref.typefaces[i].color->red, 
                                    gdict_pref.typefaces[i].color->green, 
                                    gdict_pref.typefaces[i].color->blue, 0);
    }
    
    gtk_widget_show_all (notebook);
}

/* gdict_pref_dialog_class_init
 *
 * Initialises a structure describing the GDictPrefDialog class; sets
 * up signals for pref_dialog events in the Gtk signal management
 * system
 */

static void 
gdict_pref_dialog_class_init (GDictPrefDialogClass *class) 
{
    GtkObjectClass *object_class;
    
    object_class = (GtkObjectClass *) class;
    
    gdict_pref_dialog_signals[APPLY_CHANGES_SIGNAL] =
        gtk_signal_new ("apply_changes", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictPrefDialogClass,
					   apply_changes),
                        gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
    
    gdict_pref_dialog_signals[SOCKET_ERROR_SIGNAL] =
        gtk_signal_new ("socket_error", GTK_RUN_FIRST, object_class->type,
                        GTK_SIGNAL_OFFSET (GDictPrefDialogClass, socket_error),
                        gtk_marshal_NONE__STRING, GTK_TYPE_NONE, 1,
                        GTK_TYPE_STRING);
    
    gtk_object_class_add_signals (object_class, gdict_pref_dialog_signals,
                                  LAST_SIGNAL);
    
    object_class->destroy = (void (*) (GtkObject *)) gdict_pref_dialog_destroy;
    
    class->apply_changes = NULL;
}

/* gdict_pref_dialog_new
 *
 * Creates a new GDictPrefDialog object
 */

GtkWidget *
gdict_pref_dialog_new (dict_context_t *context) 
{
    GDictPrefDialog *pref_dialog;
    
    /* g_return_val_if_fail (context != NULL, NULL); */
    /* the above is commented out because it prints ugly debug messages
     * to the console */
    if (context == NULL)
	    return NULL;
    
    pref_dialog =
	GDICT_PREF_DIALOG (gtk_type_new (gdict_pref_dialog_get_type ()));
    pref_dialog->context = context;
    pref_dialog_reset_db (pref_dialog);
    pref_dialog_reset_strat (pref_dialog);

    return GTK_WIDGET (pref_dialog);
}

/* gdict_pref_dialog_destroy
 *
 * Destroys a pref_dialog dialog
 */

void
gdict_pref_dialog_destroy (GDictPrefDialog *pref_dialog) 
{
    dict_command_destroy (pref_dialog->get_db_cmd);
    dict_command_destroy (pref_dialog->get_strat_cmd);
}

/* pref_set_db_cb
 *
 * Sets the current search strategy to the one indicated
 */

static void
pref_set_db_cb (GtkWidget *widget, gpointer data) 
{
    GDictPrefDialog *pref_dialog;
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    pref_dialog->database = 
        gtk_object_get_data (GTK_OBJECT (widget), "db_name");
}

/* pref_set_strat_cb
 *
 * Sets the current search strategy to the one indicated
 */

static void
pref_set_strat_cb (GtkWidget *widget, gpointer data) 
{
    GDictPrefDialog *pref_dialog;
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    pref_dialog->dfl_strat = 
        gtk_object_get_data (GTK_OBJECT (widget), "strat_name");
}

/* pref_dialog_add_db
 *
 * Adds a database to the database list
 */

static void
pref_dialog_add_db (GDictPrefDialog *pref_dialog, gchar *db,
		    gchar *desc) 
{
    GtkWidget *menu_item;
    
    menu_item = gtk_menu_item_new_with_label (desc);
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", 
                        GTK_SIGNAL_FUNC (pref_set_db_cb), pref_dialog);
    gtk_object_set_data (GTK_OBJECT (menu_item), "db_name", db);
    gtk_widget_show (menu_item);
    gtk_menu_append (pref_dialog->db_list, menu_item);
    
    if (!strcmp (pref_dialog->database, db))
      gtk_menu_set_active (pref_dialog->db_list, pref_dialog->database_idx);
    pref_dialog->database_idx++;
}

/* pref_dialog_add_strat
 *
 * Adds a search strategy to the search strategy list
 */

static void
pref_dialog_add_strat (GDictPrefDialog *pref_dialog, gchar *strat,
		       gchar *desc) 
{
    GtkWidget *menu_item;
    
    menu_item = gtk_menu_item_new_with_label (desc);
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", 
                        GTK_SIGNAL_FUNC (pref_set_strat_cb), pref_dialog);
    gtk_object_set_data (GTK_OBJECT (menu_item), "strat_name", strat);
    gtk_widget_show (menu_item);
    gtk_menu_append (pref_dialog->strat_list, menu_item);
    
    if (!strcmp (pref_dialog->dfl_strat, strat))
	gtk_menu_set_active (pref_dialog->strat_list,
			     pref_dialog->dfl_strat_idx);
    pref_dialog->dfl_strat_idx++;
}

/* pref_dialog_reset_db
 *
 * Resets the database option menu and begins a command to retrieve it from
 * the server again
 */

static void
pref_dialog_reset_db (GDictPrefDialog *pref_dialog) {
    GtkWidget *error_label, *alignment;

    if (pref_dialog->get_db_cmd)
      dict_command_destroy (pref_dialog->get_db_cmd);
    
    if (pref_dialog->db_sel) {
        gtk_option_menu_remove_menu (pref_dialog->db_sel);
        gtk_widget_destroy (GTK_WIDGET (pref_dialog->db_sel));
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        pref_dialog->db_sel = NULL;
    }
    
    pref_dialog->db_list = GTK_MENU (gtk_menu_new ());
    
    pref_dialog->get_db_cmd = dict_show_db_command_new ();
    pref_dialog->get_db_cmd->error_notify_cb = pref_error_cb;
    pref_dialog->get_db_cmd->data_notify_cb = pref_data_cb;
    pref_dialog->get_db_cmd->status_notify_cb = pref_status_cb;
    pref_dialog->get_db_cmd->user_data = pref_dialog;
    pref_dialog->database_idx = 0;
    
    pref_dialog_add_db (pref_dialog, "!", _("Search all databases"));
    
    if (dict_command_invoke (pref_dialog->get_db_cmd,
			     pref_dialog->context) == -1)
    {
	/* Could not look up search strategies, so just display a
	 * label; FIXME: Memory leak
	 */
	error_label = gtk_label_new (_("Cannot connect to server"));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (error_label));
        gtk_table_attach_defaults (pref_dialog->table, alignment, 1, 2, 3, 4);
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
    }
}

/* pref_dialog_reset_strat
 *
 * Resets the strategies option menu and begins a command to retrieve it from
 * the server again
 */

static void
pref_dialog_reset_strat (GDictPrefDialog *pref_dialog) {
    GtkWidget *error_label, *alignment;

    if (pref_dialog->get_strat_cmd)
	dict_command_destroy (pref_dialog->get_strat_cmd);
    
    if (pref_dialog->strat_sel) {
        gtk_option_menu_remove_menu (pref_dialog->strat_sel);
        gtk_widget_destroy (GTK_WIDGET (pref_dialog->strat_sel));
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        pref_dialog->strat_sel = NULL;
    }
    
    pref_dialog->strat_list = GTK_MENU (gtk_menu_new ());
    
    pref_dialog->get_strat_cmd = dict_show_strat_command_new ();
    pref_dialog->get_strat_cmd->error_notify_cb = pref_error_cb;
    pref_dialog->get_strat_cmd->data_notify_cb = pref_data_cb;
    pref_dialog->get_strat_cmd->status_notify_cb = pref_status_cb;
    pref_dialog->get_strat_cmd->user_data = pref_dialog;
    pref_dialog->dfl_strat_idx = 0;
    
    if (dict_command_invoke (pref_dialog->get_strat_cmd,
			     pref_dialog->context) == -1) 
    {
	/* Could not look up search strategies, so just display a
	 * label; FIXME: Memory leak
	 */
	error_label = gtk_label_new (_("Cannot connect to server"));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment), 
                           GTK_WIDGET (error_label));
        gtk_table_attach_defaults (pref_dialog->table, alignment, 1, 2, 4, 5);
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
    }
}

/* pref_dialog_update_pref
 *
 * Updates global preferences structure with changes in preferences
 */

static gint
pref_dialog_update_pref (GDictPrefDialog *pref_dialog, gboolean save_prefs) {
    gchar *server, *port;
    gchar *old_server;
    gint old_port;
    gchar *font_name;
    gboolean smart;
    gint i;
    
    server = gtk_entry_get_text (pref_dialog->server_entry);
    port = gtk_entry_get_text (pref_dialog->port_entry);
    smart = gtk_toggle_button_get_active
	(GTK_TOGGLE_BUTTON (pref_dialog->smart_lookup_btn));
    
    if (strcmp (gdict_pref.server, server) || 
		gdict_pref.port != atoi (port) ||
		gdict_pref.smart != smart) {

        old_server = gdict_pref.server; old_port = gdict_pref.port;
        gdict_pref.server = g_strdup(server);
        gdict_pref.port = atoi(port);
        
        /* Try connecting. If we fail, restore the old config and abort */
        if (gdict_init_context ()) {
			gnome_error_dialog_parented (
				_("The server you specified could not be found."), 
				GTK_WINDOW (gdict_app));
			return -1;
		}
        
        pref_dialog->context = context;
        
        g_free(old_server);
        gdict_pref.smart = smart;
        
        pref_dialog_reset_db (pref_dialog);
        pref_dialog_reset_strat (pref_dialog);
    }
    
    gdict_pref.database = g_strdup (pref_dialog->database);
    gdict_pref.dfl_strat = g_strdup (pref_dialog->dfl_strat);
    
    for (i = 0; i < NUM_TYPEFACES; i++) {
        g_free(gdict_pref.typefaces[i].font_name);
        font_name = gnome_font_picker_get_font_name(pref_dialog->font_pickers[i]);
        gdict_pref.typefaces[i].font_name = g_strdup(font_name);
        gdict_pref.typefaces[i].font = gnome_font_picker_get_font(pref_dialog->font_pickers[i]);
        
        gnome_color_picker_get_i16(pref_dialog->color_pickers[i], 
                                   &gdict_pref.typefaces[i].color->red, 
                                   &gdict_pref.typefaces[i].color->green, 
                                   &gdict_pref.typefaces[i].color->blue, NULL);
    }
    
    if (gdict_applet_toggle) {
        gdict_pref.applet_handle = 
            gtk_toggle_button_get_active
	              (GTK_TOGGLE_BUTTON (pref_dialog->applet_handle_btn));
    }
    
    if (save_prefs)
	gdict_pref_save();
    
    return 0;
}

/* pref_error_cb
 *
 * Callback used when there is a socket error
 */

static void
pref_error_cb (dict_command_t *command, DictStatusCode code, 
               gchar *message, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    
    if (code != DICT_SOCKET_ERROR) {
        gnome_error_dialog_parented (message, GTK_WINDOW (pref_dialog));
    }
    else {
        gtk_signal_emit (GTK_OBJECT (pref_dialog),
                         gdict_pref_dialog_signals[SOCKET_ERROR_SIGNAL],
                         message);
    }
}

/* pref_data_cb
 *
 * Callback used when a new database or strategy definition has arrived 
 * over the link
 */

static void
pref_data_cb (dict_command_t *command, dict_res_t *res, gpointer data) {
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    if (command->cmd == C_SHOW_DB)
	pref_dialog_add_db (pref_dialog, res->name, res->desc);
    else if (command->cmd == C_SHOW_STRAT)
	pref_dialog_add_strat (pref_dialog, res->name, res->desc);
}

/* pref_status_cb
 *
 * Callback used when a status code has arrived over the link
 */

static void 
pref_status_cb (dict_command_t *command, DictStatusCode code, 
                int num_found, gpointer data)
{
    GDictPrefDialog *pref_dialog;
    GtkWidget *alignment;
    GtkOptionMenu *option_menu = NULL;
    GtkMenu *use_menu = NULL;
    gint row = 0;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (data));
    
    pref_dialog = GDICT_PREF_DIALOG (data);
    
    if (code == DICT_STATUS_OK) {
        if (command->cmd == C_SHOW_DB) {
            row = 3;
            use_menu = pref_dialog->db_list;
        }
        else if (command->cmd == C_SHOW_STRAT) {
            row = 4;
            use_menu = pref_dialog->strat_list;
        }
        
        option_menu = GTK_OPTION_MENU (gtk_option_menu_new ());
        gtk_option_menu_set_menu (option_menu, GTK_WIDGET (use_menu));
        alignment = gtk_alignment_new (0, 0.5, 0, 0);
        gtk_container_add (GTK_CONTAINER (alignment),
			   GTK_WIDGET (option_menu));
        gtk_table_attach_defaults (pref_dialog->table, alignment, 
                                   1, 2, row, row + 1);
        gtk_widget_show_all (GTK_WIDGET (pref_dialog->table));
        
        if (command->cmd == C_SHOW_DB)
	    pref_dialog->db_sel = option_menu;
        else if (command->cmd == C_SHOW_STRAT)
	    pref_dialog->strat_sel = option_menu;
    }
}

/* pref_dialog_ok_cb
 *
 * Callback issued when the Ok button is clicked
 */

static void 
pref_dialog_ok_cb (GtkWidget *widget, gpointer user_data) {
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (user_data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (user_data));
    
    pref_dialog = GDICT_PREF_DIALOG(user_data);
    if (pref_dialog_update_pref(pref_dialog, TRUE)) return;
    
    gtk_signal_emit (GTK_OBJECT (pref_dialog),
                     gdict_pref_dialog_signals[APPLY_CHANGES_SIGNAL]);
    
    gnome_dialog_close (GNOME_DIALOG (pref_dialog));
}

/* pref_dialog_apply_cb
 *
 * Callback issued when the Apply button is clicked
 */

static void 
pref_dialog_apply_cb (GtkWidget *widget, gpointer user_data) {
    GDictPrefDialog *pref_dialog;
    
    g_return_if_fail (user_data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (user_data));
    
    pref_dialog = GDICT_PREF_DIALOG(user_data);
    if (pref_dialog_update_pref(pref_dialog, FALSE)) return;
    
    gtk_signal_emit (GTK_OBJECT (pref_dialog),
                     gdict_pref_dialog_signals[APPLY_CHANGES_SIGNAL]);
}

/* pref_dialog_close_cb
 *
 * Callback issued when the Close button is clicked
 */

static void 
pref_dialog_close_cb (GtkWidget *widget, gpointer user_data) {
    GDictPrefDialog *pref_dialog;
    gchar *old_server;
    guint old_port;
    
    g_return_if_fail (user_data != NULL);
    g_return_if_fail (IS_GDICT_PREF_DIALOG (user_data));
    
    pref_dialog = GDICT_PREF_DIALOG (user_data);
    gtk_widget_hide (GTK_WIDGET (pref_dialog));
    
    old_server = g_strdup (gdict_pref.server);
    old_port = gdict_pref.port;
    gdict_pref_load ();
    
    if (strcmp (old_server, gdict_pref.server) || old_port != gdict_pref.port)
      gdict_init_context ();
    
    gtk_signal_emit (GTK_OBJECT (pref_dialog),
                     gdict_pref_dialog_signals[APPLY_CHANGES_SIGNAL]);
    
    gnome_dialog_close (GNOME_DIALOG (pref_dialog));
}
