/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>

#include <glibtop.h>
#include <glibtop/xmalloc.h>
#include <glibtop/union.h>

#ifdef GLIBTOP_INODEDB
#include <glibtop/inodedb.h>
#endif

#include <procview.h>
#include <gtop-graph.h>
#include <details.h>

#define NUM_INFO 15
#define MAP_COLS 7

static void display_details (GTopProcViewData *d, gint pid);

static void add_info (GTopProcViewData *d, gint pid);
static void add_mem_map (GTopProcViewData *d, gint pid);
static void add_mem_graph (GTopProcViewData *d, gint pid);

static void set_mem_map (GTopProcMapRow **rows, GtkWidget *mcl);

static void  destroy_handler (GtkWidget *, GTopProcViewData *);
static void auto_update_cb (GtkWidget *, GTopProcViewData *);
static void close_cb (GtkWidget *, GTopProcViewData *);
static void update_cb (GtkWidget *, GTopProcViewData *);

enum {
    TARGET_STRING,
    TARGET_ROOTWIN
};

GtkTargetEntry gtop_target_table[] = {
    { "gtop-generic-page", 0, TARGET_STRING }
};

void
procview_details (GTopProcViewData *d, gint pid)
{
    GtkWidget *vb;
    gint auto_update;
    gint remember_position;
    gchar *title;
    gchar *form;

    form = _("Process %d Memory Maps");
    title = g_new0 (char, strlen (form)+64);
    sprintf (title, form, pid);

    d->details.pid = pid;

    auto_update = gtop_properties.procview.details_flags &
	(1 << GTOP_DETAILS_AUTO_UPDATE);
    remember_position = gtop_properties.procview.details_flags &
	(1 << GTOP_DETAILS_REMEMBER_POSITION);

    if (!d->details.dwin) {
	GtkWidget *button;

	d->details.dwin = gnome_dialog_new (title, NULL);

	gnome_dialog_append_button_with_pixmap
	    (GNOME_DIALOG (d->details.dwin), _("Update"),
	     GNOME_STOCK_PIXMAP_REFRESH);

	gnome_dialog_append_button
	    (GNOME_DIALOG (d->details.dwin),
	     GNOME_STOCK_BUTTON_CLOSE);

	gtk_widget_set_name (d->details.dwin, "GTopDetails");
		
	vb = GNOME_DIALOG(d->details.dwin)->vbox;
	d->details.nb = gtk_notebook_new ();

	if (d->details.x < 0)
	    d->details.x = gnome_config_get_int
		("gtop/details/x=-1");
	if (d->details.y < 0)
	    d->details.y = gnome_config_get_int
		("gtop/details/y=-1");

	if (!d->details.w)
	    d->details.w = gnome_config_get_int
		("gtop/details/w=920");
	if (!d->details.h)
	    d->details.h = gnome_config_get_int
		("gtop/details/h=500");

	gtk_widget_set_usize (d->details.dwin,
			      d->details.w,
			      d->details.h);
	gtk_window_set_policy (GTK_WINDOW(d->details.dwin),
			       TRUE, TRUE, FALSE);

	gtk_box_pack_start (GTK_BOX (vb), d->details.nb,
			    TRUE, TRUE, GNOME_PAD_SMALL);
		
	button = gtk_check_button_new_with_label
	    (_("Automatically update details dialog"));

	if (auto_update)
	    gtk_toggle_button_set_state
		(GTK_TOGGLE_BUTTON (button), TRUE);
		
	gtk_signal_connect (GTK_OBJECT (button), "toggled",
			    auto_update_cb, (gpointer) d);

	gtk_box_pack_start (GTK_BOX (vb), button,
			    FALSE, TRUE, GNOME_PAD_SMALL);

	gtk_signal_connect
	    (GTK_OBJECT (d->details.dwin), "destroy",
	     GTK_SIGNAL_FUNC (destroy_handler), (gpointer) d);

	gnome_dialog_button_connect
	    (GNOME_DIALOG (d->details.dwin), 1,
	     GTK_SIGNAL_FUNC (close_cb), (gpointer) d);

	gnome_dialog_button_connect
	    (GNOME_DIALOG (d->details.dwin), 0,
	     GTK_SIGNAL_FUNC (update_cb), (gpointer) d);

	gnome_dialog_set_sensitive
	    (GNOME_DIALOG (d->details.dwin), 0, !auto_update);

	gtk_widget_show (button);
	gtk_widget_show (d->details.nb);
	gtk_widget_show (d->details.dwin);



	if (remember_position &&
	    (d->details.x > 0) && (d->details.y > 0))
	    gtk_widget_set_uposition (d->details.dwin,
				      d->details.x, d->details.y);
    } else {
	gtk_window_set_title (GTK_WINDOW (d->details.dwin), title);
    }

    g_free (title);

    display_details (d, pid);
}

