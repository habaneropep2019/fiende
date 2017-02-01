#include <stdio.h>

/*
 * Gtk/XmHTML widget port.
 *
 * Miguel de Icaza, 1997
 *
 * FIXME:
 *   - Check all widget repaints against original source, many are
 *     bogus, they should be calls to ClearArea()
 */
#define SetScrollBars(HTML)

#define AdjustVerticalScrollValue(VSB,VAL) do { \
	int max = 0, size = 0; \
        max  = (int) (GTK_RANGE (VSB)->adjustment->upper); \
        size = (int) (GTK_RANGE (VSB)->adjustment->page_size);\
	if (VAL > (max - size)) \
            VAL = (max - size); \
        } while (0)
		
gint gtk_xmhtml_signals [GTK_XMHTML_LAST_SIGNAL] = { 0, };

/*
 * Gdk does not have a visibility mask thingie *yet*
 */
#ifndef GDK_VISIBILITY_MASK 
#   define GDK_VISIBILITY_MASK 0
#endif

#define SCROLLBAR_SPACING 0

static GtkContainer *parent_class = NULL;

/* prototypes for functions defined here */
static void gtk_xmhtml_realize (GtkWidget *widget) G_GNUC_UNUSED;
static void gtk_xmhtml_unrealize (GtkWidget *widget) G_GNUC_UNUSED;
static void gtk_xmhtml_forall (GtkContainer *we, gboolean include_internals, GtkCallback callback, gpointer callback_data);
static void gtk_xmhtml_map (GtkWidget *widget);
static void gtk_xmhtml_draw (GtkWidget *widget, GdkRectangle *area);
static gint gtk_xmhtml_expose (GtkWidget *widget, GdkEventExpose *event);
static void gtk_xmhtml_add (GtkContainer *container, GtkWidget *widget);
static void gtk_xmhtml_remove (GtkContainer *container, GtkWidget *widget);
static void gtk_xmhtml_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void gtk_xmhtml_size_request (GtkWidget *widget, GtkRequisition *requisition);

static void CheckScrollBars(XmHTMLWidget html);
static void GetScrollDim(XmHTMLWidget html, int *hsb_height, int *vsb_width);
static void	ExtendStart(TWidget w, TEvent *event);
static void	ExtendEnd  (TWidget w, TEvent *event);

typedef gint (*GtkXmHTMLSignal1) (GtkObject *object,
                                  gpointer   arg1,
                                  gpointer   data);

typedef gint (*GtkXmHTMLSignal2) (GtkObject *object,
                                  gpointer   arg1,
				  gpointer   arg2,
                                  gpointer   data);

static void
gtk_xmthml_marshall_1 (GtkObject *object, GtkSignalFunc func, gpointer data, GtkArg *args)
{
	GtkXmHTMLSignal1 rfunc;

	rfunc = (GtkXmHTMLSignal1) func;

	(* rfunc) (object, GTK_VALUE_POINTER (args[0]), data);
}

static void
gtk_xmthml_marshall_2 (GtkObject *object, GtkSignalFunc func, gpointer data, GtkArg *args)
{
	GtkXmHTMLSignal2 rfunc;
	gint *return_val = GTK_RETLOC_INT(args[2]);
	
	rfunc = (GtkXmHTMLSignal2) func;

	*return_val = (* rfunc) (object, GTK_VALUE_STRING (args[0]),
				 GTK_VALUE_POINTER (args [1]), data);
}

static Pixel
pixel_color (GtkXmHTML *html, char *color_name)
{
	GdkColor c;
	int n;

	_XmHTMLCheckXCC (html);
	
	gdk_color_parse (color_name, &c);

	c.pixel = 0;
	n = 0;

	XCCGetPixels (html->html.xcc, &c.red, &c.green, &c.blue, 1, &c.pixel, &n);
	return c.pixel;
}

/* These are initialized in the Motif sources with the resources */
static void
gtk_xmhtml_resource_init (GtkXmHTML *html)
{
	/* The strings */
	html->html.mime_type             = g_strdup ("text/html");
	html->html.charset               = g_strdup ("iso8859-1");
	html->html.font_family           = g_strdup ("adobe-times-normal-*");
	html->html.font_family_fixed     = g_strdup ("adobe-courier-normal-*");
	html->html.font_sizes            = g_strdup (XmHTML_DEFAULT_FONT_SCALABLE_SIZES);
	html->html.font_sizes_fixed      = g_strdup (XmHTML_DEFAULT_FONT_FIXED_SIZES);
	html->html.zCmd                  = g_strdup ("gunzip");

	html->html.event_proc            = NULL;
	html->html.needs_vsb       	 = FALSE;
	html->html.needs_hsb       	 = FALSE;
	html->html.scroll_x        	 = 0;
	html->html.scroll_y        	 = 0;
	html->html.alignment             = TALIGNMENT_BEGINNING;
	html->html.anchor_cursor         = NULL;
	html->html.anchor_display_cursor = TRUE;
	html->html.anchor_buttons        = TRUE;
	
	html->html.anchor_fg             = pixel_color (html, "blue1");
	html->html.anchor_visited_fg     = pixel_color (html, "red");
	html->html.anchor_target_fg      = pixel_color (html, "blue1");
	html->html.anchor_activated_fg   = pixel_color (html, "red");
	html->html.anchor_activated_bg   = pixel_color (html, "white");
	
	html->html.highlight_on_enter    = TRUE;
	html->html.anchor_underline_type = LINE_SOLID | LINE_UNDER | LINE_SINGLE;
	html->html.anchor_target_underline_type = LINE_DASHED | LINE_UNDER | LINE_SINGLE;
	html->html.anchor_visited_underline_type = LINE_SOLID | LINE_UNDER | LINE_SINGLE;
	
	html->html.anchor_visited_proc   = NULL;
	html->html.image_proc            = NULL;
	html->html.gif_proc              = NULL;

#if 0
	/* The Gtk edition of the code does not actually use these,
	 * it uses the signal mechanism instead
	 */
	html->html.anchor_track_callback = NULL;
	html->html.activate_callback     = NULL;
	html->html.arm_callback          = NULL;
	html->html.frame_callback        = g_list_append(NULL, donothing);
	html->html.form_callback         = NULL;
	html->html.focus_callback        = NULL;
	html->html.losing_focus_callback = NULL;
	html->html.link_callback         = NULL;
	html->html.input_callback        = NULL;
	html->html.motion_track_callback = NULL;
	html->html.imagemap_callback     = NULL;
	html->html.document_callback     = NULL;
	html->html.event_callback        = NULL;
#endif
	
	html->html.hsb                   = NULL;
	html->html.vsb                   = NULL;

	html->html.images_enabled        = TRUE;
	html->html.max_image_colors      = 0;
	html->html.screen_gamma          = 2.2;
	html->html.get_data              = NULL;
	html->html.end_data              = NULL;
	html->html.plc_delay             = PLC_DEFAULT_DELAY;
	html->html.plc_min_delay         = PLC_MIN_DELAY;
	html->html.plc_max_delay         = PLC_MAX_DELAY;
	html->html.perfect_colors        = 0; /* XmAUTOMATIC; */
	html->html.resize_height         = FALSE;
	html->html.resize_width          = FALSE;
	html->html.sb_policy             = GTK_POLICY_AUTOMATIC;
	html->html.sb_placement          = 0; /* FIXME */
	html->html.margin_height         = XmHTML_DEFAULT_MARGIN;
	html->html.margin_width          = XmHTML_DEFAULT_MARGIN;
	html->html.string_direction      = TSTRING_DIRECTION_L_TO_R;
	html->html.strict_checking       = FALSE;
	html->html.enable_outlining      = FALSE;
	html->html.top_line              = 0;
	html->html.value                 = NULL;
	html->html.work_area             = NULL;
	html->html.bad_html_warnings     = TRUE;
	html->html.body_colors_enabled   = TRUE;
	html->html.body_images_enabled   = TRUE;
	html->html.allow_color_switching = TRUE;
	html->html.allow_font_switching  = TRUE;
	html->html.allow_form_coloring   = TRUE;
	html->html.imagemap_fg           = pixel_color (html, "White");
	html->html.imagemap_draw         = FALSE;
	html->html.repeat_delay          = 25;
	html->html.freeze_animations     = FALSE;
	html->html.map_to_palette        = XmDISABLED;
	html->html.palette               = NULL;
	html->html.def_body_image_url    = NULL;
	html->html.alpha_processing      = XmALWAYS;
	html->html.rgb_conv_mode         = XmBEST;
	html->html.client_data           = NULL;
#ifdef DEBUG
	html->html.debug_disable_warnings= FALSE;
	html->html.debug_full_output     = FALSE;
	html->html.debug_save_clipmasks  = FALSE;
	html->html.debug_prefix          = NULL;
	html->html.debug_no_loopcount    = FALSE;
	html->html.debug_levels          = NULL;
#endif
}

static void
gtk_xmhtml_reset_pending_flags (GtkXmHTML *html)
{
	html->parse_needed = 0;
	html->reformat_needed = 0;
	html->redraw_needed = 0;
	html->free_images_needed = 0;
	html->layout_needed = 0;
}

static void gtk_xmhtml_create_widgets (GtkXmHTML *html);

