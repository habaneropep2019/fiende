/* gwmh.c - GNOME WM interaction helper functions
 * Copyright (C) 1999 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * this code is heavily based on the original gnomepager_applet
 * implementation of The Rasterman (Carsten Haitzler) <raster@rasterman.com>
 */
#include <gwmh.h>

#include <gstc.h>
#include <gdk/gdkx.h>

#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>


/* --- defines --- */
#define TASK_XROOT(task)	   ((task)->sroot ? GSTC_PARENT_XWINDOW ((task)->sroot) : 0)
#define GDK_WINDOW_IS_FOREIGN(w)   (((GdkWindowPrivate*) (w))->window_type == GDK_WINDOW_FOREIGN)
#define GDK_WINDOW_IS_DESTROYED(w) (((GdkWindowPrivate*) (w))->destroyed)


/* --- preinitialized Atoms --- */
gulong GWMHA_WIN_SUPPORTING_WM_CHECK = 0;
gulong GWMHA_WIN_PROTOCOLS = 0;
gulong GWMHA_WIN_LAYER = 0;
gulong GWMHA_WIN_STATE = 0;
gulong GWMHA_WIN_HINTS = 0;
gulong GWMHA_WIN_APP_STATE = 0;
gulong GWMHA_WIN_EXPANDED_SIZE = 0;
gulong GWMHA_WIN_ICONS = 0;
gulong GWMHA_WIN_WORKSPACE = 0;
gulong GWMHA_WIN_WORKSPACE_COUNT = 0;
gulong GWMHA_WIN_WORKSPACE_NAMES = 0;
gulong GWMHA_WIN_CLIENT_LIST = 0;
gulong GWMHA_WIN_AREA = 0;
gulong GWMHA_WIN_AREA_COUNT = 0;
gulong GWMHA_WIN_UNIFIED_AREA = 0;
static gulong XA_WM_STATE = 0;
static gulong XA_WM_PROTOCOLS = 0;
static gulong XA_WM_DELETE_WINDOW = 0;
static gulong XA_WM_TAKE_FOCUS = 0;
static gulong XA_ENLIGHTENMENT_DESKTOP = 0;
static gulong XA_KWM_WIN_ICON = 0;
static gulong XA_COMPOUND_TEXT = 0;

static const struct {
  gulong      *atom;
  const gchar *atom_name;
} gwmh_atoms[] = {
  { &GWMHA_WIN_PROTOCOLS,		"_WIN_PROTOCOLS", },
  { &GWMHA_WIN_EXPANDED_SIZE,	        "_WIN_EXPANDED_SIZE", },
  { &GWMHA_WIN_ICONS,		        "_WIN_ICONS", },
  { &GWMHA_WIN_WORKSPACE,		"_WIN_WORKSPACE", },
  { &GWMHA_WIN_WORKSPACE_COUNT,		"_WIN_WORKSPACE_COUNT", },
  { &GWMHA_WIN_WORKSPACE_NAMES,		"_WIN_WORKSPACE_NAMES", },
  { &GWMHA_WIN_CLIENT_LIST,		"_WIN_CLIENT_LIST", },
  { &GWMHA_WIN_AREA,			"_WIN_AREA", },
  { &GWMHA_WIN_AREA_COUNT,		"_WIN_AREA_COUNT", },
  { &GWMHA_WIN_UNIFIED_AREA,		"_WIN_UNIFIED_AREA", },
#define N_GWMHA_IGNORE_ATOMS (10)
  { &GWMHA_WIN_LAYER,			"_WIN_LAYER", },
  { &GWMHA_WIN_APP_STATE,		"_WIN_APP_STATE", },
  { &GWMHA_WIN_STATE,			"_WIN_STATE", },
  { &GWMHA_WIN_HINTS,			"_WIN_HINTS", },
  { &GWMHA_WIN_SUPPORTING_WM_CHECK,	"_WIN_SUPPORTING_WM_CHECK", },
  { &XA_WM_STATE,			"WM_STATE", },
  { &XA_WM_PROTOCOLS,			"WM_PROTOCOLS", },
  { &XA_WM_DELETE_WINDOW,		"WM_DELETE_WINDOW", },
  { &XA_WM_TAKE_FOCUS,			"WM_TAKE_FOCUS", },
  { &XA_ENLIGHTENMENT_DESKTOP,		"ENLIGHTENMENT_DESKTOP", },
  { &XA_KWM_WIN_ICON,                   "KWM_WIN_ICON", },
  { &XA_COMPOUND_TEXT,                  "COMPOUND_TEXT", },
};


/* --- prototypes --- */
static gpointer		get_typed_property_data	  (Display     *xdisplay,
						   Window       xwindow,
						   Atom         property,
						   Atom         requested_type,
						   gint        *size_p,
						   guint        expected_format);
static gboolean		wm_protocol_check_support (Window     xwin,
						   Atom       check_atom);
static gboolean		send_client_message_2L	  (Window     recipient,
						   Window     event_window,
						   Atom       message_type,
						   long       event_mask,
						   glong      long1,
						   glong      long2);
static gboolean		send_client_message_3L	  (Window     recipient,
						   Window     event_window,
						   Atom       message_type,
						   long       event_mask,
						   glong      long1,
						   glong      long2,
						   glong      long3);
static gboolean		send_client_message_5L	  (Window     recipient,
						   Window     event_window,
						   Atom       message_type,
						   long       event_mask,
						   glong      long1,
						   glong      long2,
						   glong      long3,
						   glong      long4,
						   glong      long5);
static void 		get_task_root_and_frame   (GwmhTask    *task);
static GdkWindow*	gdk_window_ref_from_xid	  (Window       xwin);
static guint            gwmh_property_atom2info   (Atom         atom,
						   gboolean     per_task);
static void		gwmh_task_queue_full	  (GwmhTask          *task,
						   GwmhTaskInfoMask   imask,
						   GwmhTaskInfoMask   notify_mask);
static void		gwmh_desk_update	  (GwmhDeskInfoMask   imask);
static gboolean		gwmh_task_update	  (GwmhTask          *task,
						   GwmhTaskInfoMask   imask,
						   gboolean           skip_notify);
static void		gwmh_desk_notify	  (GwmhDeskInfoMask   imask);
static void		gwmh_task_notify	  (GwmhTask	     *task,
						   GwmhTaskNotifyType ntype,
						   GwmhTaskInfoMask   imask);
static GwmhTask*	task_new		  (GdkWindow	      *gdkwindow);
static void		task_delete		  (GwmhTask	     *task);
static gboolean		client_list_sync	  (guint32           *xwindows_ids,
						   guint              n_ids);
static GdkFilterReturn	task_event_monitor	  (GdkXEvent         *gdk_xevent,
						   GdkEvent          *event,
						   gpointer           task_pointer);
static GdkFilterReturn	task_event_monitor_frame_wrapper (GdkXEvent*, GdkEvent*, gpointer);
static GdkFilterReturn	root_event_monitor	  (GdkXEvent         *gdk_xevent,
						   GdkEvent          *event,
						   gpointer           gdk_root);
static guint32*		hack_a_client_list	  (Display           *display,
						   Window             xroot,
						   guint             *_n_clients);


/* --- variables --- */
static Window           gwmh_gnome_wm_win = None;
static gboolean         gwmh_need_reboot = FALSE;
static GHookList        gwmh_desk_hook_list = { 0, };
static GHookList        gwmh_task_hook_list = { 0, };
static GSList          *gwmh_task_update_queue = NULL;
static guint            gwmh_syncs_frozen = 0;
static guint            gwmh_idle_handler_id = 0;
static GwmhDeskInfoMask gwmh_desk_update_queued = 0;
static guint           *gwmh_harea_cache = NULL;
static guint           *gwmh_varea_cache = NULL;
static GwmhDesk         gwmh_desk = {
  0	/* n_desktops */,
  0	/* n_hareas */,
  0	/* n_vareas */,
  NULL	/* desktop_names */,
  0	/* current_desktop */,
  0	/* current_harea */,
  0	/* current_varea */,
  NULL	/* client_list */,
  FALSE /* detected_gnome_wm */,
  FALSE /* unified_area */,
};
static gboolean		gwmh_hack_think_unified_area = FALSE;
static gboolean		gwmh_hack_violate_client_msg = TRUE;


/* --- functions --- */
static inline gint
gwmh_string_equals (const gchar *string1,
		    const gchar *string2)
{
  if (string1 && string2)
    return strcmp (string1, string2) == 0;
  else
    return string1 == string2;
}

static inline void
gwmh_sync (void)
{
  if (!gwmh_syncs_frozen)
    gdk_flush ();
}

static inline void
gwmh_thaw_syncs (void)
{
  if (gwmh_syncs_frozen)
    gwmh_syncs_frozen--;
  gwmh_sync ();
}

static inline void
gwmh_freeze_syncs (void)
{
  gwmh_syncs_frozen++;
}

