#include <config.h>

#include <eel/eel-background.h>
#include <eel/eel-font-picker.h>
#include <eel/eel-gdk-extensions.h>
#include <eel/eel-glib-extensions.h>
#include <eel/eel-gtk-extensions.h>
#include <eel/eel-label.h>
#include <eel/eel-scalable-font.h>
#include <eel/eel-string-list.h>
#include <eel/eel-string-picker.h>
#include <eel/eel-string.h>
#include <eel/eel-text-caption.h>
#include <gtk/gtkhscale.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkvbox.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>

static char *widget_get_eel_background_color (GtkWidget  *widget);
static void  widget_set_eel_background_image (GtkWidget  *widget,
						   const char *image_name);
static void  widget_set_eel_background_color (GtkWidget  *widget,
						   const char *color);

static void
red_label_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;
	guint32		  color;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);
	
	color = eel_label_get_text_color (label);

	color = EEL_RGBA_COLOR_PACK ((guchar) adjustment->value,
					  EEL_RGBA_COLOR_GET_G (color),
					  EEL_RGBA_COLOR_GET_B (color),
					  EEL_RGBA_COLOR_GET_A (color));
	
	eel_label_set_text_color (label, color);
}

static void
green_label_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;
	guint32		  color;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);
	
	color = eel_label_get_text_color (label);

	color = EEL_RGBA_COLOR_PACK (EEL_RGBA_COLOR_GET_R (color),
					  (guchar) adjustment->value,
					  EEL_RGBA_COLOR_GET_B (color),
					  EEL_RGBA_COLOR_GET_A (color));
	
	eel_label_set_text_color (label, color);
}

static void
blue_label_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;
	guint32		  color;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);
	
	color = eel_label_get_text_color (label);

	color = EEL_RGBA_COLOR_PACK (EEL_RGBA_COLOR_GET_R (color),
					  EEL_RGBA_COLOR_GET_G (color),
					  (guchar) adjustment->value,
					  EEL_RGBA_COLOR_GET_A (color));
	
	eel_label_set_text_color (label, color);
}

static void
alpha_label_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);

	eel_label_set_text_opacity (EEL_LABEL (label), (guchar) adjustment->value);
}

static void
red_background_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;
	guint32		  current_color;
	char		  *current_color_spec;
	char		  *new_color_spec;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);

	current_color_spec = widget_get_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (label)));

	current_color = eel_parse_rgb_with_white_default (current_color_spec);

	g_free (current_color_spec);

	new_color_spec = g_strdup_printf ("rgb:%04hx/%04hx/%04hx",
					  (guint16) (adjustment->value / 255.0 * 65535.0),
					  (guint16) ((double) EEL_RGBA_COLOR_GET_G (current_color) / 255.0 * 65535.0),
					  (guint16) ((double) EEL_RGBA_COLOR_GET_B (current_color) / 255.0 * 65535.0));

	widget_set_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (label)), new_color_spec);
	
	g_free (new_color_spec);
}

static void
green_background_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;
	guint32		  current_color;
	char		  *current_color_spec;
	char		  *new_color_spec;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);

	current_color_spec = widget_get_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (label)));

	current_color = eel_parse_rgb_with_white_default (current_color_spec);

	g_free (current_color_spec);

	new_color_spec = g_strdup_printf ("rgb:%04hx/%04hx/%04hx",
					  (guint16) ((double) EEL_RGBA_COLOR_GET_R (current_color) / 255.0 * 65535.0),
					  (guint16) (adjustment->value / 255.0 * 65535.0),
					  (guint16) ((double) EEL_RGBA_COLOR_GET_B (current_color) / 255.0 * 65535.0));

	widget_set_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (label)), new_color_spec);
	
	g_free (new_color_spec);
}

static void
blue_background_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;
	guint32		  current_color;
	char		  *current_color_spec;
	char		  *new_color_spec;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));

	label = EEL_LABEL (client_data);

	current_color_spec = widget_get_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (label)));

	current_color = eel_parse_rgb_with_white_default (current_color_spec);

	g_free (current_color_spec);

	new_color_spec = g_strdup_printf ("rgb:%04hx/%04hx/%04hx",
					  (guint16) ((double) EEL_RGBA_COLOR_GET_R (current_color) / 255.0 * 65535.0),
					  (guint16) ((double) EEL_RGBA_COLOR_GET_G (current_color) / 255.0 * 65535.0),
					  (guint16) (adjustment->value / 255.0 * 65535.0));

	widget_set_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (label)), new_color_spec);
	
	g_free (new_color_spec);
}

