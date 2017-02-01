/*
** ===========================================================================
** $RCSfile: CListTest.C,v $
** $Revision: 1.8 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

#include <stdio.h>

#include "CListTest.h"
#include "gtk_mini.xpm"

static char * pc_rcs_h = CListTest_h;
static char * pc_rcs = "$Id: CListTest.C,v 1.8 2000/03/07 14:21:41 kenelson Exp $";

#define USE(var) static void * use_##var = (void *) var
USE( pc_rcs_h);
USE( pc_rcs);

CListTest * CListTest::theTest = 0;

#define TRACE_LINE cout << __FILE__ << ":" << __LINE__ << endl;

#define TESTGTK_CLIST_COLUMNS 20

TestFixture*
CListTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new CListTest ();
      return theTest;
  }
  return 0;
}

void CListTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


CListTest::CListTest () :
  _clist ( 0 ),
  _clist_rows ( 0 ),
  _clist_omenu ( 0 ),
  _style1 ( 0 ),
  _style2 ( 0 ),
  _style3 ( 0 )
{
  guint i;
  static const char *titles[] =
  {
    "auto resize", "not resizeable", "max width 100", "min width 50",
      "hide column", "Title 5", "Title 6", "Title 7",
      "Title 8",  "Title 9",  "Title 10", "Title 11", "Title 12",
      "Title 13", "Title 14", "Title 15", "Title 16", "Title 17",
      "Title 18", "Title 19"
      };


  char text[TESTGTK_CLIST_COLUMNS][50];
  const char *texts[TESTGTK_CLIST_COLUMNS];

  set_border_width ( 0 );
  set_title ( "clist" );
  set_policy ( TRUE,  // allow_shrink,
	       FALSE, // allow_grow,
	       TRUE); // auto_shrink
  
  
  Gtk::VBox * vbox = new Gtk::VBox;

  actionArea . pack_start ( * manage ( vbox ) );

  Gtk::ScrolledWindow * sw = new Gtk::ScrolledWindow;

  sw -> set_border_width ( 5 );
  sw -> set_policy ( GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

  _clist = new Gtk::CList ( TESTGTK_CLIST_COLUMNS, titles );
  sw -> add ( * _clist ); // notice that the clist is NOT managed.
  _clist->click_column.connect(slot(this,&CListTest::click_column));


  /* control buttons */
  Gtk::HBox * hbox = new Gtk::HBox ( false, 5 );

  hbox -> set_border_width ( 5 );
  vbox -> pack_start ( * manage ( hbox ), false, false );

  Gtk::Button *  button = new Gtk::Button ( "Insert Row" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::insert_row));


  button = new Gtk::Button ( "Add 1,000 Rows With Pixmaps" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::add1000));


  button = new Gtk::Button ( "Add 10,000 Rows" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::add10000));



  /* second layer of buttons */
  
  hbox = new Gtk::HBox ( false, 5 );

  hbox -> set_border_width ( 5 );
  vbox -> pack_start ( * manage ( hbox ), false, false, 0 );

  button = new Gtk::Button ( "Clear List" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::clear));

  
  button = new Gtk::Button ( "Remove Selection" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::remove_selection));


  button = new Gtk::Button ( "Undo Selection" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::undo_selection));


  button = new Gtk::Button ( "Warning Test" );
  hbox -> pack_start ( * manage ( button ) );
  button->clicked.connect(slot(this,&CListTest::warning_test));


  /* third layer of buttons */

  hbox = new Gtk::HBox ( false, 5 );
  hbox -> set_border_width ( 5 );
  vbox -> pack_start ( * manage ( hbox ), false, false);

  Gtk::CheckButton * cb = new Gtk::CheckButton ( "Hide Title Buttons" );
  hbox -> pack_start ( * manage ( cb ) );
  cb->clicked.connect(slot(this,&CListTest::toggle_title_buttons));


  cb = new Gtk::CheckButton ( "Reorderable" );
  hbox -> pack_start ( * manage ( cb ), false );
  cb->clicked.connect(slot(this,&CListTest::toggle_reorderable));

  cb -> set_active ( true );

  Gtk::Label * label = new Gtk::Label ( "Selection Mode :" );
  hbox -> pack_start ( * manage ( label ), false );

  Gtk::OptionMenu * clist_omenu = build_option_menu ();
  hbox -> pack_start ( * manage ( clist_omenu ), false );

  /*
   * the rest of the clist configuration 
   */
  vbox -> pack_start ( * manage ( sw ) );
  _clist -> set_row_height ( 18 );
  _clist -> set_usize ( -1, 300 );

  for ( int i = 1; i < TESTGTK_CLIST_COLUMNS; i++ )
    {
      _clist -> set_column_width ( i, 80 );
    }
     
  _clist -> set_column_auto_resize ( 0, true );
  _clist -> set_column_resizeable ( 1, false );
  _clist -> set_column_max_width ( 2, 100 );
  _clist -> set_column_min_width ( 3, 50 );
  _clist -> set_selection_mode ( GTK_SELECTION_EXTENDED );
  _clist -> set_column_justification ( 1, GTK_JUSTIFY_RIGHT );
  _clist -> set_column_justification ( 2, GTK_JUSTIFY_CENTER );

  for ( int i = 0; i < TESTGTK_CLIST_COLUMNS; i++ )
    {
      // yeah, I know I should change this.  
      texts[i] = text[i];
      sprintf ( text[i], "Column %d", i );
    }

  sprintf ( text [ 1 ], "Right" );
  sprintf ( text [ 2 ], "Center" );

