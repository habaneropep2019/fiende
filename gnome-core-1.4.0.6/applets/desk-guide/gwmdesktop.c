/* GwmDesktop - desktop widget
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
 * this code is loosely based on the original gnomepager_applet
 * implementation of The Rasterman (Carsten Haitzler) <raster@rasterman.com>
 */
#include	"gwmdesktop.h"

#include	<gtk/gtkprivate.h>
#include	"gwmthumbnail.h"



/* --- area bitmap --- */
#define xbm_area_width 4
#define xbm_area_height 4
static const gchar xbm_area_bits[] = {
  0x03, 0x03, 0x0C, 0x0C,       /* checker board */
  /* 0x08, 0x04, 0x02, 0x01, */ /* stripe */
};


/* --- signals --- */
enum {
  SIGNAL_CHECK_TASK,
  SIGNAL_LAST
};
typedef gboolean (*SignalCheckTask) (GtkObject *object,
                                     GwmhTask  *task,
                                     gpointer   user_data);


/* --- structures --- */
typedef struct
{
  gint  x, y;
  guint width, height;
} GrabArea;


/* --- prototypes --- */
static void     gwm_desktop_class_init     (GwmDesktopClass    *klass);
static void     gwm_desktop_init           (GwmDesktop         *desktop);
static void     gwm_desktop_destroy        (GtkObject          *object);
static gboolean gwm_desktop_desk_notifier  (gpointer            func_data,
					    GwmhDesk           *desk,
					    GwmhDeskInfoMask    change_mask);
static gboolean gwm_desktop_task_notifier  (gpointer            func_data,
					    GwmhTask           *task,
					    GwmhTaskNotifyType  ntype,
					    GwmhTaskInfoMask    imask);
static void     gwm_desktop_map            (GtkWidget          *widget);
static void     gwm_desktop_realize        (GtkWidget          *widget);
static void     gwm_desktop_size_allocate  (GtkWidget          *widget,
					    GtkAllocation      *allocation);
static void     gwm_desktop_unrealize      (GtkWidget          *widget);
static void     gwm_desktop_size_request   (GtkWidget          *widget,
					    GtkRequisition     *requisition);
static gboolean gwm_desktop_expose         (GtkWidget          *widget,
					    GdkEventExpose     *event);
static gboolean gwm_desktop_button_press   (GtkWidget          *widget,
					    GdkEventButton     *event);
static gboolean gwm_desktop_button_release (GtkWidget          *widget,
					    GdkEventButton     *event);
static gboolean gwmh_desktop_motion        (GtkWidget          *widget,
					    GdkEventMotion     *event);
static void	gwm_desktop_draw	   (GtkWidget	       *widget,
					    GdkRectangle       *area);
static void	gwm_desktop_draw_task_area (GwmDesktop	       *desktop,
					    GwmhTask	       *task);


/* --- static variables --- */
static gpointer         parent_class = NULL;
static GwmDesktopClass *gwm_desktop_class = NULL;
static guint            gwm_desktop_signals[SIGNAL_LAST] = { 0 };
static GQuark           quark_grab_area = 0;
static GQuark           quark_thumb_nail = 0;
static GSList	       *thumb_queue = NULL;
static guint		thumb_queue_id = 0;


