/* corba.c -- CORBA interface and clients for gnome-terminal
  
   mostly adapted from mc/gnome/gcorba.c and mc/gnome/gmc-client.c

   todo:

	* add some more Terminal methods, e.g:

		void input (string)		[zvt_term_feed]
		void output (string)		[zvt_term_writechild]
		string get_contents (...)	[zvt_term_get_buffer]
		void bell ()			[zvt_term_bell]
		void reset ()			[zvt_term_reset]

	* set $GNOME_TERMINAL_IOR in child processes to reference the
	containing Terminal (then give _all_ terminals servants, not
	just those created through a TerminalFactory)

	* extend/add-some-more Terminal constructors to take more
	attributes (colors, font, login/no-login, etc..)  */

#include <config.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include <gnome.h>
#include <libgnorba/gnorba.h>

#include "gnome-terminal.h"
#include "Terminal.h"

/* Do we have a remote TerminalFactory? */
int has_terminal_factory = FALSE;

/* The ORB for the whole program */
static CORBA_ORB orb = CORBA_OBJECT_NIL;

/* The POA */
static PortableServer_POA poa = CORBA_OBJECT_NIL;

/* Terminal servant */
typedef struct {
	POA_GNOME_Terminal_Terminal servant;
	GtkWidget *term;
} TerminalServant;

static PortableServer_ServantBase__epv terminal_base_epv;
static POA_GNOME_Terminal_Terminal__epv terminal_epv;
static POA_GNOME_Terminal_Terminal__vepv terminal_vepv;

/* Map from terminal widgets to TerminalServant's; the reverse mapping is
   handled through the TerminalServant structure */
static GHashTable *terminal_servants;

/* TerminalFactory servant */

typedef struct {
	POA_GNOME_Terminal_TerminalFactory servant;
} TerminalFactoryServant;

static PortableServer_ServantBase__epv terminal_factory_base_epv;
static POA_GNOME_GenericFactory__epv terminal_factory_generic_factory_epv;
static POA_GNOME_Terminal_TerminalFactory__epv terminal_factory_epv;
static POA_GNOME_Terminal_TerminalFactory__vepv terminal_factory_vepv;

static GNOME_Terminal_TerminalFactory terminal_factory_server = CORBA_OBJECT_NIL;

/* Terminal implementation */

/* XXX define Terminal method implementions here.. */

/* Fills the vepv structure for the terminal object */
static void
Terminal_class_init (void)
{
	static int inited = FALSE;

	if (inited)
		return;

	inited = TRUE;

	terminal_servants = g_hash_table_new ((GHashFunc) g_direct_hash,
					      (GCompareFunc) g_direct_equal);

	/* XXX fill in Terminal method vectors here.. */

	terminal_vepv._base_epv = &terminal_base_epv;
	terminal_vepv.GNOME_Terminal_Terminal_epv = &terminal_epv;
}

/* Destroys the servant for a terminal */
static void
terminal_destroy (GtkWidget *term, TerminalServant *ts, CORBA_Environment *ev)
{
	PortableServer_ObjectId *objid;

	objid = PortableServer_POA_servant_to_id (poa, ts, ev);
	PortableServer_POA_deactivate_object (poa, objid, ev);
	CORBA_free (objid);

	POA_GNOME_Terminal_Terminal__fini (ts, ev);
	g_hash_table_remove (terminal_servants, term);
	g_free (ts);
}

/* Called when a terminal with a CORBA servant is destroyed */
static void
terminal_destroyed (GtkObject *object, gpointer data)
{
	TerminalServant *ts;
	CORBA_Environment ev;

	ts = data;

	CORBA_exception_init (&ev);
	terminal_destroy (GTK_WIDGET (object), ts, &ev);
	CORBA_exception_free (&ev);
}

