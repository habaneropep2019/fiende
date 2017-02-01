/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>

#include <fromtop.h>

#include <glibtop.h>
#include <glibtop/xmalloc.h>
#include <glibtop/union.h>

#ifdef GLIBTOP_INODEDB
#include <glibtop/inodedb.h>
#endif

#include <procview.h>
#include <gtop-graph.h>
#include <process-details.h>

static void
destroy_handler		(GtkWidget *widget, GTopProcessDetails *d);
static void
update_cb			(GtkWidget *widget, GTopProcessDetails *d);

static void
close_cb			(GtkWidget *widget, GTopProcessDetails *d);

static gint
do_update			(gpointer d);

static void
auto_update			(GTopProcessDetails *d);

static void
process_state_create		(GTopProcessDetails *d);

static void
process_state_update		(GTopProcessDetails *d);

#define STATE_FIELDS		9

enum {
    STATE_PID = 0,
    STATE_USER,
    STATE_PRIORITY,
    STATE_NICE,
    STATE_STATE,
    STATE_CPU,
    STATE_WCPU,
    STATE_MEM,
    STATE_NAME,

};

static const gchar *state_names [STATE_FIELDS] = {
    "PID", "User", "Pri", "Nice", "State", "CPU", "WCPU", "MEM", "Name"
};

static const gint state_widths [STATE_FIELDS] = {
    6, 10, 4, 4, 5, 5, 5, 5, 10
};

static void
process_args_create		(GTopProcessDetails *d);

static void
process_args_update		(GTopProcessDetails *d);

#define ARGS_FIELDS		1

enum {
    ARGS_CMDLINE = 0
};

static const gchar *args_names [ARGS_FIELDS] = {
    "Command Line"
};

static const gint args_widths [ARGS_FIELDS] = {
    80
};

static void
process_credentials_create	(GTopProcessDetails *d);

static void
process_credentials_update	(GTopProcessDetails *d);

#if (LIBGTOP_VERSION_CODE > 1001000)
#define CREDENTIAL_FIELDS	8
#else
#define CREDENTIAL_FIELDS	4
#endif

enum {
    CREDENTIALS_EUID = 0,
    CREDENTIALS_RUID,
    CREDENTIALS_EGID,
    CREDENTIALS_RGID,
#if (LIBGTOP_VERSION_CODE > 1001000)
    CREDENTIALS_SUID,
    CREDENTIALS_SGID,
    CREDENTIALS_FSUID,
    CREDENTIALS_FSGID
#endif
};

static const gchar *credential_names [CREDENTIAL_FIELDS] = {
    "EUID", "RUID", "EGID", "RGID",
#if (LIBGTOP_VERSION_CODE > 1001000)
    "SUID", "SGID", "FSUID", "FSGID"
#endif
};

static const gint credential_widths [CREDENTIAL_FIELDS] = {
    10, 10, 10, 10,
#if (LIBGTOP_VERSION_CODE > 1001000)
    10, 10, 10, 10
#endif
};

void
gtop_process_details (gint pid)
{
    GTopProcessDetails *d;
    gchar *title, *form;
    gint auto_update_flag;

    d = g_new0 (GTopProcessDetails, 1);

    form = _("Process %d Details");
    title = g_new0 (char, strlen (form)+64);
    sprintf (title, form, pid);

    d->pid = pid;
    d->dialog = gnome_dialog_new (title, NULL);

    auto_update_flag = gtop_properties.procview.details_flags &
	(1 << GTOP_DETAILS_AUTO_UPDATE);

    gnome_dialog_append_button_with_pixmap
	(GNOME_DIALOG (d->dialog), _("Update"),GNOME_STOCK_PIXMAP_REFRESH);

    gnome_dialog_append_button
	(GNOME_DIALOG (d->dialog), GNOME_STOCK_BUTTON_CLOSE);

    gtk_widget_set_name (d->dialog, "GTopProcessDetails");
	
    d->vb = GNOME_DIALOG (d->dialog)->vbox;

    gtk_container_border_width
	(GTK_CONTAINER (d->vb), GNOME_PAD_SMALL);

    process_state_create (d);
    process_args_create (d);
    process_credentials_create (d);

    auto_update (d);
    update_cb (NULL, d);

    gtk_signal_connect
	(GTK_OBJECT (d->dialog), "destroy",
	 GTK_SIGNAL_FUNC (destroy_handler), (gpointer) d);

    gnome_dialog_button_connect
	(GNOME_DIALOG (d->dialog), 0,
	 GTK_SIGNAL_FUNC (update_cb), (gpointer) d);

    gnome_dialog_button_connect
	(GNOME_DIALOG (d->dialog), 1,
	 GTK_SIGNAL_FUNC (close_cb), (gpointer) d);

    gtk_widget_show (d->dialog);

    g_free (title);
}

