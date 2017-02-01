/* GNOME Desktop Guide
 * Copyright (C) 1999 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#include <config.h>		/* PACKAGE, i18n */

#include "deskguide_applet.h"
#include <libgnomeui/gnome-window-icon.h>
#include "gwmdesktop.h"
#include "gwmtaskview.h"
#include "gwmthumbnail.h"


#define CONFIG_OBOX_BORDER 6
#define CONFIG_OBOX_SPACING 8
#define CONFIG_IBOX_BORDER 6
#define CONFIG_IBOX_SPACING 4
#define CONFIG_ITEM_BORDER 0
#define CONFIG_ITEM_SPACING 4


/* --- prototypes --- */
static void	gp_init_gui		 (void);
static void	gp_destroy_gui		 (void);
static void	gp_create_desk_widgets	 (void);
static gboolean gp_desk_notifier	 (gpointer	     func_data,
					  GwmhDesk	    *desk,
					  GwmhDeskInfoMask   change_mask);
static gboolean gp_task_notifier	 (gpointer	     func_data,
					  GwmhTask	    *task,
					  GwmhTaskNotifyType ntype,
					  GwmhTaskInfoMask   imask);
static void	gp_about		 (void);
static void	gp_help_popup_deskguide	 (void);
static void	gp_help_popup_properties (void);
static void	gp_config_popup		 (void);
static gpointer gp_config_find_value	 (const gchar	    *path,
					  gboolean	     tmp_value);
static void	gp_load_config		 (const gchar	    *privcfgpath);
static gboolean gp_save_session		 (gpointer	     func_data,
					  const gchar	    *privcfgpath,
					  const gchar	    *globcfgpath);
static gboolean gp_check_task_visible	 (GwmDesktop	    *desktop,
					  GwmhTask	    *task,
					  gpointer	     user_data);

static GtkWidget	*gp_applet = NULL;
static GtkTooltips	*gp_tooltips = NULL;
static GtkTooltips	*gp_desktips = NULL;
static GtkWidget	*gp_container = NULL;
static GtkWidget	*gp_desk_box = NULL;
static GtkWidget       **gp_desk_widget;
static guint             CUR_MAX_DESKTOPS = MAX_DESKTOPS;
static guint		 gp_n_desk_widgets = 0;
static GdkWindow	*gp_atom_window = NULL;
static GtkOrientation	 gp_orientation = GTK_ORIENTATION_HORIZONTAL;
static guint		 gp_panel_size = 12;
static guint		 GP_TYPE_HBOX = 0;
static guint		 GP_TYPE_VBOX = 0;
static guint		 GP_ARROW_DIR = 0;
static gchar		*DESK_GUIDE_NAME = NULL;

static ConfigItem gp_config_items[] = {
  CONFIG_PAGE (N_("Display")),
  CONFIG_SECTION (sect_layout,					N_("Layout")),
  CONFIG_BOOL (show_arrow,      TRUE,
	       N_("Show tasklist arrow")),
  CONFIG_BOOL (switch_arrow,	FALSE,
	       N_("Switch horizontal/vertical position of tasklist arrow")),
  CONFIG_BOOL (current_only,	FALSE,
	       N_("Only show current desktop in pager")),
  CONFIG_BOOL (raise_grid,	FALSE,
	       N_("Raise area grid over tasks")),
  CONFIG_SECTION (sect_thumb_nail,				N_("Thumb Nails")),
  CONFIG_BOOL (enable_thumb_nails,	FALSE,
	       N_("Fill window thumbnails with screen contents")),
  CONFIG_RANGE (thumb_nail_delay, 500,	50,	5000,
		N_("Incremental update delay [ms]")),
  CONFIG_SECTION (sect_tooltips,				N_("Tooltips")),
  CONFIG_BOOL (tooltips,	TRUE,
	       N_("Show Desk-Guide tooltips")),
  CONFIG_RANGE (tooltips_delay, 500,	1,	5000,
		N_("Desk-Guide tooltip delay [ms]")),
  CONFIG_BOOL (desktips,	TRUE,
	       N_("Show desktop name tooltips")),
  CONFIG_RANGE (desktips_delay, 10,	1,	5000,
		N_("Desktop name tooltip delay [ms]")),
  
  CONFIG_PAGE (N_("Tasks")),
  CONFIG_SECTION (sect_task_visibility,				N_("Visibility")),
  CONFIG_BOOL (show_hidden_tasks,	TRUE,
	       N_("Show hidden tasks (HIDDEN)")),
  CONFIG_BOOL (show_shaded_tasks,	TRUE,
	       N_("Show shaded tasks (SHADED)")),
  CONFIG_BOOL (show_skip_winlist,	FALSE,
	       N_("Show tasks which hide from window list (SKIP-WINLIST)")),
  CONFIG_BOOL (show_skip_taskbar,	FALSE,
	       N_("Show tasks which hide from taskbar (SKIP-TASKBAR)")),
  /*  CONFIG_SECTION (sect_null_1, NULL), */
  
  CONFIG_PAGE (N_("Geometry")),
  CONFIG_SECTION (sect_horizontal,				N_("Horizontal Layout")),
  CONFIG_RANGE (area_height,	44,	4,	1024,
		N_("Desktop Height [pixels]")),
  CONFIG_BOOL (abandon_area_height,		TRUE,
	       N_("Override desktop height with panel size")),
  CONFIG_BOOL (div_by_vareas,		TRUE,
	       N_("Divide height by number of vertical areas")),
  CONFIG_RANGE (row_stackup,	1,	1,	64,
		N_("Rows of Desktops")),
  CONFIG_BOOL (div_by_nrows,		TRUE,
	       N_("Divide height by number of rows")),
  CONFIG_SECTION (sect_vertical,				N_("Vertical Layout")),
  CONFIG_RANGE (area_width,	44,	4,	1024,
		N_("Desktop Width [pixels]")),
  CONFIG_BOOL (abandon_area_width,		TRUE,
	       N_("Override desktop width with panel size")),
  CONFIG_BOOL (div_by_hareas,		TRUE,
	       N_("Divide width by number of horizontal areas")),
  CONFIG_RANGE (col_stackup,	1,	1,	64,
		N_("Columns of Desktops")),
  CONFIG_BOOL (div_by_ncols,		TRUE,
	       N_("Divide width by number of columns")),

  CONFIG_PAGE (N_("Advanced")),
  CONFIG_SECTION (sect_workarounds,			N_("Window Manager Workarounds")),
  CONFIG_BOOL (skip_movement_offset,		FALSE,
	       N_("Window manager moves decoration window instead\n"
		   "(AfterStep, Enlightenment, FVWM, IceWM)")),
  CONFIG_BOOL (unified_areas,			TRUE,
	       N_("Window manager changes active area on all desktops\n"
		   "(FVWM, Sawfish)")),
  CONFIG_BOOL (violate_client_msg,		FALSE,
	       N_("Window manager expects pager to modify area+desktop properties directly\n"
		   "(Enlightenment, FVWM)")),

  CONFIG_SECTION (sect_behaviour,				N_("Behaviour")),
  CONFIG_BOOL (task_view_popdown_request,	TRUE,
	       N_("Popdown task view automatically")),
};
static guint  gp_n_config_items = (sizeof (gp_config_items) /
				   sizeof (gp_config_items[0]));


