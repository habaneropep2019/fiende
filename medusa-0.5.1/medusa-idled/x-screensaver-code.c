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

/* This code has been modified by Rebecca Schulman <rebecka@eazel.com> */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xos.h>
#include <netdb.h>	/* for gethostbyname() */
#include <time.h>

#ifdef HAVE_XIDLE_EXTENSION
# include <X11/extensions/xidle.h>
#endif /* HAVE_XIDLE_EXTENSION */
#include <libmedusa/medusa-log.h>

#include "x-screensaver-code.h"
#define PROGRAM_NAME "Medusa X Session Activity Daemon"
 
XrmOptionDescRec command_line_options [] = {
};

char *application_defaults [] = {
};

Atom XA_VROOT, XA_XSETROOT_ID;
#if 0
static Atom XA_SCREENSAVER_RESPONSE;
Atom XA_SCREENSAVER, XA_SCREENSAVER_VERSION, XA_SCREENSAVER_ID;
Atom XA_SCREENSAVER_STATUS, XA_LOCK, XA_BLANK;
Atom XA_DEMO, XA_PREFS;
#endif

static void   start_notice_events_timer                   (XScreenSaverServerInfo *server_info, 
                                                           Window w);

static Visual *
get_visual_resource (Screen *screen, char *name, char *class,
		     Bool prefer_writable_cells)
{
        return DefaultVisualOfScreen (screen);
}


/* For the case where we aren't using an server extensions, select user events
   on all the existing windows, and launch timers to select events on
   newly-created windows as well.

   If a server extension is being used, this does nothing.
 */
void
x_screensaver_select_events_from_screen_windows (XScreenSaverServerInfo *si)
{
  int i;

  if (si->using_xidle_extension ||
      si->using_mit_saver_extension ||
      si->using_sgi_saver_extension)
          return;

#ifdef DEBUG_TIMERS
  fprintf (stderr, "%s: selecting events on extant windows...", PROGRAM_NAME);
  fflush (stderr);
  fflush (stdout);
#endif

  /* Select events on the root windows of every screen.  This also selects
     for window creation events, so that new subwindows will be noticed.
   */
  for (i = 0; i < si->number_of_screens; i++)
          start_notice_events_timer (si, RootWindowOfScreen (si->screens[i].screen));

#ifdef DEBUG_TIMERS
    fprintf (stderr, " done.\n");
#endif
}



/* Open the connection to the X server, and intern our Atoms.
 */
Widget
x_screensaver_connect_to_server_and_get_display (XScreenSaverServerInfo *server_info,
                                                 int *argc, char **argv)
{
        Widget top_level_shell;
        
        server_info->last_wall_clock_time = 0;

        /* FIXME: need to initialize the application context */
        top_level_shell = XtVaAppInitialize (&server_info->application_context, 
                                             "Medusa-X-session-activity-daemon",
                                             command_line_options, 
                                             XtNumber (command_line_options),
                                             argc, argv, 
                                             NULL, NULL);

        XtAppSetErrorMsgHandler (server_info->application_context, 0);
        server_info->display = XtDisplay (top_level_shell);
        server_info->display_database = XtDatabase (server_info->display);

        /* FIXME: we need to get rid of the atoms here that
           we don't use.  I think that should be most of these.  */
        XA_VROOT = XInternAtom (server_info->display, "__SWM_VROOT", False);

        /* FIXME: if we're going to keep these, they should have the name 
           x-activity-daemon rather than being a screensaver */
#if 0
        XA_SCREENSAVER = XInternAtom (server_info->display, "SCREENSAVER", False);
        XA_SCREENSAVER_VERSION = XInternAtom (server_info->display, "_SCREENSAVER_VERSION",False);
        XA_SCREENSAVER_ID = XInternAtom (server_info->display, "_SCREENSAVER_ID", False);
        XA_SCREENSAVER_STATUS = XInternAtom (server_info->display, "_SCREENSAVER_STATUS", False);
        XA_SCREENSAVER_RESPONSE = XInternAtom (server_info->display, "_SCREENSAVER_RESPONSE",
                                               False);
#endif
        XA_XSETROOT_ID = XInternAtom (server_info->display, "_XSETROOT_ID", False);

        return top_level_shell;

}

