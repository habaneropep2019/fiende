/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#include <config.h>
#include "callbacks.h"
#include "capplet-widget.h"
#include "gnome.h"
#include "screensaver-dialog.h"
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#ifndef errno
extern int errno;
#endif

/*#define HAVE_REDHAT_XSCREENSAVER_RPM*/
extern GtkWidget *capplet;
extern GtkWidget *preview_button;
extern GtkWidget *setup_button;
extern GtkWidget *setup_label;
extern GtkWidget *monitor;
extern gint ss_priority;
extern gshort waitmins;
extern gshort dpmsmins;
extern gboolean dpms;
extern gboolean password;
extern char *screensaver;
extern char *random_screensaver;
extern screensaver_data *sd;
extern GList *sdlist;
static pid_t pid = 0;
static pid_t pid_big = 0;
static GtkWidget *item_list = NULL;
static gint selected_item = 0;
gboolean ignore_changes = FALSE;
static gint random_x = 0;
static gint random_y = 0;
static guint random_timer = 0;

void ssaver_preview (GtkWidget *parent_widget, screensaver_data *sdp);

/* Loading info... */
void
screensaver_load (void)
{
        gint defval = 0;
        ss_priority = gnome_config_get_int ("/Screensaver/Default/nice=10");
        waitmins = gnome_config_get_int ("/Screensaver/Default/waitmins=20");
        if (waitmins > 9999)
                waitmins = 9999;
        else if (waitmins < 0)
                waitmins = 0;
        dpmsmins = gnome_config_get_int ("/Screensaver/Default/dpmsmins=20");
        if (dpmsmins > 9999)
                dpmsmins = 9999;
        else if (dpmsmins < 1)
                dpmsmins = 1;
        dpms = gnome_config_get_bool ("/Screensaver/Default/dpms=false");
        password = gnome_config_get_bool ("/Screensaver/Default/password=false");
        screensaver = gnome_config_get_string_with_default ("/Screensaver/Default/mode", &defval);
        if (defval) {
                screensaver = (char *)random_screensaver;
        } else if (!strcmp (screensaver, random_screensaver)) {
                g_free (screensaver);
                screensaver = (char *)random_screensaver;
        }
        sd = NULL;
}

GtkWidget *
get_and_set_min_entry (void)
{
        gchar tempmin[5];
        static GtkWidget *retval = NULL;

        if (!retval) {
                retval = gtk_entry_new ();
                gtk_signal_connect (GTK_OBJECT (retval), "insert_text", GTK_SIGNAL_FUNC (insert_text_callback), &waitmins);
                gtk_signal_connect_after (GTK_OBJECT (retval), "insert_text", GTK_SIGNAL_FUNC (insert_text_callback2), &waitmins);
                gtk_signal_connect_after (GTK_OBJECT (retval), "delete_text", GTK_SIGNAL_FUNC (delete_text_callback), &waitmins);
                gtk_entry_set_max_length (GTK_ENTRY (retval), 4);
                gtk_widget_set_usize (retval, 50, -2);
        }
        snprintf (tempmin, 5, "%d",waitmins);
        gtk_entry_set_text (GTK_ENTRY (retval), tempmin);
        return retval;
}
GtkWidget *
get_and_set_pword (void)
{
        static GtkWidget *retval = NULL;

        if (!retval) {
                retval = gtk_check_button_new_with_label (_("Require Password"));
                gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (retval), password);
                gtk_signal_connect (GTK_OBJECT (retval), "toggled", (GtkSignalFunc) password_callback, NULL);
        } else
                gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (retval), password);

        return retval;
}

GtkAdjustment *
get_and_set_nice (void)
{
        static GtkAdjustment *retval = NULL;

        if (!retval) {
                retval = GTK_ADJUSTMENT (gtk_adjustment_new ((gfloat)ss_priority,0.0, 19.0, 1.0, 1.0, 0.0));
                gtk_signal_connect (GTK_OBJECT (retval), "value_changed", (GtkSignalFunc) nice_callback, NULL);
        } else {
                retval->value = (gfloat) ss_priority;
                gtk_adjustment_value_changed (retval);
        }
        return retval;                
}

