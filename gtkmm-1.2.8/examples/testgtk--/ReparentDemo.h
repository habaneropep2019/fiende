#include "TestFixture.h"

/*
** Reparent demo
*/

class ReparentDemo : public TestFixture 
{
public:

  static TestFixture * create ();
  virtual              ~ReparentDemo ();
  virtual void         destroyTest ();
private:
  static ReparentDemo * theTest;
                       ReparentDemo ();
  void                 reparent_label ( Gtk::Widget *button );
  void                 set_parent_signal ( Gtk::Widget *old_parent, int func_data );

  Gtk::VBox             box1;
  Gtk::HBox             hbox2;
  Gtk::VBox             vbox2;
  Gtk::Label            label;
};
