/*
 * An embeddable "paint" component.
 *
 * FIXME: This is supposed to be an example.. describe
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 */

#include <config.h>
#include <bonobo.h>
#include <bonobo/bonobo-print.h>
#include <libgnomeprint/gnome-print.h>

/*
 * The Embeddable data.
 *
 * This is where we store the document's abstract data.  Each
 * on-screen representation of the document (a BonoboView) will be
 * based on the data in this structure.
 */
typedef struct {
	BonoboEmbeddable     *embeddable;

	/*
	 * We store the image data internally in a GdkPixmap.
	 */
	GdkPixmap	    *pixmap;
	int		     width;
	int		     height;
} embeddable_data_t;

/*
 * The per-view data.
 */
typedef struct {
	BonoboView	    *view;
	embeddable_data_t   *embeddable_data;

	/*
	 * The widget used to render this view of the image data.
	 */
	GtkWidget	    *drawing_area;

	/*
	 * The width and height of this view.
	 */
	int		     width;
	int		     height;

	/*
	 * The drawing context for this view.  Each view can have a
	 * separate current pen color, and so each view maintains its
	 * own GC.
	 */
	GdkGC		    *gc;

	/*
	 * The last known x,y position of the mouse.
	 */
	int		     last_x;
	int		     last_y;
} view_data_t;

/*
 * Clean up our supplementary BonoboEmbeddable data sturctures.
 */
static void
embeddable_destroy_cb (BonoboEmbeddable *embeddable, embeddable_data_t *embeddable_data)
{
	gdk_pixmap_unref (embeddable_data->pixmap);
	g_free (embeddable_data); 
}

/*
 * This callback is invoked when the BonoboEmbeddable object
 * encounters a fatal CORBA exception.
 */
static void
embeddable_system_exception_cb (BonoboEmbeddable *embeddable, CORBA_Object corba_object,
				CORBA_Environment *ev, gpointer data)
{
	bonobo_object_unref (BONOBO_OBJECT (embeddable));
}

/*
 * The view encounters a fatal corba exception.
 */
static void
view_system_exception_cb (BonoboView *view, CORBA_Object corba_object,
			  CORBA_Environment *ev, gpointer data)
{
	bonobo_object_unref (BONOBO_OBJECT (view));
}

/*
 * The job of this function is to update the view from the
 * embeddable's representation of the image data.
 */
static void
view_update (view_data_t *view_data)
{
	gdk_draw_pixmap (view_data->drawing_area->window,
			 view_data->gc,
			 view_data->embeddable_data->pixmap,
			 0, 0,
			 0, 0,
			 MIN (view_data->width, view_data->embeddable_data->width),
			 MIN (view_data->height, view_data->embeddable_data->height));
}

static void 
update_view_foreach (BonoboView *view, void *data)
{
	view_data_t *view_data;

	view_data = gtk_object_get_data (GTK_OBJECT (view), "view_data");
	view_update (view_data);
}

/*
 * This function updates all of an embeddable's views to reflect the
 * image data stored in the embeddable.
 */
static void
embeddable_update_all_views (embeddable_data_t *embeddable_data)
{
	BonoboEmbeddable *embeddable;

	embeddable = embeddable_data->embeddable;

	bonobo_embeddable_foreach_view (embeddable, update_view_foreach, NULL);
}

/*
 * This function sets the view's current drawing color.
 */
static void
view_set_color (view_data_t *view_data, char *color)
{
	GdkColormap *colormap;
	GdkColor gdk_color;

	fprintf (stderr, "Set color to '%s'\n", color);

	colormap = gtk_widget_get_colormap (view_data->drawing_area);

	gdk_color_parse (color, &gdk_color);
	gdk_color_alloc (colormap, &gdk_color);

	gdk_gc_set_foreground (view_data->gc, &gdk_color);
}

static void
color_listener_cb (BonoboUIComponent *uic, const char *path,
		   Bonobo_UIComponent_EventType type,
		   const char *state, gpointer user_data)
{
	if (atoi (state)) {
		if (strstr (path, "Red") != NULL)
			view_set_color (user_data, "red");
		else if (strstr (path, "White") != NULL)
			view_set_color (user_data, "white");
		else if (strstr (path, "Green") != NULL)
			view_set_color (user_data, "green");
		else
			g_error ("set to unknown color");
	}
}

/*
 * When one of our views is activated, we merge our menus
 * in with our container's menus.
 */
