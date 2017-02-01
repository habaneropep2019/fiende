/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Author: Elliot Lee <sopwith@redhat.com>
 */

/* #define TESTING */

#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <locale.h>
#include <gtk/gtk.h>
#include "capplet-widget.h"
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

/* use this flag so we don't tell control center of changes that it */
/* shouldn't turn on Try/Revert because we changed it, not the user */
static gboolean wecare=FALSE;
static GtkWidget *dialog;
typedef struct {
    GtkWidget *capplet;

    /* General setup */
    GtkWidget *enable_esd_startup, *enable_sound_events, *enable_restore_gmix;

    /* Sound events setup */
    GtkWidget *table, *ctree, *notebook;

    GtkWidget *btn_filename, *btn_play;

    gint ignore_changed;

    GHashTable *by_category;
} SoundProps;

static SoundProps *sound_properties_create(void);

#ifdef HAVE_ESD

#include <esd.h>

typedef struct {
    gchar *category;
    gchar *name;
    gchar *file;
    GtkCTreeNode *row;
} SoundEvent;

static void sound_properties_regenerate_ctree(SoundProps *props);
static void sound_properties_event_free(SoundEvent *ev);
static void sound_properties_read_path(SoundProps *props,
                                       GString *tmpstr,
                                       const char *path);
static void sound_properties_event_select(GtkCTree *ctree,
                                          GtkCTreeNode *row,
                                          gint column,
                                          SoundProps *props);
static void sound_properties_event_change_file(GtkEditable *entry,
                                               SoundProps *props);
static void sound_properties_set_sensitivity(GtkToggleButton *btn,
                                             SoundProps *props);
static void sound_properties_event_apply(GtkCTree *ctree,
                                         GtkCTreeNode *node,
                                         SoundProps *props);
static void sound_properties_apply(SoundProps *props); 
static void ui_do_revert(GtkWidget *w, SoundProps *props);
static void ui_do_ok(GtkWidget *w, SoundProps *props);
static void ui_do_help(void);
static void ui_do_cancel(GtkWidget *w, SoundProps *props);
static void ui_do_show(GtkWidget *w, SoundProps *props);
static void ui_do_hide(GtkWidget *w, SoundProps *props);
static void sound_properties_play_sound(GtkWidget *btn, SoundProps *props); 
static void reload_esd_samples(const char *config_path);
static void reload_all_esd_samples(void);
#endif

int
main(int argc, char *argv[])
{
    GnomeClient *client;
    GnomeClientFlags flags;
    gchar *session_args[3];
    int token, init_results;
    SoundProps *sound_properties;

		setlocale(LC_ALL, "");
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    init_results = gnome_capplet_init("sound-properties", VERSION,
                                      argc, argv, NULL, 0, NULL);
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-audio2.png");
    if (init_results < 0)
        g_error (_("an initialization error occurred while "
                 "starting 'sound-properties-capplet'."));

    client = gnome_master_client ();

    if(client)
        flags = gnome_client_get_flags(client);
    else
        flags = 0;

    if (flags & GNOME_CLIENT_IS_CONNECTED) {
#ifdef HAVE_ESD
		token = gnome_startup_acquire_token("GNOME_SOUND_PROPERTIES",
                                            gnome_client_get_id(client));

		if (token) {
			session_args[0] = argv[0];
			session_args[1] = "--init-session-settings";
			session_args[2] = NULL;
			gnome_client_set_priority (client, 20);
			gnome_client_set_restart_style (client, 
                                            GNOME_RESTART_ANYWAY);
			gnome_client_set_restart_command (client, 2, 
                                              session_args);
		}
		else
#endif
			gnome_client_set_restart_style (client, 
                                            GNOME_RESTART_NEVER);

        gnome_client_flush (client);
    } else
		token = 1;

    switch(init_results) {
    case 1:
#ifdef HAVE_ESD
        if(token) {
            if(gnome_sound_connection < 0
               && gnome_config_get_bool("/sound/system/settings/start_esd=true")) {
                int esdpid;
                static const char *esd_cmdline[] = {"esd", "-nobeeps", NULL};
                char *tmpargv[3];
                char argbuf[32];
                time_t starttime;

                esdpid = gnome_execute_async(NULL, 2, (char **)esd_cmdline);
                g_snprintf(argbuf, sizeof(argbuf), "%d", esdpid);
                tmpargv[0] = "kill"; tmpargv[1] = argbuf; tmpargv[2] = NULL;
                gnome_client_set_shutdown_command(client, 2, tmpargv);
		starttime = time(NULL);
                gnome_sound_init(NULL);
		while(gnome_sound_connection < 0
		      && ((time(NULL) - starttime) < 4)) {
#ifdef HAVE_USLEEP
			usleep(1000);
#endif
	                gnome_sound_init(NULL);
		}
            }
            
            if(gnome_sound_connection >= 0
               && gnome_config_get_bool("/sound/system/settings/event_sounds=true"))
                reload_all_esd_samples();

            if(gnome_config_get_bool("/sound/system/settings/restore_gmix=true")) {
                static const char *gmix_cmdline[] = {"gmix", "-i", NULL};

                (void) gnome_execute_async(NULL, 2, (char **)gmix_cmdline);
            }

        }
#endif
        break;

    case 0:
        sound_properties = sound_properties_create();
        capplet_gtk_main ();

    default:
        break;
    }

    return 0;
}