static int
screen_number (Screen *screen)
{
  Display *dpy = DisplayOfScreen (screen);
  int i;
  for (i = 0; i < ScreenCount (dpy); i++)
    if (ScreenOfDisplay (dpy, i) == screen)
      return i;
  abort ();
  return 0;
}


static int
visual_depth (Screen *screen, Visual *visual)
{
  Display *dpy = DisplayOfScreen (screen);
  XVisualInfo vi_in, *vi_out;
  int out_count, d;
  vi_in.screen = screen_number (screen);
  vi_in.visualid = XVisualIDFromVisual (visual);
  vi_out = XGetVisualInfo (dpy, VisualScreenMask|VisualIDMask,
			   &vi_in, &out_count);
  if (! vi_out) abort ();
  d = vi_out [0].depth;
  XFree ((char *) vi_out);
  return d;
}


/* Examine all of the display's screens, and populate the `XScreenSaverScreen'
   structures.
 */
void
x_screensaver_initialize_per_screen_info (XScreenSaverServerInfo *si, Widget toplevel_shell)
{
        int i;
        
        si->number_of_screens = ScreenCount(si->display);
        si->screens = (XScreenSaverScreen *)
                calloc(sizeof(XScreenSaverScreen), si->number_of_screens);
        
        si->default_screen = &si->screens[DefaultScreen(si->display)];

        for (i = 0; i < si->number_of_screens; i++)
                {
                        XScreenSaverScreen *ssi = &si->screens[i];
                        ssi->screen = ScreenOfDisplay (si->display, i);
                        
                        /* Note: we can't use the resource ".visual" because Xt is SO FUCKED. */
                        ssi->default_visual =
                                get_visual_resource (ssi->screen, "visualID", "VisualID", False);
                        
                        ssi->current_visual = ssi->default_visual;
                        ssi->current_depth = visual_depth (ssi->screen, ssi->current_visual);

                        if (ssi == si->default_screen)
                                /* Since this is the default screen, use the one already created. */
                                ssi->toplevel_shell = toplevel_shell;
                        else
                                /* Otherwise, each screen must have its own unmapped root widget. */
                                ssi->toplevel_shell =
                                        XtVaAppCreateShell ("Medusa X Session Activity Daemon", 
                                                            "progclass", applicationShellWidgetClass,
                                                            si->display,
                                                            XtNscreen, ssi->screen,
                                                            XtNvisual, ssi->current_visual,
                                                            XtNdepth,  visual_depth (ssi->screen,
                                                                                     ssi->current_visual),
                                                            0);

                }
}



static void
idle_timer (XtPointer closure, XtIntervalId *id)
{
        XScreenSaverServerInfo *server_info = (XScreenSaverServerInfo *) closure;

        /* What an amazingly shitty design.  Not only does Xt execute timeout
           events from XtAppNextEvent() instead of from XtDispatchEvent(), but
           there is no way to tell Xt to block until there is an X event OR a
           timeout happens.  Once your timeout proc is called, XtAppNextEvent()
           still won't return until a "real" X event comes in.

           So this function pushes a stupid, gratuitous, unnecessary event back
           on the event queue to force XtAppNextEvent to return Right Fucking Now.
           When the code in sleep_until_idle() sees an event of type XAnyEvent,
           which the server never generates, it knows that a timeout has occurred.
        */
        XEvent fake_event;
        fake_event.type = 0;	/* XAnyEvent type, ignored. */
        fake_event.xany.display = server_info->display;
        fake_event.xany.window  = 0;
        XPutBackEvent (server_info->display, &fake_event);
}

/* Call this when user activity (or "simulated" activity) has been noticed.
 */
