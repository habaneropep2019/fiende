/*
g++ -o testdnd `gtk-config --cflags --libs` \
               `gtkmm-config --cflags --libs` testdnd.C
*/
//#include "gtk/gtk.h"

#include <iostream> 
#include <gtk--.h>

namespace std {}
using namespace std;

namespace SigC { };
using namespace SigC;

#define TRACE_LINE cout << __FILE__ << ":" << __LINE__ << endl;
#define TRACE_MSG(x) cout << __FILE__ << ":" << __LINE__ << " " << x << endl;

/* Target side drag signals */

/* XPM */
static char * drag_icon_xpm[] = {
"36 48 9 1",
" 	c None",
".	c #020204",
"+	c #8F8F90",
"@	c #D3D3D2",
"#	c #AEAEAC",
"$	c #ECECEC",
"%	c #A2A2A4",
"&	c #FEFEFC",
"*	c #BEBEBC",
"               .....................",
"              ..&&&&&&&&&&&&&&&&&&&.",
"             ...&&&&&&&&&&&&&&&&&&&.",
"            ..&.&&&&&&&&&&&&&&&&&&&.",
"           ..&&.&&&&&&&&&&&&&&&&&&&.",
"          ..&&&.&&&&&&&&&&&&&&&&&&&.",
"         ..&&&&.&&&&&&&&&&&&&&&&&&&.",
"        ..&&&&&.&&&@&&&&&&&&&&&&&&&.",
"       ..&&&&&&.*$%$+$&&&&&&&&&&&&&.",
"      ..&&&&&&&.%$%$+&&&&&&&&&&&&&&.",
"     ..&&&&&&&&.#&#@$&&&&&&&&&&&&&&.",
"    ..&&&&&&&&&.#$**#$&&&&&&&&&&&&&.",
"   ..&&&&&&&&&&.&@%&%$&&&&&&&&&&&&&.",
"  ..&&&&&&&&&&&.&&&&&&&&&&&&&&&&&&&.",
" ..&&&&&&&&&&&&.&&&&&&&&&&&&&&&&&&&.",
"................&$@&&&@&&&&&&&&&&&&.",
".&&&&&&&+&&#@%#+@#@*$%$+$&&&&&&&&&&.",
".&&&&&&&+&&#@#@&&@*%$%$+&&&&&&&&&&&.",
".&&&&&&&+&$%&#@&#@@#&#@$&&&&&&&&&&&.",
".&&&&&&@#@@$&*@&@#@#$**#$&&&&&&&&&&.",
".&&&&&&&&&&&&&&&&&&&@%&%$&&&&&&&&&&.",
".&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.",
".&&&&&&&&$#@@$&&&&&&&&&&&&&&&&&&&&&.",
".&&&&&&&&&+&$+&$&@&$@&&$@&&&&&&&&&&.",
".&&&&&&&&&+&&#@%#+@#@*$%&+$&&&&&&&&.",
".&&&&&&&&&+&&#@#@&&@*%$%$+&&&&&&&&&.",
".&&&&&&&&&+&$%&#@&#@@#&#@$&&&&&&&&&.",
".&&&&&&&&@#@@$&*@&@#@#$#*#$&&&&&&&&.",
".&&&&&&&&&&&&&&&&&&&&&$%&%$&&&&&&&&.",
".&&&&&&&&&&$#@@$&&&&&&&&&&&&&&&&&&&.",
".&&&&&&&&&&&+&$%&$$@&$@&&$@&&&&&&&&.",
".&&&&&&&&&&&+&&#@%#+@#@*$%$+$&&&&&&.",
".&&&&&&&&&&&+&&#@#@&&@*#$%$+&&&&&&&.",
".&&&&&&&&&&&+&$+&*@&#@@#&#@$&&&&&&&.",
".&&&&&&&&&&$%@@&&*@&@#@#$#*#&&&&&&&.",
".&&&&&&&&&&&&&&&&&&&&&&&$%&%$&&&&&&.",
".&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.",
".&&&&&&&&&&&&&&$#@@$&&&&&&&&&&&&&&&.",
".&&&&&&&&&&&&&&&+&$%&$$@&$@&&$@&&&&.",
".&&&&&&&&&&&&&&&+&&#@%#+@#@*$%$+$&&.",
".&&&&&&&&&&&&&&&+&&#@#@&&@*#$%$+&&&.",
".&&&&&&&&&&&&&&&+&$+&*@&#@@#&#@$&&&.",
".&&&&&&&&&&&&&&$%@@&&*@&@#@#$#*#&&&.",
".&&&&&&&&&&&&&&&&&&&&&&&&&&&$%&%$&&.",
".&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.",
".&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.",
".&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.",
"...................................."};

