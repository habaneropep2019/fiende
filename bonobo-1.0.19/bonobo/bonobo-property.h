/**
 * Bonobo property object implementation.
 *
 * Author:
 *    Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#ifndef __BONOBO_PROPERTY_H__
#define __BONOBO_PROPERTY_H__

/*
 * Every function and data structure in this file is private and
 * should never be accessed directly by a PropertyBag user, unless
 * he is implementing his own persistence mechanism.  Otherwise,
 * no peeking.
 */

#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-transient.h>

PortableServer_Servant bonobo_property_servant_new     (PortableServer_POA     poa,
							BonoboTransient        *bt,
							char                   *name,
							void                   *callback_data);
void		       bonobo_property_servant_destroy (PortableServer_Servant  servant,
							void                   *callback_data);

typedef struct {
	char			*name;
	int                      idx;
	BonoboArgType            type;
	BonoboArg               *default_value;
	char			*docstring;
	BonoboPropertyFlags	 flags;

	BonoboPropertyGetFn      get_prop;
	BonoboPropertySetFn      set_prop;
	gpointer                 user_data;
	GSList                  *listeners;
} BonoboProperty;

#endif /* ! __BONOBO_PROPERTY_H__ */
