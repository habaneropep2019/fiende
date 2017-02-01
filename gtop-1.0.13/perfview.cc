extern "C" {
#include "proc/sysinfo.h"
}
#include "perfview.h"
#include <string.h>

static GtkWidget *table;
static GtkWidget *nLabel;

static ProcMeter *m1;
static ProcMeter *m2;
static ProcMeter *m3;
static ProcMeter *m4;
static ProcMeter *m5;
static ProcMeter *m6;

void addPerformanceView () {

     nLabel = gtk_label_new (_(" Performance "));
     table =  gtk_table_new (3, 4, FALSE);
     gtk_container_border_width (GTK_CONTAINER (table), 6);
     gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, nLabel);

     m1 = new ProcMeter (table, 0, 0, _("CPU Load average"));
     m2 = new ProcMeter (table, 2, 0, _("Main memory"));
     m3 = new ProcMeter (table, 0, 1);
     m4 = new ProcMeter (table, 2, 1, _("Swap memory"));
     m5 = new ProcMeter (table, 0, 2);
     m6 = new ProcMeter (table, 2, 2);

     m1->watch (procEvents+EVENT_LOAD_AVG_1);
     m1->watch (procEvents+EVENT_LOAD_AVG_5);
     m1->watch (procEvents+EVENT_LOAD_AVG_15);

     m2->watch (procEvents+EVENT_MEM_USED);
     m2->watch (procEvents+EVENT_MEM_FREE);
     m2->watch (procEvents+EVENT_MEM_SHARED);
     m2->watch (procEvents+EVENT_MEM_BUFFERS);
     m2->watch (procEvents+EVENT_MEM_CACHED);

     m4->watch (procEvents+EVENT_SWP_USED);
     m4->watch (procEvents+EVENT_SWP_FREE);

     gtk_widget_show (nLabel);
     gtk_widget_show (table);
}

///////////////////////////////////////////////////////////
// ProcMeter
//

gint ProcMeter::proc_meter_expose (GtkWidget *widget, GdkEventExpose *e, ProcMeter *m) {

#define AL e->area
     gdk_draw_rectangle (widget->window, widget->style->black_gc, TRUE,
			 AL.x, AL.y,
			 AL.width, AL.height);
     if (m->first) {
	  m->updateGraphs ();
	  m->first = 0;
     } else
	  m->draw (AL.x, AL.width);
#undef AL
}

gint ProcMeter::proc_meter_size_allocate (GtkWidget *widget, ProcMeter *m) {
}

gint ProcMeter::update (ProcMeter *m) {
     m->updateGraphs ();

     return TRUE;
}

ProcMeter::ProcMeter (GtkWidget *t, gint x, gint y, gchar *s) {
     table = t;
     tx = x;
     ty = y;
     first = 1;

     gList = g_slist_alloc ();

     vbox = gtk_vbox_new (FALSE, 0);

     darea = gtk_drawing_area_new ();
     gtk_drawing_area_size (GTK_DRAWING_AREA (darea), -1, 100);
     gtk_widget_set_events (darea, GDK_EXPOSURE_MASK);
     gtk_signal_connect (GTK_OBJECT (darea), "expose_event",
			 GTK_SIGNAL_FUNC (ProcMeter::proc_meter_expose),
			 (gpointer)this);
     gtk_signal_connect (GTK_OBJECT (darea), "size_allocate",
			 GTK_SIGNAL_FUNC (ProcMeter::proc_meter_size_allocate),
			 (gpointer)this);

     envel = gtk_frame_new (s);
     gtk_frame_set_shadow_type (GTK_FRAME (envel), GTK_SHADOW_NONE);
     GtkWidget *frame = gtk_frame_new (NULL);
     gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

     gtk_container_add (GTK_CONTAINER (frame), darea);
     gtk_container_add (GTK_CONTAINER (envel), frame);
     gtk_table_attach (GTK_TABLE (table), envel,
		       tx, tx+1, ty, ty+1,
		       GTK_EXPAND|GTK_FILL, GTK_FILL,
		       5, 5);
     gtk_table_attach (GTK_TABLE (table), vbox,
		       tx+1, tx+2, ty, ty+1,
		       GTK_FILL, GTK_FILL,
		       5, 5);


     gtk_widget_show (darea);
     gtk_widget_show (vbox);
     gtk_widget_show (frame);
     gtk_widget_show (envel);

     gtk_timeout_add (3000, update, (gpointer)this);
}

