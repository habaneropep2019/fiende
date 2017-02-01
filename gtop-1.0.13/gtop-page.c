#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gnome.h>

#include <gtop-page.h>
#include <gtop-fsusage.h>
#include <gtop-memusage.h>
#include <gtop-procview.h>

#include <details.h>

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <assert.h>

static void       gtop_page_class_init		(GTopPageClass *);
static void       gtop_page_init		(GTopPage *);
static GtkWidget *gtop_page_create_view		(GnomeMDIChild *);
static gchar     *gtop_page_get_config_string	(GnomeMDIChild *);
static void       gtop_page_destroy		(GtkObject *);

GnomeUIInfo page_menu[] = {
	{ GNOME_APP_UI_ENDOFINFO }
};

enum {
	LAST_SIGNAL
};

static gint gtop_page_signals [(int)LAST_SIGNAL + 1];

typedef void (*GTopPageSignal) (GtkObject *, gpointer, gpointer);

static GnomeMDIChildClass *parent_class = NULL;

guint
gtop_page_get_type ()
{
	static guint doc_type = 0;

	if (!doc_type) {
		GtkTypeInfo doc_info = {
			"GTopPage",
			sizeof (GTopPage),
			sizeof (GTopPageClass),
			(GtkClassInitFunc) gtop_page_class_init,
			(GtkObjectInitFunc) gtop_page_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL,
		};
    
		doc_type = gtk_type_unique
			(gnome_mdi_child_get_type (), &doc_info);
	}
	
	return doc_type;
}

static GtkWidget *
gtop_page_create_view (GnomeMDIChild *child)
{
	GTopPage *page = GTOP_PAGE (child);
	GtkWidget *new_view = NULL;

#if DEBUG
	printf ("gtop_page_create_view (%p) - %d = %p\n", child,
		page->type, new_view);
#endif

	switch (GTOP_PAGE (child)->type) {
	case GTOP_PAGE_PROCVIEW:
		new_view = gtop_procview_new (page, page->subtype);
		break;
	case GTOP_PAGE_MEMUSAGE:
		new_view = gtop_memusage_new (page, page->subtype);
		break;
	case GTOP_PAGE_FSUSAGE:
		new_view = gtop_fsusage_new (page, page->subtype);
		break;
	case GTOP_PAGE_GENERIC:
		page = GTOP_PAGE (child);
		new_view = page->generic_func (page->generic_data);
		break;
	}

	return new_view;
}

static gchar *
gtop_page_get_config_string (GnomeMDIChild *child)
{
	GTopPage *page;

	g_return_val_if_fail (child != NULL, NULL);
	g_return_val_if_fail (IS_GTOP_PAGE (child), NULL);

	page = GTOP_PAGE (child);

	if (page->type == GTOP_PAGE_GENERIC)
	    return g_strdup (page->config_string);
	else
	    return g_strdup_printf ("%d/%d", page->type, page->subtype);
}

static void
gtop_page_destroy (GtkObject *obj)
{
	GTopPage *page;
	
	page = GTOP_PAGE (obj);
	
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy)
			(GTK_OBJECT (page));
}

static void
gtop_page_class_init (GTopPageClass *class)
{
	GtkObjectClass *object_class;
	GnomeMDIChildClass *child_class;

	object_class = (GtkObjectClass*) class;
	child_class = GNOME_MDI_CHILD_CLASS (class);

	gtk_object_class_add_signals
		(object_class, gtop_page_signals, LAST_SIGNAL);

	object_class->destroy = gtop_page_destroy;

	child_class->create_view = (GnomeMDIChildViewCreator)
		gtop_page_create_view;

	child_class->get_config_string = (GnomeMDIChildConfigFunc)
		gtop_page_get_config_string;

	parent_class = gtk_type_class (gnome_mdi_child_get_type ());
}

static void
gtop_page_init (GTopPage *page)
{
	gnome_mdi_child_set_menu_template
		(GNOME_MDI_CHILD (page), page_menu);
}

GTopPage *
gtop_page_new (GTopPageType type, gint subtype)
{
	GTopPage *page = gtk_type_new (gtop_page_get_type ());

	page->type = type;
	page->subtype = subtype;

	switch (page->type) {
	case GTOP_PAGE_PROCVIEW:
		gnome_mdi_child_set_name
			(GNOME_MDI_CHILD (page),
			 gtop_procview_label ((GTopProcViewType) subtype));
		break;
	case GTOP_PAGE_MEMUSAGE:
		gnome_mdi_child_set_name
			(GNOME_MDI_CHILD (page),
			 gtop_memusage_label ((GTopMemUsageType) subtype));
		break;
	case GTOP_PAGE_FSUSAGE:
		gnome_mdi_child_set_name
			(GNOME_MDI_CHILD (page),
			 gtop_fsusage_label ((GTopFsUsageType) subtype));
		break;
	case GTOP_PAGE_GENERIC:
		g_error ("You must use gtop_generic_page_new () to "
			 "create a GTopPage of type GTOP_PAGE_GENERIC.");
		break;
	}
	
	return page;
}

GTopPage *
gtop_generic_page_new (GTopGenericFunc generic_func, const gchar *label,
		       const gchar *config_string, gpointer generic_data)
{
	GTopPage *page = gtk_type_new (gtop_page_get_type ());

	page->type = GTOP_PAGE_GENERIC;

	page->generic_func = generic_func;
	page->generic_data = generic_data;

	page->config_string = g_strdup (config_string);
	page->label = g_strdup (label);

	gnome_mdi_child_set_name (GNOME_MDI_CHILD (page), page->label);

	return page;
}

static GtkWidget *
create_mem_graph (gpointer data)
{
    return gtop_details_create_mem_graph (GPOINTER_TO_INT (data));
}

static GtkWidget *
create_mem_maps (gpointer data)
{
    return gtop_details_create_mem_map (GPOINTER_TO_INT (data));
}

GnomeMDIChild *
gtop_page_create_from_config (const gchar *string)
{
	gint type, subtype;
	pid_t pid;

	g_return_val_if_fail (string != NULL, NULL);

	if (sscanf (string, "%d/%d", &type, &subtype) == 2)
	    return GNOME_MDI_CHILD (gtop_page_new (type, subtype));

	if (sscanf (string, "generic/graph_maps/%d", &pid) == 1) {
	    GTopPage *page;
	    gchar *label, *config;

	    label = g_strdup_printf (_("Memory Map (process %d)"), pid);
	    config = g_strdup (string);

	    page = gtop_generic_page_new
		(create_mem_graph, label, config, GINT_TO_POINTER (pid));

	    return GNOME_MDI_CHILD (page);
	}

	if (sscanf (string, "generic/text_maps/%d", &pid) == 1) {
	    GTopPage *page;
	    gchar *label, *config;

	    label = g_strdup_printf (_("Memory Map (process %d)"), pid);
	    config = g_strdup (string);

	    page = gtop_generic_page_new
		(create_mem_maps, label, config, GINT_TO_POINTER (pid));

	    return GNOME_MDI_CHILD (page);
	}

	return NULL;
}
