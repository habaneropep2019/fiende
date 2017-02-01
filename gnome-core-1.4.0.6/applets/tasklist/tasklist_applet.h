#ifndef _TASKLIST_APPLET_H_
#define _TASKLIST_APPLET_H_

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "applet-widget.h"
#include "gwmh.h"

/* The row height of a task */
#define ROW_HEIGHT 24
typedef struct _Tasklist Tasklist;
typedef struct _TasklistTask TasklistTask;
typedef struct _TasklistConfig TasklistConfig;
typedef struct _TasklistIcon TasklistIcon;

/* Simple enum for which tasks to show */
enum
{
	TASKS_SHOW_ALL,
	TASKS_SHOW_MINIMIZED,
	TASKS_SHOW_NORMAL
};

typedef enum
{
	MENU_ACTION_CLOSE,
	MENU_ACTION_SHOW,
	MENU_ACTION_HIDE,
	MENU_ACTION_SHOW_HIDE,
	MENU_ACTION_SHADE,
	MENU_ACTION_UNSHADE,
	MENU_ACTION_SHADE_UNSHADE,
	MENU_ACTION_STICK,
	MENU_ACTION_UNSTICK,
	MENU_ACTION_STICK_UNSTICK,
	MENU_ACTION_KILL,
	MENU_ACTION_LAST
} MenuAction;

struct _TasklistTask {
	Tasklist *tasklist;
	gint x, y;
	gint width, height;
	gint fullwidth;
	TasklistIcon *icon;
	Pixmap wmhints_icon;
	GtkWidget *menu;

	/* 
	 * if we are a group, this is not a real task but a false one
	 * filled in with what the task group should be like
	 */
	GwmhTask *gwmh_task;

	/* this could be a union */
	/* for real tasks */
	TasklistTask *group;
	GtkWidget *menuitem; /* the menuitem in the groups menu */

	/* for task groups */
	char *group_name;
	GSList *tasks;  /* all tasks in group */
	GSList *vtasks; /* visible tasks in group */
	TasklistTask *focused_task; /* the task in our group last focused */
	
	/* whether we are in the vtasks list */
	gboolean visible;
	/* whether we are a task group */
	gboolean task_group;

	/* set when we get a notify that the gwmh window is destroyed */
	gboolean destroyed;
};

struct _TasklistConfig {

	gboolean show_mini_icons; /* Show small icons next to tasks */
	gboolean show_minimized; /* Show minimized tasks */
	gboolean show_normal; /* Show normal tasks */
	gboolean all_desks_normal; /* Show normal tasks on all desktops */
	gboolean all_desks_minimized; /* Show minimized tasks on all desktops */
	gboolean confirm_before_kill; /* Confirm before killing windows */
	gboolean move_to_current; /* Move iconified tasks to current workspace */
	gboolean sunken; /* Sunken or popped-up look */

	/* Follow the panel sizes */
	gboolean follow_panel_size;

	/*
	 * Stuff for horizontal mode
	 */

	/* The width of the tasklist */
	gint horz_width;

	/* Number of rows */	
	gint horz_rows;

	/* Fixed or dynamic sizing */
	gboolean horz_fixed;

	/* Width of a single task (for dynamic sizing) */
	gint horz_taskwidth; 

	/* in dynamic mode, never push applets */
	gboolean horz_never_push;

	/*
	 * Stuff for vertical mode
	 */
	/* The height of the tasklist */
	gint vert_height;
	
	/* The width of the tasklist */
	gint vert_width; 

	/* Fixed or dynamic sizing */
	gboolean vert_fixed; 

	/* a mode where the width is the max width of any window title */
	gboolean vert_width_full;

	/* in dynamic mode, never push applets */
	gboolean vert_never_push;

	/* grouping options */
	gboolean enable_grouping;
	gint grouping_min;

	/* tooltips */
	gboolean enable_tooltips;
};

struct _TasklistIcon {
	GdkPixbuf *normal;
	GdkPixbuf *minimized;
};

struct _Tasklist {
	GNOME_Panel_OrientType orient; /* Tasklist orient */
	GtkWidget *handle; /* The handle box */
	GtkWidget *applet; /* The applet */
	GtkWidget *area; /* The drawing area used to display tasks */

	/* The list of tasks used */
	GHashTable *tasks;

	/* our task groups, grouped by WM_CLASS */
	GHashTable *groups;

	/* 
	 * list of visible tasks.  this should be set by the gwmh
	 * listner 
	 */
	GSList *vtasks;

	/* idle timeout for re-laying out */
	guint layout_idle;

#define MOTION_TIMEOUT 500 /* Show task motion_task if cursor over task for ... msec */

	/* Show task motion_task after MOTION_TIMEOUT in drag_motion */
	int motion_timeout;

	/* Task to show after motion_timeout */
	TasklistTask *motion_task; 

	/* Vertical height, used for resizing */
	gint vert_height;

	/* Horizontal width, used for resizing */
	gint horz_width;  

	gint panel_size;

	/* The configuration */
	TasklistConfig config;
	TasklistIcon *unknown_icon; /* The unknown icon */

	/*
	 * Used during configuration editing
	 */
	/* The tasklist properties configuration */
	TasklistConfig PropsConfig;

	/* The Property box */
	GtkWidget *prop;

	/* The About box */
	GtkWidget *about_dialog;
	
	guint task_notifier_id;
	guint desk_notifier_id;

	/* Thy evil fake widget for doing tooltips */
	GtkWidget *fake_tooltip_widget;
	GtkTooltips *tooltips;
	TasklistTask *tooltip_task;
};

void   	   tasklist_menu_popup          (TasklistTask *task, guint button,
					 guint32 activate_time);
void   	   tasklist_group_popup         (TasklistTask *task, guint button,
					 guint32 activate_time);
void       tasklist_freeze              (Tasklist *tasklist);
void       tasklist_thaw                (Tasklist *tasklist);
void       tasklist_draw_task           (TasklistTask *task, GdkRectangle *rect);
void   	   tasklist_display_properties  (Tasklist *);
void   	   tasklist_read_config         (Tasklist *);
void       tasklist_clean_menu          (TasklistTask *);
gboolean   tasklist_write_config        (GtkWidget *w,
					 const gchar *privcfgpath,
					 const gchar *globcfgpath,
					 gpointer data);
gchar     *tasklist_task_get_label      (TasklistTask *, int, gboolean add_groupcount);
void       tasklist_change_size         (Tasklist *,
					 gboolean layout,
					 int fullwidth);
void       tasklist_redo_vtasks         (Tasklist *);
void       tasklist_layout_tasklist     (Tasklist *tasklist);
GdkPixbuf *tasklist_icon_create_minimized_icon (Tasklist *, GdkPixbuf *pixbuf);
void       tasklist_icon_set        (TasklistTask *task);
void       tasklist_icon_destroy    (TasklistTask *task);
Pixmap     tasklist_icon_get_pixmap (TasklistTask *task);

#endif /* _TASKLIST_APPLET_H_ */