static void
alpha_background_color_value_changed_callback (GtkAdjustment *adjustment, gpointer client_data)
{
	EelLabel	 *label;

	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
	g_return_if_fail (EEL_IS_LABEL (client_data));
	
	label = EEL_LABEL (client_data);

	eel_label_set_text_opacity (EEL_LABEL (label), (guchar) adjustment->value);
}

static void
text_caption_changed_callback (EelTextCaption *text_caption, gpointer client_data)
{
	EelLabel *label;
 	char *text;

	g_return_if_fail (EEL_IS_TEXT_CAPTION (text_caption));
	g_return_if_fail (EEL_IS_LABEL (client_data));

 	text = eel_text_caption_get_text (text_caption);

	label = EEL_LABEL (client_data);

	eel_label_set_text (EEL_LABEL (label), text);

	g_free (text);
}

static GtkWidget*
create_value_scale (guint min,
		    guint max,
		    guint value,
		    const char *color_spec,
		    GtkSignalFunc callback,
		    gpointer callback_data)
{
	GtkAdjustment	*adjustment;
	GtkWidget	*scale;

	g_assert (max > min);
	g_assert (callback > 0);
	
	adjustment = (GtkAdjustment *) gtk_adjustment_new (value,
							   min,
							   max,
							   1,
							   (max - min) / 10,
							   0);
	
	scale = gtk_hscale_new (adjustment);

	if (color_spec != NULL) {
		eel_gtk_widget_set_background_color (scale, color_spec);
	}

	gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);

	gtk_widget_set_usize (scale, 150, 0);
	
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed", callback, callback_data);
	
	return scale;
}

static GtkWidget*
create_value_scale_caption (const gchar *title,
			    guint min,
			    guint max,
			    guint value,
			    const char *color_spec,
			    GtkSignalFunc callback,
			    gpointer callback_data)
{
	GtkWidget	*hbox;
	GtkWidget	*label;
	GtkWidget	*scale;

	scale = create_value_scale (min, max, value, color_spec, callback, callback_data);
	hbox = gtk_hbox_new (FALSE, 0);
	label = gtk_label_new (title);
	
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);
	gtk_box_pack_end (GTK_BOX (hbox), scale, FALSE, FALSE, 4);

	gtk_widget_show (label);
	gtk_widget_show (scale);

	return hbox;
}

static GtkWidget*
create_color_picker_frame (const char		*title,
			   GtkSignalFunc	red_callback,
			   GtkSignalFunc	green_callback,
			   GtkSignalFunc	blue_callback,
			   GtkSignalFunc	alpha_callback,
			   gpointer		callback_data,
			   guint32		current_color)
{
	GtkWidget *red_scale;
	GtkWidget *green_scale;
	GtkWidget *blue_scale;
	GtkWidget *alpha_scale;
	GtkWidget *frame;
	GtkWidget *vbox;

	g_return_val_if_fail (title != NULL, NULL);
	g_return_val_if_fail (red_callback != NULL, NULL);
	g_return_val_if_fail (green_callback != NULL, NULL);
	g_return_val_if_fail (blue_callback != NULL, NULL);
	g_return_val_if_fail (alpha_callback != NULL, NULL);
	
	frame = gtk_frame_new (title);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);

	red_scale = create_value_scale_caption ("Red",
						0,
						255,
						EEL_RGBA_COLOR_GET_R (current_color),
						"red",
						red_callback,
						callback_data);

	green_scale = create_value_scale_caption ("Green",
						  0,
						  255,
						  EEL_RGBA_COLOR_GET_R (current_color),
						  "green",
						  green_callback,
						  callback_data);
	
	blue_scale = create_value_scale_caption ("Blue",
						 0,
						 255,
						 EEL_RGBA_COLOR_GET_R (current_color),
						 "blue",
						 blue_callback,
						 callback_data);
	
	alpha_scale = create_value_scale_caption ("Alpha",
						  0,
						  255,
						  EEL_RGBA_COLOR_GET_R (current_color),
						  NULL,
						  alpha_callback,
						  callback_data);

	gtk_container_add (GTK_CONTAINER (frame), vbox);

	gtk_box_pack_start (GTK_BOX (vbox), red_scale, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), green_scale, TRUE, TRUE, 1);
	gtk_box_pack_start (GTK_BOX (vbox), blue_scale, TRUE, TRUE, 1);
	gtk_box_pack_end (GTK_BOX (vbox), alpha_scale, TRUE, TRUE, 2);

	gtk_widget_show_all (vbox);

	return frame;
}