/* --- main ---*/
gint 
main (gint   argc,
      gchar *argv[])
{
  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);
  
  applet_widget_init ("deskguide_applet",
		      DESKGUIDE_VERSION,
		      argc, argv,
		      NULL, 0, NULL);
  
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR "/gnome-deskguide.png");
  DESK_GUIDE_NAME = _("GNOME Desktop Guide (Pager)");
  
  /* setup applet widget
   */
  gp_tooltips = gtk_tooltips_new ();
  gp_desktips = gtk_tooltips_new ();
  gp_applet = applet_widget_new ("deskguide_applet");
  if (!gp_applet)
    g_error ("Unable to create applet widget");
  gtk_widget_ref (gp_applet);
  
  gp_desk_widget = g_new0 (GtkWidget*, CUR_MAX_DESKTOPS);
  
  /* bail out for non GNOME window managers
   */
  if (!gwmh_init ())
    {
      GtkWidget *dialog;
      gchar *error_msg = _("You are not running a GNOME Compliant\n"
			    "Window Manager. GNOME support by the \n"
			    "window manager is strongly recommended\n"
			    "for proper Desk Guide operation.");
      
      dialog = gnome_error_dialog (_("Desk Guide Alert"));
      gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
			  gtk_widget_new (GTK_TYPE_LABEL,
					  "visible", TRUE,
					  "label", DESK_GUIDE_NAME,
					  NULL),
			  TRUE, TRUE, 5);
      gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
			  gtk_widget_new (GTK_TYPE_LABEL,
					  "visible", TRUE,
					  "label", error_msg,
					  NULL),
			  TRUE, TRUE, 5);
      gnome_dialog_run (GNOME_DIALOG (dialog));
      
      /* gtk_exit (1); */
    }
  
  /* main container
   */
  gp_container = gtk_widget_new (GTK_TYPE_ALIGNMENT,
				 "signal::destroy", gtk_widget_destroyed, &gp_container,
				 "child", gtk_type_new (GTK_TYPE_WIDGET),
				 NULL);
  /* notifiers and callbacks
   */
  gwmh_desk_notifier_add (gp_desk_notifier, NULL);
  gwmh_task_notifier_add (gp_task_notifier, NULL);
  gtk_widget_set (gp_applet,
		  "signal::change-orient", gp_destroy_gui, NULL,
		  "signal::change-orient", gp_init_gui, NULL,
		  "signal::change-pixel-size", gp_destroy_gui, NULL,
		  "signal::change-pixel-size", gp_init_gui, NULL,
		  "object_signal::save-session", gp_save_session, NULL,
		  "signal::destroy", gtk_main_quit, NULL,
		  NULL);

  /* add container to applet, note that this MUST come after binding
   * the change-* signals otherwise we have a race */
  applet_widget_add (APPLET_WIDGET (gp_applet), gp_container);
  

  applet_widget_register_stock_callback (APPLET_WIDGET (gp_applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 _("Properties..."),
					 (AppletCallbackFunc) gp_config_popup,
					 NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (gp_applet),
					 "help",
					 GNOME_STOCK_PIXMAP_HELP,
					 _("Help"),
					 (AppletCallbackFunc) gp_help_popup_deskguide,
					 NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (gp_applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 (AppletCallbackFunc) gp_about,
					 NULL);
  
  /* load configuration
   */
  gp_load_config (APPLET_WIDGET (gp_applet)->privcfgpath);
  
  /* advertise to a WM that we exist and are here
   */
  gp_atom_window = gwmh_root_put_atom_window ("_GNOME_PAGER_ACTIVE",
					      GDK_WINDOW_TEMP,
					      GDK_INPUT_OUTPUT,
					      0);
  
  /* and away into the main loop
   */
  gtk_main ();
  
  /* shutdown
   */
  gwmh_desk_notifier_remove_func (gp_desk_notifier, NULL);
  gwmh_task_notifier_remove_func (gp_task_notifier, NULL);
  gdk_window_unref (gp_atom_window);
  gtk_widget_destroy (gp_applet);
  gtk_widget_unref (gp_applet);

  g_free (gp_desk_widget);

  return 0;
}

static void
gp_load_config (const gchar *privcfgpath)
{
  static guint loaded = FALSE;
  guint i;
  gchar *section = "sect_general";
  
  if (loaded)
    return;
  loaded = TRUE;
  
  gnome_config_push_prefix (privcfgpath);
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	section = item->path;
      else  if (item->min == -1 && item->max == -1)	/* boolean */
	{
	  gchar *path = g_strconcat (section, "/",
				     item->path, "=",
				     GPOINTER_TO_INT (item->value) ? "true" : "false",
				     NULL);
	  
	  item->value = GINT_TO_POINTER (gnome_config_get_bool (path));
	  g_free (path);
	}
      else						/* integer range */
	{
	  gchar *path = g_strdup_printf ("%s/%s=%d",
					 section,
					 item->path,
					 GPOINTER_TO_INT (item->value));
	  
	  item->value = GINT_TO_POINTER (gnome_config_get_int (path));
	  g_free (path);
	}
    }
  
  gnome_config_pop_prefix ();
}

