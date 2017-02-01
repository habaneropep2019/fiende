#include "TestFixture.h"

/*
 * Gtk::StatusBar
 */

class StatusBarTest : public TestFixture 
{
public:
  // functions
  static TestFixture *   create ();
  virtual               ~StatusBarTest () {};
  virtual void           destroyTest ();
  //  data
  static StatusBarTest * theTest;
private:
  // ctor
                         StatusBarTest ();
  // functions
  void                   push ();
  void                   pop ();
  void                   steal ();
  void                   popped ( guint, const char *);
  void                   contexts ();
  void                   dump_stack ();
  // data
  Gtk::Button    pushSomethingButton;
  Gtk::Button    popButton;
  Gtk::Button    stealButton;
  Gtk::Button    dumpStackButton;
  Gtk::Button    contextButton;
  Gtk::Statusbar gtkStatusBar;
  guint         statusbarCounter;
};