static void
gtk_xmhtml_init (GtkXmHTML *html)
{
	GTK_WIDGET_SET_FLAGS (html, GTK_NO_WINDOW);
	
	gtk_xmhtml_resource_init (html);
	gtk_xmhtml_reset_pending_flags (html);

	/* Gtk port bits */
	html->children = NULL;

	html->background_pixel = 0;
	html->foreground_pixel = 0;
	html->bottom_shadow_gc = NULL;
	html->top_shadow_gc    = NULL;
	html->highlight_gc     = NULL;
	html->highlight_color  = 0;

	/* moved here from gtk_xmhtml_new() so one can subclass */
	/* GtkXmHTML widget properly                            */
	GTK_WIDGET(html)->allocation.width  = 200;
	GTK_WIDGET(html)->allocation.height = 200;
	html->initialized = 0;
	html->frozen = 0;
	gtk_xmhtml_create_widgets (html);
	gtk_xmhtml_source (html, "<body></body>");
	html->frozen = 1;
}

GtkWidget *
gtk_xmhtml_new (void)
{
	GtkXmHTML *html;

	html = gtk_type_new (gtk_xmhtml_get_type ());
	return GTK_WIDGET (html);
}

static void
gtk_xmhtml_destroy (GtkObject *object)
{
	GtkXmHTML *html = GTK_XMHTML (object);

	DestroyPhaseZero (html);
}

static void
gtk_xmhtml_class_init (GtkXmHTMLClass *class)
{
	GtkObjectClass    *object_class;
	GtkWidgetClass    *widget_class;
	GtkContainerClass *container_class;
	extern FILE *__rsd_debug_file;

	__rsd_debug_file = stdout;

	object_class    = (GtkObjectClass *) class;
	widget_class    = (GtkWidgetClass *) class;
	container_class = (GtkContainerClass *) class;

	parent_class = gtk_type_class (gtk_container_get_type ());

	gtk_xmhtml_signals [GTK_XMHTML_ACTIVATE] =
		gtk_signal_new ("activate",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, activate),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_ARM] =
		gtk_signal_new ("arm",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, rearm),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_ANCHOR_TRACK] =
		gtk_signal_new ("anchor_track",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, anchor_track),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_FRAME] =
		gtk_signal_new ("frame",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, frame),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_FORM] =
		gtk_signal_new ("form",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, form),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_INPUT] =
		gtk_signal_new ("input",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, input),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_LINK] =
		gtk_signal_new ("link",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, link),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_MOTION] =
		gtk_signal_new ("motion",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, motion),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_IMAGEMAP] =
		gtk_signal_new ("imagemap",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, imagemap),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_DOCUMENT] =
		gtk_signal_new ("document",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, document),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_FOCUS] =
		gtk_signal_new ("_focus", /* there is already a focus signal */
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, focus),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_LOSING_FOCUS] =
		gtk_signal_new ("losing_focus",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, losing_focus),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_MOTION_TRACK] =
		gtk_signal_new ("motion_track",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, motion_track),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_HTML_EVENT] =
		gtk_signal_new ("html_event",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, motion_track),
				gtk_xmthml_marshall_1, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	gtk_xmhtml_signals [GTK_XMHTML_ANCHOR_VISITED] =
		gtk_signal_new ("anchor_visited",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkXmHTMLClass, anchor_visited),
				gtk_xmthml_marshall_2, GTK_TYPE_INT, 2,
				GTK_TYPE_STRING, GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, gtk_xmhtml_signals, GTK_XMHTML_LAST_SIGNAL);

	object_class->destroy       = gtk_xmhtml_destroy;
	widget_class->map           = gtk_xmhtml_map;
/*	widget_class->unmap         = gtk_xmhtml_unmap; */
	widget_class->draw          = gtk_xmhtml_draw;
	widget_class->size_request  = gtk_xmhtml_size_request;
	widget_class->size_allocate = gtk_xmhtml_size_allocate;
	widget_class->expose_event  = gtk_xmhtml_expose;
	
	container_class->add = gtk_xmhtml_add;
	container_class->remove = gtk_xmhtml_remove;
	container_class->forall = gtk_xmhtml_forall;
}

static void
gtk_xmhtml_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkXmHTML *html = GTK_XMHTML (widget);
	int extra_width, extra_height;
	GtkRequisition child_requisition;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (requisition != NULL);
	
	requisition->width  = 0;
	requisition->height = 0;

	if (GTK_WIDGET_VISIBLE (html->html.work_area)){
		gtk_widget_size_request (html->html.work_area, &child_requisition);

		requisition->width  += child_requisition.width;
		requisition->height += child_requisition.height;
	}

	extra_width = extra_height = 0;

	/* Horizontal scroll bar */
	gtk_widget_size_request (html->html.hsb, &child_requisition);
	requisition->width = MAX (requisition->width, child_requisition.width);
	extra_height = SCROLLBAR_SPACING + child_requisition.height;
		
	/* Vertical scrollbarl */
	gtk_widget_size_request (html->html.vsb, &child_requisition);
	requisition->height = MAX (requisition->height, child_requisition.height);
	extra_width = SCROLLBAR_SPACING + child_requisition.width;

	requisition->width  += GTK_CONTAINER (widget)->border_width * 2 + extra_width;
	requisition->height += GTK_CONTAINER (widget)->border_width * 2 + extra_height;
}

static void
gtk_xmhtml_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	widget->allocation = *allocation;
	
/*	printf ("Size Allocate: (%d,%d) %d %d\n",
		allocation->x,
		allocation->y,
		allocation->height,
		allocation->width);
*/
	Resize (widget);
}