static void
reset_timers (XScreenSaverServerInfo *server_info)
{

        if (server_info->using_mit_saver_extension || 
            server_info->using_sgi_saver_extension)
                return;
  
        if (server_info->wakeup_timer_id)
                {
#ifdef DEBUG_TIMERS
                        fprintf (stderr, "%s: killing idle_timer  (%d, %d)\n",
                                 PROGRAM_NAME, MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL, 
                                 server_info->wakeup_timer_id);
#endif /* DEBUG_TIMERS */
                        XtRemoveTimeOut (server_info->wakeup_timer_id);
                }
        
        /* sets server_info->wakeup_timer_id */
        x_screensaver_schedule_wakeup_event (server_info,
                                             MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL * 1000);
#ifdef DEBUG_TIMERS
        g_print ("Resetting timers because we've noticed activity.\n");
#endif
        server_info->last_activity_time = time (NULL);
}




void
x_screensaver_schedule_wakeup_event (XScreenSaverServerInfo *server_info, 
                                     Time when) 
{
        /* Wake up periodically to ask the server if we are idle. */
        server_info->wakeup_timer_id = XtAppAddTimeOut (server_info->application_context, 
                                                        when, 
                                                        idle_timer,
                                                        (XtPointer) server_info);

#ifdef DEBUG_TIMERS
        fprintf (stderr, "%s: starting idle_timer (%ld, %d)\n",
                 PROGRAM_NAME, when, server_info->check_pointer_timer_id);
#endif /* DEBUG_TIMERS */
}

/* An unfortunate situation is this: the saver is not active, because the
   user has been typing.  The machine is a laptop.  The user closes the lid
   and suspends it.  The CPU halts.  Some hours later, the user opens the
   lid.  At this point, Xt's timers will fire, and xscreensaver will blank
   the screen.

   So far so good -- well, not really, but it's the best that we can do,
   since the OS doesn't send us a signal *before* shutdown -- but if the
   user had delayed locking (lockTimeout > 0) then we should start off
   in the locked state, rather than only locking N minutes from when the
   lid was opened.  Also, eschewing fading is probably a good idea, to
   clamp down as soon as possible.

   We only do this when we'd be polling the mouse position anyway.
   This amounts to an assumption that machines with APM support also
   have /proc/interrupts.
 */
static void
check_for_clock_skew (XScreenSaverServerInfo *server_info)
{
        time_t now = time ((time_t *) 0);
        long shift = now - server_info->last_wall_clock_time;

#ifdef DEBUG_TIMERS
        fprintf (stderr, "%s: checking wall clock (%ld).\n", PROGRAM_NAME,
                 (server_info->last_wall_clock_time == 0 ? 0 : shift));
#endif /* DEBUG_TIMERS */

        if (server_info->last_wall_clock_time != 0 &&
            shift > (MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL + 15))
                {
#if 0
                        fprintf (stderr, "%s: wall clock has jumped by %d:%02d:%02d!\n",
                                 PROGRAM_NAME,
                                 (shift / (60 * 60)), ((shift / 60) % 60), (shift % 60));
#endif

                        server_info->clock_skew_has_occurred = True;
                        idle_timer ((XtPointer) server_info, 0);
                }
        else {
                server_info->clock_skew_has_occurred = False;
        }
  
        server_info->last_wall_clock_time = now;
}

#ifdef DEBUG_TIMERS
static char *
timestring (void)
{
        time_t current_time;
        
        current_time = time (NULL);
        return asctime (localtime (&current_time));
        

}
#endif


/* When we aren't using a server extension, this timer is used to periodically
   wake up and poll the mouse position, which is possibly more reliable than
   selecting motion events on every window.
 */