ProcMeter::~ProcMeter () {
     g_slist_free (gList);
}

gint ProcMeter::refresh () {
     //     draw (0, darea->allocation.width);
}

void ProcMeter::watch (ProcEvent *e) {
     GtkWidget *hb = gtk_hbox_new (FALSE, 5);
     GtkWidget *label = gtk_label_new (_(e->label));
     gtk_misc_set_alignment (GTK_MISC(label), 0, 0.5);
     GtkWidget *button = gtk_button_new ();
     gtk_widget_set_usize (button, 30, 3);

     gtk_box_pack_start (GTK_BOX (hb), label, FALSE, FALSE, 0);
     gtk_box_pack_end (GTK_BOX (hb), button, FALSE, FALSE, 0);
     gtk_box_pack_end (GTK_BOX (vbox), hb, FALSE, FALSE, 0);

     GtkTooltips *tips = gtk_tooltips_new ();
     gtk_tooltips_set_tips (tips, button, e->tip);

     gtk_widget_show (button);
     gtk_widget_show (label);
     gtk_widget_show (hb);

     ProcGraph *g = new ProcGraph (darea, button, e);

     g_slist_append (gList, (gpointer)g);
}

void ProcMeter::updateGraphs () {

     if (GTK_WIDGET_DRAWABLE (darea)) {
	  gdk_window_copy_area (darea->window,
				darea->style->black_gc,
				0, 0,
				darea->window,
				1, 0,
				darea->allocation.width-1,
				darea->allocation.height);

	  gdk_draw_line (darea->window,
			 darea->style->black_gc,
			 darea->allocation.width-1,
			 0,
			 darea->allocation.width-1,
			 darea->allocation.height-1);
     }

     GSList *cur = gList;
     do
	  if (cur->data)
	       ((ProcGraph *)cur->data)->tick ();
     while ((cur = cur->next));
}

void ProcMeter::draw (gint x, gint width) {

     GSList *cur = gList;
     do
	  if (cur->data)
	       ((ProcGraph *)cur->data)->draw (x, width);
     while ((cur = cur->next));
}

///////////////////////////////////////////////////////////
// ProcGraph
//

static gint globalClock=0;

ProcGraph::ProcGraph (GtkWidget *d, GtkWidget *b, ProcEvent *e) {
     cur = 0;
     scale = 1;
     values = NULL;

     event = e;
     darea = d;
     button = b;

     style = gtk_style_new ();
     style->bg [GTK_STATE_NORMAL] = event->color;
     style->bg [GTK_STATE_PRELIGHT] = event->color;

     gtk_widget_set_style (button, style);
}

void ProcGraph::tick () {
     globalClock++;
     
     if (!values && GTK_WIDGET_MAPPED (darea)) {
	  size = darea->allocation.width;
	  values = new gint [size];
	  int i, v = event->getValue (event->par);
	  for (i=0; i<size; i++)
	       values[i] = v;
     }
	  
     if (values) {
	  cur++;
	  if (cur>=size)
	       cur = 0;
	  values [cur] = event->getValue (event->par);
     }

     draw (darea->allocation.width-1, 1);
}

void ProcGraph::draw (gint x, gint width) {
#define YP(x) ((x)*(darea->allocation.height-1)/(scale*vSCALE))

     gint i, p1, p2;
     for (i=0; i<width; i++, x++)
	  if (values && GTK_WIDGET_DRAWABLE (darea)) {
	       p1=cur-(size-x-1);
	       p2=p1-1;

	       if (p1<0)
		    p1=size+p1;
	       if (p2<0)
		    p2=size+p2;
	       gdk_draw_line (darea->window,
			      button->style->bg_gc [GTK_STATE_NORMAL],
			      x,
			      darea->allocation.height-1-YP (values[p1]),
			      x-1,
			      darea->allocation.height-1-YP (values[p2]));
	  }
#undef YP
}

