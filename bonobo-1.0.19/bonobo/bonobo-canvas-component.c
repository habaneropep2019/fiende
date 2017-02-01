/*
 * bonobo-canvas-component.c: implements the CORBA interface for
 * the Bonobo::Canvas:Item interface used in Bonobo::Views.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999-2001 Helix Code, Inc.
 */
#include <stdio.h>
#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/Bonobo.h>
#include <libgnomeui/gnome-canvas.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-canvas-component.h>

enum {
	SET_BOUNDS,
	EVENT,
	LAST_SIGNAL
};

static gint gcc_signals [LAST_SIGNAL] = { 0, 0, };

typedef BonoboCanvasComponent Gcc;
#define GCC(x) BONOBO_CANVAS_COMPONENT(x)

struct _BonoboCanvasComponentPrivate {
	GnomeCanvasItem   *item;
};

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

/* Returns the GnomeCanvasItemClass of an object */
#define ICLASS(x) GNOME_CANVAS_ITEM_CLASS ((GTK_OBJECT (x)->klass))

static GtkObjectClass *gcc_parent_class;

static gboolean
CORBA_SVP_Segment_to_SVPSeg (Bonobo_Canvas_SVPSegment *seg, ArtSVPSeg *art_seg)
{
	int i;

	art_seg->points = art_new (ArtPoint, seg->points._length);
	if (!art_seg->points)
		return FALSE;

	art_seg->dir = seg->up ? 0 : 1;
	art_seg->bbox.x0 = seg->bbox.x0;
	art_seg->bbox.x1 = seg->bbox.x1;
	art_seg->bbox.y0 = seg->bbox.y0;
	art_seg->bbox.y1 = seg->bbox.y1;

	art_seg->n_points = seg->points._length;

	for (i = 0; i < art_seg->n_points; i++){
		art_seg->points [i].x = seg->points._buffer [i].x;
		art_seg->points [i].y = seg->points._buffer [i].y;
	}

	return TRUE;
}

static void
free_seg (ArtSVPSeg *seg)
{
	g_assert (seg != NULL);
	g_assert (seg->points != NULL);
	
	art_free (seg->points);
}

/*
 * Encodes an ArtUta
 */
static Bonobo_Canvas_ArtUTA *
CORBA_UTA (ArtUta *uta)
{
	Bonobo_Canvas_ArtUTA *cuta;

	cuta = Bonobo_Canvas_ArtUTA__alloc ();
	if (!cuta)
		return NULL;

	if (!uta) {
		cuta->width = 0;
		cuta->height = 0;
		cuta->utiles._length = 0;
		cuta->utiles._maximum = 0;

		return cuta;
	}
	cuta->utiles._buffer = CORBA_sequence_Bonobo_Canvas_int32_allocbuf (uta->width * uta->height);
	cuta->utiles._length = uta->width * uta->height;
	cuta->utiles._maximum = uta->width * uta->height;
	if (!cuta->utiles._buffer) {
		CORBA_free (cuta);
		return NULL;
	}
		
	cuta->x0 = uta->x0;
	cuta->y0 = uta->y0;
	cuta->width = uta->width;
	cuta->height = uta->height;

	memcpy (cuta->utiles._buffer, uta->utiles, uta->width * uta->height * sizeof (ArtUtaBbox));

	return cuta;
}

static void
restore_state (GnomeCanvasItem *item, const Bonobo_Canvas_State *state)
{
	double affine [6];
	int i;

	for (i = 0; i < 6; i++)
		affine [i] = state->item_aff [i];

	gnome_canvas_item_affine_absolute (item->canvas->root, affine);
	item->canvas->pixels_per_unit = state->pixels_per_unit;
	item->canvas->scroll_x1 = state->canvas_scroll_x1;
	item->canvas->scroll_y1 = state->canvas_scroll_y1;
	item->canvas->zoom_xofs = state->zoom_xofs;
	item->canvas->zoom_yofs = state->zoom_yofs;
	GTK_LAYOUT (item->canvas)->xoffset = state->xoffset;
	GTK_LAYOUT (item->canvas)->yoffset = state->yoffset;
}

