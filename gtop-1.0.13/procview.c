#include <config.h>

#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <netdb.h>

#include <glibtop.h>
#include <glibtop/signal.h>
#include <glibtop/xmalloc.h>
#include <glibtop/union.h>

#include <gnome.h>

#include <procview.h>
#include <gtop-procview.h>

#include <summary.h>

#include "fromtop.h"
#include "proc.h"
#include "details.h"
#include "process-details.h"
#include "global.h"

#include "asc.xpm"
#include "dsc.xpm"

#define PROCVIEW_DATA_KEY	"procview_data"

/* procview idle run or frozen */
static gint config_sort_field = 0;
static gint config_sort_order = 0;
static gint config_remember_sorting = 1;

static void procview_init (GTopProcViewData *);
static void procview_open_mem_maps (GTopProcViewData *);
static void procview_open_details (GTopProcViewData *);
static void procview_clist_prepare (GTopProcViewData *);
static void procview_clist_update (GTopProcViewData *);

static const gchar *default_fonts [5] = {
	"-*-*-medium-r-*-*-8-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-10-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-12-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-14-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-18-*-*-*-*-*-*-*"
};

static GtkWidget *procview_properties_init (GnomePropertyObject *object);
static void procview_properties_changed (GnomePropertyObject *);
static void procview_properties_update (GnomePropertyObject *);
static void procview_properties_load (GnomePropertyObject *);
static void procview_properties_save (GnomePropertyObject *);

static GtkWidget *procfields_properties_init (GnomePropertyObject *object);
static void procfields_properties_update (GnomePropertyObject *object);
static void procfields_properties_apply (GnomePropertyObject *object);
static void procfields_properties_load (GnomePropertyObject *object);
static void procfields_properties_save (GnomePropertyObject *object);

static void adjustment_changed_cb (GtkWidget *, GtkWidget *);

GnomePropertyDescriptor ProcViewProperty_Descriptor = {
	sizeof (GTopProcViewProperties),
	N_("Processes"),
	procview_properties_init,
	NULL,
	procview_properties_update,
	procview_properties_load,
	procview_properties_save,
	NULL, NULL, NULL,
	procview_properties_changed,
	NULL
};

GnomePropertyDescriptor ProcFieldsProperty_Descriptor = {
	sizeof (GTopProcFieldsProperties),
	N_("Process Fields"),
	procfields_properties_init,
	procfields_properties_apply,
	procfields_properties_update,
	procfields_properties_load,
	procfields_properties_save,
	NULL, NULL, NULL, NULL, NULL
};

