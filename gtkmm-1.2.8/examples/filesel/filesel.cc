/* example-start filesel filesel.cc */
#include <iostream>
#include <gtk--/fileselection.h>
#include <gtk--/main.h>

//From <iostream> 
using std::cout;
using std::endl;

using SigC::slot;

class filesel: public Gtk::FileSelection
{
public:
    filesel();
private:
    /* Get the selected filename and print it to the console */
    void file_ok_sel() {
	cout << "file_ok_sel: " << get_filename() << endl;
    }
    gint delete_event_impl(GdkEventAny*) { 
	Gtk::Main::quit(); return 0; 
    }
};

filesel::filesel():
    Gtk::FileSelection("File selection")
{
    /* Connect the ok_button_ to file_ok_sel function */
    get_ok_button()->clicked.connect(slot(this, &filesel::file_ok_sel));
    /* Connect the cancel_button_ to hiding the window */
    get_cancel_button()->clicked.connect(hide.slot());
    /* Connect hiding the window to exit the program */
    hide.connect(Gtk::Main::quit.slot());
}


int main (int argc, char *argv[])
{
    /* Initialize GTK-- */
    Gtk::Main m(&argc, &argv);
    
    /* Create a new file selection widget */
    filesel filew;
    
    /* Lets set the filename, as if this were a save dialog, and we are giving
       a default filename */
    filew.set_filename("penguin.png");

    filew.show();
    m.run();

    return 0;
}
/* example-end */