/**** sound_properties_create
      Outputs: 'retval' - info on newly created capplet thingie.
 */
static SoundProps *
sound_properties_create(void)
{
    GtkWidget *table, *vbox, *wtmp, *notebook;
    GtkWidget *hbox, *volume;
    GtkWidget *frame;
    char *path;
    char *filename;
    static const char *ctree_column_titles[] = {
        N_("Event"),
        N_("File to Play")
    };
    SoundProps *retval;

    /* dont tell control center about changes */
    wecare=FALSE;

    retval = g_new0(SoundProps, 1);

    retval->by_category = g_hash_table_new(g_str_hash, g_str_equal);

#ifdef TESTING
    retval->capplet = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_policy(GTK_WINDOW(retval->capplet), TRUE, TRUE, TRUE);
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "delete_event",
                       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
#else
    retval->capplet = capplet_widget_new();
#endif

#ifdef HAVE_ESD
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "help",
                       ui_do_help, NULL);
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "revert",
                       ui_do_revert, retval);
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "ok",
                       ui_do_ok, retval);
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "cancel",
                       ui_do_cancel, retval);
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "page_hidden",
                       ui_do_hide, retval);
    gtk_signal_connect(GTK_OBJECT(retval->capplet), "page_shown",
                       ui_do_show, retval);

    retval->notebook = notebook = gtk_notebook_new();

    /* * * * page "General" * * * */
    
    vbox = gtk_vbox_new(FALSE, GNOME_PAD);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

    filename = gnome_pixmap_file ("gnome-audio2.png");
    if (filename) {
            GtkWidget *alignment;
            GtkWidget *pixmap;

            alignment = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
            pixmap = gnome_pixmap_new_from_file (filename);
            gtk_container_add (GTK_CONTAINER (alignment), pixmap);

            gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
            g_free (filename);
    }   

    frame = gtk_frame_new (_("Sound server"));
    wtmp = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_add (GTK_CONTAINER (frame), wtmp);

    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

    retval->enable_esd_startup =
        gtk_check_button_new_with_label(_("Enable sound server startup"));
    gtk_box_pack_start (GTK_BOX (wtmp), retval->enable_esd_startup,
                        FALSE, FALSE, 0);
                        
    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(retval->enable_esd_startup),
                                gnome_config_get_bool("/sound/system/settings/start_esd=true"));

    retval->enable_sound_events =
        gtk_check_button_new_with_label(_("Sounds for events"));
    gtk_box_pack_start (GTK_BOX (wtmp), retval->enable_sound_events,
                        FALSE, FALSE, 0);

    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(retval->enable_sound_events),
                                gnome_config_get_bool("/sound/system/settings/event_sounds=true"));

    frame = gtk_frame_new (_("Audio mixer"));
    wtmp = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_add (GTK_CONTAINER (frame), wtmp);

    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

    retval->enable_restore_gmix =
        gtk_check_button_new_with_label(_("Restore mixer levels of Gnome Mixer"));
    gtk_box_pack_start (GTK_BOX (wtmp), retval->enable_restore_gmix,
                        FALSE, FALSE, 0);

    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(retval->enable_restore_gmix),
                                gnome_config_get_bool("/sound/system/settings/restore_gmix=false"));

    /* do signal connects _after_ setting state so that setting the state
       doesn't call sound_properties_set_sensitivity() and generate tons of
       warnings */
    gtk_signal_connect(GTK_OBJECT(retval->enable_esd_startup),
                       "toggled",
                       GTK_SIGNAL_FUNC(sound_properties_set_sensitivity),
                       retval);

    gtk_signal_connect(GTK_OBJECT(retval->enable_sound_events),
                       "toggled",
                       GTK_SIGNAL_FUNC(sound_properties_set_sensitivity),
                       retval);

    gtk_signal_connect(GTK_OBJECT(retval->enable_restore_gmix),
                       "toggled",
                       GTK_SIGNAL_FUNC(sound_properties_set_sensitivity),
                       retval);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox,
                             gtk_label_new(_("General")));

    /* * * * page "Sound Events" * * * */
    retval->table = table = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD);

    gtk_table_attach(GTK_TABLE(table),
                     (retval->btn_filename = gnome_file_entry_new(NULL, _("Select sound file"))),
                     1, 2, 1, 2,
                     GTK_EXPAND | GTK_SHRINK, 0,
                     GNOME_PAD_SMALL, GNOME_PAD_SMALL);
    path = gnome_datadir_file ("sounds/");
    gnome_file_entry_set_default_path(GNOME_FILE_ENTRY (retval->btn_filename), path);
    g_free (path);
    
    ctree_column_titles[0] = _(ctree_column_titles[0]);
    ctree_column_titles[1] = _(ctree_column_titles[1]);
    retval->ctree = gtk_ctree_new_with_titles(2, 0,
                                              (gchar **)ctree_column_titles);

    gtk_clist_set_selection_mode(GTK_CLIST(retval->ctree), GTK_SELECTION_BROWSE);

    gtk_ctree_set_expander_style(GTK_CTREE(retval->ctree),
                                 GTK_CTREE_EXPANDER_SQUARE);
    gtk_ctree_set_line_style(GTK_CTREE(retval->ctree), GTK_CTREE_LINES_DOTTED);

    sound_properties_regenerate_ctree(retval);

    gtk_clist_set_column_auto_resize(GTK_CLIST(retval->ctree), 0, TRUE);
    /*    gtk_clist_set_column_auto_resize(GTK_CLIST(retval->ctree), 1, TRUE);*/

    gtk_container_set_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
    wtmp = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_usize(GTK_WIDGET(wtmp),
                         gtk_clist_columns_autosize(GTK_CLIST(retval->ctree)),
                         250);

    gtk_container_add(GTK_CONTAINER(wtmp), retval->ctree);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(wtmp), GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_ALWAYS);
    gtk_table_attach_defaults(GTK_TABLE(table), wtmp, 0, 2, 0, 1);

    retval->btn_play = gtk_button_new ();
    hbox = gtk_hbox_new (FALSE, 0);
    volume = gnome_stock_pixmap_widget (retval->btn_play, GNOME_STOCK_PIXMAP_VOLUME);
    gtk_box_pack_start (GTK_BOX (hbox), volume, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new (_("Play")), FALSE, FALSE, 8);
    gtk_container_add (GTK_CONTAINER (retval->btn_play), hbox);

    gtk_table_attach(GTK_TABLE(table),
                     retval->btn_play, 
                     0, 1, 1, 2,
                     GTK_EXPAND|GTK_SHRINK, 0,
                     GNOME_PAD_SMALL, GNOME_PAD_SMALL);
    gtk_object_set_data(GTK_OBJECT(retval->btn_play), "gnome_disable_sound_events", GINT_TO_POINTER(1));

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table,
                             gtk_label_new(_("Sound Events")));

    /* * * * end notebook page setup * * * */

    gtk_container_add(GTK_CONTAINER(retval->capplet), notebook);

    sound_properties_set_sensitivity(NULL, retval);

    gtk_signal_connect(GTK_OBJECT(retval->ctree), "tree_select_row",
                       sound_properties_event_select, retval);
    gtk_clist_select_row(GTK_CLIST(retval->ctree), 1, 0);

    gtk_signal_connect(GTK_OBJECT(retval->btn_play), "clicked",
                       sound_properties_play_sound, retval);
    gtk_signal_connect(GTK_OBJECT(gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(retval->btn_filename))),
                       "changed",
                       GTK_SIGNAL_FUNC(sound_properties_event_change_file),
                       retval);