gboolean
gwmh_init (void)
{
  static volatile guint n_init_calls = 0;
  XWindowAttributes attribs = { 0, };
  guint32 *property_data;
  gint size = 0;
  guint i;

  n_init_calls++;
  if (n_init_calls == 1)
    {
      GdkWindow *window;

      /* initialize hook lists */
      g_hook_list_init (&gwmh_desk_hook_list, sizeof (GHook));
      g_hook_list_init (&gwmh_task_hook_list, sizeof (GHook));
      
      /* setup the root window event monitor */
      window = gdk_window_ref_from_xid (GDK_ROOT_WINDOW ());
      if (!window)
	g_error (G_GNUC_PRETTY_FUNCTION "(): root window id %ld invalid? bad bad...",
		 GDK_ROOT_WINDOW ());
      gdk_window_add_filter (window,
			     root_event_monitor,
			     window);
    }

  /* *first* kill remaining tasks with notification */
  while (gwmh_desk.client_list)
    task_delete (gwmh_desk.client_list->data);

  /* initialize desk structure */
  for (i = 0; i < gwmh_desk.n_desktops; i++)
    g_free (gwmh_desk.desktop_names[i]);
  gwmh_desk.n_desktops = 1;
  gwmh_desk.n_hareas = 1;
  gwmh_desk.n_vareas = 1;
  gwmh_desk.desktop_names = g_renew (gchar*,
				     gwmh_desk.desktop_names,
				     gwmh_desk.n_desktops);
  gwmh_harea_cache = g_renew (guint,
			      gwmh_harea_cache,
			      gwmh_desk.n_desktops);
  gwmh_varea_cache = g_renew (guint,
			      gwmh_varea_cache,
			      gwmh_desk.n_desktops);
  for (i = 0; i < gwmh_desk.n_desktops; i++)
    {
      gwmh_desk.desktop_names[i] = NULL;
      gwmh_harea_cache[i] = 0;
      gwmh_varea_cache[i] = 0;
    }
  gwmh_desk.current_desktop = 0;
  gwmh_desk.current_harea = 0;
  gwmh_desk.current_varea = 0;
  gwmh_desk.detected_gnome_wm = FALSE;
  gwmh_desk.unified_area = FALSE;

  /* setup preinitialized atoms */
  for (i = 0; i < sizeof (gwmh_atoms) / sizeof (gwmh_atoms[0]); i++)
    *gwmh_atoms[i].atom = gdk_atom_intern (gwmh_atoms[i].atom_name, FALSE);
  
  gdk_error_trap_push ();

  /* set event mask for events on root window */
  XGetWindowAttributes (GDK_DISPLAY (),
			GDK_ROOT_WINDOW (),
			&attribs);
  XSelectInput (GDK_DISPLAY (),
		GDK_ROOT_WINDOW (),
		attribs.your_event_mask |
		StructureNotifyMask |
		PropertyChangeMask);
  gdk_flush ();
  
  /* check for a GNOME window manager */
  gwmh_gnome_wm_win = None;
  property_data = get_typed_property_data (GDK_DISPLAY (),
					   GDK_ROOT_WINDOW (),
					   GWMHA_WIN_SUPPORTING_WM_CHECK,
					   XA_CARDINAL,
					   &size, 32);
  if (property_data)
    {
      Window check_window = property_data[0];
      guint32 *wm_check_data;
      
      wm_check_data = get_typed_property_data (GDK_DISPLAY (),
					       check_window,
					       GWMHA_WIN_SUPPORTING_WM_CHECK,
					       XA_CARDINAL,
					       &size, 32);
      if (wm_check_data && wm_check_data[0] == check_window)
	gwmh_gnome_wm_win = check_window;
      g_free (wm_check_data);
      g_free (property_data);
    }
  gwmh_desk.detected_gnome_wm = gwmh_gnome_wm_win != None;

  /* we ignore certain GWMH properties if there's no GNOME compliant wm
   * mostly, so we don't get fooled by stale properties
   */
  if (!gwmh_gnome_wm_win)
    for (i = 0; i < N_GWMHA_IGNORE_ATOMS; i++)
      *gwmh_atoms[i].atom = None;

  gwmh_syncs_frozen = 0;
  gwmh_sync ();

  gdk_error_trap_pop ();

  /* in case handlers are already setup, they need to know about wm changes,
   * and our new structure setup
   */
  gwmh_desk_notify (GWMH_DESK_INFO_ALL | GWMH_DESK_INFO_BOOTUP);

  /* and recheck everything from queue handler */
  gwmh_desk_queue_update (GWMH_DESK_INFO_FORCE_EVERYTHING);

  return gwmh_gnome_wm_win;
}

static GdkWindow*
gdk_window_ref_from_xid (Window xwin)
{
  GdkWindow *window;

  /* the xid maybe invalid already, in that case we return NULL */
  window = gdk_window_lookup (xwin);
  if (!window)
    window = gdk_window_foreign_new (xwin);
  else
    gdk_window_ref (window);

  return window;
}

static gpointer
get_typed_property_data (Display *xdisplay,
			 Window   xwindow,
			 Atom     property,
			 Atom     requested_type,
			 gint    *size_p,
			 guint    expected_format)
{
  static const guint prop_buffer_lengh = 1024 * 1024;
  unsigned char *prop_data = NULL;
  Atom type_returned = 0;
  unsigned long nitems_return = 0, bytes_after_return = 0;
  int format_returned = 0;
  gpointer data = NULL;
  gboolean abort = FALSE;

  g_return_val_if_fail (size_p != NULL, NULL);
  *size_p = 0;

  gdk_error_trap_push ();

  abort = XGetWindowProperty (xdisplay,
			      xwindow,
			      property,
			      0, prop_buffer_lengh,
			      False,
			      requested_type,
			      &type_returned, &format_returned,
			      &nitems_return,
			      &bytes_after_return,
			      &prop_data) != Success;
  if (gdk_error_trap_pop () ||
      type_returned == None)
    abort++;
  if (!abort &&
      requested_type != AnyPropertyType &&
      requested_type != type_returned)
    {
      /* aparently this can happen for some properties of broken apps, be silent */
      abort++;
    }
  if (!abort && bytes_after_return)
    {
      g_warning (G_GNUC_PRETTY_FUNCTION "(): Eeek, property has more than %u bytes, stored on harddisk?",
		 prop_buffer_lengh);
      abort++;
    }
  if (!abort && expected_format && expected_format != format_returned)
    {
      g_warning (G_GNUC_PRETTY_FUNCTION "(): Expected format (%u) unmatched (%d), programmer was drunk?",
		 expected_format, format_returned);
      abort++;
    }
  if (!abort && prop_data && nitems_return && format_returned)
    {
      switch (format_returned)
	{
	case 32:
	  *size_p = nitems_return * 4;
	  if (sizeof (gulong) == 8)
	    {
	      guint32 i, *mem = g_malloc0 (*size_p + 1);
	      gulong *prop_longs = (gulong*) prop_data;

	      for (i = 0; i < *size_p / 4; i++)
		mem[i] = prop_longs[i];
	      data = mem;
	    }
	  break;
	case 16:
	  *size_p = nitems_return * 2;
	  break;
	case 8:
	  *size_p = nitems_return;
	  break;
	default:
	  g_warning ("Unknown property data format with %d bits (extraterrestrial?)",
		     format_returned);
	  break;
	}
      if (!data && *size_p)
	{
	  guint8 *mem = NULL;

	  if (format_returned == 8 && type_returned == XA_COMPOUND_TEXT)
	    {
	      gchar **tlist = NULL;
	      gint count = gdk_text_property_to_text_list (type_returned, 8, prop_data,
							   nitems_return, &tlist);

	      if (count && tlist && tlist[0])
		{
		  mem = (guint8 *)g_strdup (tlist[0]);
		  *size_p = strlen ((char *)mem);
		}
	      if (tlist)
		gdk_free_text_list (tlist);
	    }
	  if (!mem)
	    {
	      mem = g_malloc (*size_p + 1);
	      memcpy (mem, prop_data, *size_p);
	      mem[*size_p] = 0;
	    }
	  data = mem;
	}
    }

  if (prop_data)
    XFree (prop_data);
  
  return data;
}

static gboolean
wm_protocol_check_support (Window xwin,
			   Atom   check_atom)
{
  Atom *pdata = NULL;
  guint32 *gdata = NULL;
  int n_pids = 0;
  gboolean is_supported = FALSE;
  guint i, n_gids = 0;

  gdk_error_trap_push ();

  if (!XGetWMProtocols (GDK_DISPLAY (),
			xwin,
			&pdata,
			&n_pids))
    {
      gint size = 0;

      gdata = get_typed_property_data (GDK_DISPLAY (),
				       xwin,
				       XA_WM_PROTOCOLS,
				       XA_WM_PROTOCOLS,
				       &size, 32);
      n_gids = size / 4;
    }

  gdk_error_trap_pop ();

  for (i = 0; i < n_pids; i++)
    if (pdata[i] == check_atom)
      {
	is_supported = TRUE;
	break;
      }
  if (pdata)
    XFree (pdata);
  if (!is_supported)
    for (i = 0; i < n_gids; i++)
      if (gdata[i] == check_atom)
        {
	  is_supported = TRUE;
	  break;
        }
  g_free (gdata);

  return is_supported;
}

static gboolean
send_client_message_2L (Window recipient,
			Window event_window,
			Atom   message_type,
			long   event_mask,
			glong  long1,
			glong  long2)
{
  return send_client_message_5L (recipient, event_window, message_type, event_mask, long1, long2, 0, 0, 0);
}

static gboolean
send_client_message_3L (Window recipient,
			Window event_window,
			Atom   message_type,
			long   event_mask,
			glong  long1,
			glong  long2,
			glong  long3)
{
  return send_client_message_5L (recipient, event_window, message_type, event_mask, long1, long2, long3, 0, 0);
}

static gboolean
send_client_message_5L (Window recipient,
			Window event_window,
			Atom   message_type,
			long   event_mask,
			glong  long1,
			glong  long2,
			glong  long3,
			glong  long4,
			glong  long5)
{
  XEvent xevent = { 0, };

  /* g_return_val_if_fail (n_longs < 6, FALSE); */

  xevent.type = ClientMessage;
  xevent.xclient.window = event_window;
  xevent.xclient.message_type = message_type;
  xevent.xclient.format = 32;
  xevent.xclient.data.l[0] = long1;
  xevent.xclient.data.l[1] = long2;
  xevent.xclient.data.l[2] = long3;
  xevent.xclient.data.l[3] = long4;
  xevent.xclient.data.l[4] = long5;

  gdk_error_trap_push ();

  XSendEvent (GDK_DISPLAY (), recipient, False, event_mask, &xevent);
  gwmh_sync ();

  return !gdk_error_trap_pop ();
}

GdkWindow*
gwmh_root_put_atom_window (const gchar   *atom_name,
			   GdkWindowType  window_type,
			   GdkWindowClass window_class,
			   GdkEventMask   event_mask)
{
  GdkAtom atom;
  GdkWindow *window;
  GdkWindowAttr attributes;
  guint attributes_mask;
  Window xwin;

  g_return_val_if_fail (atom_name != NULL, NULL);

  atom = gdk_atom_intern (atom_name, FALSE);
  attributes.window_type = window_type;
  attributes.wclass = window_class;
  attributes.x = -99;
  attributes.y = -99;
  attributes.width = 1;
  attributes.height = 1;
  attributes.event_mask = event_mask;
  attributes_mask = GDK_WA_X | GDK_WA_Y;
  
  window = gdk_window_new (NULL, &attributes, attributes_mask);
  xwin = GDK_WINDOW_XWINDOW (window);
  gdk_property_change (GDK_ROOT_PARENT (),
		       atom,
		       XA_WINDOW, 32,
		       GDK_PROP_MODE_REPLACE,
		       (guchar*) &xwin, 1);
  gdk_property_change (window,
		       atom,
		       XA_WINDOW, 32,
		       GDK_PROP_MODE_REPLACE,
		       (guchar*) &xwin, 1);

  return window;
}