guint
gtk_xmhtml_get_type (void)
{
	static guint gtk_xmhtml_type = 0;

	if (!gtk_xmhtml_type){
		GtkTypeInfo gtk_xmhtml_info =
		{
			"GtkXmHTML",
			sizeof (GtkXmHTML),
			sizeof (GtkXmHTMLClass),
			(GtkClassInitFunc) gtk_xmhtml_class_init,
			(GtkObjectInitFunc) gtk_xmhtml_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		gtk_xmhtml_type = gtk_type_unique (gtk_container_get_type (), &gtk_xmhtml_info);
	}
	return gtk_xmhtml_type;
}

/*****
* Name: 		CreateAnchorCursor
* Return Type: 	void
* Description: 	creates the built-in anchor cursor
* In: 
*	html:		XmHTMLWidget for which to create a cursor
* Returns:
*	nothing.
*****/
static void 
CreateAnchorCursor(XmHTMLWidget html)
{
	TPixmap shape, mask;
	TWindow window = Toolkit_Widget_Window((TWidget)html);
	GdkColormap *colormap;
	GdkColor fg, bg, white, black;

	fg.pixel = 1;
	bg.pixel = 0;
	if (html->html.anchor_cursor != None)
		return;

	if(!window)
		window = Toolkit_Default_Root_Window (display);

	shape = gdk_pixmap_create_from_data (window, fingers_bits, fingers_width, fingers_height,
					     1, &fg, &bg);
	
	mask = gdk_pixmap_create_from_data (window, fingers_m_bits, fingers_m_width, fingers_m_height,
					    1, &fg, &bg);

	colormap = gtk_widget_get_colormap (GTK_WIDGET (html));
	gdk_color_white (colormap, &white);
	gdk_color_black (colormap, &black);
	html->html.anchor_cursor = gdk_cursor_new_from_pixmap (shape, mask,
							       &white, &black,
							       fingers_x_hot, fingers_y_hot);
}

/*****
 * Name: 		FormScroll
 * Return Type: 	void
 * Description: 	scrolls all TWidgets of all forms in the current document.
 * In: 
 *	html:		XmHTML TWidget id
 * Returns:
 *	nothing.
 *****/
static void
FormScroll (XmHTMLWidget html)
{
	int x, y, xs, ys;
	XmHTMLFormData *form;
	XmHTMLForm *entry;
	Boolean did_anything = False;

	_XmHTMLDebug(1, ("XmHTML.c: FormScroll, Start\n"));

	for(form = html->html.form_data; form != NULL; form = form->next)
	{
		for(entry = form->components; entry != NULL; entry = entry->next)
		{
			if(entry->w)
			{
				/* save current TWidget position */
				x = entry->x;
				y = entry->y;

				/* compute new TWidget position */
				xs = entry->data->x - html->html.scroll_x;
				ys = entry->data->y - html->html.scroll_y;

				/* check if we need to show this TWidget */
				if(xs + entry->width > 0 && xs < html->html.work_width &&
					ys + entry->height > 0 && ys < html->html.work_height)
				{
					_XmHTMLDebug(1, ("XmHTML.c: FormScroll, moving "
						"TWidget %s to %ix%i\n", entry->name, xs, ys));

					/* save new TWidget position */
					entry->x = xs;
					entry->y = ys;

					/* and move to it */
					gtk_xmhtml_set_geometry (entry->w, xs, ys,
								 entry->width, entry->height);
					gtk_widget_show (entry->w);
					
					/* show it */
					if(!entry->mapped)
					{
						entry->mapped = True;
					}

					/* restore background at previously obscured position */
					Refresh(html, x, y, entry->width, entry->height);

					did_anything = True;
				}
				else
				{
					/* hide by unmapping it */
					if(entry->mapped)
					{
						_XmHTMLDebug(1, ("XmHTML.c: FormScroll, hiding "
							"TWidget %s\n", entry->name));

						gtk_widget_hide (entry->w);
						entry->mapped = False;

						/* restore background at previously obscured position */
						Refresh(html, x, y, entry->width, entry->height);

						did_anything = True;
					}
				}
			}
		}
	}
	if (did_anything)
		gdk_flush ();
}

static void
gtk_xmhtml_draw (GtkWidget *widget, GdkRectangle *area)
{
	GtkXmHTML    *html = GTK_XMHTML (widget);
	GdkRectangle na;
	GList        *children;
	int i;

	if (!GTK_WIDGET_DRAWABLE (widget))
		return;

	if (gtk_widget_intersect (html->html.vsb, area, &na))
		gtk_widget_draw (html->html.vsb, &na);

	if (gtk_widget_intersect (html->html.hsb, area, &na))
		gtk_widget_draw (html->html.hsb, &na);

	if (gtk_widget_intersect (html->html.work_area, area, &na))
		gtk_widget_draw (html->html.work_area, &na);

	/* frames */
	for (i = 0; i < html->html.nframes; i++)
		if (gtk_widget_intersect (html->html.frames [i]->frame, area, &na))
		    gtk_widget_draw (html->html.frames [i]->frame, &na);

	/* forms */
	for (children = html->children; children; children = children->next){
		GtkWidget *w = GTK_WIDGET (children->data);

		if (gtk_widget_intersect (w, area, &na))
			gtk_widget_draw (w, &na);
	}
}

static gint
gtk_xmhtml_expose (GtkWidget *widget, GdkEventExpose *event)
{
	return FALSE;
}

/*****
* Name: 		CheckGC
* Return Type: 	void
* Description: 	creates a Graphics Context to be used for rendering
* In: 
*	html:		XmHTMLWidget
* Returns:
*	nothing, but a GC is created and stored in the TWidget's internal data
*	structure. If background images are allowed, a seperate GC is created
*	which is used in PaintBackground to do tiling of the background with an
*	image.
*****/
static void
CheckGC(XmHTMLWidget html)
{
	_XmHTMLDebug(1, ("XmHTML.c: CheckGC Start\n"));

	/* sanity check */
	if(!GTK_WIDGET_REALIZED (GTK_WIDGET (html)))
		return;

	/* main gc */
	if (html->html.gc == NULL){
		GdkGCValues xgc;

		xgc.function = GDK_COPY;
		xgc.foreground.pixel = Toolkit_StyleColor_Foreground(html);
		xgc.background.pixel = Toolkit_StyleColor_Background(html);
		html->html.gc = gdk_gc_new_with_values (GTK_WIDGET(html)->window, &xgc,
							GDK_GC_FOREGROUND | GDK_GC_BACKGROUND |
							GDK_GC_FUNCTION);
		_XmHTMLRecomputeColors(html);

		_XmHTMLDebug(1, ("XmHTML.c: CheckGC, gc created\n"));
	}
	/* background image gc */
	if(html->html.body_images_enabled && html->html.bg_gc == NULL)
	{
		html->html.bg_gc = gdk_gc_new (GTK_WIDGET(html)->window);
		gdk_gc_copy (html->html.bg_gc, html->html.gc);
	}

	_XmHTMLDebug(1, ("XmHTML.c: CheckGC End\n"));
}

static gint
gtk_xmhtml_expose_event (GtkWidget *widget, GdkEvent *event, gpointer closure)
{
	GtkXmHTML *html = closure;
	GdkEventExpose *e = (GdkEventExpose *) event;
	
	if (html->html.formatted == NULL || html->html.nframes)
		return FALSE;

	/* FIXME: The code in the Motif port does event coalescing */
	Refresh(html, e->area.x, e->area.y, e->area.width, e->area.height);
	return FALSE;
}

/*****
* Name: 		TrackMotion
* Return Type: 	void
* Description: 	mouse tracker; calls XmNanchorTrackCallback if 
*				entering/leaving an anchor.
*				Also calls XmNmotionTrackCallback when installed.
* In: 
*	w:			XmHTMLWidget
*	event:		MotionEvent structure
*	params:		additional args, unused
*	num_parmas:	no of additional args, unused
* Returns:
*	nothing
*****/
static gint
gtk_xmhtml_motion_event (GtkWidget *widget, GdkEvent *event, gpointer closure)
{
	GtkXmHTML *html = GTK_XMHTML (closure);
	GdkEventMotion *motion = (GdkEventMotion *) event;
	XmAnyCallbackStruct cbs;
	int x, y;
	
	/* ignore if we don't have to make any more feedback to the user */
	if (!html->html.need_tracking)
		return TRUE;

	/* we are already on the correct anchor, just return */
	_XmHTMLFullDebug(1, ("XmHTML.c: TrackMotion Start.\n"));

	/* pass down to motion tracker callback if installed */
	cbs.reason = XmCR_HTML_MOTIONTRACK;
	cbs.event = event;
	gtk_signal_emit (GTK_OBJECT (html), gtk_xmhtml_signals [GTK_XMHTML_MOTION_TRACK], &cbs);
	x = motion->x;
	y = motion->y;
	AnchorTrack (html, event, x, y);

	return TRUE;
}

/*
 * Handles focus_in, focus_out, leave_notify, enter_notify
 */
static int
gtk_xmhtml_focus (GtkWidget *widget, GdkEvent *event, gpointer closure)
{
	GtkXmHTML *html = GTK_XMHTML (closure);
	GtkObject *htmlo = GTK_OBJECT (closure);
	GdkEventFocus *focus = (GdkEventFocus *) event;
	XmAnyCallbackStruct cbs;
	gint focusing_in = 0;
	
	if (event->type == GDK_FOCUS_CHANGE)
		focusing_in = (focus->window == html->html.work_area->window);

	if (event->type == GDK_FOCUS_CHANGE && focusing_in){
		cbs.reason = XmCR_FOCUS;
		cbs.event = event;
		gtk_signal_emit (htmlo, gtk_xmhtml_signals [GTK_XMHTML_FOCUS], &cbs);
		gdk_window_set_cursor (html->html.work_area->window, NULL);
		return TRUE;
	}

	/* Both Leave notify and focus out are handled here */

	/* FIXME, from the Motif code:
		* LeaveNotify Events occur when the pointer focus is transferred
		* from the DrawingArea child to another window. This can occur
		* when the pointer is moved outside the Widget *OR* when a
		* ButtonPress event occurs ON the drawingArea. When that happens,
		* the pointer focus is transferred from the drawingArea to it's
		* parent, being the Widget itself. In this case the detail
		* detail member of the XEnterWindowEvent will be NotifyAncestor,
		* and we would want to ignore this event (as it will cause a
		* flicker of the screen or an unnecessary call to any installed
		* callbacks).
		*
		*
		* The correct code is thus:
		* 
		* if(event->type == LeaveNotify &&
		*     ((XEnterWindowEvent*)event)->detail == NotifyAncestor) 
		*     then {
		*        ignore this one to avoid calling callbacks and flicker
		*     }
	        */
	/* invalidate current selection if there is one */
	if (gtk_signal_handler_pending (GTK_OBJECT(html), gtk_xmhtml_signals [GTK_XMHTML_ANCHOR_TRACK], TRUE)
		&& html->html.anchor_current_cursor_element)
		_XmHTMLTrackCallback (html, event, NULL);

	/* loses focus, remove anchor highlight */
	if(html->html.highlight_on_enter && html->html.armed_anchor)
		LeaveAnchor(html);
	
	html->html.armed_anchor = NULL;
	html->html.anchor_current_cursor_element = NULL;
	gdk_window_set_cursor (html->html.work_area->window, NULL);

	/* final step: call focusOut callback */
	if(event->type == FocusOut && CHECK_CALLBACK (html, losing_focus_callback, LOSING_FOCUS)){
		cbs.reason = XmCR_LOSING_FOCUS;
		cbs.event = event;
		gtk_signal_emit (htmlo, gtk_xmhtml_signals [GTK_XMHTML_LOSING_FOCUS], &cbs);
	}
	return TRUE;
}
		
static int
horizontal_scroll (GtkAdjustment *adj, gpointer user_data)
{
	GtkXmHTML *html = GTK_XMHTML (user_data);

	_XmHTMLDebug(1, ("XmHTML.c: ScrollCB, calling _XmHTMLMoveToPos\n"));
	_XmHTMLMoveToPos (html->html.hsb, html, adj->value);
	return 0;
}

static int
vertical_scroll (GtkAdjustment *adj, gpointer user_data)
{
	GtkXmHTML *html = GTK_XMHTML (user_data);
	
	_XmHTMLDebug(1, ("XmHTML.c: ScrollCB, calling _XmHTMLMoveToPos\n"));
	_XmHTMLMoveToPos (html->html.vsb, html, adj->value);
	return 0;
}

static void
drawing_area_realized (GtkWidget *widget, gpointer data)
{
	GtkXmHTML *html = data;

	html->background_pixel = widget->style->bg[GTK_STATE_NORMAL].pixel;
	html->foreground_pixel = widget->style->fg[GTK_STATE_NORMAL].pixel;

	html->bottom_shadow_gc = gdk_gc_new (widget->window);
	gdk_gc_copy (html->bottom_shadow_gc, widget->style->dark_gc[GTK_STATE_NORMAL]);

	html->top_shadow_gc = gdk_gc_new (widget->window);
	gdk_gc_copy (html->top_shadow_gc, widget->style->light_gc[GTK_STATE_NORMAL]);

	html->highlight_gc = gdk_gc_new (widget->window);
	gdk_gc_copy (html->highlight_gc, widget->style->bg_gc[GTK_STATE_PRELIGHT]);
	html->highlight_color = widget->style->bg[GTK_STATE_PRELIGHT].pixel;

	gtk_xmhtml_thaw (html);

	/* FIXME: remember to destroy the GCs later */
}

static void
gtk_xmhtml_create_widgets (GtkXmHTML *html)
{
	GtkWidget *draw_area;
	int events;

	/* Check if user provided a work area */
	if(html->html.work_area == NULL)
	{
		draw_area = html->html.work_area = gtk_drawing_area_new ();
		gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area), 40, 40);
		
		gtk_xmhtml_manage (GTK_CONTAINER (html), draw_area);

		gtk_signal_connect (GTK_OBJECT (draw_area), "realize",
				    (GtkSignalFunc) drawing_area_realized, html);

		gtk_signal_connect (GTK_OBJECT (draw_area), "expose_event",
				    (GtkSignalFunc) gtk_xmhtml_expose_event, html);
		
		gtk_signal_connect (GTK_OBJECT (draw_area), "motion_notify_event",
				    (GtkSignalFunc) gtk_xmhtml_motion_event, html);

		gtk_signal_connect (GTK_OBJECT (draw_area), "focus_in_event",
				    (GtkSignalFunc) gtk_xmhtml_focus, html);

		gtk_signal_connect (GTK_OBJECT (draw_area), "focus_out_event",
				    (GtkSignalFunc) gtk_xmhtml_focus, html);

		gtk_signal_connect (GTK_OBJECT (draw_area), "leave_notify_event",
				    (GtkSignalFunc) gtk_xmhtml_focus, html);

		gtk_signal_connect (GTK_OBJECT (draw_area), "enter_notify_event",
				    (GtkSignalFunc) gtk_xmhtml_focus, html);

		gtk_signal_connect (GTK_OBJECT (draw_area), "button_press_event",
				    (GtkSignalFunc) ExtendStart, html);
		
		gtk_signal_connect (GTK_OBJECT (draw_area), "button_release_event",
				    (GtkSignalFunc) ExtendEnd, html);
		
		events = gtk_widget_get_events (draw_area);
		gtk_widget_set_events (draw_area, events
				       | GDK_EXPOSURE_MASK
				       | GDK_FOCUS_CHANGE_MASK
				       | GDK_POINTER_MOTION_MASK
				       | GDK_LEAVE_NOTIFY_MASK
				       | GDK_ENTER_NOTIFY_MASK
				       | GDK_BUTTON_RELEASE_MASK
				       | GDK_BUTTON_PRESS_MASK

				       /* not yet handled */
				       | GDK_KEY_PRESS_MASK
				       | GDK_KEY_RELEASE_MASK);

		gtk_widget_show (draw_area);
	}

	/*
	 * Scrollbars for the widget
	 */
	if (html->html.hsb == NULL){
		html->hsba = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
		html->html.hsb = gtk_hscrollbar_new (GTK_ADJUSTMENT (html->hsba));
		gtk_xmhtml_manage (GTK_CONTAINER(html), GTK_WIDGET (html->html.hsb));
		gtk_widget_show (html->html.hsb);
		gtk_signal_connect (GTK_OBJECT(html->hsba), "value_changed",
				    (GtkSignalFunc) horizontal_scroll, html);
	}
	if (html->html.vsb == NULL){
		html->vsba = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
		html->html.vsb = gtk_vscrollbar_new (GTK_ADJUSTMENT (html->vsba));
		gtk_xmhtml_manage (GTK_CONTAINER(html), GTK_WIDGET (html->html.vsb));
		gtk_widget_show (html->html.vsb);
		gtk_signal_connect (GTK_OBJECT(html->vsba), "value_changed",
				    (GtkSignalFunc) vertical_scroll, html);
	}

}