void
x_screensaver_check_pointer_timer (XtPointer closure, XtIntervalId *id)
{
        int i;
        XScreenSaverServerInfo *server_info = (XScreenSaverServerInfo *) closure;
        Bool active_p = False;

        /* If an extension is in use, we should not be polling the mouse.
           Unless we're also checking /proc/interrupts, in which case, we should.
        */
#ifdef DEBUG_TIMERS
        g_print ("Checking where the pointer is currently.\n");
#endif
        g_assert (server_info->using_proc_interrupts ||
                  !(server_info->using_xidle_extension ||
                    server_info->using_mit_saver_extension ||
                    server_info->using_sgi_saver_extension));
        /* If we're idle, we need to check the pointer often
         so we are responsive when the user returns to his/her machine. */
        if (server_info->currently_idle) {
                server_info->check_pointer_timer_id =
                        XtAppAddTimeOut (server_info->application_context, 
                                         2000, /* 2 seconds */
                                         x_screensaver_check_pointer_timer,
                                         (XtPointer) server_info);
        }
        else {
                server_info->check_pointer_timer_id =
                        XtAppAddTimeOut (server_info->application_context, 
                                         MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL * 1000,
                                         x_screensaver_check_pointer_timer,
                                         (XtPointer) server_info);
        }
        for (i = 0; i < server_info->number_of_screens; i++)
                {
                        XScreenSaverScreen *screen = &server_info->screens[i];
                        Window root, child;
                        int root_x, root_y, x, y;
                        unsigned int mask;

                        XQueryPointer (server_info->display, 
                                       RootWindowOfScreen (screen->screen),
                                       &root, &child,
                                       &root_x, &root_y, &x, &y, &mask);

                        if (root_x == screen->poll_mouse_last_root_x &&
                            root_y == screen->poll_mouse_last_root_y &&
                            child  == screen->poll_mouse_last_child &&
                            mask   == screen->poll_mouse_last_mask)
                                continue;

                        active_p = True;

#ifdef DEBUG_TIMERS
                        if (root_x == screen->poll_mouse_last_root_x &&
                            root_y == screen->poll_mouse_last_root_y &&
                            child  == screen->poll_mouse_last_child)
                                fprintf (stderr, "%s: modifiers changed at %s on screen %d.\n",
                                         PROGRAM_NAME, timestring(), i);
                        else
                                fprintf (stderr, "%s: pointer moved at %s on screen %d.\n",
                                         PROGRAM_NAME, timestring(), i);
      
# if 0
                        fprintf (stderr, "%s: old: %d %d 0x%x ; new: %d %d 0x%x\n",
                                 PROGRAM_NAME, 
                                 ssi->poll_mouse_last_root_x,
                                 ssi->poll_mouse_last_root_y,
                                 (unsigned int) ssi->poll_mouse_last_child,
                                 root_x, root_y, (unsigned int) child);
# endif /* 0 */

#endif /* DEBUG_TIMERS */

                        server_info->last_activity_screen    = screen;
                        screen->poll_mouse_last_root_x = root_x;
                        screen->poll_mouse_last_root_y = root_y;
                        screen->poll_mouse_last_child  = child;
                        screen->poll_mouse_last_mask   = mask;
                }

#ifdef HAVE_PROC_INTERRUPTS
        if (!active_p &&
            server_info->using_proc_interrupts &&
            proc_interrupts_activity_p (si))
                {
# ifdef DEBUG_TIMERS
                        if (p->verbose_p)
                                fprintf (stderr, "%s: /proc/interrupts activity at %s.\n",
                                         PROGRAM_NAME, timestring());
# endif /* DEBUG_TIMERS */
                        active_p = True;
                }
#endif /* HAVE_PROC_INTERRUPTS */

        server_info->pointer_location_implies_recent_activity = active_p;
        check_for_clock_skew (server_info);
}

static int
BadWindow_ehandler (Display *display, XErrorEvent *error)
{
        /* When we notice a window being created, we spawn a timer that waits
           30 seconds or so, and then selects events on that window.  This error
           handler is used so that we can cope with the fact that the window
           may have been destroyed <30 seconds after it was created.
   */
        if (error->error_code != BadWindow &&
            error->error_code != BadMatch &&
            error->error_code != BadDrawable) {
                /* Probably a bad, bad error */
                exit (1);
        }

        return 0;
}

struct notice_events_timer_arg {
        XScreenSaverServerInfo *server_info;
        Window w;
};