#define CMP_DINT(NAME) \
\
static int cmp_ ## NAME (const ProcProcData **P, const ProcProcData **Q) \
{ \
	  return (*P)->d->sort_order * ((*P)-> ## NAME - (*Q)-> ## NAME); \
}

#define CMP_INT(NAME) \
\
static int cmp_ ## NAME (const ProcProcData **P, const ProcProcData **Q) \
{ \
	  return (*P)->d->sort_order * ((*P)->p-> ## NAME - (*Q)->p-> ## NAME); \
}

#define CMP_INT2(NAME,V1,V2) \
\
static int cmp_ ## NAME (const ProcProcData **P, const ProcProcData **Q) \
{ \
	  return (*P)->d->sort_order * ((*P)->p-> ## V1 + (*P)->p-> ## V2 - (\
                               (*Q)->p-> ## V1 + (*Q)->p-> ## V2)); \
}

#define CMP_STR(NAME) \
\
static int cmp_ ## NAME (const ProcProcData **P, const ProcProcData **Q) \
{ \
	  return (*P)->d->sort_order * strcasecmp ((*P)->p-> ## NAME, (*Q)->p-> ## NAME); \
}

#define SORT_ASC 1
#define SORT_DSC -1

CMP_INT (stime);
CMP_INT (utime);
CMP_INT2 (time, stime, utime);
CMP_INT (pid);
CMP_DINT (pcpu);
CMP_DINT (pmem);
CMP_INT (size);
CMP_INT (rss);
CMP_INT (resident);
CMP_INT (nice);
CMP_INT (priority);
CMP_INT (share);

CMP_STR (user);
CMP_STR (cmd);

static GtkAdjustment *procfield_width_adjustments [GTOP_PROCFIELD_COUNT];

#define DEFAULT_PROCVIEW_FIELD_MASK	20311

gint default_field_width [GTOP_PROCFIELD_COUNT] = {
	5, 8, 3, 3, 6, 6, 6, 6, 4, 5, 5, 7, 7, 7, 20
};

#define DEFAULT_MAX_CMD_WIDTH	20

GTopProcField gtop_proc_fields [GTOP_PROCFIELD_COUNT] = {
	{ N_("PID"), N_("Process ID"),
	  PROCVIEW_PID, cmp_pid, SORT_ASC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("User"), N_("User name"),
	  PROCVIEW_USER, cmp_user, SORT_ASC, PROC_FILLUSR, GTK_JUSTIFY_LEFT, NULL},
	{ N_("Pri"), N_("Priority"),
	  PROCVIEW_PRIORITY, cmp_priority, SORT_ASC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Ni"), N_("Nice"),
	  PROCVIEW_NICE, cmp_nice, SORT_ASC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Size"), N_("Used memory size"),
	  PROCVIEW_SIZE, cmp_size, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("RSS"), N_("Resident memory set size"),
	  PROCVIEW_RSS, cmp_rss, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Resident"), N_("Resident memory size"),
	  PROCVIEW_RESIDENT, cmp_resident, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Share"), N_("Shared memory size"),
	  PROCVIEW_SHARE, cmp_share, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Stat"), N_("Process state"),
	  PROCVIEW_STATE, NULL, SORT_ASC, 0, GTK_JUSTIFY_LEFT, NULL},
	{ N_("CPU"), N_("Percent CPU usage"),
	  PROCVIEW_PCPU, cmp_pcpu, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("MEM"), N_("Percent memory usage"),
	  PROCVIEW_PMEM, cmp_pmem, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Time"), N_("Time consumed"),
	  PROCVIEW_TIME, cmp_time, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("UTime"), N_("User time consumed"),
	  PROCVIEW_UTIME, cmp_utime, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("STime"), N_("System time consumed"),
	  PROCVIEW_STIME, cmp_stime, SORT_DSC, 0, GTK_JUSTIFY_RIGHT, NULL},
	{ N_("Cmd"), N_("Command"),
	  PROCVIEW_CMD, cmp_cmd, SORT_ASC, PROC_FILLCMD, GTK_JUSTIFY_LEFT, NULL}
};

static void procview_select_row (GtkCList *, gint, gint,
				 GdkEventButton *, GTopProcViewData *);

static void procview_unselect_row (GtkCList *, gint, gint,
				   GdkEventButton *, GTopProcViewData *);

static void procview_click_column (GtkCList *, gint, GTopProcViewData *);

static GtkWidget *procview_clist_menu_prepare (GTopProcViewData *);

static void procview_renice (GTopProcViewData *);
static void procview_menu_cb_mem_maps (GtkWidget *, GTopProcViewData *);
static void procview_menu_cb_details (GtkWidget *, GTopProcViewData *);
static void procview_menu_cb_renice (GtkWidget *, GTopProcViewData *);

#define PIX(x) widget = gnome_pixmap_new_from_xpm_d (x ## _xpm); \
                        gtk_widget_ref (widget);
               

static GList *widget_list = NULL;

static gchar *get_command_line (pid_t, size_t);

void
procview_new (GTopProcViewData *d, GtkWidget *widget, gint ftype)
{
	GTopProcView *procview = GTOP_PROCVIEW (widget);
	GtkStyle *style;
	int i;

	g_assert (d != NULL);

	d->bin = GTK_WIDGET (procview);
	d->ftype = (GTopProcViewType) ftype;

	d->sort_asc = PIX (asc);
	d->sort_dsc = PIX (dsc);

	d->details.x = d->details.y = -1;

	d->p_fields = g_new0 (GTopProcField, GTOP_PROCFIELD_COUNT);

	memcpy (d->p_fields, gtop_proc_fields,
		sizeof (GTopProcField) * GTOP_PROCFIELD_COUNT);

	d->cmd_field_index = -1;

	for (i = 0; i < GTOP_PROCFIELD_COUNT; i++) {
	    if (d->p_fields [i].fmt != PROCVIEW_CMD)
		continue;

	    d->cmd_field_index = i;
	}

	d->sort_field = -1;
	d->sort_order = SORT_ASC;

	procview_init (d);

	if (config_remember_sorting) {
		GList *c = g_list_nth (d->field_list, config_sort_field);
		if (c) {
		    GTopProcField *field = c->data;

		    field->order = config_sort_order ? SORT_ASC : SORT_DSC;
		}
	}
	
	d->vbox = gtk_vbox_new (FALSE, 0);

	d->sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	d->clist = gtk_clist_new (GTOP_PROCFIELD_COUNT);
	gtk_container_add (GTK_CONTAINER (d->sw), d->clist);

	gtk_widget_show_all (d->sw);

	procview_clist_prepare (d);

	gtk_widget_ensure_style (d->clist);
	style = gtk_style_copy (gtk_widget_get_style (d->clist));

	gdk_font_unref (style->font);
	style->font = gtop_properties.procview.font;
	gdk_font_ref (style->font);

	gtk_widget_set_style (d->clist, style);
	gtk_style_unref (style);

	procview_click_column (GTK_CLIST (d->clist), config_sort_field, d);
	
	d->summary_data = gtop_summary_new ();

	if (d->summary_data->summary) {
		gtk_box_pack_start (GTK_BOX (d->vbox),
				    d->summary_data->box,
				    FALSE, TRUE, 0);
		gtk_widget_show (d->summary_data->box);
	}

	gtk_box_pack_start (GTK_BOX (d->vbox), d->sw, TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER (d->bin), d->vbox);

	gtk_widget_show (d->vbox);

	procview_type_set (d, ftype);

	procview_update (d);

	widget_list = g_list_append (widget_list, d->bin);
}

void
procview_map (GTopProcViewData *d)
{
	GTopProcView *procview = GTOP_PROCVIEW (d->bin);

	gtop_summary_map (d->summary_data);

	procview_update (&procview->data);
}

void
procview_unmap (GTopProcViewData *d)
{
	gtop_summary_unmap (d->summary_data);
}

void
procview_destroy (GtkWidget *widget)
{
	widget_list = g_list_remove (widget_list, widget);
}

void
procview_type_set (GTopProcViewData *d, gint ftype)
{
	g_assert (d != NULL);

	d->ftype = ftype;
}

static void
procview_click_column (GtkCList *cl, gint n, GTopProcViewData *d)
{
	GtkWidget *arrow;
	GTopProcField *field;
	GList *c;
	gint nsf;

	c = g_list_nth (d->field_list, n);
	if (!c) return;

	field = c->data;
	nsf = field - d->p_fields;

	/* has compare function ??*/
	if (!field->compare)
		return;

	if (d->sort_field >= 0) {

		arrow = (d->p_fields [d->sort_field].order == SORT_ASC) ?
			d->sort_asc : d->sort_dsc;

		gtk_widget_hide (arrow);
		gtk_widget_unrealize (arrow);
		gtk_widget_hide (d->p_fields [d->sort_field].hb);

		gtk_container_remove
			(GTK_CONTAINER (d->p_fields [d->sort_field].hb), arrow);
		gtk_widget_show (d->p_fields [d->sort_field].hb);
	}

	if (nsf == d->sort_field)
		field->order = (field->order == SORT_ASC) ?
			SORT_DSC : SORT_ASC;
	
	config_sort_field = n;
	config_sort_order = (field->order == SORT_ASC);

	if (config_remember_sorting) {
		gnome_config_set_int ("gtop/procview/sort_field",
				      config_sort_field);
		gnome_config_set_int ("gtop/procview/sort_order",
				      config_sort_order);
		gnome_config_sync ();
	}

	d->sort_field = nsf;
	arrow = (field->order == SORT_ASC) ? d->sort_asc : d->sort_dsc;
	gtk_box_pack_start (GTK_BOX (field->hb), arrow, FALSE, FALSE, 2);
	gtk_widget_show (arrow);
	procview_update (d);
}

static void
procview_select_row (GtkCList *cl,
		     gint row, gint col,
		     GdkEventButton *e, GTopProcViewData *d)
{
	if (row < d->prev_count)
		d->select_pid = d->proc_data [row]->p->pid;

	if (!e)	return;

	if ((e->button == 1) &&
	    (d->details.dwin || (e->type == GDK_2BUTTON_PRESS)))
		procview_open_details (d);
}

static void
procview_unselect_row (GtkCList *cl,
		       gint row, gint col,
		       GdkEventButton *e,
		       GTopProcViewData *d)
{
	d->select_pid = -1;
}

static void
procview_clist_button_press (GtkCList *cl, GdkEventButton *e,
			     GTopProcViewData *d)
{
	gint row, col;

	gtk_clist_get_selection_info (cl, e->x, e->y, &row, &col);

	if (e->button != 1)
		gtk_clist_select_row (GTK_CLIST (cl), row, 0);

	if ((d->select_pid != -1) && (e->button == 3))
		gtk_menu_popup (GTK_MENU (d->clist_menu), NULL, NULL, NULL,
				NULL, e->button, time(NULL));
}

static void
procview_send_signal (GtkWidget *w, gint s)
{
	GTopProcViewData *d = gtk_object_get_data
		(GTK_OBJECT (w), PROCVIEW_DATA_KEY);

	if (d->select_pid != -1)
		kill (d->select_pid, s);
}

static void
procview_open_mem_maps (GTopProcViewData *d)
{
	if (d->select_pid != -1)
		procview_details (d, d->select_pid);
}

static void
procview_open_details (GTopProcViewData *d)
{
	if (d->select_pid != -1)
		gtop_process_details (d->select_pid);
}

static void
procview_menu_cb_details (GtkWidget *w, GTopProcViewData *d)
{
	procview_open_details (d);
}

static void
procview_menu_cb_mem_maps (GtkWidget *w, GTopProcViewData *d)
{
	procview_open_mem_maps (d);
}

static void
procview_menu_cb_renice (GtkWidget *w, GTopProcViewData *d)
{
	procview_renice (d);
}

static gint
renice_cb (GTopProcViewData *d)
{
	gint  rv, nv = d->renice_adj->value;
	gchar *msg, tmp [256];

	rv = setpriority (PRIO_PROCESS, d->select_pid, nv);

	msg = NULL; /* keep gcc happy */

	if (rv == -1) {
		switch (errno) {
		case ESRCH:
			g_snprintf (tmp, 256,
				    _("Cannot locate process %d"),
				    d->select_pid);
			msg = tmp;
			break;
		case EACCES:
			msg = _("Only root can set negative priority");
			break;
		case EPERM:
			msg = _("Permission denied");
			break;
		}
	} else {
		procview_update (d);
		msg = "";
	}
	
	gtk_label_set (GTK_LABEL (d->renice_msg), msg);

	return rv;
}

static void
renice_close_cb (GtkWidget *w, GTopProcViewData *d)
{
	if (d->renice_dialog)
		gnome_dialog_close (GNOME_DIALOG (d->renice_dialog));
}

static void
renice_destroy_cb (GtkWidget *w, GTopProcViewData *d)
{
	d->renice_dialog = NULL;
	d->renice_msg = NULL;
	d->renice_adj = NULL;
}

static void
renice_ok_cb (GtkWidget *w, GTopProcViewData *d)
{
	if (renice_cb (d) == 0)
		gnome_dialog_close (GNOME_DIALOG (d->renice_dialog));
}		

static void
renice_apply_cb (GtkWidget *w, GTopProcViewData *d)
{
	renice_cb (d);
}

static void
procview_renice (GTopProcViewData *d)
{
	if (!d->renice_dialog) {

		GtkWidget *hs;
		GtkWidget *lb;

		d->renice_dialog = gnome_dialog_new (_("Renice"),
						     GNOME_STOCK_BUTTON_OK,
						     GNOME_STOCK_BUTTON_APPLY,
						     GNOME_STOCK_BUTTON_CANCEL,
						     NULL);
		gtk_object_set_data (GTK_OBJECT (d->renice_dialog),
				     PROCVIEW_DATA_KEY, (gpointer) d);
		d->renice_adj = (GtkAdjustment *) gtk_adjustment_new
			(getpriority (PRIO_PROCESS, d->select_pid),
			 -20, 20, 1, 1, 0);
		hs = gtk_hscale_new (d->renice_adj);
		gtk_scale_set_digits (GTK_SCALE (hs), 0);
		lb = gtk_label_new (_("Set priority"));
		d->renice_msg = gtk_label_new ("");
		gtk_misc_set_alignment (GTK_MISC (lb), 0, 1);
		gtk_misc_set_alignment (GTK_MISC (d->renice_msg), 0, 1);

		gtk_box_pack_start
			(GTK_BOX (GNOME_DIALOG (d->renice_dialog)->vbox),
			 lb, TRUE, TRUE, 0);
		gtk_box_pack_start_defaults
			(GTK_BOX (GNOME_DIALOG (d->renice_dialog)->vbox), hs);
		gtk_box_pack_start
			(GTK_BOX (GNOME_DIALOG (d->renice_dialog)->vbox),
			 d->renice_msg, TRUE, TRUE, 0);

		gnome_dialog_button_connect
			(GNOME_DIALOG (d->renice_dialog),
			 0, (GtkSignalFunc) renice_ok_cb, (gpointer) d);
		gnome_dialog_button_connect
			(GNOME_DIALOG (d->renice_dialog),
			 1, (GtkSignalFunc) renice_apply_cb, (gpointer) d);
		gnome_dialog_button_connect
			(GNOME_DIALOG (d->renice_dialog),
			 2, (GtkSignalFunc) renice_close_cb, (gpointer) d);
		gtk_signal_connect
			(GTK_OBJECT (d->renice_dialog), "destroy",
			 (GtkSignalFunc) renice_destroy_cb, (gpointer) d);

		gtk_widget_show (lb);
		gtk_widget_show (hs);
		gtk_widget_show (d->renice_msg);
		gtk_widget_show (d->renice_dialog);
	}
}

extern void gtop_show_mtbar (GtkWidget *w, gpointer gp);

static GtkWidget *
procview_clist_menu_prepare (GTopProcViewData *d)
{
	GtkWidget *menu;
	GtkWidget *smenu;
	GtkWidget *mi;
	gint i;

	menu = gtk_menu_new ();
	mi = gtk_menu_item_new_with_label (_("Send"));
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	smenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), smenu);

	for (i=0; glibtop_sys_siglist [i].name; i++) {
		mi = gtk_menu_item_new_with_label
			(_(glibtop_sys_siglist [i].name));
		gtk_object_set_data
			(GTK_OBJECT (mi), PROCVIEW_DATA_KEY, (gpointer) d);
		gtk_signal_connect (GTK_OBJECT (mi), "activate",
				    GTK_SIGNAL_FUNC (procview_send_signal),
				    GINT_TO_POINTER (glibtop_sys_siglist [i].number));
		gtk_menu_append (GTK_MENU (smenu), mi);
		gtk_widget_show (mi);

		if (!((i+1) % 17)) {
			/* separator */
			mi = gtk_menu_item_new ();
			gtk_menu_append (GTK_MENU (smenu), mi);
			gtk_widget_show (mi);

			mi = gtk_menu_item_new_with_label (_("More ..."));
			gtk_menu_append (GTK_MENU (smenu), mi);
			gtk_widget_show (mi);

			smenu = gtk_menu_new ();
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), smenu);
		}
	}

	/* separator */
	mi = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	mi = gtk_menu_item_new_with_label (_("Kill nicely (SIGTERM)"));
	gtk_object_set_data
		(GTK_OBJECT (mi), PROCVIEW_DATA_KEY, (gpointer) d);
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (procview_send_signal),
			    (gpointer) SIGTERM);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	mi = gtk_menu_item_new_with_label (_("Kill now (SIGKILL)"));
	gtk_object_set_data
		(GTK_OBJECT (mi), PROCVIEW_DATA_KEY, (gpointer) d);
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (procview_send_signal),
			    (gpointer) SIGKILL);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	/* separator */
	mi = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	mi = gtk_menu_item_new_with_label (_("Renice ..."));
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (procview_menu_cb_renice),
			    (gpointer) d);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	mi = gtk_menu_item_new_with_label (_("Memory Maps ..."));
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (procview_menu_cb_mem_maps),
			    (gpointer) d);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	mi = gtk_menu_item_new_with_label (_("Details ..."));
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    GTK_SIGNAL_FUNC (procview_menu_cb_details),
			    (gpointer) d);
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

	/* separator */
	mi = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), mi);
	gtk_widget_show (mi);

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
procview_clist_prepare (GTopProcViewData *d)
{
	gint i, j, length;
	GList *c = d->field_list;
	GList *g = d->geometry_list;
	GtkCList *cl = GTK_CLIST (d->clist);

	gtk_clist_column_titles_show (GTK_CLIST (d->clist));
	gtk_widget_set_name (d->clist, "ProcessList");

	length = GTOP_PROCFIELD_COUNT;
	for (i = 0; i < length; i++) {
		GtkWidget *l = gtk_label_new ("");
		GtkWidget *hb = gtk_hbox_new (FALSE, 0);

		gtk_widget_show (l);
		gtk_widget_show (hb);
		
		gtk_box_pack_start_defaults (GTK_BOX (hb), l);
		gtk_clist_set_column_widget (GTK_CLIST (d->clist), i, hb);
	}

	i = 0;
	while (c) {
		GtkWidget *hb = GTK_BIN (cl->column[i].button)->child;
		GtkBoxChild *bc = (GtkBoxChild *) GTK_BOX (hb)->children->data;
		GTopProcField *field = c->data;
		GtkStyle *style;

		gtk_label_set (GTK_LABEL (bc->widget), _(field->label));

		gtk_widget_ensure_style (bc->widget);
		style = gtk_style_copy (gtk_widget_get_style (bc->widget));

		gdk_font_unref (style->font);
		style->font = gtop_properties.procview.font;
		gdk_font_ref (style->font);

		gtk_widget_set_style (bc->widget, style);
		gtk_style_unref (style);

		field->hb = hb;
		c = c->next;
		i++;
	}

	d->clist_menu = procview_clist_menu_prepare (d);

	gtk_signal_connect (GTK_OBJECT (d->clist), "click_column",
			    GTK_SIGNAL_FUNC (procview_click_column),
			    (gpointer) d);
	gtk_signal_connect (GTK_OBJECT (d->clist), "select_row",
			    GTK_SIGNAL_FUNC (procview_select_row),
			    (gpointer) d);
	gtk_signal_connect (GTK_OBJECT (d->clist), "unselect_row",
			    GTK_SIGNAL_FUNC (procview_unselect_row),
			    (gpointer) d);
	gtk_signal_connect (GTK_OBJECT (d->clist), "button_press_event",
			    GTK_SIGNAL_FUNC (procview_clist_button_press),
			    (gpointer) d);

	c = d->field_list;
	i = 0;
	while (c) {
		GTopProcField *field = c->data;
		gint *widthp = g->data;

		gtk_clist_set_column_justification (GTK_CLIST (d->clist), i,
						    field->justification);
		gtk_clist_set_column_width (GTK_CLIST (d->clist), i++, *widthp);
		c = c->next;
		g = g->next;
	}

	for (j = i; j < GTOP_PROCFIELD_COUNT; j++)
		gtk_clist_set_column_visibility (cl, j, FALSE);
	gtk_widget_queue_resize (GTK_WIDGET (cl));
}

