#include <config.h>
#include <gtk/gtk.h>
#include <gnome.h>
#include "tasklist_applet.h"
#include "pixmaps.h"

/* define to x for debug output */
#define d(x)

/* Callback for menu positioning */
static void
cb_menu_position (GtkMenu *menu, gint *x, gint *y, gpointer user_data)
{
	GtkRequisition mreq;
	gint wx, wy;
	TasklistTask *task = gtk_object_get_data (GTK_OBJECT (menu), "task");

	/* this is just sanity, it is concievable that this is now null */
	if (task == NULL) {
		*x = 0;
		*y = 0;
		return;
	}

	gtk_widget_get_child_requisition (GTK_WIDGET (menu), &mreq);
	gdk_window_get_origin (task->tasklist->area->window, &wx, &wy);

	switch (applet_widget_get_panel_orient (APPLET_WIDGET (task->tasklist->applet))) {
	case ORIENT_UP:
		*x = wx + task->x;
		*y = wy - mreq.height + task->y;
		break;
	case ORIENT_DOWN:
		*x = wx + task->x;
		*y = wy + task->y + task->height;
		break;
	case ORIENT_LEFT:
		*y = wy + task->y;
		*x = wx - mreq.width + task->x;
		break;
	case ORIENT_RIGHT:
		*y = wy + task->y;
		*x = wx + task->width;
		printf ("right\n");
		break;
	}

	*x = CLAMP (*x, 0, gdk_screen_width () - mreq.width);
	*y = CLAMP (*y, 0, gdk_screen_height () - mreq.height);
}

static gboolean
do_action (TasklistTask *task, gpointer data)
{
	MenuAction action = GPOINTER_TO_INT (data);
	Tasklist *tasklist = task->tasklist;
	GtkWidget *dialog;
	gint retval;

	switch (action) {
	case MENU_ACTION_SHADE_UNSHADE:
		action = GWMH_TASK_SHADED (task->gwmh_task) ? MENU_ACTION_UNSHADE : MENU_ACTION_SHADE;
		break;
	case MENU_ACTION_STICK_UNSTICK:
		action = GWMH_TASK_STICKY (task->gwmh_task) ? MENU_ACTION_UNSTICK : MENU_ACTION_STICK;
		break;
	case MENU_ACTION_SHOW_HIDE:
		action = GWMH_TASK_ICONIFIED (task->gwmh_task) ? MENU_ACTION_SHOW : MENU_ACTION_HIDE;
		break;
	default:
		break;
	}

	switch (action) {
	case MENU_ACTION_UNSHADE:
		gwmh_task_unset_gstate_flags (task->gwmh_task,
					      GWMH_STATE_SHADED);
		break;
	case MENU_ACTION_SHADE:
		gwmh_task_set_gstate_flags (task->gwmh_task,
					    GWMH_STATE_SHADED);
		break;
	case MENU_ACTION_UNSTICK:
		gwmh_task_unset_gstate_flags (task->gwmh_task,
					      GWMH_STATE_STICKY);
		break;
	case MENU_ACTION_STICK:
		gwmh_task_set_gstate_flags (task->gwmh_task,
					    GWMH_STATE_STICKY);
		break;
	case MENU_ACTION_KILL:
		if (!tasklist->config.confirm_before_kill && task->destroyed == FALSE) {
			gwmh_task_kill (task->gwmh_task);
			break;
		}
		dialog = gnome_message_box_new(_("Warning! Unsaved changes will be lost!\nProceed?"),
					       GNOME_MESSAGE_BOX_WARNING,
					       GNOME_STOCK_BUTTON_YES,
					       GNOME_STOCK_BUTTON_NO,
					       NULL);
		gtk_widget_show(dialog);
		retval = gnome_dialog_run(GNOME_DIALOG(dialog));
		
		if (retval)
			return TRUE;

		if (task->destroyed == FALSE)
			gwmh_task_kill(task->gwmh_task);
		break;
	case MENU_ACTION_SHOW:
		gwmh_desk_set_current_area (task->gwmh_task->desktop,
					    task->gwmh_task->harea,
					    task->gwmh_task->varea);
		gwmh_task_show (task->gwmh_task);
		gwmh_task_raise (task->gwmh_task);
		gwmh_task_focus (task->gwmh_task);
		break;
	case MENU_ACTION_HIDE:
		gwmh_task_iconify (task->gwmh_task);
		break;
	case MENU_ACTION_CLOSE:
		gwmh_task_close (task->gwmh_task);
		break;
		
	default:
		d(g_print ("Menu Callback: %d\n", GPOINTER_TO_INT (data)));
	}
	return FALSE;

}

