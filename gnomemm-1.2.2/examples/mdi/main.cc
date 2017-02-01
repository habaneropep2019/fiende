#include <gtk/gtk.h>
#include <gtk--.h>
#include <gnome--.h>

#include "mdi-window.h"

int main (int argc, char* argv[])
{
  MDIWindow *MDI_WINDOW;
  Gnome::Main application("mdi-test", "0.0", argc, argv);

  MDI_WINDOW = new MDIWindow();

  MDI_WINDOW->open_toplevel();

  application.run();

  return 0;
}
