#include "test.h"

typedef struct
{
	const char *icon;
	const char *title;
	const char *description;
} Item;

static Item items[] = {
	{ "/gnome/share/pixmaps/nautilus/tahoe/theme_preview.png",
	  "Tahoe",
	  "This theme uses photo-realistic folders."
	},
	{ "/gnome/share/pixmaps/nautilus/crux_teal/theme_preview.png",
	  "Crux-Teal",
	  "A Teal variation of the Crux theme."
	},
	{ "/gnome/share/pixmaps/nautilus/crux_eggplant/theme_preview.png",
	  "Crux-Eggplant",
	  "An Eggplant variation of the Crux theme."
	},
	{ "/gnome/share/pixmaps/nautilus/gnome/theme_preview.png",
	  "GNOME",
	  "This theme uses the classic GNOME icons."
	},
	{ "/gnome/share/pixmaps/nautilus/sierra/theme_preview.png",
	  "Sierra",
	  "Uses manila folders and gray-green backgrounds."
	},
	{ "/gnome/share/pixmaps/nautilus/theme_preview.png",
	  "Default",
	  "This is the default theme for Nautilus."
	}
};

static void
selection_changed_callback (EelImageChooser *image_chooser,
			    gpointer callback_data)
{
	int selected_row_position;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (callback_data));

	selected_row_position = eel_image_chooser_get_selected_row (image_chooser);

	eel_radio_button_group_set_active_index (EEL_RADIO_BUTTON_GROUP (callback_data),
						 selected_row_position);
}

static void
button_group_changed_callback (EelRadioButtonGroup *button_group,
			      gpointer callback_data)
{
	guint selected_radio_button;

	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group));
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (callback_data));
	
	selected_radio_button = eel_radio_button_group_get_active_index (button_group);

	eel_image_chooser_set_selected_row (EEL_IMAGE_CHOOSER (callback_data),
					 selected_radio_button);
}

static void
clear_image_chooser_callback (GtkWidget *button,
			  gpointer callback_data)
{
	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (callback_data));
	
	eel_image_chooser_clear (EEL_IMAGE_CHOOSER (callback_data));
}

static void
clear_button_group_callback (GtkWidget *button,
			     gpointer callback_data)
{
	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (callback_data));

	eel_radio_button_group_clear (EEL_RADIO_BUTTON_GROUP (callback_data));
}

#define NUM_POPULATIONS 1

static void
populate_image_chooser_callback (GtkWidget *button,
			     gpointer callback_data)
{
	guint i;
	guint j;
	GdkPixbuf *pixbuf;

	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (callback_data));

	for (j = 0; j < NUM_POPULATIONS; j++) {
		for (i = 0; i < EEL_N_ELEMENTS (items); i++) {
			pixbuf = gdk_pixbuf_new_from_file (items[i].icon);
			
			eel_image_chooser_insert_row (EEL_IMAGE_CHOOSER (callback_data),
						  pixbuf,
						  items[i].title,
						  items[i].description,
						  g_strdup (items[i].title),
						  g_free);
			
			gdk_pixbuf_unref (pixbuf);
		}
	}
}

static void
populate_button_group_callback (GtkWidget *button,
				gpointer callback_data)
{
	guint i;
	guint j;


	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (callback_data));
	
	for (j = 0; j < NUM_POPULATIONS; j++) {
		for (i = 0; i < EEL_N_ELEMENTS (items); i++) {
			eel_radio_button_group_insert (EEL_RADIO_BUTTON_GROUP (callback_data),
						       items[i].title);
		}
	}
}

static void
show_selected_button_callback (GtkWidget *button,
			       gpointer callback_data)
{
	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (callback_data));

	eel_scrolled_image_chooser_show_selected_row (EEL_IMAGE_CHOOSER (callback_data),
						  GTK_WIDGET (callback_data)->parent->parent);
}

int 
main (int argc, char* argv[])
{
	GtkWidget *window;
	GtkWidget *image_chooser;
	GtkWidget *scrolled_window;
	GtkWidget *button_group;
	GtkWidget *hbox;
	GtkWidget *populate_button;
	GtkWidget *clear_button;
	GtkWidget *show_selected_button;
	
	test_init (&argc, &argv);
	
	window = test_window_new ("Icon List Test", 0);

	eel_smooth_widget_global_set_is_smooth (1);

	hbox = gtk_hbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (window), hbox);

	scrolled_window = eel_scrolled_image_chooser_new (&image_chooser);
	gtk_widget_ensure_style (image_chooser);

	button_group = eel_radio_button_group_new (FALSE);

	gtk_box_pack_start (GTK_BOX (hbox), scrolled_window, FALSE, FALSE, 4);
	gtk_box_pack_start (GTK_BOX (hbox), button_group, FALSE, FALSE, 4);

	populate_button = gtk_button_new_with_label ("Populate");
	clear_button = gtk_button_new_with_label ("Clear");
	show_selected_button = gtk_button_new_with_label ("Show Selected");

	gtk_signal_connect (GTK_OBJECT (clear_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (clear_button_group_callback),
			    button_group);
	gtk_signal_connect (GTK_OBJECT (populate_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (populate_button_group_callback),
			    button_group);
	
	gtk_signal_connect (GTK_OBJECT (clear_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (clear_image_chooser_callback),
			    image_chooser);
	gtk_signal_connect (GTK_OBJECT (populate_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (populate_image_chooser_callback),
			    image_chooser);
	gtk_signal_connect (GTK_OBJECT (show_selected_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (show_selected_button_callback),
			    image_chooser);

	gtk_box_pack_start (GTK_BOX (hbox), populate_button, FALSE, FALSE, 4);
	gtk_box_pack_start (GTK_BOX (hbox), clear_button, FALSE, FALSE, 4);
	gtk_box_pack_start (GTK_BOX (hbox), show_selected_button, FALSE, FALSE, 4);

	populate_image_chooser_callback (populate_button, image_chooser);
	populate_button_group_callback (populate_button, button_group);

	eel_scrolled_image_chooser_set_num_visible_rows (EEL_IMAGE_CHOOSER (image_chooser),
						     scrolled_window,
						     3);

	gtk_signal_connect (GTK_OBJECT (image_chooser),
			    "selection_changed",
			    GTK_SIGNAL_FUNC (selection_changed_callback),
			    button_group);
	
	gtk_signal_connect (GTK_OBJECT (button_group),
			    "changed",
			    GTK_SIGNAL_FUNC (button_group_changed_callback),
			    image_chooser);

	gtk_widget_show_all (window);

// 	gtk_widget_hide (button_group);
// 	gtk_widget_hide (clear_button);
// 	gtk_widget_hide (populate_button);

	gtk_main ();

	return 0;
}
