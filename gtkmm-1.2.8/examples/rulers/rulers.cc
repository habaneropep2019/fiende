#include <gtk--/main.h>
#include <gtk--/table.h>
#include <gtk--/window.h>
#include <gtk--/ruler.h>
#include <gtk--/drawingarea.h>

// Gtk-- version of the "rulers" example from the gtk+ tutorial

class Rulers : public Gtk::Window
{
  Gtk::Table m_table;
  Gtk::DrawingArea m_area;
  Gtk::HRuler m_hrule;
  Gtk::VRuler m_vrule;
  
  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }
  static const int XSIZE = 600,
    YSIZE = 400;
  
public:
  Rulers();
  
};

Rulers::Rulers() :
  m_table(3, 2, false)
{
  set_border_width(10);
  add(m_table);
  
  m_area.size(XSIZE, YSIZE);
  
  m_table.attach(m_area, 1,2,1,2,
		 GtkAttachOptions(GTK_EXPAND|GTK_FILL), GTK_FILL, 0, 0);
  
  m_area.set_events(GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK );
 
  // The horizontal ruler goes on top.  As the mouse moves across the
  // drawing area, a motion_notify_event is passed to the appropriate
  // event handler for the ruler.

  m_hrule.set_metric(GTK_PIXELS);
  m_hrule.set_range(7, 13, 0, 20 );
  m_area.motion_notify_event.connect(m_hrule.motion_notify_event.slot());
  m_table.attach(m_hrule, 1,2,0,1,
		 GTK_EXPAND|GTK_SHRINK|GTK_FILL, GTK_FILL,
		 0, 0);

  m_vrule.set_metric(GTK_PIXELS);
  m_vrule.set_range(0, YSIZE, 10, YSIZE );
  m_area.motion_notify_event.connect(m_vrule.motion_notify_event.slot());
  m_table.attach(m_vrule, 0, 1, 1, 2,
		 GTK_FILL, GTK_EXPAND|GTK_SHRINK|GTK_FILL, 0, 0 );

  show_all();
  
}

int main (int argc, char *argv[])
{
  Gtk::Main myapp(argc, argv);

  Rulers rulers;

  myapp.run();
  return 0;
}