/* This is copied from gnome-canvas.c since it is declared static */
static void
invoke_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	int child_flags;
	double *child_affine;
        double i2w[6], w2c[6], i2c[6];

	child_flags = flags;
	if (!(item->object.flags & GNOME_CANVAS_ITEM_VISIBLE))
		child_flags &= ~GNOME_CANVAS_UPDATE_IS_VISIBLE;

	/* Apply the child item's transform */
        gnome_canvas_item_i2w_affine (item, i2w);
        gnome_canvas_w2c_affine (item->canvas, w2c);
        art_affine_multiply (i2c, i2w, w2c);
        child_affine = i2c;

	/* apply object flags to child flags */

	child_flags &= ~GNOME_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_UPDATE)
		child_flags |= GNOME_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_AFFINE)
		child_flags |= GNOME_CANVAS_UPDATE_AFFINE;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_CLIP)
		child_flags |= GNOME_CANVAS_UPDATE_CLIP;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_VIS)
		child_flags |= GNOME_CANVAS_UPDATE_VISIBILITY;

	if ((child_flags & (GNOME_CANVAS_UPDATE_REQUESTED
			    | GNOME_CANVAS_UPDATE_AFFINE
			    | GNOME_CANVAS_UPDATE_CLIP
			    | GNOME_CANVAS_UPDATE_VISIBILITY))
	    && GNOME_CANVAS_ITEM_CLASS (item->object.klass)->update)
		(* GNOME_CANVAS_ITEM_CLASS (item->object.klass)->update) (
			item, child_affine, clip_path, child_flags);
}

static Bonobo_Canvas_ArtUTA *
impl_Bonobo_Canvas_Component_update (PortableServer_Servant     servant,
				     const Bonobo_Canvas_State *state,
				     const Bonobo_Canvas_affine aff,
				     const Bonobo_Canvas_SVP   *clip_path,
				     CORBA_long                 flags,
				     CORBA_double              *x1, 
				     CORBA_double              *y1, 
				     CORBA_double              *x2, 
				     CORBA_double              *y2, 
				     CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	double affine [6];
	int i;
	ArtSVP *svp = NULL;
	Bonobo_Canvas_ArtUTA *cuta;

	restore_state (item, state);
	for (i = 0; i < 6; i++)
		affine [i] = aff [i];

	if (clip_path->_length > 0) {
		svp = art_alloc (sizeof (ArtSVP) + (clip_path->_length * sizeof (ArtSVPSeg)));
		if (svp == NULL)
			goto fail;

		svp->n_segs = clip_path->_length;
		
		for (i = 0; svp->n_segs; i++) {
			gboolean ok;
		
			ok = CORBA_SVP_Segment_to_SVPSeg (&clip_path->_buffer [i], &svp->segs [i]);

			if (!ok) {
				int j;

				for (j = 0; j < i; j++) {
					free_seg (&svp->segs [j]);
				}
				art_free (svp);
				goto fail;
			}
		}
	}
	
	invoke_update (item, affine, svp, flags);

	if (svp){
		for (i = 0; i < svp->n_segs; i++)
			free_seg (&svp->segs [i]);
		art_free (svp);
	}

 fail:
	if (getenv ("CC_DEBUG"))
		printf ("%g %g %g %g\n", item->x1, item->x2, item->y1, item->y2);
	*x1 = item->x1;
	*x2 = item->x2;
	*y1 = item->y1;
	*y2 = item->y2;

	cuta = CORBA_UTA (item->canvas->redraw_area);
	if (cuta == NULL) {
		CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return NULL;
	}

	/*
	 * Now, mark our canvas as fully up to date
	 */
	if (item->canvas->redraw_area) {
		art_uta_free (item->canvas->redraw_area);
		item->canvas->redraw_area = NULL;
	}
	item->canvas->need_redraw = FALSE;
	
	return cuta;
}

static GdkGC *the_gc = NULL;

static void
impl_Bonobo_Canvas_Component_realize (PortableServer_Servant  servant,
				      Bonobo_Canvas_window_id window,
				      CORBA_Environment      *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkWindow *gdk_window = gdk_window_foreign_new (window);

	if (gdk_window == NULL) {
		g_warning ("Invalid window id passed=0x%x", window);
		return;
	}

	if (!the_gc)
		the_gc = gdk_gc_new (gdk_window);

	item->canvas->layout.bin_window = gdk_window;
	ICLASS (item)->realize (item);
}

static void
impl_Bonobo_Canvas_Component_unrealize (PortableServer_Servant servant,
					CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);

	ICLASS (item)->unrealize (item);

	if (item->canvas->layout.bin_window) {
		gdk_pixmap_unref (item->canvas->layout.bin_window);
		item->canvas->layout.bin_window = NULL;
	}
}

