#include "TestFixture.h"

/*
** Gtk::RadioButton
*/

class RadioButtonTest : TestFixture 
{
public:
  static TestFixture * create ();
  virtual             ~RadioButtonTest () {};
  virtual void         destroyTest ();
private:
  // ctor
                       RadioButtonTest ();
  // functions
  // data
  static RadioButtonTest * theTest;
  Gtk::RadioButton::Group   group;
  Gtk::RadioButton          button1;
  Gtk::RadioButton          button2;
  Gtk::RadioButton          button3;
};