/* Returns a servant for a terminal, creating one if necessary */
static TerminalServant *
terminal_servant_from_terminal (GtkWidget *term, CORBA_Environment *ev)
{
	TerminalServant *ts;
	PortableServer_ObjectId *objid;

	ts = (terminal_servants
	      ? g_hash_table_lookup (terminal_servants, term) : 0);
	if (ts != 0)
		return ts;

	Terminal_class_init ();

	ts = g_new0 (TerminalServant, 1);
	ts->servant.vepv = &terminal_vepv;

	POA_GNOME_Terminal_Terminal__init ((PortableServer_Servant) ts, ev);
	objid = PortableServer_POA_activate_object (poa, ts, ev);
	CORBA_free (objid);

	ts->term = term;
	g_hash_table_insert (terminal_servants, term, (gpointer) ts);

	gtk_signal_connect (GTK_OBJECT (term), "destroy",
			    (GtkSignalFunc) terminal_destroyed, ts);

	return ts;
}

/* TerminalFactory implementation */

/* TerminalFactory::create_terminal method */
static GNOME_Terminal_Terminal
TerminalFactory_create_terminal (PortableServer_Servant servant,
				 const CORBA_char *geometry,
				 CORBA_Environment *ev)
{
	GtkWidget *term;
	TerminalServant *ts;

	term = new_terminal_for_client (geometry);
	if (term == NULL)
		return CORBA_OBJECT_NIL;
	
	ts = terminal_servant_from_terminal (term, ev);

	return PortableServer_POA_servant_to_reference (poa, ts, ev);
}

/* TerminalFactory GenericFactory::supports method */
static CORBA_boolean
TerminalFactory_supports (PortableServer_Servant servant,
			  const CORBA_char *obj_goad_id,
			  CORBA_Environment *ev)
{
	if (strcmp (obj_goad_id, "IDL:GNOME:Terminal:Terminal:1.0") == 0)
		return CORBA_TRUE;
	else
		return CORBA_FALSE;
}

/* TerminalFactory GenericFactory::create_object method */
static CORBA_Object
TerminalFactory_create_object (PortableServer_Servant servant,
			       const CORBA_char *goad_id,
			       const GNOME_stringlist *params,
			       CORBA_Environment *ev)
{
	if (strcmp (goad_id, "IDL:GNOME:Terminal:Terminal:1.0") != 0)
		return TerminalFactory_create_terminal (
			servant,
			params->_length != 0 ? params->_buffer[0] : 0,
			ev);
	else {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_GenericFactory_CannotActivate,
				     NULL);
		return CORBA_OBJECT_NIL;
	}
}

/* Fills the vepv structure for the terminal factory object */
static void
TerminalFactory_class_init (void)
{
	static int inited = FALSE;

	if (inited)
		return;

	inited = TRUE;

	terminal_factory_generic_factory_epv.supports = TerminalFactory_supports;
	terminal_factory_generic_factory_epv.create_object = TerminalFactory_create_object;

	terminal_factory_epv.create_terminal = TerminalFactory_create_terminal;

	terminal_factory_vepv._base_epv = &terminal_factory_base_epv;
	terminal_factory_vepv.GNOME_GenericFactory_epv = &terminal_factory_generic_factory_epv;
	terminal_factory_vepv.GNOME_Terminal_TerminalFactory_epv = &terminal_factory_epv;
}

/* Creates a reference for the terminal factory object */
static GNOME_Terminal_TerminalFactory
TerminalFactory_create (PortableServer_POA poa, CORBA_Environment *ev)
{
	TerminalFactoryServant *tfs;
	PortableServer_ObjectId *objid;

	TerminalFactory_class_init ();

	tfs = g_new0 (TerminalFactoryServant, 1);
	tfs->servant.vepv = &terminal_factory_vepv;

	POA_GNOME_Terminal_TerminalFactory__init ((PortableServer_Servant) tfs, ev);
	objid = PortableServer_POA_activate_object (poa, tfs, ev);
	CORBA_free (objid);

	return PortableServer_POA_servant_to_reference (poa, tfs, ev);
}

