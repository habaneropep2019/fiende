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
 *  x-screensaver-code.c -- Code taken from 
 *  xscreensaver, for detecting the idle and non-idle
 *  state of an X session.  */

/* License information included with the Xscreensaver code 
 * xscreensaver, Copyright (c) 1991-1997, 1998
 *  Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

/* Defines XrmOptionDescRec */
#include <X11/Intrinsic.h>
/* Defines the Display struct & typedef */
#include <X11/Xlib.h>

#include <glib.h>

extern XrmOptionDescRec command_line_options [];
extern char *application_defaults [];

typedef struct XScreenSaverServerInfo XScreenSaverServerInfo;
typedef struct XScreenSaverScreen XScreenSaverScreen;

Widget           x_screensaver_connect_to_server_and_get_display         (XScreenSaverServerInfo *si,
									  int *argc, 
									  char **argv);
void             x_screensaver_initialize_per_screen_info                (XScreenSaverServerInfo *si, 
                                                                          Widget toplevel_shell);
void             x_screensaver_initialize_server_extensions              (XScreenSaverServerInfo *server_info);
Bool             x_screensaver_check_for_idle_status_changes             (XScreenSaverServerInfo *server_info);
void             x_screensaver_select_events_from_screen_windows         (XScreenSaverServerInfo *si);

/* Specific timers we call from the main loop */
void             x_screensaver_schedule_wakeup_event                     (XScreenSaverServerInfo *server_info, 
                                                                          Time when);
void             x_screensaver_check_pointer_timer                       (XtPointer closure, 
                                                                          XtIntervalId *id);

struct XScreenSaverServerInfo {
        Display *display;
        XrmDatabase display_database;
        XtAppContext application_context;
  
        /* Screen Info for polling pointer location */
        int number_of_screens;
        XScreenSaverScreen *screens;
        XScreenSaverScreen *last_activity_screen;
        XScreenSaverScreen *default_screen;

        Window locking_window;          /* Offscreen Window used to set show that a system is 
                                           already running */

        gboolean using_mit_saver_extension;
        gboolean using_sgi_saver_extension;
        gboolean using_proc_interrupts;
        gboolean using_xidle_extension;

        gboolean currently_idle;
        gboolean polling_for_idleness;
        gboolean polling_mouse_position;
        gboolean scanning_all_windows;

        gboolean pointer_location_implies_recent_activity;

# ifdef HAVE_MIT_SAVER_EXTENSION
        int mit_saver_ext_event_number;
        int mit_saver_ext_error_number;
# endif

# ifdef HAVE_SGI_SAVER_EXTENSION
        int sgi_saver_ext_event_number;
        int sgi_saver_ext_error_number;
# endif

        int last_activity_time;

        int wakeup_timer_id;
        int check_pointer_timer_id;

        /* Keep track of this to deal with clock skew issues */
        int last_wall_clock_time;
        Bool clock_skew_has_occurred;
};

/* The number of seconds without user input 
   before we declare the computer "not busy"
   and safe for CPU intensive processes to stop 
   sleeping */
#define MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL 120

/* The number of milliseconds between checks for events that may have occurred on screen */
#define NOTICE_EVENTS_TIMEOUT 10

struct XScreenSaverScreen {
        Screen *screen;
        Window screensaver_window;
        Visual *current_visual;
        Visual *default_visual;
        int current_depth;
        int default_depth;
        Widget toplevel_shell;
        
        int poll_mouse_last_root_x;		/* Used only when no server exts. */
        int poll_mouse_last_root_y;
        Window poll_mouse_last_child;
        unsigned int poll_mouse_last_mask;

};
