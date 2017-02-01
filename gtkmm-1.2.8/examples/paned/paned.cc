#include <gtk--/scrolledwindow.h>
#include <gtk--/table.h>
#include <gtk--/list.h>
#include <gtk--/main.h>
#include <gtk--/scrollbar.h>
#include <gtk--/window.h>
#include <gtk--/text.h>
#include <gtk--/paned.h>
  
class AppMessages: public Gtk::ScrolledWindow 
{
public:
  AppMessages();
};

/* Create the list of "messages" */
AppMessages::AppMessages()
  : Gtk::ScrolledWindow()
{
    Gtk::List *list;
   
    /* Create a new scrolled window, with scrollbars only if needed */
    set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   
    /* Create a new list and put it in the scrolled window */
    list = manage(new Gtk::List());
    add_with_viewport (*list);

    /* Add some "messages" to the window */
   
    for (int i=0; i<10; i++)
    	list->items().push_back(*manage(new Gtk::ListItem("message")));
    list->show ();
}

class AppText : public Gtk::Table
{
  Gtk::Text* text;
public:
  AppText();

  virtual void realize_impl();
};
   
/* Create a scrolled text area that displays a "message" */
AppText::AppText()
  : Gtk::Table(2,2,false)
{
    Gtk::Scrollbar *scrollbar;
   
    /* Put a text widget in the upper left hand corner. Note the use of
     * GTK_SHRINK in the y direction */
    text = manage( new Gtk::Text () );
    attach (*text, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND,
		      GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
    text->show ();
   
    /* Put a HScrollbar in the lower left hand corner */
    scrollbar = manage( new Gtk::HScrollbar (*(text->get_hadjustment())) );
    attach (*scrollbar, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
    scrollbar->show ();
   
    /* And a VScrollbar in the upper right */
    scrollbar = manage( new Gtk::VScrollbar (*(text->get_vadjustment())) );
    attach (*scrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
    scrollbar->show ();
}

/* Add some text to our text widget - this is a callback that is invoked
when our window is realized. We could also force our window to be
realized with gtk_widget_realize, but it would have to be part of
a hierarchy first */

void AppText::realize_impl ()
{
    Gtk::Widget::realize_impl(); // always call this!
    text->freeze ();
    text->insert(Gtk::Text_Helpers::Context(),
    "From: pathfinder@nasa.gov\n"
    "To: mom@nasa.gov\n"
    "Subject: Made it!\n"
    "\n"
    "We just got in this morning. The weather has been\n"
    "great - clear but cold, and there are lots of fun sights.\n"
    "Sojourner says hi. See you soon.\n"
    " -Path\n");
   
    text->thaw ();
}

class AppWindow : Gtk::Window
{
public:
  AppWindow();
  ~AppWindow();

  /* It's a good idea to do this for all application windows. */
  gint delete_event_impl (GdkEventAny*)
  {
    Gtk::Main::quit();
    return 0;
  }

};

   
AppWindow::AppWindow()
    : Gtk::Window (GTK_WINDOW_TOPLEVEL)
{
    Gtk::Paned *vpaned;
    Gtk::Widget *list;
    Gtk::Widget *text;

    set_title ("Paned Windows");
    set_border_width (10);
    set_usize (450, 400);

    /* create a vpaned widget and add it to our toplevel window */
   
    vpaned = manage( new Gtk::VPaned () );
    add (*vpaned);
    vpaned->set_handle_size (10);
    vpaned->set_gutter_size (15);                       
   
    /* Now create the contents of the two halves of the window */
    list = manage( new AppMessages() );
    vpaned->add1 (*list);
   
    text = manage( new AppText() );
    vpaned->add2 (*text);

    show_all ();
}

AppWindow::~AppWindow() {}


int main( int   argc,
          char *argv[] )
{
  /* Initialise GTK */
  Gtk::Main kit(&argc, &argv);

  /* Create a new window */
  AppWindow app;

  /* Enter the event loop */
  Gtk::Main::run ();

  return(0);
}