static void
notice_events (XScreenSaverServerInfo *server_info, 
               Window window, Bool top_p)
{
  XWindowAttributes attrs;
  unsigned long events;
  Window root, parent, *kids;
  unsigned int nkids;

  if (XtWindowToWidget (server_info->display, window))
    /* If it's one of ours, don't mess up its event mask. */
          return;

  if (!XQueryTree (server_info->display, window, &root, &parent, &kids, &nkids))
          return;
  if (window == root)
          top_p = False;
  
  XGetWindowAttributes (server_info->display, window, &attrs);
  events = ((attrs.all_event_masks | attrs.do_not_propagate_mask)
	    & KeyPressMask);
  
  /* Select for SubstructureNotify on all windows.
     Select for KeyPress on all windows that already have it selected.
     
     Note that we can't select for ButtonPress, because of X braindamage:
     only one client at a time may select for ButtonPress on a given
     window, though any number can select for KeyPress.  Someone explain
     *that* to me.
     
     So, if the user spends a while clicking the mouse without ever moving
     the mouse or touching the keyboard, we won't know that they've been
     active, and the screensaver will come on.  That sucks, but I don't
     know how to get around it.
   */
  XSelectInput (server_info->display, window, SubstructureNotifyMask | events);

#ifdef DEBUG_TIMERS
  /* Only mention one window per tree (hack hack). */
  fprintf (stderr, "%s: selected KeyPress on 0x%lX\n", PROGRAM_NAME,
           (unsigned long) window);
  top_p = False;
#endif

  if (kids)
    {
      while (nkids)
	notice_events (server_info, kids [--nkids], top_p);
      XFree ((char *) kids);
    }
}


static void
notice_events_timer (XtPointer closure, XtIntervalId *id)
{
        struct notice_events_timer_arg *arg =
                (struct notice_events_timer_arg *) closure;

        XErrorHandler old_handler = XSetErrorHandler (BadWindow_ehandler);

        XScreenSaverServerInfo *server_info = arg->server_info;
        Window window = arg->w;

        free(arg);

        notice_events (server_info, window, True);
        XSync (server_info->display, False);
        XSetErrorHandler (old_handler);
}

static void
start_notice_events_timer (XScreenSaverServerInfo *server_info, Window w)
{
        struct notice_events_timer_arg *arg =
                (struct notice_events_timer_arg *) malloc(sizeof(*arg));
        arg->server_info = server_info;
        arg->w = w;
#ifdef DEBUG_TIMERS
        g_print ("Starting notice events timer\n");
#endif
        XtAppAddTimeOut (server_info->application_context, 
                         NOTICE_EVENTS_TIMEOUT, 
                         notice_events_timer,
                         (XtPointer) arg);
#ifdef DEBUG_TIMERS
        fprintf (stderr, "%s: starting notice_events_timer for 0x%X (%ud)\n",
                 PROGRAM_NAME, (unsigned int) w, NOTICE_EVENTS_TIMEOUT);
#endif
}




/* methods of detecting idleness:

      explicitly informed by SGI SCREEN_SAVER server event;
      explicitly informed by MIT-SCREEN-SAVER server event;
      poll server idle time with XIDLE extension;
      select events on all windows, and note absence of recent events;
      note that /proc/interrupts has not changed in a while;
      activated by clientmessage.

   methods of detecting non-idleness:

      read events on the xscreensaver window;
      explicitly informed by SGI SCREEN_SAVER server event;
      explicitly informed by MIT-SCREEN-SAVER server event;
      select events on all windows, and note events on any of them;
      note that /proc/interrupts has changed;
      deactivated by clientmessage.

   I trust that explains why this function is a big hairy mess.
 */