ProcGraph::~ProcGraph () {
     if (values)
	  delete [] values;
}

///////////////////////////////////////////////////////////
// ProcEvent
//

/////////////////// load //////////////

static gint loadClock=0;
static double loadAvg [3];

static void getLoadAvg () {
     if (loadClock != globalClock) {
	  loadavg (&loadAvg [0], &loadAvg [1], &loadAvg [2]);
	  loadClock = globalClock;
     }
}

static gint procCPULoadAvg (gint par) {

     getLoadAvg ();

     return (gint)(loadAvg [par]*vSCALE);
}

//////////// mem info //////////////////

static gint memClock = 0;
static gint memInfo [EVENT_SWP_FREE-EVENT_MEM_USED];
static gint memTotal = -1;

static void getMemInfo () {
     if (memClock != globalClock) {
	  unsigned** mi = meminfo ();

	  memInfo [EVENT_MEM_USED-EVENT_MEM_USED] = mi [meminfo_main][meminfo_used];
	  memInfo [EVENT_MEM_FREE-EVENT_MEM_USED] = mi [meminfo_main][meminfo_free];
	  memInfo [EVENT_MEM_SHARED-EVENT_MEM_USED] = mi [meminfo_main][meminfo_shared];
	  memInfo [EVENT_MEM_BUFFERS-EVENT_MEM_USED] = mi [meminfo_main][meminfo_buffers];
	  memInfo [EVENT_MEM_CACHED-EVENT_MEM_USED] = mi [meminfo_main][meminfo_cached];
	  memInfo [EVENT_SWP_USED-EVENT_MEM_USED] = mi [meminfo_swap][meminfo_used];
	  memInfo [EVENT_SWP_FREE-EVENT_MEM_USED] = mi [meminfo_swap][meminfo_free];

	  if (memTotal < 0)
	       memTotal = 
		    (mi [meminfo_main][meminfo_total] > mi [meminfo_swap][meminfo_total])
		    ? mi [meminfo_main][meminfo_total]
		    : mi [meminfo_swap][meminfo_total];
	  memClock = globalClock;
     }
}

static gint procMemInfo (gint par) {

     getMemInfo ();

     return (gint)(vSCALE*((gdouble)memInfo [par-EVENT_MEM_USED]/memTotal));
}

struct ProcEvent procEvents[] = {{
	  N_("load1"), N_("CPU load avg 1 min"),
	  EVENT_LOAD_AVG_1, procCPULoadAvg,
	  {0, 65535, 65535, 0},
     },{
	  N_("load5"), N_("CPU load avg 5 min"),
	  EVENT_LOAD_AVG_5, procCPULoadAvg,
	  {0, 0, 65535, 0},
     },{
	  N_("load15"), N_("CPU load avg 15 min"),
	  EVENT_LOAD_AVG_15, procCPULoadAvg,
	  {0, 65535, 32767, 0},
     },{
	  N_("m used"), N_("main memory used"),
	  EVENT_MEM_USED, procMemInfo,
	  {0, 65535, 65535, 0},
     },{
	  N_("m free"), N_("main memory free"),
	  EVENT_MEM_FREE, procMemInfo,
	  {0, 0, 65535, 0},
     },{
	  N_("m shrd"), N_("main memory shared"),
	  EVENT_MEM_SHARED, procMemInfo,
	  {0, 0, 65535, 65535},
     },{
	  N_("m buf"), N_("main memory buffers"),
	  EVENT_MEM_BUFFERS, procMemInfo,
	  {0, 65535, 32767, 0},
     },{
	  N_("m cache"), N_("main memory cached"),
	  EVENT_MEM_CACHED, procMemInfo,
	  {0, 65535, 65535, 65535},
     },{
	  N_("s used"), N_("swap memory used"),
	  EVENT_SWP_USED, procMemInfo,
	  {0, 65535, 65535, 0},
     },{
	  N_("s free"), N_("swap memory free"),
	  EVENT_SWP_FREE, procMemInfo,
	  {0, 0, 65535, 0},
     },
};
