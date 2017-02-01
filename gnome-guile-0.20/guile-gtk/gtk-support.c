/*
 * Copyright (C) 1997, 1998, 1999 Marius Vollmer
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <libguile.h>
#include <guile-gtk.h>
#ifdef GTK_2_0
#include <gtk-2.0/gdk/gdkprivate.h>
#else
#include <gdk/gdkprivate.h>
#endif
#include "gtk-threads.h"

/* It is not strictly correct to have Gdk support functions here.  But
   as long as we do not want to have some SCM_PROCs for the (gdk gdk)
   module, we are safe. */



GdkColor *
gdk_color_intern (GdkColor *color)
{
  return color;
}

GdkFont *
gdk_font_intern (GdkFont *font)
{
  return font;
}

GdkGC *
gtk_style_fg_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->fg_gc[state];
}

GdkGC *
gtk_style_bg_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->bg_gc[state];
}

GdkGC *
gtk_style_light_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->light_gc[state];
}

GdkGC *
gtk_style_dark_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->dark_gc[state];
}

GdkGC *
gtk_style_mid_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->mid_gc[state];
}

GdkGC *
gtk_style_text_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->text_gc[state];
}

GdkGC *
gtk_style_base_gc (GtkStyle *style, GtkStateType state)
{
  if (style == NULL || state < 0 || state >= 5)
    return NULL;

  return style->base_gc[state];
}

/* Event destructuring */

GdkEventType
gdk_event_type (GdkEvent *event)
{
  return event->any.type;
}

GdkWindow *
gdk_event_window (GdkEvent *event)
{
  return event->any.window;
}

gboolean
gdk_event_send_event (GdkEvent *event)
{
  return event->any.send_event;
}

GdkRectangle
gdk_event_area (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_EXPOSE:
      return event->expose.area;
    default:
      {
	GdkRectangle r = { 0, 0, 0, 0 };
	return r;
      }
    }
}

GdkVisibilityState
gdk_event_visibility_state (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_VISIBILITY_NOTIFY:
      return event->visibility.state;
    default:
      return GDK_VISIBILITY_UNOBSCURED; // XXX
    }
}

guint32
gdk_event_time (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.time;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.time;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.time;
      return event->key.time;
    case GDK_PROPERTY_NOTIFY:
      return event->property.time;
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
      return event->selection.time;
    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:
      return event->proximity.time;
    default:
      return 0;
    }
}

gdouble
gdk_event_x (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.x;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.x;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.x;
    default:
      return 0;
    }
}

gdouble
gdk_event_y (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.y;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.y;
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.y;
    default:
      return 0;
    }
}
#ifdef GTK_2_0
#else
gdouble
gdk_event_pressure (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.pressure;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.pressure;
    default:
      return 0;
    }
}

gdouble
gdk_event_xtilt (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.xtilt;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.xtilt;
    default:
      return 0;
    }
}

gdouble
gdk_event_ytilt (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.ytilt;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.ytilt;
    default:
      return 0;
    }
}
#endif
gint
gdk_event_button (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.button;
    default:
      return 0;
    }
}

guint
gdk_event_state (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.state;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.state;
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.state;
    default:
      return 0;
    }
}

gboolean
gdk_event_is_hint (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.is_hint;
    default:
      return 0;
    }
}
#ifndef GTK_2_0
GdkInputSource
gdk_event_source (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.source;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.source;
    default:
      return GDK_SOURCE_MOUSE; /* XXX */
    }
}


guint32
gdk_event_deviceid (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.deviceid;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.deviceid;
    default:
      return 0;
    }
}
#endif

gdouble
gdk_event_x_root (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.x_root;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.x_root;
    default:
      return 0;
    }
}

gdouble
gdk_event_y_root (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_MOTION_NOTIFY:
      return event->motion.y_root;
    case GDK_BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
      return event->button.y_root;
    default:
      return 0;
    }
}

guint
gdk_event_keyval (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      return event->key.keyval;
    default:
      return 0;
    }
}

gchar *
gdk_event_string (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
      {
	gchar *str = g_malloc (event->key.length+1);
	strncpy (str, event->key.string, event->key.length);
	str[event->key.length] = '\0';
	return str;
      }
    default:
      return NULL;
    }
}

GdkWindow *
gdk_event_subwindow (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.subwindow;
    default:
      return 0;
    }
}

GdkNotifyType
gdk_event_notify_detail (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
      return event->crossing.detail;
    default:
      return 0;
    }
}