/*****
 * Name: 		CreateHTMLWidget
 * Return Type: 	void
 * Description: 	creates the HTML TWidget
 *			The actual area we use to draw into is a drawingAreaWidget.
 * In: 
 *	html:		TWidget to be created.
 * Returns:
 *	nothing
 *****/
static void
CreateHTMLWidget(XmHTMLWidget html)
{
	int vsb_width, hsb_height;

	_XmHTMLDebug(1, ("XmHTML.c: CreateHTMLWidget Start\n"));

	/* 
	* subtract margin_width once to minimize number of calcs in
	* the paint routines: every thing rendered starts at an x position
	* of margin_width.
	*/
	GetScrollDim(html, &hsb_height, &vsb_width);

	html->html.work_width = GTK_WIDGET (html)->allocation.width - html->html.margin_width-vsb_width;
	html->html.work_height= GTK_WIDGET (html)->allocation.height;
	
	_XmHTMLDebug(1, ("XmHTML.c: CreateHTMLWidget End\n"));
	return;
}

static void
gtk_xmhtml_realize (GtkWidget *widget)
{
	GdkWindowAttr attributes;
	gint attributes_mask;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_XMHTML (widget));
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK;
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
	
	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, 
					 attributes_mask);
	gdk_window_set_user_data (widget->window, widget);
	
	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
gtk_xmhtml_unrealize (GtkWidget *widget)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_XMHTML (widget));

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_REALIZED | GTK_MAPPED);

	gtk_style_detach (widget->style);
	gdk_window_destroy (widget->window);
	widget->window = NULL;
}

void
gtk_xmhtml_manage (GtkContainer *container, GtkWidget *widget)
{
	GtkXmHTML *html;
	g_return_if_fail (container != NULL);
	g_return_if_fail (widget != NULL);

	html = GTK_XMHTML (container);
	gtk_widget_set_parent (widget, GTK_WIDGET (container));

	if (GTK_WIDGET_REALIZED (html) && !GTK_WIDGET_REALIZED (widget))
		gtk_widget_realize (widget);

	if (GTK_WIDGET_MAPPED (html) && !GTK_WIDGET_MAPPED (widget))
		gtk_widget_map (widget);

	if (GTK_WIDGET_VISIBLE (html) && GTK_WIDGET_VISIBLE (widget))
		gtk_widget_queue_resize (GTK_WIDGET (html));
}

/*
 * Add a widget to this container, used by the forms code
 */
static void
gtk_xmhtml_add (GtkContainer *container, GtkWidget *widget)
{
	GtkXmHTML *html = GTK_XMHTML (container);
	
	gtk_xmhtml_manage (container, widget);
	html->children = g_list_append (html->children, widget);
}

/*
 * Remove a widget to this container, used by the forms code
 */
static void
gtk_xmhtml_remove (GtkContainer *container, GtkWidget *widget)
{
	GtkXmHTML *html = GTK_XMHTML (container);
	GList *children;
	GtkWidget *child;
	
	children = html->children;
	while (children){
		child = children->data;
		if (child == widget){
			gtk_widget_unparent (widget);
			
			html->children = g_list_remove_link (html->children, children);
			g_list_free (children);
			g_free (child);

			if (GTK_WIDGET_VISIBLE (container))
				gtk_widget_queue_resize (GTK_WIDGET (container));
			
			break;
		}
		children = children->next;
	}
}

static void
gtk_xmhtml_forall (GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
	GtkXmHTML *html;
	
	g_return_if_fail (container != NULL);
	g_return_if_fail (GTK_IS_XMHTML (container));
	g_return_if_fail (callback != NULL);

	html = (GtkXmHTML *) container;

	if (include_internals)
	  {
	    /* First the scrollbars */
	    (*callback)(html->html.vsb, callback_data);
	    (*callback)(html->html.hsb, callback_data);
	    
	    /* The drawing area */
	    (*callback)(html->html.work_area, callback_data);
	  }
	
	/* Any childs */
	g_list_foreach (html->children, (GFunc) callback, callback_data);
}

static void
gtk_map_item (GtkWidget *w)
{
	if (GTK_WIDGET_VISIBLE (w) && !GTK_WIDGET_MAPPED (w))
		gtk_widget_map (w);
}

static void
gtk_xmhtml_map (GtkWidget *widget)
{
	GtkXmHTML *html = GTK_XMHTML (widget);
	GtkWidget *scrollbar;
	GList     *children;
	int i;
	
	g_return_if_fail (widget != NULL);

	GTK_WIDGET_SET_FLAGS(widget, GTK_MAPPED);
	
	if (GTK_WIDGET_CLASS (parent_class)->map)
		(*GTK_WIDGET_CLASS (parent_class)->map)(widget);

	gtk_map_item (html->html.work_area);
	_XmHTMLDebug(1, ("XmHTML.c: Mapped start\n"));
	_XmHTMLDebug(1, ("XmHTML.c: Mapped, work area dimensions: %ix%i\n",
		html->html.work_width, html->html.work_height));

	CheckGC (html);

	scrollbar = html->html.vsb;
	
	html->html.work_height = widget->allocation.height;
	html->html.work_width = widget->allocation.width - 
		(html->html.margin_width + (scrollbar ? scrollbar->allocation.width : 0));

	_XmHTMLDebug(1, ("XmHTML.c: Mapped, new work area dimensions: %ix%i\n",
			 html->html.work_width, html->html.work_height));

	/* configure the scrollbars, will also resize work_area */
	CheckScrollBars (html);
	gtk_map_item (html->html.vsb);
	gtk_map_item (html->html.hsb);
	
	/* Frames */
	for (i = 0; i < html->html.nframes; i++)
		gtk_map_item (html->html.frames [i]->frame);

	/* Form widgets */
	for (children = html->children; children; children = children->next)
		gtk_map_item  (GTK_WIDGET (children->data));

	Layout(html);
	
	_XmHTMLDebug(1, ("XmHTML.c: Mapped end.\n"));
}

