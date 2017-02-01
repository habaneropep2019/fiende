#ifdef JUNK

#include "SavedPosition.h"

SavedPosition * SavedPosition::theTest = 0;
gint            SavedPosition::upositionx = 0;
gint            SavedPosition::upositiony = 0;

TestFixture *
SavedPosition::create ()
{
  if ( theTest == 0 ) 
    {
      theTest = new SavedPosition ();
      return theTest;
    }
  return 0;
}

void SavedPosition::destroyTest()
{
  if (theTest) delete_self();
  theTest=0;
}


bool
SavedPosition::upositionConfigure () 
{
  Gtk::Label *lx;
  Gtk::Label *ly;
  strstream ss;

  lx = static_cast < Gtk::Label * > ( get_data ( "x" ) );
  ly = static_cast < Gtk::Label * > ( get_data ( "y" ) );

  gdk_window_get_root_origin ( get_window () ->window, &upositionx, &upositiony );
  
  ss << upositionx << ends;
  lx -> set ( ss . str () );

  ss << upositiony << ends;
  ly -> set ( ss . str () );

  return false;
}

// static gint
// uposition_configure (GtkWidget *window)
// {
//   GtkLabel *lx;
//   GtkLabel *ly;
//   gchar buffer[64];

//   lx = gtk_object_get_data (GTK_OBJECT (window), "x");
//   ly = gtk_object_get_data (GTK_OBJECT (window), "y");

//   gdk_window_get_root_origin (window->window, &upositionx, &upositiony);
//   sprintf (buffer, "%d", upositionx);
//   gtk_label_set (lx, buffer);
//   sprintf (buffer, "%d", upositiony);
//   gtk_label_set (ly, buffer);

//   return FALSE;
// }

void
SavedPosition::SavedPosition () 
{
  GtkWidget *hbox;
  GtkWidget *main_vbox;
  GtkWidget *vbox;
  GtkWidget *x_label;
  GtkWidget *y_label;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *any;

  set_title ( "Saved Position" );


  // how the hell am I going to do this. On to next widget test for now.
      window = gtk_widget_new (GTK_TYPE_WINDOW,
			       "type", GTK_WINDOW_TOPLEVEL,
			       "signal::configure_event", uposition_configure, NULL,
			       "x", upositionx,
			       "y", upositiony,
			       "title", "Saved Position",
			       NULL);

      gtk_signal_connect (GTK_OBJECT (window), "destroy",
			  GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			  &window);

      main_vbox = gtk_vbox_new (FALSE, 5);
      gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 0);
      gtk_container_add (GTK_CONTAINER (window), main_vbox);

      vbox =
	gtk_widget_new (gtk_vbox_get_type (),
			"GtkBox::homogeneous", FALSE,
			"GtkBox::spacing", 5,
			"GtkContainer::set_border_width", 10,
			"GtkWidget::parent", main_vbox,
			"GtkWidget::visible", TRUE,
			NULL);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

      label = gtk_label_new ("X Origin : ");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

      x_label = gtk_label_new ("");
      gtk_box_pack_start (GTK_BOX (hbox), x_label, TRUE, TRUE, 0);
      gtk_object_set_data (GTK_OBJECT (window), "x", x_label);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

      label = gtk_label_new ("Y Origin : ");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

      y_label = gtk_label_new ("");
      gtk_box_pack_start (GTK_BOX (hbox), y_label, TRUE, TRUE, 0);
      gtk_object_set_data (GTK_OBJECT (window), "y", y_label);

      any =
	gtk_widget_new (gtk_hseparator_get_type (),
			"GtkWidget::visible", TRUE,
			NULL);
      gtk_box_pack_start (GTK_BOX (main_vbox), any, FALSE, TRUE, 0);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
      gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, TRUE, 0);

      button = gtk_button_new_with_label ("Close");
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				 GTK_SIGNAL_FUNC (gtk_widget_destroy),
				 GTK_OBJECT (window));
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 5);
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (button);
      
      gtk_widget_show_all (window);
    }
  else
    gtk_widget_destroy (window);
}

static void
create_saved_position (void)
{
  static GtkWidget *window = NULL;

  if (!window)
    {
      GtkWidget *hbox;
      GtkWidget *main_vbox;
      GtkWidget *vbox;
      GtkWidget *x_label;
      GtkWidget *y_label;
      GtkWidget *button;
      GtkWidget *label;
      GtkWidget *any;

      window = gtk_widget_new (GTK_TYPE_WINDOW,
			       "type", GTK_WINDOW_TOPLEVEL,
			       "signal::configure_event", uposition_configure, NULL,
			       "x", upositionx,
			       "y", upositiony,
			       "title", "Saved Position",
			       NULL);

      gtk_signal_connect (GTK_OBJECT (window), "destroy",
			  GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			  &window);

      main_vbox = gtk_vbox_new (FALSE, 5);
      gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 0);
      gtk_container_add (GTK_CONTAINER (window), main_vbox);

      vbox =
	gtk_widget_new (gtk_vbox_get_type (),
			"GtkBox::homogeneous", FALSE,
			"GtkBox::spacing", 5,
			"GtkContainer::set_border_width", 10,
			"GtkWidget::parent", main_vbox,
			"GtkWidget::visible", TRUE,
			NULL);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

      label = gtk_label_new ("X Origin : ");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

      x_label = gtk_label_new ("");
      gtk_box_pack_start (GTK_BOX (hbox), x_label, TRUE, TRUE, 0);
      gtk_object_set_data (GTK_OBJECT (window), "x", x_label);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

      label = gtk_label_new ("Y Origin : ");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

      y_label = gtk_label_new ("");
      gtk_box_pack_start (GTK_BOX (hbox), y_label, TRUE, TRUE, 0);
      gtk_object_set_data (GTK_OBJECT (window), "y", y_label);

      any =
	gtk_widget_new (gtk_hseparator_get_type (),
			"GtkWidget::visible", TRUE,
			NULL);
      gtk_box_pack_start (GTK_BOX (main_vbox), any, FALSE, TRUE, 0);

      hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
      gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, TRUE, 0);

      button = gtk_button_new_with_label ("Close");
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				 GTK_SIGNAL_FUNC (gtk_widget_destroy),
				 GTK_OBJECT (window));
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 5);
      GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
      gtk_widget_grab_default (button);
      
      gtk_widget_show_all (window);
    }
  else
    gtk_widget_destroy (window);
}

#endif