static void
impl_Bonobo_Canvas_Component_map (PortableServer_Servant servant,
				  CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	
	ICLASS (item)->map (item);
}

static void
impl_Bonobo_Canvas_Component_unmap (PortableServer_Servant servant,
				    CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	
	ICLASS (item)->unmap (item);
}

static void
my_gdk_pixmap_foreign_release (GdkPixmap *pixmap)
{
	GdkWindowPrivate *priv = (GdkWindowPrivate *) pixmap;

	if (priv->ref_count != 1){
		g_warning ("This item is keeping a refcount to a foreign pixmap");
		return;
	}

	gdk_xid_table_remove (priv->xwindow);
	g_dataset_destroy (priv);
	g_free (priv);
}

static void
impl_Bonobo_Canvas_Component_draw (PortableServer_Servant        servant,
				   const Bonobo_Canvas_State    *state,
				   const Bonobo_Canvas_window_id drawable,
				   CORBA_short                   x,
				   CORBA_short                   y,
				   CORBA_short                   width,
				   CORBA_short                   height,
				   CORBA_Environment            *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkPixmap *pix;
	
	gdk_flush ();
	pix = gdk_pixmap_foreign_new (drawable);

	if (pix == NULL){
		g_warning ("Invalid window id passed=0x%x", drawable);
		return;
	}

	restore_state (item, state);
	ICLASS (item)->draw (item, pix, x, y, width, height);

	my_gdk_pixmap_foreign_release (pix);
	gdk_flush ();
}

static void
impl_Bonobo_Canvas_Component_render (PortableServer_Servant servant,
				     Bonobo_Canvas_Buf     *buf,
				     CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GnomeCanvasBuf canvas_buf;

	if (!(buf->flags & Bonobo_Canvas_IS_BUF)) {
		buf->rgb_buf._length = buf->row_stride * (buf->rect.y1 - buf->rect.y0);
		buf->rgb_buf._maximum = buf->rgb_buf._length;
		
		buf->rgb_buf._buffer = CORBA_sequence_CORBA_octet_allocbuf (
			buf->rgb_buf._length);
		CORBA_sequence_set_release (&buf->rgb_buf, TRUE);

		if (buf->rgb_buf._buffer == NULL) {
			CORBA_exception_set_system (
				ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
			return;
		}
	}

	canvas_buf.buf = buf->rgb_buf._buffer;
	
	canvas_buf.buf_rowstride = buf->row_stride;
	canvas_buf.rect.x0 = buf->rect.x0;
	canvas_buf.rect.x1 = buf->rect.x1;
	canvas_buf.rect.y0 = buf->rect.y0;
	canvas_buf.rect.y1 = buf->rect.y1;
	canvas_buf.bg_color = buf->bg_color;
	if (buf->flags & Bonobo_Canvas_IS_BG)
		canvas_buf.is_bg = 1;
	else
		canvas_buf.is_bg = 0;

	if (buf->flags & Bonobo_Canvas_IS_BUF)
		canvas_buf.is_buf = 1;
	else
		canvas_buf.is_buf = 0;

	ICLASS (item)->render (item, &canvas_buf);

	/* return */
	buf->flags =
		(canvas_buf.is_bg ? Bonobo_Canvas_IS_BG : 0) |
		(canvas_buf.is_buf ? Bonobo_Canvas_IS_BUF : 0);
}

static CORBA_boolean 
impl_Bonobo_Canvas_Component_contains (PortableServer_Servant servant,
				       CORBA_double           x,
				       CORBA_double           y,
				       CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GnomeCanvasItem *new_item;
	CORBA_boolean ret;
	
	if (getenv ("CC_DEBUG"))
		printf ("Point %g %g: ", x, y);
	ret = ICLASS (item)->point (item, x, y, 0, 0, &new_item) == 0.0;
	if (getenv ("CC_DEBUG"))
		printf ("=> %s\n", ret ? "yes" : "no");
	return ret;
}

static void
impl_Bonobo_Canvas_Component_bounds (PortableServer_Servant     servant,
				     const Bonobo_Canvas_State *state,
				     CORBA_double              *x1,
				     CORBA_double              *x2,
				     CORBA_double              *y1,
				     CORBA_double              *y2,
				     CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);

	restore_state (item, state);
	ICLASS (item)->bounds (item, x1, y1, x2, y2);
}