void
gtk_xmhtml_set_geometry (GtkWidget *widget, int x, int y, int width, int height)
{
	GtkAllocation allocation;
	
	allocation.x = x;
	allocation.y = y;
	allocation.width = width;
	allocation.height = height;
	
	gtk_widget_size_allocate (widget, &allocation);
}

/*****
* Name: 		CheckScrollBars
* Return Type: 	void
* Description: 	(re)configures scrollbars
* In: 
*	html:		HTML TWidget to configure
* Returns:
*	nothing.
*****/
static void
CheckScrollBars(XmHTMLWidget html)
{
	XmHTMLfont *f = html->html.default_font ? html->html.default_font : NULL;
	int dx, dy, hsb_height, vsb_width, st, nx, ny, sx, sy;
	int hsb_on_top, vsb_on_left;
	/* forced display of scrollbars: XmSTATIC or frames with scrolling = yes */
	int force_vsb = FALSE, force_hsb = FALSE;
	GtkAdjustment *hsba = GTK_ADJUSTMENT (html->hsba);
	GtkAdjustment *vsba = GTK_ADJUSTMENT (html->vsba);

	/* don't do a thing if we aren't managed yet */
	if (!GTK_WIDGET_MAPPED (html))
		return;

	/* Initial work area offset */
	st = dx = dy = 0;
	sx = Toolkit_Widget_Dim (html).x;
	sy = Toolkit_Widget_Dim (html).y;
	GetScrollDim (html, &hsb_height, &vsb_width);

	/* 1. Vertical scrollbar */
	
 	/*    check if we need a vertical scrollbar */
	if(html->html.formatted_height < Toolkit_Widget_Dim (html).height){
		html->html.needs_vsb = False;
		/* don't forget! */
		html->html.scroll_y = 0;
		gtk_widget_hide (html->html.vsb);
	} else
		html->html.needs_vsb = TRUE;

	/*     FIXME: Here: should we support Scrollbar policies, ie, always present,
	 *     or only on demand?  The XmHTML Motif code supports it.
	 */

	/* 2. Horizontal scrollbar */
	/*
	*     check if we need a horizontal scrollbar. If we have a vertical
	*     scrollbar, we must add it's width or text might be lost.
	*/
	if(html->html.formatted_width < Toolkit_Widget_Dim (html).width -
	   (html->html.needs_vsb ? vsb_width : 0))
	{
		html->html.needs_hsb = False;
		/* don't forget! */
		html->html.scroll_x = 0;
		gtk_widget_hide (html->html.hsb);
	} else
		html->html.needs_hsb = TRUE;

	/*     FIXME: same.  Should we support scrollbar policies? */

	/* if this is a frame, check what type of scrolling is requested */
	if(html->html.is_frame)
	{
		if(html->html.scroll_type == FRAME_SCROLL_NONE)
		{
			html->html.needs_hsb = False;
			html->html.needs_vsb = False;
			html->html.scroll_x = 0;
			html->html.scroll_y = 0;
			gtk_widget_hide (html->html.hsb);
			gtk_widget_hide (html->html.vsb);
		}
		else if(html->html.scroll_type == FRAME_SCROLL_YES)
		{
			html->html.needs_vsb = TRUE;
			html->html.needs_hsb = TRUE;
			force_vsb = TRUE;
			force_hsb = TRUE;
		}
		/* else scrolling is auto, just proceed */
	}

	/* return if we don't need any scrollbars */
	if(!html->html.needs_hsb && !html->html.needs_vsb)
	{
		_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, end, no bars needed.\n"));

		gtk_xmhtml_set_geometry (html->html.work_area, sx+dx, sy+dy,
					 Toolkit_Widget_Dim (html).width,
					 Toolkit_Widget_Dim (html).height);
		return;
	}

	/* For now: we dont support this */
	hsb_on_top = 0;
	vsb_on_left = 0;

	/* horizontal sb on top */
	if(html->html.needs_hsb && hsb_on_top)
		dy += hsb_height;

	/* vertical sb on left */
	if(html->html.needs_vsb && vsb_on_left)
		dx += vsb_width;

	nx = dx;
	ny = dy;

	/* See what space we have to reserve for the scrollbars */
	if(html->html.needs_hsb && hsb_on_top == FALSE)
		dy += hsb_height;
	if(html->html.needs_vsb && vsb_on_left == FALSE)
		dx += vsb_width;

	gtk_xmhtml_set_geometry (html->html.work_area, sx+nx, sy+ny,
			      Toolkit_Widget_Dim (html).width - dx,
			      Toolkit_Widget_Dim (html).height - dy);

	if(html->html.needs_hsb == TRUE)
	{
		int pinc;
		int sb_width, sb_height;
		GtkRequisition child_requisition;
		
		_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, setting hsb\n"));

		/* Set hsb size; adjust x-position if we have a vsb */
		dx = (html->html.needs_vsb ? vsb_width : 0);
		
		sb_width  = Toolkit_Widget_Dim (html).width - dx - 2*st;
		gtk_widget_get_child_requisition (html->html.hsb, &child_requisition);
		sb_height = child_requisition.height;

		/* pageIncrement == sliderSize */
		pinc = html->html.work_width - 2*XmHTML_HORIZONTAL_SCROLL_INCREMENT;
		/* sanity check */
		if(pinc < 1)
			pinc = XmHTML_HORIZONTAL_SCROLL_INCREMENT;

		/* adjust horizontal scroll if necessary */
		if (html->html.scroll_x > html->html.formatted_width - pinc)
			html->html.scroll_x = html->html.formatted_width - pinc;

		/*
		* Adjust if a horizontal scrollbar has been forced
		* (can only happen for frames with scrolling = yes)
		*/
		if(force_hsb && pinc > html->html.formatted_width)
		{
			pinc = html->html.formatted_width;
			html->html.scroll_x = 0;
		}

		hsba->upper          = (gfloat) html->html.formatted_width;
		hsba->value          = (gfloat) html->html.scroll_x;
		hsba->page_size      = (gfloat) pinc;
		hsba->page_increment = (gfloat) pinc;
		hsba->step_increment = (f ? f->width : XmHTML_HORIZONTAL_SCROLL_INCREMENT);
		gtk_signal_emit_by_name (GTK_OBJECT (html->hsba), "changed");
		
		/* adjust x-position if vsb is on left */
 		dx = (html->html.needs_vsb && vsb_on_left ? vsb_width : 0);

		/* place it */
		gtk_xmhtml_set_geometry (html->html.hsb, sx+dx,
					 sy+ (hsb_on_top ? 0 : Toolkit_Widget_Dim(html).height - hsb_height),
					 sb_width, sb_height);
		gtk_widget_show (html->html.hsb);
	}
	if(html->html.needs_vsb == TRUE)
	{
		int pinc;
		int sb_width, sb_height;
		GtkRequisition child_requisition;
		
		_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, setting vsb\n"));

		/* Set vsb size; adjust y-position if we have a hsb */
		dy = (html->html.needs_hsb ? hsb_height : 0);
		gtk_widget_get_child_requisition (html->html.vsb, &child_requisition);
		sb_width  = child_requisition.width;
		sb_height = Toolkit_Widget_Dim (html).height - dy - 2*st;

		/* pageIncrement == sliderSize */
		pinc = html->html.work_height - 2*(html->html.default_font ? 
			html->html.default_font->height : XmHTML_VERTICAL_SCROLL_INCREMENT);
		/* sanity check */
		if(pinc < 1)
			pinc = XmHTML_VERTICAL_SCROLL_INCREMENT;

		/* adjust vertical scroll if necessary */
		if(html->html.scroll_y > html->html.formatted_height - pinc)
			html->html.scroll_y = html->html.formatted_height - pinc;

		/*
		* Adjust if a vertical scrollbar has been forced
		* (can only happen if scrollBarDisplayPolicy == XmSTATIC)
		*/
		if(force_vsb && pinc > html->html.formatted_height)
		{
			pinc = html->html.formatted_height;
			html->html.scroll_y = 0;
		}

		vsba->upper          = (gfloat) html->html.formatted_height;
		vsba->value          = (gfloat) html->html.scroll_y;
		vsba->page_size      = (gfloat) pinc;
		vsba->page_increment = (gfloat) pinc;
		vsba->step_increment = (html->html.default_font
					 ? html->html.default_font->height
					: XmHTML_VERTICAL_SCROLL_INCREMENT);
		gtk_signal_emit_by_name (GTK_OBJECT (html->vsba), "changed");
		
		/* adjust y-position if hsb is on top */
 		dy = (html->html.needs_hsb && hsb_on_top ? hsb_height : 0);

		gtk_xmhtml_set_geometry (html->html.vsb, sx + 
					 (vsb_on_left ? 0 : Toolkit_Widget_Dim(html).width - vsb_width),
					 sy, sb_width, sb_height);
		gtk_widget_show (html->html.vsb);
	}
	_XmHTMLDebug(1, ("XmHTML.c: CheckScrollBars, end\n"));

}