/* --- functions --- */
GtkType
gwm_desktop_get_type (void)
{
  static GtkType desktop_type = 0;
  
  if (!desktop_type)
    {
      GtkTypeInfo desktop_info =
      {
        "GwmDesktop",
        sizeof (GwmDesktop),
        sizeof (GwmDesktopClass),
        (GtkClassInitFunc) gwm_desktop_class_init,
        (GtkObjectInitFunc) gwm_desktop_init,
        /* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      desktop_type = gtk_type_unique (GTK_TYPE_DRAWING_AREA, &desktop_info);
    }
  
  return desktop_type;
}

static void
gwm_desktop_marshal_check_task (GtkObject    *object,
                                GtkSignalFunc func,
                                gpointer      func_data,
                                GtkArg       *args)
{
  SignalCheckTask sfunc = (SignalCheckTask) func;
  gboolean *retval = GTK_RETLOC_BOOL (args[1]);
  
  *retval = sfunc (object, GTK_VALUE_POINTER (args[0]), func_data);
}

static void
gwm_desktop_class_init (GwmDesktopClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  
  object_class = GTK_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);
  
  gwm_desktop_class = class;
  parent_class = gtk_type_class (GTK_TYPE_DRAWING_AREA);
  
  quark_grab_area = g_quark_from_static_string ("gwm-grab-area");
  quark_thumb_nail = g_quark_from_static_string ("gwm-thumb-nail");
  
  object_class->destroy = gwm_desktop_destroy;
  
  widget_class->map = gwm_desktop_map;
  widget_class->realize = gwm_desktop_realize;
  widget_class->size_allocate = gwm_desktop_size_allocate;
  widget_class->unrealize = gwm_desktop_unrealize;
  widget_class->size_request = gwm_desktop_size_request;
  widget_class->expose_event = gwm_desktop_expose;
  widget_class->draw = gwm_desktop_draw;
  widget_class->button_press_event = gwm_desktop_button_press;
  widget_class->button_release_event = gwm_desktop_button_release;
  widget_class->motion_notify_event = gwmh_desktop_motion;
  
  class->orientation = GTK_ORIENTATION_HORIZONTAL;
  class->area_size = 22;
  class->thumb_timeout = 0;
  class->raised_grid = FALSE;
  class->move_to_frame_offset = FALSE;
  class->objects = NULL;
  class->check_task = NULL;
  
  gwm_desktop_signals[SIGNAL_CHECK_TASK] =
    gtk_signal_new ("check_task",
                    GTK_RUN_LAST,
                    object_class->type,
                    GTK_SIGNAL_OFFSET (GwmDesktopClass, check_task),
                    gwm_desktop_marshal_check_task,
                    GTK_TYPE_BOOL, 1, GTK_TYPE_POINTER);
  gtk_object_class_add_signals (object_class, gwm_desktop_signals, SIGNAL_LAST);
  
  gdk_rgb_init ();
}

static void
gwm_desktop_init (GwmDesktop *desktop)
{
  GtkWidget *widget = GTK_WIDGET (desktop);
  GwmhDesk *desk = gwmh_desk_get_config ();
  
  GTK_WIDGET_UNSET_FLAGS (desktop, GTK_NO_WINDOW);
  
  gwm_desktop_class->objects = g_slist_prepend (gwm_desktop_class->objects, desktop);
  
  desktop->index = 0;
  desktop->harea = 0;
  desktop->varea = 0;
  desktop->last_desktop = desk->current_desktop;
  desktop->task_list = NULL;
  
  desktop->area_width = 1;
  desktop->area_height = 1;
  
  desktop->bitmap = NULL;
  desktop->pixmap = NULL;
  desktop->tooltips = NULL;
  
  gtk_widget_set_events (widget,
                         GDK_BUTTON_PRESS_MASK |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_BUTTON2_MOTION_MASK |
                         GDK_EXPOSURE_MASK);
  
  gwmh_desk_notifier_add (gwm_desktop_desk_notifier, desktop);
  gwmh_task_notifier_add (gwm_desktop_task_notifier, desktop);
}

static void
gwm_desktop_destroy (GtkObject *object)
{
  GwmDesktop *desktop;
  
  g_return_if_fail (object != NULL);
  
  desktop = GWM_DESKTOP (object);
  
  gwm_desktop_class->objects = g_slist_remove (gwm_desktop_class->objects, desktop);
  
  gwmh_desk_notifier_remove_func (gwm_desktop_desk_notifier, desktop);
  gwmh_task_notifier_remove_func (gwm_desktop_task_notifier, desktop);
  
  desktop->index = 0;
  desktop->harea = 0;
  desktop->varea = 0;
  desktop->last_desktop = 0;
  g_list_free (desktop->task_list);
  desktop->task_list = NULL;
  desktop->grab_task = NULL;
  
  if (desktop->tooltips)
    {
      gtk_object_unref (GTK_OBJECT (desktop->tooltips));
      desktop->tooltips = NULL;
    }
  
  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gwm_desktop_realize (GtkWidget *widget)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  
  GTK_WIDGET_CLASS (parent_class)->realize (widget);
  
  desktop->bitmap = gdk_bitmap_create_from_data (widget->window,
                                                 xbm_area_bits,
                                                 xbm_area_width,
                                                 xbm_area_height);
  desktop->pixmap = gdk_pixmap_new (widget->window,
				    widget->allocation.width,
				    widget->allocation.height,
				    -1);
}

static void
gwm_desktop_map (GtkWidget *widget)
{
  gtk_widget_queue_draw (widget);	/* redraw the pixmap */
  
  GTK_WIDGET_CLASS (parent_class)->map (widget);
}

static void
gwm_desktop_size_allocate (GtkWidget     *widget,
                           GtkAllocation *allocation)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  
  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
  
  if (GTK_WIDGET_REALIZED (desktop))
    {
      gdk_pixmap_unref (desktop->pixmap);
      desktop->pixmap = gdk_pixmap_new (widget->window,
                                        widget->allocation.width,
                                        widget->allocation.height,
                                        -1);
      gtk_widget_queue_draw (widget);
    }
}

static void
gwm_desktop_unrealize (GtkWidget *widget)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  
  gdk_pixmap_unref (desktop->pixmap);
  desktop->pixmap = NULL;
  gdk_pixmap_unref (desktop->bitmap);
  desktop->bitmap = NULL;
  
  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

void
gwm_desktop_class_config (GwmDesktopClass *class,
                          guint            thumb_timeout,
                          GtkOrientation   orientation,
                          guint            area_size,
                          gboolean         raised_grid,
                          gboolean         move_to_frame_offset)
{
  g_return_if_fail (GWM_IS_DESKTOP_CLASS (class));
  
  raised_grid = raised_grid ? TRUE : FALSE;
  orientation = CLAMP (orientation, GTK_ORIENTATION_HORIZONTAL, GTK_ORIENTATION_VERTICAL);
  move_to_frame_offset = move_to_frame_offset ? TRUE : FALSE;
  if (thumb_timeout != class->thumb_timeout ||
      orientation != class->orientation ||
      area_size != class->area_size ||
      raised_grid != class->raised_grid ||
      move_to_frame_offset != class->move_to_frame_offset)
    {
      GSList *slist;

      if (thumb_queue_id)
	{
	  g_source_remove (thumb_queue_id);
	  thumb_queue_id = 0;
	}
      class->thumb_timeout = thumb_timeout;
      class->orientation = orientation;
      class->area_size = area_size;
      class->raised_grid = raised_grid;
      class->move_to_frame_offset = move_to_frame_offset;
      
      for (slist = class->objects; slist; slist = slist->next)
        {
          GtkWidget *widget = slist->data;
          GwmDesktop *desktop = GWM_DESKTOP (widget);
          
          if (!GTK_WIDGET_REALIZED (desktop))
            continue;
          
          gtk_widget_queue_resize (widget);
        }
    }
}

static void
gwm_desktop_size_request (GtkWidget      *widget,
                          GtkRequisition *requisition)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmDesktopClass *class = GWM_DESKTOP_GET_CLASS (desktop);
  GwmhDesk *desk = gwmh_desk_get_config ();
  gint xthick = widget->style->klass->xthickness;
  gint ythick = widget->style->klass->ythickness;
  gfloat ratio;
  gint area_width, area_height;
  
  ratio = ((gfloat) gdk_screen_width ()) / ((gfloat) gdk_screen_height ());
  
  /* frame */
  requisition->width = 2 * xthick;
  requisition->height = 2 * ythick;
  /* areas */
  if (class->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      area_height = class->area_size;
      area_width = ratio * class->area_size + 0.5;
    }
  else
    {
      area_width = class->area_size;
      area_height = class->area_size / ratio + 0.5;
    }
  requisition->width += area_width * desk->n_hareas;
  requisition->height += area_height * desk->n_vareas;
}

