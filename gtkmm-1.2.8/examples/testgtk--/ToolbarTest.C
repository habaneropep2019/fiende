#include "ToolbarTest.h"


ToolBarTestWidget::ToolBarTestWidget () :
  Gtk::Toolbar ( GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH )
{

  pixHorizontal = new Gtk::Pixmap ( "test.xpm" );
  pixVertical = new Gtk::Pixmap ( "test.xpm" );
  pixIcons = new Gtk::Pixmap ( "test.xpm" );
  pixText = new Gtk::Pixmap ( "test.xpm" );
  pixBoth = new Gtk::Pixmap ( "test.xpm" );
  pixSmall = new Gtk::Pixmap ( "test.xpm" );
  pixBig = new Gtk::Pixmap ( "test.xpm" );
  pixEnable = new Gtk::Pixmap ( "test.xpm" );
  pixDisable = new Gtk::Pixmap ( "test.xpm" );
  pixBorders = new Gtk::Pixmap ( "test.xpm" );
  pixBorderless = new Gtk::Pixmap ( "test.xpm" );
#if GTK_VERSION_GT(1,0)
  set_button_relief ( GTK_RELIEF_NONE );
#endif

  //  Gtk::Toolbar_Helpers::ToolList tool_list ( this );

  tools () . 
    push_back ( Gtk::Toolbar_Helpers::ButtonElem  ( Gtk::nstring ( "Horizontal" ),
						   * pixHorizontal, 
						   slot( this, &ToolBarTestWidget::horizontal ) ,
						   Gtk::nstring ( "Horizontal toolbar layout" ),
						   Gtk::nstring ( "Toolbar/Horizontal" ) ) );

  tools () . 
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Vertical",                       
						  *pixVertical,
						  slot ( this, &ToolBarTestWidget::vertical ),
						  "Vertical toolbar layout",
						  "Toolbar/Vertical" ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::Space () );

  tools () . 
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ("Icons", 
						 *pixIcons,
 						 slot ( this, &ToolBarTestWidget::icons ),
						 "Only show toolbar icons", 
						 "Toolbar/IconsOnly" ) );
  tools () . 
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Text", 
						  *pixText,
						  slot ( this, &ToolBarTestWidget::text ) ,
						  "Only show toolbar text", 
						  "Toolbar/TextOnly" ) );

  tools () . 
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Both", 
						  *pixBoth, 
						  slot ( this, &ToolBarTestWidget::both ),
						  "Show toolbar icons and text", 
						  "Toolbar/Both" ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::Space () );

  tools () .
    push_back ( Gtk::Toolbar_Helpers::WidgetElem ( entry, 
						  "This is an unusable GtkEntry ;)",
						  "Hey don't click me!!!" ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::Space () );

  tools () .
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Small", 
						  * pixSmall, 
						  slot ( this, &ToolBarTestWidget::smallSpace ),
						  "Use small spaces", 
						  "Toolbar/Small" ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Big", 
						  * pixBig, 
						  slot ( this, &ToolBarTestWidget::bigSpace ),
						  "Use big spaces", 
						  "Toolbar/Big" ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::Space () );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Enable", 
						  * pixEnable, 
						  slot ( this, &ToolBarTestWidget::enable ), 
						  "Enable tooltips", 
						  NULL ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Disable",
						  * pixDisable,
						  slot ( this, &ToolBarTestWidget::disable ),
						  "Disable tooltips",
						  NULL ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::Space () );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Borders", 
						  * pixBorders, 
						  slot ( this, &ToolBarTestWidget::borders ),
						  "Show Borders", 
						  NULL ) );
  tools () .
    push_back ( Gtk::Toolbar_Helpers::ButtonElem ( "Borderless", 
						  * pixBorderless, 
						  slot ( this, &ToolBarTestWidget::borderless ),
						  "Hide Borders", 
						  NULL ) );
}

ToolBarTestWidget::~ToolBarTestWidget ()
{
  delete pixHorizontal;
  delete pixVertical;
  delete pixIcons;
  delete pixText;
  delete pixBoth;
  delete pixSmall;
  delete pixBig;
  delete pixEnable;
  delete pixDisable;
  delete pixBorders;
  delete pixBorderless;
}

void
ToolBarTestWidget::horizontal ()
{
  set_orientation ( GTK_ORIENTATION_HORIZONTAL );
}

void
ToolBarTestWidget::vertical ()
{
  set_orientation ( GTK_ORIENTATION_VERTICAL );
}

void
ToolBarTestWidget::icons ()
{
  set_style( GTK_TOOLBAR_ICONS);
}

void
ToolBarTestWidget::text ()
{
  set_style ( GTK_TOOLBAR_TEXT );
}

void
ToolBarTestWidget::both ()
{
  set_style ( GTK_TOOLBAR_BOTH );
}

void
ToolBarTestWidget::smallSpace ()
{
  set_space_size ( 5 );
}

void
ToolBarTestWidget::bigSpace ()
{
  set_space_size ( 10 );
}

void
ToolBarTestWidget::enable ()
{
  set_tooltips ( true );
}

void
ToolBarTestWidget::disable ()
{
  set_tooltips ( false );
}

void
ToolBarTestWidget::borders ()
{
#if GTK_VERSION_GT(1,0)
  set_button_relief ( GTK_RELIEF_NORMAL );
#endif
}

void
ToolBarTestWidget::borderless ()
{
#if GTK_VERSION_GT(1,0)
  set_button_relief ( GTK_RELIEF_NONE );
#endif
}


ToolBarTest * ToolBarTest::theTest = 0;

TestFixture * 
ToolBarTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new ToolBarTest ();
      return theTest;
    }
  return 0;
}

void ToolBarTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


ToolBarTest::ToolBarTest () :
  TestFixture ()
{
  set_title ( "Toolbar test" );
  realize ();
  actionArea . pack_start ( toolbar );
  packControlArea ();
  show_all ();
}

ToolBarTest::~ToolBarTest () 
{
}