//   Gdk_Color col1;
//   Gdk_Color col2;

//   col1 . set_rgb ( 56000,      0,      0 );
//   col2 . set_rgb (     0,  56000,  32000 );

  GdkColor col1;
  GdkColor col2;

  col1.red   = 0;
  col1.green = 56000;
  col1.blue  = 0;
  col2.red   = 32000;
  col2.green = 0;
  col2.blue  = 56000;

  Gtk::Style * style = Gtk::Style::create();
  
  style -> set_font ( Gdk_Font( 
                      "-adobe-helvetica-bold-r-*-*-*-140-*-*-*-*-*-*" ));
  style -> set_fg ( GTK_STATE_NORMAL, col1 );
  style -> set_base ( GTK_STATE_NORMAL, col2 );

  for (i = 0; i < 10; i++)
    {
      sprintf ( text [ 0 ], "CListRow %d", _clist_rows++ );
      //      _clist -> append ( texts );
      _clist -> rows () . push_back ( texts );
      
      switch ( i % 4 )
	{
	case 2:
	  _clist -> set_row_style ( 1, *style );
	  break;
	default:
	  _clist -> set_cell_style ( i, i % 4, *style);
	  break;
	}
    }

  // set up styles
  {
//     Gdk_Color col1;
//     Gdk_Color col2;

//     col1 . set_rgb (     0, 56000,      0 );
//     col2 . set_rgb ( 32000,     0,  56000 );

    GdkColor col1;
    GdkColor col2;

    col1 . red   = 0;
    col1 . green = 56000;
    col1 . blue  = 0;
    col2 . red   = 32000;
    col2 . green = 0;
    col2 . blue  = 56000;

    _style1 = _clist -> get_style () -> copy ();
    _style1 -> set_base ( GTK_STATE_NORMAL, col1 );
    _style1 -> set_base ( GTK_STATE_SELECTED, col2 );

    _style2 = _clist -> get_style () -> copy ();
    _style2 -> set_fg ( GTK_STATE_NORMAL, col1 );
    _style2 -> set_fg ( GTK_STATE_SELECTED, col2 );

    _style3 = _clist -> get_style () -> copy ();
    _style3 -> set_fg ( GTK_STATE_NORMAL, col1 );
    _style3 -> set_base ( GTK_STATE_NORMAL, col2 );

    //    _style3 . get_font () . unref ();
    //    Gdk_Font font ( "-*-courier-medium-*-*-*-*-120-*-*-*-*-*-*" );
    //    _style3 . set_font ( font );
  }

  packControlArea ();
  show_all ();
}


