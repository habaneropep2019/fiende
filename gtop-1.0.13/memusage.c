#include <config.h>

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <glibtop.h>
#include <glibtop/xmalloc.h>
#include <glibtop/union.h>

#include <gnome.h>

#include <properties.h>

#include <gtop-memusage.h>

#define PROC_CMD_LEN 40

typedef struct _MemUsageProcInfo MemUsageProcInfo;

struct _MemUsageProcInfo {
	char          *cmd;
	gint           n;
	gint64         value;
};


static GtkAdjustment *adjustments [MEMUSAGE_FIELDS];

static gpointer
 memusage_data_fn (Graph *, GraphCmd, gpointer, gint64 *);

static unsigned *proc_tab = NULL;
static int memusage_data_count = 0;
static MemUsageProcInfo *memusage_data = NULL;

static GtkWidget *memusage_properties_init (GnomePropertyObject *);
static void	memusage_properties_apply (GnomePropertyObject *);
static void	memusage_properties_update (GnomePropertyObject *);
static void	memusage_properties_load (GnomePropertyObject *);
static void	memusage_properties_save (GnomePropertyObject *);

GnomePropertyDescriptor MemUsageProperty_Descriptor = {
	sizeof (GTopMemUsageProperties),
	N_("Memory Usage"),
	memusage_properties_init,
	memusage_properties_apply,
	memusage_properties_update,
	memusage_properties_load,
	memusage_properties_save,
	NULL, NULL, NULL, NULL, NULL
};

static GList *widget_list = NULL;

extern void gtop_show_mtbar (GtkWidget *w, gpointer gp);

static GtkWidget *
memusage_menu_prepare ()
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
memusage_button_press (GtkCList *cl, GdkEventButton *e,
		       GtkMenu *m)
{
	if (e->button == 3)
		gtk_menu_popup (m, NULL, NULL, NULL,
				NULL, e->button, e->time);
}

