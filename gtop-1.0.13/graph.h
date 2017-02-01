#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <gtk/gtk.h>

#include <global.h>
#include <properties.h>

BEGIN_GNOME_DECLS

extern GdkColor graph_default_colors [GRAPH_DEFAULT_COLORS];

enum _GraphCmd {
	GRAPH_FIRST,
	GRAPH_NEXT,
	GRAPH_VALUE,
	GRAPH_LABEL,
	GRAPH_HEAD,
	GRAPH_TAIL,
};

typedef enum _GraphCmd GraphCmd;

typedef struct _Graph Graph;

typedef gpointer (*GraphDataFunc) (Graph *, GraphCmd, gpointer, gint64 *);

struct _Graph {
	GtkWidget *da;

	GdkPixmap *bs;
	GdkColor *colors;
	GdkColor *foreground;
	GdkColor *background;

	gint colors_allocated;
	gint n;
	gint minimum_height;
	gint minimum_width;
	gint width, height;

	gpointer user_data;

	GraphDataFunc data_fn;
};

void          graph_destroy (Graph *);
Graph       * graph_new (GtkWidget *, GraphDataFunc, gpointer);
GtkWidget   * graph_widget (Graph *);
void          graph_update (Graph *);

extern GnomePropertyDescriptor GraphProperty_Descriptor;

END_GNOME_DECLS

#endif