static void 
get_task_root_and_frame (GwmhTask *task)
{
  Window *children = NULL, xframe = 0;
  Window xwin = task->xwin, xparent = xwin, xroot = xwin;
  guint size = 0;
  
  gdk_error_trap_push ();

  while (XQueryTree (GDK_DISPLAY (), xwin, &xroot, &xparent, &children, &size))
    {
      guint32 *value;

      if (children)
	{
	  XFree (children);
	  children = NULL;
	}

      xframe = xwin;
      xwin = xparent;
      if (xparent == xroot)
	break;

      value = get_typed_property_data (GDK_DISPLAY (),
				       xparent,
				       XA_ENLIGHTENMENT_DESKTOP,
				       XA_CARDINAL,
				       &size, 0);
      if (value)
	{
	  g_free (value);
	  break;
	}
    }

  if (gdk_error_trap_pop ())
    {
      xparent = None;
      xframe = None;
      /* aparently this can happen if a window is destructed shortly after
       * creation, just be silent.
       * g_warning (G_GNUC_PRETTY_FUNCTION "(): task window id %ld invalid?", task->xwin);
       */
    }

  if (!task->sroot || GSTC_PARENT_XWINDOW (task->sroot) != xparent)
    {
      GdkWindow *window;

      if (task->sroot)
	{
	  gstc_parent_delete_watch (task->sroot);
	  task->sroot = NULL;
	}

      if (xparent)
	{
	  window = gdk_window_ref_from_xid (xparent);
	  if (window)
	    {
	      task->sroot = gstc_parent_add_watch (window);
	      gdk_window_unref (window);
	    }
	}
    }
  
  if (task->xframe != xframe)
    {
      if (task->gdkframe)
	{
	  gdk_window_remove_filter (task->gdkframe, task_event_monitor_frame_wrapper, task);
	  gdk_window_unref (task->gdkframe);
	  task->gdkframe = NULL;
	}

      task->xframe = xframe;
      if (xframe)
	{
	  XWindowAttributes attribs = { 0, };
	  
	  task->gdkframe = gdk_window_ref_from_xid (task->xframe);
	  if (task->gdkframe)
	    {
	      gdk_window_add_filter (task->gdkframe, task_event_monitor_frame_wrapper, task);
	      
	      /* select events */
	      gdk_error_trap_push ();
	      XGetWindowAttributes (GDK_DISPLAY (),
				    task->xframe,
				    &attribs);
	      XSelectInput (GDK_DISPLAY (),
			    task->xframe,
			    attribs.your_event_mask |
			    StructureNotifyMask);
	      gwmh_sync ();
	      gdk_error_trap_pop ();
	    }
	}
    }
}