static void
view_create_menus (view_data_t *view_data)
{
	Bonobo_UIContainer  remote_uic;
	BonoboView         *view = view_data->view;
	BonoboUIComponent  *uic;

	const char *ui_commands =
		"<commands>\n"
		"	<cmd name=\"ColorWhite\"  _label=\"White\" group=\"Color\"/>\n"
		"	<cmd name=\"ColorRed\"    _label=\"Red\"   group=\"Color\"/>\n"
		"	<cmd name=\"ColorGreen\"  _label=\"Green\" group=\"Color\"/>\n"
		"</commands>\n";

	const char *ui_menus =
		"<menu>\n"
		"	<submenu name=\"Colors\" _label=\"Colors\">\n"
		"		<menuitem name=\"ColorWhite\" type=\"radio\" verb=\"\"/>\n"
		"		<menuitem name=\"ColorRed\"   type=\"radio\" verb=\"\"/>\n"
		"		<menuitem name=\"ColorGreen\" type=\"radio\" verb=\"\"/>\n"
		"	</submenu>\n"
		"</menu>\n";

	/*
	 * Grab our BonoboUIComponent object.
	 */
	uic = bonobo_view_get_ui_component (view);

	/*
	 * Get our container's UIContainer server.
	 */
	remote_uic = bonobo_view_get_remote_ui_container (view);

	/*
	 * We have to deal gracefully with containers
	 * which don't have a UIContainer running.
	 */
	if (remote_uic == CORBA_OBJECT_NIL) {
		g_warning ("Can't get remote UIContainer");
		return;
	}

	/*
	 * Give our BonoboUIHandler object a reference to the
	 * container's UIContainer server.
	 */
	bonobo_ui_component_set_container (uic, remote_uic);

	bonobo_ui_component_set_translate (uic, "/", ui_commands, NULL);
	bonobo_ui_component_set_translate (uic, "/", ui_menus, NULL);

	bonobo_ui_component_add_listener (uic, "ColorWhite", color_listener_cb, view_data);
	bonobo_ui_component_add_listener (uic, "ColorRed",   color_listener_cb, view_data);
	bonobo_ui_component_add_listener (uic, "ColorGreen", color_listener_cb, view_data);

	bonobo_ui_component_thaw (uic, NULL);
}

/*
 * When this view is deactivated, we must remove our menu items.
 */
static void
view_remove_menus (view_data_t *view_data)
{
	BonoboView *view = view_data->view;
	BonoboUIComponent *uic;

	uic = bonobo_view_get_ui_component (view);

	bonobo_ui_component_unset_container (uic);
}

static void
view_activate_cb (BonoboView *view, gboolean activate, view_data_t *view_data)
{
	/*
	 * The ViewFrame has just asked the View (that's us) to be
	 * activated or deactivated.  We must reply to the ViewFrame
	 * and say whether or not we want our activation state to
	 * change.  We are an acquiescent BonoboView, so we just agree
	 * with whatever the ViewFrame told us.  Most components
	 * should behave this way.
	 */
	bonobo_view_activate_notify (view, activate);

	/*
	 * If we were just activated, we merge in our menu entries.
	 * If we were just deactivated, we remove them.
	 */
	if (activate)
		view_create_menus (view_data);
	else
		view_remove_menus (view_data);
}

/*
 * This callback is envoked when the view is destroyed.  We use it to
 * free up our ancillary view-centric data structures.
 */
static void
view_destroy_cb (BonoboView *view, view_data_t *view_data)
{
	gdk_gc_destroy (view_data->gc);
	g_free (view_data);
}

/*
 * Some parts of the View initialization must be forestalled until the
 * View window is actually realized.  We perform those here.
 */
static void
view_realize_cb (GtkWidget *drawing_area, view_data_t *view_data)
{
	view_set_color (view_data, "white");
	view_update (view_data);
}

/*
 * When a part of the window is exposed, we must redraw it.
 */
static void
view_expose_cb (GtkWidget *drawing_area, GdkEventExpose *event, view_data_t *view_data)
{
	view_update (view_data);
}

/*
 * This callback is invoked whenever the user moves the mouse
 * over the drawing area.
 */
