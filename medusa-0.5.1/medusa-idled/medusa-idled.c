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
 *  x-sesssion-activity-daemon.c - Daemon that runs as part of 
 *  an X session and notifies the medusa indexer (and possibly others)
 *  to back off when user activity happens
 */

#include <config.h>
#include <fcntl.h>
#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <libmedusa/medusa-log.h>
/* Defines the Display struct & typedef */
#include <X11/Xlib.h>
 
#include "medusa-idled-private.h"
#include "x-screensaver-code.h"

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

/* Start in the busy state */
static time_t time_of_last_user_event;
static GList *client_fds = NULL;

#undef DEBUG_MEDUSA_IDLED
#define IDLED_PROPERTY_TAG_TYPE 10

#ifndef DEBUG_MEDUSA_IDLED
static void
daemon_init (void)
{
        pid_t parent_process;
        struct rlimit resource_limits;
        int i;
        
        parent_process = fork ();
        /* check if fork failed */
        if (parent_process == -1) {
                /* FIXME: We should log this event somewhere appropriate */
                exit (1);
        }
        if (parent_process != 0) {
                exit (0);
        }

        getrlimit (RLIMIT_NOFILE, &resource_limits);
        for (i = 0; i < resource_limits.rlim_cur; i++) {
                close (i);
        }

        setsid ();

        /* Fork a second time to be sure that the process can't be a session leader */
        parent_process = fork ();
        if (parent_process == -1) {
                /* FIXME: We should log this event somewhere appropriate */
                exit (1);
        }
        if (parent_process != 0) {
                exit (0);
        }

        chdir ("/");
        umask (0);
        
        
}
#endif

static const char *
idle_status_message (gboolean currently_idle)
{
        if (currently_idle) {
                return MEDUSA_IDLE_SERVICE_NO_USER_ACTIVITY_MESSAGE;
        } 
        else {
                return MEDUSA_IDLE_SERVICE_USER_ACTIVITY_MESSAGE;
        }
}

static gboolean
notify_client_of_current_idle_status (int client_fd,
                                      gboolean currently_idle)
{
        int write_result;

        write_result = write (client_fd, idle_status_message (currently_idle),
                              strlen (idle_status_message (currently_idle)));
        if (write_result == -1) {
                g_print ("No client available to send messages to.  Skipping.\n");
        }
        return (write_result != -1);
}

static gboolean
add_new_client_to_client_list (int client_fd,
                               gboolean currently_idle)
{
        g_print ("Adding new client to client list\n");
        /* Now we know an accept should not block */
        /* Send an initial notification about what the
           current idle / non-idle status was */
        notify_client_of_current_idle_status (client_fd, currently_idle);
        client_fds = g_list_prepend (client_fds, 
                                     GINT_TO_POINTER (client_fd));
        return FALSE;
}


static void
notify_clients_of_current_idle_status (gboolean currently_idle)
{
        GList *node;
        gboolean write_succeeded;
        
        for (node = client_fds; node != NULL; node = node->next) {
                write_succeeded = notify_client_of_current_idle_status (GPOINTER_TO_INT (node->data),
                                                                        currently_idle);
                /* If a client is no longer writable, we should stop sending them
                   messages */
                if (!write_succeeded) {
                        g_list_remove_link (client_fds, node);
                }
        }

}


static void
clean_up_socket_before_exiting (void)
{
        unlink (MEDUSA_IDLE_SERVICE_SOCKET_PATH);
}

static void
clean_up_socket_before_exiting_cover (int foo)
{
        clean_up_socket_before_exiting ();
}

static int
initialize_server_socket (struct sockaddr_un *daemon_address)
{
        int listening_fd;
        int bind_return_code, listen_return_code;

        listening_fd = socket (AF_LOCAL, SOCK_STREAM, 0);

        /* Set the socket to non-blocking */
        fcntl (listening_fd, F_SETFL, O_NONBLOCK);
        if (listening_fd == -1) {
                puts ("Can't get a file descriptor for the X activity daemon server's socket\n");
                exit (1);
        }
        
        daemon_address->sun_family = AF_LOCAL;
        strncpy (daemon_address->sun_path, MEDUSA_IDLE_SERVICE_SOCKET_PATH, 100);
        unlink (daemon_address->sun_path);
        bind_return_code = bind (listening_fd, 
                                 (struct sockaddr *) daemon_address,
                                 SUN_LEN (daemon_address));
        if (bind_return_code == -1) {
                puts ("X activity's server socket setup failed on bind ()\n");
                exit (1);
        }

        listen_return_code = listen (listening_fd,
                                     10);
        if (listen_return_code == -1) {
                puts ("X activity's server socket setup failed on listen ()\n");
                exit (1);
        }

        g_atexit (clean_up_socket_before_exiting);
        signal (SIGTERM, clean_up_socket_before_exiting_cover);
        return listening_fd;
}


