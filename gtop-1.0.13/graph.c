#include <graph.h>
#include <gtop-graph.h>

GdkColor graph_default_colors [GRAPH_DEFAULT_COLORS] = {
	{0, 0x8fff, 0xefff, 0x8fff},
	{0, 0x8fff, 0xafff, 0xefff},
	{0, 0xefff, 0xafff, 0xafff},
	{0, 0xefff, 0xefff, 0x5fff},
};

static const gchar *default_fonts [5] = {
	"-*-*-medium-r-*-*-10-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-12-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-14-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-18-*-*-*-*-*-*-*",
	"-*-*-medium-r-*-*-24-*-*-*-*-*-*-*"
};

static void	graph_expose (GtkWidget *, GdkEventExpose *, Graph *);
static void	graph_size_request (GtkWidget *, GtkRequisition *, Graph *);
static gint	graph_configure (GtkWidget *, GdkEventConfigure *, Graph *);
static void	graph_realize (GtkWidget *, Graph *);
static void	graph_unrealize (GtkWidget *, Graph *);

static GtkWidget *graph_properties_init (GnomePropertyObject *);
static void	graph_properties_apply (GnomePropertyObject *);
static void	graph_properties_update (GnomePropertyObject *);
static void	graph_properties_load (GnomePropertyObject *);
static void	graph_properties_save (GnomePropertyObject *);

static void	graph_properties_changed (GnomePropertyObject *);

static void	graph_colors_set (Graph *);

GnomePropertyDescriptor GraphProperty_Descriptor = {
	sizeof (GTopGraphProperties),
	N_("Graph"),
	graph_properties_init,
	graph_properties_apply,
	graph_properties_update,
	graph_properties_load,
	graph_properties_save,
	NULL, NULL, NULL,
	graph_properties_changed,
	NULL
};

static GList *widget_list = NULL;

enum {
	DEFAULT_WIDTH = 0,
	DEFAULT_HEIGHT,
	HORIZONTAL_BORDER,
	VERTICAL_BORDER,
	LINE_WIDTH,
	PAD_WIDTH,
	GRAPH_GRAPH_WIDTH,
	GRAPH_EXTRA_HEIGHT
};

enum {
	CP_FOREGROUND = 0,
	CP_BACKGROUND
};

static GtkAdjustment *adjustments [8];

static void
graph_expose (GtkWidget *w, GdkEventExpose *e, Graph *g)
{
	if (g->bs)
		gdk_draw_pixmap (w->window,
				 w->style->fg_gc [GTK_WIDGET_STATE (w)],
				 g->bs,
				 e->area.x, e->area.y,
				 e->area.x, e->area.y,
				 e->area.width, e->area.height);
	else
		gdk_draw_rectangle (w->window,
				    w->style->bg_gc [GTK_WIDGET_STATE (w)],
				    TRUE,
				    e->area.x, e->area.y,
				    e->area.width, e->area.height);
}

static gint
graph_configure (GtkWidget *w, GdkEventConfigure *event, Graph *g)
{
	g->width = event->width;
	g->height = event->height;

	if (g->bs) {
		gdk_pixmap_unref (g->bs);
		g->bs = NULL;
	}

	g->bs = gdk_pixmap_new (g->da->window, g->width, g->height, -1);

	gdk_draw_rectangle (g->bs, w->style->bg_gc [GTK_WIDGET_STATE (w)],
			    TRUE, 0, 0, g->width, g->height);

	graph_update (g);
	
	return FALSE;
}

static void
graph_size_request (GtkWidget *widget,
		    GtkRequisition *requisition, Graph *g)
{
	requisition->width  = g->minimum_width;
	requisition->height = g->minimum_height;
}

static void
graph_realize (GtkWidget *widget, Graph *g)
{
	if (!g->colors_allocated)
		graph_colors_set (g);
}

static void
graph_unrealize (GtkWidget *widget, Graph *g)
{
	g->colors_allocated = 0;
}