Bool
x_screensaver_check_for_idle_status_changes (XScreenSaverServerInfo *server_info)
{
        XEvent event;
        gboolean reset_timers_for_current_event_loop;

        reset_timers_for_current_event_loop = FALSE;
        while (XtAppPending (server_info->application_context)) {
                XtAppNextEvent (server_info->application_context, &event);
                switch (event.xany.type) {
                        
                case 0:		/* our synthetic "timeout" event has been signalled */
                        if (!server_info->currently_idle) {
                                Time idle;
#ifdef HAVE_XIDLE_EXTENSION
                                if (server_info->using_xidle_extension) {
                                        /* The XIDLE extension uses the synthetic event to prod us into
                                           re-asking the server how long the user has been idle. */
                                        if (! XGetIdleTime (si->display, &idle)) {
                                                fprintf (stderr, "%s: XGetIdleTime() failed.\n", PROGRAM_NAME);
                                                saver_exit (si, 1, 0);
                                        }
                                }
                                else 
#endif /* HAVE_XIDLE_EXTENSION */
#ifdef HAVE_MIT_SAVER_EXTENSION 
                                        if (server_info->using_mit_saver_extension) {
                                                /* We don't need to do anything in this case - the synthetic
                                                   event isn't necessary, as we get sent specific events
                                                   to wake us up.  In fact, this event generally shouldn't
                                                   be being delivered when the MIT extension is in use. */
                                                idle = 0;
                                        }
                                        else
#endif /* HAVE_MIT_SAVER_EXTENSION */
#ifdef HAVE_SGI_SAVER_EXTENSION
                                                if (server_info->using_sgi_saver_extension) {
                                                        /* We don't need to do anything in this case - the synthetic
                                                           event isn't necessary, as we get sent specific events
                                                           to wake us up.  In fact, this event generally shouldn't
                                                           be being delivered when the SGI extension is in use. */
                                                        idle = 0;
                                                }
                                                else 
#endif /* HAVE_SGI_SAVER_EXTENSION */
                                                        {
                                                                /* Otherwise, no server extension is in use.  The synthetic
                                                                   event was to tell us to wake up and see if the user is now
                                                                   idle.  Compute the amount of idle time by comparing the
                                                                   `last_activity_time' to the wall clock.  The l_a_t was set
                                                                   by calling `reset_timers()', which is called only in only
                                                                   two situations: when polling the mouse position has revealed
                                                                   the the mouse has moved (user activity) or when we have read
                                                                   an event (again, user activity.)
                                                                */
                                                                idle = time (NULL) - server_info->last_activity_time;
                                                        }
                                if (idle >= MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL &&
                                    !server_info->pointer_location_implies_recent_activity) {
                                        /* Look, we've been idle long enough.  We're done. */
                                        goto DONE;
                                }
                                else if (server_info->clock_skew_has_occurred) {
                                        /* Oops, the wall clock has jumped far into the future, so
                                           we need to lock down in a hurry! */
                                        goto DONE;
                                }
                                else {
                                        /* The event went off, but it turns out that the user has not
                                           yet been idle for long enough.  So re-signal the event.
                                        */
                                        if (server_info->polling_for_idleness)
                                                x_screensaver_schedule_wakeup_event (server_info, 
                                                                                     (MEDUSA_X_ACTIVITY_NO_LONGER_BUSY_INTERVAL - idle) * 1000);
                                }
                        }
                        break;
                case ClientMessage:
                        break;
                                
                case CreateNotify:
                                /* A window has been created on the screen somewhere.  If we're
                                   supposed to scan all windows for events, prepare this window. */
                        if (server_info->scanning_all_windows) {
                                Window w = event.xcreatewindow.window;
                                start_notice_events_timer (server_info, w);
                        }
                        break;
                                
                case KeyPress:
                case KeyRelease:
                case ButtonPress:
                case ButtonRelease:
                case MotionNotify:
#ifdef DEBUG_TIMERS
                        if (event.xany.type == MotionNotify)
                                fprintf (stderr,"%s: MotionNotify at %s\n",PROGRAM_NAME,timestring());
                        else if (event.xany.type == KeyPress)
                                fprintf (stderr, "%s: KeyPress seen on 0x%X at %s\n", PROGRAM_NAME,
                                         (unsigned int) event.xkey.window, timestring ());
                        else if (event.xany.type == ButtonPress)
                                fprintf (stderr, "%s: ButtonPress seen on 0x%X at %s\n", PROGRAM_NAME,
                                         (unsigned int) event.xbutton.window, timestring ());
#endif /* DEBUG_TIMERS */
                        
                                /* We got a user event.
                                   If we're waiting for the user to become active, this is it.
                                   If we're waiting until the user becomes idle, reset the timers
                                   (since now we have longer to wait.)
                                */
                        if (server_info->currently_idle) 
                                goto DONE;
                        else 
                                reset_timers_for_current_event_loop = TRUE;

                        break;
                        
                default:
                        
#ifdef HAVE_MIT_SAVER_EXTENSION
                        if (event.type == server_info->mit_saver_ext_event_number) {
                                /* This event's number is that of the MIT-SCREEN-SAVER server
                                   extension.  This extension has one event number, and the event
                                   itself contains sub-codes that say what kind of event it was
                                   (an "idle" or "not-idle" event.)
                                */
                                XScreenSaverNotifyEvent *sevent =
                                        (XScreenSaverNotifyEvent *) &event;
                                if (sevent->state == ScreenSaverOn) {
                                        int i = 0;
#ifdef DEBUG_TIMERS
                                        fprintf (stderr, "%s: MIT ScreenSaverOn event received.\n",
                                                 PROGRAM_NAME);
#endif
                                                
#ifdef DEBUG_TIMERS
                                        if (sevent->kind != ScreenSaverExternal) {
                                                fprintf (stderr,
                                                         "%s: ScreenSaverOn event wasn't of type External!\n",
                                                         PROGRAM_NAME);
                                        }
#endif
                                        if (until_idle_p)
                                                goto DONE;
                                }
                                else if (sevent->state == ScreenSaverOff) {
                                                
#ifdef DEBUG_TIMERS
                                        fprintf (stderr, "%s: MIT ScreenSaverOff event received.\n",
                                                 PROGRAM_NAME);
#endif
                                        if (!until_idle_p)
                                                goto DONE;
                                }
#ifdef DEBUG_TIMERS
                                else
                                        fprintf (stderr,
                                                 "%s: unknown MIT-SCREEN-SAVER event %d received!\n",
                                                 PROGRAM_NAME, sevent->state);
#endif
                        }
                        else
                                        
#endif /* HAVE_MIT_SAVER_EXTENSION */
#ifdef HAVE_SGI_SAVER_EXTENSION
                                if (event.type == (server_info->sgi_saver_ext_event_number + ScreenSaverStart))
                                        {
                                                /* The SGI SCREEN_SAVER server extension has two event numbers,
                                                   and this event matches the "idle" event. */
#ifdef DEBUG_TIMERS
                                                fprintf (stderr, "%s: SGI ScreenSaverStart event received.\n",
                                                         PROGRAM_NAME);
#endif
                                                if (until_idle_p)
                                                        goto DONE;
                                        }
                                else if (event.type == (server_info->sgi_saver_ext_event_number +
                                                        ScreenSaverEnd)) {
                                        /* The SGI SCREEN_SAVER server extension has two event numbers,
                                           and this event matches the "idle" event. */
#ifdef DEBUG_TIMERS
                                        fprintf (stderr, "%s: SGI ScreenSaverEnd event received.\n",
                                                 PROGRAM_NAME);
#endif
                                        if (!until_idle_p)
                                                goto DONE;
                                }
                                else
#endif /* HAVE_SGI_SAVER_EXTENSION */
                                        /* Just some random event.  Never mind */
                                        }
        }
        if (reset_timers_for_current_event_loop) {
                reset_timers (server_info);
        }
        /* We're no longer idle if the pointer location has changed recently. */
        if (server_info->currently_idle && 
            server_info->pointer_location_implies_recent_activity) {
                return TRUE;
        }
        else {
                return FALSE;
        }
 DONE:
        if (server_info->check_pointer_timer_id)
                {
                        XtRemoveTimeOut (server_info->check_pointer_timer_id);
                        server_info->check_pointer_timer_id = 0;
                }
        if (server_info->wakeup_timer_id)
                {
                        XtRemoveTimeOut (server_info->wakeup_timer_id);
                        server_info->wakeup_timer_id = 0;
                }
      
      
        return TRUE;
}

