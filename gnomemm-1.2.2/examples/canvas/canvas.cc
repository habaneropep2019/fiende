// This example was started by Guillaume Laurent.
// It has become a place to dump code that tests parts of the
// Gnome-- canvas code. Little thought has been given to the
// actual on-screen output.
// TODO: Would someone like to build a more sensible canvas example?

#include <gnome--.h> // don't do that : here it is just for simplicity's
                     // sake, but this drags more than a meg worth of headers


//CanvasExample:

class CanvasExample : public Gnome::Canvas
{
public:
  CanvasExample();
  ~CanvasExample();

protected:
  Gnome::CanvasGroup m_canvasgroup;
  Gnome::CanvasLine *m_line;
  Gnome::CanvasEllipse *m_ellipse;
  Gnome::CanvasRect *m_rect;
  Gnome::CanvasImage *m_image;
  Gnome::CanvasText *m_text;
};

CanvasExample::CanvasExample()
  : m_canvasgroup(*(root()), 0, 0)
{
  // create some elements there
  Gnome::CanvasPoints m_points;

  m_points.push_back(Gnome::Art::Point(0, 0));
  m_points.push_back(Gnome::Art::Point(100, 0));
  m_points.push_back(Gnome::Art::Point(0, 100));
  m_points.push_back(Gnome::Art::Point(100, 100));

  // we want to use the stream like interface
  using namespace Gnome;

  m_line = new Gnome::CanvasLine(m_canvasgroup,m_points);
  *m_line << CanvasHelpers::fill_color("red")
          << CanvasHelpers::width_units(4.0)
          << CanvasHelpers::cap_style(GDK_CAP_ROUND);

  m_ellipse = new Gnome::CanvasEllipse(m_canvasgroup, 0, 0, 100, 100);
  *m_ellipse << CanvasHelpers::fill_color("blue");

  m_rect = new Gnome::CanvasRect(m_canvasgroup, 10, 10, 50, 100);
  *m_rect << CanvasHelpers::width_pixels(2)
          << CanvasHelpers::fill_color("white");

  m_image = new Gnome::CanvasImage(m_canvasgroup, 0, 0, Gdk_Imlib::Image("example.png"));
  //The width and height are set from the information in the image file.

  m_text = new Gnome::CanvasText(m_canvasgroup, 10, 10, "Some Text");
  *m_text << CanvasHelpers::font("-Adobe-Helvetica-Medium-R-Normal--*-100-*-*-*-*-*-*")
          << CanvasHelpers::fill_color("blue"); //Changes the color of the text.
}

CanvasExample::~CanvasExample()
{
  delete m_line;
  delete m_ellipse;
  delete m_rect;
  delete m_image;
  delete m_text;
}


//MainWin:

class MainWin : public Gnome::Main
{
public:
  MainWin(const gchar * appname, const gchar * title,
	  const char *app_id, const char *app_version,
	  int argc, char **argv);
	
protected:

  //override:
  gint on_app_delete_event(GdkEventAny*);

  //Member widgets:
  CanvasExample m_canvas;
  Gnome::App m_app;
};

MainWin::MainWin(const gchar * appname, const gchar * title,
		 const char *app_id, const char *app_version,
		 int argc, char **argv)
  : Gnome::Main(app_id, app_version, argc, argv),
    m_app(appname, title)
{
  m_app.set_contents(m_canvas);

  //Connect signals:
  m_app.delete_event.connect(slot(this, &MainWin::on_app_delete_event));
  
  m_app.show_all();
}

gint MainWin::on_app_delete_event(GdkEventAny*)
{ 
  Gtk::Main::quit();
  return FALSE; 
}


//main():

int main(int argc, char *argv[])
{
  MainWin mainwin("Gnome::Canvas Example", "Gnome::Canvas Example", "GnomeCanvas", "0.0",
		  argc, argv);
  
  mainwin.run();
  return 0;
}

