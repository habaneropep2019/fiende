#include "test.h"
#include "dumb-box.h"

static const char *state_names[] = {
	"Normal",
	"Active",
	"Prelight",
	"Selected",
	"Insensitive"
};
#define NUM_STATES EEL_N_ELEMENTS (state_names)

static const char *color_names[] = {
	"fg",
	"bg",
	"light",
	"dark",
	"mid",
	"text",
	"base"
};
#define NUM_COLORS EEL_N_ELEMENTS (color_names)

static const char *gc_names[] = {
	"fg_gc",
	"bg_gc",
	"light_gc",
	"dark_gc",
	"mid_gc",
	"text_gc",
	"base_gc"
};
#define NUM_GCS EEL_N_ELEMENTS (gc_names)

static const char *pixmap_names[] = {
	"bg_pixmaps"
};
#define NUM_PIXMAPS EEL_N_ELEMENTS (pixmap_names)

static GdkColor
style_get_color (const GtkStyle *style,
		 guint n,
		 GtkStateType state)
{
	GdkColor empty;
	
	empty.red = 0;
	empty.green = 0;
	empty.blue = 0;
	empty.pixel = 0;

	g_return_val_if_fail (style != NULL, empty);
	g_return_val_if_fail (n >= 0, empty);
	g_return_val_if_fail (n < NUM_COLORS, empty);
	g_return_val_if_fail (state >= GTK_STATE_NORMAL, empty);
	g_return_val_if_fail (state <= GTK_STATE_INSENSITIVE, empty);

	switch (n) {
	case 0: return style->fg[state]; break;
	case 1: return style->bg[state]; break;
	case 2: return style->light[state]; break;
	case 3: return style->dark[state]; break;
	case 4: return style->mid[state]; break;
	case 5: return style->text[state]; break;
	case 6: return style->base[state]; break;
	default:
	}
	g_assert_not_reached ();
	return empty;
}

static GdkGC *
style_get_gc (const GtkStyle *style,
	      guint n,
	      GtkStateType state)
{
	g_return_val_if_fail (style != NULL, NULL);
	g_return_val_if_fail (n >= 0, NULL);
	g_return_val_if_fail (n < NUM_COLORS, NULL);
	g_return_val_if_fail (state >= GTK_STATE_NORMAL, NULL);
	g_return_val_if_fail (state <= GTK_STATE_INSENSITIVE, NULL);

	switch (n) {
	case 0: return style->fg_gc[state]; break;
	case 1: return style->bg_gc[state]; break;
	case 2: return style->light_gc[state]; break;
	case 3: return style->dark_gc[state]; break;
	case 4: return style->mid_gc[state]; break;
	case 5: return style->text_gc[state]; break;
	case 6: return style->base_gc[state]; break;
	default:
	}
	g_assert_not_reached ();
	return NULL;
}

static GdkPixmap *
style_get_pixmap (const GtkStyle *style,
		  GtkStateType state)
{
	g_return_val_if_fail (style != NULL, NULL);
	g_return_val_if_fail (state >= GTK_STATE_NORMAL, NULL);
	g_return_val_if_fail (state <= GTK_STATE_INSENSITIVE, NULL);

	return style->bg_pixmap[state];
}

static int
color_box_expose_event (GtkWidget *widget,
			GdkEventExpose *event,
			gpointer callback_data)
{
	guint col;
	guint row;
	GtkWidget *child;
	ArtIRect dirty_area;
	GdkColor color;
	GdkGC *gc;

 	g_return_val_if_fail (GTK_IS_DUMB_BOX (widget), FALSE);
 	g_return_val_if_fail (event != NULL, FALSE);

	col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "col"));
	row = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "row"));

	child = GTK_BIN (widget)->child;

	color = style_get_color (widget->style, row, col);

	gc = gdk_gc_new (widget->window);
	gdk_gc_set_function (gc, GDK_COPY);	
	gdk_gc_set_foreground (gc, &color);

	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);

	gdk_draw_rectangle (widget->window,
			    gc,
			    TRUE,
			    dirty_area.x0,
			    dirty_area.y0,
			    eel_art_irect_get_width (dirty_area),
			    eel_art_irect_get_height (dirty_area));

	gdk_gc_unref (gc);

	return TRUE;
}