/*
 * Converts the event marshalled from the container into a GdkEvent
 */
static void
Bonobo_Gdk_Event_to_GdkEvent (const Bonobo_Gdk_Event *gnome_event, GdkEvent *gdk_event)
{
	switch (gnome_event->_d){
	case Bonobo_Gdk_FOCUS:
		gdk_event->type = GDK_FOCUS_CHANGE;
		gdk_event->focus_change.in = gnome_event->_u.focus.inside;
		return;
		
	case Bonobo_Gdk_KEY:
		if (gnome_event->_u.key.type == Bonobo_Gdk_KEY_PRESS)
			gdk_event->type = GDK_KEY_PRESS;
		else
			gdk_event->type = GDK_KEY_RELEASE;
		gdk_event->key.time = gnome_event->_u.key.time;
		gdk_event->key.state = gnome_event->_u.key.state;
		gdk_event->key.keyval = gnome_event->_u.key.keyval;
		gdk_event->key.length = gnome_event->_u.key.length;
		gdk_event->key.string = g_strdup (gnome_event->_u.key.str);
		return;
		
	case Bonobo_Gdk_MOTION:
		gdk_event->type = GDK_MOTION_NOTIFY;
		gdk_event->motion.time = gnome_event->_u.motion.time;
		gdk_event->motion.x = gnome_event->_u.motion.x;
		gdk_event->motion.y = gnome_event->_u.motion.y;
		gdk_event->motion.x_root = gnome_event->_u.motion.x_root;
		gdk_event->motion.y_root = gnome_event->_u.motion.y_root;
		gdk_event->motion.xtilt = gnome_event->_u.motion.xtilt;
		gdk_event->motion.ytilt = gnome_event->_u.motion.ytilt;
		gdk_event->motion.state = gnome_event->_u.motion.state;
		gdk_event->motion.is_hint = gnome_event->_u.motion.is_hint;
		return;
		
	case Bonobo_Gdk_BUTTON:
		switch (gnome_event->_u.button.type){
		case Bonobo_Gdk_BUTTON_PRESS:
			gdk_event->type = GDK_BUTTON_PRESS;
			break;
		case Bonobo_Gdk_BUTTON_RELEASE:
			gdk_event->type = GDK_BUTTON_RELEASE;
			break;
		case Bonobo_Gdk_BUTTON_2_PRESS:
			gdk_event->type = GDK_2BUTTON_PRESS;
			break;
		case Bonobo_Gdk_BUTTON_3_PRESS:
			gdk_event->type = GDK_3BUTTON_PRESS;
			break;
		}
		gdk_event->button.time   = gnome_event->_u.button.time;
		gdk_event->button.x      = gnome_event->_u.button.x;
		gdk_event->button.y      = gnome_event->_u.button.y;
		gdk_event->button.x_root = gnome_event->_u.button.x_root;
		gdk_event->button.y_root = gnome_event->_u.button.y_root;
		gdk_event->button.button = gnome_event->_u.button.button;
		return;
		
	case Bonobo_Gdk_CROSSING:
		if (gnome_event->_u.crossing.type == Bonobo_Gdk_ENTER)
			gdk_event->type = GDK_ENTER_NOTIFY;
		else
			gdk_event->type = GDK_LEAVE_NOTIFY;
		
		gdk_event->crossing.time   = gnome_event->_u.crossing.time;
		gdk_event->crossing.x      = gnome_event->_u.crossing.x;
		gdk_event->crossing.y      = gnome_event->_u.crossing.y;
		gdk_event->crossing.x_root = gnome_event->_u.crossing.x_root;
		gdk_event->crossing.y_root = gnome_event->_u.crossing.y_root;
		switch (gnome_event->_u.crossing.mode){
		case Bonobo_Gdk_NORMAL:
			gdk_event->crossing.mode = GDK_CROSSING_NORMAL;
			break;
			
		case Bonobo_Gdk_GRAB:
			gdk_event->crossing.mode = GDK_CROSSING_GRAB;
			break;
		case Bonobo_Gdk_UNGRAB:
			gdk_event->crossing.mode = GDK_CROSSING_UNGRAB;
			break;
		}
		return;
	}
	g_assert_not_reached ();
}