GtkWidget *
get_and_set_dpmsmin (void)
{
        static GtkWidget *retval = NULL;
        gchar tempmin[5];

        if (!retval) {
                retval = gtk_entry_new ();
                gtk_signal_connect (GTK_OBJECT (retval), "insert_text", GTK_SIGNAL_FUNC (insert_text_callback), &dpmsmins);
                gtk_signal_connect_after (GTK_OBJECT (retval), "insert_text", GTK_SIGNAL_FUNC (insert_text_callback2), &dpmsmins);
                gtk_signal_connect_after (GTK_OBJECT (retval), "delete_text", GTK_SIGNAL_FUNC (delete_text_callback), &dpmsmins);
                gtk_entry_set_max_length (GTK_ENTRY (retval), 4);
                gtk_widget_set_usize (retval, 50, -2);
        }
        snprintf (tempmin, 5, "%d",dpmsmins);
        gtk_entry_set_text (GTK_ENTRY (retval), tempmin);

        return retval;
}

GtkWidget *
get_and_set_dpmscheck (GtkWidget *box)
{
        static GtkWidget *retval = NULL;
        static GtkWidget *localbox;

        if (!retval) {
                localbox = box;
                retval = gtk_check_button_new_with_label (_("Use power management."));
                gtk_signal_connect (GTK_OBJECT (retval), "toggled", (GtkSignalFunc) dpms_callback, localbox);
        }
        gtk_signal_handler_block_by_func (GTK_OBJECT (retval), (GtkSignalFunc) dpms_callback, localbox);
        gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (retval), dpms);
        gtk_signal_handler_unblock_by_func (GTK_OBJECT (retval), (GtkSignalFunc) dpms_callback, localbox);
        gtk_widget_set_sensitive (localbox, dpms);
        return retval;
}

static screensaver_data *
get_random_data ()
{
        screensaver_data *retval;

        retval = g_malloc (sizeof (screensaver_data));
        retval->name = (char *)random_screensaver;
        retval->tryexec = NULL;
        retval->desktop_filename = NULL;
        retval->windowid = NULL;
        retval->root = NULL;
        retval->args = NULL;
        retval->icon = NULL;
        retval->comment = NULL;
        retval->author = NULL;
        retval->demo = NULL;
        retval->dialog = NULL;
        retval->setup_data = NULL;
        return retval;
}