void
memusage_new (GTopMemUsageData *d, GtkWidget *widget, gint ftype)
{
	GTopMemUsage *memusage = GTOP_MEMUSAGE (widget);

	d->sw = GTK_WIDGET (GTK_SCROLLED_WINDOW (memusage));
	d->ftype = (GTopMemUsageType) ftype;

	gtk_scrolled_window_set_policy
		(GTK_SCROLLED_WINDOW (d->sw),
		 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_name (d->sw, "MemUsageGraph");

	d->graph = gtop_graph_new (widget, memusage_data_fn, (gpointer) d);

	gtk_scrolled_window_add_with_viewport
		(GTK_SCROLLED_WINDOW (d->sw), d->graph);

	gtk_widget_show (d->graph);

	gtk_signal_connect (GTK_OBJECT (d->sw), "button_press_event",
			    memusage_button_press, memusage_menu_prepare ());

	memusage_type_set (d, ftype);

	widget_list = g_list_append (widget_list, widget);
}

void
memusage_destroy (GtkWidget *widget)
{
	widget_list = g_list_remove (widget_list, widget);
}

void
memusage_type_set (GTopMemUsageData *d, gint ftype)
{
	d->ftype = ftype;

	switch (d->ftype) {
	case GTOP_MEMUSAGE_RESIDENT:
		d->graph_head = _("Resident Sizes of Processes");
		d->graph_tail = _("Sum of Resident Sizes: %ldk");
		break;
	case GTOP_MEMUSAGE_SHARED:
		d->graph_head = _("Shared Sizes of Processes");
		d->graph_tail = _("Sum of Shared Sizes: %ldk");
		break;
	case GTOP_MEMUSAGE_SIZE:
		d->graph_head = _("Total Sizes of Processes");
		d->graph_tail = _("Sum of Total Sizes: %ldk");
		break;
	case GTOP_MEMUSAGE_VIRTUAL:
		d->graph_head = _("Virtual Sizes of Processes");
		d->graph_tail = _("Sum of Virtual Sizes: %ldk");
		break;
	case GTOP_MEMUSAGE_SWAP:
		d->graph_head = _("Swapped Sizes of Processes");
		d->graph_tail = _("Sum of Swapped Sizes: %ldk");
		break;
	}
}

static gpointer
memusage_data_fn (Graph *graph, GraphCmd cmd, gpointer data, gint64 *ret)
{
	MemUsageProcInfo *info = data;
	GTopMemUsageData *d = (GTopMemUsageData *) graph->user_data;
	static gchar buf [256];
	static gchar tail [256];

	switch (cmd) {

	case GRAPH_FIRST:

		return (memusage_data) ? (memusage_data [0].cmd) ?
			memusage_data : 0 : 0;

	case GRAPH_NEXT:

		return (info [1].cmd) ? info+1 : NULL;

	case GRAPH_VALUE:

		if (ret) {
			*ret = info->value;
			return ret;
		}

		break;

	case GRAPH_LABEL:

		if (info->n > 1)
			sprintf (buf, "%s (%2d) : %8ldk",
				 info->cmd, info->n,
				 (unsigned long) info->value);
		else
			sprintf (buf, "%s      : %8ldk",
				 info->cmd, (unsigned long) info->value);

		return buf;

	case GRAPH_HEAD:

		return d->graph_head;

	case GRAPH_TAIL:

		sprintf (tail, d->graph_tail, d->value_total >> 10);

		return tail;

	}

	return NULL;
}

static int
memusage_cmd_cmp (const void *i1,
		  const void *i2)
{
	return strcmp (((MemUsageProcInfo *)i1)->cmd,
		       ((MemUsageProcInfo *)i2)->cmd);
}

gint
memusage_update (GTopMemUsageData *d)
{
	MemUsageProcInfo *ti;
	gint n = 0, i, j, k = 0, l;
	unsigned long value, threshold;
	char *cmd;
	static gchar bts [256];
	glibtop_proclist proclist;

	if (!d->graph)
		return FALSE;

	if (proc_tab)
		glibtop_free (proc_tab);

	memset (&proclist, 0, sizeof (glibtop_proclist));

	switch (gtop_properties.memusage.proc_select) {
	case GTOP_PROC_SELECT_ALL:
		proc_tab = glibtop_get_proclist (&proclist, 0, 0);
		break;
	case GTOP_PROC_SELECT_USER:
		proc_tab = glibtop_get_proclist
			(&proclist,
			 GLIBTOP_KERN_PROC_UID,
			 getuid ());
		break;
	case GTOP_PROC_SELECT_TTY:
		proc_tab = glibtop_get_proclist
			(&proclist,
			 GLIBTOP_KERN_PROC_UID | GLIBTOP_EXCLUDE_NOTTY,
			 getuid ());
		break;
	}

	n = proclist.number;

	/* temporary info */

	ti = g_new0 (MemUsageProcInfo, n);

	value = 0; /* keep gcc happy */
	d->value_total = 0;

	for (i = 0; i < n; i++) {
		glibtop_proc_state procstate;
		glibtop_proc_mem procmem;

		glibtop_get_proc_state (&procstate, proc_tab [i]);

		ti [i].cmd = glibtop_strdup (procstate.cmd);

		glibtop_get_proc_mem (&procmem, proc_tab [i]);

		switch (d->ftype) {
		case GTOP_MEMUSAGE_RESIDENT:
			value = procmem.rss;
			break;
		case GTOP_MEMUSAGE_SHARED:
			value = procmem.share;
			break;
		case GTOP_MEMUSAGE_SIZE:
			value = procmem.size;
			break;
		case GTOP_MEMUSAGE_VIRTUAL:
			value = procmem.vsize;
			break;
		case GTOP_MEMUSAGE_SWAP:
			value = (procmem.size > procmem.resident) ?
				(procmem.size - procmem.resident) : 0;
			break;
		}
		ti [i].value = value >> 10;
		d->value_total += value;
	}

	threshold = gtop_properties.memusage.thresholds [d->ftype];

	/*
	 * sort info by cmd
	 * we need same processes cmd grouped
	 *
	 */

	qsort (ti, n, sizeof (MemUsageProcInfo), memusage_cmd_cmp);

	/*
	 * compute # of processes > threshold to k
	 *
	 */

	for (i=0, k=0; i<n;) {
		
		value = 0;
		cmd = ti [i].cmd;

		do {
			value += ti [i].value;
			i++;

		} while (i<n && !strcmp (cmd, ti [i].cmd));

		if (value >= threshold)
			k++;
	}

	if (memusage_data) {
		for (i=0; i < memusage_data_count; i++)
			g_free (memusage_data [i].cmd);

		g_free (memusage_data);
	}

	memusage_data = g_new0 (MemUsageProcInfo, k+2);
	memusage_data_count = k+2;

	sprintf (bts, "< %ldk", threshold);
	memusage_data [k].cmd = g_strdup (bts);
	memusage_data [k].value = 0;
	memusage_data [k].n = 0;

	memusage_data [k+1].cmd = NULL;

	/*
	 * second pass
	 *
	 *   store data and fill Below field
	 *
	 */
	
	for (i=0, j=0; i<n;) {

		value = 0;
		l   = 0;
		cmd = ti [i].cmd;

		do {
			value += ti [i].value;

			i++;
			l++;

		} while (i<n && !strcmp (cmd, ti [i].cmd));

		if (value >= threshold) {

			memusage_data [j].cmd = g_strdup (cmd);
			memusage_data [j].n = l;
			memusage_data [j].value = value;
			j++;

		} else {
			memusage_data [k].value += value;
			memusage_data [k].n ++;
		}
	}

	for (i=0; i<n; i++)
		glibtop_free (ti [i].cmd);

	g_free (ti);
	
	/* memusage_data --; */

	gtk_signal_emit_by_name (GTK_OBJECT (d->graph), "redraw_graph");

	return TRUE;
}

static void
memusage_properties_update (GnomePropertyObject *object)
{
	GList *c;

	for (c = widget_list; c; c = c->next) {
		GTopMemUsage *memusage = GTOP_MEMUSAGE ((GtkWidget *) c->data);

		memusage_type_set (&memusage->data, memusage->data.ftype);

		memusage_update (&memusage->data);
	}
}

static void
adjustment_changed_cb (GtkWidget *widget, GtkWidget *adjustment)
{
	gtop_properties_changed ();
}

static void
memusage_properties_apply (GnomePropertyObject *object)
{
	GTopMemUsageProperties *prop = object->temp_data;
	gint i;
	
	for (i = 0; i < MEMUSAGE_FIELDS; i++)
		prop->thresholds [i] = adjustments [i]->value;
}

static void
radio_procall_cb (GtkWidget *button, GnomePropertyObject *object)
{
	GTopMemUsageProperties *prop = object->temp_data;

	if (!GTK_TOGGLE_BUTTON (button)->active)
		return;

	prop->proc_select = GTOP_PROC_SELECT_ALL;
	gtop_properties_changed ();
}

static void
radio_procuser_cb (GtkWidget *button, GnomePropertyObject *object)
{
	GTopMemUsageProperties *prop = object->temp_data;

	if (!GTK_TOGGLE_BUTTON (button)->active)
		return;

	prop->proc_select = GTOP_PROC_SELECT_USER;
	gtop_properties_changed ();
}

static void
radio_proctty_cb (GtkWidget *button, GnomePropertyObject *object)
{
	GTopMemUsageProperties *prop = object->temp_data;

	if (!GTK_TOGGLE_BUTTON (button)->active)
		return;

	prop->proc_select = GTOP_PROC_SELECT_TTY;
	gtop_properties_changed ();
}

static GtkWidget *
memusage_properties_init (GnomePropertyObject *object)
{
	GtkWidget *vb, *frame, *label, *button, *spin, *table;
	GTopMemUsageProperties *temp_properties = &gtop_properties.memusage;
	GtkObject *adjustment;
	GSList *group;

	vb    = gtk_vbox_new (FALSE, 0);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	frame = gtk_frame_new (_("Select Processes"));
	table = gtk_table_new (3, 1, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

	button = gtk_radio_button_new_with_label
		(NULL, _("Show all processes"));
	if (temp_properties->proc_select == GTOP_PROC_SELECT_ALL)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);
	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_procall_cb, object);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);

	group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label
		(group, _("Only show user processes"));
	if (temp_properties->proc_select == GTOP_PROC_SELECT_USER)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);
	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_procuser_cb, object);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 1, 2);

	group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label
		(group, _("Only show processes with a controlling tty"));
	if (temp_properties->proc_select == GTOP_PROC_SELECT_TTY)
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);
	gtk_signal_connect
		(GTK_OBJECT (button), "toggled", radio_proctty_cb, object);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 2, 3);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	frame = gtk_frame_new ("");
	table = gtk_table_new (5, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
	
	label = gtk_label_new (_("Minimum Resident Memory Size"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 0, 1);

	spin = gtk_spin_button_new (NULL, 1.0, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 256, 256);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 temp_properties->thresholds [GTOP_MEMUSAGE_RESIDENT]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 2, 3, 0, 1);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_MEMUSAGE_RESIDENT] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Minimum Shared Memory Size"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 1, 2);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 256, 256);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 temp_properties->thresholds [GTOP_MEMUSAGE_SHARED]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 2, 3, 1, 2);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_MEMUSAGE_SHARED] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Minimum Total Process Size"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 2, 3);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 256, 256);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 temp_properties->thresholds [GTOP_MEMUSAGE_SIZE]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 2, 3, 2, 3);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_MEMUSAGE_SIZE] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Minimum Virtual Memory Size"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 3, 4);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 256, 256);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 temp_properties->thresholds [GTOP_MEMUSAGE_VIRTUAL]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 2, 3, 3, 4);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_MEMUSAGE_VIRTUAL] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Minimum Swapped Memory Size"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 2, 4, 5);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 256, 256);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin),
		 temp_properties->thresholds [GTOP_MEMUSAGE_SWAP]);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 2, 3, 4, 5);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GTOP_MEMUSAGE_SWAP] = GTK_ADJUSTMENT (adjustment);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	return vb;
}