GtkWidget*
gwm_desktop_new (guint        index,
                 GtkTooltips *tooltips)
{
  GtkWidget *desktop;
  
  if (tooltips)
    g_return_val_if_fail (GTK_IS_TOOLTIPS (tooltips), NULL);
  
  desktop = gtk_widget_new (GWM_TYPE_DESKTOP, NULL);
  
  if (tooltips)
    {
      gtk_object_ref (GTK_OBJECT (tooltips));
      GWM_DESKTOP (desktop)->tooltips = tooltips;
    }
  
  gwm_desktop_set_index (GWM_DESKTOP (desktop), index);
  
  return desktop;
}

void
gwm_desktop_set_index (GwmDesktop *desktop,
                       guint       index)
{
  GtkWidget *widget = GTK_WIDGET (desktop);
  GwmhDesk *desk = gwmh_desk_get_config ();
  GList *node;
  
  g_return_if_fail (GWM_IS_DESKTOP (desktop));
  g_return_if_fail (index < desk->n_desktops);
  
  desktop->index = index;
  gwmh_desk_guess_desktop_area (desktop->index,
                                &desktop->harea,
                                &desktop->varea);
  desktop->last_desktop = desk->current_desktop;
  
  if (desktop->tooltips)
    gtk_tooltips_set_tip (desktop->tooltips,
                          widget,
                          desk->desktop_names[index],
                          NULL);
  
  
  g_list_free (desktop->task_list);
  desktop->task_list = NULL;
  desktop->grab_task = NULL;
  for (node = desk->client_list; node; node = node->next)
    {
      GwmhTask *task = node->data;
      
      if (task->desktop == desktop->index)
        desktop->task_list = g_list_prepend (desktop->task_list, task);
    }
  
  gtk_widget_queue_resize (widget);
}

