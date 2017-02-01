/* gnome-procbar.c - Gnome Process Bar.

   Copyright (C) 1998 Martin Baulig

   Based on the orignal gtop/procbar.c from Radek Doulik.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.
*/

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <gtk/gtkdrawingarea.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "gtop-procbar.h"

struct _GTopProcBarPrivate {
    GtkWidget *bar;
    GtkWidget *label;
    GtkWidget *frame;

    gboolean vertical : 1;

    GdkPixmap *bs;
    GdkColor *colors;

    gint colors_allocated;
    gint first_request;
    gint n;
    gint tag;

    unsigned *last;

    gint (*cb)();
    gpointer cb_data;
};

#define A (w->allocation)

static void gtop_proc_bar_class_init (GTopProcBarClass *class);
static void gtop_proc_bar_init       (GTopProcBar      *pb);

static GtkHBoxClass *parent_class;

static gint gtop_proc_bar_expose (GtkWidget *w, GdkEventExpose *e, GTopProcBar *pb);
static gint gtop_proc_bar_configure (GtkWidget *w, GdkEventConfigure *e, GTopProcBar *pb);
static void gtop_proc_bar_size_request (GtkWidget *w, GtkRequisition *r, GTopProcBar *pb);
static void gtop_proc_bar_finalize (GtkObject *o);
static void gtop_proc_bar_setup_colors (GTopProcBar *pb);
static void gtop_proc_bar_draw (GTopProcBar *pb, const guint val []);
static void gtop_proc_bar_destroy (GtkObject *obj);
static gint gtop_proc_bar_timeout (gpointer data);

guint
gtop_proc_bar_get_type (void)
{
    static guint proc_bar_type = 0;

    if (!proc_bar_type) {
	GtkTypeInfo proc_bar_info = {
	    "GTopProcBar",
	    sizeof (GTopProcBar),
	    sizeof (GTopProcBarClass),
	    (GtkClassInitFunc) gtop_proc_bar_class_init,
	    (GtkObjectInitFunc) gtop_proc_bar_init,
	    (GtkArgSetFunc) NULL,
	    (GtkArgGetFunc) NULL
	};

	proc_bar_type = gtk_type_unique (gtk_hbox_get_type (), &proc_bar_info);
    }

    return proc_bar_type;
}

static void
gtop_proc_bar_class_init (GTopProcBarClass *class)
{
    GtkObjectClass *object_class;

    object_class = (GtkObjectClass *) class;

    parent_class = gtk_type_class (gtk_hbox_get_type ());

    object_class->finalize = gtop_proc_bar_finalize;

    object_class->destroy = gtop_proc_bar_destroy;
}

static void
gtop_proc_bar_destroy (GtkObject *obj)
{
    GTopProcBar *pb = GTOP_PROC_BAR (obj);

    if (pb->_priv->tag != -1) {
	gtk_timeout_remove (pb->_priv->tag);
	pb->_priv->tag = -1;
    }

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (obj);
}

static void
gtop_proc_bar_finalize (GtkObject *o)
{
    GTopProcBar *pb;

    g_return_if_fail (o != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (o));

    pb = GTOP_PROC_BAR (o);

    g_free (pb->_priv);

    (* GTK_OBJECT_CLASS (parent_class)->finalize) (o);
}

static void
gtop_proc_bar_init (GTopProcBar *pb)
{
    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));

    pb->_priv = g_new0 (GTopProcBarPrivate, 1);
}

/**
 * gtop_proc_bar_new:
 * @pb: A #GnomeProBar object to construct
 * @label: Either %NULL or a #GtkWidget that will be shown at the left
 * side of the process bar.
 * @n: Number of items.
 * @colors: Pointer to an array of @n #GdkColor elements.
 * @cb: Callback function to update the process bar.
 *
 * Constructs the @pb objects with @n items with the colors of
 * @colors. To do automatic updating, you set the @cb to a function
 * which takes a single void pointer as an argument and returns %TRUE
 * or %FALSE.  When it returns %FALSE the timer stops running and the
 * function stops getting called. You need to call
 * #gtop_proc_bar_start with the time interval and the data argument
 * that will be passed to the callback to actually start executing the
 * timer.
 *
 */