void
CListTest::add1000()
{
  gint i, row;
  char text[TESTGTK_CLIST_COLUMNS][50];
  const char *texts[TESTGTK_CLIST_COLUMNS];
  GdkBitmap *mask;
  GdkPixmap *pixmap;
  GtkCList  *clist;

  pixmap = gdk_pixmap_create_from_xpm_d ( _clist -> gtkobj () -> clist_window,
					  &mask, 
					  &GTK_WIDGET (_clist -> gtkobj () )->style->white,
					  gtk_mini_xpm);

  for (i = 0; i < TESTGTK_CLIST_COLUMNS; i++)
    {
      texts[i] = text[i];
      sprintf (text[i], "Column %d", i);
    }
  
  texts[3] = NULL;
  sprintf (text[1], "Right");
  sprintf (text[2], "Center");
  
  _clist -> freeze ();
  for (i = 0; i < 1000; i++)
    {
      sprintf (text[0], "CListRow %d", rand() % 10000);
      //      row = _clist -> append ( texts);
      _clist -> rows () . push_back ( texts );
      row = _clist -> rows () . size ();
      _clist -> set_pixtext ( row, 3, "gtk+", 5, pixmap, mask);
    }

  _clist -> thaw ();

}

void
CListTest::add10000 ()
{
  gint i;
  char text[TESTGTK_CLIST_COLUMNS][50];
  const char *texts[TESTGTK_CLIST_COLUMNS];

  for (i = 0; i < TESTGTK_CLIST_COLUMNS; i++)
    {
      texts[i] = text[i];
      sprintf (text[i], "Column %d", i);
    }
  
  sprintf (text[1], "Right");
  sprintf (text[2], "Center");
  
  _clist -> freeze ();
  for (i = 0; i < 10000; i++)
    {
      sprintf (text[0], "CListRow %d", rand() % 10000);
      _clist -> rows () . push_back ( texts);
    }
  _clist -> thaw ();
}

void
CListTest::clear ()
{
  _clist -> clear ();
}

void
CListTest::remove_focus_row ()
{
  _clist -> remove_row ( _clist -> gtkobj () -> focus_row );
}

void
CListTest::show_titles ()
{
    _clist -> column_titles_show ( );
}

void
CListTest::hide_titles ()
{
  _clist -> column_titles_hide ();
}

void
CListTest::select ( gint row_in, gint column_in, GdkEventButton * event_in,  GtkWidget * button_in )
{
  gint i;
  guint8 spacing;
  string text;
  Gdk_Pixmap pixmap;
  Gdk_Bitmap mask;
  GList *list;
  
  g_print ("GtkCList Selection: row %d column %d button %d\n", 
	   row_in, column_in, event_in ? event_in -> button : 0 );

  for (i = 0; i < TESTGTK_CLIST_COLUMNS; i++)
    {
      switch ( _clist -> get_cell_type ( row_in, i))
	{
	case GTK_CELL_TEXT:
	  g_print ("CELL %d GTK_CELL_TEXT\n", i);
	  text = _clist -> get_text ( row_in, i );
	  cout << "TEXT: " <<  text << endl;
	  break;

	case GTK_CELL_PIXMAP:
	  g_print ("CELL %d GTK_CELL_PIXMAP\n", i);
	  _clist -> get_pixmap ( row_in, i, pixmap, mask);
	  cout << "PIXMAP: " << &pixmap << endl;
	  cout << "MASK: " << &mask << endl;
	  break;

	case GTK_CELL_PIXTEXT:
	  g_print ("CELL %d GTK_CELL_PIXTEXT\n", i);
	  _clist -> get_pixtext ( row_in, i, text, spacing, pixmap, mask);
	  cout << "TEXT: " << text << endl;
	  cout << "SPACING: " << spacing << endl;
	  cout << "PIXMAP: " << &pixmap << endl;
	  cout << "MASK: " << &mask << endl;
	  break;

	default:
	  break;
	}
    }

  /* print selections list */
  g_print ("\nSelected Rows:");
  list = _clist -> gtkobj () -> selection;
  while (list)
    {
      g_print (" %d ", GPOINTER_TO_INT (list->data));
      list = list->next;
    }

  g_print ("\n\n\n");

}

