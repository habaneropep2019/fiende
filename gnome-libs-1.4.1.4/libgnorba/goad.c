/*
 * goad.c:
 *
 * Author:
 *   Elliot Lee (sopwith@cuc.edu)
 */
#include <config.h>
#include <string.h>
#include <sys/types.h>

#include "gnorba.h"
#include <gmodule.h>
#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <signal.h>
#include "gnome-factory.h"

#define MAX_TTD        64

#ifdef HAVE_DLFCN_H
#    define SHLIB_DEPENDENCIES 1
#endif

#ifdef SHLIB_DEPENDENCIES
#    include <dlfcn.h>
#endif
#include <ctype.h>

#define SERVER_LISTING_PATH "CORBA/servers"

typedef struct {
  int refcount;
  char *filename;
  GModule *loaded;
  int mainloop_level_load;
  guint unload_id;
  gboolean unload_is_quit;
} ActivePluginInfo;

extern CORBA_ORB _gnorba_gnome_orbit_orb; /* In orbitgtk.c */

static GHashTable *living_shlib_servers = NULL, *living_by_filename = NULL;
static const char *goad_activation_id = NULL;
static int goad_fd = 0;

static void goad_server_list_read(const char *filename,
				  GArray *servinfo,
				  GString *tmpstr,
				  GoadServerList *newl);
static GoadServerType goad_server_typename_to_type(const char *typename);
static CORBA_Object real_goad_server_activate(GoadServer *sinfo,
					      GoadActivationFlags flags,
					      const char **params,
					      GoadServerList *server_list,
					      int ttd);
static CORBA_Object goad_server_activate_shlib(GoadServer *sinfo,
					       GoadActivationFlags flags,
					       const char **params,
					       CORBA_Environment *ev,
					       int ttd);
static CORBA_Object goad_server_activate_exe(GoadServer *sinfo,
					     GoadActivationFlags flags,
					     const char **params,
					     CORBA_Environment *ev,
					     int ttd);
static CORBA_Object goad_server_activate_factory(GoadServer *sinfo,
						 GoadActivationFlags flags,
						 const char **params,
						 CORBA_Environment *ev,
						 GoadServerList *slist,
						 int ttd);
void goad_register_arguments(void);

static int string_array_len(const char **array)
{
  int i;

  if(!array) return 0;

  for(i = 0; array[i]; i++) /* */ ;

  return i;
}

static gboolean string_in_array(const char *string, const char **array)
{
  int i;
  for(i = 0; array[i]; i++) {
    if(!strcmp(string, array[i]))
      return TRUE;
  }

  return FALSE;
}

/*
 * Loads the goad files in @dir_path
 */
static void
load_servers_from (const char *dir_path, GoadServerList *newl, GArray *servinfo)
{
	DIR *dirh;
	struct dirent *dent;
	GString *tmpstr;
	
	dirh = opendir (dir_path);

	if (!dirh)
		return;
	
	tmpstr = g_string_new(NULL);
	while ((dent = readdir (dirh))) {
		char *p;

		p = strrchr (dent->d_name, '.');
		if (!p || (strcmp (p, ".goad") != 0 && strcmp (p, ".gnorba") != 0))
			continue;

		g_string_sprintf (tmpstr, "=%s/%s", dir_path, dent->d_name);
		goad_server_list_read (tmpstr->str, servinfo, tmpstr, newl);
	}
	closedir (dirh);
	g_string_free (tmpstr, TRUE);
}

/*
 * Loads any servers that might be in the environment variable @env
 */
static void
load_from_env (const char *env_name, GoadServerList *newl, GArray *servinfo, gboolean append_postfix)
{
	char *env_copy, *s, *path, *tokp, *env;

	env = getenv (env_name);

	if (!env)
		return;
	
	env_copy = g_strdup (env);
	s = env_copy;
	
	while ((path = strtok_r (s, ":", &tokp)) != NULL){
		char *full;

		s = NULL;
		
		if (append_postfix)
			full = g_concat_dir_and_file (path, "etc/" SERVER_LISTING_PATH);
		else
			full = g_strdup (path);

		load_servers_from (full, newl, servinfo);
		g_free (full);
	}
	g_free (env_copy);
}

/**
 * goad_server_list_get:
 *
 * Returns an array listing all the servers available
 * for activation.
 *
 * Returns a newly created server list.
 */
GoadServerList *
goad_server_list_get(void)
{
	GArray *servinfo;
	GoadServerList *newl;
	GHashTable *by_goad_id;
	int i;
	char *system_dir, *user_dir;
	
	newl = g_new0(GoadServerList, 1);
	servinfo = g_array_new(TRUE, FALSE, sizeof(GoadServer));
	by_goad_id = g_hash_table_new(g_str_hash, g_str_equal);
	newl->by_goad_id = by_goad_id;
	
	/* User servers (preferred over system) */
	user_dir = g_concat_dir_and_file (gnome_user_dir, SERVER_LISTING_PATH);
	load_servers_from (user_dir, newl, servinfo);
	g_free (user_dir);
	
	/* System servers */
	system_dir = gnome_config_file (SERVER_LISTING_PATH);
	if (system_dir)
		load_servers_from (system_dir, newl, servinfo);
	g_free (system_dir);
	
	/* Now, user defined locations */
	load_from_env ("GNOME_PATH", newl, servinfo, TRUE);
	load_from_env ("GNOME_GNORBA_PATH", newl, servinfo, FALSE);
	
	newl->list = (GoadServer *)servinfo->data;
	if (newl->list) {
		for (i = 0; newl->list[i].repo_id; i++)
			g_hash_table_insert (newl->by_goad_id, newl->list[i].server_id, &newl->list[i]);
	}
	
	g_array_free (servinfo, FALSE);
	
	return newl;
}