/* Callback for menus */
static gboolean
cb_menu (GtkWidget *widget, gpointer data)
{
	MenuAction action = GPOINTER_TO_INT (data);
	GtkWidget *dialog;
	gint retval;
	int config_save = 0;

	TasklistTask *task = gtk_object_get_data (GTK_OBJECT (widget), "task");

	if (task == NULL)
		return FALSE;

	if (!task->task_group)
		return do_action (task, data);

	if (!task->vtasks)
		return FALSE;
	
	if (action == MENU_ACTION_KILL) {
		config_save = task->tasklist->config.confirm_before_kill;
		if (config_save) {
			dialog = gnome_message_box_new(_("Warning! Unsaved changes will be lost!\nProceed?"),
						       GNOME_MESSAGE_BOX_WARNING,
						       GNOME_STOCK_BUTTON_YES,
						       GNOME_STOCK_BUTTON_NO,
						       NULL);
			gtk_widget_show(dialog);
			retval = gnome_dialog_run(GNOME_DIALOG(dialog));
			
			if (retval)
				return TRUE;
			
			task->tasklist->config.confirm_before_kill = FALSE;
		}
	}
	g_slist_foreach (task->vtasks, (GFunc)do_action, data);

	if (action == MENU_ACTION_KILL)
		task->tasklist->config.confirm_before_kill = config_save;

	return FALSE;
}

/* Add a menu item to the popup menu */
static void 
add_menu_item (TasklistTask *task, gchar *name, GtkWidget *menu, MenuAction action, gchar **xpm)
{
	GtkWidget *menuitem;
	GdkPixmap *pixmap;
	GtkWidget *label;
	GdkBitmap *mask;
	GtkWidget *gtkpixmap;

	menuitem = gtk_pixmap_menu_item_new ();
	label = gtk_label_new (name);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (menuitem), label);
	if (xpm) {
		pixmap = gdk_pixmap_create_from_xpm_d (task->tasklist->area->window, &mask, NULL, xpm);
		gtkpixmap = gtk_pixmap_new (pixmap, mask);
		gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (menuitem), gtkpixmap);
	}

	gtk_object_set_data (GTK_OBJECT (menuitem), "task", task);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (cb_menu), GINT_TO_POINTER (action));
	
	if (task->group && task->group->menu)
		gtk_signal_connect_object (GTK_OBJECT (menuitem), "activate",
					   GTK_SIGNAL_FUNC (gtk_menu_shell_deactivate),
					   GTK_OBJECT (task->group->menu));

	gtk_widget_show_all (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

}

#if 0
/* Called when "Send to desktop" is used */
static void
cb_to_desktop (GtkWidget *widget, gpointer data)
{
	TasklistTask *task = gtk_object_get_data (GTK_OBJECT (widget), "task");

	if (task == NULL)
		return;
	
	gwmh_task_set_desktop (task->gwmh_task, 
			       GPOINTER_TO_INT (data));
	gwmh_task_set_desktop (task->gwmh_task, 
			       GPOINTER_TO_INT (data));
	tasklist_layout_tasklist (task->tasklist);
}
#endif

static void
destroy_menu (GtkWidget *w, gpointer null)
{
	d(g_print ("Destroying menu\n"));
	gtk_widget_unref (w);
}

