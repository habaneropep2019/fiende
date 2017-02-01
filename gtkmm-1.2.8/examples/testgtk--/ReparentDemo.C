#include "ReparentDemo.h"

using SigC::bind;
using SigC::slot;
using namespace Gtk;

ReparentDemo * ReparentDemo::theTest = 0;

TestFixture *
ReparentDemo::create ()
{
  if ( theTest == 0 ) 
    {
      theTest = new ReparentDemo ();
      return theTest;
    }
  return 0;
}

void ReparentDemo::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


ReparentDemo::~ReparentDemo () 
{ 
  hide(); 
};	

void
ReparentDemo::reparent_label ( Widget *newParent )
{
   label.reparent ( *newParent );
}

char* get_type_name (Widget* w)
{
  if (!w) return "NULL";
  return  gtk_type_name( GTK_OBJECT_TYPE( w->gtkobj() ) );
}

void
ReparentDemo::set_parent_signal ( Widget *old_parent,
				  int    data )
{
  // the child is always label
  cout << "set_parent for \""
       << get_type_name( &label )
       << "\": new parent: \""
       << get_type_name( label.get_parent ())
       << "\", old parent: \""
       << get_type_name( old_parent )
       << "\", data: "
       << data 
       << endl;
}

ReparentDemo::ReparentDemo () :
  hbox2 ( false, 5 ),
  vbox2 ( false, 10 ),
  label ( "Hello World" )
{
  VBox   *box3,*box4;
  Frame  *frame1,*frame2;
  Button *button1,*button2;

  label.parent_changed.connect (
    bind( slot(this, &ReparentDemo::set_parent_signal), 42));

  box3 = new VBox (false, 5);
  box3->set_border_width (5);

  frame1 = manage( new Frame("Frame 1") );
  frame1->add (*box3);

  button1 = manage( new Button( "switch" ) );
  button1->clicked.connect(
    bind<Widget*>(slot(this,&(ReparentDemo::reparent_label)),box3));

  box3->pack_start (*button1, false, true, 0);
  box3->pack_start (label, false, true, 0 );

  box4 = new VBox (false, 5 );
  box4->set_border_width (5);

  frame2 = manage( new Frame("Frame 2") );
  frame2->add (*box4);

  button2 = manage( new Button ( "switch" ) );
  button2->clicked.connect(
    bind<Widget*>( slot(this, &ReparentDemo::reparent_label), box4) );

  box4->pack_start (*button2, false, true, 0 );

  vbox2.set_border_width (10);

  hbox2.set_border_width ( 10 );
  hbox2.pack_start (*frame1, true, true, 0);
  hbox2.pack_start (*frame2, true, true, 0);

  box1.pack_start (hbox2, true, true, 0 );
  box1.pack_start (*manage( new HSeparator() ), false, true, 0 );
  box1.pack_start (vbox2, false, true, 0 );

  actionArea.pack_start (box1);
  packControlArea ();

  set_title ("reparent");
  set_border_width (0);
  show_all ();
}