static void
create_list (GtkCList *list, gchar *directory)
{
        DIR *dir;
        gchar *list_item[1];
        gint item;
        struct dirent *child;
        gchar *prefix;
        GList *desktop_items = NULL, *listp;
        screensaver_data *sdnew;

        dir = opendir (directory);
        if (dir == NULL)
                return;

        while ((child = readdir (dir)) != NULL) {
                if (!strstr(child->d_name, ".desktop"))
                        continue;

                if (child->d_name[0] != '.') {
                        desktop_items = g_list_insert_sorted(desktop_items, 
                                             g_strdup(child->d_name),
                                             (GCompareFunc)strcmp); 
                }
        }

        for (listp = desktop_items; listp; listp = g_list_next(listp)) {
                gchar *name;

                name = listp->data;
                prefix = g_strconcat ("=", directory, name, "=/Desktop Entry/", NULL);
                gnome_config_push_prefix (prefix);
                g_free (prefix);

                /* Lets make the data for the screensaver */
                sdnew = g_malloc (sizeof (screensaver_data));
                sdnew->tryexec = gnome_config_get_string ("TryExec");
                /* do we really want to load this? */
#ifdef HAVE_REDHAT_XSCREENSAVER_RPM
                if (sdnew->tryexec == NULL) {
                        g_free (sdnew);
                        gnome_config_pop_prefix ();
                        continue;
                }
                full_name = g_strconcat ("/usr/X11R6/lib/xscreensaver/", sdnew->tryexec, NULL);
                if (!g_file_test (full_name, G_FILE_TEST_ISFILE)) {
                        g_free (full_name);
                        g_free (sdnew->tryexec);
                        g_free (sdnew);
                        gnome_config_pop_prefix ();
                        continue;
                }
                g_free (full_name);
#endif
                sdnew->desktop_filename = g_strconcat (directory, name, NULL);
                sdnew->name = gnome_config_get_translated_string ("Name");
                gnome_config_pop_prefix ();
                sdnew->args = NULL;
                
                prefix = g_strconcat ("=", directory, name, "=/Screensaver Data/", NULL);
                gnome_config_push_prefix (prefix);
                g_free (prefix);
                sdnew->windowid = gnome_config_get_string ("WindowIdCommand");
                sdnew->root = gnome_config_get_string ("RootCommand");
                sdnew->author = gnome_config_get_translated_string ("Author");
                sdnew->comment = gnome_config_get_translated_string ("ExtendedComment");
                sdnew->demo = gnome_config_get_string ("Demo");
                prefix =  gnome_config_get_string ("Icon");
                if (prefix) {
                        if (prefix[0] == '/') {
                                sdnew->icon = prefix;
                                g_free (prefix);
                        } else {
                                /* we want to set up the initial settings */
                                sdnew->icon = g_strconcat (directory, prefix, NULL);
                                
                                g_free (prefix);
                        }
                } else
                        sdnew->icon = NULL;
                
                gnome_config_pop_prefix ();
                sdnew->dialog = NULL;
                sdnew->setup_data = NULL;
                if (!sdnew->name) {
                                /* bah -- useless file... */
                        break;
                }
                list_item[0] = sdnew->name;
                item = gtk_clist_append (list, list_item);
                if (screensaver && strcmp (sdnew->name, screensaver) == 0) {
                        sd = sdnew;
                        init_screensaver_data (sdnew);
                        selected_item = item;
                        if (sdnew->setup_data == NULL)
                                gtk_widget_set_sensitive (setup_button, FALSE);
                }
                gtk_clist_set_row_data(list, item, sdnew);
        }

        if (desktop_items) {
                g_list_foreach(desktop_items, (GFunc)g_free, NULL);
                g_list_free(desktop_items);
        }
        if (selected_item) {
                gtk_clist_select_row (list, selected_item, 0);
        }
}