static gboolean
gwm_desktop_desk_notifier (gpointer         func_data,
                           GwmhDesk        *desk,
                           GwmhDeskInfoMask change_mask)
{
  GwmDesktop *desktop = GWM_DESKTOP (func_data);
  GtkWidget *widget = GTK_WIDGET (desktop);
  
  if (desktop->last_desktop != desk->current_desktop)
    {
      if (desktop->last_desktop == desktop->index)
        gtk_widget_queue_draw (widget);
      desktop->last_desktop = desk->current_desktop;
      if (desktop->last_desktop == desktop->index)
        gtk_widget_queue_draw (widget);
    }
  
  if (change_mask & (GWMH_DESK_INFO_N_DESKTOPS |
                     GWMH_DESK_INFO_N_AREAS |
                     GWMH_DESK_INFO_CLIENT_LIST))
    gwm_desktop_set_index (desktop, MIN (desktop->index, desk->n_desktops - 1));
  else
    {
      if ((change_mask & GWMH_DESK_INFO_DESKTOP_NAMES) &&
          desktop->tooltips)
        gtk_tooltips_set_tip (desktop->tooltips,
                              widget,
                              desk->desktop_names[desktop->index],
                              NULL);
      
      if (change_mask & GWMH_DESK_INFO_CURRENT_AREA)
        {
	  guint harea = 0, varea = 0;
	  
	  gwmh_desk_guess_desktop_area (desktop->index, &harea, &varea);
	  if (harea != desktop->harea ||
	      varea != desktop->varea)
	    {
	      desktop->harea = harea;
	      desktop->varea = varea;
	      gtk_widget_queue_draw (widget);
	    }
        }
    }
  
  return TRUE;
}

static gboolean
gwm_desktop_task_notifier (gpointer           func_data,
                           GwmhTask          *task,
                           GwmhTaskNotifyType ntype,
                           GwmhTaskInfoMask   imask)
{
  GwmDesktop *desktop = GWM_DESKTOP (func_data);
  GtkWidget *widget = GTK_WIDGET (desktop);
  
  if (ntype == GWMH_NOTIFY_NEW)
    {
      if (task->desktop == desktop->index)
        {
          desktop->task_list = g_list_append (desktop->task_list, task);
          desktop->grab_task = NULL;
          gtk_widget_queue_draw (widget);
        }
    }
  else if (ntype == GWMH_NOTIFY_INFO_CHANGED && imask)
    {
      if (task->desktop == desktop->index)
        {
          if (imask & GWMH_TASK_INFO_DESKTOP)
            {
              desktop->task_list = g_list_append (desktop->task_list, task);
              desktop->grab_task = NULL;
            }
          gtk_widget_queue_draw (widget);
        }
      else if ((imask & GWMH_TASK_INFO_DESKTOP) &&
               task->last_desktop == desktop->index)
        {
          desktop->task_list = g_list_remove (desktop->task_list, task);
          desktop->grab_task = NULL;
          gtk_widget_queue_draw (widget);
        }
    }
  else if (ntype == GWMH_NOTIFY_DESTROY)
    {
      if (task->desktop == desktop->index)
        {
          desktop->task_list = g_list_remove (desktop->task_list, task);
          desktop->grab_task = NULL;
          gtk_widget_queue_draw (widget);
        }
    }
  
  return TRUE;
}