Graph *
graph_new (GtkWidget *da, GraphDataFunc d_fn, gpointer user_data)
{
	Graph *g = g_new0 (Graph, 1);

	memset (g, 0, sizeof (Graph));

	g->data_fn = d_fn;
	g->bs = NULL;
	g->n = 0;
	g->minimum_height = 0;
	g->height = g->width = 0;
	g->colors_allocated = 0;

	g->user_data = user_data;

	g->da = da;

	gtk_widget_set_events (g->da, GDK_EXPOSURE_MASK);

	gtk_signal_connect (GTK_OBJECT (g->da), "size_request",
			    (GtkSignalFunc) graph_size_request, (gpointer) g);
	gtk_signal_connect (GTK_OBJECT (g->da), "expose_event",
			    (GtkSignalFunc) graph_expose, (gpointer) g);
	gtk_signal_connect (GTK_OBJECT (g->da), "configure_event",
			    (GtkSignalFunc) graph_configure, (gpointer) g);
	gtk_signal_connect (GTK_OBJECT (g->da), "realize",
			    (GtkSignalFunc) graph_realize, (gpointer) g);
	gtk_signal_connect (GTK_OBJECT (g->da), "unrealize",
			    (GtkSignalFunc) graph_unrealize, (gpointer) g);

	graph_colors_set (g);

	widget_list = g_list_append (widget_list, g);

	return g;
}

void
graph_destroy (Graph *g)
{
	widget_list = g_list_remove (widget_list, g);
}

#define GRAPH_WIDTH        (gtop_properties.graph.graph_width)
#define GRAPH_LINE_WIDTH   (gtop_properties.graph.line_width)
#define GRAPH_PAD_WIDTH    (gtop_properties.graph.pad_width)

#define GRAPH_PAD_X        (gtop_properties.graph.horizontal_border)
#define GRAPH_PAD_Y        (gtop_properties.graph.vertical_border)

