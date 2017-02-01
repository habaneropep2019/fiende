#include "PaneTest.h"
PaneTest * PaneTest::theTest = 0;

TestFixture*
PaneTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new PaneTest ();
      return theTest;
  }
  return 0;
}

void PaneTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}



PaneOptions::PaneOptions(Gtk::Paned &paned,
			 const string &frame_label,
			 const string &label1x,
			 const string &label2x)
  : Gtk::Frame(frame_label),
    table(3, 2, 4),
    label1(label1x),
    label2(label2x),
    resize1("Resize"),
    shrink1("Shrink"),
    resize2("Resize"),
    shrink2("Shrink"),
    refPaned(paned)
{
  set_border_width(4);

  add(table);

  table.attach(label1,  0, 1, 0, 1);
  table.attach(resize1, 0, 1, 1, 2);

  const Gtk::Widget *child1 = paned.get_child1();
  

  resize1.toggled.connect(bind(slot(this,&PaneOptions::toggleResize),true));


  table.attach(shrink1, 0, 1, 2, 3);
  shrink1.set_active(true);
  shrink1.toggled.connect(bind(slot(this,&PaneOptions::toggleShrink),true));


  table.attach(label2,  1, 2, 0, 1);
  table.attach(resize2, 1, 2, 1, 2);
  resize2.set_active(true);
  resize2.toggled.connect(bind(slot(this,&PaneOptions::toggleResize),false));


  table.attach(shrink2, 1, 2, 2, 3);
  shrink2.set_active(true);
  shrink2.toggled.connect(bind(slot(this,&PaneOptions::toggleShrink),false));


}


void
PaneOptions::toggleResize(bool isChild1)
{
  bool resize, shrink;

  resize = isChild1 ? refPaned.get_child1_resize()
    : refPaned.get_child2_resize();

  shrink = isChild1 ? refPaned.get_child1_shrink()
    : refPaned.get_child2_shrink();

  Gtk::Widget *child = isChild1 ?
    refPaned.get_child1() : refPaned.get_child2();
  
//   child->ref();
  
  refPaned.remove(const_cast<Gtk::Widget&>(*child));

  if (isChild1)
    refPaned.pack1 (*child, !resize, shrink);
  else
    refPaned.pack2 (*child, !resize, shrink);

//   child->unref ();
}


void
PaneOptions::toggleShrink (bool isChild1)
{
  bool resize, shrink;

  resize = isChild1 ? refPaned.get_child1_resize()
    : refPaned.get_child2_resize();

  shrink = isChild1 ? refPaned.get_child1_shrink()
    : refPaned.get_child2_shrink();

  Gtk::Widget *child = isChild1 ?
    refPaned.get_child1() : refPaned.get_child2();
  
//   child->ref();
  
  refPaned.remove(const_cast<Gtk::Widget&>(*child));

  if (isChild1)
    refPaned.pack1 (*child, resize, !shrink);
  else
    refPaned.pack2 (*child, resize, !shrink);

//   child->unref ();
}


PaneTest::PaneTest()
  : button("Hi there"),
    vbox(false, 0)
{

  set_title("Panes");
  set_border_width(0);
  actionArea.pack_start(vbox);

  vbox.pack_start(vpaned);

  vpaned.set_border_width(5);
  
  vpaned.add1(hpaned);

  frame1.set_shadow_type(GTK_SHADOW_IN);
  frame1.set_usize(60, 60);

  hpaned.add1(frame1);

  frame1.add(button);
  
  frame2.set_shadow_type(GTK_SHADOW_IN);
  frame2.set_usize(80, 60);

  hpaned.add2(frame2);

  frame3.set_shadow_type(GTK_SHADOW_IN);
  frame3.set_usize(60, 80);

  vpaned.add2(frame3);

  hpaneOptions = new PaneOptions(hpaned, "Horizontal", "Left", "Right");
  vpaneOptions = new PaneOptions(vpaned, "Vertical", "Top", "Bottom");  

  vbox.pack_start(*hpaneOptions);
  vbox.pack_start(*vpaneOptions);

  packControlArea();
  show_all();
}

PaneTest::~PaneTest()
{
  delete hpaneOptions;
  delete vpaneOptions;
}