/* XPM */
static char * trashcan_closed_xpm[] = {
"64 80 17 1",
" 	c None",
".	c #030304",
"+	c #5A5A5C",
"@	c #323231",
"#	c #888888",
"$	c #1E1E1F",
"%	c #767677",
"&	c #494949",
"*	c #9E9E9C",
"=	c #111111",
"-	c #3C3C3D",
";	c #6B6B6B",
">	c #949494",
",	c #282828",
"'	c #808080",
")	c #545454",
"!	c #AEAEAC",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                       ==......=$$...===                        ",
"                 ..$------)+++++++++++++@$$...                  ",
"             ..=@@-------&+++++++++++++++++++-....              ",
"          =.$$@@@-&&)++++)-,$$$$=@@&+++++++++++++,..$           ",
"         .$$$$@@&+++++++&$$$@@@@-&,$,-++++++++++;;;&..          ",
"        $$$$,@--&++++++&$$)++++++++-,$&++++++;%%'%%;;$@         ",
"       .-@@-@-&++++++++-@++++++++++++,-++++++;''%;;;%*-$        ",
"       +------++++++++++++++++++++++++++++++;;%%%;;##*!.        ",
"        =+----+++++++++++++++++++++++;;;;;;;;;;;;%'>>).         ",
"         .=)&+++++++++++++++++;;;;;;;;;;;;;;%''>>#>#@.          ",
"          =..=&++++++++++++;;;;;;;;;;;;;%###>>###+%==           ",
"           .&....=-+++++%;;####''''''''''##'%%%)..#.            ",
"           .+-++@....=,+%#####'%%%%%%%%%;@$-@-@*++!.            ",
"           .+-++-+++-&-@$$=$=......$,,,@;&)+!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           =+-++-+++-+++++++++!++++!++++!+++!++!+++=            ",
"            $.++-+++-+++++++++!++++!++++!+++!++!+.$             ",
"              =.++++++++++++++!++++!++++!+++!++.=               ",
"                 $..+++++++++++++++!++++++...$                  ",
"                      $$=.............=$$                       ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                "};

/* XPM */
static char * trashcan_open_xpm[] = {
"64 80 17 1",
" 	c None",
".	c #030304",
"+	c #5A5A5C",
"@	c #323231",
"#	c #888888",
"$	c #1E1E1F",
"%	c #767677",
"&	c #494949",
"*	c #9E9E9C",
"=	c #111111",
"-	c #3C3C3D",
";	c #6B6B6B",
">	c #949494",
",	c #282828",
"'	c #808080",
")	c #545454",
"!	c #AEAEAC",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                      .=.==.,@                  ",
"                                   ==.,@-&&&)-=                 ",
"                                 .$@,&++;;;%>*-                 ",
"                               $,-+)+++%%;;'#+.                 ",
"                            =---+++++;%%%;%##@.                 ",
"                           @)++++++++;%%%%'#%$                  ",
"                         $&++++++++++;%%;%##@=                  ",
"                       ,-++++)+++++++;;;'#%)                    ",
"                      @+++&&--&)++++;;%'#'-.                    ",
"                    ,&++-@@,,,,-)++;;;'>'+,                     ",
"                  =-++&@$@&&&&-&+;;;%##%+@                      ",
"                =,)+)-,@@&+++++;;;;%##%&@                       ",
"               @--&&,,@&)++++++;;;;'#)@                         ",
"              ---&)-,@)+++++++;;;%''+,                          ",
"            $--&)+&$-+++++++;;;%%'';-                           ",
"           .,-&+++-$&++++++;;;%''%&=                            ",
"          $,-&)++)-@++++++;;%''%),                              ",
"         =,@&)++++&&+++++;%'''+$@&++++++                        ",
"        .$@-++++++++++++;'#';,........=$@&++++                  ",
"       =$@@&)+++++++++++'##-.................=&++               ",
"      .$$@-&)+++++++++;%#+$.....................=)+             ",
"      $$,@-)+++++++++;%;@=........................,+            ",
"     .$$@@-++++++++)-)@=............................            ",
"     $,@---)++++&)@===............................,.            ",
"    $-@---&)))-$$=..............................=)!.            ",
"     --&-&&,,$=,==...........................=&+++!.            ",
"      =,=$..=$+)+++++&@$=.............=$@&+++++!++!.            ",
"           .)-++-+++++++++++++++++++++++++++!++!++!.            ",
"           .+-++-+++++++++++++++++++++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!+++!!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           .+-++-+++-+++++++++!++++!++++!+++!++!++!.            ",
"           =+-++-+++-+++++++++!++++!++++!+++!++!+++=            ",
"            $.++-+++-+++++++++!++++!++++!+++!++!+.$             ",
"              =.++++++++++++++!++++!++++!+++!++.=               ",
"                 $..+++++++++++++++!++++++...$                  ",
"                      $$==...........==$$                       ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                "};

