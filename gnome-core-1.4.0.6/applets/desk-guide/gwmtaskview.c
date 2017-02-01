/* GwmTaskView - GwmhTask editor widget
 * Copyright (C) 2000 Tim Janik
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
 */
#include	"gwmtaskview.h"


/* --- signals --- */
enum {
  SIGNAL_POPDOWN_REQUEST,
  SIGNAL_LAST
};


/* --- prototypes --- */
static void	gwm_task_view_class_init	(GwmTaskViewClass	*klass);
static void	gwm_task_view_init		(GwmTaskView		*task_view,
						 GwmTaskViewClass	*real_class);
static void	gwm_task_view_destroy		(GtkObject		*object);
static void	gwm_task_view_finalize		(GtkObject		*object);
static void	gwm_task_view_task_update_row   (GwmTaskView		*task_view,
						 GwmhTask    		*task);
static gboolean gwm_task_view_update_task	(gpointer                func_data,
						 GwmhTask               *task,
						 GwmhTaskNotifyType      ntype,
						 GwmhTaskInfoMask        imask);
static gboolean gwm_task_view_clist_brelease	(GwmTaskView		*task_view,
						 GdkEventButton         *event);


/* --- task clist --- */
enum {
  CLIST_NAME,
  CLIST_STATE,
  CLIST_DESKTOP,
  CLIST_VIEWPORT,
  CLIST_LAYER,
  CLIST_N_COLUMNS
};
static gchar *clist_titles[CLIST_N_COLUMNS] = {
  "Name",
  "State",
  "Desktop",
  "Viewport",
  "Layer",
};


/* --- static variables --- */
static gpointer		 parent_class = NULL;
static GwmTaskViewClass *gwm_task_view_class = NULL;
static guint             task_view_signals[SIGNAL_LAST] = { 0 };


/* --- functions --- */
GtkType
gwm_task_view_get_type (void)
{
  static GtkType task_view_type = 0;
  
  if (!task_view_type)
    {
      GtkTypeInfo task_view_info =
      {
	"GwmTaskView",
	sizeof (GwmTaskView),
	sizeof (GwmTaskViewClass),
	(GtkClassInitFunc) gwm_task_view_class_init,
	(GtkObjectInitFunc) gwm_task_view_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };
      
      task_view_type = gtk_type_unique (GTK_TYPE_HBOX, &task_view_info);
    }
  
  return task_view_type;
}

static void
gwm_task_view_class_init (GwmTaskViewClass *class)
{
  GtkObjectClass *object_class;
  
  object_class = GTK_OBJECT_CLASS (class);
  
  gwm_task_view_class = class;
  parent_class = gtk_type_class (GTK_TYPE_HBOX);
  
  object_class->destroy = gwm_task_view_destroy;
  object_class->finalize = gwm_task_view_finalize;

  task_view_signals[SIGNAL_POPDOWN_REQUEST] =
    gtk_signal_new ("popdown-request",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GwmTaskViewClass, popdown_request),
		    gtk_marshal_NONE__NONE,
		    GTK_TYPE_NONE, 0);
  gtk_object_class_add_signals (object_class, task_view_signals, SIGNAL_LAST);
}

static void
gwm_task_view_init (GwmTaskView      *task_view,
		    GwmTaskViewClass *real_class)
{
  task_view->task = NULL;
  task_view->task_clist = NULL;
  gtk_container_set_resize_mode (GTK_CONTAINER (task_view), GTK_RESIZE_QUEUE);
  gwmh_task_notifier_add (gwm_task_view_update_task, task_view);
}

static void
gwm_task_view_destroy_contents (GwmTaskView *task_view)
{
  gtk_container_foreach (GTK_CONTAINER (task_view), (GtkCallback) gtk_widget_destroy, NULL);
}

