#include "TestFixture.h"

/*
** Gtk::CheckButton
*/

class CheckButtonTest : public TestFixture 
{
public:
  static TestFixture * create ();
  virtual             ~CheckButtonTest () {};
  virtual void         destroyTest ();
private:
  // ctor
                       CheckButtonTest ();
  //functions
  // data
  static CheckButtonTest * theTest;
  Gtk::CheckButton          button1;
  Gtk::CheckButton          button2;
  Gtk::CheckButton          button3;
};