static void
procview_clist_update (GTopProcViewData *d)
{
	gint i, j;
	GList *c = d->field_list;
	GList *g = d->geometry_list;
	GtkCList *cl = GTK_CLIST (d->clist);

	gtk_clist_clear (GTK_CLIST (d->clist));

	i = 0;
	while (c) {
		GtkWidget *hb = GTK_BIN (cl->column[i].button)->child;
		GtkBoxChild *bc = (GtkBoxChild *) GTK_BOX (hb)->children->data;
		GTopProcField *field = c->data;
		gint *widthp = g->data;
		GtkStyle *style;

		gtk_label_set (GTK_LABEL (bc->widget), _(field->label));

		gtk_widget_ensure_style (bc->widget);
		style = gtk_style_copy (gtk_widget_get_style (bc->widget));

		gdk_font_unref (style->font);
		style->font = gtop_properties.procview.font;
		gdk_font_ref (style->font);

		gtk_widget_set_style (bc->widget, style);
		gtk_style_unref (style);
		
		gtk_clist_set_column_visibility (cl, i, TRUE);

		gtk_clist_set_column_justification (cl, i, field->justification);
		gtk_clist_set_column_width (cl, i++, *widthp);

		field->hb = hb;
		c = c->next;
		g = g->next;
	}

	for (j = i; j < GTOP_PROCFIELD_COUNT; j++)
		gtk_clist_set_column_visibility (cl, j, FALSE);
	gtk_widget_queue_resize (GTK_WIDGET (cl));
}

