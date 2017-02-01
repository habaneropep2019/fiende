#include <config.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gnome.h>

#include <gtop-procview.h>

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <assert.h>

static void	gtop_procview_class_init	(GTopProcViewClass *);
static void	gtop_procview_init		(GTopProcView *);
static void	gtop_procview_destroy		(GtkObject *);
static void	gtop_procview_map		(GtkWidget *);
static void	gtop_procview_unmap		(GtkWidget *);
static void	gtop_procview_unrealize		(GtkWidget *);
static void	gtop_procview_type_set		(GTopProcView *, gint);
static void	gtop_procview_menu_cb		(GtkWidget *, gpointer);
static void	gtop_procview_flags_cb		(GtkWidget *, gpointer);

static void	gtop_procview_size_request	(GtkWidget *, GtkRequisition *);
static void	gtop_procview_size_allocate	(GtkWidget *, GtkAllocation *);

GnomeUIInfo gtop_procview_radio_items [] = {
	GNOMEUIINFO_ITEM_DATA (N_("_All Processes"),
			       N_("Show all processes"),
			       gtop_procview_menu_cb,
			       (gpointer) GTOP_PROCVIEW_ALL,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_User Processes"),
			       N_("Only show user processes"),
			       gtop_procview_menu_cb,
			       (gpointer) GTOP_PROCVIEW_USER,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_END
};

/* If you add items to this menu in front of the GNOME_APP_UI_RADIOITEMS,
 * you need to increase the starting number of the GTopProcViewType in
 * procview.h.
 */

static GnomeUIInfo view_type_menu [] = {
	GNOMEUIINFO_TOGGLEITEM_DATA
	(N_("Only _TTY Processes"),
	 N_("Only show processes with a controlling TTY"),
	 gtop_procview_flags_cb, (gpointer) GTOP_PROCVIEW_TTY,
	 GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_TOGGLEITEM_DATA
	(N_("Hide _Idle Processes"), N_("Hide idle processes"),
	 gtop_procview_flags_cb, (gpointer) GTOP_PROCVIEW_IDLE,
	 GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_TOGGLEITEM_DATA
	(N_("Hide _System Processes"), N_("Hide system processes"),
	 gtop_procview_flags_cb, (gpointer) GTOP_PROCVIEW_SYSTEM,
	 GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_RADIOLIST (gtop_procview_radio_items),
	GNOMEUIINFO_END
};

static GnomeUIInfo gtop_procview_menu [] = {
	GNOMEUIINFO_MENU_VIEW_TREE (view_type_menu),
	GNOMEUIINFO_END
};

enum {
	LAST_SIGNAL
};

static gint gtop_procview_signals [(int)LAST_SIGNAL + 1];

typedef void (*GTopProcViewSignal) (GtkObject *, gpointer, gpointer);

static GtkBinClass *parent_class = NULL;

guint
gtop_procview_get_type ()
{
	static guint gtf_type = 0;

	if (!gtf_type) {
		GtkTypeInfo gtf_info = {
			"GTopProcView", sizeof (GTopProcView),
			sizeof (GTopProcViewClass),
			(GtkClassInitFunc) gtop_procview_class_init,
			(GtkObjectInitFunc) gtop_procview_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		gtf_type = gtk_type_unique
			(gtk_bin_get_type(), &gtf_info);
	}

	return gtf_type;
}

static gint
gtop_procview_timeout (gpointer data)
{
	GtkWidget *obj = (GtkWidget *) data;
	GTopProcView *procview = GTOP_PROCVIEW (obj);

	if (gtop_is_running)
		procview_update (&procview->data);

	return TRUE;
}

static void
gtop_procview_map (GtkWidget *obj)
{
	GTopProcView *procview = GTOP_PROCVIEW (obj);

	if (GTK_WIDGET_CLASS (parent_class)->map)
		(* GTK_WIDGET_CLASS (parent_class)->map) (obj);

	procview_update (&procview->data);

	if (procview->run_tag != -1)
		gtk_timeout_remove (procview->run_tag);

	procview->run_tag = gtk_timeout_add
		(gtop_properties.global.update_times [GTOP_UPDATE_PROCVIEW],
		 gtop_procview_timeout, obj);

	procview_map (&procview->data);
}

static void
gtop_procview_unmap (GtkWidget *obj)
{
	GTopProcView *procview = GTOP_PROCVIEW (obj);

	if (procview->run_tag != -1) {
		gtk_timeout_remove (procview->run_tag);

		procview->run_tag = -1;
	}

	procview_unmap (&procview->data);

	if (GTK_WIDGET_CLASS (parent_class)->unmap)
		(* GTK_WIDGET_CLASS (parent_class)->unmap) (obj);
}

static void
gtop_procview_unrealize (GtkWidget *obj)
{
	GTopProcView *procview = GTOP_PROCVIEW (obj);

	if (procview->run_tag != -1) {
		gtk_timeout_remove (procview->run_tag);

		procview->run_tag = -1;
	}

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (obj);
}

static void
gtop_procview_destroy (GtkObject *obj)
{
	GTopProcView *procview = GTOP_PROCVIEW (obj);

	if (procview->run_tag != -1) {
		gtk_timeout_remove (procview->run_tag);
		
		procview->run_tag = -1;
	}

	procview_unmap (&procview->data);

	procview_destroy (GTK_WIDGET (obj));

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
}

static void
gtop_procview_menu_cb (GtkWidget *widget, gpointer data)
{
	gpointer uidata, uibdata;
	GnomeMDI *mdi;

	uidata = gtk_object_get_data
		(GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIDATA);
	uibdata = gtk_object_get_data
		(GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIBDATA);

	g_return_if_fail (GNOME_IS_MDI_CHILD (uibdata));

	mdi = (GnomeMDI *) ((GnomeMDIChild *) uibdata)->parent;

	if (GTK_CHECK_MENU_ITEM (widget)->active && mdi->active_view) {
		GTopProcView *procview = GTOP_PROCVIEW (mdi->active_view);
		
		gtop_procview_type_set (procview, GPOINTER_TO_INT (uidata));
	}
}

static void
gtop_procview_flags_cb (GtkWidget *widget, gpointer data)
{
	gpointer uidata, uibdata;
	GTopProcView *procview;
	GTopProcViewData *d;
	GnomeMDI *mdi;

	uidata = gtk_object_get_data
		(GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIDATA);
	uibdata = gtk_object_get_data
		(GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIBDATA);

	g_return_if_fail (GNOME_IS_MDI_CHILD (uibdata));

	mdi = (GnomeMDI *) ((GnomeMDIChild *) uibdata)->parent;

	if (!mdi->active_view)
		return;

	procview = GTOP_PROCVIEW (mdi->active_view);
	d = &procview->data;

	if (GTK_CHECK_MENU_ITEM (widget)->active)
		d->proc_selection_flags |= (1 << GPOINTER_TO_INT (uidata));
	else
		d->proc_selection_flags &= ~(1 << GPOINTER_TO_INT (uidata));

	if (GTK_WIDGET_REALIZED (procview))
		procview_update (&procview->data);
}

gchar *
gtop_procview_label (GTopProcViewType ftype)
{
	gchar *label;
	
	switch (ftype) {
	case GTOP_PROCVIEW_ALL:
		label = _("Processes (all)");
		break;
	case GTOP_PROCVIEW_USER:
		label = _("Processes (user)");
		break;
	default:
		label = _("Processes");
		break;
	}

	return label;
}

static void
gtop_procview_type_set (GTopProcView *procview, gint ftype)
{
	gchar *label = gtop_procview_label ((GTopProcViewType) ftype);

	if ((GTopProcViewType) ftype != procview->data.ftype)
		gnome_mdi_child_set_name
			(GNOME_MDI_CHILD (procview->page), label);

	procview_type_set (&procview->data, ftype);

	if (GTK_WIDGET_REALIZED (procview))
		procview_update (&procview->data);
}

static void
gtop_procview_class_init (GTopProcViewClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	parent_class = gtk_type_class (gtk_bin_get_type ());

	gtk_object_class_add_signals
		(object_class, gtop_procview_signals, LAST_SIGNAL);

	widget_class->map = gtop_procview_map;
	widget_class->unmap = gtop_procview_unmap;
	widget_class->unrealize = gtop_procview_unrealize;

	widget_class->size_request = gtop_procview_size_request;
	widget_class->size_allocate = gtop_procview_size_allocate;

	GTK_OBJECT_CLASS(class)->destroy = gtop_procview_destroy;
}

static void
gtop_procview_init (GTopProcView *procview)
{
	GtkWidget *widget = GTK_WIDGET (procview);

	gtk_widget_set_usize (widget,
			      gtop_properties.graph.default_width,
			      gtop_properties.graph.default_height);

	procview->run_tag = -1;
}

GtkWidget *
gtop_procview_new (GTopPage *owner, gint ftype)
{
	GTopProcView *procview = gtk_type_new (gtop_procview_get_type ());

	procview->page = owner;

	gtop_procview_type_set (procview, ftype);

	procview_new (&procview->data, GTK_WIDGET (procview), ftype);

	gnome_mdi_child_set_menu_template
		(GNOME_MDI_CHILD (owner), gtop_procview_menu);

	return GTK_WIDGET (procview);
}

static void
gtop_procview_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GTopProcView *procview;
	GtkBin *bin;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_GTOP_PROCVIEW (widget));
	g_return_if_fail (requisition != NULL);

	procview = GTOP_PROCVIEW (widget);
	bin = GTK_BIN (widget);

	requisition->width = GTK_CONTAINER (widget)->border_width * 2;
	requisition->height = GTK_CONTAINER (widget)->border_width * 2;

	if (bin->child && GTK_WIDGET_VISIBLE (bin->child)) {
		GtkRequisition child_requisition;
		gtk_widget_size_request (bin->child, &child_requisition);

		requisition->width += child_requisition.width;
		requisition->height += child_requisition.height;
	}
}

static void
gtop_procview_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GTopProcView *procview;
	GtkBin *bin;
	GtkAllocation child_allocation;
	gint width, height;
	gint x, y;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_GTOP_PROCVIEW (widget));
	g_return_if_fail (allocation != NULL);

	widget->allocation = *allocation;
	procview = GTOP_PROCVIEW (widget);
	bin = GTK_BIN (widget);
  
	if (bin->child && GTK_WIDGET_VISIBLE (bin->child)) {
		x = GTK_CONTAINER (procview)->border_width;
		y = GTK_CONTAINER (procview)->border_width;
		width = MAX (allocation->width - 2 * x, 0);
		height = MAX (allocation->height - 2 * y, 0);

		child_allocation.x = x + allocation->x;
		child_allocation.y = y + allocation->y;
		child_allocation.width = width;
		child_allocation.height = height;

		gtk_widget_size_allocate (bin->child, &child_allocation);
	}
}
