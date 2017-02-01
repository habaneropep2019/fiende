/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000, 2001 Eazel, Inc.
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
 *
 *  medusa-unsearched-locations.c -- Way to specify files and directories to
 *  skip, like nfs mounts, removeable media, and other files we specifically aren't
 *  interested in.
 */

#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "medusa-log.h"
#include "medusa-utils.h"

#ifdef HAVE_GETMNTINFO
# undef MAX
# undef MIN
# include <sys/param.h>
# include <sys/ucred.h>
# include <sys/mount.h>
#elif (HAVE_SYS_MNTTAB_H)
#include <sys/mnttab.h>
#else
#if defined (HAVE_GETMNTENT)
# include <mntent.h>
#else
#warning "Can't find a valid mount function type"
#endif
#endif

#include <glib.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "medusa-unsearched-locations.h"

#define CDROM_DRIVE_STATUS	0x5326  /* Get tray position, etc. */
#define CDSL_CURRENT    	((int) (~0U>>1))
#define STOPLIST_FILE_NAME      MEDUSA_SYSCONFDIR "/file-index-stoplist"
#define STOPLIST_MOUNT_TYPE     MEDUSA_SYSCONFDIR "/mount-type-stoplist"

static void                 file_stoplist_initialize                          (void);
static void                 unsearched_mount_list_initialize                  (void);
static void                 medusa_unsearched_locations_shutdown              (void);

#ifdef HAVE_GETMNTINFO
static gboolean             mount_point_is_cdrom                              (const struct statfs *fs_list);
#elif HAVE_SYS_MNTTAB_H
static  gboolean            mount_point_is_cdrom                              (const struct mnttab *mnttab_entry);
#elif defined(HAVE_GETMNTENT)
static gboolean             mount_point_is_cdrom                              (const struct mntent *mount_entry);
#endif

static void                 unsearched_location_free                          (gpointer key,
                                                                               gpointer value,
                                                                               gpointer user_data);

static GHashTable *unsearched_locations = NULL;

static GList *mount_type_skip_list = NULL;



static gboolean
mount_type_is_in_skip_list (const char *mount_type)
{
        GList *current_item;
        char *current_string;
       
        g_return_val_if_fail (mount_type != NULL, TRUE);

        for ( current_item = mount_type_skip_list ;
              current_item != NULL ;
              current_item = g_list_next (current_item)) {

		current_string = (char *)current_item->data; 
                if (strcmp (current_string, mount_type) == 0) {
                        return TRUE;
                }
        }

        return FALSE;
}

gboolean
medusa_is_unsearched_location (const char *file_name)
{
        g_return_val_if_fail (unsearched_locations != NULL, FALSE);

        if (g_hash_table_lookup (unsearched_locations, file_name) == NULL) {
                return FALSE;
        }
        else {
                return TRUE;
        }
}

void
medusa_unsearched_locations_initialize (void)
{
        static gboolean unsearched_locations_initialized = FALSE;

        if (!unsearched_locations_initialized) {
                unsearched_locations = g_hash_table_new (g_str_hash,
                                                         g_str_equal);
                
                file_stoplist_initialize ();
                unsearched_mount_list_initialize ();
                
                g_atexit (medusa_unsearched_locations_shutdown);
                
                unsearched_locations_initialized = TRUE;
        }
}

static void
medusa_unsearched_locations_shutdown (void)
{
	GList *list_item;

        if (unsearched_locations) {
                g_hash_table_foreach (unsearched_locations,
                                      unsearched_location_free,
                                      NULL);
                
                g_hash_table_destroy (unsearched_locations);
                unsearched_locations = NULL;
        }

        for ( list_item = mount_type_skip_list ;
              list_item != NULL ;
              list_item = g_list_next (list_item)) {

		g_free (list_item->data);
		list_item->data = NULL;
	}
	g_list_free (mount_type_skip_list);
        mount_type_skip_list = NULL;
}

#define GETLINE_INITIAL_BUFFER_SIZE 256

/**
 * getline_dup
 *
 * reads newline (or CR) or EOF terminated line from stream, allocating the return
 * buffer as appropriate
 **/
/* FIXME bugzilla.eazel.com 5055: Belongs in a library, not here. */
/* (it is also present in nautilus/src/nautilus-first-time-druid.c) */
static char * 
getline_dup (FILE* stream)
{
	char *ret;
	size_t ret_size;
	size_t ret_used;
	int char_read;
	gboolean done;

	ret = g_malloc (GETLINE_INITIAL_BUFFER_SIZE);
	ret_size = GETLINE_INITIAL_BUFFER_SIZE;

	for ( ret_used = 0, done = FALSE ;
	      !done && (EOF != (char_read = fgetc (stream))) ; 
	      ret_used++) {
		
		if (ret_size == (ret_used + 1)) {
			ret_size *= 2;
			ret = g_realloc (ret, ret_size); 
		}
		if ('\n' == char_read || '\r' == char_read ) {
			done = TRUE;
			ret [ret_used] = '\0';
		} else {
			ret [ret_used] = char_read;
		}
	}

	if (ret_used == 0) {
		g_free (ret);
		ret = NULL;
	} else {
		ret [ret_used] = '\0';
	}

	return ret;
}