#else
    gtk_container_add(GTK_CONTAINER(retval->capplet), gtk_label_new(_("This copy of the GNOME control center was not compiled with sound support")));
#endif


    gtk_widget_show_all(retval->capplet);

    /* ok we might care now */
    wecare = TRUE;
    return retval;
}

#ifdef HAVE_ESD
/**** sound_properties_regenerate_ctree

      Inputs: 'ctree' - a GtkCTree to put the entries into.

      Side effects: Clears & populates 'ctree'.

      Description: clears out 'ctree' and then puts all the available
                   data into rows.
 */
static void
sound_properties_regenerate_ctree(SoundProps *props)
{
    GString *tmpstr = g_string_new(NULL);
    char *ctmp;

    gtk_clist_freeze(GTK_CLIST(props->ctree));
    gtk_clist_clear(GTK_CLIST(props->ctree));

    /* clean up hash tables - key is first arg to GHFunc */
    g_hash_table_foreach(props->by_category, (GHFunc) g_free, NULL);
    g_hash_table_destroy(props->by_category);
    props->by_category = g_hash_table_new(g_str_hash, g_str_equal);

    ctmp = gnome_config_file("/sound/events");
    if(ctmp)
        sound_properties_read_path(props, tmpstr, ctmp);
    g_free(ctmp);

    ctmp = gnome_util_home_file("sound/events");
    if(ctmp)
        sound_properties_read_path(props, tmpstr, ctmp);
    g_free(ctmp);

    g_string_free(tmpstr, TRUE);

    gtk_clist_thaw(GTK_CLIST(props->ctree));
}