static void
free_event (GdkEvent *event)
{
	if (event->type == GDK_KEY_RELEASE || event->type == GDK_KEY_PRESS)
		g_free (event->key.string);
}

/*
 * Receives events from the container end, decodes it into a synthetic
 * GdkEvent and forwards this to the CanvasItem
 */
static CORBA_boolean
impl_Bonobo_Canvas_Component_event (PortableServer_Servant     servant,
				    const Bonobo_Canvas_State *state,
				    const Bonobo_Gdk_Event    *gnome_event,
				    CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkEvent gdk_event;
	int retval;

	Bonobo_Gdk_Event_to_GdkEvent (gnome_event, &gdk_event);

	restore_state (item, state);

	gtk_signal_emit_by_name (GTK_OBJECT (gcc), "event", &gdk_event);

	if (ICLASS (item)->event)
		retval = ICLASS (item)->event (item, &gdk_event);
	else
		retval = FALSE;

	free_event (&gdk_event);

	return retval;
}

static void
impl_Bonobo_Canvas_Component_setCanvasSize (PortableServer_Servant servant,
					    CORBA_short            x,
					    CORBA_short            y,
					    CORBA_short            width,
					    CORBA_short            height,
					    CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GtkAllocation alloc;

	alloc.x = x;
	alloc.y = y;
	alloc.width = width;
	alloc.height = height;

	gtk_widget_size_allocate (GTK_WIDGET (item->canvas), &alloc);
}

static void
impl_Bonobo_Canvas_Component_setBounds (PortableServer_Servant     servant,
					const Bonobo_Canvas_DRect *bbox,
					CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));

	gtk_signal_emit (GTK_OBJECT (gcc), gcc_signals [SET_BOUNDS], bbox, &ev);
}

static void
gcc_destroy (GtkObject *object)
{
	GnomeCanvasItem *item = BONOBO_CANVAS_COMPONENT (object)->priv->item;

	gtk_object_destroy (GTK_OBJECT (item->canvas));

	gcc_parent_class->destroy (object);
}

static void
gcc_finalize (GtkObject *object)
{
	Gcc *gcc = GCC (object);

	g_free (gcc->priv);

	gcc_parent_class->finalize (object);
}

static void
bonobo_canvas_component_class_init (BonoboCanvasComponentClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_Canvas_Component__epv *epv = &klass->epv;

	gcc_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy  = gcc_destroy;
	object_class->finalize = gcc_finalize;

	gcc_signals [SET_BOUNDS] = 
                gtk_signal_new ("set_bounds",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET (BonoboCanvasComponentClass, set_bounds), 
                                gtk_marshal_NONE__POINTER_POINTER,
                                GTK_TYPE_NONE, 2,
				GTK_TYPE_POINTER, GTK_TYPE_POINTER);

	gcc_signals [EVENT] = gtk_signal_new ("event", 
			GTK_RUN_LAST, object_class->type,
                        GTK_SIGNAL_OFFSET (BonoboCanvasComponentClass, event), 
                        gtk_marshal_BOOL__POINTER,
                        GTK_TYPE_BOOL, 1, GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, gcc_signals, LAST_SIGNAL);

	epv->update         = impl_Bonobo_Canvas_Component_update;
	epv->realize        = impl_Bonobo_Canvas_Component_realize;
	epv->unrealize      = impl_Bonobo_Canvas_Component_unrealize;
	epv->map            = impl_Bonobo_Canvas_Component_map;
	epv->unmap          = impl_Bonobo_Canvas_Component_unmap;
	epv->draw           = impl_Bonobo_Canvas_Component_draw;
	epv->render         = impl_Bonobo_Canvas_Component_render;
	epv->bounds         = impl_Bonobo_Canvas_Component_bounds;
	epv->event          = impl_Bonobo_Canvas_Component_event;
	epv->contains       = impl_Bonobo_Canvas_Component_contains;
	epv->setCanvasSize  = impl_Bonobo_Canvas_Component_setCanvasSize;
	epv->setBounds      = impl_Bonobo_Canvas_Component_setBounds;
}

