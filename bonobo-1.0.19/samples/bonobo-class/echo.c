/*
 * echo.c: Implements a Bonobo Echo server
 *
 * Author:
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * This file is here to show what are the basic steps
 * neccessary to create a Bonobo Component.
 */
#include <config.h>
#include <bonobo.h>

/*
 * This pulls the CORBA definitions for the Demo::Echo server
 */
#include "Bonobo_Sample_Echo.h"

/*
 * This pulls the definition for the BonoboObject (Gtk Type)
 */
#include "echo.h"

/*
 * Our parent Gtk object type
 */ 
#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

/*
 * A pointer to our parent object class
 */
static GtkObjectClass *echo_parent_class;

/*
 * Implemented GtkObject::destroy
 */
static void
echo_object_destroy (GtkObject *object)
{
	Echo *echo = ECHO (object);

	g_free (echo->instance_data);
	
	echo_parent_class->destroy (object);
}

/*
 * CORBA Demo::Echo::echo method implementation
 */
static void
impl_demo_echo_echo (PortableServer_Servant  servant,
		     const CORBA_char       *string,
		     CORBA_Environment      *ev)
{
	Echo *echo = ECHO (bonobo_object_from_servant (servant));
									 
	printf ("Echo message received: %s (echo instance data: %s)\n", string,
		echo->instance_data);
}

static void
echo_class_init (EchoClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_Sample_Echo__epv *epv = &klass->epv;

	echo_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = echo_object_destroy;

	epv->echo = impl_demo_echo_echo;
}

static void
echo_init (Echo *echo)
{
	static int i = 0;

	echo->instance_data = g_strdup_printf ("Hello %d!", i++);
}

GtkType
echo_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"Echo",
			sizeof (Echo),
			sizeof (EchoClass),
			(GtkClassInitFunc) echo_class_init,
			(GtkObjectInitFunc) echo_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};
		/*
		 *   Here we use bonobo_x_type_unique instead of
		 * gtk_type_unique, this auto-generates a load of
		 * CORBA structures for us. All derived types must
		 * use bonobo_x_type_unique.
		 */
		type = bonobo_x_type_unique (
			PARENT_TYPE,
			POA_Bonobo_Sample_Echo__init, NULL,
			GTK_STRUCT_OFFSET (EchoClass, epv),
			&info);
	}

	return type;
}

Echo *
echo_new (void)
{
	return gtk_type_new (echo_get_type ());
}