/* Create a popup menu */
static GtkWidget *
get_popup_menu (TasklistTask *task)
{
	GtkWidget *menu, *menuitem; /*, *desktop, *label, *gtkpixmap;*/
	/*GdkPixmap *pixmap;*/
	/*GdkBitmap *mask;*/
	/*GwmhDesk *desk_info;*/

	/*gchar *wsname;*/
	/*int i, curworkspace;*/

	menu = gtk_menu_new ();
	gtk_signal_connect (GTK_OBJECT (menu), "deactivate",
			    GTK_SIGNAL_FUNC (destroy_menu),
			    NULL);

	gtk_widget_show (menu);

	gtk_object_set_data (GTK_OBJECT (menu), "task", task);

	add_menu_item (task, GWMH_TASK_ICONIFIED (task->gwmh_task)
		       ? _("Restore") : _("Iconify"), 
		       menu, MENU_ACTION_SHOW_HIDE,
		       GWMH_TASK_ICONIFIED (task->gwmh_task)
		       ? tasklist_restore_xpm : tasklist_iconify_xpm);

	add_menu_item (task, GWMH_TASK_SHADED (task->gwmh_task)
		       ? _("Unshade") : _("Shade"), 
		       menu, MENU_ACTION_SHADE_UNSHADE,
		       GWMH_TASK_SHADED (task->gwmh_task)
		       ? tasklist_unshade_xpm: tasklist_shade_xpm);

	add_menu_item (task, GWMH_TASK_STICKY (task->gwmh_task)
		       ? _("Unstick") : _("Stick"), 
		       menu, MENU_ACTION_STICK_UNSTICK,
		       GWMH_TASK_STICKY (task->gwmh_task)
		       ? tasklist_unstick_xpm : tasklist_stick_xpm);
#if 0
	menuitem = gtk_pixmap_menu_item_new ();
	label = gtk_label_new (_("To desktop"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (menuitem), label);
	pixmap = gdk_pixmap_create_from_xpm_d (tasklist->area->window, &mask, NULL,
					       tasklist_send_to_desktop_xpm);
	gtkpixmap = gtk_pixmap_new (pixmap, mask);
	gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (menuitem), gtkpixmap);
	gtk_widget_show_all (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

	if (!GWMH_TASK_STICKY (task->gwmh_task)) {
		desktop = gtk_menu_new ();
		gtk_widget_show (desktop);
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), desktop);
		
		desk_info = gwmh_desk_get_config ();
		curworkspace = desk_info->current_desktop;

		for (i=0; i<desk_info->n_desktops;i++) {
			if (desk_info->desktop_names[i])
				wsname = g_strdup_printf ("%s", desk_info->desktop_names[i]);
			else
				wsname = g_strdup_printf ("%d", i);
			menuitem = gtk_menu_item_new_with_label (wsname);
			gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
					    GTK_SIGNAL_FUNC (cb_to_desktop), i);
			gtk_object_set_user_data (GTK_OBJECT (menuitem), tasklist);
			if (i == curworkspace)
				gtk_widget_set_sensitive (menuitem, FALSE);
			gtk_widget_show (menuitem);
			gtk_menu_append (GTK_MENU (desktop), menuitem);
			g_free (wsname);
		}
	} else 
		gtk_widget_set_sensitive (menuitem, FALSE);

	menuitem = gtk_menu_item_new ();
	gtk_widget_show (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);
#endif
	add_menu_item (task, _("Close window"), menu, MENU_ACTION_CLOSE,
		       tasklist_close_xpm);

	menuitem = gtk_menu_item_new ();
	gtk_widget_show (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	
	add_menu_item (task, _("Kill app"), menu, MENU_ACTION_KILL, tasklist_kill_xpm);
	
	return menu;
}

