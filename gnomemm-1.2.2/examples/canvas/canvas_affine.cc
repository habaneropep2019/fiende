// CanvasAffine.cc
// Sample use of the Gnome::Art::Affine and Gnome::Art::Point classes
// (c) Agustin Ferrin Pozuelo <ferrin@arrakis.es>
#include <gtk--/window.h>
#include <gnome--/main.h>
#include <gnome--/canvas.h>
#include <gnome--/canvas-line.h>

static gint on_window_delete_event(GdkEventAny* event)
{
  Gnome::Main::quit();
  return FALSE;
}

Gnome::CanvasLine* some_lines(Gnome::CanvasGroup *group, gchar * color="black")
{
  using namespace Gnome::Art;
	
  // create some elements there
  Gnome::CanvasPoints points;
  points.push_back(Point(0, 0));
  points.push_back(Point(0, -10));
  points.push_back(Point(-5, -15));
  points.push_back(Point(0,-20));
  points.push_back(Point(5, -15));
  points.push_back(Point(0, -10));
	
  Gnome::CanvasLine* line = manage(new Gnome::CanvasLine(*group, points));
  *line << Gnome::CanvasHelpers::fill_color(color);
 
  return line;
}

int main(int argc, char* argv[])
{
  using SigC::bind;
  using SigC::slot;
  using namespace Gnome::Art;
	
  Gnome::Main gnomemain("CanvasAffine", "1.0", argc, argv);  
  
  Gtk::Window *window = manage(new Gtk::Window(GTK_WINDOW_TOPLEVEL));
  window->delete_event.connect(slot(&on_window_delete_event));
	
  Gnome::Canvas *canvas = manage(new Gnome::Canvas());
  window->set_default_size(600, 500);
  window->add(*canvas);
  window->show_all();

  // First, something nice
  for(gint i=0; i<12; i++)
  {
    Gnome::CanvasLine* line = some_lines(canvas->root(), "black");
    AffineTrans tx  =AffineTrans::scaling((10.0 + i) / 3)
										* AffineTrans::rotation(i * 30);
    line->affine_absolute(tx);
    line->move(-100, -100);
  }
	
  // Next, let's see Affine are not conmutative
  for(gint i=0; i<12; i++)
  {
    Gnome::CanvasLine* line = some_lines(canvas->root(), "blue");
		
    // Translating then rotating then scaling...
    AffineTrans tx = AffineTrans::translation(5, 0)
      * AffineTrans::rotation(i * 30)
      * AffineTrans::scaling((10.0 +i ) / 3);

    line->affine_absolute(tx);
    line->move(-100, 100);
  }
	
  for(gint i=0; i<12; i++)
  {
    Gnome::CanvasLine* line = some_lines(canvas->root(), "red");
		
    // Isn't the same as scaling then translating then rotating!
    AffineTrans tx = AffineTrans::scaling((10.0 + i) / 3)
      * AffineTrans::translation(5, 0)
      * AffineTrans::rotation(i * 30);

    line->affine_absolute(tx);
    line->move(100, 100);
  }

  gnomemain.run();
  return 0;
}


