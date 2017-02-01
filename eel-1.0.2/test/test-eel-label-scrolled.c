#include "test.h"
#include <eel/eel-label.h>

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
label_window_new (void)
{
	GtkWidget *window;
	GtkWidget *label;
	EelBackground *background;

	window = test_window_new ("Scrolled Label Test", 10);

	background = eel_get_widget_background (GTK_WIDGET (window));
	eel_background_set_color (background, "white");

	/* Label */
	label = eel_label_new_solid (text,
					  0,
					  0,
					  EEL_RGB_COLOR_RED,
					  0.5,
					  0.5,
					  0,
					  0,
					  EEL_RGB_COLOR_WHITE,
					  NULL);
	eel_label_set_wrap (EEL_LABEL (label), TRUE);
	eel_label_set_justify (EEL_LABEL (label), GTK_JUSTIFY_LEFT);
	
	gtk_container_add (GTK_CONTAINER (window), label);

	gtk_widget_show (label);

	return window;
}

static GtkWidget *
label_window_new_scrolled (void)
{
	GtkWidget *window;
	GtkWidget *scrolled;
	GtkWidget *viewport;
	GtkWidget *label;
	EelBackground *background;

	window = test_window_new ("Scrolled Label Test", 10);

	/* Scrolled window */
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (window), scrolled);

	/* Viewport */
 	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (scrolled), viewport);

	background = eel_get_widget_background (GTK_WIDGET (viewport));
	eel_background_set_color (background, "white");

	/* Label */
	label = eel_label_new_solid (text,
					  0,
					  0,
					  EEL_RGB_COLOR_RED,
					  0.5,
					  0.5,
					  0,
					  0,
					  EEL_RGB_COLOR_WHITE,
					  NULL);
	eel_label_set_wrap (EEL_LABEL (label), TRUE);
	eel_label_set_justify (EEL_LABEL (label), GTK_JUSTIFY_LEFT);
//	eel_label_make_larger (EEL_LABEL (label), 10);

	gtk_container_add (GTK_CONTAINER (viewport), label);

	gtk_widget_show (label);
	gtk_widget_show (viewport);
	gtk_widget_show (scrolled);

	return window;
}

static GtkWidget *
label_window_new_table (void)
{
	GtkWidget *window;
	GtkWidget *scrolled;
	GtkWidget *viewport;
	GtkWidget *label[3];
	GtkWidget *table;
	EelBackground *background;

	window = test_window_new ("Scrolled Label Test", 10);

	/* Scrolled window */
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (window), scrolled);

	/* Viewport */
 	viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (scrolled), viewport);

	background = eel_get_widget_background (GTK_WIDGET (viewport));
	eel_background_set_color (background, "white");

	/* Table */
	table = gtk_table_new (20, 4, TRUE);
	gtk_container_add (GTK_CONTAINER (viewport), table);

	/* Label */
	label[0] = eel_label_new_solid ("Label One",
					     0,
					     0,
					     EEL_RGB_COLOR_RED,
					     0.5,
					     0.5,
					     0,
					     0,
					     EEL_RGB_COLOR_WHITE,
					     NULL);

	label[1] = eel_label_new_solid ("Label Two",
					     0,
					     0,
					     EEL_RGB_COLOR_GREEN,
					     0.5,
					     0.5,
					     0,
					     0,
					     EEL_RGB_COLOR_WHITE,
					     NULL);

	label[2] = eel_label_new_solid ("Label Three",
					     0,
					     0,
					     EEL_RGB_COLOR_BLUE,
					     0.5,
					     0.5,
					     0,
					     0,
					     EEL_RGB_COLOR_WHITE,
					     NULL);
//	eel_label_make_larger (EEL_LABEL (label[2]), 40);
	
	gtk_table_attach (GTK_TABLE (table), label[0], 0, 1, 0, 1, GTK_FILL, GTK_FILL, 4, 4);
	gtk_table_attach (GTK_TABLE (table), label[1], 1, 2, 18, 19, GTK_FILL, GTK_FILL, 4, 4);
	gtk_table_attach (GTK_TABLE (table), label[2], 0, 1, 1, 2, GTK_FILL, GTK_FILL, 4, 4);

	gtk_widget_show (label[0]);
	gtk_widget_show (label[1]);
	gtk_widget_show (label[2]);
	gtk_widget_show (table);
	gtk_widget_show (viewport);
	gtk_widget_show (scrolled);

	gtk_widget_queue_resize (GTK_WIDGET (label[2])->parent);

	return window;
}

int 
main (int argc, char* argv[])
{
	GtkWidget *label_window;
	GtkWidget *scrolled_label_window;
	GtkWidget *table_label_window;
	
	test_init (&argc, &argv);

	label_window = label_window_new ();
	scrolled_label_window = label_window_new_scrolled ();
	table_label_window = label_window_new_table ();

	if (1) gtk_widget_show (scrolled_label_window);
	if (0) gtk_widget_show (label_window);
	if (0) gtk_widget_show (table_label_window);

	gtk_main ();

	return 0;
}