GtkWidget *
get_and_set_mode(void)
{
        screensaver_data *random;
        gchar *tempdir;
        gchar *list_item[1];
        GtkWidget *swindow;

        if (!item_list) {
                swindow = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                
                item_list = gtk_clist_new (1);
                gtk_clist_set_column_auto_resize (GTK_CLIST (item_list), 0, TRUE);
                gtk_clist_set_selection_mode (GTK_CLIST (item_list), GTK_SELECTION_BROWSE);
                list_item [0] = _("No Screensaver");
                gtk_clist_append (GTK_CLIST (item_list), list_item);
                gtk_container_add (GTK_CONTAINER (swindow), item_list);

                random = get_random_data ();                
                list_item [0] = _("Random Screensaver");
                gtk_clist_append (GTK_CLIST (item_list), list_item);
                gtk_clist_set_row_data (GTK_CLIST (item_list), 1, random);
                if (screensaver == random_screensaver) {
                        /* In the case that it's the */
                        sd = random;
                        selected_item = 1;
                        gtk_widget_set_sensitive (setup_button, FALSE);
                } else if (!strcmp (screensaver, "NONE")) {
                        sd = NULL;
                        selected_item = 0;
                        gtk_widget_set_sensitive (setup_button, FALSE);
                }                
                tempdir = gnome_unconditional_datadir_file ("control-center/.data/");
                create_list (GTK_CLIST (item_list), tempdir);
                g_free (tempdir);
                gtk_signal_connect (GTK_OBJECT (item_list), 
                                    "select_row",
                                    (GtkSignalFunc) list_click_callback, 
                                    NULL);
                return swindow;
        } else {
                gint i=0;
                GList *dlist = NULL;

                screensaver = gnome_config_get_string_with_default ("/Screensaver/Default/mode=RANDOM SCREENSAVER", &i);
                /* The default, we know is the Random screensaver */
                if (i) {
                        gtk_clist_select_row (GTK_CLIST (item_list), 1, 0);
                        gtk_clist_moveto (GTK_CLIST (item_list), 1, 0, 0.5, 0.0);
                        return NULL;
                }
                if (!strcmp (screensaver, "NONE")) {
                        gtk_clist_select_row (GTK_CLIST (item_list), 0, 0);
                        gtk_clist_moveto (GTK_CLIST (item_list), 0, 0, 0.5, 0.0);
                        return NULL;
                }
                /* Damn, we need to find the stupid thing.. (-: */
                for (dlist = GTK_CLIST (item_list)->row_list; dlist; dlist = dlist->next) {
                        if (!strcmp ( screensaver,
                                       GTK_CELL_TEXT (GTK_CLIST_ROW (dlist)->cell[0])->text)) {
                                gtk_clist_moveto (GTK_CLIST (item_list), i, 0, 0.5, 0.0);
                                gtk_clist_select_row (GTK_CLIST (item_list), i, 0);
                                break;
                        }
                        i++;
                }
        }
        return NULL;
}
static gint
timer_cb ()
{
        static gint length = 0;

        if (!length)
                length = gdk_string_width (monitor->style->font, random_screensaver);
        random_x = (gint) (((gfloat)monitor->allocation.width - length)*random()/(RAND_MAX+1.0));
        random_y = monitor->style->font->descent + (gint) (((gfloat)monitor->allocation.height - monitor->style->font->descent)*random()/(RAND_MAX+1.0));

        gdk_draw_rectangle (monitor->window,monitor->style->black_gc, TRUE, 0, 0, monitor->allocation.width, monitor->allocation.height);
        gdk_draw_text (monitor->window,
                       monitor->style->font,
                       monitor->style->white_gc, 
                       random_x,
                       random_y,
                       _(random_screensaver),
                       strlen (_(random_screensaver)));
        
        return 1;
}

void
launch_miniview (screensaver_data *sd)
{
        gchar xid[11];
        int p[2];

        if (pid) {
                kill (pid, SIGTERM);
                pid = 0;
        }
        waitpid(pid, &pid, 0);
        if (sd && sd->name == random_screensaver) {
                gint length = gdk_string_width (monitor->style->font, random_screensaver);
                guint seed;

                seed = time (NULL);
                srandom (seed);

                random_x = (monitor->allocation.width-length)/2;
                random_y = monitor->allocation.height/2;
                gdk_draw_rectangle (monitor->window,monitor->style->black_gc, TRUE, 0, 0, monitor->allocation.width, monitor->allocation.height);
                gdk_draw_text (monitor->window,
                               monitor->style->font,
                               monitor->style->white_gc,
                               random_x,
                               random_y,
                               _(random_screensaver),
                               strlen (_(random_screensaver)));
                random_timer = gtk_timeout_add (3000, (GtkFunction) (timer_cb), NULL); 
                return;
        }
        if (random_timer) {
                /* removing the timer */
                gtk_timeout_remove (random_timer);
                random_timer = 0;
        }
        if (sd == NULL) {
                gdk_draw_rectangle (monitor->window,monitor->style->black_gc, TRUE, 0, 0, monitor->allocation.width, monitor->allocation.height);
                return;
        }
        if (sd->demo == NULL)
                /* FIXME: um, we _should_ do something.  Maybe steal the icon field.  */
                /* Maybe come up with a default demo.  dunno... */
                return;
        

        /* we fork and launch a spankin' new screensaver demo!!! */
        if (pipe (p) == -1)
                return;

        pid = fork ();
        if (pid == (pid_t) -1) {
                pid = 0;
                return;
        }
        if (pid == 0) {
		char *ctmp, *envpath;
		char **argv;

                close (p[0]);
                snprintf (xid, 11,"0x%x", (guint)GDK_WINDOW_XWINDOW (monitor->window));

		envpath = getenv ("PATH");
		ctmp = g_strdup_printf ("PATH=%s%s%s",
                                        envpath ? envpath : "",
                                        envpath ? ":" : "",
                                        "/usr/X11R6/lib/xscreensaver");
		putenv (ctmp);
		argv = g_strsplit(g_strconcat(sd->demo, " ", sd->windowid, " ", xid, NULL), " ", -1);

		execvp (argv[0], argv);

		/* This call should never return, but if it does... */
		_exit (0);
        }
        close (p[1]);

        return;
}

