/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Ghostscript widget for GTK/GNOME
 * Copyright (C) 1998 the Free Software Foundation
 * Author: Jonathan Blandford <jrb@redhat.com>
 * Based on code by: Federico Mena (Quartic), Szekeres Istvan (Pista)

Ghostview interface to ghostscript

When the GHOSTVIEW environment variable is set, ghostscript draws on
an existing drawable rather than creating its own window.  Ghostscript
can be directed to draw on either a window or a pixmap.

Drawing on a Window

The GHOSTVIEW environment variable contains the window id of the target
window.  The window id is an integer.  Ghostscript will use the attributes
of the window to obtain the width, height, colormap, screen, and visual of
the window. The remainder of the information is gotten from the GHOSTVIEW
property on that window.


Drawing on a Pixmap

The GHOSTVIEW environment variable contains a window id and a pixmap id.
They are integers separated by white space.  Ghostscript will use the
attributes of the window to obtain the colormap, screen, and visual to use.
The width and height will be obtained from the pixmap. The remainder of the
information, is gotten from the GHOSTVIEW property on the window.  In this
case, the property is deleted when read.

The GHOSTVIEW environment variable

parameters:	window-id [pixmap-id]

scanf format:	"%d %d"

explanation of parameters:

	window-id: tells ghostscript where to
		    - read the GHOSTVIEW property
		    - send events
		    If pixmap-id is not present,
		    ghostscript will draw on this window.

	pixmap-id: If present, tells ghostscript that a pixmap will be used
		    as the final destination for drawing.  The window will
		    not be touched for drawing purposes.

The GHOSTVIEW property

type:	STRING

parameters:

    bpixmap orient llx lly urx ury xdpi ydpi [left bottom top right]

scanf format: "%d %d %d %d %d %d %f %f %d %d %d %d"

explanation of parameters:

	bpixmap: pixmap id of the backing pixmap for the window.  If no
		pixmap is to be used, this parameter should be zero.  This
		parameter must be zero when drawing on a pixmap.

	orient:	orientation of the page.  The number represents clockwise
		rotation of the paper in degrees.  Permitted values are
		0, 90, 180, 270.

	llx, lly, urx, ury: Bounding box of the drawable.  The bounding box
		is specified in PostScript points in default user coordinates.

	xdpi, ydpi: Resolution of window.  (This can be derived from the
		other parameters, but not without roundoff error.  These
		values are included to avoid this error.)

	left, bottom, top, right: (optional)
		Margins around the window.  The margins extend the imageable
		area beyond the boundaries of the window.  This is primarily
		used for popup zoom windows.  I have encountered several
		instances of PostScript programs that position themselves
		with respect to the imageable area.  The margins are specified
		in PostScript points.  If omitted, the margins are assumed to
		be 0.

Events from ghostscript

If the final destination is a pixmap, the client will get a property notify
event when ghostscript reads the GHOSTVIEW property causing it to be deleted.

Ghostscript sends events to the window where it read the GHOSTVIEW property.
These events are of type ClientMessage.  The message_type is set to
either PAGE or DONE.  The first long data value gives the window to be used
to send replies to ghostscript.  The second long data value gives the primary
drawable.  If rendering to a pixmap, it is the primary drawable.  If rendering
to a window, the backing pixmap is the primary drawable.  If no backing pixmap
is employed, then the window is the primary drawable.  This field is necessary
to distinguish multiple ghostscripts rendering to separate pixmaps where the
GHOSTVIEW property was placed on the same window.

The PAGE message indicates that a "page" has completed.  Ghostscript will
wait until it receives a ClientMessage whose message_type is NEXT before
continuing.

The DONE message indicates that ghostscript has finished processing.

*/

#include "config.h"
#include <string.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>
#include <X11/Intrinsic.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "prefs.h"
#include "gtkgs.h"
#include "ggvutils.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

/* if POSIX O_NONBLOCK is not available, use O_NDELAY */
#if !defined(O_NONBLOCK) && defined(O_NDELAY)
#define O_NONBLOCK O_NDELAY
#endif

/* Default values to pass to gtk_gs_init */
typedef struct _GtkGSDefaults 
{
        gboolean   antialiased;
        gboolean   override_media;
        gint       default_page_media;
	gboolean   watch_doc;                /* Automatic reload if file changed */
        gboolean   override_orientation;
        gboolean   respect_eof;
        gint       fallback_orientation;
        gfloat     zoom_factor;
} GtkGSDefaults;

typedef void (*InterpreterMessageSignal)(GtkObject *object, gchar *msg,
                                         gpointer data);

enum { INTERPRETER_MESSAGE, LAST_SIGNAL };

static gboolean broken_pipe = FALSE;

/* 
   This variable will hold the defaults to use when creating new
   gtk_gs widgets.
*/
static GtkGSDefaults gtkgs_defaults =
{
     TRUE,   /* Antialised */
     TRUE,   /* override media */
     1,      /* default page media */
     TRUE,   /* Watch doc */
     FALSE,  /* Override orientation */
     FALSE,  /* Respect EOF */
     GTK_GS_ORIENTATION_PORTRAIT,   /* Orientation */
     1.0,   /* Zoom factor */
};

static void catchPipe(int i)
{
        g_print ("in catch Pipe!!!\n");
        broken_pipe = True;
}

/* Forward declarations */
static void gtk_gs_init (GtkGS *gs);
static void gtk_gs_class_init (GtkGSClass *klass);
static void gtk_gs_destroy (GtkObject *object);
static void gtk_gs_realize (GtkWidget *widget);
static void gtk_gs_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_gs_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint gtk_gs_widget_event(GtkWidget *widget, GdkEvent *event, gpointer data);
static void gtk_gs_value_adjustment_changed (GtkAdjustment *adjustment, gpointer data);
static void gtk_gs_interpreter_message(GtkGS *gs, gchar *msg, gpointer user_data);
static void gtk_gs_emit_error_msg (GtkGS *gs, const gchar *msg);
static void send_ps(GtkGS *gs, long begin, unsigned int len, gboolean close);
static void set_up_page (GtkGS *gs);
static void close_pipe (int p[2]);
static void interpreter_failed (GtkGS *gs);
static float compute_xdpi(void);
static float compute_ydpi(void);
static gboolean compute_size(GtkGS *gs);
static void output (gpointer data, gint source, GdkInputCondition condition);
static void input (gpointer data, gint source, GdkInputCondition condition);
static void stop_interpreter (GtkGS *gs);
static gint start_interpreter (GtkGS *gs);
gboolean computeSize(void);

static GtkWidgetClass *parent_class;

static GtkGSClass *gs_class;

static gint gtk_gs_signals[LAST_SIGNAL] = { 0 };

/* Static, private functions */
static void
gtk_gs_marshaller(GtkObject *object, GtkSignalFunc func,
                  gpointer func_data, GtkArg *args)
{
        InterpreterMessageSignal rfunc;

        rfunc = (InterpreterMessageSignal)func;

        (*rfunc)(object, GTK_VALUE_POINTER (args[0]), func_data);
}

static void
gtk_gs_init (GtkGS *gs)
{
        gs->bpixmap     = NULL;
        gs->use_bpixmap = TRUE;

        gs->current_page = -1;
        gs->disable_start = FALSE;
        gs->interpreter_pid = -1;

        gs->width  = -1;
        gs->height = -1;
        gs->busy      = FALSE;
        gs->changed   = FALSE;
        gs->gs_scanstyle = 0;
        gs->gs_filename = 0;
	gs->gs_filename_dsc = 0;
	gs->gs_filename_unc = 0;

	broken_pipe = FALSE;

        gs->structured_doc = FALSE;
        gs->reading_from_pipe = FALSE;
        gs->send_filename_to_gs = FALSE;

        gs->doc = NULL;
        gs->loaded = FALSE;

	gs->interpreter_input = -1;
	gs->interpreter_output = -1;
	gs->interpreter_err = -1;
	gs->interpreter_input_id = 0;
	gs->interpreter_output_id = 0;
	gs->interpreter_error_id = 0;

        gs->ps_input = NULL;
        gs->input_buffer = NULL;
        gs->input_buffer_ptr = NULL;
        gs->bytes_left = 0;
        gs->buffer_bytes_left = 0;

        gs->llx = 0;
        gs->lly = 0;
        gs->urx = 0;
        gs->ury = 0;
        gs->xdpi = compute_xdpi();
        gs->ydpi = compute_ydpi();

        gs->left_margin = 0;
        gs->top_margin = 0;
        gs->right_margin = 0;
        gs->bottom_margin = 0;
        gs->pages_marked = NULL;

        /* Set user defined defaults */
        gs->override_orientation = gtk_gs_get_default_override_orientation();
        gs->fallback_orientation = gtk_gs_get_default_orientation();
        gs->zoom_factor = gtk_gs_get_default_zoom_factor();
        gs->default_page_media = gtk_gs_get_default_page_media();
        gs->antialiased = gtk_gs_get_default_antialiased();
        gs->override_media = gtk_gs_get_default_override_media();
        gs->respect_eof = gtk_gs_get_default_respect_eof();
}

