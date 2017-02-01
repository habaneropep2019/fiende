#include <config.h>

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <glibtop.h>
#include <glibtop/xmalloc.h>
#include <glibtop/union.h>

#include <gnome.h>

#include <properties.h>

#include <gtop-fsusage.h>

#define PROC_CMD_LEN 40

typedef struct _FsUsageProcInfo		FsUsageProcInfo;

struct _FsUsageProcInfo {
	char          *cmd;
	unsigned long  percent;
	gint64         value;
};

static glibtop_mountentry *mount_list = NULL;
static FsUsageProcInfo *fsusage_data = NULL;
static gint fsusage_number = 0;

static GtkWidget *fsusage_properties_init (GnomePropertyObject *);
static void	fsusage_properties_update (GnomePropertyObject *);
static void	fsusage_properties_load (GnomePropertyObject *);
static void	fsusage_properties_save (GnomePropertyObject *);

static gpointer fsusage_data_fn (Graph *, GraphCmd, gpointer, gint64 *);

#define GTOP_NUM_FSTYPES	7

static const gchar *gtop_fstype_names [GTOP_NUM_FSTYPES]  = {
	N_("ufs"),
	N_("nfs"),
	N_("msdos"),
	N_("iso9660"),
	N_("ext2"),
	N_("minix"),
	N_("other")
};

static const gchar *gtop_fstype_labels [GTOP_NUM_FSTYPES] = {
	N_("User Filesystem (FreeBSD)"),
	N_("Network Filesystem"),
	N_("MSDOS Filesystem"),
	N_("CDROM Filesystem"),
	N_("Second Extended Filesystem (Linux)"),
	N_("Minix Filesystem (Linux)"),
	N_("Other Filesystems")
};

GnomePropertyDescriptor FsUsageProperty_Descriptor = {
	sizeof (GTopFsUsageProperties),
	N_("File System Usage"),
	fsusage_properties_init,
	NULL,
	fsusage_properties_update,
	fsusage_properties_load,
	fsusage_properties_save,
	NULL, NULL, NULL, NULL, NULL
};

static GList *widget_list = NULL;

extern void gtop_show_mtbar (GtkWidget *w, gpointer gp);

static GtkWidget *
fsusage_menu_prepare ()
{
	GtkWidget *menu;
	GtkWidget *mi;

	menu = gtk_menu_new ();

	mi = gtk_menu_item_new_with_label (_("Show menubar"));
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (gtop_show_mtbar),
			    (gpointer) FALSE);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	mi = gtk_menu_item_new_with_label (_("Show toolbar"));
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (gtop_show_mtbar),
			    (gpointer) TRUE);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	return menu;
}

static void
fsusage_button_press (GtkCList *cl, GdkEventButton *e,
		       GtkMenu *m)
{
	if (e->button == 3)
		gtk_menu_popup (m, NULL, NULL, NULL,
				NULL, e->button, e->time);
}