static void
XmHTML_Frontend_Redisplay (XmHTMLWidget html)
{
	gtk_widget_draw (GTK_WIDGET (html), NULL);

	if (GTK_WIDGET_MAPPED (html->html.vsb))
		/* update_display html->html.vsb */;
	if (GTK_WIDGET_MAPPED (html->html.hsb))
		/* update_display html->html.hsb */;
}

/*****
* Name: 		GetScrollDim
* Return Type: 	void
* Description: 	retrieves width & height of the scrollbars
* In: 
*	html:		XmHTMLWidget for which to retrieve these values
*	hsb_height: thickness of horizontal scrollbar, filled upon return
*	vsb_width:	thickness of vertical scrollbar, filled upon return
* Returns:
*	nothing
*****/
static void
GetScrollDim(XmHTMLWidget html, int *hsb_height, int *vsb_width)
{
	int height = 0, width = 0;
	GtkRequisition child_requisition;

	if(html->html.hsb) {
		gtk_widget_get_child_requisition (html->html.hsb, &child_requisition);
		height = child_requisition.height;
	}
	
	/*
	 * Sanity check if the scrollbar dimensions exceed the TWidget dimensions
	 * Not doing this would lead to X Protocol errors whenever text is set
	 * into the TWidget: the size of the workArea will be less than or equal
	 * to zero when scrollbars are required.
	 * We need always need to do this check since it's possible that some
	 * user has been playing with the dimensions of the scrollbars.
	 */
	if(height >= GTK_WIDGET (html)->allocation.height){
		_XmHTMLWarning(__WFUNC__(html->html.hsb, "GetScrollDim"),
			       "Height of horizontal scrollbar (%i) exceeds height of parent "
			       "TWidget (%i).\n    Reset to 15.", height,
			       GTK_WIDGET(html)->allocation.height);
		height = 15;
		/*
		fprintf (stderr, "Die boy boy die\n");
		exit (1);
		*/
	}
	
	if(html->html.vsb){
		gtk_widget_get_child_requisition (html->html.vsb, &child_requisition);
		width = child_requisition.width;
		if(width >= GTK_WIDGET (html)->allocation.width){
			_XmHTMLWarning(__WFUNC__(html->html.vsb, "GetScrollDim"),
				"Width of vertical scrollbar (%i) exceeds width of parent "
				"TWidget (%i).\n    Reset to 15.", width,
				       GTK_WIDGET (html)->allocation.width);
			width  = 15;
			fprintf (stderr, "Die vertical boy boy die!\n");
			exit (1);
		}
	}

	_XmHTMLDebug(1, ("XmHTML.c: GetScrollDim; height = %i, width = %i\n",
		height, width));

	*hsb_height = height;
	*vsb_width  = width;
}

void
_XmHTMLCheckXCC(XmHTMLWidget html)
{
	GtkWidget *htmlw = GTK_WIDGET (html);
	
	_XmHTMLDebug(1, ("XmHTML.c: _XmHTMLCheckXCC Start\n"));
	/*
	* CheckXCC is called each time an image is loaded, so it's quite
	* usefull if we have a GC around by the time the TWidget is being
	* mapped to the display.
	* Our SubstructureNotify event handler can fail in some cases leading to
	* a situation where we don't have a GC when images are about to be
	* rendered (especially background images can cause a problem, they
	* are at the top of the text).
	*/
	CheckGC (html);

	/*
	* Create an XCC. 
	* XmHTML never decides whether or not to use a private or standard
	* colormap. A private colormap can be supplied by setting it on the
	* TWidget's parent, we know how to deal with that.
	*/
	if(!html->html.xcc)
	{
		TVisual *visual = NULL;
		TColormap cmap;

		_XmHTMLDebug(1, ("XmHTML.c: _XmHTMLCheckXCC: creating an XCC\n"));

		cmap   = gtk_widget_get_colormap (htmlw);
		visual = gtk_widget_get_visual (htmlw);

		/* walk TWidget tree or get default visual */
		if(visual == NULL){
			/* visual = XCCGetParentVisual((TWidget)html);*/
		  fprintf(stderr,
			  "XmHTML.c: _XmHTMLCheckXCC:: should not happen\n");
		  exit (1);
		}
		/* create an xcc for this TWidget */
		html->html.xcc = XCCCreate((TWidget)html, visual, cmap);
	}
	
	_XmHTMLDebug(1, ("XmHTML.c: _XmHTMLCheckXCC End\n"));
}

/*****
* Name: 		autoSizeWidget
* Return Type: 	void
* Description: 	computes XmHTML's TWidget dimensions if we have to autosize
*				in either direction.
* In: 
*	html:		XmHTMLWidget id
* Returns:
*	nothing.
* Note:
*	This routine is fairly complicated due to the fact that the dimensions
*	of the work area are partly determined by the presence of possible
*	scrollbars.
*****/
static void
autoSizeWidget (XmHTMLWidget html)
{
}

/* XmImage configuration hook */
XmImageConfig *_xmimage_cfg;

/*
 * Configurability of the XmHTML widget 
 * 
 */

static void
gtk_xmhtml_sync_parse (GtkXmHTML *html)
{
	int ov;
	
	_XmHTMLKillPLCCycler (html);

	/* new text has been set, kill of any existing PLC's */
	_XmHTMLKillPLCCycler(html);
	
	/* release event database */
	_XmHTMLFreeEventDatabase(html, html);
	
	/* destroy any form data */
	_XmHTMLFreeForm(html, html->html.form_data);
	html->html.form_data = (XmHTMLFormData*)NULL;
	
	/* Parse the raw HTML text */
	ov = html->html.in_layout;
	html->html.elements = _XmHTMLparseHTML(html, html->html.elements, html->html.source, html);
	html->html.in_layout = ov;
	
	/* reset topline */
	html->html.top_line = 0;
	
	/* keep current frame setting and check if new frames are allowed */
	html->html.nframes = _XmHTMLCheckForFrames(html, html->html.elements);
	
	/* Trigger link callback */
	if(CHECK_CALLBACK (html, link_callback, LINK))
		_XmHTMLLinkCallback(html);

	html->reformat_needed   = TRUE;
	html->redraw_needed     = TRUE;
	html->free_images_needed = TRUE;
}

static void
gtk_xmhtml_sync_reformat (GtkXmHTML *html)
{
	int is_frame;
	
	/*
	 * Now format the list of parsed objects.
	 * Don't do a bloody thing if we are already in layout as this will
	 * cause unnecessary reloading and screen flickering.
	 */
	if (html->html.in_layout)
		return;

	/*****
	 * It the current document makes heavy use of images we first need
	 * to clear it. Not doing this would cause a shift in the colors of 
	 * the current document (as they are being released) which does not 
	 * look nice. Therefore first clear the entire display* area *before* 
	 * freeing anything at all.
	 *****/
	if(html->html.gc != NULL){
		Toolkit_Clear_Area (Toolkit_Display (html->html.work_area), 
				    Toolkit_Widget_Window(html->html.work_area),
				    0, 0,
				    Toolkit_Widget_Dim(html).width,
				    Toolkit_Widget_Dim(html).height, False);
	}
	
	/* destroy any form data */
	_XmHTMLFreeForm(html, html->html.form_data);
	html->html.form_data = (XmHTMLFormData*)NULL;

	/* Free all non-persistent resources */
	FreeExpendableResources(html, html->free_images_needed);

	/* reset some important vars */
	ResetWidget(html, html->free_images_needed);

	{
		/* FIXME: is setting the window background semantically equivalent
		 * to setting the XmNbackground resource on the work_area?
		 */

		GdkColor c;
		c.pixel = html->html.body_bg;
		gdk_window_set_background(html->html.work_area->window, &c);
	}

	/* get new values for top, bottom & highlight */
	_XmHTMLRecomputeColors(html);

	is_frame = html->html.is_frame;

	/* go and format the parsed HTML data */
	if(!_XmHTMLCreateFrames(html, html)){
		html->html.frames = NULL;
		html->html.nframes = 0;
		html->html.is_frame = is_frame;
	}
	
	_XmHTMLformatObjects(html, html);

	/* and check for possible external imagemaps */
	_XmHTMLCheckImagemaps(html);

	/* compute new screen layout */
	Layout(html);
	
	/* if new text has been set, fire up the PLCCycler */
	if(html->parse_needed){
		html->html.plc_suspended = False;
		_XmHTMLPLCCycler((TPointer)html);
	}
	html->free_images_needed = 0;
	html->redraw_needed = 1;
	html->layout_needed = 0;
	
}

