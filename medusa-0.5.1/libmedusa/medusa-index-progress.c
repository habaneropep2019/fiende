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
 *
 *  medusa-index-progress.c -- Structures that keeps track of the
 *  total amount of disk space there is to index (or search)
 *  and reports back through a callback how much has been done.
 */

#include <config.h>
#include <fcntl.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_GETMNTINFO
# undef MAX
# undef MIN
# include <sys/param.h>
# include <sys/ucred.h>
# include <sys/mount.h>
#elif defined(HAVE_SYS_MNTTAB_H)
# include <stdio.h>
# include <sys/mnttab.h>
#else
#if defined(HAVE_GETMNTENT)
# include <mntent.h>
#else
#warning "Can't find a valid mount function type"
#endif
#endif

#ifdef HAVE_STATVFS
# include <sys/statvfs.h>
#elif HAVE_STATFS
# include <sys/statfs.h>
#else
# warning "Medusa will not be able to record your indexing progress because you don't have either statfs or statvfs."
#endif

/* FIXME: What is this for? */
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

#include "medusa-index-progress.h"
#include "medusa-index-service.h"
#include "medusa-log.h"

/* FIXME: The index progress file should be specific per 
   named index */
#define INDEX_PROGRESS_FILE_NAME "/tmp/index-progress"

struct MedusaIndexProgress {
        gboolean index_progress_is_valid;
        size_t total_blocks_to_index;
        size_t block_size;
        size_t blocks_indexed_so_far;
        int percent_complete;
        int output_fd;
};

static gboolean             get_disk_used_information                  (char *root_directory,
                                                                        size_t *total_blocks_to_index,
                                                                        size_t *block_size);
static gboolean             update_new_public_percent_complete         (MedusaIndexProgress *progress,
                                                                        int new_percent_complete);


static void
clean_up_index_progress_file (void)
{
        int unlink_result;
        
        unlink_result = unlink (INDEX_PROGRESS_FILE_NAME);
        if (unlink_result != 0) {
                medusa_log_error ("Removing the index progress file failed.  "
                                  "Medusa may report that indexing is in progress when "
                                  "no indexing is occurring due to this problem");
        }
}

static void
clean_up_index_progress_file_at_exit (void)
{
        static gboolean at_exit_is_set_up = FALSE;

        if (!at_exit_is_set_up) {
                g_atexit (clean_up_index_progress_file);
                at_exit_is_set_up = TRUE;
        }
}

MedusaIndexProgress *         
medusa_index_progress_new (char *root_directory)
{
        size_t total_blocks_to_index, block_size;
        int output_fd;
        MedusaIndexProgress *index_progress;
        gboolean disk_information_was_found;

        g_print ("starting index progress\n");
        disk_information_was_found = get_disk_used_information (root_directory, 
                                                                &total_blocks_to_index, 
                                                                &block_size);
        if (!disk_information_was_found) {
                /* FIXME: Log error here */
                return NULL;
        }
        output_fd = open (INDEX_PROGRESS_FILE_NAME, O_RDWR | O_CREAT);
        if (output_fd == -1) {
                medusa_log_error ("Opening the index progress file for writing failed");
                return NULL;
        }
        clean_up_index_progress_file_at_exit ();
        
        /* FIXME: Check if this fails? */
        chmod (INDEX_PROGRESS_FILE_NAME, S_IRWXU | S_IRGRP | S_IROTH);

        index_progress = g_new0 (MedusaIndexProgress, 1);
        index_progress->total_blocks_to_index = total_blocks_to_index;
        index_progress->block_size = block_size;
        if (index_progress->total_blocks_to_index > 0) {
                index_progress->index_progress_is_valid = TRUE;
        }
        else {
                g_warning ("Can't find the amount of disk space on your drive.  We won't be recording index progress variables\n");
                index_progress->index_progress_is_valid = FALSE;
        }
        index_progress->blocks_indexed_so_far = 0;
        index_progress->output_fd = output_fd;
        index_progress->percent_complete = 0;

        if (update_new_public_percent_complete (index_progress, index_progress->percent_complete) == FALSE) {
                medusa_index_progress_destroy (index_progress);
                return NULL;
        }
        return index_progress;
}