void
fsusage_new (GTopFsUsageData *d, GtkWidget *widget, gint ftype)
{
	GTopFsUsage *fsusage = GTOP_FSUSAGE (widget);

	d->sw = GTK_WIDGET (GTK_SCROLLED_WINDOW (fsusage));
	d->ftype = (GTopFsUsageType) ftype;

	gtk_scrolled_window_set_policy
		(GTK_SCROLLED_WINDOW (d->sw),
		 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_name (d->sw, "FsUsageGraph");

	d->graph = gtop_graph_new (widget, fsusage_data_fn, (gpointer) d);

	gtk_scrolled_window_add_with_viewport
		(GTK_SCROLLED_WINDOW (d->sw), d->graph);

	gtk_widget_show (d->graph);

	gtk_signal_connect (GTK_OBJECT (d->sw), "button_press_event",
			    fsusage_button_press, fsusage_menu_prepare ());

	fsusage_type_set (d, ftype);

	widget_list = g_list_append (widget_list, widget);
}

void
fsusage_destroy (GtkWidget *widget)
{
	widget_list = g_list_remove (widget_list, widget);
}

void
fsusage_type_set (GTopFsUsageData *d, gint ftype)
{
	GTopPropFsMode fsmode = gtop_properties.fsusage.fsmode;

	d->ftype = ftype;

	switch (fsmode) {
	case GTOP_FSMODE_SUBLOCKS:
		switch (d->ftype) {
		case GTOP_FSUSAGE_TOTAL:
			d->graph_head = _("Total Filesystem Sizes "
					  "(including reserved blocks)");
			d->graph_tail = _("Sum of Total Sizes: %ldk");
			break;
		case GTOP_FSUSAGE_USED:
			d->graph_head = _("Used Space on Filesystems "
					  "(including reserved blocks)");
			d->graph_tail = _("Sum of Uses Space: %ldk");
			break;
		case GTOP_FSUSAGE_FREE:
			d->graph_head = _("Free Space on Filesystems "
					  "(including reserved blocks)");
			d->graph_tail = _("Sum of Free Space: %ldk");
			break;
		}
		break;
	case GTOP_FSMODE_BLOCKS:
		switch (d->ftype) {
		case GTOP_FSUSAGE_TOTAL:
			d->graph_head = _("Total Filesystem Sizes");
			d->graph_tail = _("Sum of Total Sizes: %ldk");
			break;
		case GTOP_FSUSAGE_USED:
			d->graph_head = _("Used Space on Filesystems");
			d->graph_tail = _("Sum of Uses Space: %ldk");
			break;
		case GTOP_FSUSAGE_FREE:
			d->graph_head = _("Free Space on Filesystems");
			d->graph_tail = _("Sum of Free Space: %ldk");
			break;
		}
		break;
	case GTOP_FSMODE_INODES:
		switch (d->ftype) {
		case GTOP_FSUSAGE_TOTAL:
			d->graph_head = _("Total Number of Inodes");
			d->graph_tail = _("Sum of Total Number of Inodes: %ld");
			break;
		case GTOP_FSUSAGE_USED:
			d->graph_head = _("Used Inodes on Filesystems");
			d->graph_tail = _("Sum of Used Inodes: %ld");
			break;
		case GTOP_FSUSAGE_FREE:
			d->graph_head = _("Free Inodes on Filesystems");
			d->graph_tail = _("Sum of Free Inodes: %ld");
			break;
		}
		break;
	}
}

static gpointer
fsusage_data_fn (Graph *graph, GraphCmd cmd, gpointer data, gint64 *ret)
{
	FsUsageProcInfo *info = data;
	GTopFsUsageData *d = (GTopFsUsageData *) graph->user_data;
	GTopPropFsMode fsmode = gtop_properties.fsusage.fsmode;
	static gchar buf [256];
	static gchar tail [256];

	switch (cmd) {

	case GRAPH_FIRST:

		return (fsusage_data) ? (fsusage_data [0].cmd) ?
			fsusage_data : 0 : 0;

	case GRAPH_NEXT:

		return (info [1].cmd) ? info+1 : NULL;

	case GRAPH_VALUE:

		if (ret) {
			*ret = info->value;
			return ret;
		}

		break;

	case GRAPH_LABEL:

		switch (fsmode) {
		case GTOP_FSMODE_SUBLOCKS:
		case GTOP_FSMODE_BLOCKS:
			sprintf (buf, "%s (%3ld%% free) : %8ldk",
				 info->cmd, info->percent,
				 (unsigned long) info->value);
			break;
		case GTOP_FSMODE_INODES:
			sprintf (buf, "%s (%3ld%% free) : %8ld ",
				 info->cmd, info->percent,
				 (unsigned long) info->value);
			break;
		}

		return buf;

	case GRAPH_HEAD:

		return d->graph_head;

	case GRAPH_TAIL:

		sprintf (tail, d->graph_tail, d->value_total >> 10);

		return d->graph_tail ? tail : NULL;

	}

	return NULL;
}

static int
fsusage_cmd_cmp (const void *i1,
		  const void *i2)
{
	return strcmp (((FsUsageProcInfo *)i1)->cmd,
		       ((FsUsageProcInfo *)i2)->cmd);
}

gint
fsusage_update (GTopFsUsageData *d)
{
	FsUsageProcInfo *ti;
	gint n = 0, i, j, k = 0;
	unsigned long value, percent;
	glibtop_mountlist mountlist;
	GTopPropFsMode fsmode;
	glong selected_fs_mask;
	gint selected_fs;

	if (!d->graph)
		return FALSE;

	if (mount_list)
		glibtop_free (mount_list);

	memset (&mountlist, 0, sizeof (glibtop_mountlist));

	mount_list = glibtop_get_mountlist (&mountlist, 0);

	n = mountlist.number;

	/* temporary info */

	ti = g_new0 (FsUsageProcInfo, n);

	value = 0; /* keep gcc happy */
	d->value_total = 0;
	percent = 0;

	fsmode = gtop_properties.fsusage.fsmode;
	selected_fs = gtop_properties.fsusage.selected_fs;
	selected_fs_mask = gtop_properties.fsusage.selected_fs_mask;

	for (i=0, k=0; i<n; i++) {
		glibtop_fsusage fsusage;

		if (selected_fs) {
			gchar *type = mount_list [i].type ?
				mount_list [i].type : "other";
			gint selected = 0;

			for (j = 0; j < GTOP_NUM_FSTYPES; j++) {
				const gchar *name = gtop_fstype_names [j];

				if (!(selected_fs_mask & (1 << j)))
					continue;

				if (!strcmp (name, "other"))
					selected = 1;

				if (!strncmp (type, name, strlen (name)))
					selected = 1;
			}

			if (!selected)
				continue;
		}

		glibtop_get_fsusage (&fsusage, mount_list [i].mountdir);

		switch (fsmode) {
		case GTOP_FSMODE_SUBLOCKS:
			switch (d->ftype) {
			case GTOP_FSUSAGE_TOTAL:
				value = fsusage.blocks;
				break;
			case GTOP_FSUSAGE_USED:
				value = fsusage.blocks - fsusage.bfree;
				break;
			case GTOP_FSUSAGE_FREE:
				value = fsusage.bfree;
				break;
			}
			percent = fsusage.blocks ?
				(fsusage.bfree * 100 / fsusage.blocks) : 0;
			break;
		case GTOP_FSMODE_BLOCKS:
			switch (d->ftype) {
			case GTOP_FSUSAGE_TOTAL:
				value = fsusage.blocks;
				break;
			case GTOP_FSUSAGE_USED:
				value = fsusage.blocks - fsusage.bavail;
				break;
			case GTOP_FSUSAGE_FREE:
				value = fsusage.bavail;
				break;
			}
			percent = fsusage.blocks ?
				(fsusage.bavail * 100 / fsusage.blocks) : 0;
			break;
		case GTOP_FSMODE_INODES:
			switch (d->ftype) {
			case GTOP_FSUSAGE_TOTAL:
				value = fsusage.files;
				break;
			case GTOP_FSUSAGE_USED:
				value = fsusage.files - fsusage.ffree;
				break;
			case GTOP_FSUSAGE_FREE:
				value = fsusage.ffree;
				break;
			}
			percent = fsusage.files ?
				(fsusage.ffree * 100 / fsusage.files) : 0;
			break;
		}

		value >>= 1;
		if (value == 0)
			continue;

		ti [k].cmd       = g_strdup (mount_list [i].mountdir);
		ti [k].percent   = percent;
		ti [k].value     = value;
		d->value_total  += value << 10;
		k++;
	}

	n = k;

	if (!n) { /* avoids some trouble. */
		ti [0].cmd     = g_strdup (_("unknown"));
		ti [0].percent = 0;
		ti [0].value   = 0;
		d->value_total = 0;
		n = 1;
	}

	/*
	 * sort info by cmd
	 * we need same processes cmd grouped
	 *
	 */

	qsort (ti, n, sizeof (FsUsageProcInfo), fsusage_cmd_cmp);

	if (fsusage_data) {
		for (i = 0; i < fsusage_number; i++)
			g_free ((fsusage_data+i)->cmd);
		g_free (fsusage_data);
	}

	fsusage_number = n;
	fsusage_data = g_new0 (FsUsageProcInfo, n+1);

	memset (fsusage_data, 0, sizeof (FsUsageProcInfo) * (n+1));
	
	memcpy (fsusage_data, ti, sizeof (FsUsageProcInfo) * n);

	g_free (ti);

	gtk_signal_emit_by_name (GTK_OBJECT (d->graph), "redraw_graph");

	return TRUE;
}

static void
fsusage_properties_update (GnomePropertyObject *object)
{
	GList *c;

	for (c = widget_list; c; c = c->next) {
		GTopFsUsage *fsusage = GTOP_FSUSAGE ((GtkWidget *) c->data);

		fsusage_type_set (&fsusage->data, fsusage->data.ftype);

		fsusage_update (&fsusage->data);
	}
}

static void
selected_fs_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopFsUsageProperties *prop_ptr = cb_data->object->temp_data;

	prop_ptr->selected_fs = GTK_TOGGLE_BUTTON (cb_data->button)->active;

	gtop_properties_changed ();
}