static GdkFilterReturn
root_event_monitor (GdkXEvent *gdk_xevent,
		    GdkEvent  *event,
		    gpointer   gdk_root)
{
  XEvent *xevent = gdk_xevent;
  
  switch (xevent->type)
    {
      GwmhDeskInfoMask imask;

    case MapNotify:
    case UnmapNotify:
    case CirculateNotify:
    case ReparentNotify:
    case DestroyNotify:
      if (!gwmh_gnome_wm_win)
	gwmh_desk_queue_update (GWMH_DESK_INFO_CLIENT_LIST);
      if (gwmh_gnome_wm_win == xevent->xdestroywindow.window)
	{
	  gwmh_need_reboot = TRUE;
	  gwmh_desk_queue_update (GWMH_DESK_INFO_FORCE_EVERYTHING);
	}
      break;
    case PropertyNotify:
      imask = gwmh_property_atom2info (xevent->xproperty.atom, FALSE);
      if (imask)
	gwmh_desk_queue_update (imask);
      if (xevent->xproperty.atom == GWMHA_WIN_SUPPORTING_WM_CHECK)
        {
	  gwmh_need_reboot = TRUE;
	  gwmh_desk_queue_update (GWMH_DESK_INFO_FORCE_EVERYTHING);
	}
      break;
    default:
      break;
    }
  
  return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn
task_event_monitor_frame_wrapper (GdkXEvent *gdk_xevent,
				  GdkEvent  *event,
				  gpointer   task_pointer)
{
  return task_event_monitor (gdk_xevent, event, task_pointer);
}

static GdkFilterReturn
task_event_monitor (GdkXEvent *gdk_xevent,
		    GdkEvent  *event,
		    gpointer   task_pointer)
{
  GwmhTask *task = task_pointer;
  XEvent *xevent = gdk_xevent;
  GwmhTaskInfoMask imask = 0;
  GwmhTaskInfoMask ichanges = 0;
  
  switch (xevent->type)
    {
    case ConfigureNotify:
      if (xevent->xconfigure.window == task->xwin)
	{
	  task->win_x = xevent->xconfigure.x;
	  task->win_y = xevent->xconfigure.y;
	  task->win_width = xevent->xconfigure.width;
	  task->win_height = xevent->xconfigure.height;
	  ichanges |= GWMH_TASK_INFO_WIN_GEO;
	}
      else if (xevent->xconfigure.window == task->xframe &&
	       !xevent->xconfigure.send_event /* fvwm sends bogus sent_event configures */)
	{
	  task->frame_x = xevent->xconfigure.x;
	  task->frame_y = xevent->xconfigure.y;
	  task->frame_width = xevent->xconfigure.width;
	  task->frame_height = xevent->xconfigure.height;
	  ichanges |= GWMH_TASK_INFO_FRAME_GEO;
	}
      break;
    case PropertyNotify:
      imask = gwmh_property_atom2info (xevent->xproperty.atom, TRUE);
      break;
    case ReparentNotify:
      /* refetch frame and root window */
      imask = GWMH_TASK_INFO_DESKTOP | GWMH_TASK_INFO_AREA;
      /* fall through */
    case MapNotify:
    case UnmapNotify:
    case CirculateNotify:
    case DestroyNotify:
      if (!gwmh_gnome_wm_win)
	{
	  imask = GWMH_TASK_INFO_ALL;
	  gwmh_desk_queue_update (GWMH_DESK_INFO_CLIENT_LIST);
	}
      if (GWMH_TASK_FOCUSED (task) && (xevent->type == UnmapNotify ||
				       xevent->type == DestroyNotify))
	{
	  task->focused = FALSE;
	  ichanges |= GWMH_TASK_INFO_FOCUSED;
	}
      break;
    case FocusOut:
      if (GWMH_TASK_FOCUSED (task) && (xevent->xfocus.detail != NotifyPointer &&
				       xevent->xfocus.detail != NotifyInferior))
	{
	  task->focused = FALSE;
	  ichanges |= GWMH_TASK_INFO_FOCUSED;
	}
      break;
    case FocusIn:
      if (!GWMH_TASK_FOCUSED (task))
	{
	  task->focused = TRUE;
	  ichanges |= GWMH_TASK_INFO_FOCUSED;
	}
      break;
    default:
      break;
    }

  if (!gwmh_gnome_wm_win && (ichanges || imask))
    imask = GWMH_TASK_INFO_ALL;

  if (imask || ichanges)
    gwmh_task_queue_full (task, imask, ichanges);

  return GDK_FILTER_CONTINUE;
}

static guint
gwmh_property_atom2info (Atom     atom,
			 gboolean per_task)
{
  static const Atom gwmh_XA_WM_HINTS = XA_WM_HINTS;
  static const Atom gwmh_XA_WM_NAME = XA_WM_NAME;
  static const Atom gwmh_XA_WM_ICON_NAME = XA_WM_ICON_NAME;
  
  static const struct {
    const Atom *atom_p;
    guint       iflag;
  } desk_atom_masks[] = {
    /* GwmhDeskInfoMask */
    { &GWMHA_WIN_CLIENT_LIST,		GWMH_DESK_INFO_CLIENT_LIST, },
    { &GWMHA_WIN_AREA,			GWMH_DESK_INFO_CURRENT_AREA, },
    { &GWMHA_WIN_AREA_COUNT,		GWMH_DESK_INFO_N_AREAS, },
    { &GWMHA_WIN_WORKSPACE,		GWMH_DESK_INFO_CURRENT_DESKTOP, },
    { &GWMHA_WIN_WORKSPACE_NAMES,	GWMH_DESK_INFO_DESKTOP_NAMES, },
    { &GWMHA_WIN_WORKSPACE_COUNT,	GWMH_DESK_INFO_N_DESKTOPS, },
    { &GWMHA_WIN_UNIFIED_AREA,		GWMH_DESK_INFO_N_AREAS, },
  }, task_atom_masks[] = {
    /* GwmhTaskInfoMask */
    { &GWMHA_WIN_APP_STATE,		GWMH_TASK_INFO_APP_STATE, },
    { &GWMHA_WIN_STATE,			GWMH_TASK_INFO_GSTATE, },
    { &GWMHA_WIN_HINTS,			GWMH_TASK_INFO_GHINTS, },
    { &GWMHA_WIN_LAYER,			GWMH_TASK_INFO_LAYER, },
    { &GWMHA_WIN_AREA,			GWMH_TASK_INFO_AREA, },
    { &GWMHA_WIN_WORKSPACE,		GWMH_TASK_INFO_DESKTOP, },
    { &XA_WM_STATE,			GWMH_TASK_INFO_ICONIFIED, },
    { &gwmh_XA_WM_HINTS,                GWMH_TASK_INFO_WM_HINTS, },
    { &gwmh_XA_WM_NAME,			GWMH_TASK_INFO_MISC, },
    { &gwmh_XA_WM_ICON_NAME,            GWMH_TASK_INFO_MISC, },
  };
  guint i;
  
  if (per_task)
    {
      for (i = 0; i < sizeof (task_atom_masks) / sizeof (task_atom_masks[0]); i++) 
	if (*task_atom_masks[i].atom_p == atom)
	  return task_atom_masks[i].iflag;
    }
  else
    {
      for (i = 0; i < sizeof (desk_atom_masks) / sizeof (desk_atom_masks[0]); i++)
	if (*desk_atom_masks[i].atom_p == atom)
	  return desk_atom_masks[i].iflag;
    }
  
  return 0;
}

static void
gwmh_desk_update (GwmhDeskInfoMask imask)
{
  GdkWindow *window = GDK_ROOT_PARENT ();
  Display *xdisplay = GDK_WINDOW_XDISPLAY (window);
  Window xwindow = GDK_WINDOW_XWINDOW (window);
  GwmhDeskInfoMask ichanges = 0;
  gboolean force_everything = FALSE;
  
  gdk_error_trap_push ();
  
  if (imask & GWMH_DESK_INFO_N_DESKTOPS)
    {
      gint size = 0;
      guint32 *n_data;
      guint n_desktops;
      
      n_data = get_typed_property_data (xdisplay, xwindow,
					GWMHA_WIN_WORKSPACE_COUNT,
					XA_CARDINAL,
					&size, 32);
      n_desktops = MAX (1, n_data ? n_data[0] : 0);
      g_free (n_data);
      if (n_desktops != gwmh_desk.n_desktops)
	{
	  guint i, old_n = gwmh_desk.n_desktops;
	  
	  gwmh_desk.n_desktops = n_desktops;
	  ichanges |= GWMH_DESK_INFO_N_DESKTOPS;
	  
	  for (i = gwmh_desk.n_desktops; i < old_n; i++)
	    g_free (gwmh_desk.desktop_names[i]);
	  gwmh_desk.desktop_names = g_renew (gchar*,
					     gwmh_desk.desktop_names,
					     gwmh_desk.n_desktops);
	  gwmh_harea_cache = g_renew (guint,
				      gwmh_harea_cache,
				      gwmh_desk.n_desktops);
	  gwmh_varea_cache = g_renew (guint,
				      gwmh_varea_cache,
				      gwmh_desk.n_desktops);
	  for (i = old_n; i < gwmh_desk.n_desktops; i++)
	    {
	      gwmh_desk.desktop_names[i] = NULL;
	      gwmh_harea_cache[i] = 0;
	      gwmh_varea_cache[i] = 0;
	    }
	  if (old_n < gwmh_desk.n_desktops)
	    imask |= GWMH_DESK_INFO_DESKTOP_NAMES;
	}
      imask |= GWMH_DESK_INFO_CURRENT_DESKTOP;
    }
  
  if (imask & GWMH_DESK_INFO_N_AREAS)
    {
      gint size = 0;
      guint32 *size_data;
      guint n_hareas, n_vareas;

      /* FIXME: this should be a property check for UNIFIED_AREA */
      if (gwmh_hack_think_unified_area != gwmh_desk.unified_area)
	{
	  ichanges |= GWMH_DESK_INFO_CURRENT_AREA | GWMH_DESK_INFO_N_AREAS;
	  gwmh_desk.unified_area = gwmh_hack_think_unified_area;
	}

      size_data = get_typed_property_data (xdisplay, xwindow,
					   GWMHA_WIN_AREA_COUNT,
					   XA_CARDINAL,
					   &size, 32);
      n_hareas = MAX (1, size_data ? size_data[0] : 0);
      n_vareas = MAX (1, size_data ? size_data[1] : 0);
      g_free (size_data);
      if (n_hareas != gwmh_desk.n_hareas ||
	  n_vareas != gwmh_desk.n_vareas)
	{
	  guint i;
	  
	  gwmh_desk.n_hareas = n_hareas;
	  gwmh_desk.n_vareas = n_vareas;
	  ichanges |= GWMH_DESK_INFO_N_AREAS;
	  
	  for (i = 0; i < gwmh_desk.n_desktops; i++)
	    {
	      gwmh_harea_cache[i] = MIN (gwmh_harea_cache[i], gwmh_desk.n_hareas - 1);
	      gwmh_varea_cache[i] = MIN (gwmh_varea_cache[i], gwmh_desk.n_vareas - 1);
	    }
	}
      imask |= GWMH_DESK_INFO_CURRENT_AREA;
    }
  
  if (imask & GWMH_DESK_INFO_DESKTOP_NAMES)
    {
      XTextProperty text_property = { 0, };
      guint i = 0;
      
      if (XGetTextProperty (xdisplay, xwindow,
			    &text_property,
			    GWMHA_WIN_WORKSPACE_NAMES))
	{
	  gchar **property_strings = NULL;
	  int n_strings = 0;
	  
	  if (XTextPropertyToStringList (&text_property,
					 &property_strings,
					 &n_strings))
	    {
	      
	      for (; i < MIN (n_strings, gwmh_desk.n_desktops); i++)
		{
		  if (!gwmh_string_equals (gwmh_desk.desktop_names[i],
					   property_strings[i]))
		    {
		      g_free (gwmh_desk.desktop_names[i]);
		      gwmh_desk.desktop_names[i] = g_strdup (property_strings[i]);
		      ichanges |= GWMH_DESK_INFO_DESKTOP_NAMES;
		    }
		}
	      
	      if (property_strings)
		XFreeStringList (property_strings);
	    }
	  
	  if (text_property.value)
	    XFree (text_property.value);
	}
      
      for (; i < gwmh_desk.n_desktops; i++)
	if (gwmh_desk.desktop_names[i])
	  {
	    g_free (gwmh_desk.desktop_names[i]);
	    gwmh_desk.desktop_names[i] = NULL;
	    ichanges |= GWMH_DESK_INFO_DESKTOP_NAMES;
	  }
    }
  
  if (imask & GWMH_DESK_INFO_CURRENT_DESKTOP)
    {
      gint size = 0;
      guint32 *indx;
      guint current_desktop;
      
      indx = get_typed_property_data (xdisplay, xwindow,
				      GWMHA_WIN_WORKSPACE,
				      XA_CARDINAL,
				      &size, 32);
      current_desktop = MIN (indx ? indx[0] : 0, gwmh_desk.n_desktops - 1);
      g_free (indx);
      if (current_desktop != gwmh_desk.current_desktop)
	{
	  gwmh_desk.current_desktop = current_desktop;
	  ichanges |= GWMH_DESK_INFO_CURRENT_DESKTOP;
	}
      imask |= GWMH_DESK_INFO_CURRENT_AREA;
    }
  
  if (imask & GWMH_DESK_INFO_CURRENT_AREA)
    {
      gint size = 0;
      guint32 *indx_data;
      guint i, harea, varea;
      
      indx_data = get_typed_property_data (xdisplay, xwindow,
					   GWMHA_WIN_AREA,
					   XA_CARDINAL,
					   &size, 32);
      harea = MIN (indx_data ? indx_data[0] : 0, gwmh_desk.n_hareas - 1);
      varea = MIN (indx_data ? indx_data[1] : 0, gwmh_desk.n_vareas - 1);
      g_free (indx_data);
      if (harea != gwmh_desk.current_harea ||
	  varea != gwmh_desk.current_varea)
	{
	  gwmh_desk.current_harea = harea;
	  gwmh_desk.current_varea = varea;
	  ichanges |= GWMH_DESK_INFO_CURRENT_AREA;
	}
      if (gwmh_harea_cache[gwmh_desk.current_desktop] != gwmh_desk.current_harea ||
	  gwmh_varea_cache[gwmh_desk.current_desktop] != gwmh_desk.current_varea)
	{
	  gwmh_harea_cache[gwmh_desk.current_desktop] = gwmh_desk.current_harea;
	  gwmh_varea_cache[gwmh_desk.current_desktop] = gwmh_desk.current_varea;
          ichanges |= GWMH_DESK_INFO_CURRENT_AREA;
	}
      if (gwmh_desk.unified_area && (ichanges & GWMH_DESK_INFO_CURRENT_AREA))
	for (i = 0; i < gwmh_desk.n_desktops; i++)
	  {
	    gwmh_harea_cache[i] = gwmh_desk.current_harea;
	    gwmh_varea_cache[i] = gwmh_desk.current_varea;
	  }
    }
  
  if (imask & GWMH_DESK_INFO_CLIENT_LIST)
    {
      guint n_tasks = 0;
      gint size = 0;
      guint32 *task_ids;

      if (!gwmh_gnome_wm_win)
	task_ids = hack_a_client_list (xdisplay, xwindow, &n_tasks);
      else
	{
	  task_ids = get_typed_property_data (xdisplay, xwindow,
					      GWMHA_WIN_CLIENT_LIST,
					      XA_CARDINAL,
					      &size, 32);
	  n_tasks = size / 4;
	}
      if (client_list_sync (task_ids, n_tasks))
	ichanges |= GWMH_DESK_INFO_CLIENT_LIST;
      g_free (task_ids);
    }
  
  gdk_error_trap_pop ();

  force_everything = (imask & GWMH_DESK_INFO_HACK_FLAG) != 0;

  gwmh_desk_update_queued &= ~imask;
  
  if ((ichanges & (GWMH_DESK_INFO_CURRENT_DESKTOP |
		   GWMH_DESK_INFO_CURRENT_AREA)) ||
      force_everything)
    {
      GList *node;

      for (node = gwmh_desk.client_list; node; node = node->next)
	{
	  GwmhTask *task = node->data;

	  if (force_everything)
	    gwmh_task_queue_update (task, GWMH_TASK_INFO_ALL);
	  else if (GWMH_TASK_STICKY (task))
	    gwmh_task_queue_update (task, (GWMH_TASK_INFO_DESKTOP |
					   GWMH_TASK_INFO_AREA));
	}
    }

  if (force_everything)
    gwmh_desk_notify (GWMH_DESK_INFO_ALL);
  else if (ichanges)
    gwmh_desk_notify (ichanges);
}

static gboolean
gwmh_task_update (GwmhTask        *task,
		  GwmhTaskInfoMask imask,
		  gboolean         skip_notify)
{
  Display *xdisplay = GDK_WINDOW_XDISPLAY (task->gdkwindow);
  Window xwindow = task->xwin;
  gint size;
  GwmhTaskInfoMask ichanges = 0;
  GwmhDeskInfoMask desk_imask_queued;
  gboolean was_queued;

  desk_imask_queued = gwmh_desk_update_queued & (GWMH_DESK_INFO_N_DESKTOPS |
						 GWMH_DESK_INFO_N_AREAS |
						 GWMH_DESK_INFO_CURRENT_DESKTOP |
						 GWMH_DESK_INFO_CURRENT_AREA |
						 GWMH_DESK_INFO_HACK_FLAG);
  if (desk_imask_queued)
    gwmh_desk_update (desk_imask_queued);

  gdk_error_trap_push ();
  
  if (imask & GWMH_TASK_INFO_GSTATE)
    {
      guint32 *flags;
      GwmhState gstate;
      gboolean was_sticky = GWMH_TASK_STICKY (task);
      
      flags = get_typed_property_data (xdisplay, xwindow,
				       GWMHA_WIN_STATE,
				       XA_CARDINAL,
				       &size, 32);
      gstate = flags ? flags[0] : 0;
      g_free (flags);
      if (task->gstate != gstate)
	{
	  task->gstate = gstate;
	  ichanges |= GWMH_TASK_INFO_GSTATE;
	}
      if (was_sticky != GWMH_TASK_STICKY (task))
	imask |= GWMH_TASK_INFO_DESKTOP | GWMH_TASK_INFO_AREA;
    }
  
  /* we need to feature this right at the beginning,
   * to complete the task_new () process
   */
  if (imask & (GWMH_TASK_INFO_DESKTOP | GWMH_TASK_INFO_AREA))
    {
      Window xroot = task->sroot ? GSTC_PARENT_XWINDOW (task->sroot) : 0;
      Window xframe = task->xframe;
      
      get_task_root_and_frame (task);
      if (!task->xframe || !task->sroot)
	{
	  /* eek, that's ugly, we need to get rid of this task as
	   * soon as possible, but *not* right here.
	   */
	  gwmh_desk_queue_update (GWMH_DESK_INFO_CLIENT_LIST);
	  return FALSE;
	}
      if (task->xframe != xframe ||
	  (task->sroot ? GSTC_PARENT_XWINDOW (task->sroot) : 0) != !xroot)
	imask |= (GWMH_TASK_INFO_DESKTOP |
		  GWMH_TASK_INFO_AREA |
		  (task->xframe != xframe ? GWMH_TASK_INFO_FRAME_GEO : 0));
    }
  
  if (imask & GWMH_TASK_INFO_MISC)
    {
      gchar *name;
      
      name = get_typed_property_data (xdisplay, xwindow,
				      XA_WM_NAME,
				      XA_STRING,
				      &size, 8);
      if (!name && !size)
	name = get_typed_property_data (xdisplay, xwindow,
					XA_WM_NAME,
					XA_COMPOUND_TEXT,
					&size, 8);
      if (!gwmh_string_equals (task->name, name))
	{
	  g_free (task->name);
	  task->name = g_strdup (name);
	  ichanges |= GWMH_TASK_INFO_MISC;
	}
      g_free (name);

      name = get_typed_property_data (xdisplay, xwindow,
				      XA_WM_ICON_NAME,
				      XA_STRING,
				      &size, 8);
      if (!name && !size)
	name = get_typed_property_data (xdisplay, xwindow,
					XA_WM_ICON_NAME,
					XA_COMPOUND_TEXT,
					&size, 8);
      if (!gwmh_string_equals (task->icon_name, name))
	{
	  g_free (task->icon_name);
	  task->icon_name = g_strdup (name);
	  ichanges |= GWMH_TASK_INFO_MISC;
	}
      g_free (name);
    }
  
  if (imask & GWMH_TASK_INFO_FOCUSED)
    {
      Window focus;
      int revert_to;
      gboolean focused;
      
      XGetInputFocus (xdisplay, &focus, &revert_to);
      focused = task->xwin == focus;
      if (focused != GWMH_TASK_FOCUSED (task))
	{
	  if (task->focused)
	    g_message ("%ssetting focus according to XGetInputFocus() for %ld",
		       !focused ? "re" : "",
		       task->xwin);
	  task->focused = focused;
	  ichanges |= GWMH_TASK_INFO_FOCUSED;
	}
    }
  
  if (imask & GWMH_TASK_INFO_ICONIFIED)
    {
      gint32 *state;
      gboolean iconified;
      
      state = get_typed_property_data (xdisplay, xwindow,
				       XA_WM_STATE,
				       XA_WM_STATE,
				       &size, 32);
      iconified = state && state[0] == IconicState;
      g_free (state);
      if (iconified != task->iconified)
	{
	  task->iconified = iconified;
	  ichanges |= GWMH_TASK_INFO_ICONIFIED;
	}
    }
  
  if (imask & GWMH_TASK_INFO_GHINTS)
    {
      guint32 *ghint_data;
      GwmhHints ghints;
      
      ghint_data = get_typed_property_data (xdisplay, xwindow,
					    GWMHA_WIN_HINTS,
					    XA_CARDINAL,
					    &size, 32);
      ghints = ghint_data ? ghint_data[0] : 0;
      g_free (ghint_data);
      if (task->ghints != ghints)
	{
	  task->ghints = ghints;
	  ichanges |= GWMH_TASK_INFO_GHINTS;
	}
    }
  
  if (imask & GWMH_TASK_INFO_APP_STATE)
    {
      guint32 *state_data;
      GnomeWinAppState app_state;
      
      state_data = get_typed_property_data (xdisplay, xwindow,
					    GWMHA_WIN_APP_STATE,
					    XA_CARDINAL,
					    &size, 32);
      app_state = state_data ? state_data[0] : 0;
      g_free (state_data);
      if (app_state != task->app_state)
	{
	  task->app_state = app_state;
	  ichanges |= GWMH_TASK_INFO_APP_STATE;
	}
    }
  
  if (imask & GWMH_TASK_INFO_DESKTOP)
    {
      guint desktop;
      
      if (!GWMH_TASK_STICKY (task))
	{
	  guint32 *indx = get_typed_property_data (xdisplay, xwindow,
						   GWMHA_WIN_WORKSPACE,
						   XA_CARDINAL,
						   &size, 32);
	  desktop = MIN (indx ? indx[0] : 0, gwmh_desk.n_desktops - 1);
	  g_free (indx);
	}
      else
	desktop = gwmh_desk.current_desktop;
      if (desktop != task->desktop)
	{
	  task->last_desktop = task->desktop;
	  task->desktop = desktop;
	  ichanges |= GWMH_TASK_INFO_DESKTOP;
	}
    }
  
  if (imask & GWMH_TASK_INFO_AREA)
    {
      guint harea, varea;
      
      if (!GWMH_TASK_STICKY (task))
	{
	  guint32 *coords = get_typed_property_data (xdisplay, xwindow,
						     GWMHA_WIN_AREA,
						     XA_CARDINAL,
						     &size, 32);
	  harea = MIN (coords ? coords[0] : 0, gwmh_desk.n_hareas - 1);
	  varea = MIN (coords ? coords[1] : 0, gwmh_desk.n_vareas - 1);
	  g_free (coords);
	}
      else
	{
	  harea = gwmh_desk.current_harea;
	  varea = gwmh_desk.current_varea;
	}
      if (task->harea != harea || task->varea != varea)
	{
	  task->last_harea = task->harea;
	  task->last_varea = task->varea;
	  task->harea = harea;
	  task->varea = varea;
	  ichanges |= GWMH_TASK_INFO_AREA;
	  imask |= GWMH_TASK_INFO_ALLOCATION;
	}
    }
  
  if (imask & GWMH_TASK_INFO_LAYER)
    {
      guint32 *layer_data;
      guint layer;
      
      layer_data = get_typed_property_data (xdisplay, xwindow,
					    GWMHA_WIN_LAYER,
					    XA_CARDINAL,
					    &size, 32);
      layer = layer_data ? layer_data[0] : 0;
      g_free (layer_data);
      if (layer != task->layer)
	{
	  task->last_layer = task->layer;
	  task->layer = layer;
	  ichanges |= GWMH_TASK_INFO_LAYER;
	}
    }
  
  if (imask & GWMH_TASK_INFO_ALLOCATION)
    {
      Window dummy_win;
      gint x = 0, y = 0;
      guint border, depth, width = 0, height = 0;
      
      if (imask & GWMH_TASK_INFO_FRAME_GEO)
	{
	  XGetGeometry (xdisplay, task->xframe,
			&dummy_win,
			&x, &y,
			&width, &height,
			&border, &depth);
	  XTranslateCoordinates (xdisplay, task->xframe,
				 TASK_XROOT (task),
				 0, 0,
				 &x, &y,
				 &dummy_win);
	  if (task->frame_x != x || task->frame_y != y ||
	      task->frame_width != width || task->frame_height != height)
	    {
	      task->frame_x = x;
	      task->frame_y = y;
	      task->frame_width = width;
	      task->frame_height = height;
	      ichanges |= GWMH_TASK_INFO_FRAME_GEO;
	    }
	}
      if (imask & GWMH_TASK_INFO_WIN_GEO)
	{
	  x = 0;
	  y = 0;
	  width = 0;
	  height = 0;
	  XGetGeometry (xdisplay, task->xwin,
			&dummy_win,
			&x, &y,
			&width, &height,
			&border, &depth);
	  XTranslateCoordinates (xdisplay, task->xwin,
				 TASK_XROOT (task),
				 0, 0,
				 &x, &y,
				 &dummy_win);
	  if (task->win_x != x || task->win_y != y ||
	      task->win_width != width || task->win_height != height)
	    {
	      task->win_x = x;
	      task->win_y = y;
	      task->win_width = width;
	      task->win_height = height;
	      ichanges |= GWMH_TASK_INFO_WIN_GEO;
	    }
	}
    }

  if (imask & GWMH_TASK_INFO_UNOTIFY)
    {
      ichanges |= imask & GWMH_TASK_INFO_UNOTIFY;
    }
  
  gdk_error_trap_pop ();
  
#if 0
  if (GWMH_TASK_STICKY (task))
    g_print ("sticky window %ld (%d:%d,%d) has frame %ld at %d, %d, %d, %d\n",
	     task->xwin,
	     task->desktop, task->harea, task->varea,
	     task->xframe,
	     task->frame_x, task->frame_y,
	     task->frame_width, task->frame_height);
#endif

  was_queued = GWMH_TASK_UPDATE_QUEUED (task);
  task->imask_queued &= ~imask;
  if (skip_notify)
    task->imask_notify |= ichanges;
  else
    {
      ichanges |= task->imask_notify;
      task->imask_notify = 0;
    }
  
  if (was_queued && !GWMH_TASK_UPDATE_QUEUED (task))
    gwmh_task_update_queue = g_slist_remove (gwmh_task_update_queue, task);
  
  if (!skip_notify && ichanges)
    gwmh_task_notify (task, GWMH_NOTIFY_INFO_CHANGED, ichanges);

  return TRUE;
}

static gboolean
gwmh_idle_handler (gpointer data)
{
  if (gwmh_need_reboot)
    {
      gwmh_need_reboot = FALSE;
      gwmh_idle_handler_id = 0;
      gwmh_init ();
      return FALSE;
    }
  
  if (gwmh_desk_update_queued)
    gwmh_desk_update (gwmh_desk_update_queued);

  while (gwmh_task_update_queue)
    {
      GSList *node = gwmh_task_update_queue;
      GwmhTask *task = node->data;
      GwmhTaskInfoMask imask = task->imask_queued;

      gwmh_task_update_queue = node->next;
      g_slist_free_1 (node);

      task->imask_queued = 0;
      gwmh_task_update (task, imask, FALSE);
    }

  gwmh_idle_handler_id = 0;

  if (gwmh_desk_update_queued)
    gwmh_desk_update (gwmh_desk_update_queued);

  return FALSE;
}

void
gwmh_desk_queue_update (GwmhDeskInfoMask imask)
{
  gwmh_task_queue_full (NULL, 0, 0);

  gwmh_desk_update_queued |= imask;
}

static void
gwmh_task_queue_full (GwmhTask        *task,
		      GwmhTaskInfoMask imask,
		      GwmhTaskInfoMask notify_mask)
{
  if (!gwmh_idle_handler_id)
    gwmh_idle_handler_id = gtk_idle_add_priority (GWMH_PRIORITY_UPDATE,
						  gwmh_idle_handler,
						  NULL);
  if (task)
    {
      gboolean was_queued = GWMH_TASK_UPDATE_QUEUED (task);

      task->imask_queued |= imask;
      task->imask_notify |= notify_mask;

      if (!was_queued && GWMH_TASK_UPDATE_QUEUED (task))
	gwmh_task_update_queue = g_slist_prepend (gwmh_task_update_queue, task);
    }
}

void
gwmh_task_queue_update (GwmhTask        *task,
			GwmhTaskInfoMask imask)
{
  g_return_if_fail (task != NULL);

  gwmh_task_queue_full (task, imask, 0);
}

guint
gwmh_desk_notifier_add (GwmhDeskNotifierFunc func,
			gpointer             func_data)
{
  GHook *hook;

  g_return_val_if_fail (func != NULL, 0);

  hook = g_hook_alloc (&gwmh_desk_hook_list);
  hook->func = func;
  hook->data = func_data;
  g_hook_prepend (&gwmh_desk_hook_list, hook);

  return hook->hook_id;
}

guint
gwmh_task_notifier_add (GwmhTaskNotifierFunc func,
			gpointer             func_data)
{
  GHook *hook;

  g_return_val_if_fail (func != NULL, 0);

  hook = g_hook_alloc (&gwmh_task_hook_list);
  hook->func = func;
  hook->data = func_data;
  g_hook_prepend (&gwmh_task_hook_list, hook);

  return hook->hook_id;
}

void
gwmh_desk_notifier_remove (guint id)
{
  g_return_if_fail (id > 0);

  if (!g_hook_destroy (&gwmh_desk_hook_list, id))
    g_warning (G_GNUC_PRETTY_FUNCTION "(): unable to remove notifier (%d)",
	       id);
}

void
gwmh_task_notifier_remove (guint id)
{
  g_return_if_fail (id > 0);

  if (!g_hook_destroy (&gwmh_task_hook_list, id))
    g_warning (G_GNUC_PRETTY_FUNCTION "(): unable to remove notifier (%d)",
	       id);
}

static gboolean
match_hook (GHook   *hook,
	    gpointer data_p)
{
  gpointer *data = data_p;

  return hook->func == data[0] && hook->data == data[1];
}

void
gwmh_desk_notifier_remove_func (GwmhDeskNotifierFunc func,
				gpointer             func_data)
{
  GHook *hook;
  gpointer data[2];

  g_return_if_fail (func != NULL);

  data[0] = func;
  data[1] = func_data;
  hook = g_hook_find (&gwmh_desk_hook_list, TRUE, match_hook, data);
  if (hook)
    g_hook_destroy_link (&gwmh_desk_hook_list, hook);
  else
    g_warning (G_GNUC_PRETTY_FUNCTION "(): unable to remove notifier <%p> (%p)",
	       func, func_data);
}

void
gwmh_task_notifier_remove_func (GwmhTaskNotifierFunc func,
				gpointer             func_data)
{
  GHook *hook;
  gpointer data[2];

  g_return_if_fail (func != NULL);

  data[0] = func;
  data[1] = func_data;
  hook = g_hook_find (&gwmh_task_hook_list, TRUE, match_hook, data);
  if (hook)
    g_hook_destroy_link (&gwmh_task_hook_list, hook);
  else
    g_warning (G_GNUC_PRETTY_FUNCTION "(): unable to remove notifier <%p> (%p)",
	       func, func_data);
}

static gboolean
desk_marshaller (GHook   *hook,
		 gpointer data_p)
{
  gpointer *data = data_p;
  GwmhDeskNotifierFunc func = hook->func;

  return func (hook->data,
	       &gwmh_desk,
	       GPOINTER_TO_UINT (data[0]));
}

static void
gwmh_desk_notify (GwmhDeskInfoMask imask)
{
  gpointer data[1];

  data[0] = GUINT_TO_POINTER (imask);
  
  g_hook_list_marshal_check (&gwmh_desk_hook_list, TRUE, desk_marshaller, data);
}

static gboolean
task_marshaller (GHook   *hook,
		 gpointer data_p)
{
  gpointer *data = data_p;
  GwmhTaskNotifierFunc func = hook->func;

  return func (hook->data,
	       data[0],
	       GPOINTER_TO_UINT (data[1]),
	       GPOINTER_TO_UINT (data[2]));
}

static void
gwmh_task_notify (GwmhTask          *task,
		  GwmhTaskNotifyType ntype,
		  GwmhTaskInfoMask   imask)
{
  gpointer data[3];

  g_return_if_fail (task != NULL);
  g_return_if_fail (ntype < GWMH_NOTIFY_LAST);

  data[0] = task;
  data[1] = GUINT_TO_POINTER (ntype);
  data[2] = ntype == GWMH_NOTIFY_INFO_CHANGED ? GUINT_TO_POINTER (imask) : 0;
  
  g_hook_list_marshal_check (&gwmh_task_hook_list, TRUE, task_marshaller, data);
}

static GwmhTask*
task_new (GdkWindow *window)
{
  GwmhTask *task;
  XWindowAttributes attribs;

  g_return_val_if_fail (window != NULL, NULL);

  task = g_new0 (GwmhTask, 1);
  gwmh_desk.client_list = g_list_prepend (gwmh_desk.client_list, task);
  task->name = NULL;
  task->icon_name = NULL;
  task->frame_x = -1;
  task->frame_y = -1;
  task->win_x = -1;
  task->win_y = -1;
  task->gdkwindow = window;
  gdk_window_ref (window);
  task->gdkframe = NULL;
  g_datalist_init (&task->datalist);
  task->xwin = GDK_WINDOW_XWINDOW (window);
  
  /* monitor events on the GdkWindow */
  if (task->gdkwindow)
    gdk_window_add_filter (task->gdkwindow, task_event_monitor, task);
  /* select events */
  gdk_error_trap_push ();
  XGetWindowAttributes (GDK_DISPLAY (), task->xwin, &attribs);
  XSelectInput (GDK_DISPLAY (),
		task->xwin,
		attribs.your_event_mask |
		PropertyChangeMask |
		FocusChangeMask |
		StructureNotifyMask);
  gwmh_sync ();
  gdk_error_trap_pop ();

  /* gwmh_task_update () will do this for us:
   * get_task_root_and_frame (task);
   */
  if (gwmh_task_update (task, GWMH_TASK_INFO_ALL, TRUE))
    {
      gwmh_task_notify (task, GWMH_NOTIFY_NEW, 0);
      gwmh_task_update (task, 0, FALSE);

      return task;
    }
  else
    {
      task_delete (task);

      return NULL;
    }
}

static void
task_delete (GwmhTask *task)
{
  GSList *list, *wlist = NULL;

  g_return_if_fail (g_list_find (gwmh_desk.client_list, task));

  /* reset event masks for still existing windows */
  wlist = g_slist_prepend (wlist, gdk_window_ref_from_xid (task->xframe));
  wlist = g_slist_prepend (wlist, gdk_window_ref_from_xid (task->xwin));
  gdk_error_trap_push ();
  for (list = wlist; list; list = list->next)
    {
      GdkWindow *window = list->data;

      if (window)
	{
	  if (GDK_WINDOW_IS_FOREIGN (window) &&
	      !GDK_WINDOW_IS_DESTROYED (window))
	    {
	      XWindowAttributes attribs = { 0, };

	      XGetWindowAttributes (GDK_WINDOW_XDISPLAY (window),
				    GDK_WINDOW_XWINDOW (window),
				    &attribs);
	      XSelectInput (GDK_WINDOW_XDISPLAY (window),
			    GDK_WINDOW_XWINDOW (window),
			    attribs.your_event_mask &
			    ~(StructureNotifyMask | FocusChangeMask | PropertyChangeMask));
	    }
	  gdk_window_unref (window);
	}
    }
  gwmh_sync ();
  g_slist_free (wlist);
  gdk_error_trap_pop ();

  gwmh_task_notify (task, GWMH_NOTIFY_DESTROY, 0);
  g_datalist_clear (&task->datalist);

  gdk_window_remove_filter (task->gdkwindow, task_event_monitor, task);
  gdk_window_unref (task->gdkwindow);
  if (task->gdkframe)
    {
      gdk_window_remove_filter (task->gdkframe, task_event_monitor_frame_wrapper, task);
      gdk_window_unref (task->gdkframe);
    }
  if (task->sroot)
    gstc_parent_delete_watch (task->sroot);
  if (GWMH_TASK_UPDATE_QUEUED (task))
    gwmh_task_update_queue = g_slist_remove (gwmh_task_update_queue, task);
  gwmh_desk.client_list = g_list_remove (gwmh_desk.client_list, task);
  g_free (task->name);
  task->name = "DELETED";
  g_free (task->icon_name);
  task->icon_name = NULL;
  
  if (g_list_find (gwmh_desk.client_list, task))
    g_error ("deleted task still in client_list\n"); /* FIXME */
  if (g_slist_find (gwmh_task_update_queue, task))
    g_error ("deleted task still in queue\n"); /* FIXME */
  
  g_free (task);
}

static gboolean
client_list_sync (guint32 *xwindow_ids,
		  guint    n_ids)
{
  guint i;
  GList *node, *client_list = NULL;
  gboolean clients_changed = FALSE;
  
  node = gwmh_desk.client_list;
  while (node)
    {
      GwmhTask *task = node->data;
      gboolean found_client = FALSE;
      
      node = node->next;

      for (i = 0; i < n_ids; i++)
	if (xwindow_ids[i] == task->xwin)
	  {
	    n_ids--;
	    xwindow_ids[i] = xwindow_ids[n_ids];
	    client_list = g_list_prepend (client_list, task);
	    found_client = TRUE;
	    break;
	  }
      if (!found_client)
	{
	  task_delete (task);
	  clients_changed = TRUE;
	}
    }

  for (i = 0; i < n_ids; i++)
    {
      GdkWindow *window = gdk_window_ref_from_xid (xwindow_ids[i]);

      if (window)
	{
	  GwmhTask *task = task_new (window);

	  if (task)
	    {
	      client_list = g_list_prepend (client_list, task);
	      clients_changed = TRUE;
	    }
	  gdk_window_unref (window);
	}
    }

  g_assert (g_list_length (gwmh_desk.client_list) == g_list_length (client_list)); /* FIXME */
  if (clients_changed)
    {
      g_list_free (gwmh_desk.client_list);
      gwmh_desk.client_list = client_list;
    }
  else
    g_list_free (client_list);
  
  return clients_changed;
}

GwmhTask*
gwmh_task_from_window (GdkWindow *window)
{
  GList *node;

  g_return_val_if_fail (window != NULL, NULL);

  for (node = gwmh_desk.client_list; node; node = node->next)
    {
      GwmhTask *task = node->data;

      if (task->gdkwindow == window)
	return task;
    }

  return NULL;
}

void
gwmh_task_set_qdata_full (GwmhTask      *task,
			  GQuark         quark,
			  gpointer       data,
			  GDestroyNotify destroy)
{
  g_return_if_fail (task != NULL);

  g_datalist_id_set_data_full (&task->datalist, quark, data, destroy);
}

gpointer
gwmh_task_get_qdata (GwmhTask *task,
		     GQuark    quark)
{
  g_return_val_if_fail (task != NULL, NULL);
  
  return g_datalist_id_get_data (&task->datalist, quark);
}

gpointer
gwmh_task_steal_qdata (GwmhTask *task,
		       GQuark    quark)
{
  gpointer data;

  g_return_val_if_fail (task != NULL, NULL);
  
  data = g_datalist_id_get_data (&task->datalist, quark);
  if (data)
    g_datalist_id_remove_no_notify (&task->datalist, quark);

  return data;
}

void
gwmh_task_kill (GwmhTask *task)
{
  g_return_if_fail (task != NULL);

  gdk_error_trap_push ();
  XKillClient (GDK_DISPLAY (), task->xwin);
  gwmh_sync ();
  gdk_error_trap_pop ();
}

void
gwmh_task_get_frame_area_pos (GwmhTask *task,
			      gint     *x_p,
			      gint     *y_p)
     
{
  gint x, y;
  gint swidth = gdk_screen_width ();
  gint sheight = gdk_screen_height ();

  g_return_if_fail (task != NULL);
  
  /* convert from current_area relative into task->area relative */

  x = task->frame_x + (gwmh_harea_cache[task->desktop] - task->harea) * swidth;
  y = task->frame_y + (gwmh_varea_cache[task->desktop] - task->varea) * sheight;
  if (x_p)
    *x_p = x;
  if (y_p)
    *y_p = y;
}

gboolean
gwmh_task_close (GwmhTask *task)
{
  gboolean can_delete = FALSE;

  g_return_val_if_fail (task != NULL, FALSE);

  can_delete = wm_protocol_check_support (task->xwin, XA_WM_DELETE_WINDOW);

  if (can_delete)
    can_delete = send_client_message_2L (task->xwin, task->xwin,
					 XA_WM_PROTOCOLS,
					 0,
					 XA_WM_DELETE_WINDOW, CurrentTime);

  return can_delete;
}

void
gwmh_task_iconify (GwmhTask *task)
{
  g_return_if_fail (task != NULL);

  gdk_error_trap_push ();
  XIconifyWindow (GDK_DISPLAY (), task->xwin, DefaultScreen (GDK_DISPLAY ()));
  gwmh_sync ();
  gdk_error_trap_pop ();
}

void
gwmh_task_deiconify (GwmhTask *task)
{
  g_return_if_fail (task != NULL);

  if (gwmh_task_update (task, GWMH_TASK_INFO_ICONIFIED, FALSE) &&
      GWMH_TASK_ICONIFIED (task))
    {
      gdk_error_trap_push ();
      
      XMapWindow (GDK_DISPLAY (), task->xwin);
      
      gwmh_sync ();
      gdk_error_trap_pop ();
    }
}

void
gwmh_task_focus (GwmhTask *task)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_FOCUSED, FALSE))
    return;

  if (wm_protocol_check_support (task->xwin, XA_WM_TAKE_FOCUS))
    send_client_message_2L (task->xwin, task->xwin,
			    XA_WM_PROTOCOLS,
			    0,
			    XA_WM_TAKE_FOCUS, CurrentTime);

  if (!GWMH_TASK_FOCUSED (task))
    {
      gdk_error_trap_push ();

      XSetInputFocus (GDK_DISPLAY (), task->xwin, RevertToPointerRoot, CurrentTime);

      gwmh_sync ();
      gdk_error_trap_pop ();
    }
}