static int
gc_box_expose_event (GtkWidget *widget,
		     GdkEventExpose *event,
		     gpointer callback_data)
{
	guint col;
	guint row;
	GtkWidget *child;
	ArtIRect dirty_area;
	GdkGC *gc;

 	g_return_val_if_fail (GTK_IS_DUMB_BOX (widget), FALSE);
 	g_return_val_if_fail (event != NULL, FALSE);

	col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "col"));
	row = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "row"));

	child = GTK_BIN (widget)->child;

	gc = style_get_gc (widget->style, row, col);
	g_return_val_if_fail (gc != NULL, FALSE);

	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);
	gdk_draw_rectangle (widget->window,
			    gc,
			    TRUE,
			    dirty_area.x0,
			    dirty_area.y0,
			    eel_art_irect_get_width (dirty_area),
			    eel_art_irect_get_height (dirty_area));
	return TRUE;
}

static int
pixmap_box_expose_event (GtkWidget *widget,
			 GdkEventExpose *event,
			 gpointer callback_data)
{
	guint col;
	guint row;
	GtkWidget *child;
	ArtIRect dirty_area;
	GdkPixmap *pixmap;
	GdkPixbuf *pixbuf;
	int pixmap_width;
	int pixmap_height;
	GdkGC *gc;

 	g_return_val_if_fail (GTK_IS_DUMB_BOX (widget), FALSE);
 	g_return_val_if_fail (event != NULL, FALSE);

	col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "col"));
	row = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), "row"));

	child = GTK_BIN (widget)->child;

	pixmap = style_get_pixmap (widget->style, col);

	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);

	if (pixmap == NULL) {
		eel_debug_draw_rectangle_and_cross (widget->window,
						    eel_art_irect_offset_to (eel_gtk_widget_get_bounds (widget), 0, 0),
						    0xFF0000,
						    TRUE);
		return TRUE;
	}

	gdk_window_get_size (pixmap, &pixmap_width, &pixmap_height);

	g_return_val_if_fail (pixmap_width > 0, TRUE);
	g_return_val_if_fail (pixmap_height > 0, TRUE);

	pixbuf = gdk_pixbuf_get_from_drawable (NULL,
					       pixmap,
					       gdk_rgb_get_cmap (),
					       0, 0,
					       0, 0, 
					       pixmap_width,
					       pixmap_height);

	g_return_val_if_fail (pixbuf != NULL, TRUE);

	gc = gdk_gc_new (widget->window);
	gdk_gc_set_function (gc, GDK_COPY);	

	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);
	eel_gdk_pixbuf_draw_to_drawable_tiled (pixbuf,
					       widget->window,
					       gc,
					       dirty_area,
					       pixmap_width,
					       pixmap_height,
					       0,
					       0,
					       GDK_RGB_DITHER_NONE,
					       GDK_PIXBUF_ALPHA_BILEVEL,
					       128);
	gdk_gc_unref (gc);
	gdk_pixbuf_unref (pixbuf);

	eel_debug_draw_rectangle_and_cross (widget->window,
					    eel_art_irect_offset_to (eel_gtk_widget_get_bounds (widget), 0, 0),
					    0x000000,
					    FALSE);

	return TRUE;
}