/* Create a popup menu */
static GtkWidget *
get_group_popup_menu (TasklistTask *task)
{
	GtkWidget *menu, *menuitem; /*, *desktop, *label, *gtkpixmap;*/
	/*GdkPixmap *pixmap;*/
	/*GdkBitmap *mask;*/
	/*GwmhDesk *desk_info;*/

	/*gchar *wsname;*/
	/*int i, curworkspace;*/

	menu = gtk_menu_new ();
	gtk_signal_connect (GTK_OBJECT (menu), "deactivate",
			    GTK_SIGNAL_FUNC (destroy_menu),
			    NULL);
	gtk_widget_show (menu);

	gtk_object_set_data (GTK_OBJECT (menu), "task", task);

	/* if (iconified window in group) */
	add_menu_item (task, _("Restore All"), menu,
		       MENU_ACTION_SHOW, tasklist_restore_xpm);

	add_menu_item (task, _("Iconify All"), menu,
		       MENU_ACTION_HIDE, tasklist_iconify_xpm);

	add_menu_item (task, _("Unshade All"), menu,
		       MENU_ACTION_UNSHADE, tasklist_unshade_xpm);

	add_menu_item (task, _("Shade All"), menu,
		       MENU_ACTION_SHADE, tasklist_shade_xpm);

	add_menu_item (task, _("Unstick All"), menu,
		       MENU_ACTION_UNSTICK, tasklist_unstick_xpm);

	add_menu_item (task, _("Stick All"), menu,
		       MENU_ACTION_STICK, tasklist_stick_xpm);

#if 0
	menuitem = gtk_pixmap_menu_item_new ();
	label = gtk_label_new (_("To desktop"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (menuitem), label);
	pixmap = gdk_pixmap_create_from_xpm_d (tasklist->area->window, &mask, NULL,
					       tasklist_send_to_desktop_xpm);
	gtkpixmap = gtk_pixmap_new (pixmap, mask);
	gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (menuitem), gtkpixmap);
	gtk_widget_show_all (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

	if (!GWMH_TASK_STICKY (task->gwmh_task)) {
		desktop = gtk_menu_new ();
		gtk_widget_show (desktop);
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), desktop);
		
		desk_info = gwmh_desk_get_config ();
		curworkspace = desk_info->current_desktop;

		for (i=0; i<desk_info->n_desktops;i++) {
			if (desk_info->desktop_names[i])
				wsname = g_strdup_printf ("%s", desk_info->desktop_names[i]);
			else
				wsname = g_strdup_printf ("%d", i);
			menuitem = gtk_menu_item_new_with_label (wsname);
			gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
					    GTK_SIGNAL_FUNC (cb_to_desktop), i);
			gtk_object_set_user_data (GTK_OBJECT (menuitem), tasklist);
			if (i == curworkspace)
				gtk_widget_set_sensitive (menuitem, FALSE);
			gtk_widget_show (menuitem);
			gtk_menu_append (GTK_MENU (desktop), menuitem);
			g_free (wsname);
		}
	} else 
		gtk_widget_set_sensitive (menuitem, FALSE);

	menuitem = gtk_menu_item_new ();
	gtk_widget_show (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);
#endif
	add_menu_item (task, _("Close All"), menu,
		       MENU_ACTION_CLOSE, tasklist_close_xpm);
		       

	menuitem = gtk_menu_item_new ();
	gtk_widget_show (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	
	add_menu_item (task, _("Kill All"), menu, MENU_ACTION_KILL, tasklist_kill_xpm);
	
	return menu;
}

static void
redraw_task (GtkWidget *w, TasklistTask *task)
{
	tasklist_draw_task (task, NULL);
}

/* Open a popup menu with window operations */
void
tasklist_menu_popup (TasklistTask *task, guint button, guint32 activate_time)
{
	tasklist_clean_menu (task);

	task->menu = task->task_group 
		? get_group_popup_menu (task)
		: get_popup_menu (task);

	gtk_signal_connect(GTK_OBJECT(task->menu), "destroy",
			   GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			   &task->menu);

	tasklist_draw_task (task, NULL);
	gtk_signal_connect (GTK_OBJECT (task->menu), "destroy",
			    GTK_SIGNAL_FUNC (redraw_task), task);

	gtk_menu_popup (GTK_MENU (task->menu), NULL, NULL,
			cb_menu_position, task,
			button, activate_time);
}