static gboolean
window_or_its_children_have_idled_tag (Display *display,
                                       Window window,
                                       Atom XA_MEDUSA_IDLED_RUNNING_ID,
                                       const char *expected_tag_value)
{
        Window root, parent, *child_windows;
        unsigned int number_of_child_windows;
        
        int format;
        Atom type = None;
        unsigned long number_of_items, bytes_after_return;
        unsigned char *returned_property;
        
        /* Check the window itself */
        /* This is the most ridiculous complicated way to get this data,
           but I'm sure it's "completely flexible and portable".  */
        XGetWindowProperty (display, window, XA_MEDUSA_IDLED_RUNNING_ID,
                            0, /* The offset of the data we want to retrieve */
                            strlen (expected_tag_value),  /* The length of the data we want to retrieve,
                                                             in 32 bit multiples (could we divide by
                                                             4 here? I'm not sure what the right thing
                                                             to do is. */
                            False, /* Whether we should delete the property */
                            AnyPropertyType, /* The identifier associated with the property type */
                            &type,  /* The type used when we set the property */
                            &format,  /* The format we uesd when we set the property */
                            &number_of_items, /* The number of items of the data format we were returned */
                            &bytes_after_return, /* The number of bytes after what we returned in the property */
                            &returned_property); /* The actual returned property data */

        if (type == IDLED_PROPERTY_TAG_TYPE) {
                if (strcmp (returned_property, expected_tag_value) == 0) {
                        XFree (returned_property);
                        return TRUE;
                }
                XFree (returned_property);
        }

        /* Now check the children for matches */
        if (!XQueryTree (display,
                         window,
                         &root, &parent, &child_windows, &number_of_child_windows)) {
                return FALSE;
        }

        while (number_of_child_windows) {
                if (window_or_its_children_have_idled_tag (display, child_windows[--number_of_child_windows],
                                                           XA_MEDUSA_IDLED_RUNNING_ID, expected_tag_value)) {
                        return TRUE;
                }
        }
                                
        /* We've checked all the children and didn't find anything */
        return FALSE;
}

static gboolean
idled_tag_is_attached_to_an_alive_window (XScreenSaverServerInfo *server_info,
                                          Atom XA_MEDUSA_IDLED_RUNNING_ID,
                                          const char *expected_tag_value)
{
        int i;

        for (i = 0; i < server_info->number_of_screens; i++) {
                if (window_or_its_children_have_idled_tag (server_info->display,
                                                           RootWindowOfScreen (server_info->screens[i].screen),
                                                           XA_MEDUSA_IDLED_RUNNING_ID,
                                                           expected_tag_value)) {
                        return TRUE;
                }
        }
                
        return FALSE;                        
}