static void
selected_fsmask_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopFsUsageProperties *prop_ptr = cb_data->object->temp_data;

	if (GTK_TOGGLE_BUTTON (cb_data->button)->active)
		prop_ptr->selected_fs_mask |= (1 << cb_data->index);
	else
		prop_ptr->selected_fs_mask &= ~(1 << cb_data->index);

	gtop_properties_changed ();
}

static void
radio_sublocks_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopFsUsageProperties *prop_ptr = cb_data->object->temp_data;

	if (!GTK_TOGGLE_BUTTON (cb_data->button)->active)
		return;

	prop_ptr->fsmode = GTOP_FSMODE_SUBLOCKS;

	gtop_properties_changed ();
}

static void
radio_blocks_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopFsUsageProperties *prop_ptr = cb_data->object->temp_data;

	if (!GTK_TOGGLE_BUTTON (cb_data->button)->active)
		return;

	prop_ptr->fsmode = GTOP_FSMODE_BLOCKS;

	gtop_properties_changed ();
}

static void
radio_inodes_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopFsUsageProperties *prop_ptr = cb_data->object->temp_data;

	if (!GTK_TOGGLE_BUTTON (cb_data->button)->active)
		return;

	prop_ptr->fsmode = GTOP_FSMODE_INODES;

	gtop_properties_changed ();
}

