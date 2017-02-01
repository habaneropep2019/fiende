/*
 * Some XML-based BonoboPropertyBag persistence helpers.
 *
 * Authors:
 *   Michael Meeks (michael@ximian.com)
 *   Nat Friedman  (nat@ximian.com)
 *
 * Copyright 1999, 2000 Ximian, Inc.
 */
#ifndef __BONOBO_PROPERTY_BAG_XML_H__
#define __BONOBO_PROPERTY_BAG_XML_H__

#include <bonobo/bonobo-ui-node.h>
#include <bonobo/bonobo-property-bag.h>

BEGIN_GNOME_DECLS


BonoboUINode *bonobo_property_bag_xml_encode_any (BonoboUINode      *opt_parent,
						  const CORBA_any         *any,
						  CORBA_Environment *ev);

CORBA_any    *bonobo_property_bag_xml_decode_any (BonoboUINode      *node,
						  CORBA_Environment *ev);

END_GNOME_DECLS

#endif /* ! __BONOBO_PROPERTY_BAG_XML_H__ */