static void
gtk_xmhtml_sync_redraw (GtkXmHTML *html)
{
	if (html->free_images_needed){
			XmHTMLImage *img;
			for(img = html->html.images; img != NULL; img = img->next)
			{
				if(!ImageInfoFreed(img) &&
					ImageInfoDelayedCreation(img->html_image))
				{
					img->options |= IMG_DELAYED_CREATION;
					html->html.delayed_creation = True;
				}
			}
			if(html->html.delayed_creation)
				_XmHTMLImageCheckDelayedCreation(html);
	}
	if (html->html.gc != NULL)
		_XmHTMLClearArea (html, 0, 0,
				  Toolkit_Widget_Dim (html).width,
				  Toolkit_Widget_Dim (html).height);
	gtk_widget_draw (GTK_WIDGET (html), NULL);
}

static void
gtk_xmhtml_sync (GtkXmHTML *html)
{
	if (!html->initialized){
		html->initialized = 1;
		XmHTML_Initialize (html, html, html->html.source);
	}
	if (html->parse_needed)
		gtk_xmhtml_sync_parse (html);
	if (html->reformat_needed)
		gtk_xmhtml_sync_reformat (html);
	if (html->redraw_needed)
		gtk_xmhtml_sync_redraw (html);

	/* FIXME: compute the need_tracking variable depending on the following settings:
	 * anchor_track_callback
	 * anchor_cursor
	 * highlight_on_enter
	 * motion_track_callback
	 * focus_callback
	 * losing_focus_callback
	 */
	gtk_xmhtml_reset_pending_flags (html);
}


static void
gtk_xmhtml_try_sync (GtkXmHTML *html)
{
	if (html->frozen)
		return;
	gtk_xmhtml_sync (html);
}

void
gtk_xmhtml_freeze (GtkXmHTML *html)
{
	html->frozen++;
}

