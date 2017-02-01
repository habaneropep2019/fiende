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
 *  medusa-log.c  -- API for logging errors and progress, depending on the
 *  current log level
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "medusa-log.h"
#define MEDUSA_LOG_DEBUG 1

/* FIXME: Log messages should also log the time of the event */
#define MEDUSA_LOG_FILE_PATH "/var/log/medusa"
#define MEDUSA_LOG_FILE "/var/log/medusa/log"

static FILE *log_file = NULL;
static gboolean tried_to_open_log_file = FALSE;

static MedusaLogLevel log_level;
static gboolean determined_log_level_from_environment = FALSE;


MedusaLogLevel
medusa_log_get_current_log_level (void)
{
        if (!determined_log_level_from_environment) {
                if (getenv ("MEDUSA_INDEX_DEBUG") != NULL) {
                        log_level = MEDUSA_DB_LOG_EVERYTHING;
                }
                else if (getenv ("MEDUSA_INDEX_LOG_ABBREVIATED") != NULL) {
                        log_level = MEDUSA_DB_LOG_ABBREVIATED;
                }
                else if (getenv ("MEDUSA_INDEX_LOG_NOTHING") != NULL) {
                        log_level = MEDUSA_DB_LOG_NOTHING;
                }
                else if (getenv ("MEDUSA_INDEX_LOG_TEXT_INDEX_DATA") != NULL) {
                        log_level = MEDUSA_DB_LOG_TEXT_INDEX_DATA;
                }
                else log_level = MEDUSA_DB_LOG_ERRORS;
        }
        
        return log_level;
}

static gboolean
open_log_file_if_necessary_or_fail (void)
{
        if (log_file == NULL && tried_to_open_log_file) {
                return FALSE;
        }
        if (log_file == NULL && !tried_to_open_log_file) {
                mkdir (MEDUSA_LOG_FILE_PATH, S_IRWXU);
                log_file = fopen (MEDUSA_LOG_FILE, "a");
                tried_to_open_log_file = TRUE;
                if (log_file == NULL) {
                        return FALSE;
                }
        }
        return TRUE;
}

static char *
current_time_as_string (void)
{
        time_t current_time;
        
        current_time = time (NULL);
        return asctime (localtime (&current_time));
        

}


void
medusa_log_error (const char *message)
{
        char *complete_error_message, *time_string;

        if (medusa_log_get_current_log_level () >= MEDUSA_DB_LOG_ERRORS) {
                if (!open_log_file_if_necessary_or_fail ()) {
                        return;
                }
                time_string = current_time_as_string ();
                complete_error_message = g_strdup_printf ("%s (Errno = %d) on %s", message, errno, time_string);
                fprintf (log_file, complete_error_message);
#ifdef MEDUSA_LOG_DEBUG
                printf (complete_error_message);
#endif
                g_free (complete_error_message);
        }
        
}

void
medusa_log_fatal_error (const char *message)
{
        medusa_log_error (message);
        medusa_log_error ("Above error is fatal.  Exiting now\n");
        exit (1);
}

void
medusa_log_event (const char *message,
                  MedusaLogLevel minimum_log_level)
{
        char *time_string, *complete_error_message;

        g_return_if_fail (message != NULL);
        if (medusa_log_get_current_log_level () >= minimum_log_level) {
                if (!open_log_file_if_necessary_or_fail ()) {
                        return;
                }
                time_string = current_time_as_string ();
                complete_error_message = g_strdup_printf ("%s on %s", message, time_string);
                fprintf (log_file, complete_error_message);
#ifdef MEDUSA_LOG_DEBUG
                printf (complete_error_message);
#endif
                g_free (complete_error_message);
        }
}

void
medusa_log_text_index_account_data (const char *message)
{
        if (medusa_log_get_current_log_level () == MEDUSA_DB_LOG_TEXT_INDEX_DATA) {
                if (!open_log_file_if_necessary_or_fail ()) {
                        return;
                }
                fprintf (log_file, message);
#ifdef MEDUSA_LOG_DEBUG
                printf (message);
#endif
        }
}

void
medusa_close_log_file_on_exit (void)
{
        if (log_file) {
                fclose (log_file);
        }
}