static void
TerminalFactory_destory (GNOME_Terminal_TerminalFactory factory,
			 PortableServer_POA poa, CORBA_Environment *ev)
{
	TerminalFactoryServant *tfs;
	PortableServer_ObjectId *objid;

	objid = PortableServer_POA_reference_to_id (poa, factory, ev);

	PortableServer_POA_deactivate_object (poa, objid, ev);
	CORBA_free (objid);

	tfs = PortableServer_POA_reference_to_servant (poa, factory, ev);
	POA_GNOME_Terminal_TerminalFactory__fini (tfs, ev);

	g_free (tfs);
}


/* Initialisation */

static CORBA_Object get_terminal_factory (void);

/* Creates and registers the CORBA servers.  Returns TRUE on success, FALSE
 * otherwise.
 */
static int
register_servers (void)
{
	CORBA_Environment ev;
	int retval;
	int v;

	retval = FALSE;
	CORBA_exception_init (&ev);

	/* Register the terminal factory and see if it was already there */

	terminal_factory_server = TerminalFactory_create (poa, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		goto out;

	v = goad_server_register (CORBA_OBJECT_NIL, terminal_factory_server,
				  "IDL:GNOME:Terminal:TerminalFactory:1.0",
				  "object", &ev);
	switch (v) {
	case 0:
		has_terminal_factory = FALSE;
		break;

	case -2: 
		/* possible race condition fix:
		   see if someone else registered between when we
		   last checked and when we tried to register
		*/
		TerminalFactory_destory (terminal_factory_server,
					 poa, &ev);

		terminal_factory_server = get_terminal_factory ();

		if (terminal_factory_server != CORBA_OBJECT_NIL)
			has_terminal_factory = TRUE;
		else
			goto out;
		break;

	default:
		goto out;
	}

	retval = TRUE;

	/* Done */
 out:
	CORBA_exception_free (&ev);

	return retval;
}

/**
 * corba_init_server:
 * @void:
 *
 * Initializes the CORBA server for gnome-terminal.  Returns whether
 * initialization was successful or not, and sets the global
 * corba_have_server variable.
 *
 * Return value: TRUE if okay, FALSE if error
 **/
int
corba_init_server (CORBA_ORB _orb)
{
	int retval;
	CORBA_Environment ev;

	orb = _orb;

	retval = FALSE;
	CORBA_exception_init (&ev);

	/* Get the POA and create the server */

	poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		goto out;

	CORBA_exception_free (&ev);

	/* See if the servers are there */
	
	terminal_factory_server = get_terminal_factory ();

	if (terminal_factory_server != CORBA_OBJECT_NIL) {
		has_terminal_factory = TRUE;
		retval = TRUE;
	} else
		retval = register_servers ();

 out:
	return retval;
}

/**
 * corba_activate_server:
 * @void:
 *
 * Activates the POA manager and thus makes the services available to the
 * outside world.
 **/
void
corba_activate_server (void)
{
	CORBA_Environment ev;
	PortableServer_POAManager poa_manager;

	g_return_if_fail (start_terminal_factory);

	/* Do nothing if the server is already running */
	if (has_terminal_factory)
		return;

	CORBA_exception_init (&ev);

	poa_manager = PortableServer_POA__get_the_POAManager (poa, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		goto out;

	PortableServer_POAManager_activate (poa_manager, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		goto out;

 out:

	CORBA_exception_free (&ev);
}


/* Client-side helper functions */

/* Tries to contact the terminal factory */
static CORBA_Object
get_terminal_factory (void)
{
	CORBA_Object obj;

	obj = goad_server_activate_with_id (
		NULL,
		"IDL:GNOME:Terminal:TerminalFactory:1.0",
		GOAD_ACTIVATE_EXISTING_ONLY,
		NULL);
	return obj;
}

/* Creates a new terminal with the specified geometry */
CORBA_Object
create_terminal_via_factory (const char *geometry, CORBA_Environment *ev)
{
	CORBA_Object obj, term;

	obj = get_terminal_factory ();
	has_terminal_factory = (obj != CORBA_OBJECT_NIL);

	if (obj == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;

	g_assert (geometry != NULL);

	term = GNOME_Terminal_TerminalFactory_create_terminal (obj, geometry, ev);
	CORBA_Object_release (obj, ev);
	return term;
}
