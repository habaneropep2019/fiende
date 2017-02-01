#include <config.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gnome.h>

#include <gtop-memusage.h>

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <assert.h>

static void	gtop_memusage_class_init	(GTopMemUsageClass *);
static void	gtop_memusage_init		(GTopMemUsage *);
static void	gtop_memusage_destroy		(GtkObject *);
static void	gtop_memusage_map		(GtkWidget *);
static void	gtop_memusage_unmap		(GtkWidget *);
static void	gtop_memusage_unrealize		(GtkWidget *);
static void	gtop_memusage_type_set		(GTopMemUsage *, gint);
static void	gtop_memusage_menu_cb		(GtkWidget *, gpointer);

GnomeUIInfo gtop_memusage_radio_items [] = {
	GNOMEUIINFO_ITEM_DATA (N_("_Resident Sizes of Processes"),
			       N_("Show resident sizes of processes"),
			       gtop_memusage_menu_cb,
			       (gpointer) GTOP_MEMUSAGE_RESIDENT,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Shared Sizes of Processes"),
			       N_("Show shared sizes of processes"),
			       gtop_memusage_menu_cb,
			       (gpointer) GTOP_MEMUSAGE_SHARED,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Total Sizes of Processes"),
			       N_("Show total sizes of processes"),
			       gtop_memusage_menu_cb,
			       (gpointer) GTOP_MEMUSAGE_SIZE,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Virtual Sizes of Processes"),
			       N_("Show virtual sizes of processes"),
			       gtop_memusage_menu_cb,
			       (gpointer) GTOP_MEMUSAGE_VIRTUAL,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("S_wapped Sizes of Processes"),
			       N_("Show swapped sizes of processes"),
			       gtop_memusage_menu_cb,
			       (gpointer) GTOP_MEMUSAGE_SWAP,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_END
};

static GnomeUIInfo view_type_menu [] = {
	GNOMEUIINFO_RADIOLIST (gtop_memusage_radio_items),
	GNOMEUIINFO_END
};

static GnomeUIInfo gtop_memusage_menu [] = {
	GNOMEUIINFO_MENU_VIEW_TREE (view_type_menu),
	GNOMEUIINFO_END
};

enum {
	LAST_SIGNAL
};

static gint gtop_memusage_signals [(int)LAST_SIGNAL + 1];

typedef void (*GTopMemUsageSignal) (GtkObject *, gpointer, gpointer);

static GtkScrolledWindowClass *parent_class = NULL;

guint
gtop_memusage_get_type ()
{
	static guint gtf_type = 0;

	if (!gtf_type) {
		GtkTypeInfo gtf_info = {
			"GTopMemUsage", sizeof (GTopMemUsage),
			sizeof (GTopMemUsageClass),
			(GtkClassInitFunc) gtop_memusage_class_init,
			(GtkObjectInitFunc) gtop_memusage_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		gtf_type = gtk_type_unique
			(gtk_scrolled_window_get_type(), &gtf_info);
	}

	return gtf_type;
}

static gint
gtop_memusage_timeout (gpointer data)
{
	GtkWidget *obj = (GtkWidget *) data;
	GTopMemUsage *memusage = GTOP_MEMUSAGE (obj);

	if (gtop_is_running)
		memusage_update (&memusage->data);

	return TRUE;
}

static void
gtop_memusage_map (GtkWidget *obj)
{
	GTopMemUsage *memusage = GTOP_MEMUSAGE (obj);

	if (GTK_WIDGET_CLASS (parent_class)->map)
		(* GTK_WIDGET_CLASS (parent_class)->map) (obj);

	memusage_update (&memusage->data);
	
	if (memusage->run_tag != -1)
		gtk_timeout_remove (memusage->run_tag);

	memusage->run_tag = gtk_timeout_add
		(gtop_properties.global.update_times [GTOP_UPDATE_MEMUSAGE],
		 gtop_memusage_timeout, obj);
}

static void
gtop_memusage_unmap (GtkWidget *obj)
{
	GTopMemUsage *memusage = GTOP_MEMUSAGE (obj);

	if (memusage->run_tag != -1) {
		gtk_timeout_remove (memusage->run_tag);

		memusage->run_tag = -1;
	}

	if (GTK_WIDGET_CLASS (parent_class)->unmap)
		(* GTK_WIDGET_CLASS (parent_class)->unmap) (obj);
}

static void
gtop_memusage_unrealize (GtkWidget *obj)
{
	GTopMemUsage *memusage = GTOP_MEMUSAGE (obj);

	if (memusage->run_tag != -1) {
		gtk_timeout_remove (memusage->run_tag);

		memusage->run_tag = -1;
	}

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (obj);
}

static void
gtop_memusage_destroy (GtkObject *obj)
{
	GTopMemUsage *memusage = GTOP_MEMUSAGE (obj);

	if (memusage->run_tag != -1) {
		gtk_timeout_remove (memusage->run_tag);

		memusage->run_tag = -1;
	}

	memusage_destroy (GTK_WIDGET (obj));

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
}

static void
gtop_memusage_menu_cb (GtkWidget *widget, gpointer data)
{
	gpointer uidata, uibdata;
	GnomeMDI *mdi;

	uidata = gtk_object_get_data (GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIDATA);
	uibdata = gtk_object_get_data (GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIBDATA);

	g_return_if_fail (GNOME_IS_MDI_CHILD (uibdata));

	mdi = (GnomeMDI *) ((GnomeMDIChild *) uibdata)->parent;

	if (GTK_CHECK_MENU_ITEM (widget)->active && mdi->active_view) {
		GTopMemUsage *memusage = GTOP_MEMUSAGE (mdi->active_view);
		
		gtop_memusage_type_set (memusage, GPOINTER_TO_INT (uidata));
	}
}

gchar *
gtop_memusage_label (GTopMemUsageType ftype)
{
	gchar *label;
	
	switch (ftype) {
	case GTOP_MEMUSAGE_RESIDENT:
		label = _("Memory Usage (resident)");
		break;
	case GTOP_MEMUSAGE_SHARED:
		label = _("Memory Usage (shared)");
		break;
	case GTOP_MEMUSAGE_SIZE:
		label = _("Memory Usage (size)");
		break;
	case GTOP_MEMUSAGE_VIRTUAL:
		label = _("Memory Usage (virtual)");
		break;
	case GTOP_MEMUSAGE_SWAP:
		label = _("Memory Usage (swap)");
		break;
	default:
		g_warning ("Invalid GTopMemUsage type %d", ftype);
		label = _("Memory Usage");
		break;
	}

	return label;
}

static void
gtop_memusage_type_set (GTopMemUsage *memusage, gint ftype)
{
	gchar *label = gtop_memusage_label ((GTopMemUsageType) ftype);

	if ((GTopMemUsageType) ftype != memusage->data.ftype)
		gnome_mdi_child_set_name
			(GNOME_MDI_CHILD (memusage->page), label);

	memusage_type_set (&memusage->data, ftype);

	if (GTK_WIDGET_REALIZED (memusage))
		memusage_update (&memusage->data);
}

static void
gtop_memusage_class_init (GTopMemUsageClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	parent_class = gtk_type_class (gtk_scrolled_window_get_type ());

	gtk_object_class_add_signals
		(object_class, gtop_memusage_signals, LAST_SIGNAL);

	widget_class->map = gtop_memusage_map;
	widget_class->unmap = gtop_memusage_unmap;
	widget_class->unrealize = gtop_memusage_unrealize;

	GTK_OBJECT_CLASS(class)->destroy = gtop_memusage_destroy;
}

static void
gtop_memusage_init (GTopMemUsage *memusage)
{
	GtkWidget *widget = GTK_WIDGET (memusage);
	GtkObject *hadj, *vadj;

	hadj = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	vadj = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	gtk_scrolled_window_set_hadjustment
		(GTK_SCROLLED_WINDOW (memusage), GTK_ADJUSTMENT (hadj));

	gtk_scrolled_window_set_vadjustment
		(GTK_SCROLLED_WINDOW (memusage), GTK_ADJUSTMENT (vadj));

	gtk_widget_set_usize (widget,
			      gtop_properties.graph.default_width,
			      gtop_properties.graph.default_height);

	memusage->run_tag = -1;
}

GtkWidget *
gtop_memusage_new (GTopPage *owner, gint ftype)
{
	GTopMemUsage *memusage = gtk_type_new (gtop_memusage_get_type ());

	memusage->page = owner;

	gtop_memusage_type_set (memusage, ftype);

	memusage_new (&memusage->data, GTK_WIDGET (memusage), ftype);

	gnome_mdi_child_set_menu_template
		(GNOME_MDI_CHILD (owner), gtop_memusage_menu);

	gtk_object_constructed (GTK_OBJECT (memusage));

	return GTK_WIDGET (memusage);
}
