/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-wrapper.c: Wrapper for plug/socket children in Bonobo
 *
 * Author:
 *    Federico Mena  (federico@nuclecu.unam.mx)
 *    Nat Friedman   (nat@nat.org)
 *    Mathieu Lacage (mlacage@aol.com)
 *
 * Copyright (C) 1999 the Free Software Foundation
 */

#include <config.h>
#include <bonobo/bonobo-wrapper.h>
#include <gdk/gdktypes.h>
#include <gtk/gtkgc.h>
#include <gtk/gtksignal.h>

#define BORDER_WIDTH	3

static void bonobo_wrapper_class_init (BonoboWrapperClass *klass);
static void bonobo_wrapper_init (BonoboWrapper *wrapper);

static void bonobo_wrapper_map (GtkWidget *widget);
static void bonobo_wrapper_unmap (GtkWidget *widget);
static void bonobo_wrapper_realize (GtkWidget *widget);
static void bonobo_wrapper_unrealize (GtkWidget *widget);
static void bonobo_wrapper_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void bonobo_wrapper_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint bonobo_wrapper_expose (GtkWidget *widget, GdkEventExpose *event);
static void bonobo_wrapper_draw (GtkWidget *widget, GdkRectangle *area);
static void bonobo_wrapper_paint (GtkWidget *widget);

static GtkBinClass *parent_class;

struct _BonoboWrapperPrivate {
	/* Whether the child is covered or not */
	gboolean covered;

	/* Whether or not we should paint the cover. */
	gboolean visible;

	/* The GC used for painting on the cover. */
	GdkGC *gc;

	/* The InputOnly window that covers the child */
	GdkWindow *cover;
};


/**
 * bonobo_wrapper_get_type:
 * 
 * Generates a unique type ID for BonoboWrapperClass.
 * 
 * Returns: the type ID for BonoboWrapperClass.
 */
GtkType
bonobo_wrapper_get_type (void)
{
	static GtkType wrapper_type = 0;

	if (!wrapper_type) {
		static const GtkTypeInfo wrapper_info = {
			"BonoboWrapper",
			sizeof (BonoboWrapper),
			sizeof (BonoboWrapperClass),
			 (GtkClassInitFunc) bonobo_wrapper_class_init,
			 (GtkObjectInitFunc) bonobo_wrapper_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			 (GtkClassInitFunc) NULL
		};

		wrapper_type = gtk_type_unique (gtk_bin_get_type (), &wrapper_info);
	}

	return wrapper_type;
}

static void
bonobo_wrapper_destroy (GtkObject *object)
{
	BonoboWrapper *wrapper;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (object));

	wrapper = BONOBO_WRAPPER (object);

	if (wrapper->priv->gc != NULL)
		gdk_gc_destroy (wrapper->priv->gc);

	if (wrapper->priv->cover != NULL) {
		gdk_window_set_user_data (wrapper->priv->cover, NULL);
		gdk_window_destroy (wrapper->priv->cover);
	}

	g_free (wrapper->priv);

	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* Standard class initialization function */
static void
bonobo_wrapper_class_init (BonoboWrapperClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_BIN);

	widget_class->map = bonobo_wrapper_map;
	widget_class->unmap = bonobo_wrapper_unmap;
	widget_class->realize = bonobo_wrapper_realize;
	widget_class->unrealize = bonobo_wrapper_unrealize;
	widget_class->size_request = bonobo_wrapper_size_request;
	widget_class->size_allocate = bonobo_wrapper_size_allocate;
	widget_class->expose_event = bonobo_wrapper_expose;
	widget_class->draw = bonobo_wrapper_draw;

	object_class->destroy = bonobo_wrapper_destroy;
}

/* Standard object initialization function */
static void
bonobo_wrapper_init (BonoboWrapper *wrapper)
{
	GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET(wrapper), GTK_NO_WINDOW);

	wrapper->priv = g_new0 (BonoboWrapperPrivate, 1);

	wrapper->priv->covered = TRUE;
	wrapper->priv->visible = TRUE;
}

/* Map handler for the wrapper widget.  We map the child, then the normal
 * widget->window, and then the wrapper->priv->cover only if it is active.
 */
static void
bonobo_wrapper_map (GtkWidget *widget)
{
	BonoboWrapper *wrapper;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (widget));

	wrapper = BONOBO_WRAPPER (widget);
	GTK_WIDGET_SET_FLAGS (wrapper, GTK_MAPPED);

	if (wrapper->bin.child
	    && GTK_WIDGET_VISIBLE (wrapper->bin.child)
	    && !GTK_WIDGET_MAPPED (wrapper->bin.child))
		gtk_widget_map (wrapper->bin.child);

	gdk_window_show (widget->window);

	if (wrapper->priv->covered)
		gdk_window_show (wrapper->priv->cover);
}