void
gwmh_task_show (GwmhTask *task)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task,
			 (GWMH_TASK_INFO_ICONIFIED |
			  GWMH_TASK_INFO_FOCUSED |
			  GWMH_TASK_INFO_GSTATE),
			 FALSE))
    return;

  if (!GWMH_TASK_SKIP_FOCUS (task) &&
      wm_protocol_check_support (task->xwin, XA_WM_TAKE_FOCUS))
    send_client_message_2L (task->xwin, task->xwin,
			    XA_WM_PROTOCOLS,
			    0,
			    XA_WM_TAKE_FOCUS, CurrentTime);

  gdk_error_trap_push ();

  if (GWMH_TASK_ICONIFIED (task))
    XMapWindow (GDK_DISPLAY (), task->xwin);
  if (!GWMH_TASK_SKIP_FOCUS (task) && !GWMH_TASK_FOCUSED (task))
    XSetInputFocus (GDK_DISPLAY (), task->xwin, RevertToPointerRoot, CurrentTime);
  XRaiseWindow (GDK_DISPLAY (), task->xwin);
  if (GWMH_TASK_SHADED (task))
    send_client_message_3L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_STATE,
			    SubstructureNotifyMask,
			    GWMH_STATE_SHADED, 0, CurrentTime);
  
  gwmh_sync ();
  
  gdk_error_trap_pop ();
}