void  
target_drag_leave	   (GtkWidget	       *widget,
			    GdkDragContext     *context,
			    guint               time)
{
  g_print("leave\n");
  exit ( 0 );
  //  have_drag = FALSE;
  //  gtk_pixmap_set (GTK_PIXMAP (widget), trashcan_closed, trashcan_closed_mask);
}

class Popupwindow : public Gtk::Window
{
public:
  Popupwindow ();
  gboolean popsite_motion ( GdkDragContext     *context,
			    gint                x,
			    gint                y,
			    guint               time );
  void popsite_leave ( GdkDragContext     *context,
		       guint               time );
private:
  bool         in_popup; // = FALSE;
  bool         popped_up; // = FALSE;
  SigC::Connection   popdown_timer; // = 0;
  bool         popdown_timer_set;
  SigC::Connection   popup_timer; // = 0;
  bool         popup_timer_set;
  Gtk::Table    table;

  gboolean  popup_motion ( GdkDragContext     *context,
			   gint                x,
			   gint                y,
			   guint32             time );
  void popup_leave ( GdkDragContext     *context,
		     guint               time );
  gint popup_cb (  );
  gint popdown_cb (  );
};

class TestDnd : public Gtk::Window
{
public:
  enum 
  {
    TARGET_STRING,
    TARGET_ROOTWIN,
    TARGET_URL
  };
  TestDnd ();
  ~TestDnd () {};
private:
  Gdk_Pixmap trashcan_open;
  Gdk_Bitmap trashcan_open_mask;
  Gdk_Pixmap trashcan_closed;
  Gdk_Bitmap trashcan_closed_mask;

  bool have_drag;
  
  Gtk::Table table;
  Gtk::Label label;
  Gtk::Label popupLabel;
  Gtk::Button button;
  Gtk::Pixmap pixmap;
  Gdk_Pixmap drag_icon;
  Gdk_Bitmap drag_mask;
  Gtk::Button quitButton;

  Popupwindow popup_window;


  void target_drag_leave ( GdkDragContext *context,
			   guint           time );
  gboolean target_drag_motion ( GdkDragContext *context,
			    gint            x,
			    gint            y,
			    guint           time );
  gboolean target_drag_drop ( GdkDragContext *context,
			      gint            x,
			      gint            y,
			      guint           time );
  void target_drag_data_received ( GdkDragContext     *context,
				   gint                x,
				   gint                y,
				   GtkSelectionData   *data,
				   guint               info,
				   guint32               time );
  void label_drag_data_received ( GdkDragContext     *context,
				  gint                x,
				  gint                y,
				  GtkSelectionData   *data,
				  guint               info,
				  guint32             time);

  void source_drag_data_get  ( GdkDragContext     *context,
			       GtkSelectionData   *selection_data,
			       guint               info,
			       guint32             time );

  void source_drag_data_delete ( GdkDragContext     *context );

  void buttonClicked (  );
};

static GtkTargetEntry target_table[] = {
  { "STRING",     0, TestDnd::TARGET_STRING },
  { "text/plain", 0, TestDnd::TARGET_STRING },
  { "text/uri-list", 0, TestDnd::TARGET_URL },
  { "application/x-rootwin-drop", 0, TestDnd::TARGET_ROOTWIN }
};