/* MIT SCREEN-SAVER server extension hackery.
 */

#ifdef HAVE_MIT_SAVER_EXTENSION

# include <X11/extensions/scrnsaver.h>


static Bool
query_mit_saver_extension (XScreenSaverServerInfo *server_info)
{
        return XScreenSaverQueryExtension (server_info->display,
                                           &server_info->mit_saver_ext_event_number,
                                           &server_info->mit_saver_ext_error_number);
}
        
#endif /* HAVE_MIT_SAVER_EXTENSION */
        
/* SGI SCREEN_SAVER server extension hackery.
         */

#ifdef HAVE_SGI_SAVER_EXTENSION

# include <X11/extensions/XScreenSaver.h>


static Bool
query_sgi_saver_extension (XScreenSaverServerInfo)
{
        return XScreenSaverQueryExtension (server_info->display,
                                           &server_info->sgi_saver_ext_event_number,
                                           &server_info->sgi_saver_ext_error_number);
}

#endif /* HAVE_SGI_SAVER_EXTENSION */

#ifdef HAVE_XIDLE_EXTENSION

# include <X11/extensions/xidle.h>

Bool
query_xidle_extension (XScreenSaverServerInfo *si)
{
        int event_number;
        int error_number;
        return XidleQueryExtension (server_info->display, &event_number, &error_number);
}