static void
sound_properties_event_free(SoundEvent *ev)
{
    g_free(ev->category); g_free(ev->file); g_free(ev->name);

    g_free(ev);
}

/**** sound_properties_read_path
 */
static void
sound_properties_read_path(SoundProps *props,
                           GString *tmpstr,
                           const char *path)
{
    DIR *dirh;
    SoundEvent *new_entry;
    char *sample_name, *sample_file, *ctmp;
    gpointer event_iter;
    GtkCTreeNode *category_node = NULL, *event_node = NULL,
        *prev_event_node = NULL;
    char *arow[2] = {NULL,NULL};
    struct dirent *dent;
    GHashTable *cathash;

    char *file;

    dirh = opendir(path);
    if(!dirh)
        return;

    while((dent = readdir(dirh))) {
	    if (!strcmp(dent->d_name, ".")
            || !strcmp(dent->d_name, ".."))
		    continue;

        file = g_strdup(dent->d_name);

        g_string_sprintf(tmpstr, "=%s/%s=", path, file);

        gnome_config_push_prefix(tmpstr->str);

        arow[1] = ""; 

        ctmp = gnome_config_get_translated_string("__section_info__/description");
        if(ctmp && *ctmp) {
            arow[0] = ctmp;
        } else {
            g_free(ctmp);
            arow[0] = g_strdup(file);
            if(strstr(arow[0], ".soundlist")) {
                *strstr(arow[0], ".soundlist") = '\0';
            }
        }

        category_node = g_hash_table_lookup(props->by_category, file);

        if(!category_node) {
            category_node = gtk_ctree_insert_node(GTK_CTREE(props->ctree),
                                                  NULL, NULL,
                                                  arow, GNOME_PAD_SMALL,
                                                  NULL, NULL, NULL, NULL, FALSE,
                                                  FALSE);
            gtk_ctree_node_set_selectable(GTK_CTREE(props->ctree), category_node,
                                          FALSE);
            gtk_ctree_node_set_row_data_full(GTK_CTREE(props->ctree),
                                        category_node,
                                        g_hash_table_new(g_str_hash,
                                                         g_str_equal),
                                        (GtkDestroyNotify)g_hash_table_destroy);
            g_hash_table_insert(props->by_category, g_strdup(file),
                                category_node);
        }

        cathash = gtk_ctree_node_get_row_data(GTK_CTREE(props->ctree),
                                              category_node);

        g_free(arow[0]);

        event_node = prev_event_node = NULL;
        event_iter = gnome_config_init_iterator_sections(tmpstr->str);
        while((event_iter = gnome_config_iterator_next(event_iter,
                                                       &sample_name,
                                                       NULL))) {

            if(!strcmp(sample_name, "__section_info__")) {
                g_free(sample_name);
                continue;
            }

            g_string_sprintf(tmpstr, "%s/file", sample_name);
            arow[1] = sample_file = gnome_config_get_string(tmpstr->str);

            event_node = g_hash_table_lookup(cathash, sample_name);

            if(event_node) {

                new_entry = gtk_ctree_node_get_row_data(GTK_CTREE(props->ctree),
                                                        event_node);

                g_free(new_entry->file);

                gtk_ctree_node_set_text(GTK_CTREE(props->ctree), event_node,
                                        1, sample_file);

                new_entry->file = sample_file;
                g_free(sample_name);

            } else {

                g_string_sprintf(tmpstr, "%s/description", sample_name);
                arow[0] = gnome_config_get_translated_string(tmpstr->str);
                if(!arow[1] || !*arow[0]) {
                    g_free(arow[0]);
                    arow[0] = g_strdup(sample_name);
                }

                event_node = gtk_ctree_insert_node(GTK_CTREE(props->ctree),
                                                   category_node, prev_event_node,
                                                   arow, GNOME_PAD_SMALL,
                                                   NULL, NULL, NULL, NULL, TRUE,
                                                   FALSE);
                g_free(arow[0]);
                new_entry = g_new0(SoundEvent, 1);
                new_entry->category = g_strdup(file);
                new_entry->name = sample_name;
                new_entry->file = sample_file;
                new_entry->row = event_node;

                gtk_ctree_node_set_row_data_full(GTK_CTREE(props->ctree),
                                                 event_node,
                                                 new_entry,
                                                 (GtkDestroyNotify)sound_properties_event_free);
                g_hash_table_insert(cathash, new_entry->name, event_node);

                prev_event_node = event_node;
            }
            gtk_ctree_expand(GTK_CTREE(props->ctree), GTK_CTREE_NODE(category_node));
        }

        gnome_config_pop_prefix();
        g_free(file);
    }
    closedir(dirh);
}

