
#include "CheckButtonTest.h"

CheckButtonTest * CheckButtonTest::theTest = 0;

TestFixture*
CheckButtonTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new CheckButtonTest ();
      return theTest;
    }
  return 0;
}

void CheckButtonTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


CheckButtonTest::CheckButtonTest () :
  button1 ( "button1" ),
  button2 ( "button2" ),
  button3 ( "button3" )
{
  set_title ( "Gtk::CheckButton" );
  actionArea . pack_start ( button1, true, true, 0 );
  actionArea . pack_start ( button2, true, true, 0 );
  actionArea . pack_start ( button3, true, true, 0 );
  packControlArea ();
  show_all ();
}