static guint n_targets = sizeof(target_table) / sizeof(target_table[0]);

void  
TestDnd::target_drag_leave ( GdkDragContext     *context,
			     guint               time)
{
  cout << "leave" << endl;
  have_drag = false;
  pixmap . set ( trashcan_closed, trashcan_closed_mask );
}

gboolean
TestDnd::target_drag_motion ( GdkDragContext     *context,
			      gint                x,
			      gint                y,
			      guint               time )
{
  GtkWidget *source_widget;

  if (!have_drag)
    {
      cout << "!have drag " << endl;
      have_drag = true;
      pixmap . set ( trashcan_open, trashcan_open_mask );
    }
  else 
    {
      cout << "have drag" << endl;
    }
    
  source_widget = gtk_drag_get_source_widget (context);

  cout << "motion, source "
       << ( source_widget ?
	    gtk_type_name (GTK_OBJECT (source_widget)->klass->type) :
	    "unknown")
       << endl;

  gdk_drag_status (context, context->suggested_action, time);
  return true;
}

gboolean
TestDnd::target_drag_drop ( GdkDragContext     *context,
			    gint                x,
			    gint                y,
			    guint               time_in)
{
  cout << __FILE__ << __LINE__ << " drop" << endl;
  have_drag = false;

  pixmap . set ( trashcan_closed, trashcan_closed_mask );

  if ( context -> targets )
    {
      //      Gdk_Drag_Context mm_context ( context );
      //      Gdk_Atom mm_atom ( GPOINTER_TO_INT ( context -> targets -> data ) );
      //      pixmap . drag_get_data ( mm_context, mm_atom, time_in );
      gtk_drag_get_data ( GTK_WIDGET ( pixmap . gtkobj () ), context, GPOINTER_TO_INT ( context->targets->data ),
			  time_in );
      return true;
    }
  
  return false;
}

void 
TestDnd::target_drag_data_received  ( GdkDragContext     *context,
				      gint                x,
				      gint                y,
				      GtkSelectionData   *data,
				      guint               info,
				      guint               time)
{
  Gdk_DragContext gdc ( context );
  if ( ( data -> length >= 0 ) && ( data -> format == 8 ) )
    {
      cout << "Received \"" 
	   << (gchar *)data->data
	   << "\" in trashcan"
	   << endl;
      Gtk::Widget::drag_finish ( gdc, true, false, time );
      return;
    }
  
  Gtk::Widget::drag_finish ( gdc, false, false, time );
}
  
void  
TestDnd::label_drag_data_received ( GdkDragContext     *context,
				    gint                x,
				    gint                y,
				    GtkSelectionData   *data,
				    guint               info,
				    guint               time )
{
  Gdk_DragContext gdc ( context );
  if ( ( data -> length >= 0 ) && ( data -> format == 8 ) )
    {
      cout << "Received \""
	   << (gchar *)data->data
	   << "\" in label"
	   << endl;
      Gtk::Widget::drag_finish ( gdc, true, false, time );
      return;
    }
  
  Gtk::Widget::drag_finish ( gdc , false, false, time );
}

void
TestDnd::buttonClicked ( )
{
  cout << "swapped pixmaps" << endl;
  pixmap . set ( trashcan_open, trashcan_open_mask );
}

void  
TestDnd::source_drag_data_get  ( GdkDragContext     *context,
				 GtkSelectionData   *selection_data,
				 guint               info,
				 guint32             time )
{
  if ( info == TARGET_ROOTWIN )
    {
      cout << "I was dropped on the rootwin" << endl;
    }
  else
    {
      if ( info == TARGET_URL )
	{
	  gtk_selection_data_set (selection_data,
				  selection_data->target,
				  8, 
				  reinterpret_cast < const unsigned char * >
				  ( "file:///home/otaylor/images/weave.png" ),
				  37 );
	}
      else
	{
	  gtk_selection_data_set (selection_data,
				  selection_data->target,
				  8, reinterpret_cast < const unsigned char * > 
				  ( "I'm Data!" ) , 9 );
	}
    }
}