static void
destroy_handler (GtkWidget *widget, GTopProcViewData *d)
{
    d->details.dwin = d->details.icl = NULL;
    d->details.mcl = d->details.gswin = NULL;
}

static void
update_cb (GtkWidget *widget, GTopProcViewData *d)
{
    gchar *title;
    gchar *form;

    form = _("Process %d details");
    title = g_new0 (char, strlen (form)+64);
    sprintf (title, form, d->details.pid);

    gtk_window_set_title (GTK_WINDOW (d->details.dwin), title);

    g_free (title);

    display_details (d, d->details.pid);
}

static void
close_cb (GtkWidget *widget, GTopProcViewData *d)
{
    gint x, y, w, h;
	
    gdk_window_get_geometry (GTK_WIDGET (d->details.dwin)->window,
			     &x, &y, &w, &h, NULL);

    gdk_window_get_position (GTK_WIDGET (d->details.dwin)->window,
			     &x, &y);

    d->details.x = x;
    d->details.y = y;
    d->details.w = w;
    d->details.h = h;

    gnome_config_set_int ("gtop/details/x", x);
    gnome_config_set_int ("gtop/details/y", y);
    gnome_config_set_int ("gtop/details/w", w);
    gnome_config_set_int ("gtop/details/h", h);
    gnome_config_sync ();

    gtk_widget_destroy(d->details.dwin);
}

static void
auto_update_cb (GtkWidget *widget, GTopProcViewData *d)
{
    if (GTK_TOGGLE_BUTTON (widget)->active)
	gtop_properties.procview.details_flags |=
	    (1 << GTOP_DETAILS_AUTO_UPDATE);
    else
	gtop_properties.procview.details_flags &=
	    ~(1 << GTOP_DETAILS_AUTO_UPDATE);

    gnome_dialog_set_sensitive (GNOME_DIALOG (d->details.dwin), 0,
				!GTK_TOGGLE_BUTTON (widget)->active);
}

static void
display_details (GTopProcViewData *d, gint pid)
{
    add_info (d, pid);
    add_mem_map (d, pid);
    add_mem_graph (d, pid);
}

