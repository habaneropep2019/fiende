#include "TestFixture.h"

/*
 * Gtk::ButtonBox
 */

class ButtonBoxFrame : public Gtk::Frame 
{
public:
  ButtonBoxFrame ( bool horizontal, string title,
		   gint spacing, gint child_w, 
		   gint child_h, GtkButtonBoxStyle layout );
  ~ButtonBoxFrame () { cout << "~ButtonBoxFrame" << endl; delete bbox;}
private:
  Gtk::ButtonBox *bbox;
  Gtk::Button     okButton;
  Gtk::Button     cancelButton;
  Gtk::Button     helpButton;
};


class ButtonBoxTest : public TestFixture 
{
public:
  static TestFixture * create ();
  virtual             ~ButtonBoxTest () {};
  virtual void         destroyTest ();
private:
  // ctor
  ButtonBoxTest ();
  // functions
  Gtk::Frame * create_bbox ( bool horizontal, string title, 
			    gint spacing, gint child_w, 
			    gint child_h, gint layout );
  // data
  static ButtonBoxTest * theTest;
  Gtk::Frame              hFrame;
  Gtk::VBox               hframeVBox;
  Gtk::Frame              vFrame;
  Gtk::HBox               vframeHBox;
};

