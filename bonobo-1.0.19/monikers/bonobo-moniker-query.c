/*
 * gnome-moniker-query.c: Sample query-activation based Moniker implementation
 *
 * This is the Oaf query based Moniker implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 */
#include <config.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include <liboaf/liboaf.h>

#include "bonobo-moniker-query.h"

#define PREFIX_LEN (sizeof ("query:") - 1)

static BonoboMonikerClass *bonobo_moniker_query_parent_class;

static Bonobo_Moniker 
query_parse_display_name (BonoboMoniker     *moniker,
			  Bonobo_Moniker     parent,
			  const CORBA_char  *name,
			  CORBA_Environment *ev)
{
	BonoboMonikerQuery *m_query = BONOBO_MONIKER_QUERY (moniker);
	int      i, brackets;
	gboolean in_string;

	g_return_val_if_fail (m_query != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (strlen (name) >= PREFIX_LEN, CORBA_OBJECT_NIL);

	bonobo_moniker_set_parent (moniker, parent, ev);

	brackets = 0;
	in_string = FALSE;
	for (i = PREFIX_LEN; name [i]; i++) {
		switch (name [i]) {
		case '(':
			if (!in_string)
				brackets++;
			break;
		case ')':
			if (!in_string)
				brackets--;
			break;
		case '\'':
			if (name [i - 1] != '\\')
				in_string = !in_string;
			break;
		}
		if (brackets == 0) {
			i++;
			break;
		}
	}
	
	if (in_string || brackets != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InvalidSyntax, NULL);
		return CORBA_OBJECT_NIL;
	}
	
	bonobo_moniker_set_name (moniker, name, i);

	return bonobo_moniker_util_new_from_name_full (BONOBO_OBJREF (m_query),
						       &name [i], ev);
}

static Bonobo_Unknown
query_resolve (BonoboMoniker               *moniker,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char            *requested_interface,
	      CORBA_Environment           *ev)
{
	Bonobo_Moniker       parent;
	Bonobo_Unknown       object;
	char                *query;
	
	parent = bonobo_moniker_get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	if (parent != CORBA_OBJECT_NIL) {
		bonobo_object_release_unref (parent, ev);

		g_warning ("wierd; queryied moniker with a parent; strange");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	query = g_strdup_printf ("%s AND repo_ids.has ('%s')",
				 bonobo_moniker_get_name (moniker),
				 requested_interface);

	object = oaf_activate (query, NULL, 0, NULL, ev);

	g_free (query);

	return bonobo_moniker_util_qi_return (object, requested_interface, ev);
}

static void
bonobo_moniker_query_class_init (BonoboMonikerQueryClass *klass)
{
	BonoboMonikerClass *mclass = (BonoboMonikerClass *) klass;
	
	bonobo_moniker_query_parent_class = gtk_type_class (
		bonobo_moniker_get_type ());

	mclass->parse_display_name = query_parse_display_name;
	mclass->resolve            = query_resolve;
}

/**
 * bonobo_moniker_query_get_type:
 *
 * Returns the GtkType for the BonoboMonikerQuery class.
 */
GtkType
bonobo_moniker_query_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMonikerQuery",
			sizeof (BonoboMonikerQuery),
			sizeof (BonoboMonikerQueryClass),
			(GtkClassInitFunc) bonobo_moniker_query_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_moniker_get_type (), &info);
	}

	return type;
}

BonoboMoniker *
bonobo_moniker_query_new (void)
{
	return bonobo_moniker_construct (
		gtk_type_new (bonobo_moniker_query_get_type ()), "query:(");
}
