#include <gtk--/main.h>
#include <gtk--/window.h>
#include "tictactoe.h"

void
win (TicTacToe *ttt)
{
  g_print ("Yay!\n");
  ttt-> clear();
}

int 
main (int argc, char *argv[])
{
  Gtk::Window *window;
  TicTacToe *ttt;
  
  Gtk::Main m(argc, argv);

  ttt = manage( new TicTacToe () );
  ttt-> tictactoe.connect ( bind (slot (&win), ttt) );

  window = manage( new Gtk::Window () );
  window-> set_title ("Tic-Tac-Toe");
  window-> destroy.connect ( Gtk::Main::quit.slot() );
  window-> set_border_width (10);
  window-> add (*ttt);
  window-> show_all ();
  
  Gtk::Main::run ();
  
  return 0;
}