/* Unmap handler for the wrapper widget */
static void
bonobo_wrapper_unmap (GtkWidget *widget)
{
	BonoboWrapper *wrapper;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (widget));

	wrapper = BONOBO_WRAPPER (widget);
	GTK_WIDGET_UNSET_FLAGS (wrapper, GTK_MAPPED);

	gdk_window_hide (widget->window);

	if (wrapper->priv->covered)
		gdk_window_hide (wrapper->priv->cover);

	if (wrapper->bin.child && GTK_WIDGET_MAPPED (wrapper->bin.child))
		gtk_widget_unmap (wrapper->bin.child);
}

/* Realize handler for the wrapper widget.  We create the widget->window, which
 * is the child's container, and the wrapper->priv->cover, which is the cover window.
 * This is a bit special in that both windows are direct children of the parent
 * widget's window.
 */
static void
bonobo_wrapper_realize (GtkWidget *widget)
{
	BonoboWrapper *wrapper;
	GdkWindow *parent_window;
	GdkWindowAttr attributes;
	int attributes_mask;
	GdkGCValues gc_values;
	char data_paint [25] = {21, 10, 21, 10, 21}; /* FIXME: What the fuck? */

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (widget));

	wrapper = BONOBO_WRAPPER (widget);
	GTK_WIDGET_SET_FLAGS (wrapper, GTK_REALIZED);

	parent_window = gtk_widget_get_parent_window (widget);

	/*
	 * Child's window.
	 */
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (parent_window, &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, wrapper);

	/*
	 * The GC used to draw the cover.
	 */
	gc_values.fill = GDK_STIPPLED;
	gc_values.stipple = gdk_bitmap_create_from_data (widget->window,
							 data_paint,
							 BORDER_WIDTH, BORDER_WIDTH);
	gc_values.subwindow_mode = GDK_CLIP_BY_CHILDREN;

	wrapper->priv->gc = gdk_gc_new_with_values (widget->window,
						    &gc_values,
						    GDK_GC_FILL | 
						    GDK_GC_STIPPLE | 
						    GDK_GC_SUBWINDOW);
	gdk_pixmap_unref (gc_values.stipple);

	/*
	 * Cover window.
	 */
	attributes.wclass = GDK_INPUT_ONLY;

	wrapper->priv->cover = gdk_window_new (parent_window, &attributes, attributes_mask);
	gdk_window_set_events (wrapper->priv->cover, GDK_BUTTON_PRESS_MASK);
	gdk_window_set_user_data (wrapper->priv->cover, wrapper);

	/*
	 * Style (pinache!).
	 */
	widget->style = gtk_style_attach (widget->style, widget->window);
	
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

/* Unrealize handler for the wrapper widget.  We destroy the cover window and let
 * the default handler do the rest.
 */
static void
bonobo_wrapper_unrealize (GtkWidget *widget)
{
	BonoboWrapper *wrapper;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (widget));

	wrapper = BONOBO_WRAPPER (widget);

	gdk_gc_destroy (wrapper->priv->gc);
	wrapper->priv->gc = NULL;

	gdk_window_set_user_data (wrapper->priv->cover, NULL);
	gdk_window_destroy (wrapper->priv->cover);
	wrapper->priv->cover = NULL;

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

/* Size request handler for the wrapper widget.  We simply use the child's
 * requisition.
 */
static void
bonobo_wrapper_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	BonoboWrapper *wrapper;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (widget));
	g_return_if_fail (requisition != NULL);

	wrapper = BONOBO_WRAPPER (widget);

	if (wrapper->bin.child) {
		gtk_widget_size_request (wrapper->bin.child, 
					 requisition);

		if (!wrapper->priv->covered && wrapper->priv->visible) {
			requisition->width += BORDER_WIDTH * 2;
			requisition->height += BORDER_WIDTH * 2;
		}	
	} else {
 		requisition->width = 1;
 		requisition->height = 1;
 	}

}

/* Size allocate handler for the wrapper widget.  We simply use the allocation
 * and don't pay attention to the border_width.
 */
static void
bonobo_wrapper_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	BonoboWrapper *wrapper;
	GtkAllocation child_allocation;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (widget));
	g_return_if_fail (allocation != NULL);

	wrapper = BONOBO_WRAPPER (widget);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED (wrapper)) {
		gdk_window_move_resize (widget->window,
					widget->allocation.x,
					widget->allocation.y,
					widget->allocation.width,
					widget->allocation.height);

		gdk_window_move_resize (wrapper->priv->cover,
					widget->allocation.x,
					widget->allocation.y,
					widget->allocation.width,
					widget->allocation.height);
	}

	if (wrapper->bin.child && GTK_WIDGET_VISIBLE (wrapper->bin.child)) {
		child_allocation.x = 0;
		child_allocation.y = 0;
		child_allocation.width = widget->allocation.width;
		child_allocation.height = widget->allocation.height;
		if (!wrapper->priv->covered && wrapper->priv->visible) {
			child_allocation.x += BORDER_WIDTH;
			child_allocation.y += BORDER_WIDTH;
			if (child_allocation.width < BORDER_WIDTH * 2)
				child_allocation.width = 0;
			else
				child_allocation.width -= BORDER_WIDTH * 2;
			if (child_allocation.height < BORDER_WIDTH * 2)
				child_allocation.height = 1;
			else
				child_allocation.height -= BORDER_WIDTH * 2;
		}
		gtk_widget_size_allocate (wrapper->bin.child, &child_allocation);
	}
	/* The above if/then/else are necesary because you have to 
	   make sure you don't calculate a width and height which 
	   are around 65xxx (ie: width and height are guint16 so if
	   you substract BORDER_WIDTH*2, you may end with a size of
	   65xxx or so).
	 */

	gtk_signal_emit_by_name (GTK_OBJECT (widget), "draw");
}