static void
file_stoplist_initialize  (void)
{
	FILE *stop_list_handle;
	char *next_file;

        g_return_if_fail (unsearched_locations != NULL);

	stop_list_handle = fopen (STOPLIST_FILE_NAME, "r");
        if (stop_list_handle == NULL) {
                medusa_log_error ("Couldn't find stop list of files and directories to be ignored.  All files and directories on the system will be considered.");
                return;
        }

	while (NULL != (next_file = getline_dup (stop_list_handle))) {
                /* Skip comment lines */
                if (next_file[0] == '#') {
                	g_free (next_file);
                	next_file = NULL;
                        continue;
                }
                g_hash_table_insert (unsearched_locations,
                                     next_file,
                                     GINT_TO_POINTER (1));
                next_file = NULL;
	}
	fclose (stop_list_handle);
        
}

static void
load_mount_type_skip_list()
{
	FILE *file_handle;
	char *line;

	g_return_if_fail (mount_type_skip_list == NULL);

	file_handle = fopen (STOPLIST_MOUNT_TYPE, "r");

	if (file_handle == NULL) {
		return;
	}

	while (NULL != (line = getline_dup (file_handle))) {
                /* Skip comment lines */
                if (line[0] == '#') {
                	g_free (line);
                	line = NULL;
                        continue;
                }

                mount_type_skip_list = g_list_prepend (mount_type_skip_list, line);
                line = NULL;
	}
	fclose (file_handle);
}

static void
unsearched_mount_list_initialize_internal (void)
{

#ifdef HAVE_GETMNTINFO

	struct statfs *fs_list;
	int fs_count, i;

	fs_count = getmntinfo (&fs_list, MNT_LOCAL); /* returns the number of FSes. */
	for (i = 0; i < fs_count; ++i) {
		if (mount_type_is_in_skip_list (fs_list[i].f_fstypename)) {
			g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (fs_list[i].f_mntonname),
                                             GINT_TO_POINTER (1));
		}
                if (mount_point_is_cdrom (&fs_list[i])) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (fs_list[i].f_mntonname),
                                             GINT_TO_POINTER (1));
                }
        }

#elif HAVE_SYS_MNTTAB_H

        FILE *mount_file;
        struct mnttab *mnttab_entry;

        mount_file = fopen ("/etc/mnttab", "r");
        while (getmntent (mount_file, mnttab_entry) != 0) {
                if (mount_type_is_in_skip_list (mnttab_entry->mnt_fstype)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mnttab_entry->mnt_mountp),
                                             GINT_TO_POINTER (1));
                }
                if (mount_point_is_cdrom (mnttab_entry)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mnttab_entry->mnt_mountp),
                                             GINT_TO_POINTER (1));
                }
        }
#ifdef HAVE_ENDMNTENT
        endmntent (mount_file);
#else
        fclose (mount_file);
#endif

#elif defined(HAVE_GETMNTENT)

        FILE *mount_file;
        struct mntent *mount_entry;
        
        mount_file = setmntent (MOUNTED, "r");
        while ((mount_entry = getmntent (mount_file)) != NULL) {
                if (mount_type_is_in_skip_list (mount_entry->mnt_type)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mount_entry->mnt_dir),
                                             GINT_TO_POINTER (1));
                }
                if (mount_point_is_cdrom (mount_entry)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mount_entry->mnt_dir),
                                             GINT_TO_POINTER (1));
                }
        }
	endmntent (mount_file);
        g_free (mount_entry);

#endif

}


/* Don't index or search nfs mount points and removeable media, for now */
static void              
unsearched_mount_list_initialize (void)
{
	load_mount_type_skip_list ();
	unsearched_mount_list_initialize_internal ();
}

static gboolean
mount_point_is_cdrom_internal (const char *path)
{
	int fd;
        
        fd = open (path, O_RDONLY | O_NONBLOCK);
        
        /* These tests are shamelessly stolen from Gene Ragan's
         * nautilus-volume-monitor code in the nautilus module.
         */
        
        if (fd < 0) {
                return FALSE;
        }
        
	if (ioctl (fd, CDROM_DRIVE_STATUS, CDSL_CURRENT) < 0) {
                close (fd);
                return FALSE;
        }
        
        close (fd);
        
        return TRUE;
}

#ifdef HAVE_GETMNTINFO        

static gboolean             
mount_point_is_cdrom (const struct statfs *fs_list)
{
        int fd;

        if (strcmp (fs_list->f_fstypename, "cdrom")) {
                return FALSE;
        }
        return mount_point_is_cdrom_internal (fs_list->f_mntonname);
}

#elif HAVE_SYS_MNTTAB_H

static gboolean
mount_point_is_cdrom (const struct mnttab *mnttab_entry)
{
        if (strcmp (mnttab_entry->mnt_fstype, "iso9660")) {
                return FALSE;
        }
        return mount_point_is_cdrom_internal (mnttab_entry->mnt_mountp);
}

#elif defined(HAVE_GETMNTENT)

static gboolean
mount_point_is_cdrom (const struct mntent *mount_entry)
{
        if (strcmp (mount_entry->mnt_type, "iso9660")) {
                return FALSE;
        }

        return mount_point_is_cdrom_internal (mount_entry->mnt_fsname);
}

#endif

static void                 
unsearched_location_free (gpointer key,
                          gpointer value,
                          gpointer user_data)
{
        char *unsearched_location;

        g_return_if_fail (key != NULL);

        unsearched_location = (char *) key;
        g_free (unsearched_location);
}