static void
set_info (GTopProcViewData *d, gint pid)
{
    ProcProcData data;
    gtop_proc_t *t = NULL;
    GTopProcField *f = gtop_proc_fields;
    GTopProcViewDetails *dt = &d->details;
    gint i;
    gchar *v;
    float old_adj_hval = GTK_CLIST (dt->icl)->hadjustment->value;
    float old_adj_vval = GTK_CLIST (dt->icl)->vadjustment->value;
    gint old_hoffset = GTK_CLIST (dt->icl)->hoffset;
    gint old_voffset = GTK_CLIST (dt->icl)->voffset;
    glibtop_proc_state procstate;
    glibtop_proc_time proctime;
    glibtop_proc_mem procmem;
    glibtop_proc_uid procuid;
    struct passwd *pwd;

    memset (&data._p, 0, sizeof (data._p));
    t = data.p = &data._p;
	
    t->pid = pid;
	
    glibtop_get_proc_state (&procstate, t->pid);
	
    t->cmd = glibtop_strdup (procstate.cmd);
	
    t->state = procstate.state;
	
    pwd = getpwuid (procstate.uid);
    if (pwd)
	t->user = glibtop_strdup (pwd->pw_name);
    else
	t->user = glibtop_strdup (_("<unknown>"));
	
    glibtop_get_proc_uid (&procuid, t->pid);
	
    t->nice     = procuid.nice;
    t->priority = procuid.priority;
	
    glibtop_get_proc_mem (&procmem, t->pid);

    t->size     = (unsigned long) procmem.size >> 10;
    t->rss      = (unsigned long) procmem.rss >> 10;
    t->resident = (unsigned long) procmem.resident >> 10;
    t->share    = (unsigned long) procmem.share >> 10;
	
    glibtop_get_proc_time (&proctime, t->pid);

    t->frequency = proctime.frequency ? proctime.frequency : 1000000;
	
    t->utime  = ((unsigned long) (proctime.utime * 100) / t->frequency);
    t->stime  = ((unsigned long) (proctime.stime * 100) / t->frequency);
    t->cutime = ((unsigned long) (proctime.cutime * 100) / t->frequency);
    t->cstime = ((unsigned long) (proctime.cstime * 100) / t->frequency);
	
    t->start_time =
	((unsigned long) (proctime.start_time * 100) / t->frequency);

    /* FIXME! */
    data.pcpu = data.pmem = 0;

    gtk_clist_freeze (GTK_CLIST (dt->icl));
    for (i = 0; i < NUM_INFO; i++) {
	if (f->label) {
	    v = (t) ? sprint_fmt (&data, f->fmt) : "";
	    gtk_clist_set_text (GTK_CLIST (dt->icl),
				i, 1, v);
	    /* printf ("clist set %s\n", v); */
	    if (t)
		g_free (v);
			
	    f++;
	}
    }

    /* keep old view position - somewhat ughly, but works for now */
    GTK_CLIST (dt->icl)->hadjustment->value = old_adj_hval;
    GTK_CLIST (dt->icl)->vadjustment->value = old_adj_vval;

    GTK_CLIST (dt->icl)->hoffset = old_hoffset;
    GTK_CLIST (dt->icl)->voffset = old_voffset;

    gtk_clist_thaw (GTK_CLIST (dt->icl));
}

static void
add_info (GTopProcViewData *d, gint pid)
{
    GTopProcViewDetails *dt = &d->details;

    if (!dt->icl) {
	GtkWidget *l;
	gint i;
	gchar *t [3];
	GtkStyle *style;

	l = gtk_label_new (_("Process info"));

	t [0] = _("Attribute");
	t [1] = _("Value");
	t [2] = "";

	dt->icl = gtk_clist_new (3);

	gtk_widget_ensure_style(dt->icl);
	style = gtk_style_copy (gtk_widget_get_style (dt->icl));
	style->font = gtop_properties.procview.font;
	gtk_widget_set_style (dt->icl, style);

	for (i = 0; i < 3; i++) {
	    GtkWidget *l = gtk_label_new (t [i]);
	    GtkWidget *hb = gtk_hbox_new (FALSE, 0);

	    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);

	    gtk_widget_ensure_style(l);
	    style = gtk_style_copy (gtk_widget_get_style (l));
	    style->font = gtop_properties.procview.font;
	    gtk_widget_set_style (l, style);

	    gtk_widget_show (l);
	    gtk_widget_show (hb);
		
	    gtk_box_pack_start_defaults (GTK_BOX (hb), l);
	    gtk_clist_set_column_widget (GTK_CLIST (dt->icl), i, hb);
	}

	gtk_clist_column_titles_show (GTK_CLIST (dt->icl));
	gtk_clist_set_column_justification (GTK_CLIST (dt->icl),
					    0, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification (GTK_CLIST (dt->icl),
					    1, GTK_JUSTIFY_RIGHT);

	for (i = 0; i < NUM_INFO; i++) {
	    t [0] = _(gtop_proc_fields [i].long_info);
	    t [1] = t [2] = "";
			
	    gtk_clist_append (GTK_CLIST (dt->icl), t);
	    /* printf ("clist set %s\n", p_fields [i].long_info); */
	}

	gtk_clist_set_column_width (GTK_CLIST (dt->icl), 0, 320);
	gtk_clist_set_column_width (GTK_CLIST (dt->icl), 1, 140);

	gtk_container_border_width
	    (GTK_CONTAINER (dt->icl), GNOME_PAD_SMALL);

	dt->icl_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy
	    (GTK_SCROLLED_WINDOW (dt->icl_sw),
	     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (dt->icl_sw), dt->icl);

	gtk_widget_show (dt->icl_sw);
	gtk_widget_show (dt->icl);
	gtk_widget_show (l);

	gtk_notebook_append_page (GTK_NOTEBOOK (dt->nb), dt->icl_sw, l);
    }

    set_info (d, pid);
}