static GtkWidget*
create_text_caption_frame (const char		*title,
			   GtkSignalFunc	changed_callback,
			   gpointer		callback_data)
{
	GtkWidget *frame;
	GtkWidget *text_caption;

	g_return_val_if_fail (title != NULL, NULL);
	g_return_val_if_fail (changed_callback != NULL, NULL);
	
	frame = gtk_frame_new (title);

	text_caption = eel_text_caption_new ();
	gtk_container_set_border_width (GTK_CONTAINER (text_caption), 6);

 	eel_caption_set_show_title (EEL_CAPTION (text_caption), FALSE);
	eel_caption_set_title_label (EEL_CAPTION (text_caption), title);

	gtk_signal_connect (GTK_OBJECT (text_caption), "changed", changed_callback, callback_data);

	gtk_container_add (GTK_CONTAINER (frame), text_caption);

	gtk_widget_show (text_caption);

	return frame;
}

static void
widget_set_eel_background_image (GtkWidget *widget, const char *image_name)
{
	EelBackground	*background;
	char		*background_path;
	char		*background_uri;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (image_name != NULL);

	background = eel_get_widget_background (widget);
	
	background_path = g_strconcat (EEL_DATADIR "/patterns/", image_name, NULL);
	background_uri = gnome_vfs_get_uri_from_local_path (background_path);
	g_free (background_path);

	eel_background_reset (background);
	eel_background_set_image_uri (background, background_uri);

	g_free (background_uri);
}

static void
widget_set_eel_background_color (GtkWidget *widget, const char *color)
{
	EelBackground	*background;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (color != NULL);

	background = eel_get_widget_background (widget);
	
	eel_background_reset (background);
	eel_background_set_color (background, color);
}

static char *
widget_get_eel_background_color (GtkWidget *widget)
{
	EelBackground *background;
	
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	background = eel_get_widget_background (widget);

	return eel_background_get_color (background);
}

static void
widget_set_background_reset (GtkWidget *widget)
{
	EelBackground	*background;

	g_return_if_fail (GTK_IS_WIDGET (widget));

	background = eel_get_widget_background (widget);

	eel_background_reset (background);
}

static void
background_changed_callback (EelStringPicker *string_picker, gpointer client_data)
{
 	char *string;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (EEL_IS_LABEL (client_data));

 	string = eel_string_picker_get_selected_string (string_picker);

	if (eel_str_has_prefix (string, "Image - ")) {
		widget_set_eel_background_image (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (client_data)),
					     string + strlen ("Image - "));
	}
	else if (eel_str_has_prefix (string, "Gradient - ")) {
		widget_set_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (client_data)),
					     string + strlen ("Gradient - "));
	}
	else if (eel_str_has_prefix (string, "Solid - ")) {
		widget_set_eel_background_color (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (client_data)),
					     string + strlen ("Solid - "));
	}
	else if (eel_str_has_prefix (string, "Reset")) {
		widget_set_background_reset (eel_gtk_widget_find_windowed_ancestor (GTK_WIDGET (client_data)));
	}

	g_free (string);
}