static void
view_motion_notify_cb (GtkWidget *drawing_area, GdkEventMotion *event,
		       view_data_t *view_data)
{
	embeddable_data_t *embeddable_data = view_data->embeddable_data;

	/*
	 * If the mouse button is depressed, we update the internal
	 * representation of the image.  Then we update all the views.
	 */

	if (! (event->state & GDK_BUTTON1_MASK))
		return;

	/*
	 * First, update the internal representation of the image.  We
	 * store the image data internally in an off-screen GdkPixmap.
	 * This is just for the convenience of being able to use gdk
	 * routines to draw into it.  We could just as easily store
	 * this data in our own image buffer, or as a list of strokes
	 * which could be replayed, or whatever.
	 *
	 * We do the drawing using the view's graphics context.  This
	 * is because each view could have a different current drawing
	 * mode (pen color, style, etc).  In general, all views for a
	 * given embeddable are supposed to be identical, but they can
	 * differ in some small ways, such as the undo history and
	 * current editing mode.
	 */
	if (view_data->last_x != -1 && view_data->last_y != -1) {
		gdk_draw_line (embeddable_data->pixmap,
			       view_data->gc,
			       view_data->last_x, view_data->last_y,
			       (gint) event->x, (gint) event->y);
	}

	view_data->last_x = (gint) event->x;
	view_data->last_y = (gint) event->y;
		
	/*
	 * Now reflect this change to the image data in all the views.
	 */
	embeddable_update_all_views (embeddable_data);
}

static void
embeddable_clear_image (embeddable_data_t *embeddable_data)
{
	GdkGC *temp_gc;

	temp_gc = gdk_gc_new (embeddable_data->pixmap);

	gdk_draw_rectangle (embeddable_data->pixmap,
			    temp_gc,
			    TRUE, 0, 0,
			    embeddable_data->width,
			    embeddable_data->height);
	gdk_gc_destroy (temp_gc);
}

/*
 * This function is invoked whenever the container requests that a new
 * view be created for a given embeddable.  Its job is to constrct the
 * new view and return it.
 *
 * All views of a given Embeddable must be identical.  The purpose of
 * the "View" concept is to make it easy for containers to implement
 * split-screen editing, where the same document is displayed in two
 * different windows simultaneously.
 *
 * Views can differ in a few small ways: they can be at different
 * zooms, can have different undo histories, and so on.  But using
 * BonoboViews to implement two radically different representations of
 * a piece of data (e.g. a hexadecimal dump of an image and the image
 * itself) is a misuse of the classes.
 */
static BonoboView *
view_factory (BonoboEmbeddable       *embeddable,
	      const Bonobo_ViewFrame  view_frame,
	      embeddable_data_t      *embeddable_data)
{
	view_data_t *view_data;
	BonoboView  *view;
	GtkWidget   *vbox;

	/*
	 * Create the private view data.
	 */
	view_data = g_new0 (view_data_t, 1);
	view_data->embeddable_data = embeddable_data;

	view_data->last_x = -1;
	view_data->last_y = -1;
	view_data->width = embeddable_data->width;
	view_data->height = embeddable_data->width;

	/*
	 * Now create the drawing area which will be used to display
	 * the current image in this view.
	 */
	view_data->drawing_area = gtk_drawing_area_new ();

	gtk_drawing_area_size (GTK_DRAWING_AREA (view_data->drawing_area),
			       view_data->width,
			       view_data->height);

	gtk_widget_set_usize (view_data->drawing_area,
			      view_data->width,
			      view_data->height);

	/*
	 * We will use this event to actually draw into the
	 * Embeddable.
	 */
	gtk_widget_set_events (view_data->drawing_area,
			       gtk_widget_get_events (view_data->drawing_area) |
			       GDK_BUTTON_MOTION_MASK |
			       GDK_BUTTON_PRESS_MASK);

	gtk_signal_connect (GTK_OBJECT (view_data->drawing_area), "motion_notify_event",
			    GTK_SIGNAL_FUNC (view_motion_notify_cb), view_data);

	/*
	 * When the widget is realized, we will draw the current image
	 * data into it.
	 */
	gtk_signal_connect (GTK_OBJECT (view_data->drawing_area), "realize",
			    GTK_SIGNAL_FUNC (view_realize_cb), view_data);

	/*
	 * We have to redraw the view when we get expose events.
	 */
	gtk_signal_connect (GTK_OBJECT (view_data->drawing_area), "expose_event",
			    GTK_SIGNAL_FUNC (view_expose_cb), view_data);
	 

	/*
	 * Insert the drawing area into a vbox.
	 */
	vbox = gtk_vbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox),
			    view_data->drawing_area,
			    TRUE, TRUE, 0);

	gtk_widget_show_all (vbox);

	/*
	 * Each view has its own GC; we create that here.
	 */
	view_data->gc = gdk_gc_new (embeddable_data->pixmap);

	/*
	 * Create the BonoboView object.
	 */
	view = bonobo_view_new (vbox);
	view_data->view = view;
	gtk_object_set_data (GTK_OBJECT (view), "view_data", view_data);

	/*
	 * When our container wants to activate a given view of this
	 * component, we will get the "activate" signal.
	 */
	gtk_signal_connect (GTK_OBJECT (view), "activate",
			    GTK_SIGNAL_FUNC (view_activate_cb), view_data);

	/*
	 * The "system_exception" signal is raised when the BonoboView
	 * encounters a fatal CORBA exception.
	 */
	gtk_signal_connect (GTK_OBJECT (view), "system_exception",
			    GTK_SIGNAL_FUNC (view_system_exception_cb), view_data);

	/*
	 * We'll need to be able to cleanup when this view gets
	 * destroyed.
	 */
	gtk_signal_connect (GTK_OBJECT (view), "destroy",
			    GTK_SIGNAL_FUNC (view_destroy_cb), view_data);

	return view;
}

