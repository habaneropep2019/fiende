#include <config.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gnome.h>

#include <gtop-fsusage.h>

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <assert.h>

static void	gtop_fsusage_class_init		(GTopFsUsageClass *);
static void	gtop_fsusage_init		(GTopFsUsage *);
static void	gtop_fsusage_destroy		(GtkObject *);
static void	gtop_fsusage_map		(GtkWidget *);
static void	gtop_fsusage_unmap		(GtkWidget *);
static void	gtop_fsusage_unrealize		(GtkWidget *);
static void	gtop_fsusage_type_set		(GTopFsUsage *, gint);
static void	gtop_fsusage_menu_cb		(GtkWidget *, gpointer);

GnomeUIInfo gtop_fsusage_radio_items [] = {
	GNOMEUIINFO_ITEM_DATA (N_("_Total Filesystem Sizes"),
			       N_("Show total filesystem sizes"),
			       gtop_fsusage_menu_cb,
			       (gpointer) GTOP_FSUSAGE_TOTAL,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Used Filesystem Sizes"),
			       N_("Show used filesystem sizes"),
			       gtop_fsusage_menu_cb,
			       (gpointer) GTOP_FSUSAGE_USED,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Free Filesystem Sizes"),
			       N_("Show free filesystem sizes"),
			       gtop_fsusage_menu_cb,
			       (gpointer) GTOP_FSUSAGE_FREE,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_END
};

static GnomeUIInfo view_type_menu [] = {
	GNOMEUIINFO_RADIOLIST (gtop_fsusage_radio_items),
	GNOMEUIINFO_END
};

static GnomeUIInfo gtop_fsusage_menu [] = {
	GNOMEUIINFO_MENU_VIEW_TREE (view_type_menu),
	GNOMEUIINFO_END
};

enum {
	LAST_SIGNAL
};

static gint gtop_fsusage_signals [(int)LAST_SIGNAL + 1];

typedef void (*GTopFsUsageSignal) (GtkObject *, gpointer, gpointer);

static GtkScrolledWindowClass *parent_class = NULL;

guint
gtop_fsusage_get_type ()
{
	static guint gtf_type = 0;

	if (!gtf_type) {
		GtkTypeInfo gtf_info = {
			"GTopFsUsage", sizeof (GTopFsUsage),
			sizeof (GTopFsUsageClass),
			(GtkClassInitFunc) gtop_fsusage_class_init,
			(GtkObjectInitFunc) gtop_fsusage_init,
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
gtop_fsusage_timeout (gpointer data)
{
	GtkWidget *obj = (GtkWidget *) data;
	GTopFsUsage *fsusage = GTOP_FSUSAGE (obj);

	if (gtop_is_running)
		fsusage_update (&fsusage->data);

	return TRUE;
}

static void
gtop_fsusage_map (GtkWidget *obj)
{
	GTopFsUsage *fsusage = GTOP_FSUSAGE (obj);

	if (GTK_WIDGET_CLASS (parent_class)->map)
		(* GTK_WIDGET_CLASS (parent_class)->map) (obj);

	fsusage_update (&fsusage->data);

	if (fsusage->run_tag != -1)
		gtk_timeout_remove (fsusage->run_tag);

	fsusage->run_tag = gtk_timeout_add
		(gtop_properties.global.update_times [GTOP_UPDATE_FSUSAGE],
		 gtop_fsusage_timeout, obj);
}

static void
gtop_fsusage_unmap (GtkWidget *obj)
{
	GTopFsUsage *fsusage = GTOP_FSUSAGE (obj);

	if (fsusage->run_tag != -1) {
		gtk_timeout_remove (fsusage->run_tag);

		fsusage->run_tag = -1;
	}

	if (GTK_WIDGET_CLASS (parent_class)->unmap)
		(* GTK_WIDGET_CLASS (parent_class)->unmap) (obj);
}

static void
gtop_fsusage_unrealize (GtkWidget *obj)
{
	GTopFsUsage *fsusage = GTOP_FSUSAGE (obj);

	if (fsusage->run_tag != -1) {
		gtk_timeout_remove (fsusage->run_tag);

		fsusage->run_tag = -1;
	}

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (obj);
}

static void
gtop_fsusage_destroy (GtkObject *obj)
{
	GTopFsUsage *fsusage = GTOP_FSUSAGE (obj);

	if (fsusage->run_tag != -1) {
		gtk_timeout_remove (fsusage->run_tag);

		fsusage->run_tag = -1;
	}

	fsusage_destroy (GTK_WIDGET (obj));

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
}

static void
gtop_fsusage_menu_cb (GtkWidget *widget, gpointer data)
{
	gpointer uidata, uibdata;
	GnomeMDI *mdi;

	uidata = gtk_object_get_data (GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIDATA);
	uibdata = gtk_object_get_data (GTK_OBJECT (widget), GNOMEUIINFO_KEY_UIBDATA);

	g_return_if_fail (GNOME_IS_MDI_CHILD (uibdata));

	mdi = (GnomeMDI *) ((GnomeMDIChild *) uibdata)->parent;

	if (GTK_CHECK_MENU_ITEM (widget)->active && mdi->active_view) {
		GTopFsUsage *fsusage = GTOP_FSUSAGE (mdi->active_view);
		
		gtop_fsusage_type_set (fsusage, GPOINTER_TO_INT (uidata));
	}
}

gchar *
gtop_fsusage_label (GTopFsUsageType ftype)
{
	gchar *label;
	
	switch (ftype) {
	case GTOP_FSUSAGE_TOTAL:
		label = _("Filesystems (total)");
		break;
	case GTOP_FSUSAGE_USED:
		label = _("Filesystems (used)");
		break;
	case GTOP_FSUSAGE_FREE:
		label = _("Filesystems (free)");
		break;
	default:
		label = _("Filesystems");
		break;
	}

	return label;
}

static void
gtop_fsusage_type_set (GTopFsUsage *fsusage, gint ftype)
{
	gchar *label = gtop_fsusage_label ((GTopFsUsageType) ftype);

	if ((GTopFsUsageType) ftype != fsusage->data.ftype)
		gnome_mdi_child_set_name
			(GNOME_MDI_CHILD (fsusage->page), label);

	fsusage_type_set (&fsusage->data, ftype);

	if (GTK_WIDGET_REALIZED (fsusage))
		fsusage_update (&fsusage->data);
}

static void
gtop_fsusage_class_init (GTopFsUsageClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	parent_class = gtk_type_class (gtk_scrolled_window_get_type ());

	gtk_object_class_add_signals
		(object_class, gtop_fsusage_signals, LAST_SIGNAL);

	widget_class->map = gtop_fsusage_map;
	widget_class->unmap = gtop_fsusage_unmap;
	widget_class->unrealize = gtop_fsusage_unrealize;

	GTK_OBJECT_CLASS(class)->destroy = gtop_fsusage_destroy;
}

static void
gtop_fsusage_init (GTopFsUsage *fsusage)
{
	GtkWidget *widget = GTK_WIDGET (fsusage);
	GtkObject *hadj, *vadj;

	hadj = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	vadj = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

	gtk_scrolled_window_set_hadjustment
		(GTK_SCROLLED_WINDOW (fsusage), GTK_ADJUSTMENT (hadj));

	gtk_scrolled_window_set_vadjustment
		(GTK_SCROLLED_WINDOW (fsusage), GTK_ADJUSTMENT (vadj));

	gtk_widget_set_usize (widget,
			      gtop_properties.graph.default_width,
			      gtop_properties.graph.default_height);

	fsusage->run_tag = -1;
}

GtkWidget *
gtop_fsusage_new (GTopPage *owner, gint ftype)
{
	GTopFsUsage *fsusage = gtk_type_new (gtop_fsusage_get_type ());

	fsusage->page = owner;

	gtop_fsusage_type_set (fsusage, ftype);

	fsusage_new (&fsusage->data, GTK_WIDGET (fsusage), ftype);

	gnome_mdi_child_set_menu_template
		(GNOME_MDI_CHILD (owner), gtop_fsusage_menu);

	gtk_object_constructed (GTK_OBJECT (fsusage));

	return GTK_WIDGET (fsusage);
}