static gboolean
gwm_desktop_button_press (GtkWidget      *widget,
                          GdkEventButton *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmhDesk *desk = gwmh_desk_get_config ();
  gint xthick = widget->style->klass->xthickness;
  gint ythick = widget->style->klass->ythickness;
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;
  gint x = event->x;
  gint y = event->y;
  GList *node;
  
  desktop->grab_task = NULL;
  
  if (x < xthick || x >= width - xthick ||
      y < ythick || y >= height - ythick)
    return FALSE;
  if (event->button >= 3)
    return FALSE;
  
  if (event->button == 1)
    {
      gint tx = x, ty = y, tw = width, th = height;
      
      tx -= xthick;
      ty -= ythick;
      tw -= xthick * 2;
      th -= ythick * 2;
      tx /= (tw / desk->n_hareas);
      ty /= (th / desk->n_vareas);
      
      if (desk->current_desktop != desktop->index ||
	  desk->current_harea != tx ||
	  desk->current_varea != ty)
	{
	  gwmh_desk_set_current_area (desktop->index, tx, ty);
	  return TRUE;
	}
    }
  
  for (node = g_list_last (desktop->task_list); node; node = node->prev)
    {
      GwmhTask *task = node->data;
      GrabArea *grab_area = gwmh_task_get_qdata (task, quark_grab_area);
      
      if (grab_area &&
          (x >= grab_area->x &&
           x < grab_area->x + grab_area->width) &&
          (y >= grab_area->y &&
           y < grab_area->y + grab_area->height))
        {
          if (event->button == 1)
            gwmh_task_show (task);
          if (event->button == 2)
            {
              gwmh_task_raise (task);
	      if (event->type != GDK_2BUTTON_PRESS)
		{
		  desktop->grab_task = task;
		  desktop->x_comp = desktop->x_spixels * (grab_area->x - x);
		  desktop->y_comp = desktop->y_spixels * (grab_area->y - y);
		  if (gwm_desktop_class->move_to_frame_offset)
		    {
		      desktop->x_comp += task->win_x - task->frame_x;
		      desktop->y_comp += task->win_y - task->frame_y;
		    }
		}
	      else /* GDK_2BUTTON_PRESS */
		{
		  gint w = gdk_screen_width ();
		  gint h = gdk_screen_height ();
		  gint x, y;
		  
		  desktop->grab_task = NULL;
		  x = (task->win_x - (task->win_x < 0) * w + task->win_width / 2) / w;
		  y = (task->win_y - (task->win_y < 0) * h + task->win_height / 2) / h;
		  if (gwm_desktop_class->move_to_frame_offset)
		    gdk_window_move (task->gdkwindow,
				     x * w + (task->win_x - task->frame_x),
				     y * h + (task->win_y - task->frame_y));
		  else
		    gdk_window_move (task->gdkwindow,
				     x * w,
				     y * h);
		  gdk_flush ();
		}
            }
          break;
        }
    }
  
  return TRUE;
}

static gboolean
gwmh_desktop_motion (GtkWidget      *widget,
                     GdkEventMotion *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmhTask *task;
  gint x = event->x, y = event->y;
  
  x = CLAMP (x, -5, widget->allocation.width + 4);
  y = CLAMP (y, -5, widget->allocation.height + 4);
  
  task = desktop->grab_task;
  
  if (task && !GWMH_TASK_UPDATE_QUEUED (task))
    {
      gdk_window_move (task->gdkwindow,
		       desktop->x_comp +
                       desktop->x_spixels * (x - desktop->x_origin),
                       desktop->y_comp +
                       desktop->y_spixels * (y - desktop->y_origin));
      gdk_flush ();
    }
  
  return TRUE;
}

static gboolean
gwm_desktop_button_release (GtkWidget       *widget,
			    GdkEventButton  *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  
  desktop->grab_task = NULL;
  
  return TRUE;
}

static gboolean
gwm_desktop_expose (GtkWidget      *widget,
                    GdkEventExpose *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  
  gdk_draw_pixmap (widget->window,
		   widget->style->fg_gc[GTK_STATE_NORMAL],
		   desktop->pixmap,
		   0, 0, 0, 0,
		   -1, -1);
  
  return TRUE;
}

static gboolean
thumb_queue_step (gpointer data)
{
  GDK_THREADS_ENTER ();

  if (gtk_grab_get_current () == NULL)
    {
      GwmDesktopClass *class;
      GwmhDesk *desk;
      GSList *new_queue = NULL;
      
      class = gtk_type_class (GWM_TYPE_DESKTOP);
      desk = gwmh_desk_get_config ();
      while (thumb_queue)
	{
	  GSList *node = thumb_queue;
	  GwmThumbNail *nail = node->data;
	  GwmhTask *task = nail->user_data;
	  
	  thumb_queue = node->next;
	  node->next = new_queue;
	  new_queue = node;
	  
	  if (task->desktop == desk->current_desktop &&
	      !GWMH_TASK_ICONIFIED (task) && !GWMH_TASK_SHADED (task) &&
	      gwm_thumb_nail_update_drawable (nail, task->gdkwindow, task->win_x, task->win_y))
	    {
	      GSList *slist;
	      
	      for (slist = class->objects; slist; slist = slist->next)
		{
		  GwmDesktop *desktop = slist->data;
		  
		  if (desktop->index == task->desktop)
		    gwm_desktop_draw_task_area (desktop, task);
		}
	      break;
	    }
	}
      thumb_queue = g_slist_concat (thumb_queue, g_slist_reverse (new_queue));
    }

  GDK_THREADS_LEAVE ();
  
  return TRUE;
}

void
gwm_desktop_class_reload_thumbs (void)
{
  GSList *node;
  
  for (node = thumb_queue; node; node = node->next)
    gwm_thumb_nail_flag_reload (node->data);
}

static void
thumb_nail_destroy (gpointer data)
{
  GwmThumbNail *nail = data;
  
  gwm_thumb_nail_destroy (nail);
}