/* The following is a rather elaborate example demonstrating/testing
 * changing of the window heirarchy during a drag - in this case,
 * via a "spring-loaded" popup window.
 */

int
Popupwindow::popdown_cb ()
{
  
  //  Popupwindow * puw = reinterpret_cast < Popupwindow * > ( data );

  popdown_timer_set = false;
  popdown_timer . disconnect ();
  hide ();
  popped_up = false;

  return false;
}

gboolean
Popupwindow::popup_motion ( GdkDragContext     *context,
			    gint                x,
			    gint                y,
			    guint32             time )
{
  if ( !in_popup )
    {
      in_popup = true;
      if ( popdown_timer_set )
	{
	  cout << "removed popdown" << endl;
	  //	  gtk_timeout_remove ( popdown_timer );
	  //	  popdown_timer = 0;
	  popdown_timer . disconnect ();
	  popdown_timer_set = false;
	}
    }

  return true;
}

void  
Popupwindow::popup_leave ( GdkDragContext     *context,
			   guint               time)
{
  cout << "popup_leave" << endl;
  if ( in_popup )
    {
      in_popup = false;
      if ( !popdown_timer_set )
	{
	  cout << "added popdown" << endl;
	  //	  popdown_timer = gtk_timeout_add (500, &Popupwindow::popdown_cb, this );
	  
	  SigC::Slot0<gint> my_slot = slot ( this, &Popupwindow::popdown_cb );
	  popdown_timer = Gtk::Main::timeout . connect ( my_slot, 500 );
	  popdown_timer_set = true;

	}
    }
}

Popupwindow::Popupwindow () :
  Gtk::Window ( GTK_WINDOW_POPUP ),
  in_popup ( false ),
  popped_up ( false ),
  popdown_timer ( 0 ),
  popdown_timer_set ( false ),
  popup_timer_set ( false ),
  table ( 3, 3, false )
{
  TRACE_LINE;
  int i, j;
	  
  set_position ( GTK_WIN_POS_MOUSE);

  Gtk::Button * button;
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      {
	char buffer[128];
	g_snprintf(buffer, sizeof(buffer), "%d,%d", i, j);
	button = new Gtk::Button (buffer);
	/*
	** by using manage the buttons will be deleted automatically.
	*/
	table . attach ( * manage ( static_cast < Gtk::Widget * > ( button ) ),
			 i, i+1, j, j+1,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL );

	button -> drag_dest_set ( GTK_DEST_DEFAULT_ALL,
				  target_table, 
				  n_targets - 1, /* no rootwin */
				  static_cast < GdkDragAction > ( GDK_ACTION_COPY | GDK_ACTION_MOVE ) );
	button -> drag_motion . connect ( slot ( this, &( Popupwindow::popup_motion ) ) );

	button -> drag_leave . connect ( slot ( this, &( Popupwindow::popup_leave ) ) );

      }

  table . show_all ();
  add ( table );
}

gint
Popupwindow::popup_cb (  )
{
  TRACE_MSG ( "popped_up " << popped_up );
  
  if (! popped_up)
    {
      TRACE_LINE;
      show_all ();
      popped_up = true;
    }

//   puw -> popdown_timer = gtk_timeout_add (500, &Popupwindow::popdown_cb, puw);
//   cout << "added popdown" << endl;
//   cout << __FILE__ << ":" << __LINE__ 
//        << "popdown_timer " << puw -> popdown_timer << endl;
  TRACE_MSG ( "connecting popdown timer" );
  popdown_timer = Gtk::Main::timeout . connect ( slot ( this, &Popupwindow::popdown_cb ), 500 );
  popdown_timer_set = true;

  return false;
}

gboolean
Popupwindow::popsite_motion ( GdkDragContext     *context,
			      gint                x,
			      gint                y,
			      guint               time)
{
  if ( ! popup_timer_set )
    {
      TRACE_MSG ( "connecting popup timer" );
      popup_timer = Gtk::Main::timeout . connect ( slot ( this, &Popupwindow::popup_cb ), 500 );
      popup_timer_set = true;
    }

  return true;
}

void  
Popupwindow::popsite_leave ( GdkDragContext     *context,
			     guint               time)
{
  if ( popup_timer_set )
    {
      TRACE_MSG ( "disconnecting popup_timer " );
      //      gtk_timeout_remove ( popup_timer );
      popup_timer . disconnect ();
      popup_timer_set = false;
    }
}

