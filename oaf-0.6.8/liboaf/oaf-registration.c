/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000, 2001 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

/* This is part of the per-app CORBA bootstrapping - we use this to get 
   hold of a running metaserver and such */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <string.h>

#include "liboaf-private.h"
#include "oaf-i18n.h"
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

static GSList *registries = NULL;

typedef struct
{
	int priority;
	const OAFBaseServiceRegistry *registry;
	gpointer user_data;
}
RegistryInfo;

typedef struct
{
	int priority;
	OAFBaseServiceActivator activator;
}
ActivatorInfo;

static gint
ri_compare (gconstpointer a, gconstpointer b)
{
	const RegistryInfo *ra, *rb;

	ra = a;
	rb = b;

	return (rb->priority - ra->priority);
}

void
oaf_registration_location_add (const OAFBaseServiceRegistry *registry,
			       int priority, gpointer user_data)
{
	RegistryInfo *new_ri;

	g_return_if_fail (registry);

	new_ri = g_new (RegistryInfo, 1);
	new_ri->priority = priority;
	new_ri->registry = registry;
	new_ri->user_data = user_data;

	registries = g_slist_insert_sorted (registries, new_ri, ri_compare);
}

CORBA_Object
oaf_registration_check (const OAFBaseService *base_service,
			CORBA_Environment *ev)
{
	GSList *link;
	CORBA_Object retval = CORBA_OBJECT_NIL;
	int dist = INT_MAX;
	char *ior = NULL;

	for (link = registries; link; link = link->next) {
		RegistryInfo *ri;
		char *new_ior;
		int new_dist = dist;

		ri = link->data;

		if (!ri->registry->check)
			continue;

		new_ior = ri->registry->check (ri->registry, base_service, 
                                             &new_dist, ri->user_data);
		if (new_ior && (new_dist < dist)) {
			g_free (ior);
			ior = new_ior;
		} else if (new_ior) {
			g_free (new_ior);
		}
	}

	if (ior) {
		retval = CORBA_ORB_string_to_object (oaf_orb_get (), ior, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			retval = CORBA_OBJECT_NIL;

		g_free (ior);
	}

	return retval;
}

/*dumb marshalling hack */
static void
oaf_registration_iterate (const OAFBaseService *base_service,
			  CORBA_Object obj, CORBA_Environment *ev,
			  gulong offset, int nargs)
{
	GSList *link;
	char *ior = NULL;

	if (nargs == 4)
		ior = CORBA_ORB_object_to_string (oaf_orb_get (), obj, ev);

	for (link = registries; link; link = link->next) {
		RegistryInfo *ri;
		void (*func_ptr) ();

		ri = link->data;

		func_ptr = *(gpointer *) ((guchar *) ri->registry + offset);

		if (!func_ptr)
			continue;

		switch (nargs) {
		case 4:
			func_ptr (ri->registry, ior, base_service, ri->user_data);
			break;
		case 2:
			func_ptr (ri->registry, ri->user_data);
			break;
		}
	}

	if (nargs == 4)
		CORBA_free (ior);
}

static int lock_count = 0;

static void
oaf_registries_lock (CORBA_Environment *ev)
{
	if (lock_count == 0)
		oaf_registration_iterate (NULL, CORBA_OBJECT_NIL, ev,
					  G_STRUCT_OFFSET
					  (OAFBaseServiceRegistry, lock), 2);
	lock_count++;
}

static void
oaf_registries_unlock (CORBA_Environment *ev)
{
	lock_count--;
	if (lock_count == 0)
		oaf_registration_iterate (NULL, CORBA_OBJECT_NIL, ev,
					  G_STRUCT_OFFSET
					  (OAFBaseServiceRegistry, unlock),
					  2);
}

void
oaf_registration_unset (const OAFBaseService *base_service,
			CORBA_Object obj, CORBA_Environment *ev)
{
	oaf_registries_lock (ev);
	oaf_registration_iterate (base_service, obj, ev,
				  G_STRUCT_OFFSET (OAFBaseServiceRegistry,
						   unregister), 4);
	oaf_registries_unlock (ev);
}

void
oaf_registration_set (const OAFBaseService *base_service,
		      CORBA_Object obj, CORBA_Environment *ev)
{
	oaf_registries_lock (ev);
	oaf_registration_iterate (base_service, obj, ev,
				  G_STRUCT_OFFSET (OAFBaseServiceRegistry,
						   register_new), 4);
	oaf_registries_unlock (ev);
}


const char *oaf_ac_cmd[] =
	{ "oafd", "--ac-activate", "--ior-output-fd=%d", NULL };
const char *oaf_od_cmd[] = { "oafd", "--ior-output-fd=%d", NULL };

struct SysServerInstance
{
	CORBA_Object already_running;
	char *username, *hostname, *domain;
};

struct SysServer
{
	const char *name;
	const char **cmd;
	int fd_arg;
	GSList *instances;
}
activatable_servers[] =
{
	{"IDL:OAF/ActivationContext:1.0", (const char **) oaf_ac_cmd,
         2, CORBA_OBJECT_NIL}, 
        {"IDL:OAF/ObjectDirectory:1.0", (const char **) oaf_od_cmd,
         1, CORBA_OBJECT_NIL},
	{ NULL}
};

#define STRMATCH(x, y) ((!x && !y) || (x && y && !strcmp(x, y)))
static CORBA_Object
existing_check (const OAFBaseService *base_service, struct SysServer *ss)
{
	GSList *link;

	for (link = ss->instances; link; link = link->next) {
		struct SysServerInstance *ssi;

		ssi = link->data;
		if (
		    (!ssi->username
		     || STRMATCH (ssi->username, base_service->username))
		    && (!ssi->hostname
			|| STRMATCH (ssi->hostname, base_service->hostname))
		    && (!ssi->domain
			|| STRMATCH (ssi->domain,
				     base_service->
				     domain))) {
                        return ssi->already_running;
                }
	}

	return CORBA_OBJECT_NIL;
}

static void
oaf_existing_set (const OAFBaseService *base_service, struct SysServer *ss,
	          CORBA_Object obj, CORBA_Environment *ev)
{
	GSList *link;
	struct SysServerInstance *ssi;

        ssi = NULL;

	for (link = ss->instances; link; link = link->next) {
		ssi = link->data;
		if (
		    (!ssi->username
		     || STRMATCH (ssi->username, base_service->username))
		    && (!ssi->hostname
			|| STRMATCH (ssi->hostname, base_service->hostname))
		    && (!ssi->domain
			|| STRMATCH (ssi->domain, base_service->domain))) break;
	}

	if (link == NULL) {
		ssi = g_new0 (struct SysServerInstance, 1);
		ssi->already_running = obj;
		ssi->username =
			base_service->username ? g_strdup (base_service->username) : NULL;
		ssi->hostname =
			base_service->hostname ? g_strdup (base_service->hostname) : NULL;
		ssi->domain =
			base_service->domain ? g_strdup (base_service->domain) : NULL;
                ss->instances = g_slist_prepend (ss->instances, ssi);
	} else {
		CORBA_Object_release (ssi->already_running, ev);
		ssi->already_running = obj;
	}
}

static GSList *activator_list = NULL;

static gint
ai_compare (gconstpointer a, gconstpointer b)
{
	const ActivatorInfo *ra, *rb;

	ra = a;
	rb = b;

	return (rb->priority - ra->priority);
}

void
oaf_registration_activator_add (OAFBaseServiceActivator activator, 
                                int priority)
{
	ActivatorInfo *new_act;

	new_act = g_new (ActivatorInfo, 1);
	new_act->priority = priority;
	new_act->activator = activator;
	activator_list =
		g_slist_insert_sorted (activator_list, new_act, ai_compare);
}

static CORBA_Object
oaf_activators_use (const OAFBaseService *base_service, const char **cmd,
		    int fd_arg, CORBA_Environment *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	GSList *link;

	for (link = activator_list; CORBA_Object_is_nil (retval, ev) && link;
	     link = link->next) {
		ActivatorInfo *actinfo;
		actinfo = link->data;

		retval = actinfo->activator (base_service, cmd, fd_arg, ev);
	}

	return retval;
}

static CORBA_Object
oaf_service_get_internal (const OAFBaseService *base_service,
                          gboolean              existing_only,
                          CORBA_Environment    *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	int i;
	CORBA_Environment myev, important_error_ev;
	gboolean ne;

	g_return_val_if_fail (base_service, CORBA_OBJECT_NIL);

	for (i = 0; activatable_servers[i].name; i++) {
		if (!strcmp (base_service->name, activatable_servers[i].name))
			break;
	}

	if (!activatable_servers[i].name)
		return retval;

	CORBA_exception_init (&myev);
        CORBA_exception_init (&important_error_ev);
        
	retval = existing_check (base_service, &activatable_servers[i]);
	if (!CORBA_Object_non_existent (retval, ev))
		goto out;

	oaf_registries_lock (ev);

	retval = oaf_registration_check (base_service, &myev);
	ne = CORBA_Object_non_existent (retval, &myev);
	if (ne && !existing_only) {
		CORBA_Object race_condition;

		CORBA_Object_release (retval, &myev);
                
		retval =
			oaf_activators_use (base_service,
					    activatable_servers[i].cmd,
					    activatable_servers[i].fd_arg,
					    &important_error_ev);

		race_condition = oaf_registration_check (base_service, &myev);

		if (!CORBA_Object_non_existent (race_condition, &myev)) {
			CORBA_Object_release (retval, &myev);
			retval = race_condition;
		} else if (!CORBA_Object_is_nil (retval, &myev)) {
			oaf_registration_set (base_service, retval, &myev);
                }
	}

	oaf_registries_unlock (ev);

	if (!CORBA_Object_non_existent (retval, ev))
		oaf_existing_set (base_service, &activatable_servers[i], retval, ev);

      out:
        /* If we overwrote ev with some stupid junk, replace
         * it with the real error
         */
        if (important_error_ev._major != CORBA_NO_EXCEPTION) {
                CORBA_exception_free (ev);
                /* This transfers memory ownership */
                *ev = important_error_ev;
        }
        
        CORBA_exception_free (&myev);

	return retval;
}

CORBA_Object
oaf_service_get (const OAFBaseService *base_service)
{
        CORBA_Environment ev;
        CORBA_Object obj;
        
        CORBA_exception_init (&ev);
        
        obj = oaf_service_get_internal (base_service, FALSE, &ev);

        CORBA_exception_free (&ev);

        return obj;
}

CORBA_Object
oaf_internal_service_get_extended (const OAFBaseService *base_service,
                                   gboolean              existing_only,
                                   CORBA_Environment    *ev)
{
        return oaf_service_get_internal (base_service, existing_only, ev);
}


/*****Implementation of the IOR registration system via plain files ******/
static int lock_fd = -1;

static void
rloc_file_lock (const OAFBaseServiceRegistry *registry, 
                gpointer user_data)
{
	char *fn;
	struct flock lock;
        int retval;
        char *err;

        fn = g_strdup_printf ("/tmp/orbit-%s/oaf-register.lock", g_get_user_name ());

	lock_fd = open (fn, O_CREAT | O_RDWR, 0700);
	fcntl (lock_fd, F_SETFD, FD_CLOEXEC);

	if (lock_fd >= 0) {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 0;
		lock.l_len = 1;
		lock.l_pid = getpid ();

		while ((retval = fcntl (lock_fd, F_SETLKW, &lock)) < 0
		       && errno == EINTR) /**/;

                if (retval != 0) {
                        /* FIXME: need to report this error in a better way. */
                        err = strerror (errno);
                        g_warning ("Failed to acquire lock: %s\n.", err);
                } 
	}

        g_free (fn);
}

static void
rloc_file_unlock (const OAFBaseServiceRegistry *registry, 
                  gpointer user_data)
{
        struct flock lock;


	if (lock_fd >= 0) {
		lock.l_type = F_UNLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 0;
		lock.l_len = 1;
		lock.l_pid = getpid ();

		fcntl (lock_fd, F_SETLKW, &lock);
		close (lock_fd);
		lock_fd = -1;
	}
}

static void
filename_fixup (char *fn)
{
	while (*(fn++)) {
		if (*fn == '/')
			*fn = '_';
	}
}

static char *
rloc_file_check (const OAFBaseServiceRegistry *registry,
		 const OAFBaseService *base_service, int *ret_distance,
		 gpointer user_data)
{
	FILE *fh;
	char *fn, *uname;
	char *namecopy;

	namecopy = g_strdup (base_service->name);
	filename_fixup (namecopy);

	uname = g_get_user_name ();

	fn = g_strdup_printf ("/tmp/orbit-%s/reg.%s-%s",
                              uname,
                              namecopy,
                              base_service->session_name ? base_service->session_name : "local");
        
	fh = fopen (fn, "r");
        g_free (fn);

        if (fh == NULL) {
                fn = g_strdup_printf ("/tmp/orbit-%s/reg.%s",
                                      uname,
                                      namecopy);
                
                fh = fopen (fn, "r");
                g_free (fn);
        }

        g_free (namecopy);

	if (fh != NULL) {
		char iorbuf[8192];

		iorbuf[0] = '\0';
		while (fgets (iorbuf, sizeof (iorbuf), fh)
		       && strncmp (iorbuf, "IOR:", 4))
			/**/;
		g_strstrip (iorbuf);

		fclose (fh);

		if (!strncmp (iorbuf, "IOR:", 4)) {
			*ret_distance = 0;
			return g_strdup (iorbuf);
		}
	}

	return NULL;
}

static void
rloc_file_register (const OAFBaseServiceRegistry *registry, const char *ior,
		    const OAFBaseService *base_service,
		    gpointer user_data)
{
	char *fn, *fn2, *uname;
	FILE *fh;
	char *namecopy;

	namecopy = g_strdup (base_service->name);
	filename_fixup (namecopy);

	uname = g_get_user_name ();

	fn = g_strdup_printf ("/tmp/orbit-%s/reg.%s-%s",
                              uname,
                              namecopy,
                              base_service->session_name ? base_service->session_name : "local");

	fn2 = g_strdup_printf ("/tmp/orbit-%s/reg.%s", uname, namecopy);
        g_free (namecopy);

	fh = fopen (fn, "w");

        if (fh != NULL) {
                fprintf (fh, "%s\n", ior);
                fclose (fh);
        }       

        symlink (fn, fn2);
        g_free (fn);
        g_free (fn2);
}

static void
rloc_file_unregister (const OAFBaseServiceRegistry *registry, 
                      const char *ior,
		      const OAFBaseService *base_service,
		      gpointer user_data)
{
	char *fn, *fn2;
        char fn3[PATH_MAX + 1];
	char *uname;
	char *namecopy;
        int link_length;

	namecopy = g_strdup (base_service->name);
	filename_fixup (namecopy);

	uname = g_get_user_name ();

	fn = g_strdup_printf ("/tmp/orbit-%s/reg.%s-%s",
                              uname,
                              namecopy,
                              base_service->session_name ? base_service->session_name : "local");
	unlink (fn);

	fn2 = g_strdup_printf ("/tmp/orbit-%s/reg.%s", uname, namecopy);

	link_length = readlink (fn2, fn3, sizeof (fn3) - 1);

        if (link_length < 0) {
		return;
        }
        
        fn3[link_length] = 0;
        
	if (strcmp (fn3, fn) == 0) {
		unlink (fn2);
        }
}

static const OAFBaseServiceRegistry rloc_file = {
	rloc_file_lock,
	rloc_file_unlock,
	rloc_file_check,
	rloc_file_register,
	rloc_file_unregister
};

void
oaf_rloc_file_register (void)
{
	oaf_registration_location_add (&rloc_file, 0, NULL);
}





