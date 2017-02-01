/*
** ===========================================================================
** $RCSfile: ToggleButtonTest.C,v $
** $Revision: 1.4 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

/*
**
**  TestFixture 
**
*/


#include "ToggleButtonTest.h"

static char * pc_rcs_h = ToggleButtonTest_h;
static char * pc_rcs = "$Id: ToggleButtonTest.C,v 1.4 2000/03/07 14:21:41 kenelson Exp $";

#define USE(var) static void * use_##var = (void *) var
USE( pc_rcs_h);
USE( pc_rcs);

ToggleButtonTest * ToggleButtonTest::theTest = 0;

TestFixture * 
ToggleButtonTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new ToggleButtonTest ();
      return theTest;
    }
  return 0;
}

void ToggleButtonTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


ToggleButtonTest::ToggleButtonTest () :
  button1 ( "button 1" ),
  button2 ( "button 2" ),
  button3 ( "button 3" )
{
  set_title ( "Gtk::ToggleButton" );
  set_border_width ( 0 );
  actionArea . pack_start ( button1 );
  actionArea . pack_start ( button2 );
  actionArea . pack_start ( button3 );
  packControlArea ();
  show_all ();
}