void
gtop_proc_bar_construct (GTopProcBar *pb, GtkWidget *label, gint n, GdkColor *colors, gint (*cb)())
{
    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));
    
    pb->_priv->cb = cb;
    pb->_priv->n = n;
    pb->_priv->colors = colors;
    pb->_priv->vertical = FALSE;

    pb->_priv->tag = -1;
    pb->_priv->first_request = 1;
    pb->_priv->colors_allocated = 0;

    pb->_priv->last = g_new (unsigned, pb->_priv->n+1);
    pb->_priv->last [0] = 0;

    pb->_priv->bar = gtk_drawing_area_new ();
    pb->_priv->frame = gtk_frame_new (NULL);

    pb->_priv->bs = NULL;

    gtk_frame_set_shadow_type (GTK_FRAME (pb->_priv->frame), GTK_SHADOW_IN);

    gtk_container_add (GTK_CONTAINER (pb->_priv->frame), pb->_priv->bar);

    pb->_priv->label = label;

    if (label) {
	gtk_box_pack_start (GTK_BOX (pb), label, FALSE, TRUE, 0);
	gtk_widget_show (pb->_priv->label);
    }

    gtk_widget_set_events (pb->_priv->bar, GDK_EXPOSURE_MASK | gtk_widget_get_events (pb->_priv->bar));

    gtk_signal_connect (GTK_OBJECT (pb->_priv->bar), "expose_event",
			(GtkSignalFunc) gtop_proc_bar_expose, pb);
    gtk_signal_connect (GTK_OBJECT (pb->_priv->bar), "configure_event",
			(GtkSignalFunc) gtop_proc_bar_configure, pb);
    gtk_signal_connect (GTK_OBJECT (pb->_priv->bar), "size_request",
			(GtkSignalFunc) gtop_proc_bar_size_request, pb);

    gtk_box_pack_start_defaults (GTK_BOX (pb), pb->_priv->frame);

    gtk_widget_show (pb->_priv->frame);
    gtk_widget_show (pb->_priv->bar);

}

/**
 * gtop_proc_bar_new:
 * @label: Either %NULL or a #GtkWidget that will be shown at the left
 * side of the process bar.
 * @n: Number of items.
 * @colors: Pointer to an array of @n #GdkColor elements.
 * @cb: Callback function to update the process bar.
 *
 * Description: Creates a new Gnome Process Bar with @n items with the
 * colors of @colors. To do automatic updating, you set the @cb to a function
 * which takes a single void pointer as an argument and returns %TRUE or %FALSE.
 * When it returns %FALSE the timer stops running and the function stops getting
 * called. You need to call #gtop_proc_bar_start with the time interval and
 * the data argument that will be passed to the callback to actually start
 * executing the timer.
 *
 * Returns: The newly created #GTopProcBar widget.
 */
GtkWidget *
gtop_proc_bar_new (GtkWidget *label, gint n, GdkColor *colors, gint (*cb)())
{
    GTopProcBar *pb;

    pb = gtk_type_new (gtop_proc_bar_get_type ());

    gtop_proc_bar_construct (pb, label, n, colors, cb);
    return GTK_WIDGET (pb);
}

static gint
gtop_proc_bar_expose (GtkWidget *w, GdkEventExpose *e, GTopProcBar *pb)
{
    if (pb->_priv->bs)
	gdk_window_copy_area (w->window,
			      w->style->black_gc,
			      e->area.x, e->area.y,
			      pb->_priv->bs,
			      e->area.x, e->area.y,
			      e->area.width, e->area.height);

    return TRUE;
}

static void
gtop_proc_bar_size_request (GtkWidget *w, GtkRequisition *r, GTopProcBar *pb)
{
    if (!pb->_priv->first_request) {
	r->width = w->allocation.width;
	r->height = w->allocation.height;
    }
    pb->_priv->first_request = 0;
}

static void
gtop_proc_bar_setup_colors (GTopProcBar *pb)
{
    GdkColormap *cmap;
    gint i;

    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));
    g_return_if_fail (pb->_priv->bar != NULL);
    g_return_if_fail (pb->_priv->bar->window != NULL);

    cmap = gdk_window_get_colormap (pb->_priv->bar->window);
    for (i=0; i<pb->_priv->n; i++)
	gdk_color_alloc (cmap, &pb->_priv->colors [i]);

    pb->_priv->colors_allocated = 1;
}

static gint
gtop_proc_bar_configure (GtkWidget *w, GdkEventConfigure *e,
			 GTopProcBar *pb)
{
    gtop_proc_bar_setup_colors (pb);

    if (pb->_priv->bs) {
	gdk_pixmap_unref (pb->_priv->bs);
	pb->_priv->bs = NULL;
    }

    pb->_priv->bs = gdk_pixmap_new (w->window,
			     w->allocation.width,
			     w->allocation.height,
			     -1);

    gdk_draw_rectangle (w->window, w->style->black_gc, TRUE, 0, 0,
			w->allocation.width, w->allocation.height);

    gtop_proc_bar_draw (pb, pb->_priv->last);

    return TRUE;
}

#undef A

#define W (pb->_priv->bar)
#define A (pb->_priv->bar->allocation)

