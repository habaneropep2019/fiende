/*
** ===========================================================================
** $RCSfile: PixmapTest.h,v $
** $Revision: 1.4 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

/*
 * GtkPixmap
 */

#include "TestFixture.h"

#ifndef PixmapTest_h
#define PixmapTest_h "$Id: PixmapTest.h,v 1.4 2000/03/07 14:21:41 kenelson Exp $"


class PixmapTest : public TestFixture 
{
public:
  static TestFixture *  create ();
  virtual              ~PixmapTest () { };
  virtual void          destroyTest ();
private:
                        PixmapTest ();
  // functions
  void                  toggleShowButton ( int buttonIndex );
  // data
  static PixmapTest * theTest;

  Gtk::VBox       vbox1;
  Gtk::VBox       vbox2;
  Gtk::HBox       hbox3;
  Gtk::VBox       vbox4;
  Gtk::Button     button;
  Gtk::Pixmap    *pixmap;
  Gtk::Label      label;
  Gtk::HSeparator separator;
};

#endif
