#include "RadioButtonTest.h"

RadioButtonTest * RadioButtonTest::theTest = 0;

TestFixture * 
RadioButtonTest::create ()
{
  if ( theTest == 0 ) 
    {
      theTest = new RadioButtonTest ();
      return theTest;
    }
  return 0;
}

void RadioButtonTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


RadioButtonTest::RadioButtonTest () :
  button1 ( group, "button1" ),
  button2 ( group, "button2" ),
  button3 ( group, "button3" )
{
  set_title ( "Radio Button Test" );
  actionArea . pack_start ( button1, false, true, 0 );
  actionArea . pack_start ( button2, false, true, 0 );
  actionArea . pack_start ( button3, false, true, 0 );
  packControlArea ();
  show_all ();
}
