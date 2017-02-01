/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2001 Eazel, Inc.
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
 *  medusa-system-state.c -- API to determine if medusa is on on a system
 */

#include <fcntl.h>
#include <gtk/gtkmain.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <config.h>

#include "medusa-lock.h"
#include "medusa-system-state.h"
#include "medusa-stdio-extensions.h"
#include "medusa-utils.h"

#define MEDUSA_SYSTEM_CONFIGURATION_FILE "medusa.conf"
#define MEDUSA_SYSTEM_CONFIGURATION_PATH MEDUSA_SYSCONFDIR "/" MEDUSA_SYSTEM_CONFIGURATION_FILE
#define MEDUSA_SYSTEM_CONFIGURATION_LOCK_PATH "/tmp/" MEDUSA_SYSTEM_CONFIGURATION_FILE


#define SYSTEM_STATE_POLL_INTERVAL 3000

 
typedef struct UpdateClosure {
        MedusaSystemStateFunc update;
        gpointer update_argument;
        int update_function_id;
        gboolean execute_only_once;
} UpdateClosure;

typedef struct SystemStatePoll {
        gboolean was_enabled_last_time;
        GList *updates;
        int check_for_state_changes_timeout_id;
} SystemStatePoll;

static SystemStatePoll *system_state_poll;
static int current_function_id;

gboolean    
medusa_system_services_are_enabled (void)
{
        char *configuration_file;
        char **configuration_lines;
        gboolean medusa_is_enabled;
        gboolean configuration_file_read_success;
        MedusaReadLock *read_lock;
        int i;
        
        
        read_lock = medusa_read_lock_unprivileged_get_with_timeout (MEDUSA_SYSTEM_CONFIGURATION_LOCK_PATH, 1000);
        if (read_lock == NULL) {
                return FALSE;
        }

        configuration_file_read_success = 
                medusa_read_whole_file (MEDUSA_SYSTEM_CONFIGURATION_PATH, &configuration_file) >= 0;
        medusa_read_lock_release (read_lock);
        
        if (configuration_file_read_success == FALSE) {
                return FALSE;
        }

        configuration_lines = g_strsplit (configuration_file, "\n", 0);
        g_free (configuration_file);

        medusa_is_enabled = FALSE;
        for (i = 0; configuration_lines[i] != NULL; i++) {
                if (strcasecmp (configuration_lines[i], "enabled=yes") == 0 ||
                    strcasecmp (configuration_lines[i], "enabled=true") == 0) {
                        medusa_is_enabled = TRUE;
                        break;
                }
        }

        g_strfreev (configuration_lines);

        return medusa_is_enabled;
}

const char *
medusa_get_configuration_file_path (void)
{
        return MEDUSA_SYSTEM_CONFIGURATION_PATH;
}

static void
call_state_changed_callback (gpointer list_data,
                             gpointer callback_data)
{
        UpdateClosure *update_closure;

        g_assert (list_data != NULL);
        update_closure = (UpdateClosure *) list_data;
        
        update_closure->update (update_closure->update_argument);
}

static gboolean
is_a_onetime_callback (gpointer list_data,
                       gpointer callback_data)
{
        UpdateClosure *update_closure;

        g_assert (list_data != NULL);
        
        update_closure = (UpdateClosure *) list_data;

        return update_closure->execute_only_once;
}

static gboolean
check_for_system_state_changes (gpointer data)
{
        gboolean now_enabled;

        now_enabled = medusa_system_services_are_enabled ();

        if (now_enabled != system_state_poll->was_enabled_last_time) {
                g_list_foreach (system_state_poll->updates,
                                call_state_changed_callback, NULL);
                system_state_poll->updates = medusa_g_list_remove_deep_custom (system_state_poll->updates,
                                                                               is_a_onetime_callback, 
                                                                               g_free, 
                                                                               NULL);
                system_state_poll->was_enabled_last_time = now_enabled;
        }
        
        return TRUE;
}

static int 
medusa_setup_callback_for_system_state_change (MedusaSystemStateFunc callback,
                                               gpointer callback_data,
                                               gboolean execute_callback_only_once)
{
        UpdateClosure *callback_closure;
        
        if (system_state_poll == NULL) {
                system_state_poll = g_new0 (SystemStatePoll, 1);
                system_state_poll->was_enabled_last_time = medusa_system_services_are_enabled ();
                system_state_poll->check_for_state_changes_timeout_id = gtk_timeout_add (SYSTEM_STATE_POLL_INTERVAL,
                                                                                         check_for_system_state_changes,
                                                                                         system_state_poll);
        }        

        callback_closure = g_new0 (UpdateClosure, 1);
        callback_closure->update = callback;
        callback_closure->update_argument = callback_data;
        callback_closure->update_function_id = ++current_function_id;
        callback_closure->execute_only_once = execute_callback_only_once;
        
        system_state_poll->updates = 
                g_list_prepend (system_state_poll->updates,
                                callback_closure);
        
        return callback_closure->update_function_id;
}

int
medusa_execute_when_system_state_changes (MedusaSystemStateFunc callback,
                                          gpointer callback_data)
{
        return medusa_setup_callback_for_system_state_change (callback,
                                                              callback_data,
                                                              FALSE);
}

int
medusa_execute_once_when_system_state_changes (MedusaSystemStateFunc callback,
                                               gpointer callback_data)
{
        return medusa_setup_callback_for_system_state_change (callback,
                                                              callback_data,
                                                              TRUE);
}


static gboolean
has_update_id (gpointer list_data,
               gpointer callback_data)
{
        UpdateClosure *update_closure;
        int function_id_to_match;
               
        g_assert (list_data != NULL);

        update_closure = (UpdateClosure *) list_data;
        function_id_to_match = GPOINTER_TO_INT (callback_data);
        
        return (update_closure->update_function_id == function_id_to_match);
}

static void
system_state_poll_destroy_and_set_to_null (void)
{
        g_assert (system_state_poll->updates == NULL);

        gtk_timeout_remove (system_state_poll->check_for_state_changes_timeout_id);
        g_free (system_state_poll);
        system_state_poll = NULL;
}


void
medusa_remove_state_changed_function (int update_function_id)
{
        if (system_state_poll == NULL) {
                return;
        }

        system_state_poll->updates = medusa_g_list_remove_deep_custom (system_state_poll->updates,
                                                                       has_update_id,
                                                                       g_free,
                                                                       GINT_TO_POINTER (update_function_id));
        if (system_state_poll->updates == NULL) {
                system_state_poll_destroy_and_set_to_null ();
        }
}


