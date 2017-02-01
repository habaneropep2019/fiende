/* -*- MODE: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Author: Chris Lahey <clahey@umich.edu>
 * Based on capplets/bell-properties/bell-properties.c.
 */
#include <config.h>
#include "capplet-widget.h"
#include <stdio.h>
#include <stdarg.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "gnome.h"

typedef struct _EditorDescription EditorDescription;
static gboolean ignore_changes = TRUE;
struct _EditorDescription
{
        gchar *name;
        gchar *executable_name;
        gboolean needs_term;
        gchar *execution_type;
        gboolean accepts_lineno;
};

EditorDescription original_info = { NULL };

EditorDescription possible_editors[] =
{
        { "Emacs", "emacs", FALSE, "executable", TRUE },
        { "XEmacs", "xemacs", FALSE, "executable", TRUE },
        { "vi", "vi", TRUE, "executable", TRUE },
        { "Go", "go", FALSE, "executable", FALSE },
        { "gEdit", "gedit", FALSE, "executable", FALSE },
        { "GWP", "gwp", FALSE, "executable", FALSE },
        { "Jed", "jed", TRUE, "executable", TRUE },
        { "Joe", "joe", TRUE, "executable", TRUE },
        { "Pico", "pico", TRUE, "executable", TRUE },
        { "vim",  "vim", TRUE, "executable", TRUE },
        { "gvim",  "gvim", FALSE, "executable", TRUE },
        { "ed", "ed", TRUE, "executable", FALSE },
        { "GMC/CoolEdit", "gmc -e", FALSE, "mc-internal", FALSE },
	{ "Nedit", "nedit", FALSE, "executable", FALSE }
};

static GtkWidget *capplet;
static GtkWidget *combo;
static GtkWidget *checkbox;

static void
set_combo( gchar *string )
{
        gint i;
        
        for ( i = 0;
              i < sizeof(possible_editors) / sizeof(possible_editors[0]);
              i++ ) {
                if ( ! strcmp( possible_editors[ i ].executable_name, string ) )
                        {
                                gtk_entry_set_text
                                        ( GTK_ENTRY(GTK_COMBO(combo)->entry),
                                          possible_editors[ i ].name ); 
                                return;
                        }
        }
        gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(combo)->entry), string );
}

static EditorDescription
get_combo( )
{
        gint i;
        gchar *string = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
        EditorDescription return_val;
        
        for ( i = 0; i < sizeof(possible_editors) / sizeof(possible_editors[0]); i++ ) {
                if ( ! strcmp( possible_editors[ i ].name, string ) ) {
                        return possible_editors[ i ];
                }
        }

        return_val.name = string;
        return_val.executable_name = string;
        return_val.needs_term = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( checkbox ) );
        return_val.execution_type = "executable";
        return return_val;
}

static void
edit_sync(EditorDescription desc)
{
        if ( combo != NULL )
                set_combo( desc.executable_name );
        if ( checkbox != NULL )
                gtk_toggle_button_set_active
                        ( GTK_TOGGLE_BUTTON( checkbox ),
                          desc.needs_term );
}

static void
edit_read(void)
{
	gchar *original_name
                = gnome_config_get_string("/editor/Editor/EDITOR=emacs");
        gboolean was_set;
        gchar    *key;
        gint     i;

        original_info.executable_name = original_name;
        set_combo( original_name );
        if(original_info.name)
                g_free(original_info.name);

        original_info.name = g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)));

        for ( i=0; i < sizeof(possible_editors) / sizeof(possible_editors[0]); i++ ) {
                if (!strcmp(possible_editors[i].name,original_info.name)) {
                        break;
                }
        }

        /* if no match, we pick reasonable defaults */
        if ( i == sizeof(possible_editors) / sizeof(possible_editors[0])) {
                original_info.needs_term = TRUE;
                original_info.execution_type = g_strdup("executable");
                original_info.accepts_lineno = FALSE;
        } else {
                original_info.needs_term = possible_editors[i].needs_term;
                original_info.execution_type = g_strdup(possible_editors[i].execution_type);
                original_info.accepts_lineno = possible_editors[i].accepts_lineno;
        }

        key = g_strconcat("/editor/Editor/NEEDS_TERM=", (original_info.needs_term) ?
                          "TRUE" : "FALSE", NULL);
        original_info.needs_term = gnome_config_get_bool_with_default(key, NULL);
        g_free(key);

        key = g_strconcat("/editor/Editor/EDITOR_TYPE=",original_info.execution_type,
                          NULL);
        if (original_info.execution_type)
                g_free(original_info.execution_type);
        original_info.execution_type = gnome_config_get_string_with_default(key, NULL);
        g_free(key);

        key = g_strconcat("/editor/Editor/ACCEPTS_LINE_NO=", 
                          (original_info.accepts_lineno) ? "TRUE" : "FALSE", NULL);
        original_info.accepts_lineno = gnome_config_get_bool_with_default(key, NULL);
        g_free(key);
}