static void
sound_properties_event_change_file(GtkEditable *entry, SoundProps *props)
{
    char *tmp, *sounddir, *tmp2;
    
    if(props->ignore_changed)
        return;

/*    g_return_if_fail(GTK_CLIST(props->ctree)->selection); */
    if (!GTK_CLIST(props->ctree)->selection)
        return;

    capplet_widget_state_changed(CAPPLET_WIDGET(props->capplet), TRUE);

    tmp = gtk_entry_get_text(GTK_ENTRY(entry));
    sounddir = gnome_sound_file("");
    if (!strncmp(tmp, sounddir, strlen(sounddir)))
        tmp2 = tmp+strlen(sounddir);
    else
        tmp2 = tmp;

    gtk_ctree_node_set_text(GTK_CTREE(props->ctree),
                            GTK_CLIST(props->ctree)->selection->data,
                            1, tmp2);
}

static void
sound_properties_event_select(GtkCTree *ctree,
                              GtkCTreeNode *row,
                              gint column,
                              SoundProps *props)
{
    char *ctmp;

    ctmp = GTK_CLIST_ROW(&row->list)->cell[1].u.text;
    if(!ctmp)
        ctmp = "";

    props->ignore_changed++;
    gtk_entry_set_text(GTK_ENTRY(gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(props->btn_filename))), ctmp);
    props->ignore_changed--;
}

