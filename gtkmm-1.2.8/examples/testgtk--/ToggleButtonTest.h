/*
** ===========================================================================
** $RCSfile: ToggleButtonTest.h,v $
** $Revision: 1.4 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

#include "TestFixture.h"

#ifndef ToggleButtonTest_h
#define ToggleButtonTest_h "$Id: ToggleButtonTest.h,v 1.4 2000/03/07 14:21:41 kenelson Exp $"

/*
** Gtk::ToggleButton
*/

class ToggleButtonTest : public TestFixture 
{
public:
  static TestFixture *  create ();
  virtual              ~ToggleButtonTest ();
  virtual void          destroyTest ();
private:
                        ToggleButtonTest ();
  //functions
  // data
  static ToggleButtonTest * theTest;
  Gtk::ToggleButton          button1;
  Gtk::ToggleButton          button2;
  Gtk::ToggleButton          button3;
};

inline
ToggleButtonTest::~ToggleButtonTest ()
{
}

#endif
