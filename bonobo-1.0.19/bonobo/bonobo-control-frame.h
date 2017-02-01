/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo control frame object.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_CONTROL_FRAME_H_
#define _BONOBO_CONTROL_FRAME_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-wrapper.h>
#include <bonobo/bonobo-property-bag-client.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_CONTROL_FRAME_TYPE        (bonobo_control_frame_get_type ())
#define BONOBO_CONTROL_FRAME(o)          (GTK_CHECK_CAST ((o), BONOBO_CONTROL_FRAME_TYPE, BonoboControlFrame))
#define BONOBO_CONTROL_FRAME_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONTROL_FRAME_TYPE, BonoboControlFrameClass))
#define BONOBO_IS_CONTROL_FRAME(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONTROL_FRAME_TYPE))
#define BONOBO_IS_CONTROL_FRAME_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONTROL_FRAME_TYPE))

typedef struct _BonoboControlFramePrivate BonoboControlFramePrivate;

typedef struct {
	BonoboXObject base;
	BonoboControlFramePrivate *priv;
} BonoboControlFrame;

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_ControlFrame__epv epv;

	/* Signals. */
	void (*activated)           (BonoboControlFrame *control_frame, gboolean state);
	void (*activate_uri)        (BonoboControlFrame *control_frame, const char *uri, gboolean relative);

} BonoboControlFrameClass;


BonoboControlFrame           *bonobo_control_frame_new                       (Bonobo_UIContainer   ui_container);

GtkWidget                    *bonobo_control_frame_get_widget                (BonoboControlFrame  *frame);

/* This is only allowed when the Control is deactivated */
void                          bonobo_control_frame_set_ui_container          (BonoboControlFrame  *control_frame,
									      Bonobo_UIContainer   uic);

/* Activating remote controls */
void                          bonobo_control_frame_control_activate          (BonoboControlFrame  *control_frame);
void                          bonobo_control_frame_control_deactivate        (BonoboControlFrame  *control_frame);
void                          bonobo_control_frame_set_autoactivate          (BonoboControlFrame  *control_frame,
									      gboolean             autoactivate);
gboolean                      bonobo_control_frame_get_autoactivate          (BonoboControlFrame  *control_frame);

/* Remote properties */
Bonobo_PropertyBag            bonobo_control_frame_get_control_property_bag  (BonoboControlFrame  *control_frame,
									      CORBA_Environment   *ev);

/* Ambient properties */
void                          bonobo_control_frame_set_propbag               (BonoboControlFrame  *control_frame,
									      BonoboPropertyBag   *propbag);
BonoboPropertyBag            *bonobo_control_frame_get_propbag               (BonoboControlFrame  *control_frame);

/* Widget state proxying */
void                          bonobo_control_frame_control_set_state         (BonoboControlFrame  *control_frame,
									      GtkStateType         state);
void                          bonobo_control_frame_set_autostate             (BonoboControlFrame  *control_frame,
									      gboolean             autostate);
gboolean                      bonobo_control_frame_get_autostate             (BonoboControlFrame  *control_frame);


/* Connecting to the remote control */
void                          bonobo_control_frame_bind_to_control           (BonoboControlFrame  *control_frame,
									      Bonobo_Control       control);

Bonobo_Control                bonobo_control_frame_get_control               (BonoboControlFrame  *control_frame);

Bonobo_UIContainer            bonobo_control_frame_get_ui_container          (BonoboControlFrame  *control_frame);


/* Object construction stuff */
GtkType                       bonobo_control_frame_get_type                  (void);
BonoboControlFrame           *bonobo_control_frame_construct                 (BonoboControlFrame  *control_frame,
									      Bonobo_UIContainer   ui_container);

/*
 * A BonoboControlFrame acts as a proxy for the remote BonoboControl object to
 * which it is bound.  These functions act as wrappers which a
 * container can use to communicate with the BonoboControl associated with
 * a given BonoboControlFrame.
 */
void  bonobo_control_frame_size_request (BonoboControlFrame *control_frame,
					 int                *desired_width,
					 int                *desired_height);

/* You almost certainly don't want these methods */
void  bonobo_control_frame_sync_realize   (BonoboControlFrame *frame);
void  bonobo_control_frame_sync_unrealize (BonoboControlFrame *frame);

/* Or this.  It exists just so that BonoboSocket can use it. */
gboolean bonobo_control_frame_focus_child (BonoboControlFrame *frame, GtkDirectionType direction);
    
END_GNOME_DECLS

#endif /* _BONOBO_CONTROL_FRAME_H_ */