static void
bonobo_canvas_component_init (GtkObject *object)
{
	Gcc *gcc = GCC (object);

	gcc->priv = g_new0 (BonoboCanvasComponentPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboCanvasComponent, 
			   Bonobo_Canvas_Component,
			   PARENT_TYPE,
			   bonobo_canvas_component);


/**
 * bonobo_canvas_component_construct:
 * @comp: a #BonoboCanvasComponent to initialize
 * @item: A #GnomeCanvasItem that is being exported
 *
 * Creates a CORBA server for the interface Bonobo::Canvas::Item
 * wrapping @item.
 *
 * Returns: The BonoboCanvasComponent.
 */
BonoboCanvasComponent *
bonobo_canvas_component_construct (BonoboCanvasComponent  *comp,
				  GnomeCanvasItem         *item)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);
	g_return_val_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp), NULL);

	comp->priv->item = item;

	return comp;
}

				  
/**
 * bonobo_canvas_component_new:
 * @item: A GnomeCanvasItem that is being exported
 *
 * Creates a CORBA server for the interface Bonobo::Canvas::Item
 * wrapping @item.
 *
 * Returns: The BonoboCanvasComponent.
 */
BonoboCanvasComponent *
bonobo_canvas_component_new (GnomeCanvasItem *item)
{
	BonoboCanvasComponent *comp;
	
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);
	
	comp = gtk_type_new (bonobo_canvas_component_get_type ());

	return bonobo_canvas_component_construct (comp, item);
}

/** 
 * bonobo_canvas_component_get_item:
 * @comp: A #BonoboCanvasComponent object
 *
 * Returns: The GnomeCanvasItem that this BonoboCanvasComponent proxies
 */
GnomeCanvasItem *
bonobo_canvas_component_get_item (BonoboCanvasComponent *comp)
{
	g_return_val_if_fail (comp != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp), NULL);

	return comp->priv->item;
}

/*
 * Hack root item
 *
 * This is a hack since we can not modify the existing GNOME Canvas to handle
 * this case.
 *
 * Here is the problem we are solving:
 *
 *    1. Items usually queue a request to be updated/redrawn by calling
 *       gnome_canvas_item_request_update().  This triggers in the container
 *       canvas an idle handler to be queued to update the display on the
 *       idle handler.
 *        
 *    2. There is no way we can catch this on the Canvas.
 *
 * To catch this we do:
 *
 *    3. replace the regular Canvas' root field (of type GnomeCanvasGroup)
 *       with a RootItemHack item.  This item has an overriden ->update method
 *       that will notify the container canvas on the container process about
 *       our update requirement. 
 */

static GnomeCanvasGroupClass *rih_parent_class;

typedef struct {
	GnomeCanvasGroup       group;
	Bonobo_Canvas_ComponentProxy proxy;
	GnomeCanvasItem *orig_root;
} RootItemHack;

typedef struct {
	GnomeCanvasGroupClass parent_class;
} RootItemHackClass;

static GtkType root_item_hack_get_type (void);
#define ROOT_ITEM_HACK_TYPE (root_item_hack_get_type ())
#define ROOT_ITEM_HACK(obj) (GTK_CHECK_CAST((obj), ROOT_ITEM_HACK_TYPE, RootItemHack))

static void
rih_destroy (GtkObject *obj)
{
	RootItemHack *rih = ROOT_ITEM_HACK (obj);

	bonobo_object_release_unref (rih->proxy, NULL);
	gtk_object_destroy (GTK_OBJECT (rih->orig_root));

	GTK_OBJECT_CLASS (rih_parent_class)->destroy (obj);
}

/*
 * Invoked by our local canvas when an update is requested,
 * we forward this to the container canvas
 */
static void
rih_update (GnomeCanvasItem *item, double affine [6], ArtSVP *svp, int flags)
{
	RootItemHack *rih = (RootItemHack *) item;
	CORBA_Environment ev;
	GnomeCanvasItemClass *gci_class = gtk_type_class (
					gnome_canvas_item_get_type ());

	CORBA_exception_init (&ev);
	Bonobo_Canvas_ComponentProxy_requestUpdate (rih->proxy, &ev);
	CORBA_exception_free (&ev);

	/*
	 * Mark our canvas and item as fully updated
	 */

	(* gci_class->update) (item, affine, svp, flags);

	if (item->canvas->redraw_area)
		art_uta_free (item->canvas->redraw_area);
	item->canvas->redraw_area = NULL;
	item->canvas->need_redraw = FALSE;
}