void
gwmh_task_raise (GwmhTask *task)
{
  g_return_if_fail (task != NULL);

  gdk_error_trap_push ();

  XRaiseWindow (GDK_DISPLAY (), task->xwin);
  gwmh_sync ();
  
  gdk_error_trap_pop ();
}

void
gwmh_task_set_gstate_flags (GwmhTask *task,
			    GwmhState flags)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_GSTATE, FALSE))
    return;

  if ((GWMH_TASK_GSTATE (task) & flags) != flags)
    send_client_message_3L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_STATE,
			    SubstructureNotifyMask,
			    flags, flags, CurrentTime);
}

void
gwmh_task_unset_gstate_flags (GwmhTask *task,
			      GwmhState flags)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_GSTATE, FALSE))
    return;

  if (GWMH_TASK_GSTATE (task) & flags)
    send_client_message_3L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_STATE,
			    SubstructureNotifyMask,
			    flags, 0, CurrentTime);
}

void
gwmh_task_set_ghint_flags (GwmhTask *task,
			   GwmhHints flags)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_GHINTS, FALSE))
    return;

  if ((GWMH_TASK_GHINTS (task) & flags) != flags)
    send_client_message_3L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_HINTS,
			    SubstructureNotifyMask,
			    flags, flags, CurrentTime);
}