static void
memusage_properties_load (GnomePropertyObject *object)
{
	gtop_properties.memusage.proc_select =
		gnome_config_get_int ("gtop/memusage/proc_select=0");

	gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_RESIDENT] =
		gnome_config_get_int ("gtop/memusage/min_resident=1024");

	gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_SHARED] =
		gnome_config_get_int ("gtop/memusage/min_shared=1024");

	gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_SIZE] =
		gnome_config_get_int ("gtop/memusage/min_size=1024");

	gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_VIRTUAL] =
		gnome_config_get_int ("gtop/memusage/min_virtual=4096");

	gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_SWAP] =
		gnome_config_get_int ("gtop/memusage/min_swap=1024");
}

static void
memusage_properties_save (GnomePropertyObject *object)
{
	gnome_config_set_int
		("gtop/memusage/proc_select",
		 gtop_properties.memusage.proc_select);

	gnome_config_set_int
		("gtop/memusage/min_resident",
		 gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_RESIDENT]);

	gnome_config_set_int
		("gtop/memusage/min_shared",
		 gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_SHARED]);

	gnome_config_set_int
		("gtop/memusage/min_size",
		 gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_SIZE]);

	gnome_config_set_int
		("gtop/memusage/min_virtual",
		 gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_VIRTUAL]);

	gnome_config_set_int
		("gtop/memusage/min_swap",
		 gtop_properties.memusage.thresholds [GTOP_MEMUSAGE_SWAP]);
}
