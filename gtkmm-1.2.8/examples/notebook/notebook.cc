#include <stdio.h>
#include <gtk--/window.h>
#include <gtk--/checkbutton.h>
#include <gtk--/frame.h>
#include <gtk--/main.h>
#include <gtk--/notebook.h>
#include <gtk--/table.h>

struct Book : public Gtk::Notebook
  {
    void rotate_book ();
    void tabsborder_book ();
    void remove_book ();
  };

/* This function rotates the position of the tabs */
void Book::rotate_book ()
  {
    gint pos=(gint)(get_tab_pos());
    ++pos;
    set_tab_pos ((GtkPositionType)(pos%4));
  }

/* Add/Remove the page tabs and the borders */
void Book::tabsborder_book ()
  {
    set_show_tabs (!get_show_tabs());
    set_show_border (!get_show_border());
  }

/* Remove a page from the notebook */
void Book::remove_book ()
{
    Gtk::Notebook::Page *page;
    
    page = get_current();
    pages().remove(page);
    /* Need to refresh the widget -- 
     This forces the widget to redraw itself. */
    draw(NULL);
}


struct AppWindow: public Gtk::Window
  {
    AppWindow();
    ~AppWindow();

    gint delete_event_impl (GdkEventAny*) 
      {
        Gtk::Main::quit(); 
        return 0;
      }
  };


AppWindow::AppWindow()
    : Gtk::Window(GTK_WINDOW_TOPLEVEL)
  {

    Gtk::Button      *button;
    Gtk::Table       *table;
    Book            *notebook;
    Gtk::CheckButton *checkbutton;

    int i;
    char bufferf[32];
    char bufferl[32];
    
    set_border_width (10);

    table = manage( new Gtk::Table(3,6,false) );
    add (*table);
    
    /* Create a new notebook, place the position of the tabs */
    notebook = manage( new Book() );
    notebook->set_tab_pos (GTK_POS_TOP);
    table->attach (*notebook, 0,6,0,1);
    
    /* Lets append a bunch of pages to the notebook */
    for (i=0; i < 5; i++) 
      {
        Gtk::Frame *frame;
	sprintf(bufferf, "Append Frame %d", i+1);
	sprintf(bufferl, "Page %d", i+1);
	
	frame = manage( new Gtk::Frame (bufferf) );
	frame->set_border_width (10);
	frame->set_usize (100, 75);
	frame->show ();
	
	notebook->pages().push_back(
          Gtk::Notebook_Helpers::TabElem(*frame,bufferl));
      }
      
    /* Now lets add a page to a specific spot */
    checkbutton = manage( new Gtk::CheckButton ("Check me please!") );
    checkbutton->set_usize (100, 75);
    checkbutton->show ();
   
    notebook->pages().insert(++(notebook->pages().begin()),
                   Gtk::Notebook_Helpers::TabElem (*checkbutton,
                                                  "Check me please!")
                  );
    
    /* Now finally lets prepend pages to the notebook */
    for (i=0; i < 5; i++) 
      {
        Gtk::Frame *frame;
	sprintf(bufferf, "Prepend Frame %d", i+1);
	sprintf(bufferl, "PPage %d", i+1);

        frame = manage( new Gtk::Frame (bufferf) );
        frame->set_border_width (10);
        frame->set_usize (100, 75);
        frame->show ();

        notebook->pages().push_front(
          Gtk::Notebook_Helpers::TabElem(*frame,bufferl));
      }
    
    /* Set what page to start at (page 4) */
    notebook->set_page (3);

    /* Create a bunch of buttons */
    button = manage( new Gtk::Button("close") );
    button->clicked.connect(Gtk::Main::quit.slot());
    table->attach(*button, 0,1,1,2);
    
    button = manage( new Gtk::Button("next page") );
    button->clicked.connect(slot(notebook,&Gtk::Notebook::next_page));
    table->attach(*button, 1,2,1,2);
    
    button = manage( new Gtk::Button("prev page") );
    button->clicked.connect(slot(notebook,&Gtk::Notebook::prev_page));
    table->attach(*button, 2,3,1,2);
    
    button = manage( new Gtk::Button("tab position") );
    button->clicked.connect(slot(notebook,&Book::rotate_book));
    table->attach(*button, 3,4,1,2);
    
    button = manage( new Gtk::Button("tabs/border on/off") );
    button->clicked.connect(slot(notebook,&Book::tabsborder_book));
    table->attach(*button, 4,5,1,2);
    
    button = manage( new Gtk::Button("remove page") );
    button->clicked.connect(slot(notebook,&Book::remove_book));
    table->attach(*button, 5,6,1,2);
    
    show_all();
  }

AppWindow::~AppWindow() {}
    
int main (int argc, char *argv[])
  {
    Gtk::Main m(&argc, &argv);
    AppWindow app;

    Gtk::Main::run();
    
    return(0);
  }