static void
task_remove_thumb (gpointer      data,
		   GwmThumbNail *nail)
{
  GwmhTask *task = data;

  if (!g_slist_find (thumb_queue, nail)) /* FIXME */
    g_error ("removing thumbnail from queue failed");
  thumb_queue = g_slist_remove (thumb_queue, nail);
  gwmh_task_steal_qdata (task, quark_thumb_nail);
}

static GwmThumbNail*
gwm_desktop_get_thumb_nail (GwmDesktop *desktop,
			    GwmhTask   *task,
			    guint       width,
			    guint       height)
{
  GwmDesktopClass *class = GWM_DESKTOP_GET_CLASS (desktop);
  GwmThumbNail *nail;
  
  if (!thumb_queue_id && class->thumb_timeout)
    thumb_queue_id = g_timeout_add_full (G_PRIORITY_LOW * 2,
					 class->thumb_timeout,
					 thumb_queue_step,
					 NULL,
					 NULL);
  nail = gwmh_task_get_qdata (task, quark_thumb_nail);
  if (!nail)
    {
      GtkStyle *style = GTK_WIDGET (desktop)->style;
      guint color = 0;
      
      color |= (style->bg[GTK_STATE_NORMAL].red >> 8) << 16;
      color |= (style->bg[GTK_STATE_NORMAL].green >> 8) << 8;
      color |= style->bg[GTK_STATE_NORMAL].blue >> 8;
      
      nail = gwm_thumb_nail_new (color, width, height, (glong) desktop, task, task_remove_thumb);
      if (nail)
	{
	  thumb_queue = g_slist_append (thumb_queue, nail);
	  gwmh_task_set_qdata_full (task, quark_thumb_nail, nail, thumb_nail_destroy);
	}
    }
  else
    gwm_thumb_nail_grow (nail, width, height, (glong) desktop);
  
  return nail;
}

static inline void
gwm_desktop_draw_task (GwmDesktop *desktop,
		       GwmhTask   *task,
		       GrabArea   *grab_area,
		       gboolean    update_grab_area)
{
  GtkWidget *widget = GTK_WIDGET (desktop);
  GdkWindow *drawable = desktop->pixmap;
  GtkStyle *style = widget->style;
  
  if (update_grab_area)
    {
      gfloat swidth = gdk_screen_width ();
      gfloat sheight = gdk_screen_height ();
      gfloat area_width = desktop->area_width;
      gfloat area_height = desktop->area_height;
      gint xthick = widget->style->klass->xthickness;
      gint ythick = widget->style->klass->ythickness;
      gint task_x, task_y;
      gint x, y, x2, y2;
      
      /* offset to area */
      x = xthick + task->harea * area_width;
      y = ythick + task->varea * area_height;
      x2 = x;
      y2 = y;
      /* offset within area */
      gwmh_task_get_frame_area_pos (task, &task_x, &task_y);
      x += task_x * area_width / swidth;
      y += task_y * area_height / sheight;
      x2 += (task_x + task->frame_width - 1) * area_width / swidth;
      y2 += (task_y + task->frame_height - 1) * area_height / sheight;
      grab_area->x = x;
      grab_area->y = y;
      grab_area->width = x2 - x + 1;
      grab_area->height = y2 - y + 1;
    }
  
#if 0
  g_print ("draw task %p %d, %d, %d, %d\n", task, grab_area->x, grab_area->y, grab_area->width, grab_area->height);
#endif
  
  if (grab_area->width > 2 && grab_area->height > 2)
    {
      GwmThumbNail *nail = gwm_desktop_get_thumb_nail (desktop, task, grab_area->width - 2, grab_area->height - 2);
      
      if (nail)
	{
	  GdkPixbuf *pixbuf;
	  
	  if (gdk_pixbuf_get_width (nail->pixbuf) != grab_area->width - 2 ||
	      gdk_pixbuf_get_height (nail->pixbuf) != grab_area->height - 2)
	    pixbuf = gdk_pixbuf_scale_simple (nail->pixbuf,
					      grab_area->width - 2,
					      grab_area->height - 2,
					      GDK_INTERP_NEAREST);
	  else
	    pixbuf = nail->pixbuf;
	  
	  gdk_draw_rgb_image (drawable,
			      style->black_gc,
			      grab_area->x + 1, grab_area->y + 1,
			      grab_area->width - 2, grab_area->height - 2,
			      GDK_RGB_DITHER_NONE,
			      gdk_pixbuf_get_pixels (pixbuf),
			      gdk_pixbuf_get_rowstride (pixbuf));
	  if (pixbuf != nail->pixbuf)
	    gdk_pixbuf_unref (pixbuf);
	}
      gtk_draw_shadow (style, drawable,
		       GWMH_TASK_FOCUSED (task) ? GTK_STATE_PRELIGHT : GTK_STATE_NORMAL,
		       GWMH_TASK_FOCUSED (task) ? GTK_SHADOW_OUT : GTK_SHADOW_ETCHED_IN,
		       grab_area->x,
		       grab_area->y,
		       grab_area->width,
		       grab_area->height);
    }
  else
    gtk_draw_box (style, drawable,
		  GWMH_TASK_FOCUSED (task) ? GTK_STATE_PRELIGHT : GTK_STATE_NORMAL,
		  GWMH_TASK_FOCUSED (task) ? GTK_SHADOW_OUT : GTK_SHADOW_ETCHED_IN,
		  grab_area->x,
		  grab_area->y,
		  grab_area->width,
		  grab_area->height);
}