gboolean
gdk_event_in (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_FOCUS_CHANGE:
      return event->focus_change.in;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_x (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.x;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_y (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.y;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_width (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.width;
    default:
      return 0;
    }
}

gint16
gdk_event_configure_height (GdkEvent *event)
{
  switch (event->any.type)
    {
    case GDK_CONFIGURE:
      return event->configure.height;
    default:
      return 0;
    }
}

#ifndef GTK_2_0
guint32
gdk_get_leader_window_id ()
{
  return (guint32) gdk_leader_window;
}

guint32
gdk_window_get_id (GdkWindow *window)
{
  return (guint32) ((GdkWindowPrivate*)window)->xwindow;
}
#endif

SCM
gdk_window_get_size_scm (GdkWindow *window)
{
  gint width;
  gint height;
  gdk_window_get_size (window, &width, &height);
  return scm_cons (SCM_MAKINUM (width), SCM_MAKINUM (height));
}

SCM
gdk_window_get_origin_scm (GdkWindow *window)
{
  gint x;
  gint y;
  gdk_window_get_origin (window, &x, &y);
  return scm_cons (SCM_MAKINUM (x), SCM_MAKINUM (y));
}

void
gdk_draw_text_scm (GdkDrawable  *drawable,
		   GdkFont      *font,
		   GdkGC	       *gc,
		   gint		x,
		   gint		y,
		   const gchar  *text)
{
  gdk_draw_text (drawable, font, gc, x, y, text, strlen(text));
}

SCM_SYMBOL (sym_type, "-type");
SCM_SYMBOL (sym_flags, "-flags");

static SCM kw_type;
static SCM kw_flags;

extern sgtk_enum_info sgtk_gtk_arg_flags_info;

SCM
gtk_object_query_args_scm (GtkType type)
{
  guint nargs;
  guint32 *arg_flags = NULL;
  GtkArg *args;
  SCM res = SCM_EOL, *restail = &res;
  int i;

  args = gtk_object_query_args (type, &arg_flags, &nargs);
  if (args == NULL)
    {
      if (arg_flags)
	g_free (arg_flags);
      return SCM_BOOL_F;
    }

  for (i = 0; i < nargs; i++)
    {
      *restail =
	scm_cons (scm_listify (scm_makfrom0str (args[i].name),
			       kw_type,
			       sgtk_type2scm (args[i].type),
			       kw_flags,
			       sgtk_flags2scm (arg_flags[i],
					       &sgtk_gtk_arg_flags_info),
			       SCM_UNDEFINED),
		  SCM_EOL);
      restail = SCM_CDRLOC(*restail);
    }

  g_free (args);
  g_free (arg_flags);

  return res;
}

int
gtk_editable_insert_text_scm (GtkEditable *editable,
			      gchar       *text,
			      int          position)
{
  gtk_editable_insert_text (editable, text, strlen (text), &position);
  return position;
}

int gtk_widget_allocation_width_scm (GtkWidget *widget)
{
  return widget->allocation.width;
}

int gtk_widget_allocation_height_scm (GtkWidget *widget)
{
  return widget->allocation.height;
}

int gtk_widget_allocation_x_scm (GtkWidget *widget)
{
  return widget->allocation.x;
}

int gtk_widget_allocation_y_scm (GtkWidget *widget)
{
  return widget->allocation.y;
}

void* g_timer_fake_copy (void* ptr)
{
  return ptr;
}

void *gtk_no_copy (void *ptr)
{
  return NULL;
}

void gtk_no_free (void *ptr)
{
}



/* These SCM_PROCs are here to have them initialized in
   sgtk_init_gtk_support.  Having them in sgtk_init_substrate is wrong
   because then they are not guaranteed to end up in the (gtk gtk)
   module. */

SCM_PROC (s_gtk_callback_trampoline, "gtk-callback-trampoline", 0, 1, 0, sgtk_callback_trampoline);
SCM_PROC (s_gtk_standalone_p, "gtk-standalone?", 0, 0, 0, sgtk_standalone_p);

SCM sgtk_gtk_object_new (SCM, SCM);
SCM sgtk_gtk_object_set (SCM, SCM);
SCM sgtk_gtk_object_get (SCM, SCM);

SCM_PROC (s_gtk_object_new, "gtk-object-new", 1, 0, 1, sgtk_gtk_object_new);
SCM_PROC (s_gtk_object_set, "gtk-object-set", 1, 0, 1, sgtk_gtk_object_set);
SCM_PROC (s_gtk_object_get, "gtk-object-get", 2, 0, 0, sgtk_gtk_object_get);
SCM_PROC (s_gtk_widget_new, "gtk-widget-new", 1, 0, 1, sgtk_gtk_object_new);
SCM_PROC (s_gtk_widget_set, "gtk-widget-set", 1, 0, 1, sgtk_gtk_object_set);
SCM_PROC (s_gtk_widget_get, "gtk-widget-get", 2, 0, 0, sgtk_gtk_object_get);

SCM_PROC (s_gtk_threads_update, "gtk-threads-update", 0, 0, 0, sgtk_threads_update);

void
sgtk_init_gtk_support ()
{
#ifndef SCM_MAGIC_SNARFER
#ifndef MKDEP
#include "gtk-support.x"
#endif /* MKDEP */
#endif /* SCM_MAGIC_SNARFER */

  kw_type = scm_make_keyword_from_dash_symbol (sym_type);
  kw_flags = scm_make_keyword_from_dash_symbol (sym_flags);
}
