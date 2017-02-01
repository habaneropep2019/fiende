#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-property-bag-xml.h>

int
main (int argc, char *argv [])
{
	BonoboXObject *xobject;

	CORBA_ORB    orb;

	free (malloc (8));

	gtk_type_init ();

	orb = oaf_init (argc, argv);
	
	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Can not bonobo_init");

	xobject = BONOBO_X_OBJECT (gtk_type_new (bonobo_moniker_get_type ()));
	
	g_assert (bonobo_x_object (xobject) == xobject);
	g_assert (bonobo_x_object (&xobject->servant) == xobject);

	bonobo_object_unref (BONOBO_OBJECT (xobject));

	return 0;
}
