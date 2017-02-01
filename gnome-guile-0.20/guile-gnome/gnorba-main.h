#include <libguile.h>
#include <guile-gtk.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>

void *
gnorba_corba_orb_copy (void *orb);

void
gnorba_corba_orb_free (void *orb);

void *
gnorba_corba_env_copy (void *env);

void
gnorba_corba_env_free (void *env);

CORBA_Environment *
gnorba_CORBA_exception_init (void);

void
gnorba_CORBA_exception_finish (CORBA_Environment *c_ev, SCM p_ev);

void
gnorba_CORBA_exception_throw (CORBA_Environment *c_ev, SCM p_ev);

void
gnome_gnorba_init_guile_support (void);
