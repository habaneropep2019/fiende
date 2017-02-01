/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 */

#include <config.h>
#include "medusa-authenticate.h"

#include <fcntl.h>
#include <glib.h>
#include <grp.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "medusa-search-service-private.h"
#include <libmedusa/medusa-file-info-utilities.h>
#include <libmedusa-internal/medusa-conf.h>


#define   CACHED_PERMISSION_NOT_CACHED 0
#define   CACHED_PERMISSION_CAN_READ 1
#define   CACHED_PERMISSION_CANNOT_READ 2

typedef struct {
        int uid;
        int pid;
        int random_key;
} UserKey;

GList *user_keys;

static int             get_user_key                              (int uid, int pid);


static gboolean        calculate_permissions_for_file            (const char *file_name, 
                                                                  int uid);

static gboolean        can_read_this_file                        (const char *directory, 

                                                                 int uid, 
                                                                  GHashTable *cache);

void
medusa_authenticate_create_cookie_file (char *request_line)
{
        int uid, pid;
        int cookie_fd;
        char cookie_file[FILE_NAME_MAXIMUM_LENGTH];
        int key;
        int result_of_mkdir_cookie_path;
  
        request_line = strchr (request_line, '\t');
        g_return_if_fail (request_line != NULL);
  
        uid = (int) strtol (request_line, NULL, 10);
  
        request_line++;
        request_line = strchr (request_line, '\t');
        g_return_if_fail (request_line != NULL);

        pid = (int) strtol (request_line, NULL, 10);
#ifdef SEARCH_DAEMON_DEBUG
        printf ("Creating cooking file with uid %d and pid %d\n", uid, pid);
#endif

        /* Make cookie path if it isn't there */
        if (access (COOKIE_PATH, F_OK) != 0) {
                result_of_mkdir_cookie_path = mkdir (COOKIE_PATH, 0777);
                g_return_if_fail (result_of_mkdir_cookie_path != -1);
        }
        g_return_if_fail (access (COOKIE_PATH, R_OK | W_OK) == 0);
  
        sprintf (cookie_file, "%s/%d_%d", COOKIE_PATH, uid, pid);
  
        cookie_fd = open (cookie_file, O_WRONLY | O_CREAT, 
                          S_IRUSR | S_IWUSR);
        g_return_if_fail (cookie_fd != -1);
        key = get_user_key (uid, pid);
        
        if (write (cookie_fd, &key, sizeof (int)) == -1) {
                g_warning ("Authenticate write failed\n");
        }
        if (fchown (cookie_fd, uid, -1) == -1) {
                g_warning ("Changing the owner of the cookie file failed\n");
        }
        if (close (cookie_fd) == -1) {
                g_warning ("Couldn't close the cookie file\n");
        }
  
}


gboolean
medusa_authenticate_is_cookie_request (char *request_line)
{
        return strncmp (request_line, COOKIE_REQUEST, strlen (COOKIE_REQUEST)) == 0;
}


gboolean 
medusa_authenticate_is_correct_cookie (int uid, int pid, int cookie)
{
        return cookie == get_user_key (uid, pid);
}



gboolean
medusa_authenticate_user_can_see_file (char *uri, int uid, GHashTable *cache)
{
        char *path_name;
        char *location, *next_directory;
        gboolean file_still_exists;


        g_return_val_if_fail (uri != NULL, FALSE);

        /* The superuser can read any file on the system */
        if (uid == 0) {
                return TRUE;
        }

        path_name = gnome_vfs_get_local_path_from_uri (uri);
        g_return_val_if_fail (path_name != NULL, FALSE);

        if (can_read_this_file ("/", uid, cache) == FALSE) {
                return FALSE;
        }
        
        location = path_name + 1;
        /* Check that each parent directory is readable by the user, starting from the top */
        while (location != NULL) {
                if (strchr (location, '/')) {
                        next_directory = g_strndup (path_name, strchr (location, '/') - path_name);
                        location = strchr (location, '/') + 1;
                }
                else {
                        break;
                }

#ifdef AUTHENTICATION_DEBUG                
                g_print ("Checking permissions on %s\n", next_directory);
#endif
                if (can_read_this_file (next_directory, uid, cache) == FALSE) {
                        g_free (next_directory);
                        g_free (path_name);
                        return FALSE;
                }
                g_free (next_directory);
        }

        file_still_exists = (access (path_name, F_OK) == 0);
        g_free (path_name);
        return file_still_exists;
}


