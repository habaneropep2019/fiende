/*
 * bonobo-property-bag-client.c: C sugar for property bags.
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#ifndef __BONOBO_PROPERTY_BAG_CLIENT_H__
#define __BONOBO_PROPERTY_BAG_CLIENT_H__

#include <stdarg.h>
#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-object-client.h>

BEGIN_GNOME_DECLS

GList			*bonobo_property_bag_client_get_properties     (Bonobo_PropertyBag       pb,
									CORBA_Environment       *ev);
void                     bonobo_property_bag_client_free_properties    (GList *list);
GList			*bonobo_property_bag_client_get_property_names (Bonobo_PropertyBag       pb,
									CORBA_Environment       *ev);
Bonobo_Property		 bonobo_property_bag_client_get_property       (Bonobo_PropertyBag       pb,
									const char              *property_name,
									CORBA_Environment       *ev);
void			 bonobo_property_bag_client_persist	       (Bonobo_PropertyBag       pb,
									Bonobo_Stream            stream,
									CORBA_Environment       *ev);
void			 bonobo_property_bag_client_depersist	       (Bonobo_PropertyBag       pb,
									Bonobo_Stream            stream,
									CORBA_Environment       *ev);

GtkType			 bonobo_property_bag_client_get_type	       (void);

char                    *bonobo_property_bag_client_setv               (Bonobo_PropertyBag       pb,
									CORBA_Environment       *ev,
									const char              *first_arg,
									va_list                  var_args);
char                    *bonobo_property_bag_client_getv               (Bonobo_PropertyBag       pb,
									CORBA_Environment       *ev,
									const char              *first_arg,
									va_list                  var_args);

/*
 *
 * Property querying/manipulation routines.
 *
 * These are just provided as a convenience; you can also manipulate
 * the properties directly.
 *
 */

/* Querying the property type. */
CORBA_TypeCode           bonobo_property_bag_client_get_property_type    (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);

/* Querying property values. */
gboolean		 bonobo_property_bag_client_get_value_gboolean   (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
gint			 bonobo_property_bag_client_get_value_gint       (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
glong			 bonobo_property_bag_client_get_value_glong      (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
gfloat			 bonobo_property_bag_client_get_value_gfloat     (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
gdouble			 bonobo_property_bag_client_get_value_gdouble    (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
char			*bonobo_property_bag_client_get_value_string     (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
BonoboArg		*bonobo_property_bag_client_get_value_any        (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);

/* Querying property default values. */ 						  		       
gboolean		 bonobo_property_bag_client_get_default_gboolean (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
gint			 bonobo_property_bag_client_get_default_gint     (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
glong			 bonobo_property_bag_client_get_default_glong    (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
gfloat			 bonobo_property_bag_client_get_default_gfloat   (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
gdouble			 bonobo_property_bag_client_get_default_gdouble  (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
char			*bonobo_property_bag_client_get_default_string   (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
BonoboArg		*bonobo_property_bag_client_get_default_any      (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);

/* Setting property values. */
void			 bonobo_property_bag_client_set_value_gboolean   (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  gboolean                value,
									  CORBA_Environment       *ev);
void			 bonobo_property_bag_client_set_value_gint       (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  gint                     value,
									  CORBA_Environment       *ev);
void			 bonobo_property_bag_client_set_value_glong      (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  glong                    value,
									  CORBA_Environment       *ev);
void			 bonobo_property_bag_client_set_value_gfloat     (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  gfloat                   value,
									  CORBA_Environment       *ev);
void			 bonobo_property_bag_client_set_value_gdouble    (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  gdouble                  value,
									  CORBA_Environment       *ev);
void			 bonobo_property_bag_client_set_value_string     (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  const char              *value,
									  CORBA_Environment       *ev);
void			 bonobo_property_bag_client_set_value_any        (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  BonoboArg               *value,
									  CORBA_Environment       *ev);

/* Querying other fields and flags. */
char			*bonobo_property_bag_client_get_docstring        (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);
BonoboPropertyFlags	 bonobo_property_bag_client_get_flags	         (Bonobo_PropertyBag       pb,
									  const char              *propname,
									  CORBA_Environment       *ev);

END_GNOME_DECLS

#endif /* ! ___BONOBO_PROPERTY_BAG_CLIENT_H__ */
