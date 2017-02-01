/*
** ===========================================================================
** $RCSfile: TestFixture.h,v $
** $Revision: 1.7 $
** $Date: 2001/02/16 21:18:37 $
** $Author: kenelson $
** ===========================================================================
*/

#include<iostream>

#include "gtk--.h"

namespace std { };
using namespace std;

namespace SigC { };
using namespace SigC;

/*
**
**  TestFixture 
**
*/

#ifndef TestFixture_h
#define TestFixture_h "$Id: TestFixture.h,v 1.7 2001/02/16 21:18:37 kenelson Exp $"

class TestFixture : public Gtk::Window 
{
public:
  virtual void destroyTest () = 0;
  //  static TestFixture * getTheTest() { return theTest;}
  SigC::Signal1 < void, TestFixture * > finished;

protected:
           TestFixture ();
  virtual ~TestFixture ();
  void     packControlArea ();
  // data
  Gtk::VBox            vbox;
  Gtk::VBox            actionArea;
  Gtk::VBox            controlArea;
  Gtk::HSeparator      separator;
  Gtk::Button          closeButton;
  //  list< Gtk::Widget *> childrenToDelete;
  //
  void delete_self() { manage(this)->destroy(); }
};

#endif