static gboolean
can_read_this_file (const char *directory, int uid, GHashTable *cache)
{
        int cached_value;
        gboolean user_has_read_permissions_here;
#ifdef AUTHENTICATION_DEBUG  
        printf ("trying to locate cached permissions for %s\n", directory);
#endif 
        cached_value = GPOINTER_TO_INT (g_hash_table_lookup (cache, directory));
        switch (cached_value) {
                
        case CACHED_PERMISSION_NOT_CACHED:
                /* directory hasn't been cached yet */
#ifdef AUTHENTICATION_DEBUG
                printf ("Permissions not cached yet , calculating them myself \n");
#endif
                user_has_read_permissions_here = calculate_permissions_for_file (directory, uid);
                if (user_has_read_permissions_here) {
#ifdef AUTHENTICATION_DEBUG
                        printf ("user can read %s\n", directory);
#endif
                        g_hash_table_insert (cache,
                                             g_strdup (directory),
                                             GINT_TO_POINTER (CACHED_PERMISSION_CAN_READ));
                        return TRUE;
                }
                else {
#ifdef AUTHENTICATION_DEBUG
                        printf ("user can't read %s\n", directory);
#endif
                        g_hash_table_insert (cache,
                                             g_strdup (directory),
                                             GINT_TO_POINTER (CACHED_PERMISSION_CANNOT_READ));
                        return FALSE;
                }
                break;
        case CACHED_PERMISSION_CAN_READ:
#ifdef AUTHENTICATION_DEBUG
                printf ("Permissions cached as positive\n");
#endif
                return TRUE;
        case CACHED_PERMISSION_CANNOT_READ:
#ifdef AUTHENTICATION_DEBUG
                printf ("permissions cached as negative\n");
#endif
                return FALSE;
        default:
                g_assert_not_reached ();
        }
        return FALSE;
}

static gboolean
calculate_permissions_for_file (const char *file_name, int uid)
{
        int stat_result;
        struct stat file_stats;

        stat_result = lstat (file_name, &file_stats);
        /* If we can't stat the file, it's been erased or changed since the
           last indexing; so let's not consider it a result */
        if (stat_result == -1) {
                return FALSE;
        }
#ifdef AUTHENTICATION_DEBUG
        printf ("Calculating permissions for %s\n", file_name);
#endif
        if (uid == 0) {
                return TRUE;
        }
        if (file_stats.st_uid == uid) {
                if (file_stats.st_mode & S_IRUSR) {
                        return TRUE;
                }
                else {
                        return FALSE;
                }
        }
        if (medusa_group_contains (file_stats.st_gid, uid)) {
                if (file_stats.st_mode & S_IRGRP) {
                        return TRUE;
                }
                else {
                        return FALSE;
                }
        }
        if (file_stats.st_mode & S_IROTH) {
                return TRUE;
        }
        else {
                return FALSE;
        }
  
}

static int
get_user_key (int uid, int pid)
{
        GList *keys;
        UserKey *new_key;
  
        for (keys = user_keys; keys != NULL; keys = keys->next) {
                if (((UserKey *) keys->data)->uid == uid && ((UserKey *) keys->data)->pid == pid) {
                        return (((UserKey *) keys->data)->random_key);
                }
        }
        new_key = g_new0 (UserKey, 1);
        new_key->uid = uid;
        new_key->pid = pid;
        new_key->random_key = random();
        user_keys = g_list_prepend (user_keys, new_key);
        return new_key->random_key;
}