void
gwmh_task_unset_ghint_flags (GwmhTask *task,
			     GwmhHints flags)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_GHINTS, FALSE))
    return;

  if (GWMH_TASK_GHINTS (task) & flags)
    send_client_message_3L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_HINTS,
			    SubstructureNotifyMask,
			    flags, 0, CurrentTime);
}

void
gwmh_task_set_app_state (GwmhTask        *task,
			 GnomeWinAppState app_state)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_APP_STATE, FALSE))
    return;

  if (GWMH_TASK_APP_STATE (task) != app_state)
    send_client_message_2L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_APP_STATE,
			    SubstructureNotifyMask,
			    app_state, CurrentTime);
}

void
gwmh_task_get_mini_icon (GwmhTask   *task,
			 GdkPixmap **pixmap,
			 GdkBitmap **mask)
{
  GdkGC *gc;
  GdkGCPrivate *gc_private;
  GdkPixmapPrivate *private;
  guint32 *atomdata;
  Window xwindow = task->xwin;
  Window root;
  int x, y;
  guint b, width, height, depth;
  gint size;
  Display *xdisplay;

  g_return_if_fail (task != NULL);
  g_return_if_fail (pixmap != NULL);
  g_return_if_fail (mask != NULL);

  xdisplay = GDK_WINDOW_XDISPLAY (task->gdkwindow);

  atomdata = get_typed_property_data (xdisplay,
				      xwindow,
				      XA_KWM_WIN_ICON,
				      XA_KWM_WIN_ICON,
				      &size,
				      32);
  if (!atomdata || !atomdata[0] || !size)
    {
      *pixmap = NULL;
      *mask = NULL;

      return;
    }

  /* Get icon size and depth */
  XGetGeometry (xdisplay, (Drawable) atomdata[0], &root, &x, &y, 
		&width, &height, &b, &depth);
  
  /* Create a new GdkPixmap and copy the mini icon pixmap to it */
  *pixmap = gdk_pixmap_new (NULL, width, height, depth);
  gc = gdk_gc_new (*pixmap);
  gc_private = (GdkGCPrivate*) gc;
  private = (GdkPixmapPrivate*) *pixmap;
  XCopyArea (private->xdisplay, atomdata[0], private->xwindow, gc_private->xgc,
	     0, 0, width, height, 0, 0);
  gdk_gc_destroy (gc);
  
  /* get mask size and depth */
  XGetGeometry (xdisplay, (Drawable)atomdata[1], &root, &x, &y, 
		&width, &height, &b, &depth);

  /* Create a new GdkBitmap and copy the mini icon mask to it */
  *mask = gdk_pixmap_new (NULL, width, height, 1);
  gc = gdk_gc_new (*mask);
  gc_private = (GdkGCPrivate*) gc;
  private = (GdkPixmapPrivate*) *mask;
  XCopyArea (private->xdisplay, atomdata[1], private->xwindow, gc_private->xgc,
	     0, 0, width, height, 0, 0);
  gdk_gc_destroy (gc);
  g_free (atomdata);
}

void
gwmh_task_set_layer (GwmhTask *task,
		     GwmhLayer layer)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_LAYER, FALSE))
    return;

  if (task->layer != layer)
    send_client_message_2L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_LAYER,
			    SubstructureNotifyMask,
			    layer, CurrentTime);
}

