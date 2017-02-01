/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-view-frame.h: view frame object.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_VIEW_FRAME_H_
#define _BONOBO_VIEW_FRAME_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-control-frame.h>
#include <bonobo/bonobo-ui-container.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_VIEW_FRAME_TYPE        (bonobo_view_frame_get_type ())
#define BONOBO_VIEW_FRAME(o)          (GTK_CHECK_CAST ((o), BONOBO_VIEW_FRAME_TYPE, BonoboViewFrame))
#define BONOBO_VIEW_FRAME_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_VIEW_FRAME_TYPE, BonoboViewFrameClass))
#define BONOBO_IS_VIEW_FRAME(o)       (GTK_CHECK_TYPE ((o), BONOBO_VIEW_FRAME_TYPE))
#define BONOBO_IS_VIEW_FRAME_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_VIEW_FRAME_TYPE))

typedef struct _BonoboViewFrame        BonoboViewFrame;
typedef struct _BonoboViewFrameClass   BonoboViewFrameClass;
typedef struct _BonoboViewFramePrivate BonoboViewFramePrivate;

#include <bonobo/bonobo-client-site.h>

struct _BonoboViewFrame {
	BonoboControlFrame	 base;
	BonoboViewFramePrivate	*priv;
};

struct _BonoboViewFrameClass {
	BonoboControlFrameClass parent_class;

	POA_Bonobo_ViewFrame__epv epv;

	/* Signals. */
	void (*user_activate)       (BonoboViewFrame *view_frame);
	void (*user_context)        (BonoboViewFrame *view_frame);
};

GtkType                    bonobo_view_frame_get_type         (void);
BonoboViewFrame           *bonobo_view_frame_construct        (BonoboViewFrame   *view_frame,
							       BonoboClientSite  *client_site,
							       Bonobo_UIContainer uih);
BonoboViewFrame           *bonobo_view_frame_new              (BonoboClientSite *client_site,
							       Bonobo_Unknown    uih);
void                       bonobo_view_frame_bind_to_view     (BonoboViewFrame  *view_frame,
							       Bonobo_View       view);
Bonobo_View                bonobo_view_frame_get_view         (BonoboViewFrame  *view_frame);
BonoboClientSite          *bonobo_view_frame_get_client_site  (BonoboViewFrame  *view_frame);
GtkWidget                 *bonobo_view_frame_get_wrapper      (BonoboViewFrame  *view_frame);
void                       bonobo_view_frame_set_covered      (BonoboViewFrame  *view_frame,
							       gboolean          covered);

Bonobo_UIContainer         bonobo_view_frame_get_ui_container (BonoboViewFrame *view_frame);

/*
 * A BonoboViewFrame acts as a proxy for the remote BonoboView object to
 * which it is bound.  These functions act as wrappers which a
 * container can use to communicate with the BonoboView associated with
 * a given BonoboViewFrame.
 */
void                       bonobo_view_frame_view_activate    (BonoboViewFrame  *view_frame);
void                       bonobo_view_frame_view_deactivate  (BonoboViewFrame  *view_frame);
void                       bonobo_view_frame_set_zoom_factor  (BonoboViewFrame  *view_frame,
							       double            zoom);

END_GNOME_DECLS

#endif /* _BONOBO_VIEW_FRAME_H_ */
