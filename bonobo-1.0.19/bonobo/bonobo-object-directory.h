/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 c-set-style: linux -*- */
/**
 * bonobo-object-directory.c: abstract the object directory
 *
 * Authors:
 *    Havoc Pennington  (hp@redhat.com)
 *    Anders Carlsson   (andersca@gnu.org)
 *    Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2000 Havoc Pennington, Anders Carlsson,
 *                      Eazel, Inc.
 */
#ifndef __BONOBO_OBJECT_DIRECTORY_H__
#define __BONOBO_OBJECT_DIRECTORY_H__

#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <orb/orbit.h>
#include <ORBitservices/CosNaming.h>

BEGIN_GNOME_DECLS

/*
 * This file used to provide a compatibility abstraction for the
 * Gnorba -> Oaf transition. This is now complete, hence these
 * functions are deprecated in favour of the oaf equivalents.
 */

typedef struct _ODServerInfo ODServerInfo;

typedef enum {
        OD_REG_SUCCESS,
        OD_REG_NOT_LISTED,
        OD_REG_ALREADY_ACTIVE,
        OD_REG_ERROR
} ODRegistrationResult;

CORBA_ORB            bonobo_directory_get_orb                     (void);

ODServerInfo        *bonobo_directory_new_server_info             (const gchar       *iid,
						     const gchar       *name,
                                                     const gchar       *desc);
const gchar         *bonobo_directory_get_server_info_id          (ODServerInfo      *info);
const gchar         *bonobo_directory_get_server_info_name        (ODServerInfo      *info);
const gchar         *bonobo_directory_get_server_info_description (ODServerInfo      *info);
void                 bonobo_directory_server_info_ref             (ODServerInfo      *info);
void                 bonobo_directory_server_info_unref           (ODServerInfo      *info);

/* returns list of ODServerInfo */
GList               *bonobo_directory_get_server_list             (const gchar      **required_ids);
void                 bonobo_directory_free_server_list            (GList             *list);
CORBA_Object         od_server_activate_with_id     (const gchar       *iid,
						     gint               flags,
                                                     CORBA_Environment *ev);
ODRegistrationResult bonobo_directory_register_server             (CORBA_Object       objref,
                                                     const gchar       *iid);
ODRegistrationResult bonobo_directory_unregister_server           (CORBA_Object       objref,
						     const gchar       *iid);

CORBA_Object         bonobo_directory_get_name_service            (CORBA_Environment *ev);

char                *bonobo_directory_find_for_file (const char  *fname,
						     const char **required_ids,
						     char       **error);

END_GNOME_DECLS

#endif /* __BONOBO_OBJECT_DIRECTORY_H__ */


