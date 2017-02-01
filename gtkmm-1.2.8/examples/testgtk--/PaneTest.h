#include "TestFixture.h"

class PaneOptions : public Gtk::Frame
{
public:
  PaneOptions(Gtk::Paned &paned,
	      const string &frame_label,
	      const string &label1,
	      const string &label2);

  void toggleResize(bool isChild1);
  void toggleShrink(bool isChild1);

private:
  Gtk::Table table;
  Gtk::Label label1, label2;
  Gtk::CheckButton resize1, shrink1,
    resize2, shrink2;

  Gtk::Paned &refPaned;
  
};

class PaneTest : public TestFixture
{
  public:
  static TestFixture *  create ();
  virtual              ~PaneTest ();
  virtual void          destroyTest ();
                        PaneTest ();
private:
  // data
  static PaneTest * theTest;
  Gtk::Frame           frame1, frame2, frame3;
  Gtk::HPaned          hpaned;
  Gtk::VPaned          vpaned;
  Gtk::Button          button;
  Gtk::VBox            vbox;

  PaneOptions         *hpaneOptions, *vpaneOptions;
  
};