static void
procview_init (GTopProcViewData *d)
{
	glong field_mask;
	gint *ip, i, char_width, width;

	if (d->field_list) {
		g_list_free (d->field_list);
		d->field_list = NULL;
	}

	if (d->geometry_list) {
		g_list_free (d->geometry_list);
		d->geometry_list = NULL;
	}

	d->field_list_length = 0;
	field_mask = gtop_properties.procfields.field_mask;

	if (field_mask == 0)
		field_mask = 1;

	for (i = 0; i < GTOP_PROCFIELD_COUNT; i++) {
		if (field_mask & (1 << i)) {
			gint min_width = strlen (_(d->p_fields [i].label));

			d->field_list = g_list_append
				(d->field_list, d->p_fields+i);

			ip = g_new0 (gint, 1);
			width = gtop_properties.procfields.field_width [i];

			if (width < min_width)
				width = min_width;

			char_width = gdk_char_width
				(gtop_properties.procview.font, 'M');

			*ip = width * char_width;

			d->geometry_list = g_list_append
				(d->geometry_list, ip);

			d->field_list_length++;
		}
	}
}

gchar *
sprint_fmt (ProcProcData *d, p_fmt fmt)
{
	gtop_proc_t *t = d->p;
	gchar tmp[256];
	gchar *rv;

	switch (fmt) {
	case PROCVIEW_TIME:
	case PROCVIEW_UTIME:
	case PROCVIEW_STIME:
	case PROCVIEW_PID:
	case PROCVIEW_PRIORITY:
	case PROCVIEW_NICE:
	case PROCVIEW_RSS:
	case PROCVIEW_SIZE:
	case PROCVIEW_SHARE:
	case PROCVIEW_PCPU:
	case PROCVIEW_PMEM:
	case PROCVIEW_RESIDENT:
		switch (fmt) {
		case PROCVIEW_TIME:
			sprint_time (tmp, t->utime + t->stime, t->frequency);
			break;
		case PROCVIEW_UTIME:
			sprint_time (tmp, t->utime, t->frequency);
			break;
		case PROCVIEW_STIME:
			sprint_time (tmp, t->stime, t->frequency);
			break;
		case PROCVIEW_PID:
			sprintf (tmp, "%ld", t->pid);
			break;
		case PROCVIEW_PRIORITY:
			sprintf (tmp, "%ld", t->priority);
			break;
		case PROCVIEW_NICE:
			sprintf (tmp, "%ld", t->nice);
			break;
		case PROCVIEW_RSS:
			sprintf (tmp, "%ld", t->rss);
			break;
		case PROCVIEW_SIZE:
			sprintf (tmp, "%ld", t->size);
			break;
		case PROCVIEW_SHARE:
			sprintf (tmp, "%ld", t->share);
			break;
		case PROCVIEW_PCPU:
			sprintf (tmp, "%2d.%1d", d->pcpu / 10, d->pcpu % 10);
			break;
		case PROCVIEW_PMEM:
			sprintf (tmp, "%2d.%1d", d->pmem / 10, d->pmem % 10);
			break;
		case PROCVIEW_RESIDENT:
			sprintf (tmp, "%ld", t->resident);
			break;
		default:
			break;
		}
		rv = tmp;
		break;
	case PROCVIEW_STATE:
		rv = status (t);
		break;
	case PROCVIEW_USER:
		rv = t->user;
		break;
	case PROCVIEW_CMD:
		rv = t->cmd;
		break;
	default:
		rv = "?";
	}

	return g_strdup (rv);
}

