#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gnome.h>

#include <gtop-graph.h>
#include <graph.h>

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include <assert.h>

static void	gtop_graph_class_init	(GTopGraphClass *);
static void	gtop_graph_init		(GTopGraph *);
static void	gtop_graph_destroy	(GtkObject *);
static void	gtop_graph_map		(GtkWidget *);
static void	gtop_graph_unmap	(GtkWidget *);
static void	gtop_graph_redraw	(GTopGraph *);

enum {
	REDRAW_GRAPH,
	LAST_SIGNAL
};

static gint gtop_graph_signals [LAST_SIGNAL];

typedef void (*GTopGraphSignal) (GtkObject *, gpointer, gpointer);

static GtkDrawingAreaClass *parent_class = NULL;

static void
gtop_graph_marshal (GtkObject		*object,
		    GtkSignalFunc	func,
		    gpointer		func_data,
		    GtkArg		*args)
{
	GTopGraphSignal rfunc;
	
	rfunc = (GTopGraphSignal) func;
	
	(* rfunc) (object, GTK_VALUE_POINTER(args[0]), func_data);
}

guint
gtop_graph_get_type ()
{
	static guint gtf_type = 0;

	if (!gtf_type) {
		GtkTypeInfo gtf_info = {
			"GTopGraph", sizeof (GTopGraph),
			sizeof (GTopGraphClass),
			(GtkClassInitFunc) gtop_graph_class_init,
			(GtkObjectInitFunc) gtop_graph_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};

		gtf_type = gtk_type_unique
			(gtk_drawing_area_get_type(), &gtf_info);
	}

	return gtf_type;
}

static void
gtop_graph_map (GtkWidget *obj)
{
	GTopGraph *graph = GTOP_GRAPH (obj);

#if DEBUG
	printf ("gtop_graph_map (%p)\n", obj);
#endif

	if (GTK_WIDGET_CLASS (parent_class)->map)
		(* GTK_WIDGET_CLASS (parent_class)->map) (obj);

	graph_update (graph->graph);

#if DEBUG
	printf ("gtop_graph_map: %p - (%d, %d)\n", graph->parent,
		graph->graph->width, graph->graph->height);
#endif
}

static void
gtop_graph_unmap (GtkWidget *obj)
{
#if DEBUG
	printf ("gtop_graph_unmap (%p)\n", obj);
#endif

	if (GTK_WIDGET_CLASS (parent_class)->unmap)
		(* GTK_WIDGET_CLASS (parent_class)->unmap) (obj);
}

static void
gtop_graph_redraw (GTopGraph *graph)
{
	graph_update (graph->graph);
}

static void
gtop_graph_destroy (GtkObject *obj)
{
	graph_destroy (GTOP_GRAPH (obj)->graph);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
}

static void
gtop_graph_class_init (GTopGraphClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;

	parent_class = gtk_type_class (gtk_drawing_area_get_type ());

	gtop_graph_signals [REDRAW_GRAPH] =
		gtk_signal_new ("redraw_graph",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET
				(GTopGraphClass, redraw_graph),
				gtop_graph_marshal,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals
		(object_class, gtop_graph_signals, LAST_SIGNAL);

	class->redraw_graph = gtop_graph_redraw;

	widget_class->map = gtop_graph_map;
	widget_class->unmap = gtop_graph_unmap;

	GTK_OBJECT_CLASS(class)->destroy = gtop_graph_destroy;
}

static void
gtop_graph_init (GTopGraph *graph)
{
}

GtkWidget *
gtop_graph_new (GtkWidget *parent, GraphDataFunc data_func, gpointer user_data)
{
	GtkWidget *widget = gtk_type_new (gtop_graph_get_type ());
	GTopGraph *graph = GTOP_GRAPH (widget);

	graph->parent = parent;

	graph->user_data = user_data;

	graph->graph = graph_new
		(GTK_WIDGET (GTK_DRAWING_AREA (graph)), data_func, user_data);

	return widget;
}