int 
main (int argc, char* argv[])
{
	GtkWidget *window;
	GtkWidget *vbox;
	guint i;
	guint j;

	GtkWidget *colors_table;
	GtkWidget *colors_frame;
	GtkWidget *colors_labels[NUM_STATES + 1][NUM_COLORS + 1];
	GtkWidget *colors_boxes[NUM_STATES + 1][NUM_COLORS + 1];

	GtkWidget *gcs_table;
	GtkWidget *gcs_frame;
	GtkWidget *gcs_labels[NUM_STATES + 1][NUM_GCS + 1];
	GtkWidget *gcs_boxes[NUM_STATES + 1][NUM_GCS + 1];

	GtkWidget *pixmaps_table;
	GtkWidget *pixmaps_frame;
	GtkWidget *pixmaps_labels[NUM_STATES + 1][NUM_PIXMAPS + 1];
	GtkWidget *pixmaps_boxes[NUM_STATES + 1][NUM_PIXMAPS + 1];

	test_init (&argc, &argv);

	window = test_window_new ("Gtk Style Tester", 20);

	vbox = gtk_vbox_new (FALSE, 0);

	gtk_container_add (GTK_CONTAINER (window), vbox);


	/* Colors table */
	colors_frame = gtk_frame_new ("Colors");
	colors_table = gtk_table_new (NUM_COLORS, NUM_STATES, TRUE);
	gtk_container_add (GTK_CONTAINER (colors_frame), colors_table);
	gtk_box_pack_start (GTK_BOX (vbox), colors_frame, TRUE, TRUE, 0);

	/* Column titles */
	for (i = 0; i < NUM_STATES; i++) {
		colors_labels[i][0] = gtk_label_new (state_names[i]);
		
		gtk_table_attach (GTK_TABLE (colors_table),
				  colors_labels[i][0],
				  i + 1,
				  i + 2,
				  0,
				  1,
				  GTK_EXPAND | GTK_FILL,
				  GTK_EXPAND | GTK_FILL,
				  10,
				  10);

		gtk_misc_set_alignment (GTK_MISC (colors_labels[i][0]), 1.0, 1.0);

		gtk_widget_show (colors_labels[i][0]);
	}

	/* Row titles */
	for (j = 0; j < NUM_COLORS; j++) {
		colors_labels[0][j] = gtk_label_new (color_names[j]);
		
		gtk_table_attach (GTK_TABLE (colors_table),
				  colors_labels[0][j],
				  0,
				  1,
				  j + 1,
				  j + 2,
				  GTK_EXPAND | GTK_FILL,
				  GTK_EXPAND | GTK_FILL,
				  10,
				  10);

		gtk_misc_set_alignment (GTK_MISC (colors_labels[0][j]), 1.0, 0.5);

		gtk_widget_show (colors_labels[0][j]);
	}

	for (j = 0; j < NUM_COLORS; j++) {
		for (i = 0; i < NUM_STATES; i++) {
			colors_boxes[i][j] = eel_dumb_box_new ();
			gtk_widget_ensure_style (colors_boxes[i][j]);

			gtk_signal_connect (GTK_OBJECT (colors_boxes[i][j]),
					    "expose_event",
					    GTK_SIGNAL_FUNC (color_box_expose_event),
					    NULL);

			gtk_object_set_data (GTK_OBJECT (colors_boxes[i][j]),
					     "col",
					     GINT_TO_POINTER (i));

			gtk_object_set_data (GTK_OBJECT (colors_boxes[i][j]),
					     "row",
					     GINT_TO_POINTER (j));

			gtk_table_attach (GTK_TABLE (colors_table),
					  colors_boxes[i][j],
					  i + 1,
					  i + 2,
					  j + 1,
					  j + 2,
					  GTK_EXPAND | GTK_FILL,
					  GTK_EXPAND | GTK_FILL,
					  4,
					  4);

			gtk_widget_show_all (colors_boxes[i][j]);
		}
	}
	gtk_widget_show (colors_frame);
	gtk_widget_show (colors_table);



	/* GCs table */
	gcs_frame = gtk_frame_new ("Gcs");
	gcs_table = gtk_table_new (NUM_GCS, NUM_STATES, TRUE);
	gtk_container_add (GTK_CONTAINER (gcs_frame), gcs_table);
	gtk_box_pack_start (GTK_BOX (vbox), gcs_frame, TRUE, TRUE, 0);

	/* Column titles */
	for (i = 0; i < NUM_STATES; i++) {
		gcs_labels[i][0] = gtk_label_new (state_names[i]);
		
		gtk_table_attach (GTK_TABLE (gcs_table),
				  gcs_labels[i][0],
				  i + 1,
				  i + 2,
				  0,
				  1,
				  GTK_EXPAND | GTK_FILL,
				  GTK_EXPAND | GTK_FILL,
				  10,
				  10);

		gtk_misc_set_alignment (GTK_MISC (gcs_labels[i][0]), 1.0, 1.0);

		gtk_widget_show (gcs_labels[i][0]);
	}

	/* Row titles */
	for (j = 0; j < NUM_GCS; j++) {
		gcs_labels[0][j] = gtk_label_new (gc_names[j]);
		
		gtk_table_attach (GTK_TABLE (gcs_table),
				  gcs_labels[0][j],
				  0,
				  1,
				  j + 1,
				  j + 2,
				  GTK_EXPAND | GTK_FILL,
				  GTK_EXPAND | GTK_FILL,
				  10,
				  10);

		gtk_misc_set_alignment (GTK_MISC (gcs_labels[0][j]), 1.0, 0.5);

		gtk_widget_show (gcs_labels[0][j]);
	}

	for (j = 0; j < NUM_GCS; j++) {
		for (i = 0; i < NUM_STATES; i++) {
			gcs_boxes[i][j] = eel_dumb_box_new ();
			gtk_widget_ensure_style (gcs_boxes[i][j]);
			
			gtk_signal_connect (GTK_OBJECT (gcs_boxes[i][j]),
					    "expose_event",
					    GTK_SIGNAL_FUNC (gc_box_expose_event),
					    NULL);

			gtk_object_set_data (GTK_OBJECT (gcs_boxes[i][j]),
					     "col",
					     GINT_TO_POINTER (i));

			gtk_object_set_data (GTK_OBJECT (gcs_boxes[i][j]),
					     "row",
					     GINT_TO_POINTER (j));

			gtk_table_attach (GTK_TABLE (gcs_table),
					  gcs_boxes[i][j],
					  i + 1,
					  i + 2,
					  j + 1,
					  j + 2,
					  GTK_EXPAND | GTK_FILL,
					  GTK_EXPAND | GTK_FILL,
					  4,
					  4);

			gtk_widget_show_all (gcs_boxes[i][j]);
		}
	}
	gtk_widget_show (gcs_frame);
	gtk_widget_show (gcs_table);


	/* Pixmaps table */
	pixmaps_frame = gtk_frame_new ("Pixmaps");
	pixmaps_table = gtk_table_new (NUM_PIXMAPS, NUM_STATES, TRUE);
	gtk_container_add (GTK_CONTAINER (pixmaps_frame), pixmaps_table);
	gtk_box_pack_start (GTK_BOX (vbox), pixmaps_frame, TRUE, TRUE, 0);

	/* Column titles */
	for (i = 0; i < NUM_STATES; i++) {
		pixmaps_labels[i][0] = gtk_label_new (state_names[i]);
		
		gtk_table_attach (GTK_TABLE (pixmaps_table),
				  pixmaps_labels[i][0],
				  i + 1,
				  i + 2,
				  0,
				  1,
				  GTK_EXPAND | GTK_FILL,
				  GTK_EXPAND | GTK_FILL,
				  10,
				  10);

		gtk_misc_set_alignment (GTK_MISC (pixmaps_labels[i][0]), 1.0, 1.0);

		gtk_widget_show (pixmaps_labels[i][0]);
	}

	/* Row titles */
	for (j = 0; j < NUM_PIXMAPS; j++) {
		pixmaps_labels[0][j] = gtk_label_new (pixmap_names[j]);
		
		gtk_table_attach (GTK_TABLE (pixmaps_table),
				  pixmaps_labels[0][j],
				  0,
				  1,
				  j + 1,
				  j + 2,
				  GTK_EXPAND | GTK_FILL,
				  GTK_EXPAND | GTK_FILL,
				  10,
				  10);

		gtk_misc_set_alignment (GTK_MISC (pixmaps_labels[0][j]), 1.0, 0.5);

		gtk_widget_show (pixmaps_labels[0][j]);
	}

	for (j = 0; j < NUM_PIXMAPS; j++) {
		for (i = 0; i < NUM_STATES; i++) {
			pixmaps_boxes[i][j] = eel_dumb_box_new ();
			gtk_widget_ensure_style (pixmaps_boxes[i][j]);

			gtk_signal_connect (GTK_OBJECT (pixmaps_boxes[i][j]),
					    "expose_event",
					    GTK_SIGNAL_FUNC (pixmap_box_expose_event),
					    NULL);

			gtk_object_set_data (GTK_OBJECT (pixmaps_boxes[i][j]),
					     "col",
					     GINT_TO_POINTER (i));

			gtk_object_set_data (GTK_OBJECT (pixmaps_boxes[i][j]),
					     "row",
					     GINT_TO_POINTER (j));

			gtk_table_attach (GTK_TABLE (pixmaps_table),
					  pixmaps_boxes[i][j],
					  i + 1,
					  i + 2,
					  j + 1,
					  j + 2,
					  GTK_EXPAND | GTK_FILL,
					  GTK_EXPAND | GTK_FILL,
					  4,
					  4);

			gtk_widget_show_all (pixmaps_boxes[i][j]);
		}
	}
	
	gtk_widget_show (pixmaps_frame);
	gtk_widget_show (pixmaps_table);



	gtk_widget_show (vbox);
	gtk_widget_show (window);

	gtk_main ();

	return test_quit (EXIT_SUCCESS);
}