static void
procview_update_clist (GTopProcViewData *d, ProcProcData **pd)
{
	gint i=0;
	gint j, k;
	gchar **text = g_new0 (gchar *, GTOP_PROCFIELD_COUNT);
	ProcProcData *data;
	GList *c;
	float old_adj_hval = GTK_CLIST (d->clist)->hadjustment->value;
	float old_adj_vval = GTK_CLIST (d->clist)->vadjustment->value;
	gint old_hoffset = GTK_CLIST (d->clist)->hoffset;
	gint old_voffset = GTK_CLIST (d->clist)->voffset;
	gint row = -1;
	
	gtk_clist_freeze (GTK_CLIST (d->clist));
	gtk_clist_clear (GTK_CLIST (d->clist));

	while ((data = pd [i])) {
		i++;
		c = d->field_list;
		j = 0;

		while (c) {
			GTopProcField *field = c->data;

			text [j] = sprint_fmt (data, field->fmt);
			j++;
			c = c->next;
		}
		gtk_clist_append (GTK_CLIST (d->clist), text);

		for (k=0; k<j; k++) {
			g_free (text [k]);
		}
	}

	/* keep old view position - somewhat ughly, but works now */
	GTK_CLIST (d->clist)->hadjustment->value = old_adj_hval;
	GTK_CLIST (d->clist)->vadjustment->value = old_adj_vval;

	GTK_CLIST (d->clist)->hoffset = old_hoffset;
	GTK_CLIST (d->clist)->voffset = old_voffset;

	for (i=0; i < d->prev_count; i++)
		if (pd [i]->p->pid == d->select_pid)
			row = i;
	if (row >= 0)
		gtk_clist_select_row (GTK_CLIST (d->clist), row, -1);

	gtk_clist_thaw (GTK_CLIST (d->clist));

	g_free (text);
}