void
medusa_index_progress_update (MedusaIndexProgress *progress,
                              size_t new_bytes_indexed)
{
        int new_percent_complete;

        if (progress == NULL) {
                return;
        }
        
        /* Don't calculate progress if we couldn't establish a total amount of space to index */
        if (!progress->index_progress_is_valid) {
                return;
        }

        if (new_bytes_indexed % progress->block_size) {
                progress->blocks_indexed_so_far += new_bytes_indexed / progress->block_size + 1;
        }
        else {
                progress->blocks_indexed_so_far += new_bytes_indexed / progress->block_size;
        }
                
        new_percent_complete = progress->blocks_indexed_so_far * 100 / progress->total_blocks_to_index;
        /* Don't update progress above 100% */
        if (new_percent_complete > 100) {
                return;
        }
        if (new_percent_complete > progress->percent_complete) {
                if (update_new_public_percent_complete (progress, 
                                                        new_percent_complete) == FALSE) {
                        g_warning ("Could not update progress successfully\n");
                }
                progress->percent_complete = new_percent_complete;
        }
}

void
medusa_index_progress_destroy (MedusaIndexProgress *progress)
{
        if (progress == NULL) {
                return;
        }
        close (progress->output_fd);
        g_free (progress);
}
                                     

gboolean
medusa_indexing_is_currently_in_progress (void)
{
        return access (INDEX_PROGRESS_FILE_NAME, F_OK) == 0;
}
          
int
medusa_index_progress_get_percentage_complete (void)
{
        int progress_fd;
        int percentage_complete;
        int read_result;

        progress_fd = open (INDEX_PROGRESS_FILE_NAME, O_RDONLY);
        if (progress_fd == -1) {
                return 0;
        }

        read_result = read (progress_fd, &percentage_complete, sizeof (int));
        if (read_result < sizeof (int)) {
                return 0;
        }
        close (progress_fd);

        return percentage_complete;
}


static gboolean
get_disk_used_information (char *root_directory,
                           size_t *total_blocks_to_index,
                           size_t *block_size)
{
#ifdef HAVE_STATVFS
        struct statvfs stat_buffer;
#elif defined(HAVE_STATFS)
        struct statfs stat_buffer;
#endif
        int stat_result;
       
        g_print ("Root directory is %s\n", root_directory);
#ifdef HAVE_STATVFS
        stat_result = statvfs (root_directory, &stat_buffer);
#elif defined(HAVE_STATFS)
        stat_result = statfs (root_directory, &stat_buffer);
#else
        stat_result = -1;
#endif

        /* Don't return disk used information for directories that are not
           the root of file systems */
        if (stat_result != 0) {
                /* FIXME bugzilla.eazel.com 4557: log that we are doing this */
                return FALSE;
        }
        *block_size = stat_buffer.f_bsize; 
        *total_blocks_to_index = stat_buffer.f_blocks;
        return TRUE;
}

static gboolean           
update_new_public_percent_complete (MedusaIndexProgress *progress,
                                    int new_percent_complete)
{
        int lseek_result, write_result;
        /*        const char *new_line = "\n"; */

        lseek_result = lseek (progress->output_fd, 0, SEEK_SET);
        if (lseek_result != 0) {
                return FALSE;
        }
        write_result = write (progress->output_fd, &new_percent_complete, sizeof (int));
        if (write_result != sizeof (int)) {
                return FALSE;
        }
        /* write_result = write (progress->output_fd, new_line, sizeof (char) * strlen (new_line));
        if (write_result != strlen (new_line) * sizeof (char)) {
                return FALSE;
                }*/
        return TRUE;
}
