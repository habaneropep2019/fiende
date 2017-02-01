/*
** ===========================================================================
** $RCSfile: PixmapTest.C,v $
** $Revision: 1.6 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

/*
 * GtkPixmap
 */

#include "PixmapTest.h"

static char * pc_rcs_h = PixmapTest_h;
static char * pc_rcs = "$Id: PixmapTest.C,v 1.6 2000/03/07 14:21:41 kenelson Exp $";

#define USE(var) static void * use_##var = (void *) var
USE( pc_rcs_h);
USE( pc_rcs);

#define TRACE_LINE cout << __FILE__ << ":" << __LINE__ << endl;

PixmapTest * PixmapTest::theTest = 0;

TestFixture*
PixmapTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new PixmapTest ();
      return theTest;
  }
  return 0;
}

void PixmapTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


PixmapTest::PixmapTest () :
  vbox1 ( false, 0 ),
  vbox2 ( false, 10 ),
  hbox3 ( false, 0 ),
  vbox4 ( false, 10 ),
  pixmap ( 0 ),
  label ( "Pixmap\ntest" )
{
  set_title ( "Gtk::Pixmap" );
  set_border_width ( 0 );
  realize ();
  
  actionArea . pack_start ( vbox1 );
  vbox1 . pack_start ( vbox2 );

  vbox2 . pack_start ( button, false, false );
  
  pixmap = new Gtk::Pixmap ( "test.xpm" );

  hbox3 . set_border_width ( 2 );
  hbox3 . pack_start ( * manage ( static_cast < Gtk::Widget * > ( pixmap ) ) );
  hbox3 . pack_start ( label );
  
  button . add ( hbox3 );

  vbox1 . pack_start ( separator, false );

  packControlArea ();
  show_all ();
}

