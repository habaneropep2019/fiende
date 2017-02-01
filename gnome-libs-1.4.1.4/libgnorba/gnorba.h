#ifndef GNORBA_H
#define GNORBA_H 1

#include <orb/orbit.h>
#include <ORBitservices/CosNaming.h>
#include <gnome.h>
#include <libgnorba/gnome-factory.h>

/**** orbitgtk module ****/

typedef enum {
  GNORBA_INIT_SERVER_FUNC = 1 << 0,
  GNORBA_INIT_DISABLE_COOKIES = 1 << 1,
  GNORBA_INIT_CORBA_PRIO_HIGH = 1 << 2
} GnorbaInitFlags;
/* 
 * Almost the same as gnome_init, except it initializes ORBit for use
 * with gtk+ too 
 */
CORBA_ORB gnome_CORBA_init          (const char *app_id,
				     const char *app_version,
				     int *argc, char **argv,
				     GnorbaInitFlags gnorba_flags,
				     CORBA_Environment *ev);

CORBA_ORB gnome_CORBA_init_with_popt_table(const char *app_id,
					   const char *app_version,
					   int *argc, char **argv,
					   const struct poptOption *options,
					   int popt_flags,
					   poptContext *return_ctx,
					   GnorbaInitFlags gnorba_flags,
					   CORBA_Environment *ev);
CORBA_ORB gnome_CORBA_ORB(void); /* Just returns the same value as the above */

/* This only initializes CORBA */
CORBA_ORB gnorba_CORBA_init (int *argc, char **argv,
			     GnorbaInitFlags flags,
			     CORBA_Environment *ev);
/*
  Gets the naming server from the X Propery on the root window. If
  this property does not exist, or the name server which has been
  registered has died, a new name server is started and a naming
  context object is returned.
*/
CORBA_Object gnome_name_service_get       (void);

/* If this program was activated for a specific goad id, print it out */
const char *goad_server_activation_id(void);

/* register an object with the name server. name_server is the object
 * returned by a call to gnome_name_service_get, and server is your
 *  CORBA server.
 *
 * Return -1 on error,
 *        -2 if another server with the same NAME and
 *            KIND is already active and running,
 *         0 otherwise;
 * You might check ev for more error information.
 */
int
goad_server_register (CORBA_Object name_server,
		      CORBA_Object server,
		      const char* name,
		      const char* kind,
		      CORBA_Environment* ev);

/*
 * Deregister name from the name server.
 */
int
goad_server_unregister  (CORBA_Object name_server,
			 const char* name,
			 const char* kind,
			 CORBA_Environment* ev);

/**** gnome-plugins module ****/
typedef struct {
	const char   **repo_id;
	const char   *server_id;
	const char   *kind;
	const char   *description;
	CORBA_Object (*activate)   (PortableServer_POA poa,
				    const char *goad_id,
				    const char **params,
				    gpointer *impl_ptr,
				    CORBA_Environment *ev);

	void         (*deactivate) (PortableServer_POA poa,
				    const char *goad_id,
				    gpointer impl_ptr,
				    CORBA_Environment *ev); /* Not used - make it NULL */
} GnomePluginObject;

typedef struct {
	const GnomePluginObject *plugin_object_list;
	const char *description;
} GnomePlugin;

void gnome_plugin_unuse(gpointer impl_ptr);

/**** goad module ****/
typedef enum {
	GOAD_SERVER_SHLIB = 1,
	GOAD_SERVER_EXE = 2,
	GOAD_SERVER_RELAY = 3,
	GOAD_SERVER_FACTORY = 4
} GoadServerType;

typedef enum {
	/* these two are mutually exclusive */
	GOAD_ACTIVATE_SHLIB = 1 << 0, 	/* prefer shlib activation */
	GOAD_ACTIVATE_REMOTE = 1 << 1, 	/* prefer remote activation */

	/* these two are mutually exclusive */
	GOAD_ACTIVATE_EXISTING_ONLY = 1 << 2, /* Only do lookup in name
					       * service for currently running
					       * version.
					       */
	GOAD_ACTIVATE_NEW_ONLY = 1 << 3,      /* No lookup in name service. */
	GOAD_ACTIVATE_ASYNC = 1 << 4 /* Just make sure that the object is running */
} GoadActivationFlags;

typedef struct {
	GoadServerType type;
        GoadActivationFlags flags; /* only GOAD_ACTIVATE_NEW_ONLY
				      currently parsed in */
	char     **repo_id;
	char     *server_id;
	char     *description;

        /*
	 * Executable/shlib path, relayer IOR, whatever.
	 * This field may disappear at any time. You have been warned ;-)
	 */
	char     *location_info;
} GoadServer;

typedef struct {
  GoadServer *list;
  GHashTable *by_goad_id;
} GoadServerList;

/*
 * goad_servers_list:
 *
 * Return value:
 *   An array of GoadServers. The repo_id in the last array element is NULL
 */
GoadServerList *      goad_server_list_get              (void);
void              goad_server_list_free             (GoadServerList *server_list);

/*
 * Passing GOAD_ACTIVATE_{REMOTE,SHLIB} flags to this routine doesn't make sense,
 * since the activation info is already specified in 'sinfo'.
 */
CORBA_Object      goad_server_activate              (GoadServer *sinfo,
						     GoadActivationFlags flags,
						     const char **params);

/*
 * Picks the first one on the list that meets criteria.
 * You can pass in a NULL 'server_list' if you wish, and
 * this routine will call goad_server_list_get() internally.
 */
CORBA_Object      goad_server_activate_with_repo_id (GoadServerList *server_list,
						     const char *repo_id,
						     GoadActivationFlags flags,
						     const char **params);

/*
 * Activates a specific server by its GOAD ID.
 */
CORBA_Object
goad_server_activate_with_id(GoadServerList *server_list,
			     const char *server_id,
			     GoadActivationFlags flags,
			     const char **params);

#endif