static void
render_fn (GnomePrintContext         *ctx,
	   double                     width,
	   double                     height,
	   const Bonobo_PrintScissor *scissor,
	   gpointer                   user_data)
{
	GnomeFont         *font;
	double             w;
	const char         str [] = "Hello World";

	gnome_print_setlinewidth (ctx, 2);
	font = gnome_font_new ("Helvetica", 12.0);
	g_return_if_fail (font != NULL);
	gnome_print_setrgbcolor (ctx, 0.0, 0.0, 0.0);
	gnome_print_setfont (ctx, font);

	w = gnome_font_get_width_string (font, str);
	gnome_print_moveto (ctx, (width / 2) - (w / 2),
			    height / 2);
	gnome_print_show (ctx, str);
	gtk_object_unref (GTK_OBJECT (font));

	gnome_print_moveto (ctx, 0, 0);
	gnome_print_lineto (ctx, width, height);
	gnome_print_stroke (ctx);

/* We need a sensible internal representation in order to find the rowstride etc. */
/*	gnome_print_rgbimage (ctx, embeddable_data->
						  embeddable_data->width,
						  embeddable_data->height,
						  gdk_visual_get_best_depth ());*/
}


/*
 * When a container asks our GenericFactory for a new paint
 * component, this function is called.  It creates the new
 * BonoboEmbeddable object and returns it.
 */
static BonoboObject *
embeddable_factory (BonoboGenericFactory *this,
		    void *data)
{
	BonoboEmbeddable *embeddable;
	BonoboObject     *print;
	embeddable_data_t *embeddable_data;

	/*
	 * Create a data structure in which we can store
	 * Embeddable-object-specific data about this document.
	 */
	embeddable_data = g_new0 (embeddable_data_t, 1);
	if (embeddable_data == NULL)
		return NULL;

	/*
	 * Our paint component is very simple.  It only works with one
	 * size of image.
	 */
	embeddable_data->width = 100;
	embeddable_data->height = 100;

	/*
	 * The embeddable must maintain an internal representation of
	 * the data for its document.  In our case, that document is
	 * an image, and it so happens that the most convenient way of
	 * storing an image for us is a GdkPixmap.
	 */
	embeddable_data->pixmap = gdk_pixmap_new (NULL,
						  embeddable_data->width,
						  embeddable_data->height,
						  gdk_visual_get_best_depth ());

	/*
	 * Blank the pixmap.
	 */
	embeddable_clear_image (embeddable_data);
	
	/*
	 * Create the BonoboEmbeddable object.
	 */
	embeddable = bonobo_embeddable_new (BONOBO_VIEW_FACTORY (view_factory),
					    embeddable_data);

	print = BONOBO_OBJECT (bonobo_print_new (render_fn, embeddable_data));
	if (!print)
		g_warning ("Serious error creating print interface");
	else
		bonobo_object_add_interface (BONOBO_OBJECT (embeddable),
					     BONOBO_OBJECT (print));

	if (embeddable == NULL) {
		g_free (embeddable_data);
		return NULL;
	}
	
	embeddable_data->embeddable = embeddable;

	/*
	 * If the Embeddable encounters a fatal CORBA exception, it
	 * will emit a "system_exception" signal, notifying us that
	 * the object is defunct.  Our callback --
	 * embeddable_system_exception_cb() -- destroys the defunct
	 * BonoboEmbeddable object.
	 */
	gtk_signal_connect (GTK_OBJECT (embeddable), "system_exception",
			    GTK_SIGNAL_FUNC (embeddable_system_exception_cb),
			    embeddable_data);

	/*
	 * Catch the destroy signal so that we can free up resources.
	 * When an Embeddable is destroyed, its views will
	 * automatically be destroyed.
	 */
	gtk_signal_connect (GTK_OBJECT (embeddable), "destroy",
			    GTK_SIGNAL_FUNC (embeddable_destroy_cb),
			    embeddable_data);

	return BONOBO_OBJECT (embeddable);
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_Paint_EmbeddableFactory",
		    "bonobo-simple-paint", VERSION,
		    embeddable_factory,
		    NULL)
