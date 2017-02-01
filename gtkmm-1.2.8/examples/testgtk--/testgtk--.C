/* $Id: testgtk--.C,v 1.13 2000/03/03 07:39:18 tdukes Exp $ */
/* GTK-- - The GIMP Toolkit C++ Wrapper
 * Copyright (C) 1998-1999 Todd Dukes
 * based on work  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/* $Id: testgtk--.C,v 1.13 2000/03/03 07:39:18 tdukes Exp $ */
/* GTK-- - The GIMP Toolkit C++ Wrapper
 * Copyright (C) 1998-1999 Todd Dukes
 * based on work  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#undef	G_LOG_DOMAIN

#include <strstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <list>
#include "gtk--.h"
#include "gdk/gdk.h"
#include "gdk/gdkx.h"
#include "gdk/gdkkeysyms.h"

#include "ButtonBoxTest.h"      
#include "ButtonTest.h"	     
#include "CheckButtonTest.h"    
#include "CListTest.h"
#include "FileSelectionTest.h"  
#include "HandleBoxTest.h"      
#include "PaneTest.h"		  
#include "RadioButtonTest.h"	  
#include "ReparentDemo.h"	  
#include "SavedPosition.h"	  
#include "StatusbarTest.h"
#include "TestFixture.h"
#include "ToggleButtonTest.h"
#include "ToolbarTest.h"
#include "TooltipsTest.h"
#include "TreeTest.h"
#include "PixmapTest.h"
#include "circles.xbm"

#define TRACE_LINE cout << __FILE__ << ":" << __LINE__ << endl

typedef  TestFixture * ( * TestFunc ) ();

struct TestTest 
{

  string   buttonLabel;
  TestFunc f;

} testToTest [] = { { "buttons",        &ButtonTest::create },
		    { "toggle buttons", &ToggleButtonTest::create },
		    { "check buttons",  &CheckButtonTest::create },
		    { "radio buttons",  &RadioButtonTest::create },
		    { "button box",     &ButtonBoxTest::create },
		    { "tool bar",       &ToolBarTest::create },
		    { "status bar",     &StatusBarTest::create },
		    { "file selection", &FileSelectionTest::create },
		    { "handle box",     &HandleBoxTest::create },
		    { "reparent demo",  &ReparentDemo::create },
		    { "panes",          &PaneTest::create },
		    { "pixmap",         &PixmapTest::create },
                    { "tooltips",       &TooltipsTest::create },
		    { "clist",          &CListTest::create }
};

//typedef list < Gtk::Widget * > PL_Gtk_Widget;

// This is a callback that will hand a widget being destroyed.
void destroy_handler()
{
  cout << "destroy_handler" << endl;
  Gtk::Main::quit();
  cout << "after quit" << endl;
}

class MainWindow : public Gtk::Window 
{
public:
  MainWindow ();
  ~MainWindow ();
  void testFinished ( TestFixture * testWindow );
private:
  /* test fixtures */
  void buttonClicked( int n);
  /* main window data */
  Gtk::VBox           vbox;
  Gtk::VBox           testFixturesBox;
  Gtk::ScrolledWindow scrolled_window;
  Gtk::VBox           closeButtonBox;
  Gtk::Button         *button;
  Gtk::Label          label;
  string             buffer;
  Gtk::HSeparator     separator;
  Gtk::Button         closeButton;
};

MainWindow::MainWindow ():
  // main window data
  vbox ( false, 0 ),
  testFixturesBox ( false, 0 ),
  closeButtonBox ( false, 10 ), 
  button ( 0 ),
  closeButton ( "close" )
{

  set_policy ( FALSE, FALSE, FALSE );
  set_name ( "main window" );
  set_usize ( 200, 400 );
  set_uposition ( 20, 20 );
  //  destroy . connect ( SigC::slot ( &destroy_handler ) );

  // what does this do? How can I connect it?
  //  delete_event.connect(slot(gtk_false));

  //  gtk_signal_connect (GTK_OBJECT (window), "delete-event",
  //		      GTK_SIGNAL_FUNC (gtk_false),
  //		      NULL);
  add ( vbox );
  {

    strstream ss;

    if ( gtkmm_micro_version > 0 )
      {
	ss << "Gtk-- v" 
	   << gtkmm_major_version << "."
	   << gtkmm_minor_version << "."
	   << gtkmm_micro_version << ends;
      } 
    else 
      {
	ss << "Gtk-- v"
	   << gtkmm_major_version << "."
	   << gtkmm_minor_version << ends;
      }
    buffer = ss . str ();
  } // end scope of ss
  label . set_text ( buffer );
  vbox . pack_start ( label, false, false, 0 );

  //  scrolled_window = new Gtk::ScrolledWindow();
  scrolled_window . set_border_width ( 10 );
  scrolled_window . set_policy ( GTK_POLICY_AUTOMATIC, 
				 GTK_POLICY_AUTOMATIC );
  vbox . pack_start ( scrolled_window, true, true, 0 );
  testFixturesBox . set_border_width ( 10 );
  scrolled_window . add_with_viewport ( testFixturesBox );
  
  Gtk::Adjustment * adj = scrolled_window . get_vadjustment();

  if ( adj )
    {
      testFixturesBox . set_focus_vadjustment ( *adj );
    } 
  else
    {
      cout << "testgtk-- warning: scrolled_window.get_vadjustment() returned 0" << endl;
    }
  testFixturesBox . show ();

  int numberOfTest = sizeof ( testToTest ) / sizeof ( TestTest );

  cout << "testFixturesBox object is at " << testFixturesBox . gtkobj () << endl;

  cout << "check to see if it is an container " << endl;
  cout << "GTK_IS_CONTAINER ( ... ) "
       << GTK_IS_CONTAINER ( testFixturesBox . gtkobj () )
       << endl;

  for ( int n = 0; n < numberOfTest; n++ )
    {

      Gtk::Button * pb = new Gtk::Button ( testToTest [ n ] . buttonLabel );

      //      deletableChildren . push_back ( pb );
      testFixturesBox . pack_start ( * manage ( pb ) );
      //testFixturesBox . pack_start ( * pb ) ;
      
      pb -> clicked . connect ( bind ( slot ( this, &( MainWindow::buttonClicked ) ), n ) );

    }
  vbox . pack_start ( separator, false );
  closeButtonBox . set_border_width ( 10 );
  vbox . pack_start ( closeButtonBox, false );
  closeButton.clicked.connect(Gtk::Main::quit.slot());

  closeButtonBox . pack_start ( closeButton );
  closeButton . set_flags ( GTK_CAN_DEFAULT );
  closeButton . grab_default ();
  show_all ();

}

MainWindow::~MainWindow ()
{
  cout << "check to see if testFixturesBox is still a container " << endl;
  cout << "GTK_IS_CONTAINER ( ... ) "
       << GTK_IS_CONTAINER ( testFixturesBox . gtkobj () )
       << endl;
}

void 
MainWindow::testFinished ( TestFixture * testWindow )
{ 
  testWindow->hide();
  testWindow->destroyTest();
}

void
MainWindow::buttonClicked ( int n )
{
  TestFixture * testFixture;
  testFixture = testToTest [ n ] . f ();
  if ( testFixture ) 
    testFixture->finished.connect(slot(this,&MainWindow::testFinished));


}

int 
main ( int argc, char *argv [] )
{
  Gtk::Main m ( argc, argv );
  {
    MainWindow mainWindow;
 
    mainWindow . show ();
    m . run ();
  }
  TRACE_LINE;
  return 0;
}
