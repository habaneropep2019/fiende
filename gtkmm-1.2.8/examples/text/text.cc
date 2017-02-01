#include <stdio.h>
#include <gtk--/text.h>
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/checkbutton.h>
#include <gtk--/buttonbox.h>
#include <gtk--/table.h>
#include <gtk--/scrollbar.h>
#include <gtk--/separator.h>

class AppWindow : public Gtk::Window {
  Gtk::Text text;
  Gtk::CheckButton edit_check;
  Gtk::CheckButton wrap_check;
public:
  AppWindow();
  ~AppWindow();

  void text_toggle_editable () {
    text.set_editable(edit_check.get_active());
  }

  void text_toggle_word_wrap () {
    text.set_word_wrap(wrap_check.get_active());
  }

  gint delete_event_impl (GdkEventAny*) {
    Gtk::Main::quit();
    return 0;
  }
 

};

void close_application( GtkWidget *widget, gpointer data )
{
       gtk_main_quit();
}


AppWindow::AppWindow() :
  Gtk::Window(GTK_WINDOW_TOPLEVEL), edit_check("Editable"),
  wrap_check("Wrap Words")
{
  Gtk::VBox *box1;
  Gtk::VBox *box2;
  Gtk::HButtonBox *hbox;
  Gtk::Button *button;
  Gtk::HSeparator *separator;
  Gtk::Table *table;
  Gtk::VScrollbar *vscrollbar;

  FILE *infile;

 
  set_usize(600, 500);
  set_policy ( true, true, false);  
  set_title ("Text Widget Example");
  set_border_width (0);
  
  
  box1 = manage( new Gtk::VBox (false, 0) );
  add(*box1);
  
  box2 = manage( new Gtk::VBox (false, 10) );
  box2->set_border_width (10);
  box1->pack_start (*box2, true, true, 0);
  
  table = manage( new Gtk::Table (2, 2, false) );
  table->set_row_spacing (0, 2);
  table->set_col_spacing (0, 2);
  box2->pack_start (*table, true, true, 0);
  
  /* Create the GtkText widget */
  text.set_editable (true);
  table->attach (text, 0, 1, 0, 1,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);

  /* Add a vertical scrollbar to the GtkText widget */
  vscrollbar = manage( new Gtk::VScrollbar (*text.get_vadjustment()));
  table->attach ( *vscrollbar, 1, 2, 0, 1,
		    GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);

  /* Get the system colour map and allocate the colour red */
  Gdk_Color red("red");
  Gdk_Color white("white");
  Gdk_Color black("black");

  /* Load a fixed font */
  Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");

  /* Realizing a widget creates a window for it, ready for us to insert some text */
  text.realize ();

  /* Freeze the text widget, ready for multiple updates */
  text.freeze ();
  
  /* Insert some coloured text */
  text.insert ( Gdk_Font(), black, white, "Supports ", -1);
  text.insert ( Gdk_Font(), red,   white, "colored ", -1);
  text.insert ( Gdk_Font(), black, white, "text and different ", -1);
  text.insert ( fixed_font, black, red  , "fonts\n\n", -1);
  
  /* Load the file text.c into the text window */

  infile = fopen("text.cc", "r");
  
  if (infile) {
    char buffer[1024];
    int nchars;
    Gtk::Text::Context cx;
    cx.set_font(fixed_font); 
    while (1)
      {
	nchars = fread(buffer, 1, 1024, infile);
        buffer[nchars]='\0';
  	text.insert ( cx, buffer);
	
	if (nchars < 1024)
	  break;
      }
    
    fclose (infile);
  }

  /* Thaw the text widget, allowing the updates to become visible */  
  text.thaw ();
  
  hbox = manage( new Gtk::HButtonBox () );
  box2->pack_start ( *hbox, false, false, 0);

  hbox->pack_start ( edit_check, false, false, 0);
  edit_check.toggled.connect(slot(this, &AppWindow::text_toggle_editable));
  edit_check.set_active(true);

  hbox->pack_start (wrap_check, false, true, 0);
  wrap_check.toggled.connect(slot(this, &AppWindow::text_toggle_word_wrap));
  wrap_check.set_active(false);

  separator = manage( new Gtk::HSeparator () );
  box1->pack_start ( *separator, false, true, 0);

  box2 = manage( new Gtk::VBox (false, 10) );
  box2->set_border_width (10);
  box1->pack_start ( *box2, false, true, 0);
  
  button = manage( new Gtk::Button ("close") );
  button->clicked.connect( Gtk::Main::quit.slot() );
  box2->pack_start ( *button, true, true, 0);
  button->set_flags( GTK_CAN_DEFAULT);
  button->grab_default ();

  show_all ();
}

AppWindow::~AppWindow() {}

int main (int argc, char *argv[])
{
  Gtk::Main m(argc, argv);
  AppWindow app;

  Gtk::Main::run();
  
  return(0);       
}