static gpointer
gp_config_find_value (const gchar *path,
		      gboolean	   tmp_value)
{
  guint i;
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if ((path && item->path && strcmp (path, item->path) == 0) || path == item->path)
	return tmp_value ? item->tmp_value : item->value;
    }
  
  g_warning (G_GNUC_PRETTY_FUNCTION "(): unable to find config value for <%s>", path);
  
  return NULL;
}

static gboolean
gp_save_session (gpointer     func_data,
		 const gchar *privcfgpath,
		 const gchar *globcfgpath)
{
  guint i;
  gchar *section = "sect_general";
  
  gnome_config_push_prefix (privcfgpath);
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      gchar *path;

      if (!item->path)					/* page */
	continue;

      path = g_strconcat (section, "/", item->path, NULL);
      if (item->min == -2 && item->max == -2)		/* section */
	section = item->path;
      else if (item->min == -1 && item->max == -1)	/* boolean */
	gnome_config_set_bool (path, GPOINTER_TO_INT (item->value));
      else						/* integer range */
	gnome_config_set_int (path, GPOINTER_TO_INT (item->value));
      g_free (path);
    }
  
  gnome_config_pop_prefix ();
  
  gnome_config_sync ();
  gnome_config_drop_all ();
  
  return FALSE;
}

static void
gp_config_reset_tmp_values (void)
{
  guint i;
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	continue;
      else if (item->min == -1 && item->max == -1)	/* boolean */
	item->tmp_value = item->value;
      else						/* integer range */
	item->tmp_value = item->value;
    }
}

static void
gp_config_apply_tmp_values (void)
{
  guint i;
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	continue;
      else if (item->min == -1 && item->max == -1)	/* boolean */
	item->value = item->tmp_value;
      else						/* integer range */
	item->value = item->tmp_value;
    }
}

static gboolean
gp_desk_notifier (gpointer	   func_data,
		  GwmhDesk	  *desk,
		  GwmhDeskInfoMask change_mask)
{
  if (change_mask & GWMH_DESK_INFO_BOOTUP)
    {
      gp_destroy_gui ();
      gp_init_gui ();
      gwm_desktop_class_reload_thumbs ();
    }
  else
    {
      /* keep number of desktop widgets in sync with desk */
      if (change_mask & (GWMH_DESK_INFO_DESKTOP_NAMES |
			 GWMH_DESK_INFO_N_DESKTOPS |
			 GWMH_DESK_INFO_N_AREAS |
			 GWMH_DESK_INFO_BOOTUP))
	gp_create_desk_widgets ();
      
      if (gp_n_desk_widgets && BOOL_CONFIG (current_only))
	gwm_desktop_set_index (GWM_DESKTOP (gp_desk_widget[0]),
			       desk->current_desktop);
    }
  
  return TRUE;
}

static gboolean
gp_task_notifier (gpointer	     func_data,
		  GwmhTask	    *task,
		  GwmhTaskNotifyType ntype,
		  GwmhTaskInfoMask   imask)
{
  return TRUE;
}