static GoadActivationFlags
goad_activation_combine_flags(GoadServer *sinfo,
			      GoadActivationFlags user_flags)
{
  GoadActivationFlags retval;

  retval = (user_flags & ~(GOAD_ACTIVATE_REMOTE|GOAD_ACTIVATE_SHLIB|GOAD_ACTIVATE_NEW_ONLY|GOAD_ACTIVATE_EXISTING_ONLY));
  
  if(sinfo->flags & (GOAD_ACTIVATE_REMOTE|GOAD_ACTIVATE_SHLIB))
    retval |= sinfo->flags & (GOAD_ACTIVATE_REMOTE|GOAD_ACTIVATE_SHLIB);
  else
    retval |= user_flags & (GOAD_ACTIVATE_REMOTE|GOAD_ACTIVATE_SHLIB);

  if(sinfo->flags & (GOAD_ACTIVATE_NEW_ONLY|GOAD_ACTIVATE_EXISTING_ONLY))
    retval |=
      sinfo->flags & (GOAD_ACTIVATE_NEW_ONLY|GOAD_ACTIVATE_EXISTING_ONLY);
  else
    retval |= user_flags & (GOAD_ACTIVATE_NEW_ONLY|GOAD_ACTIVATE_EXISTING_ONLY);

  return retval;
}
  
static GoadActivationFlags
goad_server_flagstring_to_flags(char *flstr)
{
  char **ctmp;
  GoadActivationFlags retval = 0;
  int i;

  if(!flstr) return retval;

  g_strstrip(flstr);

  if(!*flstr) return retval;

  ctmp = g_strsplit(flstr, "|", 0);

  for(i = 0; ctmp[i]; i++) {
    if(!strcasecmp(ctmp[i], "new_only")) {
      if(retval & GOAD_ACTIVATE_EXISTING_ONLY)
	g_warning("Can't combine existing_only and new_only activation flags");
      else
	retval |= GOAD_ACTIVATE_NEW_ONLY;
    } else if(!strcasecmp(ctmp[i], "existing_only")) {
      if(retval & GOAD_ACTIVATE_NEW_ONLY)
	g_warning("Can't combine existing_only and new_only activation flags");
      else
	retval |= GOAD_ACTIVATE_EXISTING_ONLY;
    } else if(!strcasecmp(ctmp[i], "shlib")) {
      if(retval & GOAD_ACTIVATE_REMOTE)
	g_warning("Can't combine shlib and remote activation flags");
      else
	retval |= GOAD_ACTIVATE_SHLIB;
    } else if(!strcasecmp(ctmp[i], "remote")) {
      if(retval & GOAD_ACTIVATE_SHLIB)
	g_warning("Can't combine shlib and remote activation flags");
      else
	retval |= GOAD_ACTIVATE_REMOTE;
    } else
      g_warning("Unknown activation flag %s", ctmp[i]);
  }

  g_strfreev(ctmp);

  return retval;
}

static GoadServerType
goad_server_typename_to_type(const char *typename)
{
  if(!strcasecmp(typename, "shlib"))
    return GOAD_SERVER_SHLIB;
  else if(!strcmp(typename, "exe"))
    return GOAD_SERVER_EXE;
  else if(!strcasecmp(typename, "relay"))
    return GOAD_SERVER_RELAY;
  else if(!strcasecmp(typename, "factory"))
    return GOAD_SERVER_FACTORY;
  else
    return 0; /* Invalid */
}

/**** goad_server_list_read
      Inputs: 'filename' - file to read entries from.
              'servinfo' - array to append entries onto.
	      'tmpstr' - GString for scratchpad use.

      Side effects: Adds entries to 'servinfo'. Modifies 'tmpstr'.

      Notes: Called by goad_server_list_get() only.

      Description: Adds GoadServer entries from 'filename' onto the
      array 'servinfo'

 */
static void
goad_server_list_read(const char *filename,
		      GArray *servinfo,
		      GString *tmpstr,
		      GoadServerList *newl)
{
  gpointer iter;
  char *typename;
  GoadServer newval;
  GString*   dummy;

  dummy = g_string_new("");
  
  gnome_config_push_prefix(filename);
  iter = gnome_config_init_iterator_sections(filename);

  while((iter = gnome_config_iterator_next(iter, &newval.server_id, NULL))) {
    int vlen;

    if (*filename == '=')
      g_string_sprintf(dummy, "=%s/=type",
		       newval.server_id);
    else
      g_string_sprintf(dummy, "%s/type",
		       newval.server_id);
    typename = gnome_config_get_string(dummy->str);
    newval.type = goad_server_typename_to_type(typename);
    g_free(typename);
    if(!newval.type) {
      g_warning("Server %s has invalid activation method.", newval.server_id);
      g_free(newval.server_id);
      continue;
    }

    if (*filename == '=')
      g_string_sprintf(dummy, "=%s/=flags",
		       newval.server_id);
    else
      g_string_sprintf(dummy, "%s/flags",
		       newval.server_id);
    typename = gnome_config_get_string(dummy->str);
    newval.flags = goad_server_flagstring_to_flags(typename);
    g_free(typename);

    if (*filename == '=')
      g_string_sprintf(dummy, "=%s/=repo_id",
		       newval.server_id);
    else
      g_string_sprintf(dummy, "%s/repo_id",
		       newval.server_id);
    gnome_config_get_vector(dummy->str, &vlen, &newval.repo_id);
    newval.repo_id = g_realloc(newval.repo_id, (vlen + 1)*sizeof(char *));
    newval.repo_id[vlen] = NULL;

    if (*filename == '=')
      g_string_sprintf(dummy, "=%s/=description",
		       newval.server_id);
    else
      g_string_sprintf(dummy, "%s/description",
		       newval.server_id);
    newval.description = gnome_config_get_string(dummy->str);

    if (*filename == '=')
      g_string_sprintf(dummy, "=%s/=location_info",
		       newval.server_id);
    else
      g_string_sprintf(dummy, "%s/location_info",
		       newval.server_id);
    newval.location_info = gnome_config_get_string(dummy->str);
    g_array_append_val(servinfo, newval);
  }

  gnome_config_pop_prefix();
  /*forget the config information about this file, otherwise we
    take up gobs of memory*/
  if (*filename == '=')
	  g_string_sprintf(dummy, "%s=", filename);
  else
	  g_string_assign(dummy,filename);
  gnome_config_drop_file(dummy->str);
  g_string_free(dummy,TRUE);
}