static void
graph_process_data (Graph *g)
{
	gint w ,h, draw;
	gpointer data;
	unsigned total;
	gint ll, mll = 0;
	gint th, gh=0, cgh;
	gint i, c = 0;
	GdkFont *graph_font;
	GdkGC **tgcs, *text_gc;
	gint lines, fh, asc2, ht;
	gchar *head, *tail;
	gint bonus, w_bonus;
	gint extra_height;

	lines = total = 0;
	w = h = 10;

	if (!g->da || !GTK_WIDGET_REALIZED (g->da))
		return;

	gtk_widget_ensure_style (g->da);

	graph_font = gtop_properties.graph.font ?
		gtop_properties.graph.font : g->da->style->font;
	
	extra_height = gtop_properties.graph.extra_height;

	if (!g->colors_allocated) {
		GdkColormap *cmap;
		GtkStyle *style;

		cmap = gdk_window_get_colormap (g->da->window);
		for (i = 0; i < g->n; i++)
			gdk_color_alloc (cmap, &g->colors [i]);

		gdk_color_alloc (cmap, g->foreground);
		gdk_color_alloc (cmap, g->background);

		style = gtk_style_copy (gtk_widget_get_style (g->da));

		for (i=0; i<5; i++)
			memcpy (&(style->bg [i]),
				g->background,
				sizeof (GdkColor));

		gtk_widget_set_style (g->da, style);

		g->colors_allocated = 1;
	}

	tgcs = g_new0 (GdkGC *, g->n);
	for (i = 0; i < g->n; i++) {
		tgcs [i] = gdk_gc_new (g->da->window);

		gdk_gc_copy (tgcs [i], g->da->style->text_gc [0]);
		gdk_gc_set_foreground (tgcs [i], &g->colors [i]);
	}

	text_gc = gdk_gc_new (g->da->window);
	gdk_gc_copy (text_gc, g->da->style->white_gc);
	gdk_gc_set_foreground (text_gc, g->foreground);

	th = graph_font->ascent;
	fh = (graph_font->ascent + graph_font->descent);
	asc2 = (th >> 1) - (th&1);
	
	head = g->data_fn (g, GRAPH_HEAD, NULL, NULL);
	tail = g->data_fn (g, GRAPH_TAIL, NULL, NULL);

	/*
	 * two passes
	 *   1. compute some values
	 *   2. draw
	 *
	 */

	bonus = w_bonus = 0; /* keep gcc happy */

	for (draw = 0; draw < 2; draw++) {
		if (draw) {

			gint fh2 = fh + GRAPH_PAD_Y;

			ht = 0;

			ht += (head) ? fh2 : 0;
			ht += (tail) ? fh2 : 0;
			
			ht += (GRAPH_PAD_Y << 1);

			/* bonus - lines to fit to minimal height */
			bonus = g->height - (h + ht);
			bonus = (bonus > 0) ? bonus : 0;

			g->minimum_height = h + ht + extra_height;

			h += bonus;

			gdk_draw_rectangle
				(g->bs,
				 g->da->style->bg_gc [GTK_WIDGET_STATE (g->da)],
				 TRUE, 0, 0,
				 w + w_bonus + (GRAPH_PAD_X << 1),
				 h + ht + bonus);

			h -= lines;

			if (head) {

				gint x, head_len;

				head_len = gdk_string_width
					(graph_font, tail) +
					(GRAPH_PAD_WIDTH << 1);

				if (g->minimum_width < head_len)
					g->minimum_width = head_len;

				x = (w + w_bonus - gdk_string_width
				     (graph_font, head)) >> 1;

				gdk_draw_text (g->bs, graph_font,
					       text_gc,
					       x,
					       GRAPH_PAD_Y + th,
					       head, strlen (head));

				th += fh2;
				gh += fh2;
			}

			th += GRAPH_PAD_Y;
			gh += GRAPH_PAD_Y;

		}

		data = g->data_fn (g, GRAPH_FIRST, NULL, NULL);

		while (data) {
			gchar *label = g->data_fn
				(g, GRAPH_LABEL, data, NULL);
			gint64 value = 0;

			g->data_fn (g, GRAPH_VALUE, data, &value);
			if (total == 0) total = 1; /* avoids division by 0. */

			data = g->data_fn (g, GRAPH_NEXT, data, NULL);

			if (draw) {
				double idx = (double)value / total;
				gint cb = idx * bonus;
				gint lb, ll1, ll2, le;

				cgh = h * idx;
				lb = gh + (cgh >> 1);
				ll1 = th - asc2;
				ll2 = ll1 + cb;
				le = (lb >= ll1 && lb <= ll2) ?
					lb : ((lb - ll2 > 0) ? ll2 : ll1);
				
				gdk_draw_rectangle (g->bs,
						    tgcs [c],
						    TRUE,
						    GRAPH_PAD_X, gh,
						    GRAPH_WIDTH, cgh);
				gdk_draw_line (g->bs,
					       tgcs [c],
					       GRAPH_WIDTH +
					       GRAPH_PAD_WIDTH +
					       GRAPH_PAD_X,
					       lb,
					       GRAPH_WIDTH + GRAPH_LINE_WIDTH +
					       GRAPH_PAD_WIDTH + GRAPH_PAD_X,
					       le);

				ll = gdk_string_width (graph_font, label);
				       
				gdk_draw_text (g->bs, graph_font,
					       tgcs [c],
					       GRAPH_WIDTH + GRAPH_LINE_WIDTH +
					       (GRAPH_PAD_WIDTH << 1) +
					       GRAPH_PAD_X + w_bonus + (mll-ll),
					       le+asc2,
					       label, strlen (label));
				       
				gh += (cgh) ? cgh + 1 : 0;
				th += fh + cb;
				c++;
				c %= g->n;
				bonus -= cb;
				h -= cgh;
				total -= value;

			} else {
				total += value;
				h+=2;
				ll = gdk_string_width (graph_font, label);
				if (mll < ll)
					mll = ll;
				g->minimum_width = mll + GRAPH_WIDTH +
					GRAPH_LINE_WIDTH +
					(GRAPH_PAD_WIDTH << 1) +
					(GRAPH_PAD_X << 1);
			}

			lines ++;
		}

		if (!draw) {
			w = GRAPH_WIDTH + GRAPH_LINE_WIDTH +
				(GRAPH_PAD_WIDTH << 1) +
				(GRAPH_PAD_X << 1) + mll;
			w_bonus = g->width - w;
			w_bonus = (w_bonus > 0) ? w_bonus : 0;
			h = fh * lines;
		}

		if (!lines)
			break;
	}

	if (tail) {
		
		gint x, tail_len;

		tail_len = gdk_string_width (graph_font, tail) +
			(GRAPH_PAD_WIDTH << 1);

		if (g->minimum_width < tail_len)
			g->minimum_width = tail_len;

		x = (w + w_bonus - gdk_string_width (graph_font, tail)) >> 1;

		gdk_draw_text (g->bs, graph_font,
			       text_gc,
			       x,
			       th + GRAPH_PAD_Y,
			       tail, strlen (tail));
	}

	for (i = 0; i < g->n; i++)
		gdk_gc_unref (tgcs [i]);
	g_free (tgcs);

	gdk_gc_unref (text_gc);

	if (g->bs)
		gdk_draw_pixmap (g->da->window,
				 g->da->style->fg_gc [GTK_WIDGET_STATE (g->da)],
				 g->bs, 0, 0, 0, 0,
				 g->width, g->height);
}