static void
justification_changed_callback (EelStringPicker *string_picker, gpointer client_data)
{
	GtkJustification justification;
	char *string;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (EEL_IS_LABEL (client_data));

 	string = eel_string_picker_get_selected_string (string_picker);

	if (eel_str_is_equal (string, "Left")) {
		justification = GTK_JUSTIFY_LEFT;
	} else if (eel_str_has_prefix (string, "Center")) {
		justification = GTK_JUSTIFY_CENTER;
	} else if (eel_str_has_prefix (string, "Right")) {
		justification = GTK_JUSTIFY_RIGHT;
	} else {
		g_assert_not_reached ();
		justification = GTK_JUSTIFY_LEFT;
	}

	eel_label_set_justify (EEL_LABEL (client_data), justification);
	
	g_free (string);
}

static void
drop_shadow_offset_changed_callback (EelStringPicker *string_picker, gpointer client_data)
{
	char *string;
	int   drop_shadow_offset;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (EEL_IS_LABEL (client_data));

 	string = eel_string_picker_get_selected_string (string_picker);

	if (eel_eat_str_to_int (string, &drop_shadow_offset)) {
		eel_label_set_smooth_drop_shadow_offset (EEL_LABEL (client_data), drop_shadow_offset);
	}

	g_free (string);
}

static GtkWidget*
create_background_frame (const char	*title,
			 GtkSignalFunc	background_changed_callback,
			 gpointer	callback_data)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *background_picker;

	g_return_val_if_fail (title != NULL, NULL);
 	g_return_val_if_fail (background_changed_callback != NULL, NULL);
	
	vbox = gtk_vbox_new (FALSE, 0);
	frame = gtk_frame_new (title);

	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	background_picker = eel_string_picker_new ();
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Image - pale_coins.png");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Image - bubbles.png");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Image - irish_spring.png");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Image - white_ribs.png");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "-----------------------");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Gradient - rgb:bbbb/bbbb/eeee-rgb:ffff/ffff/ffff:h");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Gradient - rgb:bbbb/bbbb/eeee-rgb:ffff/ffff/ffff");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "-----------------------");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Solid - rgb:bbbb/bbbb/eeee");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "-----------------------");
	eel_string_picker_insert_string (EEL_STRING_PICKER (background_picker), "Reset");

 	eel_caption_set_show_title (EEL_CAPTION (background_picker), FALSE);

	gtk_signal_connect (GTK_OBJECT (background_picker), "changed", background_changed_callback, callback_data);

	gtk_box_pack_start (GTK_BOX (vbox), background_picker, FALSE, FALSE, 0);

	gtk_widget_show_all (vbox);

	return frame;
}

static GtkWidget*
create_justification_frame (const char		*title,
			    GtkSignalFunc	justification_changed_callback,
			    gpointer		callback_data)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *justification_picker;

	g_return_val_if_fail (title != NULL, NULL);
 	g_return_val_if_fail (justification_changed_callback != NULL, NULL);
	
	vbox = gtk_vbox_new (FALSE, 0);
	frame = gtk_frame_new (title);

	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	justification_picker = eel_string_picker_new ();
	eel_string_picker_insert_string (EEL_STRING_PICKER (justification_picker), "Left");
	eel_string_picker_insert_string (EEL_STRING_PICKER (justification_picker), "Center");
	eel_string_picker_insert_string (EEL_STRING_PICKER (justification_picker), "Right");

 	eel_caption_set_show_title (EEL_CAPTION (justification_picker), FALSE);

	gtk_signal_connect (GTK_OBJECT (justification_picker), "changed", justification_changed_callback, callback_data);

	gtk_box_pack_start (GTK_BOX (vbox), justification_picker, FALSE, FALSE, 0);

	gtk_widget_show_all (vbox);

	return frame;
}