/**
 * goad_server_list_free:
 * @server_list: a GoadServerList structure.
 *
 * Frees up all the memory associated with @server_list
 * (which should have been received from goad_server_list_get ())
 *
 * Side effects: Invalidates the memory pointed to by
 * 'server_list'.
 */
void
goad_server_list_free (GoadServerList *server_list)
{
  int i;
  GoadServer *sl;

  sl = server_list->list;

  if(sl) {
    for(i = 0; sl[i].repo_id; i++) {
      g_strfreev(sl[i].repo_id);
      g_free(sl[i].server_id);
      g_free(sl[i].description);
      g_free(sl[i].location_info);
    }

    g_free(sl);
  }

  g_hash_table_destroy(server_list->by_goad_id);

  g_free(server_list);
}


static CORBA_Object
real_server_activate_with_id (GoadServerList *server_list,
			      const char *server_id,
			      GoadActivationFlags flags,
			      const char **params,
			      int ttd)
{
  GoadServerList *my_servlist;
  GoadServer *slist;
  CORBA_Object retval = CORBA_OBJECT_NIL;

  g_return_val_if_fail(server_id, CORBA_OBJECT_NIL);
  g_return_val_if_fail(!((flags & GOAD_ACTIVATE_EXISTING_ONLY)
		   && (flags & GOAD_ACTIVATE_NEW_ONLY)), CORBA_OBJECT_NIL);
  g_return_val_if_fail(!((flags & GOAD_ACTIVATE_SHLIB)
		   && (flags & GOAD_ACTIVATE_REMOTE)), CORBA_OBJECT_NIL);

  /* XXX TODO: try getting a running server from the name service first */
  if(server_list)
    my_servlist = server_list;
  else
    my_servlist = goad_server_list_get();

  g_return_val_if_fail(my_servlist, CORBA_OBJECT_NIL);

  slist = my_servlist->list;

  if(!slist)
    goto errout;

  slist = g_hash_table_lookup(my_servlist->by_goad_id, server_id);

  if(slist)
    retval = real_goad_server_activate(slist, flags, params, my_servlist, ttd);

 errout:
  if(!server_list)
    goad_server_list_free(my_servlist);

  return retval;
}

/**
 * goad_server_activate_with_id:
 * @server_list: a server listing returned by goad_server_list_get.
 * If NULL, we will call that function ourself and use that.
 * @server_id: the GOAD ID of the server that we want to activate.
 * @flags: information on how the application wants the server to be activated.
 * @params: NULL for now.
 *
 * Activates a CORBA server specified by 'repo_id', using
 * the 'flags' hints on how to activate that server.
 * Picks the first one on the list that matches.
 *
 * Returns the newly activated object.
 */
CORBA_Object
goad_server_activate_with_id (GoadServerList *server_list,
			      const char *server_id,
			      GoadActivationFlags flags,
			      const char **params)
{
	return real_server_activate_with_id (server_list, server_id,
					     flags, params, MAX_TTD);
}

/**
 * goad_server_activate_with_repo_id:
 * @server_list: a server listing returned by goad_server_list_get.
 *               If NULL, we will call the function ourself and use that.
 * @repo_id: the repository ID of the interface that we want to activate a server for.
 * @flags: information on how the application wants the server to be activated.
 * @params: NULL for normal applications.
 *
 * Activates a CORBA server specified by 'repo_id', using the 'flags'
 * hints on how to activate that server.  Picks the first one on the
 * list that meets criteria.
 *
 * This is done by possibly making three passes through the list, the
 * first pass checking for existing objects only, the second pass
 * taking into account any activation method preferences, and the last
 * pass just doing "best we can get" service.
 *
 * Returns the activated object.
 */