static GtkWidget *
fsusage_properties_init (GnomePropertyObject *object)
{
	GtkWidget *vb, *frame, *label, *button, *table;
	GTopFsUsageProperties *temp_properties = &gtop_properties.fsusage;
	RadioButtonCbData *cb_data;
	GSList *group;
	gint i;

	vb = gtk_vbox_new (FALSE, 0);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	frame = gtk_frame_new (_("Select information to show"));
	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	button = gtk_radio_button_new_with_label
		(NULL, _("Show 1k-blocks reserved blocks"));
	if (temp_properties->fsmode == GTOP_FSMODE_SUBLOCKS)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);
	
	cb_data = g_new0 (RadioButtonCbData, 1);

	cb_data->button = button;
	cb_data->object = object;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_sublocks_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);

	group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label (group, _("Show 1k-blocks"));
	if (temp_properties->fsmode == GTOP_FSMODE_BLOCKS)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);

	cb_data->button = button;
	cb_data->object = object;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_blocks_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 1, 2);

	group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label (group, _("Show inodes"));
	if (temp_properties->fsmode == GTOP_FSMODE_INODES)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);

	cb_data->button = button;
	cb_data->object = object;

	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_inodes_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, 0, 1);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	frame = gtk_frame_new ("");
	table = gtk_table_new (GTOP_NUM_FSTYPES+2, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD << 1);

	label = gtk_check_button_new_with_label
		(_("Only show selected filesystems"));
	if (temp_properties->selected_fs)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", selected_fs_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, 0, 1);

	for (i = 0; i < GTOP_NUM_FSTYPES; i++) {
		RadioButtonCbData *cb_data;

		label = gtk_check_button_new_with_label
			(_(gtop_fstype_names [i]));

		if (temp_properties->selected_fs_mask & (1 << i))
			gtk_toggle_button_set_state
				(GTK_TOGGLE_BUTTON (label), TRUE);

		cb_data = g_new0 (RadioButtonCbData, 1);

		cb_data->index = i;
		cb_data->button = label;
		cb_data->object = object;

		gtk_signal_connect (GTK_OBJECT (label), "toggled",
				    selected_fsmask_cb, cb_data);

		gtk_table_attach_defaults
			(GTK_TABLE (table), label, 1, 2, i+1, i+2);

		label = gtk_label_new (_(gtop_fstype_labels [i]));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_table_attach_defaults
			(GTK_TABLE (table), label, 2, 3, i+1, i+2);
	}

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	return vb;
}

static void
fsusage_properties_load (GnomePropertyObject *object)
{
	gtop_properties.fsusage.fsmode =
		gnome_config_get_int ("gtop/fsusage/fsmode=1");

	gtop_properties.fsusage.selected_fs =
		gnome_config_get_int ("gtop/fsusage/selected_fs=0");

	gtop_properties.fsusage.selected_fs_mask =
		gnome_config_get_int ("gtop/fsusage/selected_fs_mask=0");
}

static void
fsusage_properties_save (GnomePropertyObject *object)
{
	gnome_config_set_int
		("gtop/fsusage/fsmode", gtop_properties.fsusage.fsmode);

	gnome_config_set_int
		("gtop/fsusage/selected_fs", gtop_properties.fsusage.selected_fs);

	gnome_config_set_int ("gtop/fsusage/selected_fs_mask",
			      gtop_properties.fsusage.selected_fs_mask);
}
