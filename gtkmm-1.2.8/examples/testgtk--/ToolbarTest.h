#include "TestFixture.h"

/*
** GtkToolBar
*/

class ToolBarTestWidget : public Gtk::Toolbar 
{
public:
       ToolBarTestWidget ();
      ~ToolBarTestWidget ();
  void horizontal ();
  void vertical ();
  void icons ();
  void text ();
  void both ();
  void smallSpace ();
  void bigSpace ();
  void enable ();
  void disable ();
  void borders ();
  void borderless ();
private:
  Gtk::Pixmap  *pixHorizontal;
  Gtk::Pixmap  *pixVertical;
  Gtk::Pixmap  *pixIcons;
  Gtk::Pixmap  *pixText;
  Gtk::Pixmap  *pixBoth;
  Gtk::Entry    entry;
  Gtk::Pixmap  *pixSmall;
  Gtk::Pixmap  *pixBig;
  Gtk::Pixmap  *pixEnable;
  Gtk::Pixmap  *pixDisable;
  Gtk::Pixmap  *pixBorders;
  Gtk::Pixmap  *pixBorderless;
};

class ToolBarTest : public TestFixture 
{
public:
  // functions
  static TestFixture * create ();
  virtual             ~ToolBarTest ();
  virtual void         destroyTest ();
  //  data
  static ToolBarTest * theTest;
private:
  // ctor
                       ToolBarTest ();
  // functions
  ToolBarTestWidget    toolbar;
  // data
};
