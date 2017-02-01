#include <gtk--/main.h>
#include <gtk--/window.h>

int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);

    Gtk::Window window (GTK_WINDOW_TOPLEVEL);
    window.show();

    kit.run();  // you will need to ^C to exit.
    
    return(0);
}
