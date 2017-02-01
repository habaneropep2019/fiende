#include "FileSelectionTest.h"

FileSelectionTest * FileSelectionTest::theTest = 0;

TestFixture * 
FileSelectionTest::create ()
{
  if ( theTest == 0 ) 
    {
      theTest = new FileSelectionTest ();
      return theTest;
    }
  return 0;
}

void FileSelectionTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


void
FileSelectionTest::hide_fileops () 
{
  fileSelection . hide_fileop_buttons ();
}

void
FileSelectionTest::show_fileops () 
{
  fileSelection . show_fileop_buttons ();
}

void
FileSelectionTest::ok ()
{
  cout << fileSelection . get_filename () << endl;
  finished ( this );
}


FileSelectionTest::FileSelectionTest () :
  fileSelection ( "file selection dialog" )
{
  Gtk::Button * button;

  fileSelection . hide_fileop_buttons ();

  set_position ( GTK_WIN_POS_MOUSE );

  //fileSelection.get_ok_button()->clicked.connect(slot(this,&FileSelectionTest::ok));

  fileSelection . get_ok_button () -> clicked . connect ( slot (*this, &FileSelectionTest::ok ) );

  fileSelection.get_cancel_button()->clicked.connect(
      SigC::bind<TestFixture*>(finished.slot(),this));
      
  button = new Gtk::Button ( "Hide Fileops" );
  button->clicked.connect(slot(this,&FileSelectionTest::hide_fileops));

  fileSelection . get_action_area () -> pack_start ( * manage ( button ), false, false, 0 );
  button -> show ();
  
  button = new Gtk::Button ( "Show Fileops" );
  button->clicked.connect(slot(this,&FileSelectionTest::show_fileops));

  fileSelection . get_action_area () -> pack_start ( * manage ( button ), false, false, 0 );
  button -> show ();
  fileSelection . show ();
}