void
setup_callback (GtkWidget *widget, gpointer data)
{
        if (sd && sd->dialog == NULL) {
                gtk_widget_set_sensitive (setup_button, FALSE);
                sd->dialog = make_dialog (sd);
        }
}
static void
handle_list_change(void *data)
{
        GString *label_string;

        if (sd == data)
                return;
        if (data == NULL) {
                screensaver = NULL;
                sd = NULL;
                gtk_label_set (GTK_LABEL (setup_label), _("Settings..."));                
                gtk_widget_set_sensitive (setup_button, FALSE);
                launch_miniview (sd);
                return;
        }
        sd = (screensaver_data *) data;
        if (sd->name == random_screensaver) {
                /* we need to special case random */
                gtk_label_set (GTK_LABEL (setup_label),_("Random Settings"));
                gtk_widget_set_sensitive (setup_button, FALSE);
                launch_miniview (sd);
                screensaver = sd->name;
                return;
        }
        if (sd->args == NULL)
                init_screensaver_data (sd);

        label_string = g_string_new (NULL);
        g_string_sprintf (label_string, _("%s Settings..."), sd->name);
        gtk_label_set (GTK_LABEL (setup_label),label_string->str);
        g_string_free (label_string, TRUE);

        if ((sd->setup_data == NULL) || (sd->dialog != NULL))
                gtk_widget_set_sensitive (setup_button, FALSE);
        else
                gtk_widget_set_sensitive (setup_button, TRUE);
        
        launch_miniview (sd);
        screensaver = sd->name;
}
void
list_click_callback (GtkWidget *widget,
                     gint row,
                     gint column,
                     GdkEventButton *event,
                     gpointer data)
{
        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
        handle_list_change(gtk_clist_get_row_data(GTK_CLIST (widget), row));
}

void
monitor_expose_callback (GtkWidget *monitor, GdkEventButton *event, void *data)
{
        static gint length = 0;

        if (length == 0) {
                gdk_draw_rectangle (monitor->window,monitor->style->black_gc, TRUE, 0, 0, monitor->allocation.width, monitor->allocation.height);
                handle_list_change(data);
                length = gdk_string_width (monitor->style->font, random_screensaver);
                gtk_clist_moveto (GTK_CLIST (item_list), selected_item, 0, 0.5, 0.0);
        }
        if (!sd || sd->name != random_screensaver)
                return;
        gdk_draw_rectangle (monitor->window,monitor->style->black_gc, TRUE, 0, 0, monitor->allocation.width, monitor->allocation.height);
        gdk_draw_text (monitor->window,
                       monitor->style->font,
                       monitor->style->white_gc,
                       random_x,
                       random_y,
                       _(random_screensaver),
                       strlen (_(random_screensaver)));
}
void
monitor_died_callback (GtkWidget *check, void *data)
{
        /*g_print ("it died (booo hoo);\n");*/
}