static void 
gp_destroy_gui (void)
{
  if (gp_container)
    {
      gtk_widget_hide (gp_container);
      if (GTK_BIN (gp_container)->child)
	gtk_widget_destroy (GTK_BIN (gp_container)->child);
      else
	g_warning (G_GNUC_PRETTY_FUNCTION "(): missing container child");
    }
  else
    g_warning (G_GNUC_PRETTY_FUNCTION "(): missing container");
}

static void
gp_create_desk_widgets (void)
{
  gdouble area_size;

  if (N_DESKTOPS > CUR_MAX_DESKTOPS) {
    gp_desk_widget = g_renew (GtkWidget*, gp_desk_widget, CUR_MAX_DESKTOPS + 1);
    gp_desk_widget[CUR_MAX_DESKTOPS++] = NULL;
    g_message ("Growing desktop array to allow %d desktops.", CUR_MAX_DESKTOPS);
  }

  /* some gwmh configuration hacks ;( */
  gwmh_desk_set_hack_values (BOOL_CONFIG (unified_areas),
			     BOOL_CONFIG (violate_client_msg));

  if (!gp_desk_box)
    return;

  gtk_widget_ensure_style (gp_desk_box);

  /* configure Desktop widget class for us */
  if (gp_orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      if (BOOL_CONFIG (abandon_area_height))
	area_size = gp_panel_size - 2 * gp_desk_box->style->klass->ythickness;
      else
	area_size = RANGE_CONFIG (area_height);
      if (BOOL_CONFIG (div_by_vareas))
	area_size /= (gdouble) N_VAREAS;
      if (BOOL_CONFIG (div_by_nrows) && !BOOL_CONFIG (current_only))
	{
	  guint n = RANGE_CONFIG (row_stackup);

	  area_size /= (gdouble) MIN (n, N_DESKTOPS);
	}
    }
  else /* gp_orientation == GTK_ORIENTATION_VERTICAL */
    {
      if (BOOL_CONFIG (abandon_area_width))
	area_size = gp_panel_size - 2 * gp_desk_box->style->klass->xthickness;
      else
	area_size = RANGE_CONFIG (area_width);
      if (BOOL_CONFIG (div_by_hareas))
	area_size /= (gdouble) N_HAREAS;
      if (BOOL_CONFIG (div_by_ncols) && !BOOL_CONFIG (current_only))
	{
	  guint n = (gdouble) RANGE_CONFIG (col_stackup);

	  area_size /= (gdouble) MIN (n, N_DESKTOPS);
	}
    }
  gwm_thumb_nails_set_active (BOOL_CONFIG (enable_thumb_nails));
  gwm_desktop_class_config (gtk_type_class (GWM_TYPE_DESKTOP),
			    BOOL_CONFIG (enable_thumb_nails) ? RANGE_CONFIG (thumb_nail_delay) : 0,
			    gp_orientation,
			    area_size,
			    BOOL_CONFIG (raise_grid),
			    BOOL_CONFIG (skip_movement_offset));

  gp_n_desk_widgets = 0;
  gtk_container_forall (GTK_CONTAINER (gp_desk_box), (GtkCallback) gtk_widget_destroy, NULL);
  if (TRUE)	/* FIXME: remove BOOL_CONFIG (show_pager) */
    {
      GtkWidget *desk_column = NULL;
      guint n_desks_per_column;
      guint i, max = BOOL_CONFIG (current_only) ? 1 : N_DESKTOPS;
      
      if (gp_orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	  n_desks_per_column = RANGE_CONFIG (row_stackup);
	  n_desks_per_column = MIN (n_desks_per_column, N_DESKTOPS);
	  n_desks_per_column = (N_DESKTOPS + n_desks_per_column - 1) / n_desks_per_column;
	}
      else
	{
	  guint n = RANGE_CONFIG (col_stackup);

	  n_desks_per_column = MIN (n, N_DESKTOPS);
	}

      for (i = 0; i < max; i++)
	{
	  GtkWidget *alignment;
	  
	  if (!desk_column)
	    desk_column = gtk_widget_new (GTK_TYPE_HBOX,
					  "visible", TRUE,
					  "spacing", 0,
					  "parent", gp_desk_box,
					  NULL);
	  gp_desk_widget[i] = gwm_desktop_new (max > 1 ? i : CURRENT_DESKTOP, gp_desktips);
	  alignment = gtk_widget_new (GTK_TYPE_ALIGNMENT,
				      "visible", TRUE,
				      "xscale", 0.0,
				      "yscale", 0.0,
				      NULL);
	  gtk_widget_set (gp_desk_widget[i],
			  "parent", alignment,
			  "visible", TRUE,
			  "signal::check-task", gp_check_task_visible, NULL,
			  "signal::destroy", gtk_widget_destroyed, &gp_desk_widget[i],
			  NULL);
	  gtk_box_pack_start (GTK_BOX (desk_column), alignment, FALSE, FALSE, 0);
	  if ((i + 1) % n_desks_per_column == 0)
	    desk_column = NULL;
	}
      gp_n_desk_widgets = i;
    }
}

