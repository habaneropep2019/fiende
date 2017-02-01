#include "HandleBoxTest.h"
#include "ToolbarTest.h"

HandleBoxTest * HandleBoxTest::theTest = 0;

TestFixture * 
HandleBoxTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new HandleBoxTest ();
      return theTest;
    }
  return 0;
}

void HandleBoxTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}



void
HandleBoxTest::childSignal ( Gtk::Widget *child, const string action ) 
{
  cout << gtk_type_name ( GTK_OBJECT_TYPE ( gtkobj () ) )
       << ": child <" << gtk_type_name ( GTK_OBJECT_TYPE ( child -> gtkobj ()  ) )
       << action << "ed" << endl;
}
			  

HandleBoxTest::HandleBoxTest () :
  handleBox ()
{

  ToolBarTestWidget *toolbar;
  Gtk::Label         *label;
  Gtk::HSeparator    *separator;

  set_border_width ( 20 );

  actionArea . pack_start ( vbox );

  vbox . pack_start ( * manage ( new Gtk::Label ( "Above" ) ) );
  vbox . pack_start ( * manage ( new Gtk::HSeparator ) );
  vbox . pack_start ( hbox );
  vbox . pack_start ( * manage ( new Gtk::HSeparator ) );
  vbox . pack_start ( * manage ( new Gtk::Label ( "Below" )) );
  vbox . show_all ();

  hbox . pack_start ( handleBox );

  toolbar = manage( new ToolBarTestWidget);
  toolbar -> borders ();

  handleBox.child_attached.connect(bind(slot(this,&(HandleBoxTest::childSignal)),string("attached")));

  handleBox.child_detached.connect(bind(slot(this,&(HandleBoxTest::childSignal)),string("detached")));
  
  handleBox . add ( *toolbar );
  handleBox . show_all ();

  hbox . pack_start ( handleBox2 );
  handleBox2 . show ();

  handleBox2.child_attached.connect(bind(slot(this,&(HandleBoxTest::childSignal)),string("child_attached")));

  handleBox2.child_detached.connect(bind(slot(this,&(HandleBoxTest::childSignal)),string("child_detached")));


  handleBox2 . add ( handleBox3 );
  
  handleBox3.child_attached.connect(bind(slot(this,&(HandleBoxTest::childSignal)),string("attached")));

  handleBox3.child_detached.connect(bind(slot(this,&(HandleBoxTest::childSignal)),string("detached")));

  handleBox3 . add ( * manage( new Gtk::Label ("Fooo!") ) );
  handleBox3 . show_all ();

  packControlArea ();
  show_all ();
}