static GTopProcMapRow **
get_map_rows (pid_t pid, GTopProcMapRow **rs)
{
    GTopProcMapRow *row;
    GTopProcMapRow **rows = NULL;
#ifdef GLIBTOP_INODEDB
    static glibtop_inodedb *inodedb = NULL;
#endif
    GList *c, *list = NULL;
    guint i;

    glibtop_proc_map procmap;
    glibtop_map_entry *maps;

#ifdef GLIBTOP_INODEDB
    if (!inodedb)
	inodedb = glibtop_inodedb_open (0, 0);
#endif

    /* free old rows */

    if (rs) {
	GTopProcMapRow **c = rs;
	/* i = 0; */
	while (*c) {
	    if ((*c)->filename)
		glibtop_free ((*c)->filename);
	    g_free (*c);
	    c++;
	    /* i++; */
	}

	/* printf ("freed: %d\n", i); */
	g_free (rs);
    }

    /* now we sample new ones */

    maps = glibtop_get_proc_map (&procmap, pid);

    if (maps) {
	for (i = 0; i < procmap.number; i++) {
	    unsigned perm = maps [i].perm;
	    const char *filename = NULL;

	    row = g_new0 (GTopProcMapRow, 1);
	    list = g_list_append (list, row);

	    memset (row, 0, sizeof (GTopProcMapRow));

	    row->VMstart = maps [i].start;
	    row->VMend = maps [i].end;
	    row->VMoffset = maps [i].offset;
	    row->dev_minor = maps [i].device & 255;
	    row->dev_major = (maps [i].device >> 8) & 255;
	    row->inode = maps [i].inode;

	    row->flags [0] =
		(perm & GLIBTOP_MAP_PERM_READ) ? 'r' : '-';
	    row->flags [1] =
		(perm & GLIBTOP_MAP_PERM_WRITE) ? 'w' : '-';
	    row->flags [2] =
		(perm & GLIBTOP_MAP_PERM_EXECUTE) ? 'x' : '-';
	    row->flags [3] =
		(perm & GLIBTOP_MAP_PERM_SHARED) ? 's' : '-';
	    if (perm & GLIBTOP_MAP_PERM_PRIVATE)
		row->flags [3] = 'p';

	    row->flags [4] = 0;

	    if (maps [i].flags & (1 << GLIBTOP_MAP_ENTRY_FILENAME))
		filename = glibtop_strdup (maps [i].filename);
			
#ifdef GLIBTOP_INODEDB
	    if (inodedb && (!filename || !strcmp (filename, ""))) {
		filename = glibtop_inodedb_lookup
		    (inodedb, maps [i].device,
		     maps [i].inode);
	    }
#endif

	    if (filename)
		row->filename = (gchar *) filename;
	    else
		row->filename = glibtop_strdup ("");
	}

	rows = g_new0 (GTopProcMapRow *, procmap.number+1);
	rows [procmap.number] = NULL;

	for (i = 0, c = list; i < procmap.number; i++) {
	    rows [i] = c->data;
	    c = c->next;
	}
	g_list_free (list);
    }

    glibtop_free (maps);

    return rows;
}

