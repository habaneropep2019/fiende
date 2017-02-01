/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-canvas-component.h: implements the CORBA interface for
 * the Bonobo::Canvas:Item interface used in Bonobo::Views.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_CANVAS_COMPONENT_H_
#define _BONOBO_CANVAS_COMPONENT_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_CANVAS_COMPONENT_TYPE        (bonobo_canvas_component_get_type ())
#define BONOBO_CANVAS_COMPONENT(o)          (GTK_CHECK_CAST ((o), BONOBO_CANVAS_COMPONENT_TYPE, BonoboCanvasComponent))
#define BONOBO_CANVAS_COMPONENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CANVAS_COMPONENT__TYPE, BonoboCanvasComponentClass))
#define BONOBO_IS_CANVAS_COMPONENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_CANVAS_COMPONENT_TYPE))
#define BONOBO_IS_CANVAS_COMPONENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CANVAS_COMPONENT_TYPE))

typedef struct _BonoboCanvasComponentPrivate BonoboCanvasComponentPrivate;

typedef struct {
	BonoboXObject base;
	BonoboCanvasComponentPrivate *priv;
} BonoboCanvasComponent;

typedef struct {
	BonoboXObjectClass parent_class;
	
	POA_Bonobo_Canvas_Component__epv epv;

	/* Signals */
	void (*set_bounds) (BonoboCanvasComponent *component,
			    Bonobo_Canvas_DRect   *bbox,
			    CORBA_Environment     *ev);
	
	gboolean (*event)  (BonoboCanvasComponent *component,
			    GdkEvent              *event);
} BonoboCanvasComponentClass;

GtkType                 bonobo_canvas_component_get_type         (void);
BonoboCanvasComponent  *bonobo_canvas_component_construct        (BonoboCanvasComponent       *comp,
								  GnomeCanvasItem             *item);
BonoboCanvasComponent  *bonobo_canvas_component_new              (GnomeCanvasItem             *item);
GnomeCanvasItem        *bonobo_canvas_component_get_item         (BonoboCanvasComponent       *comp);
void		        bonobo_canvas_component_grab		 (BonoboCanvasComponent       *comp, guint mask, GdkCursor *cursor, guint32 time);
void		        bonobo_canvas_component_ungrab		 (BonoboCanvasComponent       *comp, guint32 time);
Bonobo_UIContainer      bonobo_canvas_component_get_ui_container (BonoboCanvasComponent       *comp);
					  
/* This is a helper function for creating a canvas with the root item replaced
 * by a proxy to the client side proxy.
 */
GnomeCanvas *bonobo_canvas_new (gboolean                     is_aa,
				Bonobo_Canvas_ComponentProxy proxy);

END_GNOME_DECLS

#endif /* */
