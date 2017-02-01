// CanvasEvents.cc
// This program tries to sample the event handling inside the canvas
// (c) Agustin Ferrin Pozuelo <ferrin@arrakis.es>
// Note that you have to click 2^N times over the canvas,
// It took me 5.60 seconds to have 64 valid events! I'm Termclickator
#include <gtk--/window.h>
#include <gnome--/main.h>
#include <gnome--/canvas.h>
#include <gnome--/canvas-rect.h>


guint32 etime=0;
guint	status=0;
enum {CE=0,CEA=1,CBE=2,IE=3,IEA=4,CBEA=5,N=6};
enum {ENDING_STATUS=(1<<N)};
gchar record[N+1],stmask[N+1];
gchar* cname[N]={"canvas_event","canvas_event_after"
		,"canvas_button_press_event"
		,"item_button_press_event","item_button_press_event_after"
		,"canvas_button_press_event_after"};

static gint event_explorer(GdkEvent* e, guint caller)
{
  GdkEventButton* b;
  // Ignore all but button-press events:
  if(e->type != GDK_BUTTON_PRESS)
  return TRUE;
	  
  b = &(e->button);
  if(b->time != etime) // if new event
  {
    gint i;
    if(etime != 0) // if not first, inc status and show last record
    {
      for(i=0;i<N;i++)stmask[i]= (status & (1<<i)) ? '1': '0';
      stmask[N]=0;
      printf("  Event mask / Events reached %s/%s\n",stmask,record);
      if((++status)>=ENDING_STATUS) exit(0);
    }
		
    for(i=0;i<N;i++)record[i]='0'; record[N]=0;
    etime=b->time;
    printf("New event (%u) at time %X\n", status, (guint)etime);
  }
	
  record[caller]='1';
  printf("  %-30s() is returning ", cname[caller]);
	
  if(status & (1 << caller))
  {
    printf("TRUE\n");
    return TRUE;
  }
  else
  {
    printf("FALSE\n");
    return FALSE;
  }
}

static gint button_event_stub(GdkEventButton *b, guint caller)
{
  return event_explorer((GdkEvent *) b, caller);
}

static gint on_window_delete_event(GdkEventAny* event)
{
  Gnome::Main::quit();
  return FALSE;
}

int main(int argc, char* argv[])
{
  using SigC::bind;
  using SigC::slot;
	
  Gnome::Main gnomemain("CanvasEvents", "1.0", argc, argv);  
  
  Gtk::Window *window = manage(new Gtk::Window(GTK_WINDOW_TOPLEVEL));
  window->delete_event.connect(slot(&on_window_delete_event));
	
  Gnome::Canvas *canvas = manage(new Gnome::Canvas());

  Gnome::CanvasRect* item = manage(new Gnome::CanvasRect(
    *canvas->root(),
    (gdouble) -1000, (gdouble) -1000,
    (gdouble) 1000, (gdouble) 1000) );
  *item << Gnome::CanvasHelpers::fill_color("black");

  //Connect canvas signals:
  canvas->event.connect(bind(slot(&event_explorer),CE));
  canvas->event.connect_after(bind(slot(&event_explorer),CEA));
  canvas->button_press_event.connect(bind(slot(&button_event_stub),CBE));
  canvas->button_press_event.connect_after(bind(slot(&button_event_stub),CBEA));
	
  item->event.connect(bind(slot(&event_explorer),IE));
  item->event.connect_after(bind(slot(&event_explorer),IEA));
	
  window->add(*canvas);
  window->show_all();

  gnomemain.run();
  return 0;
}

