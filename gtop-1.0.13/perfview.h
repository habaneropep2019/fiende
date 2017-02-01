
#ifndef __PERFVIEW_H__
#define __PERFVIEW_H__

#include "global.h"
#include <gtk/gtk.h>

#define vSCALE 1000

enum {
     EVENT_LOAD_AVG_1,
     EVENT_LOAD_AVG_5,
     EVENT_LOAD_AVG_15,
     EVENT_MEM_USED,
     EVENT_MEM_FREE,
     EVENT_MEM_SHARED,
     EVENT_MEM_BUFFERS,
     EVENT_MEM_CACHED,
     EVENT_SWP_USED,
     EVENT_SWP_FREE,
};

void addPerformanceView ();

struct ProcEvent {
     gchar *label;
     gchar *tip;
     gint par;
     gint (*getValue)(gint);
     GdkColor color;
};

extern struct ProcEvent procEvents[];

struct ProcGraph {
     gint *values;
     gint cur;
     gint size;
     gint scale;

     GtkWidget *darea;
     GtkWidget *button;
     GtkStyle *style;

     ProcEvent *event;

     void tick ();
     void draw (gint x, gint width);

     ProcGraph (GtkWidget *d, GtkWidget *b, ProcEvent *e);
     ~ProcGraph ();
     static void setWidgetColor (GtkWidget *w, ProcGraph *g);
};

class ProcMeter {
     GSList *events;
     GtkWidget *hbox;
     GtkWidget *vbox;
     GtkWidget *darea;
     GtkWidget *envel;
     GtkWidget *table;

     gint tx, ty;
     gint first;

     GSList *gList;

     static gint proc_meter_expose (GtkWidget *widget, GdkEventExpose *e,
				    ProcMeter *m);
     static gint proc_meter_size_allocate (GtkWidget *widget, ProcMeter *m);
     static gint update (ProcMeter *m);

     void draw (gint x, gint width);
     void updateGraphs ();

public:
     GtkWidget* widget () { return hbox; }
     void watch (ProcEvent *);
     gint refresh ();

     ProcMeter (GtkWidget *t, gint x, gint y, gchar *s="");
     ~ProcMeter ();
};

#endif
