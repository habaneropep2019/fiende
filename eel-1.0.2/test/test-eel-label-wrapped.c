#include <config.h>

#include <gtk/gtk.h>
#include <eel/eel-label.h>
#include <eel/eel-gdk-extensions.h>
#include <eel/eel-gtk-extensions.h>

static void
delete_event (GtkWidget *widget, GdkEvent *event, gpointer callback_data)
{
	gtk_main_quit ();
}


static const char text[] = 
"The Eel shell is under development; it's not "
"ready for daily use. Some features are not yet done, "
"partly done, or unstable. The program doesn't look "
"or act exactly the way it will in version 1.0."
"\n\n"
"If you do decide to test this version of Eel,  "
"beware. The program could do something  "
"unpredictable and may even delete or overwrite  "
"files on your computer."
"\n\n"
"For more information, visit http://eel.eazel.com.";

static GtkWidget *
create_gtk_label ()
{
	GtkWidget *label;

 	label = gtk_label_new (text);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	
	return label;
}

static GtkWidget *
create_eel_label (gboolean adjust_wrap_on_resize)
{
	GtkWidget *label;

 	label = eel_label_new (text);
	eel_label_set_wrap (EEL_LABEL (label), TRUE);
	eel_label_set_justify (EEL_LABEL (label), GTK_JUSTIFY_LEFT);
	eel_label_set_smooth_drop_shadow_offset (EEL_LABEL (label), 1);
 	eel_label_set_background_mode (EEL_LABEL (label), EEL_SMOOTH_BACKGROUND_SOLID_COLOR);
	eel_label_set_solid_background_color (EEL_LABEL (label), EEL_RGB_COLOR_WHITE);
	eel_label_set_smooth_drop_shadow_color (EEL_LABEL (label), EEL_RGB_COLOR_BLUE);
	eel_label_set_text_color (EEL_LABEL (label), EEL_RGB_COLOR_RED);
	eel_label_set_adjust_wrap_on_resize (EEL_LABEL (label), adjust_wrap_on_resize);

	return label;
}

static GtkWidget *
create_gtk_label_window (void)
{
	GtkWidget *gtk_window;
	GtkWidget *gtk_vbox;
	GtkWidget *gtk_label;

	gtk_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	eel_gtk_widget_set_background_color (gtk_window, "white");
	gtk_signal_connect (GTK_OBJECT (gtk_window), "delete_event", GTK_SIGNAL_FUNC (delete_event), NULL);
	gtk_window_set_title (GTK_WINDOW (gtk_window), "Gtk Wrapped Label Test");
	gtk_window_set_policy (GTK_WINDOW (gtk_window), TRUE, TRUE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (gtk_window), 10);
	gtk_vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (gtk_window), gtk_vbox);
 	gtk_label = create_gtk_label ();
	gtk_box_pack_start (GTK_BOX (gtk_vbox), gtk_label, TRUE, TRUE, 0);

	return gtk_window;
}

static GtkWidget *
create_eel_label_window (gboolean adjust_wrap_on_resize)
{
	GtkWidget *eel_window;
	GtkWidget *eel_vbox;
	GtkWidget *eel_label;

	eel_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	eel_gtk_widget_set_background_color (eel_window, "white");
	gtk_signal_connect (GTK_OBJECT (eel_window), "delete_event", GTK_SIGNAL_FUNC (delete_event), NULL);
	gtk_window_set_title (GTK_WINDOW (eel_window), "Eel Wrapped Label Test");
	gtk_window_set_policy (GTK_WINDOW (eel_window), TRUE, TRUE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (eel_window), 10);
	eel_vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (eel_window), eel_vbox);
 	eel_label = create_eel_label (adjust_wrap_on_resize);
	gtk_box_pack_start (GTK_BOX (eel_vbox), eel_label, TRUE, TRUE, 0);

	return eel_window;
}

int 
main (int argc, char* argv[])
{
	GtkWidget *eel_window = NULL;
	GtkWidget *eel_window_wrapped = NULL;
	GtkWidget *gtk_window = NULL;

	gtk_init (&argc, &argv);
	gdk_rgb_init ();

	if (0) eel_window = create_eel_label_window (FALSE);
	if (1) eel_window_wrapped = create_eel_label_window (TRUE);
	if (0) gtk_window =  create_gtk_label_window ();

	if (eel_window) gtk_widget_show_all (eel_window);
	if (eel_window_wrapped) gtk_widget_show_all (eel_window_wrapped);
	if (gtk_window) gtk_widget_show_all (gtk_window);

	gtk_main ();

	return 0;
}