CORBA_Object
goad_server_activate_with_repo_id(GoadServerList *server_list,
				  const char *repo_id,
				  GoadActivationFlags flags,
				  const char **params)
{
  GoadServerList *my_slist;
  GoadServer *slist;
  CORBA_Object retval = CORBA_OBJECT_NIL;
  int i;
  enum { PASS_CHECK_EXISTING = 0, PASS_PREFER, PASS_FALLBACK, PASS_DONE } passnum;

  g_return_val_if_fail(repo_id, CORBA_OBJECT_NIL);
  g_return_val_if_fail(!((flags & GOAD_ACTIVATE_EXISTING_ONLY)
		   && (flags & GOAD_ACTIVATE_NEW_ONLY)), CORBA_OBJECT_NIL);
  g_return_val_if_fail(!((flags & GOAD_ACTIVATE_SHLIB)
		   && (flags & GOAD_ACTIVATE_REMOTE)), CORBA_OBJECT_NIL);

  if(server_list)
    my_slist = server_list;
  else
    my_slist = goad_server_list_get();

  g_return_val_if_fail(my_slist, retval);
  slist = my_slist->list;

  if(!slist) goto errout;
  
  /* (unvalidated assumption) If we need to only activate existing objects, then
     we don't want to bother checking activation methods, because
     we won't be activating anything. :)
                       OR
     if the app has not specified any activation method preferences,
     then we obviously don't need to bother with that pass through the list.
  */

  for(passnum = PASS_CHECK_EXISTING; passnum < PASS_DONE; passnum++) {
    
    for(i = 0; slist[i].repo_id; i++) {
      if(passnum == PASS_PREFER) {
	/* Check the type */
	if(((flags & GOAD_ACTIVATE_SHLIB)
	    && slist[i].type != GOAD_SERVER_SHLIB))
	  continue;
	    
	if((flags & GOAD_ACTIVATE_REMOTE)
	   && slist[i].type != GOAD_SERVER_EXE)
	  continue;
      }

      if(!string_in_array(repo_id, (const char **)slist[i].repo_id))
	continue;
	
      /* entry matched */
      if(passnum == PASS_CHECK_EXISTING) {
	retval = real_goad_server_activate(&slist[i], flags | GOAD_ACTIVATE_EXISTING_ONLY,
				      params, my_slist, MAX_TTD);
      }
      else {
	retval = real_goad_server_activate(&slist[i], flags | GOAD_ACTIVATE_NEW_ONLY,
					   params, my_slist, MAX_TTD);
      }
      if (retval != CORBA_OBJECT_NIL)
	break;
    }
    /* If we got something, out of here.	
       If we were asked to check existing servers and we are done that,	
       out of here.	
       If we were not asked to do any special activation method checking,	
       then we've done all the needed passes, out of here.
    */
      
    if(retval != CORBA_OBJECT_NIL
       || ((passnum == PASS_CHECK_EXISTING)
	   && (flags & GOAD_ACTIVATE_EXISTING_ONLY))
       || ((passnum == PASS_PREFER)
	   && !(flags & (GOAD_ACTIVATE_SHLIB|GOAD_ACTIVATE_REMOTE))))
      break;
  }

 errout:
  if(!server_list)
    goad_server_list_free(my_slist);

  return retval;
}

/**
 * goad_server_activate:
 * @sinfo: information on the server to be "activated"
 * @flags: information on how the application wants the server to be activated.
 * @params: Pass NULL here for normal applications.
 *
 * Activates a CORBA server specified by 'sinfo', using the 'flags'
 * hints on how to activate that server.
 *
 * Returns a CORBA_Object that points to this server, or CORBA_OBJECT_NIL
 * if the activation failed.
 */
CORBA_Object
goad_server_activate(GoadServer *sinfo,
		     GoadActivationFlags flags,
		     const char **params)
{
  return real_goad_server_activate(sinfo, flags, params, NULL, MAX_TTD);
}

/* Allows using an already-read server list */
static CORBA_Object
real_goad_server_activate(GoadServer *sinfo,
			  GoadActivationFlags flags,
			  const char **params,
			  GoadServerList *server_list,
			  int ttd)
{
  CORBA_Environment       ev;
  CORBA_Object            name_service;
  CosNaming_NameComponent nc[3] = {{"GNOME", "subcontext"},
				   {"Servers", "subcontext"}};
  CosNaming_Name          nom;

  CORBA_Object retval = CORBA_OBJECT_NIL;

  nom._maximum = 0;
  nom._length = 3;
  nom._buffer = nc;
  nom._release = CORBA_FALSE;

  g_return_val_if_fail(sinfo, CORBA_OBJECT_NIL);

  /* make sure they passed in a sane 'flags' */
  g_return_val_if_fail(!((flags & GOAD_ACTIVATE_SHLIB)
		   && (flags & GOAD_ACTIVATE_REMOTE)), CORBA_OBJECT_NIL);
  g_return_val_if_fail(!((flags & GOAD_ACTIVATE_EXISTING_ONLY)
		   && (flags & GOAD_ACTIVATE_NEW_ONLY)), CORBA_OBJECT_NIL);

  if (ttd-- < 1) { /* A circular reference */
	  g_warning ("Circular reference for '%s'",
		     sinfo->server_id?sinfo->server_id:"no-id");
	  return CORBA_OBJECT_NIL;
  }

  flags = goad_activation_combine_flags(sinfo, flags);

  CORBA_exception_init(&ev);

  /* First, do name service lookup (if not specifically told not to) */
  if(!(flags & GOAD_ACTIVATE_NEW_ONLY)) {

    name_service = gnome_name_service_get();
    g_assert(name_service != CORBA_OBJECT_NIL);

    nc[2].id = sinfo->server_id;
    nc[2].kind = "object";
    retval = CosNaming_NamingContext_resolve(name_service, &nom, &ev);
    if (ev._major == CORBA_USER_EXCEPTION
	&& strcmp(CORBA_exception_id(&ev), ex_CosNaming_NamingContext_NotFound) == 0) {
      retval = CORBA_OBJECT_NIL;
    }
    else if (ev._major != CORBA_NO_EXCEPTION) {
      g_warning("goad_server_activate: %s %d: unexpected exception %s:", __FILE__, __LINE__, ev._repo_id);
      switch( ev._major ) {
	case CORBA_SYSTEM_EXCEPTION:
	  g_warning("sysex: %s.\n", CORBA_exception_id(&ev));
	case CORBA_USER_EXCEPTION:
	  g_warning( "usrex: %s.\n", CORBA_exception_id( &ev ) );
	default:
	  break;
	}
    }
    ev._major = CORBA_NO_EXCEPTION;
    CORBA_Object_release(name_service, &ev);
    if (ev._major != CORBA_NO_EXCEPTION) {
	retval = CORBA_OBJECT_NIL;
      }
    
    if(!CORBA_Object_is_nil(retval, &ev) || (flags & GOAD_ACTIVATE_EXISTING_ONLY))
      goto out;
  }
  
  switch(sinfo->type) {
  case GOAD_SERVER_SHLIB:
    if (flags & GOAD_ACTIVATE_REMOTE) {
      GoadServer fake_sinfo;
      gchar cmdline[1024];

      fake_sinfo = *sinfo;
      fake_sinfo.type = GOAD_SERVER_EXE;

      g_snprintf(cmdline, sizeof(cmdline), "loadshlib -i %s -r %s %s",
		 sinfo->server_id, sinfo->repo_id[0], sinfo->location_info);
      fake_sinfo.location_info = cmdline;
      retval = goad_server_activate_exe(&fake_sinfo, flags, params, &ev, ttd);
    } else {
      retval = goad_server_activate_shlib(sinfo, flags, params, &ev, ttd);
    }
    break;
  case GOAD_SERVER_EXE:
    retval = goad_server_activate_exe(sinfo, flags, params, &ev, ttd);
    break;
  case GOAD_SERVER_RELAY:
    g_warning("Relay interface not yet defined (write an RFC :). Relay objects NYI");
    break;
  case GOAD_SERVER_FACTORY:
    retval = goad_server_activate_factory(sinfo, flags, params, &ev,
					  server_list, ttd);
  }
 out:
  CORBA_exception_free(&ev);
  return retval;
}

