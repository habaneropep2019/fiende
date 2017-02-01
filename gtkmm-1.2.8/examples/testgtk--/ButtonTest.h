/*
** ===========================================================================
** $RCSfile: ButtonTest.h,v $
** $Revision: 1.4 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

/*
** Gtk::Button
*/

#include "TestFixture.h"

#ifndef ButtonTest_h
#define ButtonTest_h "$Id: ButtonTest.h,v 1.4 2000/03/07 14:21:41 kenelson Exp $"


class ButtonTest : public TestFixture 
{
public:
  static TestFixture *  create ();
  virtual              ~ButtonTest () { };
  virtual void          destroyTest ();
private:
                        ButtonTest ();
  // functions
  void                  toggleShowButton ( int buttonIndex );
  // data
  static ButtonTest * theTest;
  Gtk::VBox            box2;
  Gtk::Table           table;
  Gtk::Button         *button [ 9 ];
};

#endif