void
CListTest::insert_row ()
{
  static char *text[] =
  {
    "This", "is an", "inserted", "row.",
    "This", "is an", "inserted", "row.",
    "This", "is an", "inserted", "row."
  };

  static Gtk::Style *style1 = NULL;
  static Gtk::Style *style2 = NULL;
  static Gtk::Style *style3 = NULL;
  gint row;
  
#if 0
  if (GTK_CLIST (data)->focus_row >= 0)
    row = gtk_clist_insert (GTK_CLIST (data), GTK_CLIST (data)->focus_row,
			    text);
  else
    row = gtk_clist_prepend (GTK_CLIST (data), text);

  if (!style1)
    {
      GdkColor col1;
      GdkColor col2;

      col1.red   = 0;
      col1.green = 56000;
      col1.blue  = 0;
      col2.red   = 32000;
      col2.green = 0;
      col2.blue  = 56000;

      style1 = gtk_style_copy (GTK_WIDGET (data)->style);
      style1->base[GTK_STATE_NORMAL] = col1;
      style1->base[GTK_STATE_SELECTED] = col2;

      style2 = gtk_style_copy (GTK_WIDGET (data)->style);
      style2->fg[GTK_STATE_NORMAL] = col1;
      style2->fg[GTK_STATE_SELECTED] = col2;

      style3 = gtk_style_copy (GTK_WIDGET (data)->style);
      style3->fg[GTK_STATE_NORMAL] = col1;
      style3->base[GTK_STATE_NORMAL] = col2;
      gdk_font_unref (style3->font);
      style3->font =
	gdk_font_load ("-*-courier-medium-*-*-*-*-120-*-*-*-*-*-*");
    }

  gtk_clist_set_cell_style (GTK_CLIST (data), row, 3, style1);
  gtk_clist_set_cell_style (GTK_CLIST (data), row, 4, style2);
  gtk_clist_set_cell_style (GTK_CLIST (data), row, 0, style3);

  clist_rows++;
#endif
}

void
CListTest::warning_test ()
{
}

void
CListTest::undo_selection ()
{
}

void
CListTest::toggle_sel_mode ()
{
}

void
CListTest::click_column ( gint column )
{
}



Gtk::OptionMenu *
CListTest::build_option_menu ()
{
  char * items[] =  { "Single",
		      "Browse", 
		      "Multiple",
		      "Extended"
		    };

  Gtk::OptionMenu * omenu = new Gtk::OptionMenu ();

  Gtk::Menu * menu = new Gtk::Menu;

  Gtk::RadioMenuItem::Group group;
  
  for ( unsigned int i = 0 ; i < sizeof ( items ) / sizeof ( char * ); i++ )
    {
      Gtk::RadioMenuItem * menu_item = new Gtk::RadioMenuItem ( group, items [ i ] );

      menu_item->activate.connect(slot(this,&CListTest::toggle_sel_mode));

      group = menu_item -> group ();

      menu -> append ( * manage ( menu_item ) );
  
      if ( i == 3 )
	menu_item -> set_state ( GTK_STATE_SELECTED );

      menu_item -> show ();
    }

  omenu -> set_menu ( *menu );
  omenu -> set_history ( 3 );
  
  return omenu;
}

void
CListTest::remove_selection ()
{

#if 0
  _clist -> freeze ();

  while ( _clist -> selbegin () != _clist ->selend () )
    {
      gint row;

      _clist_rows--;
      row = GPOINTER_TO_INT (clist->selection->data);

      gtk_clist_remove (clist, row);

      if (clist->selection_mode == GTK_SELECTION_BROWSE)
	break;
    }

  if (clist->selection_mode == GTK_SELECTION_EXTENDED && !clist->selection &&
      clist->focus_row >= 0)
    gtk_clist_select_row (clist, clist->focus_row, -1);

  gtk_clist_thaw (clist);
#endif
}
