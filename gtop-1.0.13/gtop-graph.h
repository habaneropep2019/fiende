/*
 * gtop-graph.h
 * written by Martin Baulig <martin@home-of-linux.org>
 * based upon hex-document.h from Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef __GTOP_GRAPH_H__
#define __GTOP_GRAPH_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gnome.h>

#include <graph.h>

BEGIN_GNOME_DECLS

#define GTOP_GRAPH(obj)		GTK_CHECK_CAST (obj, gtop_graph_get_type (), GTopGraph)
#define GTOP_GRAPH_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtop_graph_get_type (), GTopGraphClass)
#define IS_GTOP_GRAPH(obj)	GTK_CHECK_TYPE (obj, gtop_graph_get_type ())

typedef struct _GTopGraph	GTopGraph;
typedef struct _GTopGraphClass	GTopGraphClass;

struct _GTopGraph
{
	GtkDrawingArea drawing_area;

	GtkWidget *parent;

	gpointer user_data;

	Graph *graph;
};

struct _GTopGraphClass
{
	GtkDrawingAreaClass parent_class;
	
	void (*redraw_graph) (GTopGraph *);
};

guint gtop_graph_get_type (void);
GtkWidget *gtop_graph_new (GtkWidget *, GraphDataFunc, gpointer);

END_GNOME_DECLS

#endif /* __GTOP_GRAPH_H__ */

