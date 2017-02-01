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
 *  test-system-state.c -- Tests for the system state changed API 
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-popt.h>

#include <libmedusa/medusa-system-state.h>

static gboolean one_time_callback_has_fired = FALSE;
static int callback_called_count = 0;

static void
print_current_system_state (gpointer data)
{
        if (medusa_system_services_are_enabled ()) {
                g_print ("Medusa system services are enabled\n");
        }
        else {
                g_print ("Medusa system services are not enabled\n");
        }
  
}


static void
standard_callback_count (gpointer data)
{
        callback_called_count++;

        if (callback_called_count == 2) {
                g_print ("Second state change recorded.  One more change will confirm that the one time callback exits successfully.\n");
        }

        if (callback_called_count == 3) {
                g_print ("Third state change recorded. One time callback test passed\n");
                exit (0);
        }
}

static void
print_current_system_state_and_fail_if_called_twice (gpointer data)
{
        if (one_time_callback_has_fired) {
                g_print ("Test for one time callback FAILED: Callback function has been called twice\n");
                exit (1);
        }

        g_print ("One time callback called once\n");
        print_current_system_state (NULL);
        one_time_callback_has_fired = TRUE;
}




int
main (int argc, char *argv[])
{
        gboolean test_callbacks, test_one_time_callbacks;
        poptContext popt_context;
        struct poptOption command_line_options[] = {
                { "callback", 'c', POPT_ARG_NONE, &test_callbacks,
                  0, N_("Test system state callback by printing a message every time medusa turns on or off."),
                  NULL },
                { "one-time-callback", 'o', POPT_ARG_NONE, &test_one_time_callbacks,
                  0, N_("Test one time system state callbacks by printing a message once when medusa turns on or off."),
                  NULL },
                POPT_AUTOHELP
                { NULL, '\0', 0, NULL, 0, NULL, NULL }
        };

        test_callbacks = FALSE;
        test_one_time_callbacks = FALSE;

        gnomelib_register_popt_table (command_line_options, "Test for Medusa System State Options");
        popt_context = gnomelib_parse_args (argc, argv, 0);
        poptFreeContext (popt_context);

        print_current_system_state (NULL);

        if (test_callbacks &&
            test_one_time_callbacks) {
                g_print ("Can't test both regular and one time callbacks at once.\n");
        }

        if (test_callbacks) {
                medusa_execute_when_system_state_changes (print_current_system_state,
                                                          NULL);
                gtk_main ();
        }

        if (test_one_time_callbacks) {
                medusa_execute_once_when_system_state_changes (print_current_system_state_and_fail_if_called_twice,
                                                               NULL);
                medusa_execute_when_system_state_changes (standard_callback_count,
                                                          NULL);

                gtk_main ();
        }

        return 0;
}