static GtkWidget*
create_drop_shadow_offset_frame (const char	*title,
				 GtkSignalFunc	drop_shadow_changed_callback,
				 gpointer	callback_data)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *drop_shadow_offset_picker;

	g_return_val_if_fail (title != NULL, NULL);
 	g_return_val_if_fail (drop_shadow_changed_callback != NULL, NULL);
	
	vbox = gtk_vbox_new (FALSE, 0);
	frame = gtk_frame_new (title);

	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	drop_shadow_offset_picker = eel_string_picker_new ();
	eel_string_picker_insert_string (EEL_STRING_PICKER (drop_shadow_offset_picker), "0");
	eel_string_picker_insert_string (EEL_STRING_PICKER (drop_shadow_offset_picker), "1");
	eel_string_picker_insert_string (EEL_STRING_PICKER (drop_shadow_offset_picker), "2");
	eel_string_picker_insert_string (EEL_STRING_PICKER (drop_shadow_offset_picker), "3");
	eel_string_picker_insert_string (EEL_STRING_PICKER (drop_shadow_offset_picker), "4");
	eel_string_picker_insert_string (EEL_STRING_PICKER (drop_shadow_offset_picker), "5");

 	eel_caption_set_show_title (EEL_CAPTION (drop_shadow_offset_picker), FALSE);

	gtk_signal_connect (GTK_OBJECT (drop_shadow_offset_picker), "changed", drop_shadow_changed_callback, callback_data);

	gtk_box_pack_start (GTK_BOX (vbox), drop_shadow_offset_picker, FALSE, FALSE, 0);

	gtk_widget_show_all (vbox);

	return frame;
}

static void
delete_event (GtkWidget *widget, GdkEvent *event, gpointer callback_data)
{
	gtk_main_quit ();
}

int 
main (int argc, char* argv[])
{
	GtkWidget		*window;
	GtkWidget		*main_box;
	GtkWidget		*bottom_box;
	GtkWidget		*tool_box1;
	GtkWidget		*tool_box2;
	GtkWidget		*color_tool_box;
	GtkWidget		*label;
	GtkWidget		*label_color_picker_frame;
	GtkWidget		*background_color_picker_frame;
	GtkWidget		*text_caption_frame;
	GtkWidget		*background_frame;
	GtkWidget		*justification_frame;
	GtkWidget		*drop_shadow_offset_frame;
	GtkWidget		*middle_box;

	gtk_init (&argc, &argv);
	gdk_rgb_init ();
	gnome_vfs_init ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (delete_event), NULL);
	gtk_window_set_title (GTK_WINDOW (window), "Label Test");
	gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	main_box = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), main_box);

 	label = eel_label_new ("Foo");

	bottom_box = gtk_vbox_new (FALSE, 4);

	tool_box1 = gtk_hbox_new (FALSE, 0);
	tool_box2 = gtk_hbox_new (FALSE, 0);

	color_tool_box = gtk_hbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (bottom_box), tool_box1, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (bottom_box), tool_box2, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (main_box), label, TRUE, TRUE, 10);
	gtk_box_pack_end (GTK_BOX (main_box), bottom_box, TRUE, TRUE, 10);

	widget_set_eel_background_image (eel_gtk_widget_find_windowed_ancestor (label), "pale_coins.png");
	
	label_color_picker_frame = create_color_picker_frame ("Label Color",
							      red_label_color_value_changed_callback,
							      green_label_color_value_changed_callback,
							      blue_label_color_value_changed_callback,
							      alpha_label_color_value_changed_callback,
							      label,
							      eel_label_get_text_color (EEL_LABEL (label)));

	background_color_picker_frame = create_color_picker_frame ("Background Color",
								   red_background_color_value_changed_callback,
								   green_background_color_value_changed_callback,
								   blue_background_color_value_changed_callback,
								   alpha_background_color_value_changed_callback,
								   label,
								   eel_label_get_text_color (EEL_LABEL (label)));

	text_caption_frame = create_text_caption_frame ("Text",
							text_caption_changed_callback,
							label);
	
	background_frame = create_background_frame ("Background",
						    background_changed_callback,
						    label);

	justification_frame = create_justification_frame ("Justification",
							  justification_changed_callback,
							  label);

	drop_shadow_offset_frame = create_drop_shadow_offset_frame ("Drop Shadow Offset",
								    drop_shadow_offset_changed_callback,
								    label);
	
	middle_box = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (middle_box), background_frame, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (middle_box), drop_shadow_offset_frame, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (middle_box), justification_frame, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (color_tool_box), label_color_picker_frame, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (color_tool_box), middle_box, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (color_tool_box), background_color_picker_frame, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (tool_box1), color_tool_box, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (tool_box2), text_caption_frame, TRUE, TRUE, 0);

	gtk_widget_show_all (window);

	gtk_main ();

	gnome_vfs_shutdown ();
	
	return 0;
}