void
insert_text_callback (GtkEditable    *editable, const gchar    *text,
                           gint length, gint *position,
                           void *data)
{
        gint i;

        for (i = 0; i < length; i++)
                if (!isdigit(text[i])) {
                        gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), "insert_text");
                        return;
                }
}
void
insert_text_callback2 (GtkEditable    *editable, const gchar    *text,
                            gint            length,
                            gint           *position,
                            void *data)
{
        *((gshort *) data) = atoi (gtk_entry_get_text (GTK_ENTRY (editable)));
        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
void
delete_text_callback (GtkEditable    *editable,
                            gint            length,
                            gint           *position,
                            void *data)
{
        *((gshort *) data) = atoi (gtk_entry_get_text (GTK_ENTRY (editable)));
        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
void
dpms_callback (GtkWidget *check, void *data)
{
        dpms = !dpms;
        if (dpms)
                gtk_widget_set_sensitive (GTK_WIDGET (data), TRUE);
        else
                gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);

        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
void
password_callback (GtkWidget *pword, void *data)
{

        password = !password;
        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
void
nice_callback (GtkObject *adj, void *data)
{
        ss_priority = (gint) GTK_ADJUSTMENT(adj)->value;
        if (!ignore_changes)
                capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
static void
ssaver_callback (GtkWidget *dialog, GdkEvent *event, GdkWindow *sswin)
{
        if (pid_big){
                kill (pid_big, SIGKILL);
                pid_big = 0;
//#if 0
/* 	uncommented it, so preview button doesn't get pressed again when mouse is clicked
	while screen saver preview is running; 
	why was it commented ? how come dialog's preview works without it ? */
                if (event->any.type == GDK_KEY_PRESS)
                        gtk_signal_emit_stop_by_name (GTK_OBJECT (dialog),"key_press_event");
                else if (event->any.type == GDK_BUTTON_PRESS)
                        gtk_signal_emit_stop_by_name (GTK_OBJECT (dialog),"button_press_event");
//#endif
                if (sswin) {
                        gdk_window_hide (sswin);
                }
        }
}

void
ssaver_preview (GtkWidget *parent_widget, screensaver_data *sdp)
{
        gchar *temp;
        static GdkWindow *sswin = NULL;
        GdkWindowAttr attributes;
        gchar xid[11];
		
	g_return_if_fail (sdp != NULL);
	
	if (sswin == NULL) {
		attributes.title = NULL;
		attributes.x = 0;
                attributes.y = 0;
		attributes.width = gdk_screen_width ();
		attributes.height = gdk_screen_height ();
		attributes.wclass = GDK_INPUT_OUTPUT;
		attributes.window_type = GDK_WINDOW_TOPLEVEL;
		attributes.override_redirect = TRUE;
		sswin = gdk_window_new (NULL, &attributes, GDK_WA_X|GDK_WA_Y|GDK_WA_NOREDIR);
	}
	gtk_signal_connect (GTK_OBJECT (parent_widget), "key_press_event", (GtkSignalFunc) ssaver_callback, sswin);
	gtk_signal_connect (GTK_OBJECT (parent_widget), "button_press_event", (GtkSignalFunc) ssaver_callback, sswin);
	XSelectInput (GDK_WINDOW_XDISPLAY (sswin), GDK_WINDOW_XWINDOW (sswin), ButtonPressMask | KeyPressMask);
	gdk_window_show (sswin);
	gdk_window_set_user_data (sswin, parent_widget);
                

        pid_big = fork ();
        if (pid_big == (pid_t) -1)
                return;
        if (pid_big == 0) {
                char *ctmp, *envpath;
                char **argv;

                snprintf (xid, 11,"0x%x", (guint) GDK_WINDOW_XWINDOW (sswin));

                envpath = getenv("PATH");
                ctmp = g_strdup_printf ("PATH=%s%s%s",
                        envpath ? envpath : "",
                        envpath ? ":" : "",
                        "/usr/X11R6/lib/xscreensaver");
                putenv (ctmp);
                /* trim args, so it will work for savers without any args (a space) */
                temp = g_strconcat (g_strchomp(sd->args), " ", sd->windowid, " ", xid, NULL);
                argv = g_strsplit (temp, " ", -1);

                execvp (argv[0], argv);
                _exit (1);
                /* This call should never return */
        }
}
	

void
preview_callback (GtkWidget *widget, gpointer data)
{
        ssaver_preview (preview_button, sd);
}


void
dialog_destroy_callback (GtkWidget *dialog, screensaver_data *newsd)
{
        newsd->dialog = NULL;
        newsd->dialog = NULL;
        if (sd == newsd)
                gtk_widget_set_sensitive (setup_button, TRUE);
}
void
dialog_callback (GtkWidget *dialog, gint button, screensaver_data *newsd)
{
        store_screensaver_data (newsd);
        switch (button) {
        case 0:
                ssaver_preview (dialog, newsd);
                break;
        case 1:
                gnome_dialog_close (GNOME_DIALOG (dialog));
                newsd->dialog = NULL;
                if (sd == newsd)
                        gtk_widget_set_sensitive (setup_button, TRUE);
                break;
        }
}
void
destroy_callback (GtkWidget *widget, void *data)
{
        if (pid)
                kill (pid, SIGTERM);
}
void
ok_callback (void)
{
        /* we want to commit our changes to disk. */
        GString *command;
        gchar temp[10];

        /* save the stuff... */
        gnome_config_set_int ("/Screensaver/Default/nice", ss_priority);
        gnome_config_set_int ("/Screensaver/Default/waitmins", waitmins);
        gnome_config_set_int ("/Screensaver/Default/dpmsmins", dpmsmins);
        gnome_config_set_bool ("/Screensaver/Default/dpms", dpms);
        gnome_config_set_bool ("/Screensaver/Default/password", password);
        if (screensaver)
                gnome_config_set_string ("/Screensaver/Default/mode", screensaver);
        else {
                gnome_config_set_string ("/Screensaver/Default/mode", "NONE");
                gnome_config_set_string ("/Screensaver/Default/command", "NONE");
        }
        
        system ("xscreensaver-command -exit");
        if (sd && waitmins) {
                if (sd->dialog)
                        store_screensaver_data (sd);
                command = g_string_new ("xscreensaver -no-splash -timeout ");
                snprintf (temp, 5, "%d", waitmins );
                g_string_append (command, temp);
                g_string_append (command, " -nice ");
                snprintf (temp, 5, "%d",20 - ss_priority);
                g_string_append (command, temp);
                if (password)
                        g_string_append (command, " -lock-mode");
                if (sd->name != random_screensaver) {
                        g_string_append (command," -xrm \"*programs:\t");
#ifdef HAVE_REDHAT_XSCREENSAVER_RPM
                        g_string_append (command, "/usr/X11R6/lib/xscreensaver/");
#endif
                }
                if (sd->args && *(sd->args))
                        g_string_append (command, sd->args);
                else if (sd->tryexec)
                        g_string_append (command, sd->tryexec);
                if (sd->root) {
                        g_string_append (command, " ");
                        g_string_append (command, sd->root);
                }
                if (sd->name != random_screensaver)
                        g_string_append (command, "\"");
                gnome_config_set_string ("/Screensaver/Default/command", command->str);
                g_string_append (command, " &");
                system (command->str);
                g_string_free (command, TRUE);
        }
        if (dpms) {
                /* does anyone know what standby is? */
                /* does it matter? laptops? */
                snprintf (temp, 10, "%d",(gint) (dpmsmins + waitmins) * 60);
                command = g_string_new ("xset dpms ");
                g_string_append (command, temp); /* suspend */
                g_string_append_c (command, ' ');
                g_string_append (command, temp); /* standby */
                g_string_append_c (command, ' ');
                g_string_append (command, temp); /* off */

                system (command->str);
                g_string_free (command, TRUE);
        }
        else
                system ("xset -dpms");

        gnome_config_sync ();
}
void
page_hide_callback (void)
{
        if (sd && sd->dialog)
                gtk_widget_hide (sd->dialog);
}
void
page_show_callback (void)
{
        if (sd && sd->dialog)
                gtk_widget_show (sd->dialog);
}
void
help_callback (void)
{
    GnomeHelpMenuEntry help_entry= {"control-center",
    "desktop-intro.html#GCCSCREEN"};
    gnome_help_display (NULL, &help_entry);
}
void
try_callback (void)
{
        GString *command;
        gchar temp[10];
        
        system ("xscreensaver-command -exit");

        
        if (sd && waitmins) {
                if (sd->dialog)
                        store_screensaver_data (sd);
                command = g_string_new ("xscreensaver -no-splash -timeout ");
                snprintf (temp, 5, "%d", waitmins );
                g_string_append (command, temp);
                g_string_append (command, " -nice ");
                snprintf (temp, 5, "%d",20 - ss_priority);
                g_string_append (command, temp);
                if (password)
                        g_string_append (command, " -lock-mode");
                if (sd->name != random_screensaver) {
                        g_string_append (command," -xrm \"*programs:\t");
#ifdef HAVE_REDHAT_XSCREENSAVER_RPM
                        g_string_append (command, "/usr/X11R6/lib/xscreensaver/");
#endif
                }
                if (sd->args && *(sd->args)) 
                        g_string_append (command, sd->args);
                else if (sd->tryexec)
                        g_string_append (command, sd->tryexec);
                if (sd->root) {
                        g_string_append (command, " ");
                        g_string_append (command, sd->root);
                }
                if (sd->name != random_screensaver)
                        g_string_append (command, "\"");
                g_string_append (command, "&");
                
                system (command->str);
                g_string_free (command, TRUE);
        }
        if (dpms) {
                /* does anyone know what standby is? */
                /* does it matter? laptops? */
                snprintf (temp, 10, "%d",(gint) (dpmsmins + waitmins) * 60);
                command = g_string_new ("xset dpms ");
                g_string_append (command, temp); /* suspend */
                g_string_append_c (command, ' ');
                g_string_append (command, temp); /* standby */
                g_string_append_c (command, ' ');
                g_string_append (command, temp); /* off */

                system (command->str);
                g_string_free (command, TRUE);
        }
        else
                system ("xset -dpms");
}
void
revert_callback (void)
{
        gchar *temp, *temp2;

        ignore_changes = TRUE;

        if (sd && sd->dialog) {
                gnome_dialog_close (GNOME_DIALOG (sd->dialog));
                sd->dialog = NULL;
                gtk_widget_set_sensitive (setup_button, TRUE);
        }
        gnome_config_drop_all ();
        screensaver_load ();

        if (pid) {
                kill (pid, SIGTERM);
                pid = 0;
        }
        waitpid(pid, &pid, 0);
        /* try to kill any existing xscreensaver */
        system ("xscreensaver-command -exit");
        
        gdk_draw_rectangle (monitor->window,monitor->style->black_gc, TRUE, 0, 0, monitor->allocation.width, monitor->allocation.height);

        temp = gnome_config_get_string ("/Screensaver/Default/command=xscreensaver -no-splash -timeout 20 -nice 10");
        if (!strchr(temp, '&')) {
                /* ugh! */
                temp2 = g_strconcat (temp, " &", NULL);
                system (temp2);
                g_free (temp2);
        } else {
                system (temp);
                g_free (temp);
        }

        /* set the devices to the correct one. */
        get_and_set_min_entry ();
        get_and_set_pword ();
        get_and_set_nice ();
        get_and_set_dpmsmin();
        get_and_set_dpmscheck(NULL);

        get_and_set_mode();

        ignore_changes = FALSE;
}
void
sig_child(int sig)
{
        int pid;
        int status;
        while ( (pid = wait3(&status, WNOHANG, (struct rusage *) 0)) > 0)
                ;
}