/*most of this function stolen from the real gtk_menu_popup*/
static void
restore_grabs(GtkWidget *w, gpointer data)
{
	GtkWidget *menu_item = data;
	GtkMenu *menu = GTK_MENU(menu_item->parent); 
	GtkWidget *xgrab_shell;
	GtkWidget *parent;

	d(g_print ("restore_grabs\n"));
	/* Find the last viewable ancestor, and make an X grab on it
	 */
	parent = GTK_WIDGET (menu);
	xgrab_shell = NULL;
	while (parent) {
		gboolean viewable = TRUE;
		GtkWidget *tmp = parent;

		while (tmp) {
			if (!GTK_WIDGET_MAPPED (tmp)) {
				viewable = FALSE;
				break;
			}
			tmp = tmp->parent;
		}

		if (viewable)
			xgrab_shell = parent;

		parent = GTK_MENU_SHELL (parent)->parent_menu_shell;
	}

	/*only grab if this HAD a grab before*/
	if (xgrab_shell && (GTK_MENU_SHELL (xgrab_shell)->have_xgrab)) {
		GdkCursor *cursor = gdk_cursor_new (GDK_ARROW);

		GTK_MENU_SHELL (xgrab_shell)->have_xgrab = 
			(gdk_pointer_grab (xgrab_shell->window, TRUE,
					   GDK_BUTTON_PRESS_MASK |
					   GDK_BUTTON_RELEASE_MASK |
					   GDK_ENTER_NOTIFY_MASK |
					   GDK_LEAVE_NOTIFY_MASK,
					   NULL, cursor, 0) == 0);
		gdk_cursor_destroy (cursor);
	}
	
	gtk_grab_add (GTK_WIDGET (menu));
}

static gboolean
cb_activate_task (GtkWidget *w, GdkEventButton *event, gpointer data)
{
	TasklistTask *task = gtk_object_get_data (GTK_OBJECT (w), "task");
	if (task == NULL)
		return FALSE;

	gwmh_desk_set_current_area (task->gwmh_task->desktop,
				    task->gwmh_task->harea,
				    task->gwmh_task->varea);
	gwmh_task_show (task->gwmh_task);
	gwmh_task_raise (task->gwmh_task);
	gwmh_task_focus (task->gwmh_task);

	return FALSE;
}

static gboolean
cb_show_popup (GtkWidget *w, GdkEventButton *event, gpointer data)
{
	TasklistTask *task = gtk_object_get_data (GTK_OBJECT (w), "task");
	if (task == NULL)
		return FALSE;

	if (event->type != GDK_BUTTON_PRESS) return FALSE;

	if (event->button == 1) {
		d(g_print ("click!\n"));
		gwmh_desk_set_current_area (task->gwmh_task->desktop,
					    task->gwmh_task->harea,
					    task->gwmh_task->varea);
		gwmh_task_show (task->gwmh_task);
		gwmh_task_raise (task->gwmh_task);
		gwmh_task_focus (task->gwmh_task);
		return TRUE;
	}
	
	if (event->button == 3) {
		/*gtk_signal_emit_stop_by_name (GTK_OBJECT (w), "button_press_event");*/
		tasklist_clean_menu (task);

		task->menu = get_popup_menu (task);

		gtk_signal_connect (GTK_OBJECT(task->menu), "destroy",
				    GTK_SIGNAL_FUNC(gtk_widget_destroyed),
				    &task->menu);

		gtk_signal_connect(GTK_OBJECT(task->menu),"deactivate",
				   GTK_SIGNAL_FUNC(restore_grabs), w);

		gtk_menu_popup (GTK_MENU (task->menu), NULL, NULL,
				NULL, NULL, event->button, event->time);

		return FALSE;
	}
	return FALSE;
}