void
procview_update (GTopProcViewData *d)
{
	static gint first = 1;
 	static struct save_hist save_history [NR_TASKS];
	struct save_hist new_save_hist [NR_TASKS];
	glibtop_proclist proclist;
	glibtop_proc_state procstate;
	glibtop_proc_time proctime;
	glibtop_proc_mem procmem;
	glibtop_proc_uid procuid;
	glibtop_uptime uptime;
	glibtop_mem mem;
	gint which, arg;
	struct passwd *pwd;
	int stime, utime;
	size_t max_cmd_len;

	unsigned long p_gl_main_mem;
	ProcProcData *data;
	gtop_proc_t *p;
	gint i, j, n=0;
	gint p_cl_sum;

	time_t p_time;
	double p_elapsed_time;
	double p_total_time;
	double p_current_time;

	p_cl_sum = gtop_properties.procview.details_flags &
		(1 << GTOP_DETAILS_CUMULATIVE_TIMINGS);

	if (d->proc_tab)
		glibtop_free (d->proc_tab);

	glibtop_get_uptime (&uptime);
	p_current_time = uptime.uptime;
	p_time = time (NULL);
	p_elapsed_time = get_elapsed_time ();

	glibtop_get_mem (&mem);
	p_gl_main_mem = mem.total >> 10;
	/* avoid divison by zero later. */
	if (!p_gl_main_mem) p_gl_main_mem = 1;

	switch (d->ftype) {
	case GTOP_PROCVIEW_USER:
		which = GLIBTOP_KERN_PROC_UID;
		if (d->select_pid != -1) {
		  glibtop_proc_uid procuid;

		  glibtop_get_proc_uid (&procuid, d->select_pid);
		  if (procuid.flags & (1L << GLIBTOP_PROC_UID_UID))
		    arg = procuid.uid;
		  else
		    arg = getuid ();
		} else {
		  arg = getuid ();
		}
		break;
	default:
		which = GLIBTOP_KERN_PROC_ALL;
		arg = 0;
		break;
	}

	if (d->proc_selection_flags & (1 << GTOP_PROCVIEW_TTY))
		which |= GLIBTOP_EXCLUDE_NOTTY;

	if (d->proc_selection_flags & (1 << GTOP_PROCVIEW_IDLE))
		which |= GLIBTOP_EXCLUDE_IDLE;

	if (d->proc_selection_flags & (1 << GTOP_PROCVIEW_SYSTEM))
		which |= GLIBTOP_EXCLUDE_SYSTEM;

	memset (&proclist, 0, sizeof (glibtop_proclist));

	d->proc_tab = glibtop_get_proclist (&proclist, which, arg);

	if (d->proc_data) {
		for (i = 0; i < d->prev_count; i++) {
			if ((p = d->proc_data [i]->p)) {
				glibtop_free (p->cmd);
				glibtop_free (p->user);
			}
			g_free (d->proc_data [i]);
			d->proc_data [i] = NULL;
		}
		g_free (d->proc_data);
		d->proc_data = NULL;
	}

	n = proclist.number;

	d->proc_data = (ProcProcData **) g_new0 (ProcProcData *, n+1);
	d->proc_data [n] = NULL;

	max_cmd_len = (d->cmd_field_index >= 0) ?
		gtop_properties.procfields.field_width [d->cmd_field_index] :
			DEFAULT_MAX_CMD_WIDTH;

	for (i = 0; i < n; i++) {
		/* allocate proc_data */
		data = d->proc_data [i] = 
			(ProcProcData *) g_new0 (ProcProcData, 1);
		data->d = d;

		memset (&data->_p, 0, sizeof (data->_p));
		p = data->p = &data->_p;

		p->pid = d->proc_tab [i];

		glibtop_get_proc_state (&procstate, p->pid);

		p->cmd = get_command_line (p->pid, max_cmd_len);
		if (!p->cmd)
		    p->cmd = glibtop_strdup (procstate.cmd);

		p->state = procstate.state;

		pwd = getpwuid (procstate.uid);
		if (pwd)
			p->user = glibtop_strdup (pwd->pw_name);
		else
			p->user = glibtop_strdup (_("<unknown>"));

		glibtop_get_proc_uid (&procuid, p->pid);

		p->nice     = procuid.nice;
		p->priority = procuid.priority;

		glibtop_get_proc_mem (&procmem, p->pid);

		p->size     = (unsigned long) procmem.size >> 10;
		p->rss      = (unsigned long) procmem.rss >> 10;
		p->resident = (unsigned long) procmem.resident >> 10;
		p->share    = (unsigned long) procmem.share >> 10;

		glibtop_get_proc_time (&proctime, p->pid);

		p->frequency = proctime.frequency ? proctime.frequency : 1000000;

		p->utime  = (unsigned long) proctime.utime;
		p->stime  = (unsigned long) proctime.stime;
		p->cutime = (unsigned long) proctime.cutime;
		p->cstime = (unsigned long) proctime.cstime;

		p->start_time = (unsigned long) proctime.start_time;

		/* compute some own fields for proc_data */
		/* PCPU */
		p_total_time = (p->utime + p->stime +
				(p_cl_sum ? p->cutime + p->cstime : 0));

		/* history */
		new_save_hist[i].ticks = p_total_time;
		new_save_hist[i].pid = p->pid;
		stime = p->stime;
		utime = p->utime;
		new_save_hist[i].stime = stime;
		new_save_hist[i].utime = utime;

		/* find matching entry from previous pass */
		j = 0;
		while (!first && j < MIN (d->prev_count, NR_TASKS)) {
			if (save_history[j].pid == p->pid) {
				p_total_time -= save_history[j].ticks;
				stime -= save_history[j].stime;
				utime -= save_history[j].utime;

				j = NR_TASKS;
			}
			j++;
		}

		if (first)
			p_elapsed_time = p_current_time - ((double) p->start_time / p->frequency);

		data->pcpu = (p_elapsed_time > 0.001) ? (unsigned int)
			((p_total_time * 1000.0 / (double) p->frequency) / p_elapsed_time) : 0;

		if (data->pcpu > 999)
			data->pcpu = 999;
		/* PMEM */
		data->pmem = p->rss * 1000 / p_gl_main_mem;

	}

	/* copy the relevant info for the next pass */
	for (i = 0; i < n; i++) {
		save_history[i].pid = new_save_hist[i].pid;
		save_history[i].ticks = new_save_hist[i].ticks;
		save_history[i].stime = new_save_hist[i].stime;
		save_history[i].utime = new_save_hist[i].utime;
	}

	d->prev_count = n;

	if (d->p_fields [d->sort_field].compare) {
		d->sort_order = d->p_fields [d->sort_field].order;
		qsort (d->proc_data, n, sizeof (ProcProcData *),
		       (int (*)(const void *, const void *))
		       d->p_fields [d->sort_field].compare);
	}

	/* update screen*/
	procview_update_clist (d, d->proc_data);
	first = 0;

	if (d->details.dwin &&
	    (gtop_properties.procview.details_flags &
	     (1 << GTOP_DETAILS_AUTO_UPDATE)))
		procview_open_mem_maps (d);
}