static void
rih_draw (GnomeCanvasItem *item, GdkDrawable *drawable, 
	  int x, int y, int width, int height)
{
	RootItemHack *rih = (RootItemHack *) item;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	Bonobo_Canvas_ComponentProxy_requestRedraw (
			rih->proxy, x, y, x + width, y + height, &ev);
	CORBA_exception_free (&ev);
}

static void
rih_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf)
{
	RootItemHack *rih = (RootItemHack *) item;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	Bonobo_Canvas_ComponentProxy_requestRedraw (
					rih->proxy, item->x1, item->y1, 
					item->x2, item->y2, &ev);
	CORBA_exception_free (&ev);
}

static void
rih_class_init (GnomeCanvasItemClass *item_class)
{
	rih_parent_class = gtk_type_class (gnome_canvas_group_get_type ());

	GTK_OBJECT_CLASS (item_class)->destroy  = rih_destroy;
	item_class->update = rih_update;
	item_class->draw = rih_draw;
	item_class->render = rih_render;
}
      
static GtkType
root_item_hack_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"RootItemHack",
			sizeof (RootItemHack),
			sizeof (RootItemHackClass),
			(GtkClassInitFunc) rih_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_canvas_group_get_type (), &info);
	}

	return type;
}

static RootItemHack *
root_item_hack_new (GnomeCanvas *canvas, Bonobo_Canvas_ComponentProxy proxy)
{
	RootItemHack *item_hack;

	item_hack = gtk_type_new (root_item_hack_get_type ());
	item_hack->proxy = proxy;
	item_hack->orig_root = canvas->root;
	GNOME_CANVAS_ITEM (item_hack)->canvas = canvas;

	return item_hack;
}

/**
 * bonobo_canvas_new:
 * @is_aa: Flag indicating is antialiased canvas is desired
 * @proxy: Remote proxy for the component this canvas will support
 *
 * Returns: A #GnomeCanvas with the root replaced by a forwarding item.
 */ 
GnomeCanvas *
bonobo_canvas_new (gboolean is_aa, Bonobo_Canvas_ComponentProxy proxy)
{
	GnomeCanvas *canvas;
	GnomeCanvasItem *orig_root;

	if (is_aa) {
		gdk_rgb_init ();
		canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
	} else
		canvas = GNOME_CANVAS (gnome_canvas_new ());

	orig_root = canvas->root;

	canvas->root = GNOME_CANVAS_ITEM (root_item_hack_new (canvas, proxy));

	gtk_widget_realize (GTK_WIDGET (canvas));
	
	/* Gross */
	GTK_WIDGET_SET_FLAGS (canvas, GTK_VISIBLE | GTK_MAPPED);

	return canvas;
}

/**
 * bonobo_canvas_component_grab:
 * @comp: A #BonoboCanvasComponent object
 * @mask: Mask of events to grab
 * @cursor: #GdkCursor to display during grab
 * @time: Time of last event before grab
 *
 * Grabs the mouse focus via a call to the remote proxy.
 */
void
bonobo_canvas_component_grab (BonoboCanvasComponent *comp, guint mask,
			      GdkCursor *cursor, guint32 time)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	Bonobo_Canvas_ComponentProxy_grabFocus (
		ROOT_ITEM_HACK (comp->priv->item->canvas->root)->proxy, 
		mask, cursor->type, time, &ev);
	CORBA_exception_free (&ev);
}

/**
 * bonobo_canvas_component_ungrab:
 * @comp: A #BonoboCanvasComponent object
 * @time: Time of last event before grab
 *
 * Grabs the mouse focus via a call to the remote proxy.
 */
void
bonobo_canvas_component_ungrab (BonoboCanvasComponent *comp, guint32 time)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	Bonobo_Canvas_ComponentProxy_ungrabFocus (
		ROOT_ITEM_HACK (comp->priv->item->canvas->root)->proxy, time, &ev);
	CORBA_exception_free (&ev);
}

/**
 * bonobo_canvas_component_get_ui_container:
 * @comp: A #BonoboCanvasComponent object
 *
 * Returns: The UI container for the component's remote proxy.
 */
Bonobo_UIContainer
bonobo_canvas_component_get_ui_container (BonoboCanvasComponent *comp)
{
	Bonobo_UIContainer corba_uic;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	corba_uic = Bonobo_Canvas_ComponentProxy_getUIContainer (
			ROOT_ITEM_HACK (comp->priv->item->canvas->root)->proxy, &ev);
	CORBA_exception_free (&ev);

	return corba_uic;
}