void  
TestDnd::source_drag_data_delete  ( GdkDragContext *context )
{
  cout << "Delete the data!" << endl;
}

TestDnd::TestDnd () :
  Gtk::Window ( GTK_WINDOW_TOPLEVEL ),
  have_drag ( false ),
  table ( 2, 2, false ),
  label ("Drop Here"),
  popupLabel ("Popup"),
  button ( "Drag here" ),
  pixmap ( trashcan_closed_xpm )
{
  
  destroy . connect ( Gtk::Main::quit . slot () );

  add ( table );

  Gdk_Color transparent;
  Gdk_Colormap cmap ( get_colormap () );

  drag_icon . create_colormap_from_xpm_d ( NULL, // drawable 
					   cmap, // colormap
					   drag_mask, // mask
					   transparent,  // transparent color
					   drag_icon_xpm); // data

  trashcan_open . create_colormap_from_xpm_d ( NULL,
					       cmap,
					       trashcan_open_mask,
					       transparent, 
					       trashcan_open_xpm);
  trashcan_closed . create_colormap_from_xpm_d ( NULL,
						 cmap,
						 trashcan_closed_mask,
						 transparent,
						 trashcan_closed_xpm);

  label . drag_dest_set ( GTK_DEST_DEFAULT_ALL,
			  target_table, n_targets - 1, /* no rootwin */
			  static_cast < GdkDragAction > ( GDK_ACTION_COPY | GDK_ACTION_MOVE) );

  label.drag_data_received . connect ( slot ( this, & ( TestDnd::label_drag_data_received ) ) );

  table . attach ( label, 0, 1, 0, 1,
		   GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		   0, 0);

  popupLabel . drag_dest_set ( GTK_DEST_DEFAULT_ALL,
			       target_table, n_targets - 1, /* no rootwin */
			       static_cast < GdkDragAction > ( GDK_ACTION_COPY | GDK_ACTION_MOVE) );

  table . attach ( popupLabel, 1, 2, 1, 2,
		   GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		   0, 0);

  popupLabel.drag_motion.connect(slot(popup_window,&Popupwindow::popsite_motion));


  popupLabel . drag_leave . connect ( slot ( popup_window, &( Popupwindow::popsite_leave ) ) );

  pixmap . set (trashcan_closed, trashcan_closed_mask);
  
  pixmap . drag_dest_set ( static_cast < GtkDestDefaults > ( 0 ) , 
 			   NULL, 0, static_cast < GdkDragAction > ( 0 ) );

  table . attach ( pixmap, 1, 2, 0, 1,
		   GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		   0, 0);

  pixmap . drag_leave . connect ( slot ( this, &( TestDnd::target_drag_leave ) ) );

//   gtk_signal_connect (GTK_OBJECT (pixmap . gtkobj () ), "drag_leave",
// 		      GTK_SIGNAL_FUNC (::target_drag_leave), NULL);


  pixmap . drag_motion . connect ( slot ( this, &( TestDnd::target_drag_motion ) ) );

  pixmap . drag_drop . connect ( slot ( this, &( TestDnd::target_drag_drop ) ) );

  pixmap . drag_data_received . connect ( slot ( this, &( TestDnd::target_drag_data_received ) ) );


  /* Drag site */

  button . drag_source_set ( static_cast < GdkModifierType > ( GDK_BUTTON1_MASK | GDK_BUTTON3_MASK ) ,
			     target_table, n_targets, 
			     static_cast < GdkDragAction > ( GDK_ACTION_COPY | GDK_ACTION_MOVE ) );

  button . drag_source_set_icon ( cmap,
				  drag_icon, drag_mask);

  button.clicked.connect(slot(this,&TestDnd::buttonClicked));

  table . attach ( button, 0, 1, 1, 2,
		   GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		   0, 0);

  button.drag_data_get.connect(slot(this,&TestDnd::source_drag_data_get));

  button.drag_data_delete.connect(slot(this,&TestDnd::source_drag_data_delete));

  show_all ();
}
  
int 
main (int argc, char **argv)
{
  Gtk::Main m (argc, argv); 

  TestDnd testDnd;
  
  m . run ();
}