static gboolean
gp_check_task_visible (GwmDesktop *desktop,
		       GwmhTask	  *task,	
		       gpointer	   user_data)
{
  if (GWMH_TASK_ICONIFIED (task) ||
      (GWMH_TASK_HIDDEN (task) && !BOOL_CONFIG (show_hidden_tasks)) ||
      (GWMH_TASK_SHADED (task) && !BOOL_CONFIG (show_shaded_tasks)) ||
      (GWMH_TASK_SKIP_WINLIST (task) && !BOOL_CONFIG (show_skip_winlist)) ||
      (GWMH_TASK_SKIP_TASKBAR (task) && !BOOL_CONFIG (show_skip_taskbar)))
    return FALSE;
  else
    return TRUE;
}

static void
gp_task_view_popdown_request (GtkWidget *dialog)
{
  if (BOOL_CONFIG (task_view_popdown_request))
    gtk_widget_hide (dialog);
}

static void
gp_task_view_dialog_event (GtkWidget *dialog,
			   GdkEvent  *event)
{
  gboolean popdown = FALSE;

  switch (event->type)
    {
      GtkWidget *ewidget;
    case GDK_KEY_PRESS:
      if (event->key.keyval == GDK_Escape)
	popdown = TRUE;
      break;
    case GDK_BUTTON_PRESS:
      ewidget = gtk_get_event_widget (event);
      if (event->any.window == dialog->window || !ewidget ||
	  gtk_widget_get_toplevel (ewidget) != dialog)
	popdown = TRUE;
      break;
    default:
      break;
    }
  if (popdown && BOOL_CONFIG (task_view_popdown_request))
    gtk_widget_hide (dialog);
}

static void
gp_widget_button_toggle_task_list (GtkWidget *widget,
				   gpointer   data)
{
  static GtkWidget *dialog = NULL;

  if (!dialog)
    {
      GtkWidget *task_view;

      dialog = gtk_widget_new (GTK_TYPE_WINDOW,
			       "title", _("Desk Guide Task View"),
			       "auto_shrink", FALSE,
			       "allow_shrink", FALSE,
			       "allow_grow", TRUE,
			       "signal::delete_event", gtk_widget_hide_on_delete, NULL,
			       "signal::destroy", gtk_widget_destroyed, &dialog,
			       "signal::destroy", gtk_widget_destroyed, &dialog,
			       "signal::button_press_event", gp_task_view_dialog_event, NULL,
			       "signal::key_press_event", gp_task_view_dialog_event, NULL,
			       "type", GTK_WINDOW_POPUP,
			       "events", (GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK),
			       NULL);
      task_view = gtk_widget_new (GWM_TYPE_TASK_VIEW,
				  "visible", TRUE,
				  "parent", gtk_widget_new (GTK_TYPE_FRAME,
							    "visible", TRUE,
							    "shadow", GTK_SHADOW_IN,
							    "parent", gtk_widget_new (GTK_TYPE_FRAME,
										      "visible", TRUE,
										      "shadow", GTK_SHADOW_OUT,
										      "parent", dialog,
										      NULL),
							    NULL),
				  "object_signal::popdown_request", gp_task_view_popdown_request, dialog,
				  NULL);
      gwm_task_view_rebuild (GWM_TASK_VIEW (task_view));
      gtk_widget_size_request (dialog, NULL);
      gtk_widget_realize (dialog);
    }
  if (!GTK_WIDGET_VISIBLE (dialog))
    {
      GwmhTask *task;

      gtk_container_check_resize (GTK_CONTAINER (dialog));
      if (gp_applet && GTK_WIDGET_REALIZED (gp_applet))
	{
	  GtkAllocation dsize;
	  gint x1, y1, x2, y2, x, y;

	  dsize = dialog->allocation; /* gtk_widget_get_child_requisition (dialog, &dsize); */
	  gdk_window_get_origin (gp_applet->window, &x1, &y1);
	  x2 = x1 + gp_applet->allocation.width;
	  y2 = y1 + gp_applet->allocation.height;
	  if (GP_ARROW_DIR == GTK_ARROW_UP)
	    y = MAX (y1 - dsize.height, 0);
	  else if (GP_ARROW_DIR == GTK_ARROW_DOWN)
	    y = MIN (y2, gdk_screen_height () - dsize.height);
	  else
	    y = CLAMP (y1, 0, gdk_screen_height () - dsize.height);
	  if (GP_ARROW_DIR == GTK_ARROW_LEFT)
	    x = MAX (x1 - dsize.width, 0);
	  else if (GP_ARROW_DIR == GTK_ARROW_RIGHT)
	    x = MIN (x2, gdk_screen_width () - dsize.width);
	  else
	    x = CLAMP (x1, 0, gdk_screen_width () - dsize.width);
	  gtk_widget_set_uposition (dialog, x, y);
	}
      gtk_window_set_modal (GTK_WINDOW (dialog), BOOL_CONFIG (task_view_popdown_request));
      gtk_widget_show (dialog);
      gdk_flush ();
      gwmh_window_send_client_message (dialog->window,
				       GWMHA_WIN_STATE,
				       GWMH_STATE_STICKY | GWMH_STATE_HIDDEN,
				       GWMH_STATE_STICKY | GWMH_STATE_HIDDEN,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME);
      gwmh_window_send_client_message (dialog->window,
				       GWMHA_WIN_LAYER,
				       GWMH_LAYER_ABOVE_DOCK,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME);
      gwmh_window_send_client_message (dialog->window,
				       GWMHA_WIN_HINTS,
				       GWMH_HINTS_DO_NOT_COVER,
				       GWMH_HINTS_DO_NOT_COVER,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME,
				       GDK_CURRENT_TIME);
      gdk_window_raise (dialog->window);
      task = gwmh_task_from_window (dialog->window);
      if (gdk_pointer_grab (dialog->window, TRUE,
			    (GDK_POINTER_MOTION_HINT_MASK |
			     GDK_BUTTON1_MOTION_MASK |
			     GDK_BUTTON2_MOTION_MASK |
			     GDK_BUTTON3_MOTION_MASK |
			     GDK_BUTTON_PRESS_MASK |
			     GDK_BUTTON_RELEASE_MASK),
			    NULL,
			    NULL,
			    GDK_CURRENT_TIME) != 0 ||
	  gdk_keyboard_grab (dialog->window, FALSE, GDK_CURRENT_TIME) != 0)
	gtk_widget_hide (dialog);
    }
  else
    gtk_widget_hide (dialog);
}