static void
sound_properties_event_apply(GtkCTree *ctree,
                             GtkCTreeNode *node,
                             SoundProps *props)
{
    SoundEvent *ev;
    char *cur_filename = NULL, *ctmp;

    ev = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);
#if 0
    g_print("get row data on %p = %p", node, ev);
#endif

    g_return_if_fail(ev);

    gtk_ctree_node_get_text(GTK_CTREE(ctree),
                            node, 1, &cur_filename);

    if(!cur_filename || !(*cur_filename)) return;

    /* If the user didn't change the setting, no need to set it */
    if(!strcmp(cur_filename, ev->file))
        return;

    ctmp = g_strconcat("/sound/events/", ev->category, "/", ev->name, "/file", NULL);
    gnome_config_set_string(ctmp, cur_filename);
    g_free(ctmp);

    reload_all_esd_samples();
}

static void
sound_properties_apply(SoundProps *props)
{
    gboolean esd_startup, sound_events, restore_gmix;
    gtk_ctree_post_recursive(GTK_CTREE(props->ctree), NULL,
                             (GtkCTreeFunc)sound_properties_event_apply, props);

    esd_startup = GTK_TOGGLE_BUTTON(props->enable_esd_startup)->active;
    sound_events = GTK_TOGGLE_BUTTON(props->enable_sound_events)->active;
    restore_gmix = GTK_TOGGLE_BUTTON(props->enable_restore_gmix)->active;
    gnome_config_set_bool("/sound/system/settings/start_esd",
                          esd_startup);
    gnome_config_set_bool("/sound/system/settings/event_sounds",
                          esd_startup && sound_events);
    gnome_config_set_bool("/sound/system/settings/restore_gmix",
                          restore_gmix);
    gnome_config_sync();
    reload_all_esd_samples();
}

static void
sound_properties_set_sensitivity(GtkToggleButton *btn,
                                 SoundProps *props)
{
    GtkWidget *page;
    gboolean do_events;

    gtk_widget_set_sensitive(props->enable_sound_events,
                             GTK_TOGGLE_BUTTON(props->enable_esd_startup)->active);

    page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(props->notebook), 1);
    do_events = GTK_TOGGLE_BUTTON(props->enable_sound_events)->active;

    gtk_widget_set_sensitive(page, do_events);

    gtk_widget_set_sensitive(gtk_notebook_get_tab_label(GTK_NOTEBOOK(props->notebook),
                                                        page), do_events);
    if (wecare)
        capplet_widget_state_changed(CAPPLET_WIDGET(props->capplet), TRUE);
}

static void
ui_do_revert(GtkWidget *w, SoundProps *props)
{
    wecare=FALSE;

    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(props->enable_esd_startup),
                                gnome_config_get_bool("/sound/system/settings/start_esd=true"));

    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(props->enable_sound_events),
                                gnome_config_get_bool("/sound/system/settings/event_sounds=true"));

    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(props->enable_restore_gmix),
                                gnome_config_get_bool("/sound/system/settings/restore_gmix=false"));

    sound_properties_regenerate_ctree(props);

    gtk_clist_select_row(GTK_CLIST(props->ctree), 1, 0);

    wecare=TRUE;
}

static void
ui_do_help(void)
{
    GnomeHelpMenuEntry help_entry= {"control-center",
    "multimedia-intro.html#MULTIMEDIA-SOUND"};
    gnome_help_display (NULL, &help_entry);

}

static void
ui_do_ok(GtkWidget *w, SoundProps *props)
{
    sound_properties_apply(props);
    gtk_main_quit();
}

static void
ui_do_cancel(GtkWidget *w, SoundProps *props)
{
    gtk_main_quit();
}
static void
ui_do_show(GtkWidget *w, SoundProps *props)
{
    if (dialog)
        gtk_widget_show (dialog);
}
static void
ui_do_hide(GtkWidget *w, SoundProps *props)
{
    if (dialog)
        gtk_widget_hide (dialog);
}

