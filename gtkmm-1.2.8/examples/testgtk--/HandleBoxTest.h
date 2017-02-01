#include "TestFixture.h"

/*
** Gtk::HandleBox
*/

class HandleBoxTest : public TestFixture
{
public:
  static TestFixture *   create ();
  virtual               ~HandleBoxTest () {} ;
  virtual void           destroyTest ();
private:
                         HandleBoxTest ();
  void                   childSignal ( Gtk::Widget * child, const string action );

  static HandleBoxTest * theTest;

  Gtk::VBox       vbox;
  Gtk::HBox       hbox;
  Gtk::HandleBox  handleBox;
  Gtk::HandleBox  handleBox2;
  Gtk::HandleBox  handleBox3;
  
};