static void
destroy_handler (GtkWidget *widget, GTopProcessDetails *d)
{
    if (d->timeout_id) {
	gtk_timeout_remove (d->timeout_id);
	d->timeout_id = 0;
    }

    d->dialog = NULL;
    g_free (d);
}

static void
update_cb (GtkWidget *widget, GTopProcessDetails *d)
{
    process_state_update (d);
    process_args_update (d);
    process_credentials_update (d);
}

static void
close_cb (GtkWidget *widget, GTopProcessDetails *d)
{
    gint x, y, w, h;
	
    gdk_window_get_geometry (GTK_WIDGET (d->dialog)->window,
			     &x, &y, &w, &h, NULL);

    gdk_window_get_position (GTK_WIDGET (d->dialog)->window,
			     &x, &y);

    gnome_config_set_int ("gtop/process_details/x", x);
    gnome_config_set_int ("gtop/process_details/y", y);
    gnome_config_set_int ("gtop/process_details/w", w);
    gnome_config_set_int ("gtop/process_details/h", h);
    gnome_config_sync ();

    gtk_widget_destroy(d->dialog);
}

static gint
do_update (gpointer d)
{
    process_state_update ((GTopProcessDetails *) d);
    return TRUE;
}

static void
auto_update (GTopProcessDetails *d)
{
    gint auto_update = gtop_properties.procview.details_flags &
	(1 << GTOP_DETAILS_AUTO_UPDATE);

    if (d->timeout_id) {
	gtk_timeout_remove (d->timeout_id);
	d->timeout_id = 0;
    }

    if (!auto_update)
	return;

    d->timeout_id = gtk_timeout_add
	(gtop_properties.global.update_times [GTOP_UPDATE_DETAILS],
	 do_update, (gpointer) d);
}

static GtkWidget *
create_clist (const gchar * titles[], const gint widths [], const gint columns)
{
    GtkCList *list;
    gint cw, i;
    
    list = GTK_CLIST (gtk_clist_new_with_titles (columns, (gchar **)titles));

    gtk_clist_set_shadow_type (list, GTK_SHADOW_OUT);

    /* Fixme, eventually you might could select an item 
       for cut and paste, or some other effect. */
    gtk_clist_set_shadow_type (list, GTK_SHADOW_OUT);
    gtk_clist_set_selection_mode (list, GTK_SELECTION_BROWSE);
    gtk_clist_column_titles_passive (list);

    cw = gdk_char_width (gtop_properties.procview.font, 'M');

    for (i = 0; i < 3; i++)
	gtk_clist_set_column_width (list, i, cw * widths [i]);

    return GTK_WIDGET (list);
}

/* Want a function not a macro, so a and b are calculated only once */
static gint
max (gint a, gint b)
{
    if (a > b)
	return a;
    else
	return b;
}

static void
fill_clist (GtkCList *list, const gint widths [],
	    const gchar **col_items [], gint numitems,
	    gint columns)
{
    const gchar **row;
    gint *col_widths, total_width;
    GtkRequisition req;
    GdkFont *font;
    gint cw, i;

    row = g_new0 (const gchar *, columns);
    col_widths = g_new0 (gint, columns);

    gtk_clist_freeze (list);
    gtk_clist_clear (list);

    gtk_widget_ensure_style(GTK_WIDGET(list));
    font = gtk_widget_get_style (GTK_WIDGET(list))->font;
    cw = gdk_char_width (font, 'M');

    for (i = 0; i < columns; i++ ){
	col_widths [i] = widths [i] * cw;
    }

    i = 0;
    while (i < numitems) {
	gint j;

	for (j = 0; j < columns; j++) {
	    row [j] = col_items [i][j] ? col_items [i][j] : "";
	}

	gtk_clist_append (list, (gchar **) row);
	gtk_clist_set_selectable (list, i, FALSE);

	/* If the string is longer than any previous ones,
	   increase the column width */
    
	for (j = 0; j < columns; j++) {
	    col_widths [j] = max (gdk_string_width (font, row [j]),
				  col_widths [j]);
	}

	++i;
    }

    /* The first column is a little wider than the largest string, so 
       it's not too close to the second column. */

    total_width = 0;
    for (i = 0; i < columns-1; i++) {
	gtk_clist_set_column_width (list, i, col_widths [i] + 3*cw);
	total_width += col_widths [i] + 30;
    }

    gtk_clist_set_column_width (list, columns-1, col_widths [columns-1] + 2*cw);
    total_width += col_widths [columns-1] + 8*cw;

    gtk_clist_thaw (list);

    gtk_widget_size_request (GTK_WIDGET (list), &req);
    gtk_widget_set_usize (GTK_WIDGET (list), total_width, req.height);

    g_free (row);
    g_free (col_widths);
}