static void
sound_properties_play_sound(GtkWidget *btn, SoundProps *props)
{
    char *ctmp, *ctmp2;
    GtkCTreeNode *node;

    g_return_if_fail(GTK_CLIST(props->ctree)->selection);

    node = GTK_CTREE_NODE(GTK_CLIST(props->ctree)->selection->data);
    ctmp = GTK_CLIST_ROW(&node->list)->cell[1].u.text;

    if(*ctmp == '/' || g_file_exists(ctmp))
        ctmp2 = g_strdup(ctmp);
    else
        ctmp2 = gnome_sound_file(ctmp);

    if(ctmp2 && g_file_exists(ctmp2))
       gnome_sound_play(ctmp2);
    else {
        char *msg;

        if(ctmp2 && ctmp2[0]=='/')
            msg = _("The sound file for this event does not exist.");
        else
            msg = _("The sound file for this event does not exist.\n"
                    "You may want to install the gnome-audio package\n"
                    "for a set of default sounds.");

        dialog = gnome_message_box_new(msg,
                                       GNOME_MESSAGE_BOX_ERROR,
                                       _("Close"), NULL);
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
        gnome_dialog_run(GNOME_DIALOG (dialog));
        dialog = NULL;
    }

    g_free(ctmp2);
}

static void
reload_esd_samples(const char *config_path)
{
    DIR *dirh;
    char *category_name, *sample_name, *sample_file, *ctmp;
    gpointer event_iter;
    struct dirent *dent;
    GString *tmpstr;

    dirh = opendir(config_path);
    if(!dirh)
        return;

    tmpstr = g_string_new(NULL);

    while((dent = readdir(dirh))) {
        /* ignore no-good dir entries.
           We ignore "gnome" because the system sounds are listed in there.
        */

        if (!strcmp(dent->d_name, ".")
            || !strcmp(dent->d_name, ".."))
            continue;

        g_string_sprintf(tmpstr, "=%s/%s=", config_path, dent->d_name);

        gnome_config_push_prefix(tmpstr->str);

        category_name = dent->d_name;
        ctmp = strstr(category_name, ".soundlist");
        if(ctmp) *ctmp = '\0';

        event_iter = gnome_config_init_iterator_sections(tmpstr->str);
        while((event_iter = gnome_config_iterator_next(event_iter,
                                                       &sample_name, NULL))) {
            if(!strcmp(sample_name, "__section_info__")) {
                g_free(sample_name);
                continue;
            }

            g_string_sprintf(tmpstr, "%s/file", sample_name);
            sample_file = gnome_config_get_string(tmpstr->str);

            if(!sample_file || !*sample_file) {
                g_free(sample_file);
                g_free(sample_name);
                continue;
            }

            if(*sample_file != '/') {
                char *tmp = gnome_sound_file(sample_file);
                g_free(sample_file);
                sample_file = tmp;
            }

            if(sample_file) {
                int sid;

                g_string_sprintf(tmpstr, "%s/%s", category_name, sample_name);

                /* We need to free up the old sample, because
                   esd allows multiple samples with the same name,
                   putting memory to waste. */
                sid = esd_sample_getid(gnome_sound_connection, tmpstr->str);
                if(sid >= 0)
                    esd_sample_free(gnome_sound_connection, sid);

                sid = gnome_sound_sample_load(tmpstr->str, sample_file);

                if(sid < 0)
                    g_warning("Couldn't load sound file %s as sample %s",
                              sample_file, tmpstr->str);
            }
            
            g_free(sample_name);
            g_free(sample_file);
        }

        gnome_config_pop_prefix();
    }
    closedir(dirh);

    g_string_free(tmpstr, TRUE);
}

static void
reload_all_esd_samples(void)
{
    char *val;
    val = gnome_config_file("/sound/events");
    if(val) {
        reload_esd_samples(val);
        g_free(val);
    }
    val = gnome_util_home_file("/sound/events");
    if(val) {
        reload_esd_samples(val);
        g_free(val);
    }
}

#endif
