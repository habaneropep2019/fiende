/*
** ===========================================================================
** $RCSfile: TooltipsTest.C,v $
** $Revision: 1.6 $
** $Date: 2000/05/14 18:48:17 $
** $Author: kenelson $
** ===========================================================================
*/

#include "TooltipsTest.h"

static char * pc_rcs_h = TooltipsTest_h;
static char * pc_rcs = "$Id: TooltipsTest.C,v 1.6 2000/05/14 18:48:17 kenelson Exp $";

#define USE(var) static void * use_##var = (void *) var
USE( pc_rcs_h);
USE( pc_rcs);

TooltipsTest * TooltipsTest::theTest = 0;

#define TRACE_LINE cout << __FILE__ << ":" << __LINE__ << endl;

TestFixture*
TooltipsTest::create () 
{
  if ( theTest == 0 ) 
    {
      theTest = new TooltipsTest ();
      return theTest;
  }
  return 0;
}

void TooltipsTest::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}



TooltipsTest::TooltipsTest () :
  vbox2 ( false, 10 ),
  toggleButton1 ( "button 1" ),
  toggleButton2 ( "button 2" ),
  toggleButton3 ( "Override TipsQuery Label"),
  button ( "[?]" ),
  frame ( "ToolTips Inspector" )
{
  set_border_width ( 0 );
  set_title ( "Tooltips" );
  set_policy ( TRUE,  // allow_shrink,
	       FALSE, // allow_grow,
	       TRUE); // auto_shrink
  
  actionArea . pack_start ( vbox1 );

  vbox2 . set_border_width ( 10 );
  vbox1 . pack_start ( vbox2 );

  vbox2 . pack_start ( toggleButton1 );
  tooltips . set_tip ( toggleButton1, "This is button 1",
		       "ContextHelp/button/1" );

  vbox2 . pack_start ( toggleButton2 );
  tooltips . set_tip ( toggleButton2,
		       "This is button 2. This is also a really "
		       "long tooltip which probably won't fit on "
		       "a single line and will therefore need to "
		       "be wrapped. Hopefully the wrapping will "
		       "work correctly.",
		       "ContextHelp/buttons/2_long");
 
  vbox2 . pack_start ( toggleButton3 );
  tooltips . set_tip ( toggleButton3, "Toggle TipsQuery view.", 
		       "Hi msw! ;)");

  // don't connect this signal until the widget_entered/widget_selected signals are
  // available, and work.
  //  button.clicked.connect(tipsQuery.start_query.slot());


  vbox3 . pack_start ( button );

  tooltips . set_tip ( button, 
		       "Start the Tooltips Inspector - "
		       "Currently this does not work.",
		       "ContextHelp/buttons/?");

  vbox3. pack_start ( tipsQuery );
  tipsQuery . set_caller ( button );

  // these two signals are commented out in tipsquery.gen_h
  //  tipsQuery.widget_entered.connect(slot(this,&TooltipsTest::widget_entered));

  //  tipsQuery.widget_selected.connect(slot(this,&TooltipsTest::widget_selected));

  //  frame . label_xalign ( static_cast < double > ( 0.5 ) );

  // there seems to be no way to set x alignment only, I will let y align default
  frame . set_label_align ( static_cast < double > ( 0.5 ) );
  frame . set_border_width ( 0 );
  vbox2 . pack_start ( frame, true, true, 10 );
  frame . add ( vbox3 );

  packControlArea ();
  show_all ();
}

void
TooltipsTest::widget_entered ( Gtk::Widget      *widget,
			       const gchar    *tip_text,
			       const gchar    *tip_private )
{
  TRACE_LINE;
  //  if (GTK_TOGGLE_BUTTON (toggle)->active)
  if ( toggleButton3 . gtkobj () -> active )
    {
      tipsQuery . set ( tip_text ? "There is a Tip!" : "There is no Tip!");
      //      gtk_label_set (GTK_LABEL (tips_query), tip_text ? "There is a Tip!" : "There is no Tip!");
      /* don't let GtkTipsQuery reset it's label */
      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( tipsQuery . gtkobj () ), "widget_entered" );
    }
}

gint
TooltipsTest::widget_selected ( Gtk::Widget      *widget,
				const gchar    *tip_text,
				const gchar    *tip_private,
				GdkEventButton *event/*,
						       gpointer        func_data */ )
{
  TRACE_LINE;
  if (widget)
    g_print ("Help \"%s\" requested for <%s>\n",
	     tip_private ? tip_private : "None",
	     gtk_type_name (GTK_OBJECT_TYPE (widget)));
  return TRUE;
}