static void
gtk_gs_class_init (GtkGSClass *klass)
{
        GtkObjectClass *object_class;
        GtkWidgetClass *widget_class;

        object_class    = (GtkObjectClass *) klass;
        widget_class    = (GtkWidgetClass *) klass;
        parent_class    = gtk_type_class(gtk_widget_get_type());
        gs_class = klass;

	gtk_gs_signals[INTERPRETER_MESSAGE] = gtk_signal_new(
                       "interpreter_message", GTK_RUN_LAST, object_class->type,
                       GTK_SIGNAL_OFFSET(GtkGSClass, interpreter_message),
                       gtk_gs_marshaller, GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
        gtk_object_class_add_signals(object_class, gtk_gs_signals, LAST_SIGNAL);

        object_class->destroy = gtk_gs_destroy;

        widget_class->realize       = gtk_gs_realize;
        widget_class->size_request  = gtk_gs_size_request;
        widget_class->size_allocate = gtk_gs_size_allocate;

        /* Create atoms */
        klass->gs_atom        = gdk_atom_intern ("GHOSTVIEW", FALSE);
        klass->gs_colors_atom = gdk_atom_intern ("GHOSTVIEW_COLORS", FALSE);
        klass->next_atom      = gdk_atom_intern ("NEXT", FALSE);
        klass->page_atom      = gdk_atom_intern ("PAGE", FALSE);
        klass->done_atom      = gdk_atom_intern ("DONE", FALSE);
        klass->string_atom    = gdk_atom_intern ("STRING", FALSE);

        /* a default handler for "interpreter_message" signal */
        klass->interpreter_message = gtk_gs_interpreter_message;
}

/* free message as it was allocated in output() */
static void
gtk_gs_interpreter_message(GtkGS *gs, gchar *msg, gpointer user_data)
{
        /* g_print("interpreter message: %s\n", msg); */
        g_free(msg);
}

/* Clean all memory and temporal files */
static void 
gtk_gs_cleanup (GtkGS *gs)
{
        g_return_if_fail (gs != NULL);
        g_return_if_fail (GTK_IS_GS (gs));

        stop_interpreter(gs);

	if (gs->gs_psfile) {
		fclose (gs->gs_psfile);
		gs->gs_psfile = NULL;
	}
	if (gs->gs_filename) {
                g_free(gs->gs_filename);
		gs->gs_filename = NULL;
	}
	if (gs->doc) {
                psfree(gs->doc);
		gs->doc = NULL;
	}
        if (gs->gs_filename_dsc){
                unlink(gs->gs_filename_dsc);
                g_free(gs->gs_filename_dsc);
                gs->gs_filename_dsc = NULL;
        }
        if (gs->gs_filename_unc){
                unlink(gs->gs_filename_unc);
                g_free(gs->gs_filename_unc);
                gs->gs_filename_unc = NULL;
        }
        if (gs->pages_marked) {
                g_free(gs->pages_marked);
                gs->pages_marked = NULL;
	}
        if(gs->timer_tag) {
 		gtk_timeout_remove (gs->timer_tag);
 		gs->timer_tag = 0;
 	}
        gs->llx = 0;
        gs->lly = 0;
        gs->urx = 0;
        gs->ury = 0;
}

static void
gtk_gs_destroy (GtkObject *object)
{
        GtkGS *gs;
  
        g_return_if_fail (object != NULL);
        g_return_if_fail (GTK_IS_GS (object));
        
        gs = GTK_GS (object);

        gtk_gs_cleanup(gs);

        if (GTK_OBJECT_CLASS (parent_class)->destroy)
                (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* FIXME: I'm not sure if all this is supposed to be here 
 * this is just a quick hack so that this can be called whenever
 * something changes.
 */
static void
gtk_gs_munge_adjustments (GtkGS *gs)
{
        gint x, y;

        gdk_window_get_position (gs->pstarget, &x, &y);

	/* g_print("Munge adj: waw=%d gsw=%d hau=%d hal=%d \n", gs->widget.allocation.width, gs->width, gs->hadj->upper, gs->hadj->lower);
	 */

	/* 
	 * This is a bit messy:
	 * we want to make sure that we do the right thing if dragged.
	 */
	if (gs->widget.allocation.width >= gs->width) {
		x = (gs->widget.allocation.width - gs->width)/2;
		gs->hadj->value = (gs->hadj->upper - gs->hadj->lower)/2;
		gs->hadj->page_size = gs->hadj->upper - gs->hadj->lower;
	} else {
		if (x > 0)
			x = 0;
		else if (gs->widget.allocation.width > x + gs->width) 
			x = gs->widget.allocation.width - gs->width;
		gs->hadj->page_size = ((gfloat) gs->widget.allocation.width)/gs->width;
		gs->hadj->value = (gs->hadj->upper-gs->hadj->lower) *
			((gfloat) (gs->widget.allocation.width/2 - x))/gs->width;
	}
	if (gs->widget.allocation.height >= gs->height) {
		y = (gs->widget.allocation.height - gs->height)/2;
		gs->vadj->value = (gs->vadj->upper - gs->vadj->lower)/2;
		gs->vadj->page_size = gs->vadj->upper - gs->vadj->lower;
	} else {
		if (y > 0)
			y = 0;
		else if (gs->widget.allocation.height > y + gs->height) 
			y = gs->widget.allocation.height - gs->height;
		gs->vadj->page_size = ((gfloat) gs->widget.allocation.height)/gs->height;
		gs->vadj->value = (gs->vadj->upper-gs->vadj->lower) *
			((gfloat) (gs->widget.allocation.height/2 - y))/gs->height;
	}

	/* now we want to update the slider. */
	gdk_window_move (gs->pstarget, x, y);
	gtk_adjustment_changed (gs->hadj);
	gtk_adjustment_changed (gs->vadj);
}
			  

static void
gtk_gs_realize (GtkWidget *widget)
{
        GtkGS  *gs;
        GdkWindowAttr  attributes;
        gint           attributes_mask;

        g_return_if_fail (widget != NULL);
        g_return_if_fail (GTK_IS_GS (widget));

        gs = GTK_GS (widget);

        /* we set up the main widget! */
        GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.x           = widget->allocation.x;
        attributes.y           = widget->allocation.y;
        attributes.width       = widget->allocation.width;
        attributes.height      = widget->allocation.height;
        attributes.wclass      = GDK_INPUT_OUTPUT;
        attributes.visual      = gtk_widget_get_visual (widget);
        attributes.colormap    = gtk_widget_get_colormap (widget);
        attributes.event_mask  = gtk_widget_get_events (widget);
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

        widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);
        gdk_window_set_user_data (widget->window, gs);
        widget->style = gtk_style_attach (widget->style, widget->window);
        gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

        /* now we set up the child window.  This is the one that ps actually draws too. */
        attributes.x = 0;
        attributes.y = 0;

        gs->pstarget = gdk_window_new (widget->window, &attributes, attributes_mask);
        gdk_window_set_user_data (gs->pstarget, widget);
        /* gdk_window_show (gs->pstarget); */
        gdk_window_clear (gs->pstarget);
        gtk_style_set_background (widget->style, gs->pstarget, GTK_STATE_ACTIVE);

        gs->width = 0;
        gs->height = 0;

        gtk_gs_set_pagemedia(gs, -1, 0);

        if ((gs->width > 0) && (gs->height > 0) && GTK_WIDGET_REALIZED(gs)) {
		gtk_gs_munge_adjustments (gs);
        }

        gtk_signal_connect(GTK_OBJECT(widget), "event",
                           GTK_SIGNAL_FUNC(gtk_gs_widget_event), gs);
}


static void
gtk_gs_size_request (GtkWidget      *widget,
                     GtkRequisition *requisition)
{
        GtkGS *gs = GTK_GS (widget);

	compute_size(gs);
	if(gs->width == 0 || gs->height == 0)
                return;
	requisition->width = gs->width;
	requisition->height = gs->height;
}

static void
gtk_gs_size_allocate (GtkWidget     *widget,
                      GtkAllocation *allocation)
{
        GtkGS *gs = GTK_GS (widget);
        GdkEventConfigure event;
        
        g_return_if_fail (widget != NULL);
        g_return_if_fail (GTK_IS_GS (widget));
        g_return_if_fail (allocation != NULL);

        widget->allocation = *allocation;
        if (GTK_WIDGET_REALIZED (widget)) {
                gdk_window_move_resize (widget->window,
                                        allocation->x, allocation->y,
                                        allocation->width, allocation->height);

                event.type = GDK_CONFIGURE;
                event.window = widget->window;
                event.x = allocation->x;
                event.y = allocation->y;
                event.width = allocation->width;
                event.height = allocation->height;

                gtk_widget_event (widget, (GdkEvent*) &event);
        }
        /* 
         * update the adjustment if necessary (ie. a resize);
         */
        if ((gs->width > 0) && (gs->height >0) && GTK_WIDGET_REALIZED(gs)) {
		gtk_gs_munge_adjustments (gs);
        }
}

static gboolean 
gtk_gs_widget_event(GtkWidget *widget,
                    GdkEvent *event,
                    gpointer data)
{
        GtkGS *gs = (GtkGS *)data;

	if (event->type != GDK_CLIENT_EVENT) 
                return FALSE;

        /* the first long is the window to communicate with gs,
         only if event if client_event */

        gs->message_window = event->client.data.l[0];
        /* g_print("Message window %ld\n", gs->message_window); */

	if (event->client.message_type == gs_class->page_atom) {
                /*
                  g_print("Interpreter completed: busy -> FALSE\n");
                */
                gs->busy = FALSE;
	}
        return TRUE;
}


static void
gtk_gs_value_adjustment_changed (GtkAdjustment *adjustment, gpointer data)
{
        GtkGS *gs;
        gint x, y, width, height, depth;
        gint newx, newy;
        
        g_return_if_fail (adjustment != NULL);
        g_return_if_fail (data != NULL);
        gs = GTK_GS (data);
        if (gs->bpixmap == NULL)
                return;

#if 0
        g_print("Adjustment %c: val = %f, page = %f, upper = %f, lower = %f\n",
                (adjustment == gs->hadj)?'H':'V',
                adjustment->value, adjustment->page_size,
                adjustment->upper, adjustment->lower);
#endif
   
        gdk_window_get_geometry (gs->pstarget, &x, &y, &width, &height, &depth);
        if (gs->width <= gs->widget.allocation.width)
                newx = (gs->widget.allocation.width-gs->width)/2;
        else
                newx = gs->widget.allocation.width/2 -
                        (gs->hadj->value - gs->hadj->lower) * width /
                        (gs->hadj->upper - gs->hadj->lower);
        if (gs->height <= gs->widget.allocation.height)
                newy = (gs->widget.allocation.height-gs->height)/2;
        else
                newy = gs->widget.allocation.height/2 -
                        (gs->vadj->value - gs->vadj->lower) * height /
                        (gs->vadj->upper - gs->vadj->lower);
        gdk_window_move (gs->pstarget, newx, newy);
}

void 
gtk_gs_center_page(GtkGS *gs)
{
	g_return_if_fail (gs != NULL);

        gdk_window_move (gs->pstarget,
                         (gs->widget.allocation.width - gs->width)/2,
                         (gs->widget.allocation.height - gs->height)/2);
        gs->hadj->page_size = ((gfloat) gs->widget.allocation.width)/gs->width;
        gs->vadj->page_size = ((gfloat) gs->widget.allocation.height)/gs->height;
        gs->hadj->value = (gs->hadj->upper - gs->hadj->lower)/2;
        gs->vadj->value = (gs->vadj->upper - gs->vadj->lower)/2;
        gtk_adjustment_changed (gs->hadj);
        gtk_adjustment_changed (gs->vadj);
}

void
gtk_gs_scroll(GtkGS *gs, gint x_delta, gint y_delta)
{
        gfloat hval, vval;

        hval = gs->hadj->value + ((gfloat)x_delta)/gs->width;
        vval = gs->vadj->value + ((gfloat)y_delta)/gs->height;
        if(hval <= gs->hadj->upper - gs->hadj->page_size/2 &&
           hval >= gs->hadj->lower + gs->hadj->page_size/2)
                gtk_adjustment_set_value(gs->hadj, hval);
        if(vval <= gs->vadj->upper - gs->vadj->page_size/2 &&
           vval >= gs->vadj->lower + gs->vadj->page_size/2)
                gtk_adjustment_set_value(gs->vadj, vval);
}

void
gtk_gs_set_center(GtkGS *gs, gfloat hval, gfloat vval)
{
        if(hval <= gs->hadj->upper - gs->hadj->page_size/2 &&
           hval >= gs->hadj->lower + gs->hadj->page_size/2)
                gtk_adjustment_set_value(gs->hadj, hval);
        if(vval <= gs->vadj->upper - gs->vadj->page_size/2 &&
           vval >= gs->vadj->lower + gs->vadj->page_size/2)
                gtk_adjustment_set_value(gs->vadj, vval);
}        

static void
send_ps(GtkGS *gs, long begin, unsigned int len, gboolean close)
{
        struct record_list *ps_new;
        
        if (gs->interpreter_input < 0) {
                g_print ("no pipe to gs\nerror in send_ps\n");
                return;
        }

        ps_new = (struct record_list *) g_malloc(sizeof (struct record_list));
        ps_new->fp = gs->gs_psfile;
        ps_new->begin = begin;
        ps_new->len = len;
        ps_new->seek_needed = TRUE;
        ps_new->close = close;
        ps_new->next = NULL;
        
        if (gs->input_buffer == NULL) {
                gs->input_buffer = g_malloc(MAX_BUFSIZE);
        }
        
        if (gs->ps_input == NULL) {
                gs->input_buffer_ptr = gs->input_buffer;
                gs->bytes_left = len;
                gs->buffer_bytes_left = 0;
                gs->ps_input = ps_new;
                gs->interpreter_input_id = 
                        gdk_input_add(gs->interpreter_input,
                                      GDK_INPUT_WRITE,
                                      input,
                                      gs);
        } else {
                struct record_list *p = gs->ps_input;
                while (p->next != NULL) {
                        p = p->next;
                }
                p->next = ps_new;
        }
}

static void
set_up_page (GtkGS *gs) 
     /* 
      * This is used to prepare the widget internally for
      * a new document. It sets gs->pstarget to the
      * correct size and position, and updates the 
      * adjustments appropriately.
      *
      * It is not meant to be used every time a specific page
      * is selected.
      *
      * NOTE: It expects the widget is realized.
      */
{ 
        guint orientation;
        char               buf[1024];
        GdkPixmapPrivate  *pprivate;
        GdkColormap *colormap;
        GdkGC *fill;
        GdkColor white = {0, 0xFFFF, 0xFFFF, 0xFFFF}; /* pixel, r, g, b */
        gint w, h;
#ifdef HAVE_SETLOCALE
        char *savelocale;
#endif

        /* Do we have to check if the actual geometry changed? */
        
        stop_interpreter(gs);
        
        orientation = gtk_gs_get_orientation(gs);
        
        if ((gs->use_bpixmap) && compute_size (gs)) {
                gdk_flush(); 

                if (gs->bpixmap) {
                        gdk_pixmap_unref(gs->bpixmap);
                        gs->bpixmap = NULL;
                }
                gs->bpixmap = gdk_pixmap_new(gs->pstarget, gs->width, gs->height, -1);

                /* clear new pixmap (set to white) */
                fill = gdk_gc_new (gs->pstarget);
                colormap = gtk_widget_get_colormap (GTK_WIDGET(gs));
                gdk_color_alloc (colormap, &white);
                gdk_gc_set_foreground (fill, &white);
                gdk_draw_rectangle (gs->bpixmap, fill, TRUE,
                                    0, 0, gs->width, gs->height);
                gdk_gc_unref(fill);

                gdk_window_set_back_pixmap(gs->pstarget, gs->bpixmap, FALSE);

                gdk_window_resize (gs->pstarget, gs->width, gs->height); 

                gdk_flush();
        }

#ifdef HAVE_SETLOCALE
        /* gs needs floating point parameters with '.' as decimal point
         * while some (european) locales use ',' instead, so we set the 
         * locale for this snprintf to "C".
         */
        savelocale = setlocale (LC_NUMERIC, "C");
#endif
        pprivate = (GdkPixmapPrivate *) gs->bpixmap;
        snprintf(buf, 1024, "%ld %d %d %d %d %d %f %f %d %d %d %d",
                 (pprivate ? pprivate->xwindow : 0L),
                 orientation * 90,
                 gs->llx, gs->lly,
                 gs->urx, gs->ury,
                 gs->xdpi * gs->zoom_factor,
                 gs->ydpi * gs->zoom_factor,
                 gs->left_margin,
                 gs->bottom_margin,
                 gs->right_margin,
                 gs->top_margin);

        /*        g_print("Sent to GS: %s\n", buf); */

#ifdef HAVE_SETLOCALE
        setlocale (LC_NUMERIC, savelocale);
#endif
        gdk_property_change(gs->pstarget,
                            gs_class->gs_atom,
                            gs_class->string_atom,
                            8,
                            GDK_PROP_MODE_REPLACE,
                            buf,
                            strlen(buf));
        gdk_flush(); 
}

static void
close_pipe(int p[2])
{
	if (p[0] != -1)
		close(p[0]);
	if (p[1] != -1)
                close(p[1]);
}

static gboolean
is_interpreter_ready(GtkGS *gs)
{
        /*        g_print("Is interpreter ready pid %d busy %d %lx [%s]",  
                  gs->interpreter_pid,gs->busy, gs->ps_input,gs->ps_input); */
        return (gs->interpreter_pid != -1 &&
                !gs->busy &&
                gs->ps_input == NULL);
}

/* GhostviewIsInterpreterRunning:
 * Returns True if the interpreter is running.
 */
static gboolean
is_interpreter_running(GtkGS *gs)
{
    return (gs->interpreter_pid != -1);
}

static void
interpreter_failed (GtkGS *gs)
{
        stop_interpreter (gs);
}

static void
output (gpointer data, gint source, GdkInputCondition condition)
{
        char buf[MAX_BUFSIZE+1], *msg;
        guint bytes = 0;
        GtkGS *gs = GTK_GS (data);
        
        if (source == gs->interpreter_output) {
                bytes = read(gs->interpreter_output, buf, MAX_BUFSIZE);
                if (bytes == 0) { /* EOF occurred */
                        close (gs->interpreter_output);
                        gs->interpreter_output = -1;
                        gdk_input_remove (gs->interpreter_output_id);
                        return;
                } else if (bytes == -1) {
                        /* trouble... */
                        interpreter_failed(gs);
                        return;
                }
        } else if (source == gs->interpreter_err) {
                bytes = read(gs->interpreter_err, buf, MAX_BUFSIZE);
                if (bytes == 0) { /* EOF occurred */
                        close(gs->interpreter_err);
                        gs->interpreter_err = -1;
                        gdk_input_remove (gs->interpreter_error_id);
                        return;
                } else if (bytes == -1) {
                        /* trouble... */
                        interpreter_failed(gs);
                        return;
                }
        }
        if (bytes > 0) {
                buf[bytes] = '\0';
                msg = g_strdup(buf);
                gtk_signal_emit(GTK_OBJECT(gs),
                                gtk_gs_signals[INTERPRETER_MESSAGE], msg);
        }
}

static void
input (gpointer data, gint source, GdkInputCondition condition)
{
        GtkGS *gs = GTK_GS (data);
        int bytes_written;
	void (*oldsig)(int);
	oldsig = signal(SIGPIPE, catchPipe);
                
        do {
                if (gs->buffer_bytes_left == 0) {
                        /* Get a new section if required */
                        if (gs->ps_input && gs->bytes_left == 0) {
                                struct record_list *ps_old = gs->ps_input;
                                gs->ps_input = ps_old->next;
                                if (ps_old->close) 
                                        fclose(ps_old->fp);
                                g_free((char *)ps_old);
                        }
                        /* Have to seek at the beginning of each section */
                        if (gs->ps_input && gs->ps_input->seek_needed) {

                                        fseek(gs->ps_input->fp,
                                              gs->ps_input->begin, SEEK_SET);
                                gs->ps_input->seek_needed = FALSE;
                                gs->bytes_left = gs->ps_input->len;
                        }

                        if (gs->bytes_left > MAX_BUFSIZE) {
                                gs->buffer_bytes_left =
                                        fread(gs->input_buffer,
                                              sizeof (char), MAX_BUFSIZE,
                                              gs->ps_input->fp);
                        } else if (gs->bytes_left > 0) {
                                gs->buffer_bytes_left =
                                        fread(gs->input_buffer,
                                              sizeof (char), gs->bytes_left,
                                              gs->ps_input->fp);
                        } else {
                                gs->buffer_bytes_left = 0;
                        }
                        if (gs->bytes_left > 0 && gs->buffer_bytes_left == 0) {
                                interpreter_failed(gs);	/* Error occurred */
                        }
                        gs->input_buffer_ptr = gs->input_buffer;
                        gs->bytes_left -= gs->buffer_bytes_left;
                }

                if (gs->buffer_bytes_left > 0) {
                        /* g_print (" writing: %s\n",gs->input_buffer_ptr); */
                        
                        bytes_written = write(gs->interpreter_input,
                                              gs->input_buffer_ptr,
                                              gs->buffer_bytes_left);

                        if (broken_pipe) {
                                gtk_gs_emit_error_msg(gs, g_strdup(_("Broken pipe.")));
                                broken_pipe = FALSE;
                                interpreter_failed(gs);
                        } else if (bytes_written == -1) {
                                if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
                                        interpreter_failed(gs);	/* Something bad happened */
                                }
                        } else {
                                gs->buffer_bytes_left -= bytes_written;
                                gs->input_buffer_ptr += bytes_written;
                        }
                }
        } while(gs->ps_input &&
                gs->buffer_bytes_left == 0);
        
        signal(SIGPIPE, oldsig);

        if (gs->ps_input == NULL && gs->buffer_bytes_left == 0) {
                if (gs->interpreter_input_id != 0) {
                        gdk_input_remove(gs->interpreter_input_id);
                        gs->interpreter_input_id = 0;
                }
        }
}


static int
start_interpreter (GtkGS *gs)
{
	int std_in[2] = {-1, -1}; /* pipe to interp stdin */
	int std_out[2]; /* pipe from interp stdout */
	int std_err[2]; /* pipe from interp stderr */
#define NUM_ARGS 100
#define NUM_GS_ARGS (NUM_ARGS - 10)

        char *argv[NUM_ARGS], *dir, *gv_env;
        char **gs_args;
        int argc = 0, i;

	if (!gs->gs_filename)
                return 0;

	stop_interpreter(gs);
        
	if (gs->disable_start == TRUE)
                return 0;
        
        /* set up the args... */
	/* change to directory where the input file is. This helps
	 * with postscript-files which include other files using
         * a relative path */
	dir = g_dirname(gs->gs_filename);
	chdir(dir);
	g_free(dir);

        /* set up the args... */
        gs_args = g_strsplit(gs_cmd, " ", NUM_GS_ARGS);
        for(i = 0; i < NUM_GS_ARGS && gs_args[i]; i++, argc++)
                argv[argc] = gs_args[i];
        if( gs->antialiased ) 
		argv[argc++] = "-sDEVICE=x11alpha";
        else 
		argv[argc++] = "-sDEVICE=x11";
        argv[argc++] = "-dNOPAUSE";
        argv[argc++] = "-dQUIET";
        /* I assume we do _not_ want to change this... (: */
        argv[argc++] = "-dSAFER";

        /* set up the pipes */
        if (gs->send_filename_to_gs) {
                argv[argc++] = GTK_GS_GET_PS_FILE(gs);
                argv[argc++] = "-c";
                argv[argc++] = "quit";
        }
        else 
                argv[argc++] = "-";
   
        argv[argc++] = NULL;

        if (!gs->reading_from_pipe && !gs->send_filename_to_gs) {
                if (pipe (std_in) == -1) {
                        g_print("Unable to open pipe to ghostview\n");
                        return -1;
                }
        }
	if (pipe (std_out) == -1) {
		close_pipe(std_in);
		return -1;
	}
	if (pipe (std_err) == -1) {
		close_pipe(std_in);
		close_pipe(std_out);
		return -1;
	}

        gs->busy = TRUE;
	gs->interpreter_pid = fork();
	switch (gs->interpreter_pid) {
	case -1: /* error */
		close_pipe(std_in);
		close_pipe(std_out);
		close_pipe(std_err);
		return -2;
		break;
	case 0: /* child */
		close(std_out[0]);
                dup2(std_out[1], 1);
                close(std_out[1]);

		close(std_err[0]);
                dup2(std_err[1], 2);
                close (std_err[1]);

                if (!gs->reading_from_pipe) {
                        if (gs->send_filename_to_gs) {
                                int stdinfd;
                                /* just in case gs tries to read from stdin */
                                stdinfd = open ("/dev/null", O_RDONLY);
                                if (stdinfd != 0) {
                                        dup2 (stdinfd, 0);
                                        close (stdinfd);
                                }
                        } else {
                                close(std_in[1]);
                                dup2(std_in[0], 0);
                                close(std_in[0]);
                        }
                }

                gv_env = g_strdup_printf ("GHOSTVIEW=%ld",
                                          ((GdkWindowPrivate*)(gs->pstarget))->xwindow);
                putenv (gv_env);
                execvp(argv[0], argv);
                
                /* Notify error */
                g_print("Unable to execute [%s]\n", argv[0]);
                g_strfreev(gs_args);
                g_free(gv_env);
		_exit(1);
		break;
	default: /* parent */
                if (!gs->send_filename_to_gs && !gs->reading_from_pipe) {
                        int result;
			close(std_in[0]);
                        /* use non-blocking IO for pipe to ghostscript */
			result = fcntl (std_in[1], F_GETFL, 0);
			fcntl (std_in[1], F_SETFL, result | O_NONBLOCK);
                        gs->interpreter_input = std_in[1];
		} else {
               		gs->interpreter_input = -1;
                }
                close (std_out[1]);
		gs->interpreter_output = std_out[0];
                close (std_err[1]);
		gs->interpreter_err = std_err[0];
                gs->interpreter_output_id =
                        gdk_input_add (std_out[0], GDK_INPUT_READ, output, gs);
                gs->interpreter_error_id = 
                        gdk_input_add (std_err[0], GDK_INPUT_READ, output, gs);
		break;
	}
        return TRUE;
}

static void
stop_interpreter (GtkGS *gs)
{
        if (gs->interpreter_pid >= 0) {
                kill(gs->interpreter_pid, SIGTERM);
                wait(0);
		gs->interpreter_pid = -1;
	}

	if (gs->interpreter_input >= 0) {
		close(gs->interpreter_input);
		gs->interpreter_input = -1;
                if (gs->interpreter_input_id != 0) {
                        gdk_input_remove(gs->interpreter_input_id);
                        gs->interpreter_input_id = 0;
                }
                while (gs->ps_input) {
                        struct record_list *ps_old = gs->ps_input;
                        gs->ps_input = gs->ps_input->next;
                        if (ps_old->close)
                                fclose (ps_old->fp);
                        g_free ((char *)ps_old);
                }
	}

	if (gs->interpreter_output >= 0) {
		close(gs->interpreter_output);
		gs->interpreter_output = -1;
                if (gs->interpreter_output_id) {
                        gdk_input_remove(gs->interpreter_output_id);
                        gs->interpreter_output_id = 0;
                }
	}
	
	if (gs->interpreter_err >= 0) {
		close(gs->interpreter_err);
		gs->interpreter_err = -1;
                if (gs->interpreter_error_id) {
                        gdk_input_remove(gs->interpreter_error_id);
                        gs->interpreter_error_id = 0;
                }
	}

	gs->busy = FALSE;
}

/* publicly accessible functions */

guint
gtk_gs_get_type (void)
{
        static guint gs_type = 0;

        if (!gs_type) {
                GtkTypeInfo gs_info = { "GtkGS",
                                        sizeof (GtkGS),
                                        sizeof (GtkGSClass),
                                        (GtkClassInitFunc) gtk_gs_class_init,
                                        (GtkObjectInitFunc) gtk_gs_init,
                                        (GtkArgSetFunc) NULL,
                                        (GtkArgGetFunc) NULL };
                gs_type = gtk_type_unique (gtk_widget_get_type (), &gs_info);
        }

        return gs_type;
}

GtkWidget *
gtk_gs_new (GtkAdjustment *hadj, GtkAdjustment *vadj)
{
	GtkGS *gs = gtk_type_new(gtk_gs_get_type());
        gs->hadj = hadj;
        gs->vadj = vadj;

        /* we set the adjustment widget to a default size... */
        hadj->lower = 0.0;
        hadj->upper = 1.0;
        hadj->value = 0.5;
        hadj->page_size = 1.0;
        vadj->lower = 0.0;
        vadj->upper = 1.0;
        vadj->value = 0.5;
        vadj->page_size = 1.0;

        gtk_signal_connect (GTK_OBJECT (hadj), "value_changed",
                            (GtkSignalFunc) gtk_gs_value_adjustment_changed,
                            (gpointer) gs);
        gtk_signal_connect (GTK_OBJECT (vadj), "value_changed",
                            (GtkSignalFunc) gtk_gs_value_adjustment_changed,
                            (gpointer) gs);


	return GTK_WIDGET(gs);
}


GtkWidget *
gtk_gs_new_from_file(GtkAdjustment *hadj, GtkAdjustment *vadj, char *fname)
{
	GtkWidget *gs = gtk_gs_new(hadj, vadj);
	gtk_gs_load(GTK_GS(gs), fname);
	return gs;
}


/*
 * Show error message -> send signal "interpreter_message"
 */
static void gtk_gs_emit_error_msg (GtkGS *gs, const gchar *msg)
{
	gtk_signal_emit (GTK_OBJECT (gs),
			 gtk_gs_signals [INTERPRETER_MESSAGE],
			 g_strdup (msg));
}


/*
 * Decompress gs->gs_filename if necessary
 * Set gs->filename_unc to the name of the uncompressed file or NULL.
 * Error reporting via signal 'interpreter_message'
 * Return name of input file to use or NULL on error..
 */
static gchar *check_filecompressed (GtkGS *gs)
{
	FILE *file;
	gchar buf[1024];
	gchar *filename, *filename_unc, *filename_err, *cmd, *cmdline;

	cmd = NULL;

	if ((file = fopen (gs->gs_filename, "r"))
	    && (fread (buf, sizeof (gchar), 3, file) == 3)) {
		if ((buf[0] == '\037')
		    && ((buf[1] == '\235') || (buf[1] == '\213'))) {
			/* file is gzipped or compressed */
			cmd = gs_ungzip_cmd;
		} else if (strncmp (buf, "BZh", 3) == 0) {
			/* file is compressed with bzip2 */
			cmd = gs_unbzip2_cmd;
		}
	}
	fclose (file);

	if (!cmd)
		return gs->gs_filename;

	/* do the decompression */

	filename = ggv_quote_filename (gs->gs_filename);
	filename_unc = tempnam (g_get_tmp_dir(), "unc");
	filename_err = tempnam (g_get_tmp_dir(), "err");
	cmdline = g_strdup_printf ("%s %s >%s 2>%s", cmd,
				   filename, filename_unc, filename_err);
	if ((system (cmdline) == 0)
	    && ggv_file_readable (filename_unc)
	    && (ggv_file_length (filename_err) == 0)) {
		/* sucessfully uncompressed file */
		gs->gs_filename_unc = filename_unc;
	} else {
		/* report error */
		g_snprintf (buf, 1024, _("Error while decompressing file %s:\n"),
			   filename);
		gtk_gs_emit_error_msg (gs, buf);
		if (ggv_file_length (filename_err) > 0) {
			FILE *err;
			if ((err = fopen (filename_err, "r"))) {
				/* print file to message window */
				while (fgets (buf, 1024, err))
					gtk_gs_emit_error_msg (gs, buf);
				fclose (err);
			}
		}
		unlink (filename_unc);
		g_free (filename_unc);
		filename_unc = NULL;
	}
	unlink (filename_err);
	g_free (filename_err);
	g_free (cmdline);
	g_free (filename);
	return filename_unc;
}

/*
 * Check if gs->gs_filename or gs->gs_filename_unc is a pdf file and scan
 * pdf file if necessary.
 * Set gs->filename_dsc to the name of the dsc file or NULL.
 * Error reporting via signal 'interpreter_message'.
 */
static gchar *check_pdf (GtkGS *gs)
{
	FILE *file;
	gchar buf[1024], *filename;

	/* use uncompressed file as input if necessary */
	filename = (gs->gs_filename_unc ? gs->gs_filename_unc
		    : gs->gs_filename);

	if ((file = fopen (filename, "r"))
	    && (fread (buf, sizeof(char), 5, file) == 5)
	    && (strncmp (buf, "%PDF-", 5) == 0)) {
		/* we found a PDF file */
		gchar *fname, *filename_dsc, *filename_err, *cmd, *cmdline;
		fname = ggv_quote_filename (filename);
		filename_dsc = tempnam (g_get_tmp_dir(), "dsc");
		filename_err = tempnam (g_get_tmp_dir(), "err");
		cmd = g_strdup_printf (gs_scan_pdf_cmd, "\"", fname ,"\"", "\", filename_dsc");
		g_free (fname);
		/* this command (sometimes?) prints error messages to stdout! */
		cmdline = g_strdup_printf ("%s >%s 2>&1", cmd, filename_err);
		g_free (cmd);

		if ((system (cmdline) == 0)
		    && ggv_file_readable (filename_dsc)
		    && (ggv_file_length (filename_err) == 0)) {
			/* success */
			filename = gs->gs_filename_dsc = filename_dsc;
		} else {
			/* report error */
			g_snprintf (buf, 1024,
				    _("Error while scanning pdf file %s:\n"),
				   filename);
			gtk_gs_emit_error_msg (gs, buf);

			if (ggv_file_length (filename_err) > 0) {
				FILE *err;
				if ((err = fopen (filename_err, "r"))) {
					/* print file to message window */
					while (fgets (buf, 1024, err))
						gtk_gs_emit_error_msg (gs, buf);
				}
			}
			unlink (filename_dsc);
			g_free (filename_dsc);
			filename = NULL;
		}
		unlink (filename_err);
		g_free (filename_err);
		g_free (cmdline);
	}
	fclose (file);
	return filename;
}


gboolean
gtk_gs_load (GtkGS *gs, const gchar *fname)
{
        struct stat sbuf;
	int i;

	g_assert(gs != NULL);

        /* clean up previous document */
        gtk_gs_cleanup(gs);

	if (fname == NULL) {
                if(gdk_window_is_visible(gs->pstarget))
                        gdk_window_hide(gs->pstarget);
		return FALSE;
	}

 	/* prepare this document */

 	/* default values: no dsc information available  */
        gs->structured_doc = FALSE;
 	gs->send_filename_to_gs = TRUE;
	gs->current_page = -1;
        gs->loaded = FALSE;
        if(*fname == '/') {
                /* an absolute path */
                gs->gs_filename = g_strdup (fname);
        }
        else {
                /* path relative to our cwd: make it absolute */
                gchar *cwd = g_get_current_dir();
                gs->gs_filename = g_strconcat(cwd, "/", fname, NULL);
                g_free(cwd);
        }

	if ((gs->reading_from_pipe = (strcmp (fname, "-") == 0))) {
                gs->send_filename_to_gs = FALSE;
	} else {
                /*
                 * We need to make sure that the file is loadable/exists!
                 * otherwise we want to exit without loading new stuff...
                 */
		gchar *filename = NULL;

		if (!ggv_file_readable (fname)) {
			gchar buf[1024];
			g_snprintf (buf, 1024,
				    _("Cannot open file %s\n"), fname);
			gtk_gs_emit_error_msg (gs, buf);
		} else {
                        filename = check_filecompressed (gs);
			if (filename)
				filename = check_pdf (gs);
		}

		if (!filename ||
		    (gs->gs_psfile = fopen (filename, "r")) == NULL) {
                        gtk_gs_cleanup (gs);
                        return FALSE;
		}

                /* we grab the vital statistics!!! */
		gs->doc = psscan(gs->gs_psfile, gs->respect_eof);

                if (gs->doc == NULL) {
                        /* File does not seem to be a Postscript one */
                        /* We have to report error to the reader, do we do it here */
			gchar buf[1024];
			g_snprintf (buf, 1024,
				    _("Error while scanning file %s\n"), fname);
			gtk_gs_emit_error_msg (gs, buf);
                        gtk_gs_cleanup (gs);
                        /*
                         * if (gdk_window_is_visible (gs->pstarget))
			 *          gdk_window_hide (gs->pstarget);
                        */
                        return FALSE;
                }

                if ((!gs->doc->epsf && gs->doc->numpages > 0) || 
                    (gs->doc->epsf && gs->doc->numpages > 1)) {
                        gs->structured_doc = TRUE; 
			gs->send_filename_to_gs = FALSE;
			gs->pages_marked = g_new0 (gint, gs->doc->numpages);
		}
                
                /* We have to set up the orientation of the document */
                
                
                /* orientation can only be portrait, and landscape or none.
                 This is the document default. A document can have
                 pages in landscape and some in portrait */
                if (gs->override_orientation) {
                        /* If the orientation should be override... 
                           then gs->orientation has already the correct
                           value (it was set when the widget was created */
                        /* So do nothing */

                } else {
                        /* Otherwise, set the proper orientation for the doc */
                        switch  (gs->doc->orientation) {
                        case LANDSCAPE:
                                gs->real_orientation = GTK_GS_ORIENTATION_LANDSCAPE;
                                break;
                        case PORTRAIT:
                                gs->real_orientation = GTK_GS_ORIENTATION_PORTRAIT;
                                break;
                        case NONE:
                                gs->real_orientation = GTK_GS_ORIENTATION_NONE;
                                break;
                        default:
                                gs->real_orientation = GTK_GS_ORIENTATION_NONE;
                                g_print("Orientation not handled yet %d\n",
                                        gs->doc->orientation);
                                break;
                        }
                }
                stat(fname, &sbuf);
                gs->mtime = sbuf.st_mtime;
  	}
	gtk_widget_queue_resize (&(gs->widget));
        gs->loaded = TRUE;

	return TRUE;
}


gboolean
gtk_gs_next_page(GtkGS *gs)
{
	XEvent event;
	GdkWindowPrivate *pw;
	
	g_return_val_if_fail (gs != NULL, FALSE);

	if (gs->interpreter_pid == 0) { /* no interpreter active */ 
                /* g_print ("pid = 0\n"); */
		return FALSE;
        }

	if (gs->busy) { /* interpreter is busy */
                /* g_print ("int busy\n"); */
		return FALSE;
        }

	gs->busy = TRUE;
	pw = (GdkWindowPrivate *)gs->widget.window;

        /*
        g_print("Sending NEXT message to %ld\n", gs->message_window);
        */
	event.xclient.type          = ClientMessage;
	event.xclient.display       = gdk_display;
	event.xclient.window        = gs->message_window;
	event.xclient.message_type  = gs_class->next_atom;
	event.xclient.format        = 32;
        /*
        g_print ("sending event.");
        */
	gdk_send_xevent(gs->message_window, FALSE, 0, &event);
        /*
        g_print ("..sent\n");
        */
        gdk_flush();
	return TRUE;
}

gboolean
gtk_gs_goto_page (GtkGS *gs, gint page) 
{
        struct stat sbuf;

	g_assert (gs != NULL);
        
        if (!gs->gs_filename) {
                return FALSE;
        }

        if(!GTK_WIDGET_REALIZED(gs)) {
                g_assert("GS not realized!");
        }
        
        if (gs->gs_psfile) {
                /* has it changed? */
                if (!stat(gs->gs_filename, &sbuf) &&
                    gs->mtime != sbuf.st_mtime) {
                        gchar *fname = g_strdup(gs->gs_filename);
                        gtk_gs_load (gs, fname);
                        g_free(fname);
                }
        }

        /* range checking... */
        if (page < 0)
                page = 0;
        
        if (gs->structured_doc && gs->doc) {
                if (page >= gs->doc->numpages)
                        page = gs->doc->numpages - 1;
                
                if (page == gs->current_page && !gs->changed)
                        return FALSE;
                
                gs->changed = FALSE;
                gs->current_page = page;
                
                if (is_interpreter_ready (gs)) {
                        /*g_print ("the interpreter is ready!\n"); */
                        gtk_gs_next_page (gs);
                } else {
                        /* g_print ("the interpreter is not ready -- starting now\n"); */
                        gtk_gs_enable_interpreter (gs);
                        send_ps(gs, gs->doc->beginprolog, 
                                gs->doc->lenprolog, FALSE);
                        send_ps(gs, gs->doc->beginsetup, 
                                gs->doc->lensetup, FALSE);
                }
		
                send_ps(gs, gs->doc->pages[gs->current_page].begin,
                        gs->doc->pages[gs->current_page].len, FALSE);
        }        
        else {
                /* Unstructured document */
                /* In the case of non structured documents,
                   GS read the PS from the  actual file (via command
                   line. Hence, ggv only send a signal next page.
                   If ghostview is not running it is usually because
                   the last page of the file was displayed. In that
                   case, ggv restarts GS again and the first page is displayed.
                */
                   
                /* g_print("sending page [non structured]\n"); */

                if (!is_interpreter_ready (gs))
                        gtk_gs_enable_interpreter (gs);

		gtk_gs_next_page(gs);
        }
        return TRUE;
}

/*
 * set pagemedia sets the media from
 * if new_pagemedia is -1, then it is set to either
 *  a) the default settings of pageid, if they exist, or if pageid != -1.
 *  b) the default setting of the document, if it exists.
 *  c) the default setting of the widget.
 * otherwise, the new_pagemedia is used as the pagemedia
 */
gboolean
gtk_gs_set_pagemedia(GtkGS *gs, gint new_pagemedia, gint pageid)
{
        gint new_llx;
        gint new_lly;
        gint new_urx;
        gint new_ury;

	g_assert (gs != NULL);

        if(new_pagemedia == -1) {
                new_pagemedia = gs->default_page_media;
                if (!gs->override_media && gs->doc) {
                        /* If we have a document:
                           We use -- the page media (if specified)
                                  or the doc. media (if specified)
                                  or the page bbox (if specified)
                                  or the bounding box
                          */
                        if ((pageid >= 0) && (gs->doc->numpages > pageid) &&
                            (gs->doc->pages) && (gs->doc->pages[pageid].media)) {
                                new_pagemedia = gs->doc->pages[pageid].media - gs->doc->media;
                        }
                        else if (gs->doc->default_page_media != NULL) {
                                new_pagemedia = gs->doc->default_page_media - gs->doc->media;
                        }
                        else if((pageid >= 0) &&
                                (gs->doc->numpages > pageid) &&
                                (gs->doc->pages) &&
                                (gs->doc->pages[pageid].boundingbox[URX] >
                                 gs->doc->pages[pageid].boundingbox[LLX]) &&
                                (gs->doc->pages[pageid].boundingbox[URY] >
                                 gs->doc->pages[pageid].boundingbox[LLY])) {
                                new_pagemedia = -1;
                        }
                        else if((gs->doc->boundingbox[URX] > gs->doc->boundingbox[LLX]) &&
                                (gs->doc->boundingbox[URY] > gs->doc->boundingbox[LLY])) {
                                new_pagemedia = -1;
                        }
                }
        }

        /* Compute bounding box */
        if (!gs->doc) {
                new_llx = new_lly = 0;
                new_urx = papersizes[new_pagemedia].width;
                new_ury = papersizes[new_pagemedia].height;
        }
        else if ((gs->doc->epsf || new_pagemedia == -1)) { /* epsf or bbox */
                if((gs->doc->pages) &&
                   (gs->doc->pages[pageid].boundingbox[URX] > gs->doc->pages[pageid].boundingbox[LLX]) &&
                   (gs->doc->pages[pageid].boundingbox[URY] > gs->doc->pages[pageid].boundingbox[LLY])) {
                        /* use page bbox */
                        new_llx = gs->doc->pages[pageid].boundingbox[LLX];
                        new_lly = gs->doc->pages[pageid].boundingbox[LLY];
                        new_urx = gs->doc->pages[pageid].boundingbox[URX];
                        new_ury = gs->doc->pages[pageid].boundingbox[URY];
                }
                else if((gs->doc->boundingbox[URX] > gs->doc->boundingbox[LLX]) &&
                        (gs->doc->boundingbox[URY] > gs->doc->boundingbox[LLY])) {
                        /* use doc bbox */
                        new_llx = gs->doc->boundingbox[LLX];
                        new_lly = gs->doc->boundingbox[LLY];
                        new_urx = gs->doc->boundingbox[URX];
                        new_ury = gs->doc->boundingbox[URY];
                }
                new_pagemedia = gs->default_page_media;
        }
        else {
                new_llx = new_lly = 0;
                if (!gs->override_media && gs->doc->media &&
                    (new_pagemedia < gs->doc->nummedia)) {
                        new_urx = gs->doc->media[new_pagemedia].width;
                        new_ury = gs->doc->media[new_pagemedia].height;
                } else {
                        new_urx = papersizes[new_pagemedia].width;
                        new_ury = papersizes[new_pagemedia].height;
                }
        }

        /* If bounding box changed, setup for new size. */
                /* gtk_gs_disable_interpreter (gs); */
        if ((new_llx != gs->llx) || (new_lly != gs->lly) ||
            (new_urx != gs->urx) || (new_ury != gs->ury)) {
                gs->llx = new_llx;
                gs->lly = new_lly;
                gs->urx = new_urx;
                gs->ury = new_ury;
                gs->changed = TRUE;
        }
   
        if (gs->changed) {
                if(GTK_WIDGET_REALIZED (gs)) {
                        set_up_page(gs);
                        gtk_widget_queue_resize (&(gs->widget));
                }
                return TRUE;
        }

        return FALSE;
}

gboolean
gtk_gs_set_orientation (GtkGS *gs, gint orientation) {

        gint iOldOrientation = gtk_gs_get_orientation(gs);
        gs->fallback_orientation = orientation;
                
	g_assert (gs != NULL);
        g_assert (
                  (orientation == GTK_GS_ORIENTATION_PORTRAIT) ||
                  (orientation == GTK_GS_ORIENTATION_LANDSCAPE) ||
                  (orientation == GTK_GS_ORIENTATION_UPSIDEDOWN) ||
                  (orientation == GTK_GS_ORIENTATION_SEASCAPE)
                  );

        /* We are setting the fallback orientation */
        if (iOldOrientation != gtk_gs_get_orientation(gs)) {
                gs->changed = TRUE;
                if(GTK_WIDGET_REALIZED(gs))
                        set_up_page(gs);
                return TRUE;
        }
	gtk_widget_queue_resize (&(gs->widget));
        return FALSE;
}

void
gtk_gs_set_override_orientation (GtkGS *gs, gboolean bNewOverride) {
        
        gint iOldOrientation = gtk_gs_get_orientation(gs);
	g_assert (gs != NULL);

        gs->override_orientation = bNewOverride;
        
        /* If the current orientation is different from the 
           new orientation  then redisplay */
        if (iOldOrientation != gtk_gs_get_orientation(gs)) {
                gs->changed = TRUE;
                if(GTK_WIDGET_REALIZED(gs))
                        set_up_page(gs);
        }
	gtk_widget_queue_resize (&(gs->widget));
}

void
gtk_gs_set_zoom (GtkGS *gs, gfloat zoom) {
	g_assert (gs != NULL);

        if (gs->zoom_factor != zoom) { 
                gs->zoom_factor = zoom;
                if (GTK_WIDGET_REALIZED(gs)) 
                        set_up_page(gs);
                gs->changed = TRUE;
        }
	gtk_widget_queue_resize (&(gs->widget));
}

static float
compute_xdpi(void)
{
        return 25.4 * gdk_screen_width()/gdk_screen_width_mm();
}

static float
compute_ydpi(void)
{
        return 25.4 * gdk_screen_height()/gdk_screen_height_mm();
}

/* Compute new size of window, sets xdpi and ydpi if necessary.
 * returns True if new window size is different */
static gboolean
compute_size(GtkGS *gs)
{
        guint new_width;
        guint new_height;
        gboolean change = FALSE;
        gint orientation;

	/* width and height can be changed, calculate window size according */
	/* to xpdi and ydpi */
        orientation = gtk_gs_get_orientation(gs);

        switch(orientation) {
        case GTK_GS_ORIENTATION_PORTRAIT:
        case GTK_GS_ORIENTATION_UPSIDEDOWN:
                new_width = (gs->urx - gs->llx) / 72.0 *
                        gs->xdpi + 0.5;
                new_height = (gs->ury - gs->lly) / 72.0 *
                        gs->ydpi + 0.5;
                break;
        case GTK_GS_ORIENTATION_LANDSCAPE:
        case GTK_GS_ORIENTATION_SEASCAPE:
                new_width = (gs->ury - gs->lly) / 72.0 *
                        gs->xdpi + 0.5;
                new_height = (gs->urx - gs->llx) / 72.0 *
                        gs->ydpi + 0.5;
                break;
        }

        change = (new_width != gs->width * gs->zoom_factor) || (new_height != gs->height * gs->zoom_factor);
        gs->width = (gint) (new_width * gs->zoom_factor);
        gs->height = (gint) (new_height * gs->zoom_factor);
        if(GTK_WIDGET_REALIZED(gs)) {
                if(!gdk_window_is_visible(gs->pstarget) && gs->width > 0 && gs->height > 0)
                        gdk_window_show(gs->pstarget);
                gtk_gs_munge_adjustments (gs);
        }

        return (change);
}

gint
gtk_gs_enable_interpreter (GtkGS *gs)
{
	g_assert (gs != NULL);
        if (!gs->gs_filename)
                return 0;

	gs->disable_start = FALSE;
	if (GTK_WIDGET_REALIZED (gs)) {
		return start_interpreter (gs);
	} else {
		return 0;
	}
}


void
gtk_gs_disable_interpreter (GtkGS *gs)
{
	g_assert (gs != NULL);
	gs->disable_start = TRUE;
	if (GTK_WIDGET_REALIZED (GTK_WIDGET (gs)))
		stop_interpreter (gs);
}

gint
gtk_gs_get_orientation(GtkGS *gs)
{
	g_return_val_if_fail(gs != NULL, -1);
	g_return_val_if_fail(GTK_IS_GS(gs), -1);
        
        if (gs->override_orientation ||
            gs->real_orientation == GTK_GS_ORIENTATION_NONE)
                return gs->fallback_orientation;
        else
                return gs->real_orientation;
}

const gchar *
gtk_gs_document_title(GtkGS *gs)
{
	g_return_val_if_fail(gs != NULL, NULL);
	g_return_val_if_fail(GTK_IS_GS(gs), NULL);

        if (gs->doc && gs->doc->title)
		return gs->doc->title;
        
	return NULL;
}

guint
gtk_gs_document_numpages(GtkGS *widget)
{
	g_return_val_if_fail(widget != NULL, 0);
	g_return_val_if_fail(GTK_IS_GS(widget), 0);
        
        if (widget->doc) 
                return widget->doc->numpages;
        
	return 0;
}

const gchar *
gtk_gs_document_page_label(GtkGS *widget, int page)
{
	g_return_val_if_fail(widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_GS(widget), NULL);

        if (widget->doc && widget->doc->pages &&
            (widget->doc->numpages >= page))
		return widget->doc->pages[page-1].label;
        
	return NULL;
}

gint 
gtk_gs_count_marked_pages(GtkGS *widget)
{
        int i;
        int count=0;
	g_return_val_if_fail(widget != NULL, 0);
	g_return_val_if_fail(GTK_IS_GS(widget), 0);

        if (!(widget->structured_doc && widget->doc &&
              widget->pages_marked))
                return 0;

        for (i=0;i<widget->doc->numpages; i++)
                if (widget->pages_marked[i])
                        count++;
        return count;
}

void
gtk_gs_set_default_orientation(gint iNewOrientation) {
        g_assert (
                  (iNewOrientation == GTK_GS_ORIENTATION_PORTRAIT) ||
                  (iNewOrientation == GTK_GS_ORIENTATION_LANDSCAPE) ||
                  (iNewOrientation == GTK_GS_ORIENTATION_UPSIDEDOWN) ||
                  (iNewOrientation == GTK_GS_ORIENTATION_SEASCAPE)
                  );
        gtkgs_defaults.fallback_orientation = iNewOrientation;
}

void
gtk_gs_set_default_page_media(gint iNewPageMedia)
{
        gtkgs_defaults.default_page_media = iNewPageMedia;
}

gint
gtk_gs_get_default_page_media()
{
        return gtkgs_defaults.default_page_media;
}

void
gtk_gs_set_default_watch_doc(gint iNewWatchDoc)
{
        gtkgs_defaults.watch_doc = iNewWatchDoc;
}


gint
gtk_gs_get_default_watch_doc()
{
        return gtkgs_defaults.watch_doc;
}



gboolean
gtk_gs_get_default_override_media()
{
        return gtkgs_defaults.override_media;
}

void
gtk_gs_set_default_override_media(gboolean bOverMedia)
{
        gtkgs_defaults.override_media = bOverMedia;
}



gboolean
gtk_gs_get_default_override_orientation()
{
        return gtkgs_defaults.override_orientation;
}

void
gtk_gs_set_default_override_orientation(gboolean bOverOrien)
{
        gtkgs_defaults.override_orientation = bOverOrien;
}

gboolean
gtk_gs_get_default_antialiased()
{
        return gtkgs_defaults.antialiased;
}

void
gtk_gs_set_default_antialiased(gint iNewAntialiased)
{
        gtkgs_defaults.antialiased = iNewAntialiased;
}


gint
gtk_gs_get_default_orientation()
{
        return gtkgs_defaults.fallback_orientation;
}

gfloat
gtk_gs_get_default_zoom_factor()
{
        return gtkgs_defaults.zoom_factor;
}

void
gtk_gs_set_default_zoom_factor(gfloat fZoom)
{
        gtkgs_defaults.zoom_factor = fZoom;
}

gboolean
gtk_gs_get_default_respect_eof()
{
        return gtkgs_defaults.respect_eof;
}

void
gtk_gs_set_default_respect_eof(gboolean resp)
{
        gtkgs_defaults.respect_eof = resp;
}
