/* example-start scrolledwin scrolledwin.c */

#include <stdio.h>
#include <gtk--/table.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/togglebutton.h>
#include <gtk--/main.h>

class Scrolledwin : public Gtk::Dialog
{
public:
	Scrolledwin();
	~Scrolledwin();
};

Scrolledwin::~Scrolledwin() {};

Scrolledwin::Scrolledwin() {
    char buffer[32];
    int i, j;
    Gtk::ScrolledWindow *scrolled_window;
    Gtk::Table *table;
    Gtk::Button *button;
    
    /* Create a new dialog window for the scrolled window to be
     * packed into. A dialog is just like a normal window except it has a 
     * vbox and a horizontal separator packed into it. It's just a shortcut
     * for creating dialogs */
    destroy.connect(Gtk::Main::quit.slot());

    set_title ("GtkScrolledWindow example");
    set_border_width (0);
    set_usize(300, 300);
    
    /* create a new scrolled window. */
    scrolled_window = manage(new Gtk::ScrolledWindow());
    
    scrolled_window->set_border_width (10);
    
    /* the policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS.
     * GTK_POLICY_AUTOMATIC will automatically decide whether you need
     * scrollbars, whereas GTK_POLICY_ALWAYS will always leave the scrollbars
     * there.  The first one is the horizontal scrollbar, the second, 
     * the vertical. */
    scrolled_window->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    /* The dialog window is created with a vbox packed into it. */
    get_vbox()->pack_start (*scrolled_window);
    scrolled_window->show();
    
    /* create a table of 10 by 10 squares. */
    table = manage(new Gtk::Table(10, 10, false));
    
    /* set the spacing to 10 on x and 10 on y */
    table->set_row_spacings (10);
    table->set_col_spacings (10);
    
    /* pack the table into the scrolled window */
    scrolled_window->add_with_viewport (*table);
    table->show();
    
    /* this simply creates a grid of toggle buttons on the table
     * to demonstrate the scrolled window. */
    for (i = 0; i < 10; i++)
       for (j = 0; j < 10; j++) {
          sprintf (buffer, "button (%d,%d)\n", i, j);
	  button = manage(new Gtk::ToggleButton(buffer));
	  table->attach(*button, i, i+1, j, j+1);
          button->show();
       }
    
    /* Add a "close" button to the bottom of the dialog */
    button = manage(new Gtk::Button("close"));
    button->clicked.connect(destroy.slot());
    
    /* this makes it so the button is the default. */
    button->set_flags(GTK_CAN_DEFAULT);
    get_action_area()->pack_start (*button);
    
    /* This grabs this button to be the default button. Simply hitting
     * the "Enter" key will cause this button to activate. */
    button->grab_default();
    button->show ();
    
    show();
}

int main (int argc, char *argv[])
{
    Gtk::Main myapp(argc, argv);
    Scrolledwin scrolledwin;
    myapp.run();
    return(0);
}
/* example-end */