static void
gwm_desktop_draw_task_area (GwmDesktop *desktop,
			    GwmhTask   *task)
{
  GrabArea *grab_area = gwmh_task_get_qdata (task, quark_grab_area);
  GList *node = desktop->task_list;
  
  while (node && node->data != task)
    node = node->next;
  if (grab_area && node)
    {
      GwmDesktopClass *class = GWM_DESKTOP_GET_CLASS (desktop);
      GtkWidget *widget = GTK_WIDGET (desktop);
      GtkStyle *style = widget->style;
      GdkWindow *drawable = desktop->pixmap;
      GwmhDesk *desk = gwmh_desk_get_config ();
      gboolean current = desk->current_desktop == desktop->index;
      gint bx = grab_area->x, by = grab_area->y;
      gint bwidth = grab_area->width, bheight = grab_area->height;
      gint task_desktop = task->desktop;
      
      gwm_desktop_draw_task (desktop, task, grab_area, FALSE);
      
      for (; node; node = node->next)
	{
	  task = node->data;
	  if (task->desktop != task_desktop)
	    continue;
	  grab_area = gwmh_task_get_qdata (task, quark_grab_area);
	  /* check for overlap */
	  if (!grab_area ||
	      grab_area->x >= bx + bwidth ||
	      grab_area->y >= by + bheight ||
	      grab_area->x + grab_area->width <= bx ||
	      grab_area->y + grab_area->height <= by)
	    continue;
	  /* widen bounding rectangle */
	  bwidth = MAX (bx + bwidth, grab_area->x + grab_area->width) - bx;
	  bheight = MAX (by + bheight, grab_area->y + grab_area->height) - by;
	  bx = MIN (bx, grab_area->x);
	  by = MIN (by, grab_area->y);
	  /* redraw on top */
	  gwm_desktop_draw_task (desktop, task, grab_area, FALSE);
	}
      
      /* draw grid above */
      if (class->raised_grid)
	{
	  gint xthick = widget->style->klass->xthickness;
	  gint ythick = widget->style->klass->ythickness;
	  gint x, y;

	  /* draw grid */
	  for (x = 1; x < desk->n_hareas; x++)
	    gtk_draw_vline (style, drawable,
			    GTK_WIDGET_STATE (widget),
			    ythick,
			    ythick + desktop->area_height * desk->n_vareas - ythick / 2,
			    xthick + x * desktop->area_width - xthick / 2);
	  for (y = 1; y < desk->n_vareas; y++)
	    gtk_draw_hline (style, drawable,
			    GTK_WIDGET_STATE (widget),
			    xthick,
			    xthick + desktop->area_width * desk->n_hareas - xthick / 2,
			    ythick + y * desktop->area_height - ythick / 2);
	}
      
      /* draw border */
      gtk_draw_shadow (style,
		       drawable,
		       GTK_STATE_ACTIVE,
		       current ? GTK_SHADOW_IN : GTK_SHADOW_ETCHED_IN,
		       0, 0,
		       -1, -1);
      
      /* and blast to screen */
      gdk_draw_pixmap (widget->window,
		       widget->style->fg_gc[GTK_STATE_NORMAL],
		       desktop->pixmap,
		       bx, by, bx, by,
		       bwidth, bheight);
    }
}