#if LIBGTOP_VERSION_CODE >= 1001004

static gchar *
get_command_line (pid_t pid, size_t max_len)
{
	glibtop_array array;
	char **args, *retval = NULL;
    
	args = glibtop_get_proc_args (&array, pid);

	if (args && (array.number >= 1) && (array.size > 0)) {
		size_t len;

		len = strlen (args [0]);
		if (len > max_len)
			len = max_len;

		if (len) {
			retval = glibtop_malloc (len+1);
			memcpy (retval, args [0], len);
			*(retval+len) = 0;
		}
	}

	if (args) {
		int i;

		for (i = 0; i < array.number; i++) {
			glibtop_free (args [i]);
		}

		glibtop_free (args);
	}

	return retval;
}

#else /* LIBGTOP_VERSION_CODE < 1001004 */

static gchar *
get_command_line (pid_t pid, size_t max_len)
{
	glibtop_proc_args procargs;
	gchar *args, *retval = NULL;

	args = glibtop_get_proc_args (&procargs, pid, max_len);

	if (args) {
		if (strlen (args))
			retval = glibtop_strdup (args);

		glibtop_free (args);

		return retval;
	}

	return NULL;
}

#endif /* LIBGTOP_VERSION_CODE < 1001004 */

static void
procview_properties_changed (GnomePropertyObject *object)
{
	gtop_properties_changed ();
}

static void
procview_properties_update (GnomePropertyObject *object)
{
	GList *c;

	for (c = widget_list; c; c = c->next) {
		GTopProcView *procview = GTOP_PROCVIEW ((GtkWidget *) c->data);
		GTopProcViewData *d = &procview->data;
		GtkStyle *style;

		gtk_widget_ensure_style (d->clist);
		style = gtk_style_copy (gtk_widget_get_style (d->clist));

		gdk_font_unref (style->font);
		style->font = gtop_properties.procview.font;
		gdk_font_ref (style->font);

		gtk_widget_set_style (d->clist, style);
		gtk_style_unref (style);

		procview_update (&procview->data);
	}
}

static void
procview_details_flags_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopProcViewProperties *prop_ptr = cb_data->object->temp_data;

	if (GTK_TOGGLE_BUTTON (cb_data->button)->active)
		prop_ptr->details_flags |= (1 << cb_data->index);
	else
		prop_ptr->details_flags &= ~(1 << cb_data->index);

	gtop_properties_changed ();
}

static GtkWidget *
procview_properties_init (GnomePropertyObject *object)
{
	GTopProcViewProperties *prop_ptr = &gtop_properties.procview;
	GtkWidget *vb, *frame, *label, *table, *hb;
	RadioButtonCbData *cb_data;

	vb    = gtk_vbox_new (FALSE, 0);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	frame = gtk_frame_new (_("Process View Font"));

	table = gnome_property_entry_font
		(object, _("ProcView Font"),
		 &gtop_properties.procview.font_name,
		 &gtop_properties.procview.font);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Details Dialog"));
	table = gtk_table_new (1, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	label = gtk_check_button_new_with_label
		(_("Automatically update details dialog"));

	if (prop_ptr->details_flags & (1 << GTOP_DETAILS_AUTO_UPDATE))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = label;
	cb_data->object = object;
	cb_data->index = GTOP_DETAILS_AUTO_UPDATE;

	gtk_signal_connect (GTK_OBJECT (label), "toggled",
			    procview_details_flags_cb, cb_data);

	gtk_table_attach_defaults
		(GTK_TABLE (table), label, 0, 1, 0, 1);

	label = gtk_check_button_new_with_label
		(_("Use full pathnames in memory maps"));

	if (prop_ptr->details_flags & (1 << GTOP_DETAILS_FULL_PATHNAMES))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = label;
	cb_data->object = object;
	cb_data->index = GTOP_DETAILS_FULL_PATHNAMES;

	gtk_signal_connect (GTK_OBJECT (label), "toggled",
			    procview_details_flags_cb, cb_data);

	gtk_table_attach_defaults
		(GTK_TABLE (table), label, 0, 1, 1, 2);

	label = gtk_check_button_new_with_label
		(_("Remember dialog position"));

	if (prop_ptr->details_flags & (1 << GTOP_DETAILS_REMEMBER_POSITION))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = label;
	cb_data->object = object;
	cb_data->index = GTOP_DETAILS_REMEMBER_POSITION;

	gtk_signal_connect (GTK_OBJECT (label), "toggled",
			    procview_details_flags_cb, cb_data);

	gtk_table_attach_defaults
		(GTK_TABLE (table), label, 0, 1, 2, 3);

	gtk_container_add (GTK_CONTAINER (frame), table);
	hb = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (hb), frame, TRUE, TRUE, 0);

	frame = gtk_frame_new (_("Misc"));
	table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	label = gtk_check_button_new_with_label
		(_("Use cumulative timings"));

	if (prop_ptr->details_flags & (1 << GTOP_DETAILS_CUMULATIVE_TIMINGS))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = label;
	cb_data->object = object;

	gtk_signal_connect (GTK_OBJECT (label), "toggled",
			    procview_details_flags_cb, cb_data);

	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (hb), frame, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, TRUE, GNOME_PAD_SMALL);

	return vb;
}