static inline gboolean
gp_widget_ignore_button (GtkWidget *widget,
			 GdkEvent  *event,
			 gpointer   data)
{
  guint button = GPOINTER_TO_UINT (data);

  if (event->type == GDK_BUTTON_PRESS ||
      event->type == GDK_2BUTTON_PRESS ||
      event->type == GDK_3BUTTON_PRESS ||
      event->type == GDK_BUTTON_RELEASE)
    {
      GdkEventButton *bevent = &event->button;
  
      if (bevent->button == button)
	{
	  if (widget->parent)
	    gtk_propagate_event (widget->parent, event);

	  return TRUE;
	}
    }

  return FALSE;
}

static void 
gp_init_gui (void)
{
  GtkWidget *button, *abox, *arrow;
  gboolean arrow_at_end = FALSE;
  GtkWidget *main_box;

  abox = NULL;

  gtk_widget_set_usize (gp_container, 0, 0);
  gp_panel_size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (gp_applet));
  gp_panel_size = MAX (gp_panel_size, 12);
  switch (applet_widget_get_panel_orient (APPLET_WIDGET (gp_applet)))
    {
    case ORIENT_UP:
      GP_TYPE_HBOX = GTK_TYPE_HBOX;
      GP_TYPE_VBOX = GTK_TYPE_VBOX;
      GP_ARROW_DIR = GTK_ARROW_UP;
      gtk_widget_set (gp_container,
		      NULL);
      gp_orientation = GTK_ORIENTATION_HORIZONTAL;
      arrow_at_end = FALSE;
      break;
    case ORIENT_DOWN:
      GP_TYPE_HBOX = GTK_TYPE_HBOX;
      GP_TYPE_VBOX = GTK_TYPE_VBOX;
      GP_ARROW_DIR = GTK_ARROW_DOWN;
      gtk_widget_set (gp_container,
		      NULL);
      gp_orientation = GTK_ORIENTATION_HORIZONTAL;
      arrow_at_end = TRUE;
      break;
    case ORIENT_LEFT:
      GP_TYPE_HBOX = GTK_TYPE_VBOX;
      GP_TYPE_VBOX = GTK_TYPE_HBOX;
      GP_ARROW_DIR = GTK_ARROW_LEFT;
      gtk_widget_set (gp_container,
		      NULL);
      gp_orientation = GTK_ORIENTATION_VERTICAL;
      arrow_at_end = FALSE;
      break;
    case ORIENT_RIGHT:
      GP_TYPE_HBOX = GTK_TYPE_VBOX;
      GP_TYPE_VBOX = GTK_TYPE_HBOX;
      GP_ARROW_DIR = GTK_ARROW_RIGHT;
      gtk_widget_set (gp_container,
		      NULL);
      gp_orientation = GTK_ORIENTATION_VERTICAL;
      arrow_at_end = TRUE;
      break;
    }
  
  /* configure tooltips
   */
  (BOOL_CONFIG (tooltips)
   ? gtk_tooltips_enable
   : gtk_tooltips_disable) (gp_tooltips);
  gtk_tooltips_set_delay (gp_tooltips, RANGE_CONFIG (tooltips_delay));
  (BOOL_CONFIG (desktips)
   ? gtk_tooltips_enable
   : gtk_tooltips_disable) (gp_desktips);
  gtk_tooltips_set_delay (gp_desktips, RANGE_CONFIG (desktips_delay));
  
  /* main container
   */
  main_box = gtk_widget_new (GP_TYPE_HBOX,
			     "visible", TRUE,
			     "spacing", 0,
			     "parent", gp_container,
			     NULL);
  
  /* provide box for arrow and button
   */
  if (BOOL_CONFIG (show_arrow))
  {
    abox = gtk_widget_new (GP_TYPE_VBOX,
			   "visible", TRUE,
			   "spacing", 0,
			   NULL);

    (BOOL_CONFIG (switch_arrow)
     ? gtk_box_pack_end
     : gtk_box_pack_start) (GTK_BOX (main_box), abox, FALSE, TRUE, 0);
  }

  /* provide desktop widget container
   */
  gp_desk_box = gtk_widget_new (GTK_TYPE_VBOX,
				"visible", TRUE,
				"spacing", 0,
				"signal::destroy", gtk_widget_destroyed, &gp_desk_box,
				"parent", gtk_widget_new (GTK_TYPE_ALIGNMENT,
							  "visible", TRUE,
							  "xscale", 0.0,
							  "yscale", 0.0,
							  "parent", main_box,
							  NULL),
				NULL);
  
  /* add arrow and button
   */
  if (BOOL_CONFIG (show_arrow))
  {
    arrow = gtk_widget_new (GTK_TYPE_ARROW,
			    "visible", TRUE,
			    "arrow_type", GP_ARROW_DIR,
			    NULL);
    button = gtk_widget_new (GTK_TYPE_BUTTON,
			     "visible", TRUE,
			     "can_focus", FALSE,
			     "child", arrow,
			     "signal::clicked", gp_widget_button_toggle_task_list, NULL,
			     "signal::event", gp_widget_ignore_button, GUINT_TO_POINTER (2),
			     "signal::event", gp_widget_ignore_button, GUINT_TO_POINTER (3),
			     NULL);
    gtk_tooltips_set_tip (gp_tooltips,
			  button,
			  DESK_GUIDE_NAME,
			  NULL);
    (arrow_at_end
     ? gtk_box_pack_end
     : gtk_box_pack_start) (GTK_BOX (abox), button, TRUE, TRUE, 0);
  }

  /* desktop pagers
   */
  gp_create_desk_widgets ();
  
  gtk_widget_show (gp_container);
  gtk_widget_show (gp_applet);
}