static void
gwm_desktop_draw (GtkWidget    *widget,
		  GdkRectangle *uneeded_area)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GtkObject *object = GTK_OBJECT (desktop);
  GwmDesktopClass *class = GWM_DESKTOP_GET_CLASS (desktop);
  GdkWindow *drawable = desktop->pixmap;
  GtkStyle *style = widget->style;
  gint xthick = widget->style->klass->xthickness;
  gint ythick = widget->style->klass->ythickness;
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;
  gfloat swidth = gdk_screen_width ();
  gfloat sheight = gdk_screen_height ();
  GwmhDesk *desk = gwmh_desk_get_config ();
  gfloat area_width = ((gfloat) (width - 2 * xthick)) / ((gfloat) desk->n_hareas);
  gfloat area_height = ((gfloat) (height - 2 * ythick)) / ((gfloat) desk->n_vareas);
  gboolean current = desk->current_desktop == desktop->index;
  GList *node;
  gint x, y;
  
  /* wrongly sized widget? */
  if (area_width < 1 || area_height < 1)
    return;
  
  desktop->x_spixels = swidth / area_width;
  desktop->y_spixels = sheight / area_height;
  desktop->area_width = area_width;
  desktop->area_height = area_height;
  
  /* clear out */
  gtk_draw_box (style,
                drawable,
                GTK_STATE_ACTIVE,
                current ? GTK_SHADOW_OUT : GTK_SHADOW_ETCHED_IN,
                0, 0,
                -1, -1);
  
  /* create gc for current area and draw its background */
  if (desk->n_hareas * desk->n_vareas > 1)
    {
      GdkGC *gc = current ? style->bg_gc[GTK_STATE_SELECTED] : style->base_gc[GTK_STATE_NORMAL];
      
      gdk_gc_set_stipple (gc, desktop->bitmap);
      gdk_gc_set_clip_mask (gc, NULL);
      gdk_gc_set_fill (gc, GDK_STIPPLED);
      gdk_gc_set_ts_origin (gc, 0, 0);
      x = xthick + desktop->harea * area_width;
      y = ythick + desktop->varea * area_height;
      gdk_draw_rectangle (drawable, gc, TRUE,
                          x, y,
                          area_width,
                          area_height);
      gdk_gc_set_fill (gc, GDK_SOLID);
      desktop->x_origin = x;
      desktop->y_origin = y;
    }
  
  /* draw grid, take 1 */
  if (!class->raised_grid)
    {
      /* draw grid */
      for (x = 1; x < desk->n_hareas; x++)
        gtk_draw_vline (style, drawable,
                        GTK_WIDGET_STATE (widget),
                        ythick,
                        ythick + area_height * desk->n_vareas - ythick / 2,
                        xthick + x * area_width - xthick / 2);
      for (y = 1; y < desk->n_vareas; y++)
        gtk_draw_hline (style, drawable,
                        GTK_WIDGET_STATE (widget),
                        xthick,
                        xthick + area_width * desk->n_hareas - xthick / 2,
                        ythick + y * area_height - ythick / 2);
    }
  
  /* draw tasks */
  desktop->task_list = gwmh_task_list_stack_sort (desktop->task_list);
  for (node = desktop->task_list; node; node = node->next)
    {
      GwmhTask *task = node->data;
      GrabArea *grab_area = gwmh_task_get_qdata (task, quark_grab_area);
      gboolean show_task = TRUE;
      
      gtk_signal_emit (object, gwm_desktop_signals[SIGNAL_CHECK_TASK], task, &show_task);
      if (!show_task)
        {
          if (grab_area)
	    gwmh_task_set_qdata_full (task, quark_grab_area, NULL, NULL);
          continue;
        }
      if (!grab_area)
	{
	  grab_area = g_new (GrabArea, 1);
	  gwmh_task_set_qdata_full (task, quark_grab_area, grab_area, g_free);
	}
      
      gwm_desktop_draw_task (desktop, task, grab_area, TRUE);
    }
  
  /* draw grid, take 2 */
  if (class->raised_grid)
    {
      /* draw grid */
      for (x = 1; x < desk->n_hareas; x++)
        gtk_draw_vline (style, drawable,
                        GTK_WIDGET_STATE (widget),
                        ythick,
                        ythick + area_height * desk->n_vareas - ythick / 2,
                        xthick + x * area_width - xthick / 2);
      for (y = 1; y < desk->n_vareas; y++)
        gtk_draw_hline (style, drawable,
                        GTK_WIDGET_STATE (widget),
                        xthick,
                        xthick + area_width * desk->n_hareas - xthick / 2,
                        ythick + y * area_height - ythick / 2);
    }
  
  /* draw border */
  gtk_draw_shadow (style,
                   drawable,
                   GTK_STATE_ACTIVE,
                   current ? GTK_SHADOW_IN : GTK_SHADOW_ETCHED_IN,
                   0, 0,
                   -1, -1);
  
  gdk_draw_pixmap (widget->window,
		   widget->style->fg_gc[GTK_STATE_NORMAL],
		   desktop->pixmap,
		   0, 0, 0, 0,
		   -1, -1);
}