static void
gtop_proc_bar_draw (GTopProcBar *pb, const guint val [])
{
    unsigned tot = 0;
    gint i;
    gint x;
    gint wr, w;
    GdkGC *gc;

    w = pb->_priv->vertical ? A.height : A.width;
    x = 0;

    for (i=0; i<pb->_priv->n; i++)
	tot += val [i+1];

    if (!GTK_WIDGET_REALIZED (pb->_priv->bar) || !tot)
	return;

    gc = gdk_gc_new (pb->_priv->bar->window);

    for (i=0; i<pb->_priv->n; i++) {
	if (i<pb->_priv->n-1)
	    wr = (unsigned) w * ((float)val [i+1]/tot);
	else
	    wr = (pb->_priv->vertical ? A.height : A.width) - x;

	gdk_gc_set_foreground (gc,
			       &pb->_priv->colors [i]);

	if (pb->_priv->vertical)
	    gdk_draw_rectangle (pb->_priv->bs,
				gc,
				TRUE,
				0, A.height - x - wr,
				A.width, wr);
	else
	    gdk_draw_rectangle (pb->_priv->bs,
				gc,
				TRUE,
				x, 0,
				wr, A.height);
	
	x += wr;
    }
		
    gdk_window_copy_area (pb->_priv->bar->window,
			  gc,
			  0, 0,
			  pb->_priv->bs,
			  0, 0,
			  A.width, A.height);

    gdk_gc_destroy (gc);
}

#undef W
#undef A

static gint
gtop_proc_bar_timeout (gpointer data)
{
    GTopProcBar *pb = data;
    gint result;

    GDK_THREADS_ENTER ();
    result = pb->_priv->cb (pb->_priv->cb_data);
    GDK_THREADS_LEAVE ();

    return result;
}

/**
 * gtop_proc_bar_set_values:
 * @pb: Pointer to a #GTopProcBar object
 * @val: pointer to an array of @pb->_priv->n integers
 *
 * Description: Set the values of @pb to @val and redraw it. You will
 * probably call this function in the callback to update the values.
 *
 * Returns:
 */
void
gtop_proc_bar_set_values (GTopProcBar *pb, const guint val [])
{
    gint i;
    gint change = 0;

    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));

    if (!GTK_WIDGET_REALIZED (pb->_priv->bar)) {
	for (i=0; i<pb->_priv->n+1; i++)
	    pb->_priv->last [i] = val [i];
	return;
    }

    /* check if values changed */

    for (i=0; i<pb->_priv->n+1; i++) {
	if (val[i] != pb->_priv->last [i]) {
	    change = 1;
	    break;
	}
	pb->_priv->last [i] = val [i];
    }

    gtop_proc_bar_draw (pb, val);
}

/**
 * gtop_proc_bar_start:
 * @pb: Pointer to a #GTopProcBar object
 * @gtime: time interval in ms
 * @data: data to the callback
 *
 * Description: Start a timer, and call the callback that was set
 * on #gtop_proc_bar_new with the @data.
 *
 * Returns:
 */
void
gtop_proc_bar_start (GTopProcBar *pb, gint gtime, gpointer data)

{
    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));

    if (pb->_priv->tag != -1)
	gtk_timeout_remove (pb->_priv->tag);

    if (pb->_priv->cb) {
	pb->_priv->cb (data);
        pb->_priv->cb_data = data;
	pb->_priv->tag = gtk_timeout_add (gtime, gtop_proc_bar_timeout, pb);
    }
}

/**
 * gtop_proc_bar_stop:
 * @pb: Pointer to a #GTopProcBar object
 *
 * Description: Stop running the callback in the timer.
 *
 * Returns:
 */
void
gtop_proc_bar_stop (GTopProcBar *pb)

{
    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));

    if (pb->_priv->tag != -1)
	gtk_timeout_remove (pb->_priv->tag);

    pb->_priv->tag = -1;
}

/**
 * gtop_proc_bar_update:
 * @pb: Pointer to a #GTopProcBar object
 * @colors: Pointer to an array of @pb->_priv->n #GdkColor elements
 *
 * Description: Update @pb with @colors. @pb is not redrawn,
 * it is only redrawn when you call #gtop_proc_bar_set_values
 *
 * Returns:
 */
void
gtop_proc_bar_update (GTopProcBar *pb, GdkColor *colors)
{
    char tmp [BUFSIZ];
    gint i;

    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));

    for (i=0;i<pb->_priv->n;i++) {
	sprintf (tmp, "#%04x%04x%04x",
		 colors [i].red, colors [i].green,
		 colors [i].blue);
	gdk_color_parse (tmp, &pb->_priv->colors [i]);
    }

    gtop_proc_bar_setup_colors (pb);
}

/**
 * gtop_proc_bar_set_orient:
 * @pb: Pointer to a #GTopProcBar object
 * @vertical: %TRUE if vertical %FALSE if horizontal
 *
 * Description: Sets the orientation of @pb to vertical if
 * @vertical is %TRUE or to horizontal if @vertical is %FALSE.
 *
 * Returns:
 */
void
gtop_proc_bar_set_orient (GTopProcBar *pb, gboolean vertical)
{
    g_return_if_fail (pb != NULL);
    g_return_if_fail (GTOP_IS_PROC_BAR (pb));

    pb->_priv->vertical = vertical;
}
