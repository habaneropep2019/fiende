#include "ButtonBoxTest.h"

ButtonBoxFrame::ButtonBoxFrame( bool horizontal, string title, 
				gint spacing, gint child_w, 
				gint child_h, GtkButtonBoxStyle layout ) :
  Gtk::Frame ( title ),
  bbox ( horizontal ? static_cast < Gtk::ButtonBox * > ( new Gtk::HButtonBox () )
                    : static_cast < Gtk::ButtonBox * > ( new Gtk::VButtonBox () ) ),
  okButton ( "OK" ),
  cancelButton ( "Cancel" ),
  helpButton ( "Help" )
{
  add ( *bbox );
  bbox -> set_border_width ( 5 );
  bbox -> set_layout ( layout );
  bbox -> set_spacing ( spacing );
  bbox -> set_child_size ( child_w, child_h );
  bbox -> add ( okButton );
  bbox -> add ( cancelButton );
  bbox -> add ( helpButton );
}

ButtonBoxTest * ButtonBoxTest::theTest = 0;

TestFixture * 
ButtonBoxTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new ButtonBoxTest ();
      return theTest;
    }
  return 0;
}

void ButtonBoxTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}

ButtonBoxTest::ButtonBoxTest () :
  hFrame ( "Horizontal Button Boxes" ),
  hframeVBox ( false, 0 ),
  vFrame ( "Vertical Button Boxes" ),
  vframeHBox ( false, 0 )
{
  ButtonBoxFrame * bbf;
  set_title ( "Button Box Test" );
  actionArea . pack_start ( hFrame, true, true, 10 );
  hFrame . add ( hframeVBox );

  bbf = new ButtonBoxFrame  ( true, "Spread", 40, 85, 20, GTK_BUTTONBOX_SPREAD );
  // widgets packed or added by pointer will be converted to an object handle
  // and will be deleted when the container is deleted. To have the widget persist
  // past the container ( probably not a good idea ) add the pointer by dereferencing.
  // for example here, replace bff with *bff.
  hframeVBox . pack_start ( *manage ( bbf ), true, true, 0 );

  bbf = new ButtonBoxFrame  ( true, "Edge", 40, 85, 20, GTK_BUTTONBOX_EDGE );
  hframeVBox . pack_start ( *manage ( bbf ), true, true, 5 );

  bbf = new ButtonBoxFrame  ( true, "Start", 40, 85, 20, GTK_BUTTONBOX_START );
  hframeVBox . pack_start ( *manage ( bbf ), true, true, 5 );

  bbf = new ButtonBoxFrame  ( true, "End", 40, 85, 20, GTK_BUTTONBOX_END );
  hframeVBox . pack_start ( *manage ( bbf ), true, true, 5 );

  actionArea . pack_start ( vFrame, true, true, 10 );
  vFrame . add ( vframeHBox );

  bbf = new ButtonBoxFrame ( false, "Spread", 30, 85, 20, GTK_BUTTONBOX_SPREAD );
  vframeHBox . pack_start ( * manage ( bbf ), true, true, 0 );
  
  bbf = new ButtonBoxFrame ( false, "Edge", 30, 85, 20, GTK_BUTTONBOX_EDGE );
  vframeHBox . pack_start ( * manage ( bbf ), true, true, 5 );

  bbf = new ButtonBoxFrame ( false, "Start", 30, 85, 20, GTK_BUTTONBOX_START );
  vframeHBox . pack_start ( * manage ( bbf ), true, true, 5 );

  bbf = new ButtonBoxFrame ( false, "End", 30, 85, 20, GTK_BUTTONBOX_END );
  vframeHBox . pack_start ( * manage ( bbf ), true, true, 5 );

  packControlArea ();
  show_all ();
}


