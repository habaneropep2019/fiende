/*
** ===========================================================================
** $RCSfile: ButtonTest.C,v $
** $Revision: 1.6 $
** $Date: 2000/05/14 18:48:17 $
** $Author: kenelson $
** ===========================================================================
*/

#include <strstream>

#include "ButtonTest.h"

static char * pc_rcs_h = ButtonTest_h;
static char * pc_rcs = "$Id: ButtonTest.C,v 1.6 2000/05/14 18:48:17 kenelson Exp $";

#define USE(var) static void * use_##var = (void *) var
USE( pc_rcs_h);
USE( pc_rcs);

ButtonTest * ButtonTest::theTest = 0;

TestFixture*
ButtonTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new ButtonTest ();
      return theTest;
  }
  return 0;
}

void ButtonTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}

    
ButtonTest::ButtonTest () :
  box2 ( false, 10 ),
  table ( 3 /* rows */, 3 /* columns */ , false /* homogeneous (false by default) */ )
{
  set_title ( "Gtk::Button" );
  set_border_width ( 0 );
  //  add(box1);
  table . set_row_spacings ( 5 );
  table . set_col_spacings ( 5 );
  table . set_border_width ( 10 );
  actionArea . pack_start ( table );
  for ( int row = 0; row < 3; row++ ) 
    {
      for ( int column = 0; column < 3; column++ ) 
	{
	  int i = row * 3 + column;
	  strstream ss;

	  ss << "button" << i << ends;
	  button [ i ] = new Gtk::Button ( ss . str () );
	  /* toggleShowButton is renamed from button_window in gtk+'s testgtk */
	  button[i]->clicked.connect(bind(slot(this,&(ButtonTest::toggleShowButton)),(i+1)%8));

	  table . attach ( * button [ i ], column, column + 1, row, row + 1 );
	}
    }
  packControlArea ();
  show_all ();
}

void
ButtonTest::toggleShowButton ( int buttonIndex ) 
{

  if ( button [ buttonIndex ] ) 
    if ( button [ buttonIndex ] -> is_visible () ) 
      button [ buttonIndex ] -> hide ();
    else 
      button [ buttonIndex ] -> show ();

}