#endif /* HAVE_XIDLE_EXTENSION */


/* If any server extensions have been requested, try and initialize them.
   Issue warnings if requests can't be honored.
        */

void
x_screensaver_initialize_server_extensions (XScreenSaverServerInfo *server_info)
{
        
        server_info->using_xidle_extension = False;
        server_info->using_sgi_saver_extension = False;
        server_info->using_mit_saver_extension = False;
        server_info->using_proc_interrupts = False;
        
        /* We use all extensions if they are available,
           as opposed to XScreenSaver, which allows you to set prefs about this */
#ifdef HAVE_XIDLE_EXTENSION
        server_info->using_xidle_extension = query_xidle_extension (server_info);
#endif
#ifdef HAVE_SGI_SAVER_EXTENSION
        server_info->using_sgi_saver_extension = query_sgi_saver_extension (server_info);
#endif
#ifdef HAVE_MIT_SAVER_EXTENSION
        server_info->using_mit_saver_extension = query_mit_saver_extension (server_info);
#endif
#ifdef HAVE_PROC_INTERRUPTS
        server_info->using_proc_interrupts = query_proc_interrupts_available (serer_inf, &piwhy);
#endif

        /* We need to select events on all windows if we're not using any extensions.
           Otherwise, we don't need to. */
        server_info->scanning_all_windows = !(server_info->using_xidle_extension ||
                                      server_info->using_mit_saver_extension ||
                                      server_info->using_sgi_saver_extension);

        /* We need to periodically wake up and check for idleness if we're not using
           any extensions, or if we're using the XIDLE extension.  The other two
           extensions explicitly deliver events when we go idle/non-idle, so we
           don't need to poll. */
        server_info->polling_for_idleness = !(server_info->using_mit_saver_extension ||
                                              server_info->using_sgi_saver_extension);
        
        /* Whether we need to periodically wake up and check to see if the mouse has
           moved.  We only need to do this when not using any extensions.  The reason
           this isn't the same as `polling_for_idleness' is that the "idleness" poll
           can happen (for example) 5 minutes from now, whereas the mouse-position
           poll should happen with low periodicity.  We don't need to poll the mouse
           position with the XIDLE extension, but we do need to periodically wake up
           and query the server with that extension.  For our purposes, polling
           /proc/interrupts is just like polling the mouse position.  It has to
           happen on the same kind of schedule. */
        server_info->polling_mouse_position = (server_info->using_proc_interrupts ||
                                               !(server_info->using_xidle_extension ||
                                                 server_info->using_mit_saver_extension ||
                                                 server_info->using_sgi_saver_extension));
  
}

        