static void
gwm_task_view_destroy (GtkObject *object)
{
  GwmTaskView *task_view;
  
  g_return_if_fail (object != NULL);
  
  task_view = GWM_TASK_VIEW (object);
  
  gwm_task_view_destroy_contents (task_view);
  
  gwm_task_view_set_task (task_view, NULL);

  gwmh_task_notifier_remove_func (gwm_task_view_update_task, task_view);

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gwm_task_view_finalize (GtkObject *object)
{
  GwmTaskView *task_view;
  
  g_return_if_fail (object != NULL);
  
  task_view = GWM_TASK_VIEW (object);
  
  GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gwm_task_view_task_update_row (GwmTaskView *task_view,
			       GwmhTask    *task)
{
  GtkCList *clist;
  gint row;

#define BLANK_FLAG '.'
  
  clist = GTK_CLIST (task_view->task_clist);
  row = gtk_clist_find_row_from_data (clist, task);
  if (row >= 0)
    {
      gchar *string, mask[64];
      
      gtk_clist_set_text (clist, row, CLIST_NAME, task->name);
      /* state */
      string = mask;
      *(string++) = GWMH_TASK_ICONIFIED (task) ? 'I' : BLANK_FLAG;
      *(string++) = GWMH_TASK_STICKY (task) ? 'T' : BLANK_FLAG;
      *(string++) = GWMH_TASK_SHADED (task) ? 'S' : BLANK_FLAG;
      *(string++) = GWMH_TASK_MINIMIZED (task) ? 'N' : BLANK_FLAG;
      *(string++) = (GWMH_TASK_MAXIMIZED_VERT (task) && GWMH_TASK_MAXIMIZED_HORIZ (task) ? 'M'
		     : GWMH_TASK_MAXIMIZED_HORIZ (task) ? 'H'
		     : GWMH_TASK_MAXIMIZED_VERT (task) ? 'V'
		     : BLANK_FLAG);
      *(string++) = GWMH_TASK_HIDDEN (task) ? 'C' : BLANK_FLAG; /* concealed */
      *(string++) = GWMH_TASK_DO_NOT_COVER (task) ? 'U' : BLANK_FLAG;
      *(string++) = GWMH_TASK_FOCUSED (task) ? 'F' : BLANK_FLAG;
      *(string++) = 0;
      gtk_clist_set_text (clist, row, CLIST_STATE, mask);
      string = g_strdup_printf ("%u", task->desktop);
      gtk_clist_set_text (clist, row, CLIST_DESKTOP, string);
      g_free (string);
      string = g_strdup_printf ("%ux%u", task->harea, task->varea);
      gtk_clist_set_text (clist, row, CLIST_VIEWPORT, string);
      g_free (string);
      string = g_strdup_printf ("%u", task->layer);
      gtk_clist_set_text (clist, row, CLIST_LAYER, string);
      g_free (string);
    }
}

static gboolean
gwm_task_view_update_task (gpointer           func_data,
			   GwmhTask          *task,
			   GwmhTaskNotifyType ntype,
			   GwmhTaskInfoMask   imask)
{
  GwmTaskView *task_view = GWM_TASK_VIEW (func_data);
  
  if (ntype == GWMH_NOTIFY_NEW)
    {
      static gchar *text[CLIST_N_COLUMNS] = { NULL, };
      GtkCList *clist = GTK_CLIST (task_view->task_clist);
      gint row = gtk_clist_insert (clist, GWMH_TASK_ICONIFIED (task) ? 0 : -1, text);
      
      gtk_clist_set_row_data (clist, row, task);
      gwm_task_view_task_update_row (task_view, task);
      gwm_task_view_select_task (task_view, task);
    }
  else if (ntype == GWMH_NOTIFY_INFO_CHANGED)
    gwm_task_view_task_update_row (task_view, task);
  else if (ntype == GWMH_NOTIFY_DESTROY)
    {
      GtkCList *clist = GTK_CLIST (task_view->task_clist);
      gint row = gtk_clist_find_row_from_data (clist, task);

      if (row >= 0)
	gtk_clist_remove (clist, row);
    }
  
  return TRUE;
}

void
gwm_task_view_update_list (GwmTaskView *task_view)
{
  GtkCList *clist;
  GList *list;

  g_return_if_fail (GWM_IS_TASK_VIEW (task_view));

  clist = GTK_CLIST (task_view->task_clist);

  gtk_clist_freeze (clist);
  gtk_clist_clear (clist);
  
  for (list = gwmh_task_list_get (); list; list = list->next)
    gwm_task_view_update_task (task_view, list->data, GWMH_NOTIFY_NEW, 0);

  gtk_clist_thaw (clist);

#if 0
  {
    static gchar *text[CLIST_N_COLUMNS] = { NULL, };
    gint row;
    GtkCList *clist = GTK_CLIST (task_view->task_clist);
    
    row = gtk_clist_insert (clist, -1, text);
    gtk_clist_set_row_data (clist, row, task);
    gwm_task_view_task_update_row (task_view, task);
    
    
    gint row;
    GtkCList *clist = GTK_CLIST (task_view->task_clist);
    
    row = gtk_clist_find_row_from_data (clist, task);
    if (row >= 0)
      gtk_clist_remove (clist, row);
  }
#endif
}

static void
clist_adjust_visibility (GtkCList *clist)
{
  if (clist->selection)
    {
      gint row = GPOINTER_TO_INT (clist->selection->data);
      
      if (gtk_clist_row_is_visible (clist, row) != GTK_VISIBILITY_FULL)
	gtk_clist_moveto (clist, row, -1, 0.5, 0);
    }
}

static void
scrolled_window_size_request (GtkWidget      *scrolled_window,
			      GtkRequisition *requisition)
{
  if (GTK_BIN (scrolled_window)->child)
    {
      GtkRequisition child_requisition;

      gtk_widget_get_child_requisition (GTK_BIN (scrolled_window)->child, &child_requisition);
      requisition->width = MIN (child_requisition.width, gdk_screen_width () * 0.9);
    }
}

static void
gwm_task_view_selection_changed (GwmTaskView *task_view)
{
  GtkCList *clist = GTK_CLIST (task_view->task_clist);

  gwm_task_view_set_task (task_view, clist->selection ? clist->selection->data : NULL);

  clist_adjust_visibility (clist);
}

void
gwm_task_view_rebuild (GwmTaskView *task_view)
{
  GtkCList *clist;
  GtkWidget *vbox, *list_box;
  
  g_return_if_fail (GWM_IS_TASK_VIEW (task_view));
  
  gwm_task_view_destroy_contents (task_view);
  
  /* list box, containing list and action buttons
   */
  list_box = GTK_WIDGET (task_view);
  gtk_widget_set (list_box,
		  "homogeneous", FALSE,
		  "spacing", 5,
		  "border_width", 5,
		  "visible", TRUE,
		  NULL);

  /* action buttons
   */
  vbox = gtk_widget_new (GTK_TYPE_VBOX,
			 "homogeneous", TRUE,
			 "spacing", 5,
			 "border_width", 0,
			 "visible", TRUE,
			 NULL);
  gtk_box_pack_end (GTK_BOX (list_box),
		    gtk_widget_new (GTK_TYPE_ALIGNMENT, /* don't want vexpand */
				    "visible", TRUE,
				    "xscale", 0.0,
				    "yscale", 0.0,
				    "xalign", 0.0,
				    "yalign", 0.0,
				    "child", vbox,
				    NULL),
		    FALSE, FALSE, 0);
  gtk_widget_new (GTK_TYPE_BUTTON,
		  "visible", FALSE,
		  "label", "test",
		  "parent", vbox,
		  "signal::clicked", gdk_beep, NULL,
		  NULL);

  /* task list
   */
  task_view->task_clist =
    gtk_widget_new (GTK_TYPE_CLIST,
		    "visible", TRUE,
		    "n_columns", CLIST_N_COLUMNS,
		    "selection_mode", GTK_SELECTION_BROWSE,
		    "titles_active", FALSE,
		    "border_width", 0,
		    "height", 300,
		    "signal::destroy", gtk_widget_destroyed, &task_view->task_clist,
		    "object_signal::select_row", gwm_task_view_selection_changed, task_view,
		    "signal_after::size_allocate", clist_adjust_visibility, NULL,
		    "signal_after::map", clist_adjust_visibility, NULL,
		    "object_signal_after::button_release_event", gwm_task_view_clist_brelease, task_view,
		    "parent", gtk_widget_new (GTK_TYPE_SCROLLED_WINDOW,
					      "visible", TRUE,
					      "hscrollbar_policy", GTK_POLICY_AUTOMATIC,
					      "vscrollbar_policy", GTK_POLICY_AUTOMATIC,
					      "parent", list_box,
					      "signal_after::size_request", scrolled_window_size_request, NULL,
					      NULL),
		    NULL);
  clist = GTK_CLIST (task_view->task_clist);
  gtk_clist_set_column_title (clist, CLIST_NAME, clist_titles[CLIST_NAME]);
  gtk_clist_set_column_title (clist, CLIST_STATE, clist_titles[CLIST_STATE]);
  gtk_clist_set_column_title (clist, CLIST_DESKTOP, clist_titles[CLIST_DESKTOP]);
  gtk_clist_set_column_title (clist, CLIST_VIEWPORT, clist_titles[CLIST_VIEWPORT]);
  gtk_clist_set_column_title (clist, CLIST_LAYER, clist_titles[CLIST_LAYER]);
  gtk_clist_set_column_auto_resize (clist, CLIST_NAME, TRUE);
  gtk_clist_column_titles_show (clist);
  gtk_clist_column_titles_passive (clist);
  
  gwm_task_view_update_list (task_view);
}

void
gwm_task_view_select_task (GwmTaskView *task_view,
			   GwmhTask    *task)
{
  GtkCList *clist;
  gint row;
  
  g_return_if_fail (GWM_IS_TASK_VIEW (task_view));
  g_return_if_fail (task != NULL);
  
  clist = GTK_CLIST (task_view->task_clist);
  row = gtk_clist_find_row_from_data (clist, task);
  if (row >= 0)
    {
      gtk_clist_freeze (clist);
      gtk_clist_undo_selection (clist);
      gtk_clist_unselect_all (clist);
      gtk_clist_select_row (clist, row, 0);
      gtk_clist_thaw (clist);
    }
}

static gboolean
gwm_task_view_clist_brelease (GwmTaskView    *task_view,
			      GdkEventButton *event)
{
  GtkCList *clist = GTK_CLIST (task_view->task_clist);
  GwmhTask *task = NULL;
  gint row = -1;

  if (event->window == clist->clist_window &&
      gtk_clist_get_selection_info (clist, event->x, event->y, &row, NULL))
    task = gtk_clist_get_row_data (clist, row);

  if (event->button == 1 && task)
    {
      if (GWMH_TASK_ICONIFIED (task) || GWMH_TASK_SHADED (task))
	gwmh_task_show (task);
      else
	{
	  gwmh_desk_set_current_area (task->desktop, task->harea, task->varea);

	  gwmh_task_show (task);
	}
      gtk_signal_emit (GTK_OBJECT (task_view), task_view_signals[SIGNAL_POPDOWN_REQUEST]);
    }

  return TRUE;
}

void
gwm_task_view_set_task (GwmTaskView *task_view,
			GwmhTask    *task)
{
  g_return_if_fail (GWM_IS_TASK_VIEW (task_view));
}