static void
process_state_create (GTopProcessDetails *d)
{
    GtkStyle *style;
    GtkWidget *vb, *l;

    vb = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

    l = gtk_label_new (_("Process State:"));
    gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (l), GNOME_PAD_SMALL, 0);
    gtk_widget_ensure_style(l);

    style = gtk_style_copy(gtk_widget_get_style (l));
    style->font = gtop_properties.procview.font;
    gtk_widget_set_style (l, style);

    gtk_box_pack_start (GTK_BOX (vb), l, TRUE, FALSE, 0);
    gtk_widget_show (l);

    d->status_clist = create_clist (state_names, state_widths, STATE_FIELDS);

    gtk_container_border_width
	(GTK_CONTAINER (d->status_clist), GNOME_PAD_SMALL);

    gtk_box_pack_start (GTK_BOX (vb), d->status_clist, FALSE, FALSE, 0);

    gtk_widget_show (d->status_clist);

    gtk_box_pack_start (GTK_BOX (d->vb), vb, FALSE, FALSE, GNOME_PAD);

    gtk_widget_show (vb);
}

static void
process_state_update (GTopProcessDetails *d)
{
    const gchar *data [STATE_FIELDS], **ptr [1];

    d->p = get_proc_data (d->p, d->pid);

    memset (data, 0, sizeof (data));

    data [STATE_NAME]		= g_strdup (d->p->cmd);
    data [STATE_USER]		= g_strdup (d->p->user);
    data [STATE_PID]		= g_strdup_printf ("%d", (int) d->p->pid);
    data [STATE_PRIORITY]	= g_strdup_printf ("%d", (int) d->p->priority);
    data [STATE_NICE]		= g_strdup_printf ("%d", (int) d->p->nice);
    data [STATE_MEM]		= g_strdup_printf
	("%2d.%1d", (int) d->p->pmem / 10, (int) d->p->pmem % 10);
    data [STATE_STATE]		= d->p->state_string;

    if (d->p->update_count > 1) {
	data [STATE_CPU]	= g_strdup_printf
	    ("%2d.%1d", (int) d->p->pcpu / 10, (int) d->p->pcpu % 10);
	data [STATE_WCPU]	= g_strdup_printf
	    ("%2d.%1d", (int) d->p->wcpu / 10, (int) d->p->wcpu % 10);
    }

    ptr [0] = data;

    fill_clist (GTK_CLIST (d->status_clist),
		state_widths, ptr, 1, STATE_FIELDS);
}

static void
process_args_create (GTopProcessDetails *d)
{
    GtkStyle *style;
    GtkWidget *l, *vb;

    vb = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

    l = gtk_label_new (_("Command Line:"));
    gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (l), GNOME_PAD_SMALL, 0);
    gtk_widget_ensure_style(l);

    style = gtk_style_copy(gtk_widget_get_style (l));
    style->font = gtop_properties.procview.font;
    gtk_widget_set_style (l, style);

    gtk_box_pack_start (GTK_BOX (vb), l, TRUE, FALSE, 0);
    gtk_widget_show (l);

    d->args_clist = create_clist (args_names, args_widths, ARGS_FIELDS);

    gtk_container_border_width
	(GTK_CONTAINER (d->args_clist), GNOME_PAD_SMALL);

    gtk_box_pack_start (GTK_BOX (vb), d->args_clist, FALSE, FALSE, 0);

    gtk_widget_show (d->args_clist);

    gtk_box_pack_start (GTK_BOX (d->vb), vb, FALSE, FALSE, GNOME_PAD);

    gtk_widget_show (vb);
}

#if LIBGTOP_VERSION_CODE >= 1001004

static void
process_args_update (GTopProcessDetails *d)
{
    const gchar *data [ARGS_FIELDS], **ptr [1];
    char *command_line = NULL, **args;
    glibtop_array array;
    size_t len = 0;
    int i;
    
    args = glibtop_get_proc_args (&array, d->pid);

    if (args) {
	for (i = 0; i < array.number; i++)
	    len += strlen (args [i]);

	command_line = g_malloc0 (len+1);
	memset (command_line, 0, len);

	for (i = 0; i < array.number; i++) {
	    strcat (command_line, args [i]);
	    glibtop_free (args [i]);
	}

	glibtop_free (args);
    }

    data [ARGS_CMDLINE] = command_line;

    ptr [0] = data;

    fill_clist (GTK_CLIST (d->args_clist),
		args_widths, ptr, 1, ARGS_FIELDS);
}