static void
cb_menuitem_destroyed (GtkWidget *menuitem, gpointer data)
{
	TasklistTask *task = gtk_object_get_data (GTK_OBJECT (menuitem),
						  "task");
	if (task == NULL)
		return;

	task->menuitem = NULL;
}

static void
create_task_item (TasklistTask *task, TasklistTask *group)
{
	GtkWidget *pixmap, *label;
	GdkPixmap *pix;
	GdkBitmap *bit;
	gchar *label_string, *tooltip;
	int width;

	static GwmhDesk *desk = NULL;

	gdk_pixbuf_render_pixmap_and_mask (
		task->gwmh_task->iconified
		? task->icon->minimized
		: task->icon->normal, &pix, &bit, 128);

	pixmap = gtk_pixmap_new (pix, bit);

	gdk_pixmap_unref (pix);
	gdk_bitmap_unref (bit);

	/* are we leaking the pixmap and bitmap? */
	
	task->menuitem = gtk_pixmap_menu_item_new ();
	gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (task->menuitem), pixmap);

	tooltip = tasklist_task_get_label (task, 0, FALSE);

	width = gdk_screen_width () / 3;
	if (group->width > width)
		width = group->width;
	label_string = tasklist_task_get_label (task, width, FALSE);
	label = gtk_label_new (label_string);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (task->menuitem), label);

	/* only set a tip if the label didn't fit properly */
	if (strcmp (tooltip, label_string) != 0)
		gtk_tooltips_set_tip (task->tasklist->tooltips, task->menuitem,
				      tooltip, NULL);
	g_free (tooltip);
	g_free (label_string);

	if (!desk)
		desk = gwmh_desk_get_config ();

	if (task->gwmh_task->desktop == desk->current_desktop &&
	    task->gwmh_task->harea   == desk->current_harea   &&
	    task->gwmh_task->varea   == desk->current_varea)
		gtk_menu_prepend (GTK_MENU (group->menu), task->menuitem);
	else
		gtk_menu_append (GTK_MENU (group->menu), task->menuitem);

	gtk_object_set_data (GTK_OBJECT (task->menuitem), "task", task);

	gtk_signal_connect (GTK_OBJECT (task->menuitem), "button_release_event",
			    GTK_SIGNAL_FUNC (cb_activate_task), NULL);
	gtk_signal_connect (GTK_OBJECT (task->menuitem), "button_press_event",
			    GTK_SIGNAL_FUNC (cb_show_popup), NULL);
	gtk_signal_connect (GTK_OBJECT (task->menuitem), "destroy",
			    GTK_SIGNAL_FUNC (cb_menuitem_destroyed), NULL);
}

/* Open a popup menu with windows in a group */
void
tasklist_group_popup (TasklistTask *task, guint button, guint32 activate_time)
{
	GtkWidget *menuitem;

	tasklist_clean_menu (task);

	task->menu = gtk_menu_new ();
	gtk_signal_connect (GTK_OBJECT (task->menu), "deactivate",
			    GTK_SIGNAL_FUNC (destroy_menu),
			    NULL);

	if (task->tasklist->config.all_desks_normal ||
	    task->tasklist->config.all_desks_minimized) {
		/* seperator */
		menuitem = gtk_menu_item_new ();
		gtk_widget_set_sensitive (menuitem, FALSE);
		gtk_widget_show (menuitem);
		gtk_menu_append (GTK_MENU (task->menu), menuitem);
	}

	g_slist_foreach (task->vtasks, (GFunc)create_task_item, task);

	gtk_widget_show_all (task->menu);
	gtk_signal_connect (GTK_OBJECT (task->menu), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &task->menu);

	tasklist_draw_task (task, NULL);
	gtk_signal_connect (GTK_OBJECT (task->menu), "destroy",
			    GTK_SIGNAL_FUNC (redraw_task), task);

	gtk_object_set_data (GTK_OBJECT (task->menu), "task", task);

	tasklist_draw_task (task, NULL);

	gtk_menu_popup (GTK_MENU (task->menu), NULL, NULL,
			cb_menu_position, task,
			button, activate_time);
}