/**
 * goad_server_activate_shlib:
 * @sinfo: information on the plugin to be loaded.
 * @flags: information about how the plugin should be loaded, etc.
 * @ev: exception information (passed in to save us creating another one)
 *
 * Pre-conditions: Assumes sinfo->type == GOAD_SERVER_SHLIB
 *
 * Side effects: May add information on the newly created server to
 * 'our_active_servers' list, so we can unregister the server from the
 * name service when we exit.
 *
 * Loads the plugin specified in 'sinfo'. Looks for an
 * object of id 'sinfo->server_id' in it, and activates it if
 * found.
 */
static CORBA_Object
goad_server_activate_shlib(GoadServer *sinfo,
			   GoadActivationFlags flags,
			   const char **params,
			   CORBA_Environment *ev,
			   int ttd)
{
  const GnomePlugin *plugin;
  int i;
  PortableServer_POA poa;
  CORBA_Object retval;
  ActivePluginInfo *local_plugin_info = NULL;
  gpointer impl_ptr;
  GModule *gmod;
  char *filename = NULL;

#ifdef SHLIB_DEPENDENCIES
  FILE* lafile;
  gchar* ptr;
  gchar line[128];
  gint len = strlen(sinfo->location_info);
  void* handle;
  
  if (!strcmp(sinfo->location_info + (len - 2), "la")) {
    /* find inter-library depencies in the .la file */
    lafile = fopen(sinfo->location_info, "r");
    if (!lafile)
      goto normal_loading;	/* bail out and let the normal loading
				   code handle the error */
    while ((ptr = fgets(line, sizeof(line), lafile))) {
      if (!strncmp(line, "dependency_libs='", strlen("dependency_libs='")))
	break;
    }
    fclose(lafile);
    if (!ptr) {
      /* no dependcy line, just load the lib */
      goto normal_loading;
    }
    ptr = line + strlen("dependency_libs='") + 1;
    /* ptr now is on the '-' of the first lib */ 
    while (1) {
      gchar*  libstart;
      gchar   libname[128];
      ptr += 2;
      libstart = ptr;
      while (*ptr != '\'' && !isspace(*ptr))
	ptr++;
      if (*ptr == '\'')
	break;
      memset(libname, 0, sizeof(libname));
      strcpy(libname, "lib");
      strncat(libname, libstart, ptr - libstart);
      strcat(libname, ".so");
      fprintf(stderr,"Using '%s' for dlopen\n", libname);
#ifdef RTLD_GLOBAL
      handle = dlopen(libname, RTLD_GLOBAL | RTLD_LAZY);
#else
      handle = dlopen(libname, RTLD_LAZY);
#endif /*RTLD_GLOBAL*/

      if (!handle) {
	g_warning("Cannot load %s: %s", libname, dlerror());
      }
      ptr += 1;
    }
normal_loading:
    sinfo->location_info[len-1] = 'o';
    sinfo->location_info[len-2] = 's';
  }
#endif

  if(living_by_filename)
    local_plugin_info = g_hash_table_lookup(living_by_filename, sinfo->location_info);

  if(local_plugin_info)
    gmod = local_plugin_info->loaded;
  else {
    gmod = g_module_open(sinfo->location_info, G_MODULE_BIND_LAZY);

    if(gmod)
      filename = g_strdup(sinfo->location_info);
    else if(*sinfo->location_info != '/') {
      char *ctmp;
      ctmp = gnome_libdir_file(sinfo->location_info);
      if(living_by_filename)
	local_plugin_info = g_hash_table_lookup(living_by_filename, ctmp);

      if(local_plugin_info) {
	g_free(ctmp);
	gmod = local_plugin_info->loaded;
      } else {
	gmod = g_module_open(ctmp, G_MODULE_BIND_LAZY);

	if(gmod)
	  filename = ctmp;
	else
	  g_warning("CORBA plugin load failed: %s", g_module_error());
      }
    } else
      g_warning("CORBA plugin load failed: %s", g_module_error());
  }

  g_return_val_if_fail(gmod, CORBA_OBJECT_NIL);

  i = g_module_symbol(gmod, "GNOME_Plugin_info",
		      (gpointer *)&plugin);
  g_return_val_if_fail(i, CORBA_OBJECT_NIL);

  for(i = 0; plugin->plugin_object_list[i].repo_id; i++) {
    if(!strcmp(sinfo->server_id, plugin->plugin_object_list[i].server_id))
       break;
  }
  g_return_val_if_fail(plugin->plugin_object_list[i].repo_id, CORBA_OBJECT_NIL);

  poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(_gnorba_gnome_orbit_orb,
								 "RootPOA", ev);

  retval = plugin->plugin_object_list[i].activate(poa, plugin->plugin_object_list[i].server_id, params, &impl_ptr, ev);


  if (ev->_major != CORBA_NO_EXCEPTION) {
    g_warning("goad_server_activate_shlib: activation function raises exception");
    switch( ev->_major ) {
    case CORBA_SYSTEM_EXCEPTION:
      g_warning("sysex: %s.\n", CORBA_exception_id(ev));
    case CORBA_USER_EXCEPTION:
      g_warning( "usrex: %s.\n", CORBA_exception_id( ev ) );
    default:
      break;
    }
    return CORBA_OBJECT_NIL;
  }

  if(!living_shlib_servers) {
    living_shlib_servers = g_hash_table_new(g_direct_hash, g_direct_equal);
    living_by_filename = g_hash_table_new(g_str_hash, g_str_equal);
  }

  if(!local_plugin_info) {
    local_plugin_info = g_new0(ActivePluginInfo, 1);
    local_plugin_info->filename = filename;
    local_plugin_info->loaded = gmod;
    local_plugin_info->mainloop_level_load = gtk_main_level();
    local_plugin_info->unload_id = -1;

    g_hash_table_insert(living_by_filename, filename, local_plugin_info);
  }

  g_hash_table_insert(living_shlib_servers, impl_ptr, local_plugin_info);

  local_plugin_info->refcount++;

  if(local_plugin_info->unload_id != -1) {
    if(local_plugin_info->unload_is_quit)
      gtk_quit_remove(local_plugin_info->unload_id);
    else
      gtk_idle_remove(local_plugin_info->unload_id);
    local_plugin_info->unload_id = -1;
  }

  return retval;
}