void
gwmh_task_set_area (GwmhTask *task,
		    guint     desktop,
		    guint     harea,
		    guint     varea)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_DESKTOP | GWMH_TASK_INFO_AREA, FALSE))
    return;
  
  if (desktop >= gwmh_desk.n_desktops ||
      harea >= gwmh_desk.n_hareas ||
      varea >= gwmh_desk.n_vareas ||
      (desktop == task->desktop &&
       harea == task->harea &&
       varea == task->varea))
    return;
  
  gwmh_sync ();
  
  /* ugly hack for buggy window managers
   * that violate the spec and expect us to modify properties directly
   */
  if (gwmh_hack_violate_client_msg)
    {
      long data[2];
      
      gdk_error_trap_push ();
      
      data[0] = desktop;
      if (desktop != task->desktop)
	XChangeProperty (GDK_DISPLAY (), task->xwin, GWMHA_WIN_WORKSPACE,
			 XA_CARDINAL, 32, PropModeReplace, (unsigned char*) data, 1);
      
      data[0] = harea;
      data[1] = varea;
      XChangeProperty (GDK_DISPLAY (), task->xwin, GWMHA_WIN_AREA,
		       XA_CARDINAL, 32, PropModeReplace, (unsigned char*) data, 2);
      
      gdk_error_trap_pop ();
    }
  else
    {
      gwmh_freeze_syncs ();
      if (desktop != task->desktop)
	send_client_message_2L (GDK_ROOT_WINDOW (), task->xwin,
				GWMHA_WIN_WORKSPACE,
				SubstructureNotifyMask,
				desktop, CurrentTime);
      send_client_message_3L (GDK_ROOT_WINDOW (), task->xwin,
			      GWMHA_WIN_AREA,
			      SubstructureNotifyMask,
			      harea, varea, CurrentTime);
      gwmh_thaw_syncs ();
    }
}

void
gwmh_task_set_desktop (GwmhTask *task,
		       guint     desktop)
{
  g_return_if_fail (task != NULL);

  if (!gwmh_task_update (task, GWMH_TASK_INFO_DESKTOP, FALSE) ||
      desktop >= gwmh_desk.n_desktops || task->desktop == desktop)
    return;
  
  /* ugly hack for buggy window managers
   * that violate the spec and expect us to modify properties directly
   */
  if (gwmh_hack_violate_client_msg)
    {
      long data[2];
      
      gdk_error_trap_push ();
      
      data[0] = desktop;
      if (desktop != task->desktop)
	XChangeProperty (GDK_DISPLAY (), task->xwin, GWMHA_WIN_WORKSPACE,
			 XA_CARDINAL, 32, PropModeReplace, (unsigned char*) data, 1);
      
      gdk_error_trap_pop ();
    }
  else
    send_client_message_2L (GDK_ROOT_WINDOW (), task->xwin,
			    GWMHA_WIN_WORKSPACE,
			    SubstructureNotifyMask,
			    desktop, CurrentTime);
}

GwmhDesk*
gwmh_desk_get_config (void)
{
  return &gwmh_desk;
}

void
gwmh_window_send_client_message (GdkWindow *window,
				 gulong     atom,
				 gulong	    long1,
				 gulong	    long2,
				 gulong	    long3,
				 gulong	    long4,
				 gulong	    long5)
{
  g_return_if_fail (window != NULL);
  g_return_if_fail (atom > 0);

  if (!GDK_WINDOW_IS_DESTROYED (window))
    send_client_message_5L (GDK_ROOT_WINDOW (),
			    GDK_WINDOW_XWINDOW (window),
			    atom,
			    SubstructureNotifyMask,
			    long1, long2, long3, long4, long5);
}

void
gwmh_desk_guess_desktop_area (guint  desktop,
			      guint *harea,
			      guint *varea)
{
  if (harea)
    *harea = 0;
  if (varea)
    *varea = 0;
  g_return_if_fail (desktop < gwmh_desk.n_desktops);
  
  if (harea)
    *harea = gwmh_harea_cache ? gwmh_harea_cache[desktop] : 1;
  if (varea)
    *varea = gwmh_varea_cache ? gwmh_varea_cache[desktop] : 1;
}

void
gwmh_desk_set_current_desktop (guint desktop)
{
  g_return_if_fail (desktop < gwmh_desk.n_desktops);

  gwmh_desk_update (GWMH_DESK_INFO_CURRENT_DESKTOP);
  
  if (desktop >= gwmh_desk.n_desktops ||
      desktop == gwmh_desk.current_desktop)
    return;

  send_client_message_2L (GDK_ROOT_WINDOW (), GDK_ROOT_WINDOW (),
			  GWMHA_WIN_WORKSPACE,
			  SubstructureNotifyMask,
			  desktop, CurrentTime);
}

void
gwmh_desk_set_hack_values (gboolean unified_area,
			   gboolean violate_client_msg)
{
  gwmh_hack_think_unified_area = unified_area != FALSE;
  gwmh_hack_violate_client_msg = violate_client_msg != FALSE;

  gwmh_desk_update (GWMH_DESK_INFO_N_AREAS);
}

void
gwmh_desk_set_current_area (guint desktop,
			    guint harea,
			    guint varea)
{
  gwmh_desk_update (GWMH_DESK_INFO_CURRENT_DESKTOP |
		    GWMH_DESK_INFO_CURRENT_AREA);
  
  if (desktop >= gwmh_desk.n_desktops ||
      harea >= gwmh_desk.n_hareas ||
      varea >= gwmh_desk.n_vareas ||
      (desktop == gwmh_desk.current_desktop &&
       harea == gwmh_desk.current_harea &&
       varea == gwmh_desk.current_varea))
    return;

  gwmh_sync ();
  gwmh_freeze_syncs ();
  if (desktop != gwmh_desk.current_desktop)
    send_client_message_2L (GDK_ROOT_WINDOW (), GDK_ROOT_WINDOW (),
			    GWMHA_WIN_WORKSPACE,
			    SubstructureNotifyMask,
			    desktop, CurrentTime);
  send_client_message_3L (GDK_ROOT_WINDOW (), GDK_ROOT_WINDOW (),
			  GWMHA_WIN_AREA,
			  SubstructureNotifyMask,
			  harea, varea, CurrentTime);
  gwmh_thaw_syncs ();
}

void
gwmh_desk_set_desktop_name (guint        desktop,
			    const gchar *name)
{
  gchar *old_name;

  g_return_if_fail (desktop < gwmh_desk.n_desktops);

  gwmh_desk_update (GWMH_DESK_INFO_DESKTOP_NAMES);

  if (desktop >= gwmh_desk.n_desktops)
    return;

  old_name = (gwmh_desk.desktop_names[desktop]
	      ? gwmh_desk.desktop_names[desktop]
	      : "");
  if (!gwmh_string_equals (name, old_name))
    {
      g_warning ("gwmh_desk_set_desktop_name() unimplemented");
    }
}

GList*
gwmh_task_list_get (void)
{
  return gwmh_desk.client_list;
}

GList*
gwmh_task_list_stack_sort (GList *task_list)
{
  GstcParent *sroot_cache = NULL;
  GSList *slist, *sparent_list = NULL;
  GList *node, *stacked_tasks = NULL;

  for (node = task_list; node; node = node->next)
    {
      GwmhTask *task = node->data;

      if (sroot_cache == task->sroot)
	continue;
      sroot_cache = task->sroot;
      if (task->sroot &&
	  !g_slist_find (sparent_list, task->sroot))
	sparent_list = g_slist_prepend (sparent_list, task->sroot);
    }
  sparent_list = g_slist_reverse (sparent_list);

  node = NULL;
  for (slist = sparent_list; slist; slist = slist->next)
    {
      GstcParent *sparent = slist->data;
      guint i;

      for (i = 0; i < sparent->n_children; i++)
	for (node = task_list; node; node = node->next)
	  {
	    GwmhTask *task = node->data;
	    
	    if (task->xframe == sparent->children[i])
	      {
		if (node->prev)
		  node->prev->next = node->next;
		else
		  task_list = node->next;
		if (node->next)
		  node->next->prev = node->prev;
		
		if (stacked_tasks)
		  stacked_tasks->prev = node;
		node->next = stacked_tasks;
		node->prev = NULL;
		stacked_tasks = node;
		
		break;
	      }
	  }
    }
  g_slist_free (sparent_list);

  node = g_list_last (task_list);
  stacked_tasks = g_list_reverse (stacked_tasks);
  if (node)
    {
      if (stacked_tasks)
	stacked_tasks->prev = node;
      node->next = stacked_tasks;
      stacked_tasks = task_list;
    }
  
  return stacked_tasks;
}

static gboolean
check_client (Display *display,
	      Window   xwindow,
	      Atom     state_atom)
{
  gboolean valid_client = TRUE;

  if (valid_client)
    {
      Atom dummy1;
      int format = 0;
      unsigned long dummy2, nitems = 0, *prop = NULL;
      
      XGetWindowProperty (display, xwindow, state_atom, 0, 1024, False, state_atom,
			  &dummy1, &format, &nitems, &dummy2, (unsigned char **) &prop);
      if (prop)
	{
	  valid_client = format == 32 && nitems > 0 && (prop[0] == NormalState ||
							prop[0] == IconicState);
	  XFree (prop);
	}
    }

  if (valid_client)
    {
      XWindowAttributes attributes = { 0, };

      XGetWindowAttributes (display, xwindow, &attributes);
      valid_client = (attributes.class == InputOutput &&
		      attributes.map_state == IsViewable);
    }

  if (valid_client)
    {
      XWMHints *hints = XGetWMHints (display, xwindow);
      
      valid_client &= hints && (hints->flags & InputHint) && hints->input;
      if (hints)
	XFree (hints);
    }
  
  return valid_client;
}

static Window
find_input_client (Display *display,
		   Window   xwindow,
		   Atom     state_atom)
{
  Window dummy1, dummy2, *children = NULL;
  unsigned int n_children = 0;
  guint i;
  
  if (check_client (display, xwindow, state_atom))
    return xwindow;
  
  if (!XQueryTree (display, xwindow, &dummy1, &dummy2, &children, &n_children) || !children)
    return None;
  
  for (i = 0; i < n_children; i++)
    {
      xwindow = find_input_client (display, children[i], state_atom);
      if (xwindow)
	break;
    }
  
  XFree (children);
  
  return xwindow;
}

static guint32*
hack_a_client_list (Display *display,
		    Window   xroot,
		    guint   *_n_clients)
{
  Window xwindow, dummy, *children = NULL;
  unsigned int n_children = 0;
  guint i, n_clients = 0;
  guint32 *clients = NULL;
  
  *_n_clients = 0;
  
  gdk_error_trap_push ();

  XGrabServer (display);

  if (!XQueryTree (display, xroot, &xwindow, &dummy, &children, &n_children) || !children)
    {
      XUngrabServer (display);
      return NULL;
    }
  
  for (i = 0; i < n_children; i++)
    {
      xwindow = find_input_client (display,
				   children[i],
				   XInternAtom (display, "WM_STATE", False));
      if (xwindow)
	{
	  n_clients++;
	  clients = g_renew (guint32, clients, n_clients);
	  clients[n_clients - 1] = xwindow;
	}
    }

  XUngrabServer (display);
  
  XFree (children);
  
  *_n_clients = n_clients;

  gdk_error_trap_pop ();
  
  return clients;
}