static void
bonobo_wrapper_paint (GtkWidget *widget)
{
	BonoboWrapper *wrapper = BONOBO_WRAPPER (widget);

	if (wrapper->priv->visible && !wrapper->priv->covered) {
		gdk_draw_rectangle (widget->window,
				    wrapper->priv->gc,
				    TRUE,
				    0, 0,
				    widget->allocation.width,
				    widget->allocation.height);
 	}
}

static void
bonobo_wrapper_draw (GtkWidget *widget, GdkRectangle *area)
{
	bonobo_wrapper_paint (widget);
}

static gint
bonobo_wrapper_expose (GtkWidget *widget, GdkEventExpose *event)
{
	bonobo_wrapper_paint (widget);
	return FALSE;
}

/**
 * bonobo_wrapper_new:
 * 
 * Creates a new wrapper widget.  It starts covered by default.
 * 
 * Returns: The newly-created wrapper widget.
 */
GtkWidget *
bonobo_wrapper_new (void)
{
	return GTK_WIDGET (gtk_type_new (bonobo_wrapper_get_type ()));
}

/**
 * bonobo_wrapper_set_covered:
 * @wrapper: A wrapper widget
 * @covered: Whether it should be covered or not
 * 
 * Sets the covered status of a wrapper widget by showing or hiding the cover
 * window as appropriate.
 */
void
bonobo_wrapper_set_covered (BonoboWrapper *wrapper, gboolean covered)
{
	g_return_if_fail (wrapper != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (wrapper));

	if (wrapper->priv->covered && !covered) {
		wrapper->priv->covered = FALSE;

		if (GTK_WIDGET_MAPPED (wrapper)) {
			gdk_window_hide (wrapper->priv->cover);
			gtk_widget_queue_resize (GTK_WIDGET (wrapper));
		}
	} else if (!wrapper->priv->covered && covered) {
		wrapper->priv->covered = TRUE;

		if (GTK_WIDGET_MAPPED (wrapper)) {
			gdk_window_show (wrapper->priv->cover);
			gtk_widget_queue_resize (GTK_WIDGET (wrapper));
		}
	}
}

/**
 * bonobo_wrapper_is_covered:
 * @wrapper: A wrapper widget.
 * 
 * Queries the covered status of a wrapper widget.
 * 
 * Returns: Whether the wrapper widget is covering its child or not.
 */
gboolean
bonobo_wrapper_is_covered (BonoboWrapper *wrapper)
{
	g_return_val_if_fail (wrapper != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_WRAPPER (wrapper), FALSE);

	return wrapper->priv->covered;
}


/**
 * bonobo_wrapper_set_visibility:
 * @wrapper: A BonoboWrapper.
 * @visible: A flag to indicate whether @wrapper's cover should be
 * visible.
 *
 * Use this function to set the visibility of the wrapper's cover
 * window.  If the visibility flag is TRUE, then a stipple pattern
 * will be drawn on the cover window to indicate when the wrapper's
 * contents are covered.  Of course, this stipple pattern will only
 * be drawn when the cover is there; use bonobo_wrapper_set_covered()
 * to enable the cover.
 */
void
bonobo_wrapper_set_visibility (BonoboWrapper *wrapper, gboolean visible)
{
	g_return_if_fail (wrapper != NULL);
	g_return_if_fail (BONOBO_IS_WRAPPER (wrapper));

	if (wrapper->priv->visible == visible)
		return;
	
	wrapper->priv->visible = visible;
	gtk_widget_queue_resize (GTK_WIDGET (wrapper));
}

/**
 * bonobo_wrapper_get_visibility:
 * @wrapper: A BonoboWrapper.
 *
 * Returns: Whether or not visual hints should be drawn on the cover
 * to indicate when it is covering @wrapper's contents.
 */
gboolean
bonobo_wrapper_get_visibility (BonoboWrapper *wrapper)
{
	g_return_val_if_fail (wrapper != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_WRAPPER (wrapper), FALSE);

	return wrapper->priv->visible;
}
















