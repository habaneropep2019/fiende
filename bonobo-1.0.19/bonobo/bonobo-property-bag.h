/*
 * bonobo-property-bag.h: property bag object implementation.
 *
 * Authors:
 *   Nat Friedman  (nat@helixcode.com)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef __BONOBO_PROPERTY_BAG_H__
#define __BONOBO_PROPERTY_BAG_H__

#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboPropertyBagPrivate BonoboPropertyBagPrivate;
typedef struct _BonoboPropertyBag        BonoboPropertyBag;

/* Keep this enumeration synced with the docs in bonobo-property.idl */
typedef enum {
	BONOBO_PROPERTY_UNSTORED        = 1,
	BONOBO_PROPERTY_READABLE        = 2,
	BONOBO_PROPERTY_WRITEABLE       = 4,
	BONOBO_PROPERTY_USE_DEFAULT_OPT = 8,
	BONOBO_PROPERTY_NO_LISTENING    = 16
} BonoboPropertyFlags;

#include <bonobo/bonobo-arg.h>

typedef void (*BonoboPropertyGetFn) (BonoboPropertyBag *bag,
				     BonoboArg         *arg,
				     guint              arg_id,
				     CORBA_Environment *ev,
				     gpointer           user_data);
typedef void (*BonoboPropertySetFn) (BonoboPropertyBag *bag,
				     const BonoboArg   *arg,
				     guint              arg_id,
				     CORBA_Environment *ev,
				     gpointer           user_data);

#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-event-source.h>

struct _BonoboPropertyBag {
	BonoboXObject             parent;
	BonoboPropertyBagPrivate *priv;
	BonoboEventSource        *es;
};

typedef struct {
	BonoboXObjectClass        parent;

	POA_Bonobo_PropertyBag__epv epv;
} BonoboPropertyBagClass;

#define BONOBO_PROPERTY_BAG_TYPE                (bonobo_property_bag_get_type ())
#define BONOBO_PROPERTY_BAG(o)		        (GTK_CHECK_CAST ((o), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBag))
#define BONOBO_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBagClass))
#define BONOBO_IS_PROPERTY_BAG(o)		(GTK_CHECK_TYPE ((o), BONOBO_PROPERTY_BAG_TYPE))
#define BONOBO_IS_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_BAG_TYPE))

GtkType		          bonobo_property_bag_get_type        (void);
BonoboPropertyBag	 *bonobo_property_bag_new	      (BonoboPropertyGetFn get_prop,
							       BonoboPropertySetFn set_prop,
							       gpointer            user_data);

BonoboPropertyBag	 *bonobo_property_bag_new_full	      (BonoboPropertyGetFn get_prop,
							       BonoboPropertySetFn set_prop,
							       BonoboEventSource  *event_source,
							       gpointer            user_data);

BonoboPropertyBag        *bonobo_property_bag_construct       (BonoboPropertyBag   *pb,
							       BonoboPropertyGetFn  get_prop,
							       BonoboPropertySetFn  set_prop,
							       BonoboEventSource   *event_source,
							       gpointer             user_data);

void                      bonobo_property_bag_add              (BonoboPropertyBag  *pb,
								const char         *name,
								int                 idx,
								BonoboArgType       type,
								BonoboArg          *default_value,
								const char         *docstring,
								BonoboPropertyFlags flags);

void                      bonobo_property_bag_add_full         (BonoboPropertyBag  *pb,
								const char         *name,
								int                 idx,
								BonoboArgType       type,
								BonoboArg          *default_value,
								const char         *docstring,
								BonoboPropertyFlags flags,
								BonoboPropertyGetFn get_prop,
								BonoboPropertySetFn set_prop,
								gpointer            user_data);

void                      bonobo_property_bag_add_gtk_args     (BonoboPropertyBag  *pb,
								GtkObject          *object);

BonoboArgType             bonobo_property_bag_get_property_type (BonoboPropertyBag *pb, 
								const char *name,
								CORBA_Environment *opt_ev);

/* Modifying properties. */		   		      
void		          bonobo_property_bag_set_value        (BonoboPropertyBag *pb,
								const char        *name,
								const BonoboArg   *value,
								CORBA_Environment *opt_ev);

BonoboArg                *bonobo_property_bag_get_value        (BonoboPropertyBag *pb, 
								const char *name, 
								CORBA_Environment *opt_ev);
BonoboArg                *bonobo_property_bag_get_default      (BonoboPropertyBag *pb, 
								const char *name, 
								CORBA_Environment *opt_ev);
const char	         *bonobo_property_bag_get_docstring    (BonoboPropertyBag *pb, 
								const char *name, 
								CORBA_Environment *opt_ev);
const BonoboPropertyFlags bonobo_property_bag_get_flags        (BonoboPropertyBag *pb,
								const char *name, 
								CORBA_Environment *opt_ev);

gboolean		  bonobo_property_bag_has_property     (BonoboPropertyBag *pb, 
								const char *name);
void                      bonobo_property_bag_notify_listeners (BonoboPropertyBag *pb,
								const char        *name,
								const BonoboArg   *new_value,
								CORBA_Environment *opt_ev);

/* A private function, only to be used by persistence implementations. */
GList                    *bonobo_property_bag_get_prop_list    (BonoboPropertyBag *pb);

END_GNOME_DECLS

#endif /* ! __BONOBO_PROPERTY_BAG_H__ */