static gboolean
gnome_plugin_unload(ActivePluginInfo *api)
{
  g_return_val_if_fail(api->refcount <= 0, FALSE);

  g_module_close(api->loaded);
  g_hash_table_remove(living_by_filename, api->filename);
  g_free(api->filename);
  g_free(api);

  return FALSE;
}

/**
 * gnome_plugin_unuse:
 * @impl_ptr: The impl_ptr that was returned when the
 *
 * Side effects: May arrange for shared library that the implementation is in to be unloaded.
 *
 * When a shlib plugin for a CORBA object is destroying an
 * implementation, it should call this function to make sure that the
 * shared library is unloaded as needed.
 */
void
gnome_plugin_unuse(gpointer impl_ptr)
{
  ActivePluginInfo *api;

  api = g_hash_table_lookup(living_shlib_servers, impl_ptr);

  g_return_if_fail(api);

  g_hash_table_remove(living_shlib_servers, impl_ptr);

  api->refcount--;

  if(api->refcount <= 0) {
    if(api->unload_id != -1)
      g_warning("Plugin %s already queued for unload!", api->filename);

    if(gtk_main_level() <= api->mainloop_level_load) {
      api->unload_is_quit = FALSE;
      api->unload_id = gtk_idle_add_priority(G_PRIORITY_LOW, (GtkFunction)gnome_plugin_unload, api);
    } else {
      api->unload_is_quit = TRUE;
      api->unload_id = gtk_quit_add(api->mainloop_level_load + 1, (GtkFunction)gnome_plugin_unload, api);
    }
  }
}

/* Talks to the factory and asks it for info */
static CORBA_Object
goad_server_activate_factory(GoadServer *sinfo,
			     GoadActivationFlags flags,
			     const char **params,
			     CORBA_Environment *ev,
			     GoadServerList *slist,
			     int ttd)
{
  CORBA_Object factory_obj, retval;
  GNOME_stringlist sl;
  int dout;

  factory_obj = real_server_activate_with_id (slist, sinfo->location_info,
					      flags & ~(GOAD_ACTIVATE_ASYNC|GOAD_ACTIVATE_NEW_ONLY), NULL, ttd);

  dout = getenv("GOAD_DEBUG_EXERUN")?1:0;

  if(factory_obj == CORBA_OBJECT_NIL) {
    if(dout)
      g_message("activate_with_id returned NIL");
    return CORBA_OBJECT_NIL;
  }

  sl._length = string_array_len(params);
  sl._buffer = (char **)params;
  sl._release = CORBA_FALSE;

  retval = GNOME_GenericFactory_create_object(factory_obj,
					      sinfo->server_id, &sl, ev);

  if(ev->_major != CORBA_NO_EXCEPTION) {
    if(dout)
      g_message("Got exception %s on create_object call", ev->_repo_id);
    retval = CORBA_OBJECT_NIL;
  }

  CORBA_Object_release(factory_obj, ev);

  return retval;
}

typedef struct {
  GMainLoop *mloop;
  char iorbuf[2048];
  char *do_srv_output;
  FILE *fh;
} EXEActivateInfo;