#else /* LIBGTOP_VERSION_CODE < 1001004 */

static void
process_args_update (GTopProcessDetails *d)
{
    const gchar *data [ARGS_FIELDS], **ptr [1], *args;
    gchar *command_line = NULL;
    glibtop_proc_args procargs;
    gint i;
    
    args = glibtop_get_proc_args
	(&procargs, d->pid, args_widths [ARGS_CMDLINE]);

    if (args) {
	command_line = g_malloc0 (procargs.size+1);

	memcpy (command_line, args, procargs.size);
	command_line [procargs.size] = '\0';

	for (i = 0; i < procargs.size; i++)
	    if (command_line [i] == '\0')
		command_line [i] = ' ';
    }

    data [ARGS_CMDLINE] = command_line;

    ptr [0] = data;

    fill_clist (GTK_CLIST (d->args_clist),
		args_widths, ptr, 1, ARGS_FIELDS);
}

#endif /* LIBGTOP_VERSION_CODE < 1001004 */

static void
process_credentials_create (GTopProcessDetails *d)
{
    GtkStyle *style;
    GtkWidget *l, *vb;

    vb = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

    l = gtk_label_new (_("Process Credentials:"));
    gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (l), GNOME_PAD_SMALL, 0);
    gtk_widget_ensure_style(l);

    style = gtk_style_copy(gtk_widget_get_style (l));
    style->font = gtop_properties.procview.font;
    gtk_widget_set_style (l, style);

    gtk_box_pack_start (GTK_BOX (vb), l, TRUE, FALSE, 0);
    gtk_widget_show (l);

    d->credential_clist = create_clist
	(credential_names, credential_widths, CREDENTIAL_FIELDS);

    gtk_container_border_width
	(GTK_CONTAINER (d->credential_clist), GNOME_PAD_SMALL);

    gtk_box_pack_start (GTK_BOX (vb), d->credential_clist, FALSE, FALSE, 0);

    gtk_widget_show (d->credential_clist);

    gtk_box_pack_start (GTK_BOX (d->vb), vb, FALSE, FALSE, GNOME_PAD);

    gtk_widget_show (vb);
}

static gchar *
getpwuid_string (uid_t uid)
{
    struct passwd *pwd = getpwuid (uid);

    if (pwd)
	return g_strdup (pwd->pw_name);
    else
	return g_strdup_printf ("<%d>", uid);
}

static void
process_credentials_update (GTopProcessDetails *d)
{
    const gchar *row_0 [CREDENTIAL_FIELDS];
    const gchar *row_1 [CREDENTIAL_FIELDS];
    const gchar **ptr [2];

    glibtop_proc_uid procuid;

    glibtop_get_proc_uid (&procuid, d->pid);

    row_0 [CREDENTIALS_RUID] = g_strdup_printf ("%d", procuid.uid);
    row_0 [CREDENTIALS_RGID] = g_strdup_printf ("%d", procuid.gid);

    row_0 [CREDENTIALS_EUID] = g_strdup_printf ("%d", procuid.euid);
    row_0 [CREDENTIALS_EGID] = g_strdup_printf ("%d", procuid.egid);

#if (LIBGTOP_VERSION_CODE > 1001000)
    row_0 [CREDENTIALS_SUID] = g_strdup_printf ("%d", procuid.suid);
    row_0 [CREDENTIALS_SGID] = g_strdup_printf ("%d", procuid.sgid);

    row_0 [CREDENTIALS_FSUID] = g_strdup_printf ("%d", procuid.fsuid);
    row_0 [CREDENTIALS_FSGID] = g_strdup_printf ("%d", procuid.fsgid);
#endif

    row_1 [CREDENTIALS_RUID] = getpwuid_string (procuid.uid);
    row_1 [CREDENTIALS_RGID] = getpwuid_string (procuid.gid);

    row_1 [CREDENTIALS_EUID] = getpwuid_string (procuid.euid);
    row_1 [CREDENTIALS_EGID] = getpwuid_string (procuid.egid);

#if (LIBGTOP_VERSION_CODE > 1001000)
    row_1 [CREDENTIALS_SUID] = getpwuid_string (procuid.suid);
    row_1 [CREDENTIALS_SGID] = getpwuid_string (procuid.sgid);

    row_1 [CREDENTIALS_FSUID] = getpwuid_string (procuid.fsuid);
    row_1 [CREDENTIALS_FSGID] = getpwuid_string (procuid.fsgid);
#endif

    ptr [0] = row_0;
    ptr [1] = row_1;

    fill_clist (GTK_CLIST (d->credential_clist),
		credential_widths, ptr, 2, CREDENTIAL_FIELDS);
}

