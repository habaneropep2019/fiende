#include <strstream>

#include "StatusbarTest.h"

StatusBarTest * StatusBarTest::theTest = 0;

StatusBarTest::StatusBarTest () :
  pushSomethingButton ( "push something" ),
  popButton ( "pop" ),
  stealButton ( "steal #4" ),
  dumpStackButton ( "dump stack" ),
  contextButton ( "contexts" ),
  statusbarCounter ( 1 )
{
  set_title ( "statusbar" );
  set_border_width ( 0 );

  actionArea . pack_end ( gtkStatusBar );
  gtkStatusBar.text_popped.connect(slot(this,&StatusBarTest::popped));

  actionArea . pack_end ( pushSomethingButton );
  pushSomethingButton.clicked.connect(slot(this,&StatusBarTest::push));

  actionArea . pack_end ( popButton );
  popButton.clicked.connect(slot(this,&StatusBarTest::pop));

  actionArea . pack_end ( stealButton );
  stealButton.clicked.connect(slot(this,&StatusBarTest::steal));

  actionArea . pack_end ( dumpStackButton );
  dumpStackButton.clicked.connect(slot(this,&StatusBarTest::dump_stack));

  actionArea . pack_end ( contextButton );
  contextButton.clicked.connect(slot(this,&StatusBarTest::contexts));

  packControlArea ();
  show_all ();
}

TestFixture* 
StatusBarTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new StatusBarTest ();
      return theTest;
    }
  return 0;
}

void StatusBarTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


void
StatusBarTest::push () 
{
  strstream text;
  text << "something ";
  text << statusbarCounter++ << ends;
  gtkStatusBar.push ( 1, text . str () );
}

void
StatusBarTest::pop () 
{
  gtkStatusBar . pop ( 1 );
}

void
StatusBarTest::steal () 
{
  gtkStatusBar . remove_message ( 1, 4 );
}

void
StatusBarTest::popped ( guint, const char * ) 
{
  if ( gtkStatusBar . messages_begin () == gtkStatusBar . messages_end () )
    statusbarCounter = 1;
}

void
StatusBarTest::contexts () 
{
  string descriptions [] = 
  {
    "any context",
    "idle messages",
    "some text",
    "hit the mouse",
    "hit the mouse2"
  };

  int nDescriptions = sizeof ( descriptions ) / sizeof ( string );

  for ( int n = 0; n < nDescriptions; n++ ) 
      cout << "GtkStatusBar: context=\""
	   << descriptions[ n ]
	   << "\", context_id="
	   << gtkStatusBar . get_context_id ( descriptions [ n ] )
	   << endl;

}

void
StatusBarTest::dump_stack ()
{
  GSList * list;

  for ( Gtk::Statusbar::iterator iter = gtkStatusBar . messages_begin ();
       iter != gtkStatusBar . messages_end ();
       iter++ ) 
    {
      GtkStatusbarMsg * msg = *iter;

      cout << "context_id: "
	   <<  msg->context_id
	   << ", message_id: "
	   << msg->message_id
	   << ", status_text: \""
	   << msg->text
	   << "\""
	   << endl;
    }
}