static void
make_sure_no_idled_is_running_and_tag_session (XScreenSaverServerInfo *server_info)
{
        Atom XA_MEDUSA_IDLED_RUNNING_ID;
        char *medusa_idled_tag;
        XSetWindowAttributes window_attributes;

        /* First check if we can find a window with our property set on it */
        
        medusa_idled_tag = g_strdup_printf ("MEDUSA_IDLED_RUNNING_ID Version %s", VERSION);
        XA_MEDUSA_IDLED_RUNNING_ID = XInternAtom (server_info->display, medusa_idled_tag, False);
        
        if (idled_tag_is_attached_to_an_alive_window (server_info,
                                                      XA_MEDUSA_IDLED_RUNNING_ID,
                                                      medusa_idled_tag)) {
                g_free (medusa_idled_tag);
                exit (1);
        }


        /* Otherwise no other idled is currently running, so we should
           create our own window to notify other copies of the idled
           that try to initialize that an idled is already running */
        window_attributes.event_mask = NoEventMask; /* This new window wants no events, */
        server_info->locking_window = XCreateWindow (server_info->display,
                                                     DefaultRootWindow (server_info->display),
                                                     -100, -100, /* This window is offscreen */
                                                     10, 10, 0, 0,
                                                     InputOnly, CopyFromParent,
                                                     CWEventMask, &window_attributes);
        XChangeProperty (server_info->display,
                         server_info->locking_window,
                         XA_MEDUSA_IDLED_RUNNING_ID,
                         IDLED_PROPERTY_TAG_TYPE, /* Type, an arbitrary non-zero value, because we don't use it, except to check if it's 0 */
                         8, /* Format: Can be 8, 16, or 32 bits.  We use 8 because we're passing an unsigned char * */
                         PropModeReplace, /* Mode: Whether we want to replace, prepend, or append to the property */
                         medusa_idled_tag, /* The data we actually set the property to */
                         strlen (medusa_idled_tag)); /* The number of elements in the data we're ising */
        g_free (medusa_idled_tag);
        /* FIXME: We should exit the idle daemon if this window
           is destroyed */
}

int
main (int argc, char **argv)
{
        int socket_fd, client_fd;
        struct sockaddr_un *daemon_address;
        socklen_t address_length;
        XScreenSaverServerInfo server_info;
        Widget top_level_shell;
        gboolean idle_status_changed;

#ifndef DEBUG_MEDUSA_IDLED
        daemon_init ();
#endif
        time_of_last_user_event = time (NULL);

        /* Let clients disconnect without issue */
        signal (SIGPIPE, SIG_IGN);

        
        /* Initialize application */
        memset (&server_info, 0, sizeof (XScreenSaverServerInfo));
        top_level_shell = x_screensaver_connect_to_server_and_get_display (&server_info,
                                                                           &argc,
                                                                           argv);
        x_screensaver_initialize_per_screen_info (&server_info,
                                                  top_level_shell);
        /* Make sure no other idled is running on this display,
           and if not, set up an offscreen window to signify that we're still
           running, so others don't start */
        make_sure_no_idled_is_running_and_tag_session (&server_info);

        x_screensaver_initialize_server_extensions (&server_info);
        x_screensaver_select_events_from_screen_windows (&server_info);
        
        
        daemon_address = g_new0 (struct sockaddr_un, 1);
        socket_fd = initialize_server_socket (daemon_address);
        address_length = SUN_LEN (daemon_address);
        /* Add timeouts for X events */
        if (server_info.polling_for_idleness) {
                /* This causes a no-op event to be delivered to us in a while, so that
                   we come back around through the event loop again.  Use of this timer
                   is economical: for example, if the screensaver should come on in 5
                   minutes, and the user has been idle for 2 minutes, then this
                   timeout will go off no sooner than 3 minutes from now.  (jwz) */
                x_screensaver_schedule_wakeup_event (&server_info,
                                                     MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL * 1000);
        }
        if (server_info.polling_mouse_position) {
                x_screensaver_check_pointer_timer ((XtPointer) &server_info, 0);
        }
        /* Main loop */
        while (1) {
#ifdef DEBUG_MEDUSA_IDLED
                g_print ("Last activity was at %d, %ld seconds ago\n", server_info.last_activity_time, time (NULL) - server_info.last_activity_time);
                if (server_info.pointer_location_implies_recent_activity) {
                        g_print ("Pointer has moved recently, so we're not idle\n");
                }
                else {
                        g_print ("No pointer movement detected recently.\n");
                }
#endif
                if ((client_fd = accept (socket_fd,
                                         (struct sockaddr *) daemon_address,
                                         &address_length)) != -1) {
                        add_new_client_to_client_list (client_fd,
                                                       server_info.currently_idle);
                }

                idle_status_changed= x_screensaver_check_for_idle_status_changes (&server_info);
                if (idle_status_changed) {
#ifdef DEBUG_MEDUSA_IDLED
                        g_print ("Idle status changed to: %s\n", server_info.currently_idle ? "Not Idle" : "Idle");
#endif
                        server_info.currently_idle = !server_info.currently_idle;
                        notify_clients_of_current_idle_status (server_info.currently_idle);
                }
                sleep (2);
        }
}