static void 
gp_about (void)
{
  static GtkWidget *dialog = NULL;
  
  if (!dialog)
    {
      const char *authors[] = {
	"Tim Janik",
	NULL
      };
      
      dialog = gnome_about_new ("Desk Guide",
				DESKGUIDE_VERSION,
				"Copyright (C) 1999 Tim Janik",
				authors,
				DESK_GUIDE_NAME,
				"gnome-deskguide-splash.png");
      gtk_widget_set (dialog,
		      "signal::destroy", gtk_widget_destroyed, &dialog,
		      NULL);
      gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
      gtk_quit_add_destroy (1, GTK_OBJECT (dialog));
    }
  
  gtk_widget_show (dialog);
  gdk_window_raise (dialog->window);
}

static void
gp_help_popup_deskguide (void)
{
  GnomeHelpMenuEntry help_entry = { "desk-guide_applet", "index.html" };

  gnome_help_display (NULL, &help_entry);
}

static void
gp_help_popup_properties (void)
{
  GnomeHelpMenuEntry help_entry = { "desk-guide_applet", "index.html#DESKGUIDE-PROPERTIES" };

  gnome_help_display (NULL, &help_entry);
}

static void
gp_config_check (GtkWidget *widget)
{
  GtkWidget *toplevel = gtk_widget_get_toplevel (widget);

  gtk_widget_set_sensitive (CONFIG_WIDGET (toplevel, area_height),
			    !BOOL_TMP_CONFIG (abandon_area_height));
  gtk_widget_set_sensitive (CONFIG_WIDGET (toplevel, area_width),
			    !BOOL_TMP_CONFIG (abandon_area_width));
  gtk_widget_set_sensitive (CONFIG_WIDGET (toplevel, thumb_nail_delay),
			    BOOL_TMP_CONFIG (enable_thumb_nails));
  gtk_widget_set_sensitive (CONFIG_WIDGET (toplevel, switch_arrow),
			    BOOL_TMP_CONFIG (show_arrow));
  gtk_widget_set_sensitive (CONFIG_WIDGET (toplevel, task_view_popdown_request),
			    BOOL_TMP_CONFIG (show_arrow));
}

static void
gp_config_toggled (GtkToggleButton *button,
		   ConfigItem	   *item)
{
  item->tmp_value = GINT_TO_POINTER (button->active);
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_widget_get_toplevel (GTK_WIDGET (button))));
}

static inline GtkWidget*
gp_config_add_boolean (GtkWidget  *vbox,
		       ConfigItem *item)
{
  GtkWidget *widget, *toplevel = gtk_widget_get_toplevel (vbox);
  
  widget = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
			   "visible", TRUE,
			   "label", _(item->name),
			   "active", GPOINTER_TO_INT (item->value),
			   "border_width", CONFIG_ITEM_BORDER,
			   "signal::toggled", gp_config_toggled, item,
			   "object_signal_after::toggled", gp_config_check, toplevel,
			   NULL);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, TRUE, 0);
  gtk_widget_set (GTK_BIN (widget)->child,
		  "xalign", 0.0,
		  NULL);
  gtk_object_set_data (GTK_OBJECT (toplevel), item->path, widget);
  
  return widget;
}

static void
gp_config_value_changed (GtkAdjustment *adjustment,
			 ConfigItem	 *item)
{
  item->tmp_value = GINT_TO_POINTER ((gint) adjustment->value);
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_widget_get_toplevel (gtk_object_get_user_data (GTK_OBJECT (adjustment)))));
}