static void
edit_changed (GtkWidget *widget, gpointer data)
{
        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}

static void
combo_changed (GtkWidget *widget, gpointer data)
{
        EditorDescription desc = get_combo ();
        if ( checkbox != NULL )
                gtk_toggle_button_set_active
                        ( GTK_TOGGLE_BUTTON( checkbox ),
                          desc.needs_term );
        edit_changed( widget, data );
}

static void
edit_help(void)
{
    GnomeHelpMenuEntry help_entry= {"control-center",
    "doc-handlers.html#GCCEDIT"};
    gnome_help_display (NULL, &help_entry);
}

static void
edit_apply(void)
{
        EditorDescription desc = get_combo();
        gnome_config_set_string("/editor/Editor/EDITOR",
                                desc.executable_name );
        gnome_config_set_bool("/editor/Editor/NEEDS_TERM",
                              gtk_toggle_button_get_active
                              ( GTK_TOGGLE_BUTTON( checkbox ) ));
        gnome_config_set_string("/editor/Editor/EDITOR_TYPE",
                                desc.execution_type );
        gnome_config_set_bool("/editor/Editor/ACCEPTS_LINE_NO",
                              desc.accepts_lineno);
        gnome_config_sync();
}

static void
edit_write(void)
{
	edit_apply();
}

static void
edit_revert (void)
{
        ignore_changes = TRUE;
        gnome_config_set_string("/editor/Editor/EDITOR", original_info.executable_name );
        gnome_config_set_bool("/editor/Editor/NEEDS_TERM", original_info.needs_term );
        gnome_config_set_string("/editor/Editor/EDITOR_TYPE", original_info.execution_type );
        gnome_config_set_bool("/editor/Editor/ACCEPTS_LINE_NO", original_info.accepts_lineno);
        gnome_config_sync();

        edit_read ();
        edit_sync (original_info);
        ignore_changes = FALSE;
}

static void
edit_setup(void)
{
	GtkWidget *vbox;
	GtkWidget *frame;
        GtkWidget *listitem;
        gint i;

        capplet = capplet_widget_new();
        ignore_changes = TRUE;
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD);

	frame = gtk_frame_new(_("Gnome editor"));
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

        combo = gtk_combo_new();
        for ( i = 0; i < sizeof(possible_editors) / sizeof(possible_editors[0]); i++ ) {
                listitem = gtk_list_item_new_with_label ( possible_editors[ i ].name );
                gtk_widget_show( listitem );
                gtk_container_add( GTK_CONTAINER( GTK_COMBO( combo )->list ), listitem );
        }
        gtk_container_add (GTK_CONTAINER (frame), combo);

        checkbox = gtk_check_button_new_with_label( _("Run In Terminal" ) );
        gtk_box_pack_start (GTK_BOX (vbox), checkbox, FALSE, FALSE, 0);
        
	/* Finished */

        edit_read ();
        edit_sync (original_info);

        gtk_signal_connect (GTK_OBJECT (capplet), "help",
                            GTK_SIGNAL_FUNC (edit_help), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "try",
                            GTK_SIGNAL_FUNC (edit_apply), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "revert",
                            GTK_SIGNAL_FUNC (edit_revert), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "ok",
                            GTK_SIGNAL_FUNC (edit_write), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "cancel",
                            GTK_SIGNAL_FUNC (edit_revert), NULL);

        gtk_signal_connect (GTK_OBJECT (GTK_COMBO(combo)->entry), "activate",
                            GTK_SIGNAL_FUNC (combo_changed), NULL);
        gtk_signal_connect (GTK_OBJECT (GTK_COMBO(combo)->list), "selection_changed",
                            GTK_SIGNAL_FUNC (combo_changed), NULL);

        gtk_signal_connect (GTK_OBJECT (checkbox), "toggled",
                            GTK_SIGNAL_FUNC (edit_changed), NULL);

        gtk_container_add (GTK_CONTAINER (capplet), vbox);
        gtk_widget_show_all (capplet);
        ignore_changes = FALSE;
}


int
main (int argc, char **argv)
{
				setlocale(LC_ALL, "");
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
        textdomain (PACKAGE);

        switch (gnome_capplet_init ("gnome-edit-properties", VERSION, argc,
                                    argv, NULL, 0, NULL)) {
                
        case -1:
                return 0;
        default:
		break;
        }

        edit_setup ();
        capplet_gtk_main ();

	return 0;
}