static void
set_mem_map (GTopProcMapRow **rows, GtkWidget *mcl)
{
    gchar *t [MAP_COLS];
    gchar buf [6][64];
    float old_adj_hval = GTK_CLIST (mcl)->hadjustment->value;
    float old_adj_vval = GTK_CLIST (mcl)->vadjustment->value;
    gint old_hoffset = GTK_CLIST (mcl)->hoffset;
    gint old_voffset = GTK_CLIST (mcl)->voffset;
    gint min_length = 0;

    /* printf ("set mem map\n"); */

    gtk_clist_freeze (GTK_CLIST (mcl));
    gtk_clist_clear (GTK_CLIST (mcl));

    if (rows)
	for (; *rows; rows++) {
	    gint full_pathname, length;
	    gchar *format = (sizeof (void*) == 8) ?
		"%016lx" : "%08lx";

	    sprintf (buf [0], format, (*rows)->VMstart);
	    t [0] = buf [0];
	    sprintf (buf [1], format, (*rows)->VMend);
	    t [1] = buf [1];
	    t [2] = (*rows)->flags;
	    sprintf (buf [2], format, (*rows)->VMoffset);
	    t [3] = buf [2];
	    sprintf (buf [3], "%02hx:%02hx",
		     (*rows)->dev_major,
		     (*rows)->dev_minor);
	    t [4] = buf [3];
	    sprintf (buf [4], "%ld", (*rows)->inode);
	    t [5] = buf [4];
	    t [6] = (*rows)->filename;

	    full_pathname = gtop_properties.procview.details_flags &
		(1 << GTOP_DETAILS_FULL_PATHNAMES);

	    if (!full_pathname) {
		gchar *p = strrchr (t [6], '/');
		if (p) t [6] = p+1;
	    }

	    length = gdk_text_width (gtop_properties.procview.font,
				     t [6], strlen (t [6]));

	    if (length > min_length)
		min_length = length;
			
	    gtk_clist_append (GTK_CLIST (mcl), t);
	}

    gtk_clist_set_column_min_width (GTK_CLIST (mcl), 6, min_length);
    gtk_clist_set_column_max_width (GTK_CLIST (mcl), 6, -1);
	
    /* keep old view position - somewhat ughly, but works for now */
    GTK_CLIST (mcl)->hadjustment->value = old_adj_hval;
    GTK_CLIST (mcl)->vadjustment->value = old_adj_vval;
	
    GTK_CLIST (mcl)->hoffset = old_hoffset;
    GTK_CLIST (mcl)->voffset = old_voffset;

    gtk_clist_thaw (GTK_CLIST (mcl));
}

static void
drag_data_get (GtkWidget *widget, GdkDragContext *context,
	       GtkSelectionData *selection_data, guint info,
	       guint32 time, gchar *config_string)
{
    gtk_selection_data_set (selection_data, selection_data->target,
			    1, config_string, strlen (config_string));
}