static inline GtkWidget*
gp_config_add_range (GtkWidget	*vbox,
		     ConfigItem *item)
{
  GtkObject *adjustment;
  GtkWidget *hbox, *label, *spinner, *toplevel = gtk_widget_get_toplevel (vbox);
  
  adjustment = gtk_adjustment_new (GPOINTER_TO_UINT (item->tmp_value),
				   item->min,
				   item->max,
				   1, 1,
				   (item->max - item->min) / 10);
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "homogeneous", FALSE,
			 "visible", TRUE,
			 "spacing", CONFIG_ITEM_SPACING,
			 "border_width", CONFIG_ITEM_BORDER,
			 NULL);
  label = gtk_widget_new (GTK_TYPE_LABEL,
			  "visible", TRUE,
			  "xalign", 0.0,
			  "label", _(item->name),
			  "parent", hbox,
			  NULL);
  spinner = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 0, 0);
  gtk_widget_set (spinner,
		  "visible", TRUE,
		  "width", 80,
		  NULL);
  gtk_object_set_user_data (GTK_OBJECT (adjustment), spinner);
  gtk_box_pack_end (GTK_BOX (hbox), spinner, FALSE, TRUE, 0);
  gtk_object_set (adjustment,
		  "signal::value_changed", gp_config_value_changed, item,
		  "object_signal_after::value_changed", gp_config_check, toplevel,
		  NULL);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_object_set_data (GTK_OBJECT (toplevel), item->path, hbox);
  
  return hbox;
}

static inline GtkWidget*
gp_config_add_section (GtkBox	   *parent,
		       const gchar *section)
{
  GtkWidget *widget;
  GtkWidget *box;
  
  if (section)
    {
      widget = gtk_widget_new (GTK_TYPE_FRAME,
			       "visible", TRUE,
			       "label", section,
			       NULL);
      box = gtk_widget_new (GTK_TYPE_VBOX,
			    "visible", TRUE,
			    "parent", widget,
			    "border_width", CONFIG_IBOX_BORDER,
			    "spacing", CONFIG_IBOX_SPACING,
			    NULL);
    }
  else
    {
      box = gtk_widget_new (GTK_TYPE_VBOX,
			    "visible", TRUE,
			    "border_width", 0,
			    "spacing", CONFIG_OBOX_SPACING,
			    NULL);
      widget = box;
    }

  gtk_box_pack_start (parent, widget, FALSE, TRUE, 0);
  
  return box;
}

static GSList*
gp_config_create_page (GSList		*item_slist,
		       GnomePropertyBox *pbox)
{
  gchar *page_name;
  ConfigItem *item;
  GtkWidget *page, *vbox = NULL;
  
  g_return_val_if_fail (item_slist != NULL, NULL);
  
  item = item_slist->data;
  if (!item->path)
    {
      GSList *node = item_slist;
      
      item_slist = node->next;
      
      page_name = _(item->name);
    }
  else
    page_name = _("Global");
  
  page = gtk_widget_new (GTK_TYPE_VBOX,
			 "visible", TRUE,
			 "border_width", CONFIG_OBOX_BORDER,
			 "spacing", CONFIG_OBOX_SPACING,
			 NULL);
  gnome_property_box_append_page (pbox, page, gtk_label_new (_(page_name)));
  
  while (item_slist)
    {
      ConfigItem *item = item_slist->data;
      
      if (!item->path)						/* page */
	break;
      item_slist = item_slist->next;
      
      if (item->min == -2 && item->max == -2)			/* section */
	vbox = gp_config_add_section (GTK_BOX (page), _(item->name));
      else if (item->min == -1 && item->max == -1)		/* boolean */
	{
	  if (!vbox)
	    vbox = gp_config_add_section (GTK_BOX (page), NULL);
	  gp_config_add_boolean (vbox, item);
	}
      else							/* integer range */
	{
	  if (!vbox)
	    vbox = gp_config_add_section (GTK_BOX (page), NULL);
	  gp_config_add_range (vbox, item);
	}
    }
  
  return item_slist;
}

static void 
gp_config_popup (void)
{
  static GtkWidget *dialog = NULL;
  
  g_return_if_fail (gp_n_config_items > 0);
  
  if (!dialog)
    {
      GSList *fslist, *slist = NULL;
      guint i;
      
      gp_config_reset_tmp_values ();
      for (i = 0; i < gp_n_config_items; i++)
	slist = g_slist_prepend (slist, gp_config_items + i);
      slist = g_slist_reverse (slist);
      fslist = slist;
      
      dialog = gnome_property_box_new ();
      gtk_widget_set (dialog,
		      "title", _("Desk Guide Settings"),
		      "signal::apply", gp_destroy_gui, NULL,
		      "signal::apply", gp_config_apply_tmp_values, NULL,
		      "signal::apply", gp_init_gui, NULL,
		      "signal::destroy", gp_config_reset_tmp_values, NULL,
		      "signal::destroy", gtk_widget_destroyed, &dialog,
		      "signal::help", gp_help_popup_properties, NULL,
		      NULL);
      
      while (slist)
	slist = gp_config_create_page (slist, GNOME_PROPERTY_BOX (dialog));
      g_slist_free (fslist);

      gtk_quit_add_destroy (1, GTK_OBJECT (dialog));
    }
  
  gp_config_check (dialog);
  gtk_widget_show (dialog);
  gdk_window_raise (dialog->window);
}