#ifdef JUNK
static void
create_tooltips (void)
{
  static GtkWidget *window = NULL;
  GtkWidget *box1;
  GtkWidget *box2;
  GtkWidget *box3;
  GtkWidget *button;
  GtkWidget *toggle;
  GtkWidget *frame;
  GtkWidget *tips_query;
  GtkWidget *separator;
  GtkTooltips *tooltips;

  if (!window)
    {
      window =
	gtk_widget_new (gtk_window_get_type (),
			"GtkWindow::type", GTK_WINDOW_TOPLEVEL,
			"GtkContainer::set_border_width", 0,
			"GtkWindow::title", "Tooltips",
			"GtkWindow::allow_shrink", TRUE,
			"GtkWindow::allow_grow", FALSE,
			"GtkWindow::auto_shrink", TRUE,
			"GtkWidget::width", 200,
			NULL);

      gtk_signal_connect (GTK_OBJECT (window), "destroy",
                          GTK_SIGNAL_FUNC (destroy_tooltips),
                          &window);

      tooltips=gtk_tooltips_new();
      gtk_object_set_data (GTK_OBJECT (window), "tooltips", tooltips);
      
      box1 = gtk_vbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (window), box1);

      box2 = gtk_vbox_new (FALSE, 10);
      gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
      gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 0);

      button = gtk_toggle_button_new_with_label ("button1");
      gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);

      gtk_tooltips_set_tip (tooltips,button,"This is button 1", "ContextHelp/buttons/1");

      button = gtk_toggle_button_new_with_label ("button2");
      gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);

      gtk_tooltips_set_tip (tooltips,
			    button,
			    "This is button 2. This is also a really long tooltip which probably won't fit on a single line and will therefore need to be wrapped. Hopefully the wrapping will work correctly.",
			    "ContextHelp/buttons/2_long");

      toggle = gtk_toggle_button_new_with_label ("Override TipsQuery Label");
      gtk_box_pack_start (GTK_BOX (box2), toggle, TRUE, TRUE, 0);

      gtk_tooltips_set_tip (tooltips, toggle, "Toggle TipsQuery view.", "Hi msw! ;)");

      box3 =
	gtk_widget_new (gtk_vbox_get_type (),
			"GtkBox::homogeneous", FALSE,
			"GtkBox::spacing", 5,
			"GtkContainer::set_border_width", 5,
			"GtkWidget::visible", TRUE,
			NULL);

      tips_query = gtk_tips_query_new ();

      button =
	gtk_widget_new (gtk_button_get_type (),
			"GtkButton::label", "[?]",
			"GtkWidget::visible", TRUE,
			"GtkWidget::parent", box3,
			"GtkObject::object_signal::clicked", gtk_tips_query_start_query, tips_query,
			NULL);
      gtk_box_set_child_packing (GTK_BOX (box3), button, FALSE, FALSE, 0, GTK_PACK_START);
      gtk_tooltips_set_tip (tooltips,
			    button,
			    "Start the Tooltips Inspector",
			    "ContextHelp/buttons/?");
      
      
      gtk_widget_set (tips_query,
		      "GtkWidget::visible", TRUE,
		      "GtkWidget::parent", box3,
		      "GtkTipsQuery::caller", button,
		      "GtkObject::signal::widget_entered", tips_query_widget_entered, toggle,
		      "GtkObject::signal::widget_selected", tips_query_widget_selected, NULL,
		      NULL);
      
      frame =
	gtk_widget_new (gtk_frame_get_type (),
			"GtkFrame::label", "ToolTips Inspector",
			"GtkFrame::label_xalign", (double) 0.5,
			"GtkContainer::set_border_width", 0,
			"GtkWidget::visible", TRUE,
			"GtkWidget::parent", box2,
			"GtkContainer::child", box3,
			NULL);
      gtk_box_set_child_packing (GTK_BOX (box2), frame, TRUE, TRUE, 10, GTK_PACK_START);

      separator = gtk_hseparator_new ();
      gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

      box2 = gtk_vbox_new (FALSE, 10);
      gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
      gtk_box_pack_start (GTK_BOX (box1), box2, FALSE, TRUE, 0);

      button = gtk_button_new_with_label ("close");
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                                 GTK_SIGNAL_FUNC(gtk_widget_destroy),
                                 GTK_OBJECT (window));
      gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (button);

      gtk_tooltips_set_tip (tooltips, button, "Push this button to close window", "ContextHelp/buttons/Close");
    }

  if (!GTK_WIDGET_VISIBLE (window))
    gtk_widget_show_all (window);
  else
    gtk_widget_destroy (window);
}


static void
tips_query_widget_entered (GtkTipsQuery   *tips_query,
			   GtkWidget      *widget,
			   const gchar    *tip_text,
			   const gchar    *tip_private,
			   GtkWidget	  *toggle)
{
  if (GTK_TOGGLE_BUTTON (toggle)->active)
    {
      gtk_label_set (GTK_LABEL (tips_query), tip_text ? "There is a Tip!" : "There is no Tip!");
      /* don't let GtkTipsQuery reset it's label */
      gtk_signal_emit_stop_by_name (GTK_OBJECT (tips_query), "widget_entered");
    }
}

static gint
tips_query_widget_selected (GtkWidget      *tips_query,
			    GtkWidget      *widget,
			    const gchar    *tip_text,
			    const gchar    *tip_private,
			    GdkEventButton *event,
			    gpointer        func_data)
{
  if (widget)
    g_print ("Help \"%s\" requested for <%s>\n",
	     tip_private ? tip_private : "None",
	     gtk_type_name (GTK_OBJECT_TYPE (widget)));
  return TRUE;
}
#endif
