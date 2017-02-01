/**
 * bonobo-bonobo-item.h: Canvas item implementation for embedding remote canvas-items
 *
 * Author:
 *     Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_CANVAS_ITEM_H_
#define _BONOBO_CANVAS_ITEM_H_

#include <libgnome/gnome-defs.h>
#include <libgnomeui/gnome-canvas.h>
#include <bonobo/bonobo-embeddable.h>

#define BONOBO_CANVAS_ITEM(obj)          (GTK_CHECK_CAST((obj), bonobo_canvas_item_get_type (), BonoboCanvasItem))
#define BONOBO_CANVAS_ITEM_CLASS(k)      (GTK_CHECK_CLASS_CAST ((k), bonobo_canvas_item_get_type (), BonoboCanvasItemClass))
#define BONOBO_IS_CANVAS_ITEM(o)         (GTK_CHECK_TYPE((o), bonobo_canvas_item_get_type ()))

typedef struct _BonoboCanvasItemPrivate BonoboCanvasItemPrivate;

typedef struct {
	GnomeCanvasItem         canvas_item;
	BonoboEmbeddable        *embeddable;
	BonoboCanvasItemPrivate *priv;
} BonoboCanvasItem;

typedef struct {
	GnomeCanvasItemClass parent_class;
} BonoboCanvasItemClass;

GtkType          bonobo_canvas_item_get_type    (void);

void		 bonobo_canvas_item_set_bounds (BonoboCanvasItem *item, double x1, double y1, double x2, double y2);

#endif /* _BONOBO_CANVAS_ITEM_H_ */