void
graph_update (Graph *g)
{
	graph_process_data (g);
}

static void
graph_colors_set (Graph *g)
{
	if (g->colors_allocated) {
		GdkColormap *cmap;

		cmap = gdk_window_get_colormap (g->da->window);
		gdk_colormap_free_colors (cmap, g->colors, g->n);
		
		gdk_colormap_free_colors (cmap, g->foreground, 1);
		gdk_colormap_free_colors (cmap, g->background, 1);
		g->colors_allocated = 0;
	}

	g->colors = gtop_properties.graph.colors+2;
	g->foreground = &gtop_properties.graph.colors [0];
	g->background = &gtop_properties.graph.colors [1];
	g->n = GRAPH_DEFAULT_COLORS;
}

static void
graph_properties_update (GnomePropertyObject *object)
{
	GList *c;

	if (mdi->active_view)
		gtk_widget_queue_resize (GTK_WIDGET (mdi->active_view));

	for (c = widget_list; c; c = c->next) {
		Graph *graph = c->data;

		graph_colors_set (graph);

		graph_update (graph);
	}
}

static void
adjustment_changed_cb (GtkWidget *widget,
		       GtkWidget *adjustment)
{
	gtop_properties_changed ();
}

static void
graph_properties_apply (GnomePropertyObject *object)
{
	GTopGraphProperties *prop_ptr = object->temp_data;

	prop_ptr->default_width =
		adjustments [DEFAULT_WIDTH]->value;

	prop_ptr->default_height =
		adjustments [DEFAULT_HEIGHT]->value;

	prop_ptr->horizontal_border =
		adjustments [HORIZONTAL_BORDER]->value;

	prop_ptr->vertical_border =
		adjustments [VERTICAL_BORDER]->value;

	prop_ptr->line_width =
		adjustments [LINE_WIDTH]->value;

	prop_ptr->pad_width =
		adjustments [PAD_WIDTH]->value;

	prop_ptr->graph_width =
		adjustments [GRAPH_GRAPH_WIDTH]->value;

	prop_ptr->extra_height =
		adjustments [GRAPH_EXTRA_HEIGHT]->value;
}

static void
graph_properties_changed (GnomePropertyObject *object)
{
	GTopGraphProperties *temp_ptr = object->temp_data;
	GTopGraphProperties *prop_ptr = object->prop_data;
	gint i;

	/* [FIXME]: Yes, this is an ugly hack, but we cannot fix it in
	 *          libgnomeui right now. Sep 29, 1999 Martin Baulig */

	for (i = 0; i < GRAPH_DEFAULT_COLORS+2; i++)
		temp_ptr->colors [i] = prop_ptr->colors [i];

	graph_properties_update (object);

	gtop_properties_changed ();
}