static gboolean
handle_exepipe(GIOChannel      *source,
	       GIOCondition     condition,
	       EXEActivateInfo *data)
{
  gboolean retval = TRUE;

  *data->iorbuf = '\0';
  if(!(condition & G_IO_IN)
     || !fgets(data->iorbuf, sizeof(data->iorbuf), data->fh)) {
    retval = FALSE;
  }

  if(retval && !strncmp(data->iorbuf, "IOR:", 4))
    retval = FALSE;

  if(data->do_srv_output)
    g_message("srv output[%d]: '%s'", retval, data->iorbuf);

  if(!retval)
    g_main_quit(data->mloop);

  return retval;
}


static void print_exit_status(int status)
{
  if(WIFEXITED(status)) {
    g_message("Exit status was %d", WEXITSTATUS(status));
  }
  if(WIFSIGNALED(status)) {
    g_message("signal was %d", WTERMSIG(status));
  }
}

/**** goad_server_activate_exe
 * sinfo: information on the program to be run.
 * flags: information about how the program should be run, etc.
 * (no flags applicable at the present time)
 * ev: exception information (passed in to save us creating another one)
 *
 * Pre-conditions: Assumes sinfo->type == GOAD_SERVER_EXE
 *
 * Calls setsid to daemonize the server. Expects the server to
 * register itself with the nameing service. Returns after the server
 * printed it's IOR string.  Ignores SIGPIPE in the child, so that the
 * server doesn't die if it writes to a closed fd.
 *
 * Returns an objref to the newly-started server.
 */
static CORBA_Object
goad_server_activate_exe(GoadServer *sinfo,
			 GoadActivationFlags flags,
			 const char **params,
			 CORBA_Environment *ev,
			 int ttd)
{
  gint                    iopipes[2];
  CORBA_Object            retval = CORBA_OBJECT_NIL;
  int childpid;

  pipe(iopipes);
  /* fork & get the ior from stdout */

  childpid = fork();
  if(childpid) {
    int     status;
    FILE*   iorfh;
    EXEActivateInfo ai;
    guint watchid;
    GIOChannel *gioc;

    waitpid(childpid, &status, 0); /* de-zombify */

    ai.do_srv_output = getenv("GOAD_DEBUG_EXERUN");

    if(ai.do_srv_output)
      print_exit_status(status);

    close(iopipes[1]);
    ai.fh = iorfh = fdopen(iopipes[0], "r");

    if(flags & GOAD_ACTIVATE_ASYNC)
      goto no_wait;

    ai.mloop = g_main_new(FALSE);
    gioc = g_io_channel_unix_new(iopipes[0]);
    watchid = g_io_add_watch(gioc, G_IO_IN|G_IO_HUP|G_IO_NVAL|G_IO_ERR,
			     (GIOFunc)&handle_exepipe, &ai);
    g_io_channel_unref(gioc);
    g_main_run(ai.mloop);
    g_main_destroy(ai.mloop);

    g_strstrip(ai.iorbuf);
    if (strncmp(ai.iorbuf, "IOR:", 4)) {
      if(ai.do_srv_output) g_message("string doesn't match IOR:");
      retval = CORBA_OBJECT_NIL;
      goto out;
    }
    retval = CORBA_ORB_string_to_object(_gnorba_gnome_orbit_orb,
					ai.iorbuf, ev);
    if(ai.do_srv_output)
      g_message("Did string_to_object on %s", ai.iorbuf);
  no_wait:
    fclose(iorfh);
  } else if(fork()) {
    _exit(0); /* de-zombifier process, just exit */
  } else {
    char **args;
    char fd_str [20];
    int i;
    struct sigaction sa;
    int devnull_fd;
    int dup2_rv;

    /* stdin needs to be closed here for some stuff to work (panel?) so 
     * we do it by attatching /dev/null to it instead of just closing it.
     * That way, code in the executable that depends on stdin being alive
     * and kicking will continue to work.
     */
    devnull_fd = open ("/dev/null", O_RDONLY);
    g_assert(devnull_fd != -1);
    dup2_rv = dup2 (0, devnull_fd);
    g_assert(dup2_rv != -1);
    
    setsid();
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
    args = g_strsplit(sinfo->location_info, " ", -1);
    for(i = 0; args[i]; i++) /**/ ;

    if (sinfo->server_id) {
      args = g_realloc(args, sizeof(char *) * (i+5));
      args[i] = "--activate-goad-server";
      args[i+1] = sinfo->server_id;
      args[i+2] = "--goad-fd";
      g_snprintf (fd_str, sizeof (fd_str), "%d", iopipes [1]);
      args[i+3] = fd_str;
      args[i+4] = NULL;
      i += 4;
    }

    if (params) {
      int j;

      for (j = 0; params[j]; j++) /**/ ;

      args = g_realloc(args, sizeof(char *) * (i+j));

      for (j = 0; params[j]; j++)
	args[i+j] = (char *)params[j];
      args[i+j] = NULL;
    }

    execvp(args[0], args);
    _exit(1);
  }
out:
  if(getenv("GOAD_DEBUG_EXERUN"))
    g_message("Retval on %s is %p", sinfo->server_id, retval);
  return retval;
}

/**
 * goad_server_register:
 * @name_server: points to a running name_server
 * @server: the server object we want to register.
 * @name: The GOAD id of the server that is being registered
 * @kind: "object" for now.
 * @ev: CORBA_Environment to return errors
 *
 * Registers @server in the @name_server with @name.
 *
 * Returns zero upon success, non-zero if registration failed or another registration for this name already exists.
 */