void
gtk_xmhtml_thaw (GtkXmHTML *html)
{
	if (!html->frozen)
		return;

	html->frozen--;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_source (GtkXmHTML *html, char *html_source)
{
	int parse = FALSE;

	/* If we already have some HTML source code */
	if (html->html.source){
		if (html_source){	/* new text supplied */
			if (strcmp (html_source, html->html.source)){
				parse = TRUE;
				free (html->html.source);
				html->html.source = strdup (html_source);
			} else
				parse = FALSE;
		} else {	/* have to clear current text */
			parse = TRUE;
			free (html->html.source);
			html->html.source = NULL;
		}
	} else { 		/* we did not have any source */
		if (html_source){
			parse = TRUE;
			html->html.source = strdup (html_source);
		} else
			parse = FALSE; /* still empty */
	}
	html->html.value = html->html.source;
	html->parse_needed = parse;
	
	if (html->initialized)
		gtk_xmhtml_try_sync (html);
}

/* XXX: This function does an XQueryColors() the hard way, because there is
 * no corresponding function in Gdk.
 */

void
my_x_query_colors(GdkColormap *colormap,
		  GdkColor    *colors,
		  gint         ncolors)
{
	XColor *xcolors;
	gint    i;

	xcolors = g_new(XColor, ncolors);
	for (i = 0; i < ncolors; i++)
		xcolors[i].pixel = colors[i].pixel;

	XQueryColors(gdk_display, GDK_COLORMAP_XCOLORMAP(colormap), xcolors, ncolors);

	for (i = 0; i < ncolors; i++) {
		colors[i].red   = xcolors[i].red;
		colors[i].green = xcolors[i].green;
		colors[i].blue  = xcolors[i].blue;
	}

	g_free(xcolors);
}

void
gtk_xmhtml_set_string_direction (GtkXmHTML *html, int direction)
{
	if (html->html.string_direction == direction)
		return;
	html->html.string_direction = direction;
	html->parse_needed  = 1;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_alignment (GtkXmHTML *html, int alignment)
{
	if (html->html.enable_outlining)
		html->html.default_halign = XmHALIGN_JUSTIFY;
	else {
		/* default alignment depends on string direction */
		if(html->html.string_direction == TSTRING_DIRECTION_R_TO_L)
			html->html.default_halign = XmHALIGN_RIGHT;
		else
			html->html.default_halign = XmHALIGN_LEFT;

		html->html.alignment = alignment;
		if(alignment == TALIGNMENT_BEGINNING)
			html->html.default_halign = XmHALIGN_LEFT;
		if(alignment == TALIGNMENT_END)
			html->html.default_halign = XmHALIGN_RIGHT;
		else if(alignment == TALIGNMENT_CENTER)
			html->html.default_halign = XmHALIGN_CENTER;
	}
	html->parse_needed  = 1;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_outline (GtkXmHTML *html, int flag)
{
	html->html.enable_outlining = flag;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

static void
gtk_xmhtml_fonts_changed (GtkXmHTML *html)
{
	html->html.default_font = _XmHTMLSelectFontCache (html, TRUE);
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_font_familty (GtkXmHTML *html, char *family, char *sizes)
{
	g_free (html->html.font_family);
	g_free (html->html.font_sizes);
	html->html.font_family = g_strdup (family);
	html->html.font_sizes = g_strdup (sizes);
	gtk_xmhtml_fonts_changed (html);
}

void
gtk_xmhtml_set_font_familty_fixed (GtkXmHTML *html, char *family, char *sizes)
{
	g_free (html->html.font_family_fixed);
	g_free (html->html.font_sizes);
	html->html.font_family_fixed = g_strdup (family);
	html->html.font_sizes_fixed = g_strdup (sizes);
	gtk_xmhtml_fonts_changed (html);
}

void
gtk_xmhtml_set_font_charset (GtkXmHTML *html, char *charset)
{
	g_free (html->html.charset);
	html->html.charset = g_strdup (charset);
	gtk_xmhtml_fonts_changed (html);
}

void
gtk_xmhtml_set_allow_body_colors (GtkXmHTML *html, int enable)
{
	if (enable == html->html.body_colors_enabled)
		return;

	html->html.body_fg             = html->html.body_fg_save;
	html->html.body_bg             = html->html.body_bg_save;
	html->html.anchor_fg           = html->html.anchor_fg_save;
	html->html.anchor_visited_fg   = html->html.anchor_visited_fg_save;
	html->html.anchor_activated_fg = html->html.anchor_activated_fg_save;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_colors (GtkXmHTML *html,
		       Pixel foreground,
		       Pixel background,
		       Pixel anchor_fg,
		       Pixel anchor_target_fg,
		       Pixel anchor_visited_fg,
		       Pixel anchor_activated_fg,
		       Pixel anchor_activated_bg)
{
	/* FIXME: Not so sure about the api, what to do? */
	/* FIXME; put the code to update our colors with the new values here */
	_XmHTMLRecomputeColors (html);
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_hilight_on_enter (GtkXmHTML *html, int flag)
{
	html->html.armed_anchor = NULL;
	html->html.highlight_on_enter = flag;
}

static void
gtk_xmhtml_check_underline_type (int underline_type, int *type, int *value)
{
	switch (underline_type){
	case GTK_ANCHOR_NOLINE:
		*value = NO_LINE;
		break;
	case GTK_ANCHOR_DASHED_LINE:
		*value = LINE_DASHED|LINE_UNDER|LINE_SINGLE;
		break;
	case GTK_ANCHOR_DOUBLE_LINE:
		*value = LINE_SOLID|LINE_UNDER|LINE_DOUBLE;;
		break;
	case GTK_ANCHOR_DOUBLE_DASHED_LINE:
		*value = LINE_DASHED|LINE_UNDER|LINE_DOUBLE;;
		break;
	case GTK_ANCHOR_SINGLE_LINE:
	default:
		*value = LINE_SOLID | LINE_UNDER | LINE_SINGLE;
		underline_type = GTK_ANCHOR_SINGLE_LINE;
		break;
	}
	*type = underline_type;
}

void
gtk_xmhtml_set_anchor_underline_type (GtkXmHTML *html, int underline_type)
{
	int type, value;
	
	gtk_xmhtml_check_underline_type (underline_type, &type, &value);
	html->html.anchor_underline_type = type;
	html->html.anchor_line = value;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_anchor_visited_underline_type (GtkXmHTML *html, int underline_type)
{
	int type, value;
	
	gtk_xmhtml_check_underline_type (underline_type, &type, &value);
	html->html.anchor_visited_underline_type = type;
	html->html.anchor_visited_line = value;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_anchor_target_underline_type (GtkXmHTML *html, int underline_type)
{
	int type, value;
	
	gtk_xmhtml_check_underline_type (underline_type, &type, &value);
	html->html.anchor_target_underline_type = type;
	html->html.anchor_target_line = value;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_allow_color_switching (GtkXmHTML *html, int flag)
{
	if (html->html.allow_color_switching == flag)
		return;
	html->html.allow_color_switching = flag;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_allow_font_switching (GtkXmHTML *html, int flag)
{
	if (html->html.allow_font_switching == flag)
		return;
	html->html.allow_font_switching = flag;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

/* See documentation for XmNimageMapToPalette/XmNimageRGBConversion for possible values */
void
gtk_xmhtml_set_dithering (GtkXmHTML *html, XmHTMLDitherType flag)
{
	if (html->html.map_to_palette == flag)
		return;

	/* from on to off or off to on */
	if (html->html.map_to_palette == XmDISABLED || flag == XmDISABLED ){
		/* free current stuff */
		XCCFree(html->html.xcc);
		
		/* and create a new one */
		html->html.xcc = NULL;
		_XmHTMLCheckXCC (html);
		
		/* add palette if necessary */
		if(flag != XmDISABLED)
			_XmHTMLAddPalette(html);
	} else {
		/* fast & best methods require precomputed error matrices */
		if(flag == XmBEST || flag == XmFAST)
			XCCInitDither(html->html.xcc);
		else
			XCCFreeDither(html->html.xcc);
	}
	html->html.map_to_palette = flag;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_max_image_colors (GtkXmHTML *html, int max_colors)
{
	int prev_max = html->html.max_image_colors;
	int new_max  = max_colors;
	int free_images = html->free_images_needed;
	
	html->html.max_image_colors = max_colors;
	CheckMaxColorSetting(html);
	
	/*
	 * check if we have any images with more colors than allowed or
	 * we had images that were dithered. If so we need to redo the layout
	 */
	if(html->reformat_needed){
		XmHTMLImage *image;
		
		for(image = html->html.images; image != NULL && !free_images; image = image->next){
			/* ImageInfo is still available. Compare against it */
			if(!ImageInfoFreed(image))
			{
				/*
				 * redo image composition if any of the following
				 * conditions is True:
				 * - current image has more colors than allowed;
				 * - current image has less colors than allowed but the
				 *	original image had more colors than allowed previously.
				 */
				if(image->html_image->ncolors > new_max ||
				   (image->html_image->scolors < new_max &&
				    image->html_image->scolors > prev_max))
					html->free_images_needed = True;
			}
				/* info no longer available. Check against allocated colors */
			else
				if(image->npixels > new_max)
					free_images = True;
		}
		/* need to redo the layout if we are to redo the images */
		html->reformat_needed = free_images;
	}
	html->free_images_needed = free_images;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_allow_images (GtkXmHTML *html, int flag)
{
	if (html->html.images_enabled == flag)
		return;
	html->html.images_enabled = flag;
	html->free_images_needed = 1;
	html->reformat_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_plc_intervals (GtkXmHTML *html, int min_delay, int max_delay, int def_delay)
{
	if ((html->html.plc_min_delay == min_delay) &&
	    (html->html.plc_max_delay == max_delay) &&
	    (html->html.plc_delay == def_delay))
		return;
	html->html.plc_min_delay = min_delay;
	html->html.plc_max_delay = max_delay;
	html->html.plc_def_delay = def_delay;
	html->html.plc_delay     = def_delay;
	CheckPLCIntervals (html);
}

void
gtk_xmhtml_set_def_body_image_url (GtkXmHTML *html, char *url)
{
	/* FIXME, need to write this routien based on the Motif code */
}

void
gtk_xmhtml_set_anchor_buttons (GtkXmHTML *html, int flag)
{
	if (html->html.anchor_buttons == flag)
		return;
	html->html.anchor_buttons = flag;
	html->redraw_needed = 1;
	gtk_xmhtml_try_sync (html);
}

void
gtk_xmhtml_set_anchor_cursor (GtkXmHTML *html, GdkCursor *cursor, int flag)
{
	html->html.anchor_display_cursor = flag;
	if (!flag){
		Toolkit_Free_Cursor (Toolkit_Display(html), html->html.anchor_cursor);
		html->html.anchor_cursor = None;
		return;
	}
	html->html.anchor_cursor = (TCursor) cursor;
}

void
gtk_xmhtml_set_topline (GtkXmHTML *html, int line)
{
	html->html.top_line = line;
	ScrollToLine (html, line);
	html->redraw_needed = 1;
	gtk_xmhtml_try_sync (html);
}

int
gtk_xmhtml_get_topline (GtkXmHTML *html)
{
	return html->html.top_line;
}

/* FIXME: should we support setting the scroll repeat value?  Motif code does it */

void
gtk_xmhtml_set_freeze_animations (GtkXmHTML *html, int flag)
{
	if (html->html.freeze_animations == flag)
		return;

	_XmHTMLRestartAnimations (html);
}

void
gtk_xmhtml_set_screen_gamma (GtkXmHTML *html, float screen_gamma)
{
	html->html.screen_gamma = screen_gamma;
}

/* Use for progressive image loading */
void
gtk_xmhtml_set_image_procs (GtkXmHTML         *html,
			    XmImageProc       image_proc,
			    XmImageGifProc    gif_proc,
			    XmHTMLGetDataProc get_data,
			    XmHTMLEndDataProc end_data)
{
	html->html.image_proc = image_proc;
	html->html.gif_proc   = gif_proc;
	html->html.get_data = get_data;
	html->html.end_data = end_data;
}

void
gtk_xmhtml_set_event_proc (GtkXmHTML *html, XmHTMLEventProc event_proc)
{
	html->html.event_proc = event_proc;
}

void
gtk_xmhtml_set_perfect_colors (GtkXmHTML *html, int flag)
{
	html->html.perfect_colors = flag;
}

void
gtk_xmhtml_set_uncompress_command (GtkXmHTML *html, char *cmd)
{
	g_free (html->html.zCmd);
	html->html.zCmd = g_strdup (cmd);
}

void
gtk_xmhtml_set_strict_checking (GtkXmHTML *html, int flag)
{
	html->html.strict_checking = flag;
}

void
gtk_xmhtml_set_bad_html_warnings (GtkXmHTML *html, int flag)
{
	html->html.bad_html_warnings = flag;
}

void
gtk_xmhtml_set_allow_form_coloring (GtkXmHTML *html, int flag)
{
	html->html.allow_form_coloring = flag;
}

void
gtk_xmhtml_set_imagemap_draw (GtkXmHTML *html, int flag)
{
	html->html.imagemap_draw = flag;
}

void
gtk_xmhtml_set_mime_type (GtkXmHTML *html, char *mime_type)
{
	g_free (html->html.mime_type);
	html->html.mime_type = g_strdup (mime_type);
}

void
gtk_xmhtml_set_alpha_processing (GtkXmHTML *html, int flag)
{
	html->html.alpha_processing = flag;
}

void
gtk_xmhtml_set_rgb_conv_mode (GtkXmHTML *html, int val)
{
	html->html.rgb_conv_mode = val;
}

/* These two functions can be removed when XmHTML is converted to use
 * 16-bit color information.  The corresponding XCCGetPixels() and
 * XCCGetPixelsIncremental() macros in toolkit.h should be changed to
 * use the un-wrapped Gdk functions.
 */

#define UPSCALE(c) (((c) << 8) | (c))

void
wrap_gdk_cc_get_pixels (int              incremental,
			GdkColorContext *cc,
			gushort         *reds,
			gushort         *greens,
			gushort         *blues,
			gint             ncolors,
			gint            *used,
			gulong          *colors,
			gint            *nallocated)
{
	gushort *wreds, *wgreens, *wblues;
	int i;

	wreds   = g_new (gushort, ncolors);
        wgreens = g_new (gushort, ncolors);
	wblues  = g_new (gushort, ncolors);

	for (i = 0; i < ncolors; i++) {
		wreds[i]   = UPSCALE (reds[i]);
		wgreens[i] = UPSCALE (greens[i]);
		wblues[i]  = UPSCALE (blues[i]);
	}

	if (incremental)
		gdk_color_context_get_pixels_incremental (cc, wreds, wgreens, wblues, ncolors, used, colors, nallocated);
	else
		gdk_color_context_get_pixels (cc, wreds, wgreens, wblues, ncolors, colors, nallocated);

	g_free (wreds);
	g_free (wgreens);
	g_free (wblues);
}

#ifndef HAVE_GTK_SIGNAL_HANDLER_PENDING
guint
gtk_signal_handler_pending (GtkObject *object, gint signal_type, gboolean may_be_blocked)
{
	static int warning_printed;
	/* On old Gtks, we return 1 and force all of the signals to do the full processing */
	if (!warning_printed){
		warning_printed = 1;
		fprintf (stderr, "GtkXmHTML widget running in slow mode due to old Gtk version\n");
	}
	return 1;
}
#endif

void
Toolkit_Draw_Shadows(XmHTMLWidget html, TGC top_shadow, TGC bottom_shadow,
		     gint xs, gint ys, gint w, gint h, gint shadow_thick, gint shadow_type)
{
	GtkWidget *win;
	win = GTK_WIDGET(html->html.work_area);
	switch (shadow_type)
	  {
	  case XmSHADOW_IN:
	    gtk_draw_shadow(GTK_WIDGET(html)->style, win->window,
			    GTK_STATE_NORMAL, GTK_SHADOW_IN,
			    xs, ys,
			    w, h);
	    break;
	  case XmSHADOW_OUT:
	    gtk_draw_shadow(GTK_WIDGET(html)->style, win->window,
			    GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			    xs, ys,
			    w, h);
	    break;
	  default:
	    break;
	    ;
	  }
}

void
_XmHTMLRecomputeShadowColors(XmHTMLWidget html, Pixel base) 
{
	/* 
	* We can only compute the colors when we have a GC. If we don't
	* have a GC, the widget is not yet realized. Use managers defaults
	* then.
	*/
#if 0
	if(html->html.gc != NULL)
	{
		Pixel top = None, bottom = None;
		Arg args[2];

		XmGetColors(XtScreen((Widget)html), html->core.colormap,
			base, NULL, &top, &bottom, NULL);
		XtSetArg(args[0], XmNtopShadowColor, top);
		XtSetArg(args[1], XmNbottomShadowColor, bottom);
		XtSetValues((Widget)html, args, 2);
	}
#endif
}
