/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-property-control.h: Property control implementation.
 *
 * Author:
 *      Iain Holmes  <iain@helixcode.com>
 *
 * Copyright 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_PROPERTY_CONTROL_H_
#define _BONOBO_PROPERTY_CONTROL_H_

#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-event-source.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboPropertyControl        BonoboPropertyControl;
typedef struct _BonoboPropertyControlPrivate BonoboPropertyControlPrivate;

#define BONOBO_PROPERTY_CONTROL_CHANGED "Bonobo::PropertyControl_changed"

#define BONOBO_PROPERTY_CONTROL_TYPE        (bonobo_property_control_get_type ())
#define BONOBO_PROPERTY_CONTROL(o)          (GTK_CHECK_CAST ((o), BONOBO_PROPERTY_CONTROL_TYPE, BonoboPropertyControl))
#define BONOBO_PROPERTY_CONTROL_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_CONTROL_TYPE, BonoboPropertyControlClass))
#define BONOBO_IS_PROPERTY_CONTROL(o)       (GTK_CHECK_TYPE ((o), BONOBO_PROPERTY_CONTROL_TYPE))
#define BONOBO_IS_PROPERTY_CONTROL_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_CONTROL_TYPE))

typedef BonoboControl *(* BonoboPropertyControlGetControlFn) (BonoboPropertyControl *control,
							      int page_number,
							      void *closure);

struct _BonoboPropertyControl {
        BonoboXObject		object;

	BonoboPropertyControlPrivate *priv;
};

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_PropertyControl__epv epv;

	void (* action) (BonoboPropertyControl *property_control, 
			 Bonobo_PropertyControl_Action action);
} BonoboPropertyControlClass;

GtkType bonobo_property_control_get_type (void);

BonoboPropertyControl *bonobo_property_control_construct (BonoboPropertyControl *property_control,
							  BonoboEventSource     *event_source,
							  BonoboPropertyControlGetControlFn get_fn,
							  int                    num_pages,
							  void                  *closure);
BonoboPropertyControl *bonobo_property_control_new_full  (BonoboPropertyControlGetControlFn get_fn,
							  int                    num_pages,
							  BonoboEventSource     *event_source,
							  void                  *closure);
BonoboPropertyControl *bonobo_property_control_new       (BonoboPropertyControlGetControlFn get_fn,
							  int                    num_pages,
							  void                  *closure);
void                   bonobo_property_control_changed   (BonoboPropertyControl *property_control,
							  CORBA_Environment     *opt_ev);
BonoboEventSource *bonobo_property_control_get_event_source (BonoboPropertyControl *property_control);

END_GNOME_DECLS

#endif /* _BONOBO_PROPERTY_CONTROL_H_ */