static void
procview_properties_load (GnomePropertyObject *object)
{
	const gchar *default_font;
	gchar name [BUFSIZ];

	guint width;

	width = gdk_screen_width ();

	if (width <= 640)
		default_font = default_fonts [0];
	else if (width <= 800)
		default_font = default_fonts [1];
	else if (width <= 1024)
		default_font = default_fonts [2];
	else if (width <= 1280)
		default_font = default_fonts [3];
	else
		default_font = default_fonts [4];

	sprintf (name, "gtop/procview/font_name=%s", default_font);
	gtop_properties.procview.font_name = gnome_config_get_string (name);

	gtop_properties.procview.font = gdk_fontset_load
		(gtop_properties.procview.font_name);

	if (!gtop_properties.procview.font) {
		g_free (gtop_properties.procview.font_name);
		gtop_properties.procview.font_name = g_strdup (default_font);
		gtop_properties.procview.font = gdk_fontset_load
			(gtop_properties.procview.font_name);
	}

	gtop_properties.procview.details_flags =
		gnome_config_get_int ("gtop/procview/details_flags=0");

	config_remember_sorting =
		gnome_config_get_int ("gtop/procview/remember_sorting=1");

	if (config_remember_sorting) {
		config_sort_field =
			gnome_config_get_int ("gtop/procview/sort_field=8");

		config_sort_order =
			gnome_config_get_int ("gtop/procview/sort_order=0");
	}
}

static void
procview_properties_save (GnomePropertyObject *object)
{
	gnome_config_set_string ("gtop/procview/font_name",
				 gtop_properties.procview.font_name);

	gnome_config_set_int ("gtop/procview/details_flags",
			      gtop_properties.procview.details_flags);
}

static void
procfields_properties_update (GnomePropertyObject *object)
{
	GList *c;

	for (c = widget_list; c; c = c->next) {
		GTopProcView *procview = GTOP_PROCVIEW ((GtkWidget *) c->data);

		procview_init (&procview->data);

		procview_clist_update (&procview->data);

		procview_update (&procview->data);
	}
}

static void
procfields_mask_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopProcFieldsProperties *prop_ptr = cb_data->object->temp_data;

	if (GTK_TOGGLE_BUTTON (cb_data->button)->active)
		prop_ptr->field_mask |= (1 << cb_data->index);
	else
		prop_ptr->field_mask &= ~(1 << cb_data->index);

	gtop_properties_changed ();
}

static void
adjustment_changed_cb (GtkWidget *widget,
		       GtkWidget *adjustment)
{
	gtop_properties_changed ();
}

static GtkWidget *
procfields_properties_init (GnomePropertyObject *object)
{
	GTopProcFieldsProperties *prop_ptr = &gtop_properties.procfields;
	GtkWidget *label, *spin, *sw, *table;
	GtkObject *adjustment;
	gint i;

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_ALWAYS);

	gtk_container_border_width (GTK_CONTAINER (sw), GNOME_PAD);

	table = gtk_table_new (GTOP_PROCFIELD_COUNT, 4, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	for (i = 0; i < GTOP_PROCFIELD_COUNT; i++) {
		RadioButtonCbData *cb_data;

		label = gtk_check_button_new_with_label
			(_(gtop_proc_fields [i].label));

		if (prop_ptr->field_mask & (1 << i))
			gtk_toggle_button_set_state
				(GTK_TOGGLE_BUTTON (label), TRUE);

		cb_data = g_new0 (RadioButtonCbData, 1);

		cb_data->index = i;
		cb_data->button = label;
		cb_data->object = object;

		gtk_signal_connect (GTK_OBJECT (label), "toggled",
				    procfields_mask_cb, cb_data);

		gtk_table_attach_defaults
			(GTK_TABLE (table), label, 1, 2, i, i+1);

		label = gtk_label_new (_(gtop_proc_fields [i].long_info));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_table_attach_defaults
			(GTK_TABLE (table), label, 2, 3, i, i+1);

		spin = gtk_spin_button_new (NULL, 1, 0);
		adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 10, 10);
		gtk_spin_button_set_adjustment
			(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
		gtk_spin_button_set_value
			(GTK_SPIN_BUTTON (spin), prop_ptr->field_width [i]);
		gtk_table_attach_defaults (GTK_TABLE (table), spin, 3, 4, i, i+1);

		gtk_signal_connect
			(GTK_OBJECT (adjustment), "value_changed",
			 adjustment_changed_cb, adjustment);

		procfield_width_adjustments [i] = GTK_ADJUSTMENT (adjustment);
	}

	gtk_scrolled_window_add_with_viewport
		(GTK_SCROLLED_WINDOW (sw), table);

	return sw;
}

static void
procfields_properties_apply (GnomePropertyObject *object)
{
	GTopProcFieldsProperties *prop_ptr = object->temp_data;
	gint i;
	
	for (i = 0; i < GTOP_PROCFIELD_COUNT; i++)
		prop_ptr->field_width [i] =
			procfield_width_adjustments [i]->value;
}

static void
procfields_properties_load (GnomePropertyObject *object)
{
	gchar *string, *pos, *default_string;
	gint index = 0, i;

	default_string = g_strdup_printf
		("gtop/procview/procview_field_mask=%d",
		 DEFAULT_PROCVIEW_FIELD_MASK);

	gtop_properties.procfields.field_mask = gnome_config_get_int
		(default_string);

	if (!gtop_properties.procfields.field_mask)
		gtop_properties.procfields.field_mask =
			DEFAULT_PROCVIEW_FIELD_MASK;

	for (i = 0; i < GTOP_PROCFIELD_COUNT; i++)
		gtop_properties.procfields.field_width [i] =
			default_field_width [i];

	string = gnome_config_get_string
		("gtop/procview/procview_field_width");
	if (!string) return;

	pos = string;
	while (*pos) {
		gint value;
		gchar *tmp;

		tmp = strchr (pos, ',');
		if (tmp) *tmp = '\0';

		if (sscanf (pos, "%d", &value) == 1)
			gtop_properties.procfields.field_width [index] = value;

		if (index++ >= GTOP_PROCFIELD_COUNT)
			break;

		if (tmp)
			pos = tmp+1;
		else
			break;
	}

	g_free (string);
}

static void
procfields_properties_save (GnomePropertyObject *object)
{
	gchar string [BUFSIZ];
	gint i;

	gnome_config_set_int
		("gtop/procview/procview_field_mask",
		 gtop_properties.procfields.field_mask);

	string [0] = '\0';
	for (i = 0; i < GTOP_PROCFIELD_COUNT; i++) {
		gchar buffer [BUFSIZ];

		if (*string) strcat (string, ",");
		sprintf (buffer, "%d", gtop_properties.procfields.field_width [i]);
		strcat (string, buffer);
	}

	gnome_config_set_string
		("gtop/procview/procview_field_width", string);
}
