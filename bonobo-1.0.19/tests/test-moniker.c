#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>

#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <orb/orbit.h>
#include <bonobo.h>

static void
check_string (const char *prefix, const char *escaped, const char *unescaped)
{
	BonoboMoniker *moniker;
	const char    *const_str;
	char          *str;
	char          *s, *name;

	moniker = bonobo_moniker_construct (
		gtk_type_new (bonobo_moniker_get_type ()), prefix);
	
	name = g_strconcat (prefix, escaped, NULL);
	bonobo_moniker_set_name (moniker, name, strlen (name));

	const_str = bonobo_moniker_get_name (moniker);
	fprintf (stderr, "'%s' == '%s'\n", unescaped, const_str);
	g_assert (!strcmp (const_str, unescaped));


	s = g_strconcat (prefix, escaped, NULL);
	str = bonobo_moniker_get_name_escaped (moniker);
	fprintf (stderr, "'%s' == '%s'\n", str, s);
	g_assert (!strcmp (str, s));
	g_free (str);
	g_free (s);

	g_assert (bonobo_moniker_client_equal (
		BONOBO_OBJREF (moniker), name, NULL));

	bonobo_object_unref (BONOBO_OBJECT (moniker));

	g_free (name);
}

int
main (int argc, char *argv [])
{
	CORBA_Environment real_ev, *ev;
	CORBA_ORB    orb;

	free (malloc (8));

	ev = &real_ev;
	CORBA_exception_init (ev);

	gtk_type_init ();

	orb = oaf_init (argc, argv);
	
	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Can not bonobo_init");

	check_string ("a:", "\\\\", "\\");

	check_string ("a:", "\\#", "#");

	check_string ("prefix:", "\\!", "!");

	check_string ("a:",
		      "1\\!\\#\\!\\!\\#\\\\",
		      "1!#!!#\\");

	return 0;
}