GtkWidget *
gtop_details_create_mem_map (gint pid)
{
    GtkWidget *gswin, *f, *l, *mcl, *mcl_sw;
    GTopProcMapRow **rows;
    gchar *t [MAP_COLS];
    GtkStyle *style;
    gint cw, i, pw;

    gswin = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_name (gswin, "TextMemoryMap");
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (gswin),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    f = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_NONE);

    l = gtk_label_new (_("Raw memory map"));

    t [0] = _("VM start");
    t [1] = _("VM end");
    t [2] = _("Flags");
    t [3] = _("VM offset");
    t [4] = _("Device");
    t [5] = _("Inode");
    t [6] = _("Filename");

    mcl = gtk_clist_new (MAP_COLS);
		
    gtk_widget_ensure_style(mcl);	
    style = gtk_style_copy (gtk_widget_get_style (mcl));
    style->font = gtop_properties.procview.font;
    gtk_widget_set_style (mcl, style);

    for (i = 0; i < MAP_COLS; i++) {
	GtkWidget *l = gtk_label_new (t [i]);
	GtkWidget *hb = gtk_hbox_new (FALSE, 0);

	if (i == MAP_COLS-1)
	    gtk_misc_set_alignment (GTK_MISC (l), 0.5, 0.5);
	else
	    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);

	gtk_widget_ensure_style(l);
	style = gtk_style_copy (gtk_widget_get_style (l));
	style->font = gtop_properties.procview.font;
	gtk_widget_set_style (l, style);

	gtk_widget_show (l);
	gtk_widget_show (hb);
		
	gtk_box_pack_start_defaults (GTK_BOX (hb), l);
	gtk_clist_set_column_widget (GTK_CLIST (mcl), i, hb);
    }

    gtk_clist_column_titles_show (GTK_CLIST (mcl));

    cw = gdk_char_width (gtop_properties.procview.font, 'M');

    pw = sizeof (void*)*2 + 1;

    gtk_clist_set_column_width (GTK_CLIST (mcl), 0, cw * pw);
    gtk_clist_set_column_width (GTK_CLIST (mcl), 1, cw * pw);
    gtk_clist_set_column_width (GTK_CLIST (mcl), 2, cw * 5);
    gtk_clist_set_column_width (GTK_CLIST (mcl), 3, cw * pw);
    gtk_clist_set_column_width (GTK_CLIST (mcl), 4, cw * 6);
    gtk_clist_set_column_width (GTK_CLIST (mcl), 5, cw * 7);

    gtk_clist_set_column_justification
	(GTK_CLIST (mcl), 0, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification
	(GTK_CLIST (mcl), 1, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification
	(GTK_CLIST (mcl), 2, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification
	(GTK_CLIST (mcl), 3, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification
	(GTK_CLIST (mcl), 4, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification
	(GTK_CLIST (mcl), 5, GTK_JUSTIFY_RIGHT);

    gtk_container_border_width
	(GTK_CONTAINER (mcl), GNOME_PAD_SMALL);

    mcl_sw = gtk_scrolled_window_new (NULL,NULL);
    gtk_scrolled_window_set_policy
	(GTK_SCROLLED_WINDOW (mcl_sw),
	 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (mcl_sw), mcl);

    gtk_widget_show (mcl_sw);
    gtk_widget_show (mcl);
    gtk_widget_show (l);

    gtk_container_add (GTK_CONTAINER (f), mcl_sw);

    gtk_container_border_width
	(GTK_CONTAINER (gswin), GNOME_PAD_SMALL);
    gtk_container_border_width
	(GTK_CONTAINER (f), GNOME_PAD_SMALL << 1);

    rows = get_map_rows (pid, NULL);
    set_mem_map (rows, mcl);

    gtk_scrolled_window_add_with_viewport
	(GTK_SCROLLED_WINDOW (gswin), f);

    gtk_widget_show (f);
    gtk_widget_show (gswin);

    return gswin;
}

static void
add_mem_map (GTopProcViewData *d, gint pid)
{
    GTopProcViewDetails *dt = &d->details;
 
    if (!dt->mcl) {
	gchar *config_string;
	GtkWidget *l;
	gchar *t [MAP_COLS];
	GtkStyle *style;
	gint cw, i, pw;

	l = gtk_label_new (_("Raw memory map"));

	t [0] = _("VM start");
	t [1] = _("VM end");
	t [2] = _("Flags");
	t [3] = _("VM offset");
	t [4] = _("Device");
	t [5] = _("Inode");
	t [6] = _("Filename");

	dt->mcl = gtk_clist_new (MAP_COLS);
		
	gtk_widget_ensure_style(dt->mcl);
	style = gtk_style_copy (gtk_widget_get_style (dt->mcl));
	style->font = gtop_properties.procview.font;
	gtk_widget_set_style (dt->mcl, style);

	for (i = 0; i < MAP_COLS; i++) {
	    GtkWidget *l = gtk_label_new (t [i]);
	    GtkWidget *hb = gtk_hbox_new (FALSE, 0);

	    if (i == MAP_COLS-1)
		gtk_misc_set_alignment (GTK_MISC (l), 0.5, 0.5);
	    else
		gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);

	    gtk_widget_ensure_style (l);
	    style = gtk_style_copy (gtk_widget_get_style (l));
	    style->font = gtop_properties.procview.font;
	    gtk_widget_set_style (l, style);

	    gtk_widget_show (l);
	    gtk_widget_show (hb);
		
	    gtk_box_pack_start_defaults (GTK_BOX (hb), l);
	    gtk_clist_set_column_widget (GTK_CLIST (dt->mcl), i, hb);
	}

	gtk_clist_column_titles_show (GTK_CLIST (dt->mcl));

	cw = gdk_char_width (gtop_properties.procview.font, 'M');

	pw = sizeof (void*)*2 + 1;

	gtk_clist_set_column_width (GTK_CLIST (dt->mcl), 0, cw * pw);
	gtk_clist_set_column_width (GTK_CLIST (dt->mcl), 1, cw * pw);
	gtk_clist_set_column_width (GTK_CLIST (dt->mcl), 2, cw * 5);
	gtk_clist_set_column_width (GTK_CLIST (dt->mcl), 3, cw * pw);
	gtk_clist_set_column_width (GTK_CLIST (dt->mcl), 4, cw * 6);
	gtk_clist_set_column_width (GTK_CLIST (dt->mcl), 5, cw * 7);

	gtk_clist_set_column_justification
	    (GTK_CLIST (dt->mcl), 0, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification
	    (GTK_CLIST (dt->mcl), 1, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification
	    (GTK_CLIST (dt->mcl), 2, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification
	    (GTK_CLIST (dt->mcl), 3, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification
	    (GTK_CLIST (dt->mcl), 4, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification
	    (GTK_CLIST (dt->mcl), 5, GTK_JUSTIFY_RIGHT);

	gtk_container_border_width
	    (GTK_CONTAINER (dt->mcl), GNOME_PAD_SMALL);

	dt->mcl_sw = gtk_scrolled_window_new (NULL,NULL);
	gtk_scrolled_window_set_policy
	    (GTK_SCROLLED_WINDOW (dt->mcl_sw),
	     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (dt->mcl_sw), dt->mcl);

	gtk_widget_show (dt->mcl_sw);
	gtk_widget_show (dt->mcl);
	gtk_widget_show (l);

	gtk_notebook_append_page
	    (GTK_NOTEBOOK (dt->nb), dt->mcl_sw, l);

	config_string = g_strdup_printf ("generic/text_maps/%d", pid);
		
	gtk_signal_connect (GTK_OBJECT (dt->nb), "drag_data_get",
			    GTK_SIGNAL_FUNC (drag_data_get),
			    config_string);

	gtk_drag_source_set (dt->mcl, GDK_BUTTON1_MASK,
			     gtop_target_table, 1,
			     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	gtk_widget_queue_resize (dt->mcl_sw);
    }

    dt->rows = get_map_rows (pid, dt->rows);

    set_mem_map (dt->rows, dt->mcl);
}

static gchar *
mem_seg (GTopProcMapRow **rs)
{
    GTopProcMapRow *r = *rs;
    gchar *f = r->flags;

    if (r->filename) {
	if (!strcmp (f, "rw-p"))
	    return _("Data");
	else if (!strcmp (f, "r-xp"))
	    return _("Text");
	else if (f[0] == 'r')
	    return (f[1]=='w') ? "RW" : "RO";

    } else {
	if (!strcmp (f, "rwxp") && !*(rs+1))
	    return _("Stack");
	if (!strcmp (f, "---p"))
	    return _("Hole");
	else if (f[0]=='r')
	    return (f[1]=='w') ? "RW" : "RO";
    }

    return "?????";
}

static gpointer
mem_graph_data_fn (Graph *graph, GraphCmd cmd, gpointer data, gint64 *ret)
{
    GTopProcMapRow **rs = data;
    static gchar buf [256];
    char *fname;

    switch (cmd) {
    case GRAPH_FIRST:
	return * (GTopProcMapRow ***) graph->user_data;
    case GRAPH_NEXT:
	return (*(rs+1)) ? rs+1 : NULL;
    case GRAPH_VALUE:
	if (ret) {
	    *ret = (*rs)->VMend - (*rs)->VMstart;
	    return ret;
	}
	return NULL;
    case GRAPH_LABEL:
	if ((*rs)->filename) {
	    fname = strrchr ((*rs)->filename, '/');
	    if (fname)
		fname++;
	    else
		fname = (*rs)->filename;
	} else
	    fname = "";

	sprintf (buf, "(%5s): %8ldk  %-30s",
		 mem_seg (rs),
		 ((*rs)->VMend - (*rs)->VMstart) >> 10,
		 fname);
		
	return buf;
    default:
	return NULL;
    }
}

GtkWidget *
gtop_details_create_mem_graph (gint pid)
{
    GtkWidget *gswin, *f, *da;
    GTopProcMapRow ***row_ptr;

    gswin = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_name (gswin, "GraphMemoryMap");
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (gswin),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    row_ptr = g_new0 (GTopProcMapRow **, 1);
    *row_ptr = get_map_rows (pid, NULL);

    f = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_NONE);
    da = gtop_graph_new (gswin, mem_graph_data_fn, row_ptr);

    gtk_container_border_width
	(GTK_CONTAINER (gswin), GNOME_PAD_SMALL);
    gtk_container_border_width
	(GTK_CONTAINER (f), GNOME_PAD_SMALL << 1);

    gtk_container_add (GTK_CONTAINER (f), da);
    gtk_scrolled_window_add_with_viewport
	(GTK_SCROLLED_WINDOW (gswin), f);

    gtk_widget_show (f);
    gtk_widget_show (da);
    gtk_widget_show (gswin);

    return gswin;
}

static void
add_mem_graph (GTopProcViewData *d, gint pid)
{
    GTopProcViewDetails *dt = &d->details;

    if (!dt->gswin) {
	gchar *config_string;
	GtkWidget *l;
	GtkWidget *da;
	GtkWidget *f;

	l = gtk_label_new (_("Graphical memory map"));
	dt->gswin = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_name (dt->gswin, "GraphMap");
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dt->gswin),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	f = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_NONE);

	da = gtop_graph_new (dt->gswin, mem_graph_data_fn, &dt->rows);

	dt->gg = GTOP_GRAPH (da)->graph;

	gtk_container_border_width
	    (GTK_CONTAINER (dt->gswin), GNOME_PAD_SMALL);
	gtk_container_border_width
	    (GTK_CONTAINER (f), GNOME_PAD_SMALL << 1);

	gtk_container_add (GTK_CONTAINER (f), da);
	gtk_scrolled_window_add_with_viewport
	    (GTK_SCROLLED_WINDOW (dt->gswin), f);

	gtk_widget_show (l);
	gtk_widget_show (da);
	gtk_widget_show (f);
	gtk_widget_show (dt->gswin);

	gtk_notebook_append_page
	    (GTK_NOTEBOOK (dt->nb), dt->gswin, l);

	config_string = g_strdup_printf ("generic/graph_maps/%d", pid);

	gtk_signal_connect (GTK_OBJECT (da), "drag_data_get",
			    GTK_SIGNAL_FUNC (drag_data_get), config_string);

	gtk_drag_source_set (da, GDK_BUTTON1_MASK,
			     gtop_target_table, 1,
			     GDK_ACTION_COPY | GDK_ACTION_MOVE);

	graph_update (dt->gg);
	gtk_widget_queue_resize (dt->gswin);
    }
}