static GtkWidget *
graph_properties_init (GnomePropertyObject *object)
{
	GTopGraphProperties *prop_ptr = &gtop_properties.graph;
	GtkWidget *vb, *frame, *table, *label, *spin;
	GtkObject *adjustment;
	int i;

	gchar *labels [GRAPH_DEFAULT_COLORS+2];
	gint table_pos [GRAPH_DEFAULT_COLORS+2];
	
	vb = gtk_vbox_new (FALSE, 0);
	gtk_box_set_spacing (GTK_BOX (vb), GNOME_PAD_SMALL << 1);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	frame = gtk_frame_new (_("Graph Font"));

	table = gnome_property_entry_font (object, _("Graph Font"),
					   &gtop_properties.graph.font_name,
					   &gtop_properties.graph.font);

	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Graph Geometry"));
	table = gtk_table_new (5, 4, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	label = gtk_label_new (_("Default width:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->default_width);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 0, 1);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [DEFAULT_WIDTH] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Default height:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (1, 1, INT_MAX, 1, 100, 100);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->default_height);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 1, 2);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [DEFAULT_HEIGHT] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Horizontal border:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 0, 1);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 10, 10);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->horizontal_border);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 3, 4, 0, 1);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [HORIZONTAL_BORDER] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Vertical border height:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 1, 2);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 10, 10);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->vertical_border);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 3, 4, 1, 2);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [VERTICAL_BORDER] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Line width:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 3, 4);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 10, 10);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->line_width);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 3, 4);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [LINE_WIDTH] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Pad width:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 3, 4);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 10, 10);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->pad_width);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 3, 4, 3, 4);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [PAD_WIDTH] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Graph width:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 4, 5);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 10, 10);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->graph_width);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 1, 2, 4, 5);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GRAPH_GRAPH_WIDTH] = GTK_ADJUSTMENT (adjustment);

	label = gtk_label_new (_("Extra height:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 4, 5);

	spin = gtk_spin_button_new (NULL, 1, 0);
	adjustment = gtk_adjustment_new (0, 0, INT_MAX, 1, 10, 10);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin),
				   prop_ptr->extra_height);
	gtk_table_attach_defaults (GTK_TABLE (table), spin, 3, 4, 4, 5);

	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	adjustments [GRAPH_EXTRA_HEIGHT] = GTK_ADJUSTMENT (adjustment);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	labels [0] = _("Foreground");
	labels [1] = _("Background");

	table_pos [0] = 0;
	table_pos [1] = GRAPH_DEFAULT_COLORS-1;

	for (i = 0; i < GRAPH_DEFAULT_COLORS; i++) {
		labels [i+2] = g_strdup_printf (_("Color %d"), i);
		table_pos [i+2] = GRAPH_DEFAULT_COLORS+i;
	}

	frame = gnome_property_entry_colors
		(object, N_("Graph Colors"), GRAPH_DEFAULT_COLORS+2,
		 GRAPH_DEFAULT_COLORS, table_pos, prop_ptr->colors,
		 (const gchar **) labels);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	for (i = 0; i < GRAPH_DEFAULT_COLORS; i++)
		g_free (labels [i+2]);

	return vb;
}

static void
graph_properties_load (GnomePropertyObject *object)
{
	const gchar *default_font;
	char name [BUFSIZ], *tmp;
	guint width, height;
	GdkColor *color;
	int i;

	width = gdk_screen_width ();

	if (width <= 640)
		default_font = default_fonts [0];
	else if (width <= 800)
		default_font = default_fonts [1];
	else if (width <= 1024)
		default_font = default_fonts [2];
	else if (width <= 1280)
		default_font = default_fonts [3];
	else
		default_font = default_fonts [4];

	sprintf (name, "gtop/graph/font_name=%s", default_font);
	gtop_properties.graph.font_name = gnome_config_get_string (name);

	if (gtop_properties.graph.font_name)
		gtop_properties.graph.font = gdk_fontset_load
			(gtop_properties.graph.font_name);

	if (!gtop_properties.graph.font) {
		g_free (gtop_properties.graph.font_name);
		gtop_properties.graph.font_name = g_strdup (default_font);
		gtop_properties.graph.font = gdk_fontset_load
			(gtop_properties.graph.font_name);
	}

	width = (gfloat) gdk_screen_width () * 0.60;
	height = (gfloat) gdk_screen_height () * 0.40;

	sprintf (name, "gtop/graph/default_width=%d", width);
	gtop_properties.graph.default_width = gnome_config_get_int (name);

	sprintf (name, "gtop/graph/default_height=%d", height);
	gtop_properties.graph.default_height = gnome_config_get_int (name);

	gtop_properties.graph.horizontal_border =
		gnome_config_get_int ("gtop/graph/horizontal_border=20");

	gtop_properties.graph.vertical_border =
		gnome_config_get_int ("gtop/graph/vertical_border=20");

	gtop_properties.graph.line_width =
		gnome_config_get_int ("gtop/graph/line_width=80");

	gtop_properties.graph.pad_width =
		gnome_config_get_int ("gtop/graph/pad_width=20");

	gtop_properties.graph.graph_width =
		gnome_config_get_int ("gtop/graph/graph_width=80");

	gtop_properties.graph.extra_height =
		gnome_config_get_int ("gtop/graph/extra_height=0");

	color = &gtop_properties.graph.colors [0];

	sprintf (name, "gtop/graph/foreground=#%04x%04x%04x",
		 65535, 65535, 65535);

	tmp = gnome_config_get_string (name);
	gdk_color_parse (tmp, color);
	g_free (tmp);

	color = &gtop_properties.graph.colors [1];

	sprintf (name, "gtop/graph/background=#%04x%04x%04x", 0, 0, 0);

	tmp = gnome_config_get_string (name);
	gdk_color_parse (tmp, color);
	g_free (tmp);

	for (i = 0; i < GRAPH_DEFAULT_COLORS; i++) {
		color = &(gtop_properties.graph.colors [i+2]);

		sprintf (name, "gtop/graph/color%d=#%04x%04x%04x", i,
			 graph_default_colors [i].red,
			 graph_default_colors [i].green,
			 graph_default_colors [i].blue);

		tmp = gnome_config_get_string (name);
		gdk_color_parse (tmp, color);
		g_free (tmp);
	}
}

static void
graph_properties_save (GnomePropertyObject *object)
{
	char name [BUFSIZ], tmp [BUFSIZ];
	GdkColor *color;
	gint i;

	gnome_config_set_string ("gtop/graph/font_name",
				 gtop_properties.graph.font_name);

	gnome_config_set_int ("gtop/graph/default_width",
			      gtop_properties.graph.default_width);

	gnome_config_set_int ("gtop/graph/default_height",
			      gtop_properties.graph.default_height);

	gnome_config_set_int ("gtop/graph/horizontal_border",
			      gtop_properties.graph.horizontal_border);

	gnome_config_set_int ("gtop/graph/vertical_border",
			      gtop_properties.graph.vertical_border);

	gnome_config_set_int ("gtop/graph/line_width",
			      gtop_properties.graph.line_width);

	gnome_config_set_int ("gtop/graph/pad_width",
			      gtop_properties.graph.pad_width);

	gnome_config_set_int ("gtop/graph/graph_width",
			      gtop_properties.graph.graph_width);

	gnome_config_set_int ("gtop/graph/extra_height",
			      gtop_properties.graph.extra_height);

	color = &gtop_properties.graph.colors [0];
	sprintf (tmp, "#%04x%04x%04x", color->red, color->green, color->blue);
	gnome_config_set_string ("gtop/graph/foreground", tmp);

	color = &gtop_properties.graph.colors [1];
	sprintf (tmp, "#%04x%04x%04x", color->red, color->green, color->blue);
	gnome_config_set_string ("gtop/graph/background", tmp);

	for (i = 0; i < GRAPH_DEFAULT_COLORS; i++) {
		color = &(gtop_properties.graph.colors [i+2]);

		sprintf (tmp, "#%04x%04x%04x",
			 color->red, color->green, color->blue);

		sprintf (name, "gtop/graph/color%d", i);
		gnome_config_set_string (name, tmp);
	}
}