int
goad_server_register(CORBA_Object name_server,
		     CORBA_Object server,
		     const char* name,
		     const char* kind,
		     CORBA_Environment* ev)
{
  CosNaming_NameComponent nc[3] = {{"GNOME", "subcontext"},
				   {"Servers", "subcontext"}};
  CosNaming_Name          nom;
  CORBA_Object            old_server, orig_ns = name_server;
  static int did_print_ior = 0;

  nom._maximum = 0;
  nom._length = 3;
  nom._buffer = nc;
  nom._release = CORBA_FALSE;

  CORBA_exception_free(ev);

  if(!did_print_ior
     && goad_activation_id
     && (!name || !strcmp(goad_activation_id, name))) {
    CORBA_char *strior;
    FILE *iorout;
    struct sigaction oldaction, myaction;

    iorout = fdopen (goad_fd, "a");
    if(iorout) {
      memset(&myaction, '\0', sizeof(myaction));
      myaction.sa_handler = SIG_IGN;
      
      sigaction(SIGPIPE, &myaction, &oldaction);
      strior = CORBA_ORB_object_to_string(gnome_CORBA_ORB(), server, ev);
      if(ev->_major == CORBA_NO_EXCEPTION) {
	fprintf(iorout, "%s\n", strior);
	CORBA_free(strior);
      }
      fflush(iorout);
      fclose(iorout);
      sigaction(SIGPIPE, &oldaction, NULL);
    }

    did_print_ior = 1;
  }

  if (!name)
    return 0;

  if (!kind)
    kind = "object";
  
  nc[2].id   = (char *)name;
  nc[2].kind = (char *)kind;

  CORBA_exception_free(ev);

  if(name_server == CORBA_OBJECT_NIL)
    name_server = gnome_name_service_get();

  g_assert(name_server != CORBA_OBJECT_NIL);

  old_server = CosNaming_NamingContext_resolve(name_server, &nom, ev);

  if(ev->_major == CORBA_NO_EXCEPTION
     || (ev->_major == CORBA_USER_EXCEPTION
	 && strcmp(CORBA_exception_id(ev),
		   ex_CosNaming_NamingContext_NotFound))) {
    CORBA_Object_release(old_server, ev);

    if(orig_ns == CORBA_OBJECT_NIL)
      CORBA_Object_release(name_server, ev);

    return -2;
  }

  CORBA_exception_free(ev);

  CosNaming_NamingContext_bind(name_server, &nom, server, ev);

  if (ev->_major != CORBA_NO_EXCEPTION) {

    if(orig_ns == CORBA_OBJECT_NIL)
      CORBA_Object_release(name_server, ev);

    return -1;
  }

  if(orig_ns == CORBA_OBJECT_NIL)
    CORBA_Object_release(name_server, ev);

  CORBA_exception_free(ev);

  return 0;
}

/**
 * goad_server_unregister:
 * @name_server: points to a running name_server
 * @name: The GOAD ID of the server we want to remove from the name server registration.
 * @kind: "object" for normal use.
 * @ev: CORBA_Environment to return errors
 *
 * Removes the registration of @server in the @name_server.
 *
 * Returns zero upon success, non-zero on error.
 */
int
goad_server_unregister(CORBA_Object name_server,
		       const char* name,
		       const char* kind,
		       CORBA_Environment* ev)
{
  CosNaming_NameComponent nc[3] = {{"GNOME", "subcontext"},
				   {"Servers", "subcontext"}};
  CosNaming_Name          nom;
  CORBA_Object orig_ns = name_server;

  nom._maximum = 0;
  nom._length = 3;
  nom._buffer = nc;
  nom._release = CORBA_FALSE;

  if(name_server == CORBA_OBJECT_NIL)
    name_server = gnome_name_service_get();

  g_assert(name_server != CORBA_OBJECT_NIL);

  nc[2].id   = (char *)name;
  if (!kind)
    kind = "object";
  nc[2].kind = (char *)kind;

  CosNaming_NamingContext_unbind(name_server, &nom, ev);
  if (ev->_major != CORBA_NO_EXCEPTION) {
#if 0
    switch( ev->_major ) {
    case CORBA_SYSTEM_EXCEPTION:
      g_warning("sysex: %s.\n", CORBA_exception_id(ev));
    case CORBA_USER_EXCEPTION:
      g_warning( "usr	ex: %s.\n", CORBA_exception_id(ev));
    default:
      break;
    }
#endif
    if(orig_ns == CORBA_OBJECT_NIL)
      CORBA_Object_release(name_server, ev);

    return -1;
  }

  if(orig_ns == CORBA_OBJECT_NIL)
    CORBA_Object_release(name_server, ev);

  CORBA_exception_free(ev);
  return 0;
}

/**
 * goad_server_activation_id:
 *
 * When an application that implements a GOAD-registered object is
 * started, it should call this function to check if it was started to create one of those
 * objects.
 *
 * Returns the GOAD ID that the program was executed to make available.
 */
const char *
goad_server_activation_id(void)
{
  return goad_activation_id;
}

/**
 * goad_register_arguments:
 *
 * Internal gnome function.  Used to register the arguments for the
 * popt parser.
 */
void
goad_register_arguments(void)
{
	static const struct poptOption options[] = {
		{"activate-goad-server", '\0', POPT_ARG_STRING, &goad_activation_id, 0,
		 N_("(Internal use only) GOAD server ID to activate"), "GOAD_ID"},
		{"goad-fd", '\0', POPT_ARG_INT, &goad_fd, 0,
		 N_("(Internal use only) GOAD file descriptor"), "GOAD_FD"},
		{NULL, '\0', 0, NULL, 0}
	};
	
	gnomelib_register_popt_table(options, _("Gnome Object Activation Directory"));
}
