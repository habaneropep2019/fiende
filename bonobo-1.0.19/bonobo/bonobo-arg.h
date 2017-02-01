/*
 * bonobo-arg.h Bonobo argument support:
 *
 *  A thin wrapper of CORBA_any's with macros
 * to assist in handling values safely.
 *
 * Author:
 *    Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#ifndef __BONOBO_ARG_H__
#define __BONOBO_ARG_H__

#include <bonobo/Bonobo.h>
#include <libgnome/gnome-defs.h>
#include <gtk/gtktypeutils.h>

BEGIN_GNOME_DECLS

typedef CORBA_any      BonoboArg;
typedef CORBA_TypeCode BonoboArgType;

#define BONOBO_ARG_NULL    TC_null
#define BONOBO_ARG_BOOLEAN TC_boolean
#define BONOBO_ARG_INT     TC_long
#define BONOBO_ARG_LONG    TC_long
#define BONOBO_ARG_FLOAT   TC_float
#define BONOBO_ARG_DOUBLE  TC_double
#define BONOBO_ARG_STRING  TC_string

#ifdef __GNUC__
#	define BONOBO_ARG_GET_GENERAL(a,c,t,e)   (g_assert (bonobo_arg_type_is_equal ((a)->_type, c, e)),\
					          *((t *)(a->_value)))
#	define BONOBO_ARG_SET_GENERAL(a,v,c,t,e) (g_assert (bonobo_arg_type_is_equal ((a)->_type, c, e)),\
					          *((t *)(a->_value)) = (t)(v))
#else
#	define BONOBO_ARG_GET_GENERAL(a,c,t,e)   (*((t *)(a->_value)))
#	define BONOBO_ARG_SET_GENERAL(a,v,c,t,e) (*((t *)(a->_value)) = (t)(v))
#endif

#define BONOBO_ARG_GET_BOOLEAN(a)   (BONOBO_ARG_GET_GENERAL (a, TC_boolean, CORBA_boolean, NULL))
#define BONOBO_ARG_SET_BOOLEAN(a,v) (BONOBO_ARG_SET_GENERAL (a, v, TC_boolean, CORBA_boolean, NULL))

#define BONOBO_ARG_GET_INT(a)       (BONOBO_ARG_GET_GENERAL (a, TC_long, CORBA_long, NULL))
#define BONOBO_ARG_SET_INT(a,v)     (BONOBO_ARG_SET_GENERAL (a, v, TC_long, CORBA_long, NULL))
#define BONOBO_ARG_GET_LONG(a)      (BONOBO_ARG_GET_GENERAL (a, TC_long, CORBA_long, NULL))
#define BONOBO_ARG_SET_LONG(a,v)    (BONOBO_ARG_SET_GENERAL (a, v, TC_long, CORBA_long, NULL))

#define BONOBO_ARG_GET_FLOAT(a)     (BONOBO_ARG_GET_GENERAL (a, TC_float, CORBA_float, NULL))
#define BONOBO_ARG_SET_FLOAT(a,v)   (BONOBO_ARG_SET_GENERAL (a, v, TC_float, CORBA_float, NULL))

#define BONOBO_ARG_GET_DOUBLE(a)    (BONOBO_ARG_GET_GENERAL (a, TC_double, CORBA_double, NULL))
#define BONOBO_ARG_SET_DOUBLE(a,v)  (BONOBO_ARG_SET_GENERAL (a, v, TC_double, CORBA_double, NULL))


#ifdef __GNUC__
#define BONOBO_ARG_GET_STRING(a)    (g_assert ((a)->_type->kind == CORBA_tk_string),\
				     *((CORBA_char **)(a->_value)))
#define BONOBO_ARG_SET_STRING(a,v)  (g_assert ((a)->_type->kind == CORBA_tk_string), CORBA_free (*(char **)a->_value),\
				     *((CORBA_char **)(a->_value)) = CORBA_string_dup ((v)?(v):""))
#else
#define BONOBO_ARG_GET_STRING(a)    (*((CORBA_char **)(a->_value)))
#define BONOBO_ARG_SET_STRING(a,v)  (CORBA_free (*(char **)a->_value),\
				     *((CORBA_char **)(a->_value)) = CORBA_string_dup ((v)?(v):""))
#endif

BonoboArg    *bonobo_arg_new           (BonoboArgType  t);

void          bonobo_arg_release       (BonoboArg     *arg);

BonoboArg    *bonobo_arg_copy          (const BonoboArg *arg);

void          bonobo_arg_from_gtk      (BonoboArg    *a, 
					const GtkArg *arg);
BonoboArgType bonobo_arg_type_from_gtk (GtkType t);

void          bonobo_arg_to_gtk        (GtkArg          *a, 
					const BonoboArg *arg);

GtkType       bonobo_arg_type_to_gtk   (BonoboArgType id);

gboolean      bonobo_arg_is_equal      (BonoboArg         *a, 
					BonoboArg         *b, 
					CORBA_Environment *opt_ev);

gboolean      bonobo_arg_type_is_equal (BonoboArgType      a, 
					BonoboArgType      b,
					CORBA_Environment *opt_ev);

END_GNOME_DECLS

#endif /* ! __BONOBO_ARG_H__ */
