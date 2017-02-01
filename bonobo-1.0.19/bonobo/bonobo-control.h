/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo control object
 *
 * Author:
 *   Nat Friedman (nat@helixcode.com)
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_CONTROL_H_
#define _BONOBO_CONTROL_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-control-frame.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-property-bag-client.h>
#include <bonobo/bonobo-ui-component.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_CONTROL_TYPE        (bonobo_control_get_type ())
#define BONOBO_CONTROL(o)          (GTK_CHECK_CAST ((o), BONOBO_CONTROL_TYPE, BonoboControl))
#define BONOBO_CONTROL_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONTROL_TYPE, BonoboControlClass))
#define BONOBO_IS_CONTROL(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONTROL_TYPE))
#define BONOBO_IS_CONTROL_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONTROL_TYPE))

typedef struct _BonoboControlPrivate BonoboControlPrivate;

typedef struct {
	BonoboXObject base;

	BonoboControlPrivate *priv;
} BonoboControl;

typedef struct {
	BonoboXObjectClass      parent_class;

	POA_Bonobo_Control__epv epv;

	/* Signals. */
	void (*set_frame)      (BonoboControl *control);
	void (*activate)       (BonoboControl *control, gboolean state);
} BonoboControlClass;

/* The main API */
BonoboControl              *bonobo_control_new                     (GtkWidget     *widget);
GtkWidget                  *bonobo_control_get_widget              (BonoboControl *control);
void                        bonobo_control_set_automerge           (BonoboControl *control,
								    gboolean       automerge);
gboolean                    bonobo_control_get_automerge           (BonoboControl *control);

void                        bonobo_control_set_property            (BonoboControl       *control,
								    const char          *first_prop,
								    ...);
void                        bonobo_control_get_property            (BonoboControl       *control,
								    const char          *first_prop,
								    ...);

/* "Internal" stuff */
GtkType                     bonobo_control_get_type                (void);
BonoboControl              *bonobo_control_construct               (BonoboControl       *control,
								    GtkWidget           *widget);
BonoboUIComponent          *bonobo_control_get_ui_component        (BonoboControl       *control);
void                        bonobo_control_set_ui_component        (BonoboControl       *control,
								    BonoboUIComponent   *component);
Bonobo_UIContainer          bonobo_control_get_remote_ui_container (BonoboControl       *control);
void                        bonobo_control_set_control_frame       (BonoboControl       *control,
								    Bonobo_ControlFrame  control_frame);
Bonobo_ControlFrame         bonobo_control_get_control_frame       (BonoboControl       *control);
void                        bonobo_control_set_properties          (BonoboControl       *control,
								    BonoboPropertyBag   *pb);
BonoboPropertyBag          *bonobo_control_get_properties          (BonoboControl       *control);
Bonobo_PropertyBag          bonobo_control_get_ambient_properties  (BonoboControl       *control,
								    CORBA_Environment   *ev);
void                        bonobo_control_activate_notify         (BonoboControl       *control,
								    gboolean             activated);
Bonobo_Control_windowId     bonobo_control_windowid_from_x11       (guint32              x11_id);


END_GNOME_DECLS

#endif /* _BONOBO_CONTROL_H_ */
