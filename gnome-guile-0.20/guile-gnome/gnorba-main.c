#include <libguile.h>
#include <guile-gtk.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>

#include <gnorba-main.h>

static SCM scm_gnorba_system_exception = SCM_UNDEFINED;
static SCM scm_gnorba_user_exception = SCM_UNDEFINED;

void *
gnorba_corba_orb_copy (void *orb)
{
#ifdef GNORBA_DEBUG
    fprintf (stderr, "gnorba_CORBA_ORB_copy (%p)\n", orb);
#endif
    ORBIT_ROOT_OBJECT_REF (orb);
    return orb;
}

void
gnorba_corba_orb_free (void *orb)
{
#ifdef GNORBA_DEBUG
    fprintf (stderr, "gnorba_CORBA_ORB_free (%p)\n", orb);
#endif
    ORBIT_ROOT_OBJECT_UNREF (orb);
}

void *
gnorba_corba_env_copy (void *env)
{
#ifdef GNORBA_DEBUG
    fprintf (stderr, "gnorba_CORBA_Environment_copy (%p)\n", env);
#endif
    return env;
}

void
gnorba_corba_env_free (void *env)
{
#ifdef GNORBA_DEBUG
    fprintf (stderr, "gnorba_CORBA_Environment_free (%p)\n", env);
#endif
}

CORBA_Environment *
gnorba_CORBA_exception_init (void)
{
    CORBA_Environment *evp = g_new0 (CORBA_Environment, 1);

    CORBA_exception_init (evp);
    return evp;
}

void
gnorba_CORBA_exception_finish (CORBA_Environment *c_ev, SCM p_ev)
{
    if (c_ev && !SCM_NIMP (p_ev)) {
	gnorba_CORBA_exception_throw (c_ev, SCM_UNDEFINED);
	CORBA_exception_free (c_ev);
    }
}

void
gnorba_CORBA_exception_throw (CORBA_Environment *c_ev, SCM p_ev)
{
    switch (c_ev->_major) {
    case CORBA_SYSTEM_EXCEPTION:
	scm_throw (scm_gnorba_system_exception,
		   SCM_LIST2 (scm_makfrom0str (CORBA_exception_id (c_ev)),
			      p_ev));
	break;
    case CORBA_USER_EXCEPTION:
	scm_throw (scm_gnorba_system_exception,
		   SCM_LIST2 (scm_makfrom0str (CORBA_exception_id (c_ev)),
			      p_ev));
	break;
    }
}

void
gnome_gnorba_init_guile_support ()
{
    scm_gnorba_system_exception = SCM_CAR (scm_protect_object (scm_intern0 ("corba-system-exception")));
    scm_gnorba_user_exception = SCM_CAR (scm_protect_object (scm_intern0 ("corba-user-exception")));
}
