/* example-start buttonbox buttonbox.c */

#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gtk--/frame.h>
#include <gtk--/button.h>
#include <gtk--/window.h>
#include <gtk--/main.h>

class BBox : public Gtk::Frame
{
public:
  BBox                 (gint horizontal,
                        const Gtk::string& title,
                        gint  spacing,
                        gint  child_w,
                        gint  child_h,
                        GtkButtonBoxStyle  layout);
};

BBox::BBox             (gint horizontal,
                        const Gtk::string& title,
                        gint  spacing,
                        gint  child_w,
                        gint  child_h,
                        GtkButtonBoxStyle  layout)
  : Gtk::Frame(title)
{
  Gtk::ButtonBox *bbox;
  Gtk::Button *button;

  if (horizontal)
    bbox = manage( new Gtk::HButtonBox () );
  else
    bbox = manage( new Gtk::VButtonBox () );

  bbox->set_border_width (5);

  add (*bbox);

  /* Set the appearance of the Button Box */
  bbox->set_layout (layout);
  bbox->set_spacing (spacing);
  bbox->set_child_size (child_w, child_h);

  button = manage( new Gtk::Button ("OK"));
  bbox->add (*button);

  button = manage( new Gtk::Button ("Cancel"));
  bbox->add (*button);

  button = manage( new Gtk::Button ("Help"));
  bbox->add (*button);

}

class AppWindow : public Gtk::Window
{
public:
  AppWindow ();
  ~AppWindow ();

  virtual gint delete_event_impl (GdkEventAny*);
};

AppWindow::AppWindow()
  : Gtk::Window(GTK_WINDOW_TOPLEVEL)
{
  Gtk::Box *main_vbox;
  Gtk::Box *vbox;
  Gtk::Box *hbox;
  Gtk::Frame *frame_horz;
  Gtk::Frame *frame_vert;

  set_title ("Button Boxes");
  set_border_width (10);

  main_vbox = manage( new Gtk::VBox(false, 0) );
  add (*main_vbox);

  frame_horz = manage( new Gtk::Frame ("Horizontal Button Boxes") );
  main_vbox->pack_start (*frame_horz, true, true, 10);

  vbox = manage( new Gtk::VBox (false, 0) );
  vbox -> set_border_width (10);
  frame_horz-> add (*vbox);

  vbox->pack_start (*manage( new BBox (true, "Spread (spacing 40)", 
                                      40, 85, 20, GTK_BUTTONBOX_SPREAD)),
		      true, true, 0);

  vbox->pack_start (*manage( new BBox (true, "Edge (spacing 30)", 
	                              30, 85, 20, GTK_BUTTONBOX_EDGE)),
		      true, true, 5);

  vbox->pack_start (*manage( new BBox (true, "Start (spacing 20)", 
                                      20, 85, 20, GTK_BUTTONBOX_START)),
		      true, true, 5);

  vbox->pack_start (*manage( new BBox (true, "end (spacing 10)", 
                                      10, 85, 20, GTK_BUTTONBOX_END)),
		      true, true, 5);

  frame_vert = manage( new Gtk::Frame ("Vertical Button Boxes") );
  main_vbox->pack_start (*frame_vert, true, true, 10);

  hbox = manage( new Gtk::HBox (false, 0) );
  hbox->set_border_width (10);
  frame_vert->add (*hbox);

  hbox->pack_start (*manage( new BBox (false, "Spread (spacing 5)", 
                                      5, 85, 20, GTK_BUTTONBOX_SPREAD)),
		      true, true, 0);

  hbox->pack_start (*manage( new BBox (false, "Edge (spacing 30)", 
	                              30, 85, 20, GTK_BUTTONBOX_EDGE)),
		      true, true, 5);

  hbox->pack_start (*manage( new BBox (false, "Start (spacing 20)", 
                                      20, 85, 20, GTK_BUTTONBOX_START)),
		      true, true, 5);

  hbox->pack_start (*manage( new BBox (false, "End (spacing 10)", 
                                      10, 85, 20, GTK_BUTTONBOX_END)),
		      true, true, 5);

  show_all ();

}

AppWindow::~AppWindow() {}

gint AppWindow::delete_event_impl (GdkEventAny*)
{
  Gtk::Main::quit();
  return 0;
}

int main( int   argc,
          char *argv[] )
{
  Gtk::Main main (argc,argv);
  AppWindow window;

  /* Enter the event loop */
  Gtk::Main::run ();
    
  return(0);
}
/* example-end */
