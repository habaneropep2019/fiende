#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "gstc.h"
#include "gwmh.h"

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "tasklist_applet.h"
#include "unknown.xpm"

/* from gtkhandlebox.c */
#define DRAG_HANDLE_SIZE 10

/* define to x for debugging output */
#define d(x)

#define APPLET_COMPILE_AS_PROCESS
/* sorting the list... */
static gint
tlsort (gconstpointer data1, gconstpointer data2)
{
	const TasklistTask *la = data1;
	const TasklistTask *lb = data2;
	const char *sa, *sb;

	d(g_print ("la: %p\tlb: %p\n", la, lb));

	sa = la->task_group ? la->group_name : (la->group ? la->group->group_name : la->gwmh_task->name);
	sb = lb->task_group ? lb->group_name : (lb->group ? lb->group->group_name : lb->gwmh_task->name);

	if (sa && sb) {
		int cmp = g_strcasecmp (sa, sb);
		/* if the same, then sort by the window id, this is arbitrary
		 * but makes tasks of the same group stay in the same order */
		if (cmp == 0) {
			if (la->gwmh_task->xwin > lb->gwmh_task->xwin)
				return 1;
			else
				return -1;
		} else {
			return cmp;
		}
	} else if (sa) {
		return 1;
	} else if (sb) {
		return -1;
	} else {
		return 0;
	}
}

static void
clamp_size (Tasklist *tasklist, int *size)
{
	int free_space;

	free_space = applet_widget_get_free_space (APPLET_WIDGET (tasklist->applet));

	if (free_space > 0 && free_space < *size)
		*size = free_space;
}

void
tasklist_clean_menu (TasklistTask *task)
{
	if(task->menu) {
		d(g_print ("group had a menu: %p\t", task->menu));
		gtk_widget_destroy (task->menu);
		d(g_print ("%p\n", task->menu));
	}
}

static char *
get_task_class (GwmhTask *task)
{
	XClassHint hint;
	char *retval;
	Status status;

	gdk_error_trap_push ();
	status = XGetClassHint (GDK_DISPLAY (), task->xwin, &hint);
	gdk_flush ();
	if (gdk_error_trap_pop ())
		return NULL;

	if ( ! status)
		return NULL;

	d(g_print ("name: %s\tclass: %s\n", hint.res_name, hint.res_class));
	retval = g_strdup (hint.res_class);

	XFree (hint.res_name);
	XFree (hint.res_class);

	return retval;
}

/* get the horz_rows depending on the configuration settings */
static gint
get_horz_rows(Tasklist *tasklist)
{
	int result;

	g_return_val_if_fail (tasklist != NULL, 1);
			      
	if (tasklist->config.follow_panel_size)
		result = tasklist->panel_size/ROW_HEIGHT;
	else
		result = tasklist->config.horz_rows;

	if (result < 1)
		result = 1;

	return result;
}

/* Shorten a label that is too long */
gchar *
tasklist_task_get_label (TasklistTask *task, int width, gboolean add_groupcount)
{	
	Tasklist *tasklist = task->tasklist;
	gchar *das_string;
	gchar *str, *tempstr, *groupcount = NULL;
	gint len, label_len, overhead, allowed_width;

	das_string = task->gwmh_task->name;

	if (das_string == NULL)
		das_string = _("???");

	label_len = gdk_string_width (tasklist->area->style->font, das_string);

	overhead = tasklist->config.show_mini_icons ? 30 : 6;

	if (add_groupcount) {
		GSList *vtasks = task->vtasks;
		/* not sure why this was there but at least
		 * now it has the correct logic, perhaps sometime
		 * we'd want to draw a task with it's group count */
		if (vtasks == NULL &&
		    task->group != NULL)
			vtasks = task->group->vtasks;
		groupcount = g_strdup_printf ("(%d) ", g_slist_length (vtasks));
		
		overhead += 10 + gdk_string_width (tasklist->area->style->font, 
						   groupcount);
	}
	
	if (GWMH_TASK_ICONIFIED (task->gwmh_task))
		overhead += gdk_string_width (tasklist->area->style->font, "[]");	

	allowed_width = width - overhead;
	
	if ( (width > 0) && (label_len > allowed_width) ) {
		GdkWChar *wstr;

		g_assert (width > 0);

		len = strlen (das_string);
		wstr = g_new (GdkWChar, len + 3);
		len = gdk_mbstowcs (wstr, das_string, len);
		/* ok, the below thing is broken */
		if ( len < 0 ) { /* if the conversion is failed */
			wstr[0] = wstr[1] = wstr[2] = '?'; 
			wstr[3] = '\0'; /* wcscpy(wstr,"???");*/
			len = 3;
			label_len = gdk_text_width_wc(tasklist->area->style->font,
                                                      wstr, len);
			if (label_len <= allowed_width) {
				str = gdk_wcstombs(wstr);
				g_free(wstr);
				goto finish_label_up;
			}
		}
		wstr[len] = wstr[len+1] = '.';
		wstr[len+2] = '\0'; /*wcscat(wstr,"..");*/
		len--;
		
		for (; len > 0; len--) {
			wstr[len] = '.';
			wstr[len + 3] = '\0';
			
			label_len = gdk_text_width_wc (tasklist->area->style->font,
						       wstr, len + 3);
			
			if (label_len <= allowed_width)
				break;
		}
		str = gdk_wcstombs (wstr);
		g_free (wstr);
	} else {
		str = g_strdup (das_string);
	}

finish_label_up:

	if (task->gwmh_task && GWMH_TASK_ICONIFIED (task->gwmh_task)) {
		tempstr = g_strdup_printf ("[%s]", str);	
		g_free(str);
		str = tempstr;
	}
	
	if (groupcount) {
		tempstr = g_strconcat (groupcount, str, NULL);
		g_free (str);
		str = tempstr;
	}

	return str;
}

/* Check if a task is "visible", 
   if it should be drawn onto the tasklist */
static gboolean
is_task_visible (TasklistTask *task)
{
	Tasklist *tasklist;
	GwmhDesk *desk_info;

	if (!task || task->destroyed || task->task_group)
		return FALSE;

	tasklist = task->tasklist;

	desk_info = gwmh_desk_get_config ();

	if (GWMH_TASK_SKIP_TASKBAR (task->gwmh_task))
		return FALSE;
	
	if (task->gwmh_task->desktop != desk_info->current_desktop ||
	    task->gwmh_task->harea != desk_info->current_harea ||
	    task->gwmh_task->varea != desk_info->current_varea) {
		if (!GWMH_TASK_STICKY (task->gwmh_task)) {
			if (!tasklist->config.all_desks_minimized && 
			    !tasklist->config.all_desks_normal)
				return FALSE;
				
			else if (tasklist->config.all_desks_minimized && 
				 !tasklist->config.all_desks_normal) {
				if (!GWMH_TASK_ICONIFIED (task->gwmh_task))
					return FALSE;
			}
			else if (tasklist->config.all_desks_normal && 
				 !tasklist->config.all_desks_minimized) {
				if (GWMH_TASK_ICONIFIED (task->gwmh_task))
					return FALSE;
			}
		}
	}			

	if (GWMH_TASK_ICONIFIED (task->gwmh_task)) {
		if (!tasklist->config.show_minimized)
			return FALSE;
	} else {
		if (!tasklist->config.show_normal)
			return FALSE;
	}
		
	return TRUE;
}

static gboolean
is_task_really_visible (TasklistTask *task)
{
	g_return_val_if_fail (task != NULL, FALSE);

	if (!task->tasklist->config.enable_grouping)
		return is_task_visible (task);

	/* we can probably unroll the length test */
	if (task->group && g_slist_length (task->group->vtasks) > task->tasklist->config.grouping_min)
		return FALSE;
	else if (task->task_group)
		return g_slist_length (task->vtasks) > task->tasklist->config.grouping_min;
	return is_task_visible (task);
}

static void print_task (TasklistTask *task);

static void
print_task_iterator (gpointer data, gpointer user_data)
{
	TasklistTask *task = data;
	print_task (task);
}

static void
print_task (TasklistTask *task)
{
	if (!task)
		g_print (" * * NULL TASK * *\n");
	else if (task->group)
		g_print ("task: %p (%p) [%d, %d]: %s\n", 
			 task, task->gwmh_task,
			 is_task_visible (task),
			 is_task_really_visible (task),
			 task->gwmh_task->name);
	else if (task->task_group) {
		g_print ("group: %p [%d]: %s\n", task, is_task_really_visible (task), task->group_name);
		g_slist_foreach (task->tasks, print_task_iterator, NULL);
		g_print ("/\n");
	} else {
		g_print ("Unknown task: %p\n", task);
		g_assert_not_reached ();
	}
}

/* returns TRUE if the group entry should be redrawn */
static gboolean
fixup_group (TasklistTask *group)
{
	TasklistTask *focused_task;
	gboolean iconified;
	TasklistTask *task;
	GSList *item;
	
	g_return_val_if_fail (group != NULL, FALSE);
	
	focused_task = NULL;
	iconified = TRUE;
	
	g_slist_free (group->vtasks);
	group->vtasks = NULL;
	
	for (item = group->tasks; item; item = item->next) {
		task = (TasklistTask *)item->data;
		if (is_task_visible (task)) {
			group->vtasks = g_slist_prepend (group->vtasks, task);
			if (!task->gwmh_task->iconified)
				iconified = FALSE;
			if (task->gwmh_task->focused)
				focused_task = task;
		}
	}

	if (group->focused_task != focused_task ||
	    group->gwmh_task->iconified != iconified) {
		group->focused_task = focused_task;
		group->gwmh_task->iconified = iconified;

		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean fixup_vtask (TasklistTask *task, gboolean *redraw);

static void
fixup_vtask_iterator (gpointer data, gpointer user_data)
{
	TasklistTask *task = data;
	fixup_vtask (task, NULL);
}

static gboolean
fixup_vtask (TasklistTask *task, gboolean *redraw)
{
	gboolean visible;

	/* why not layout if we are confused */
	g_return_val_if_fail (task, TRUE);

	if (task->task_group) {
		gboolean do_redraw = fixup_group (task);
		if (redraw != NULL)
			*redraw = do_redraw;
	}

	visible = is_task_really_visible (task);

	if (visible == task->visible)
		return FALSE;

	task->visible = visible;
	if (visible) {
		task->tasklist->vtasks =
			g_slist_insert_sorted (task->tasklist->vtasks, task, tlsort);
	} else {
		task->tasklist->vtasks =
			g_slist_remove (task->tasklist->vtasks, task);
	}

	if (task->tasklist->config.enable_grouping) {
		if (task->task_group) {
			g_slist_foreach (task->tasks, fixup_vtask_iterator, NULL);
		} else if (task->group) {
			fixup_vtask (task->group, NULL);
		}
	}

	d(g_print (">>>>>\tfixup_vtask "));
	d(print_task (task));
	d(g_slist_foreach (task->tasklist->vtasks, print_task_iterator, NULL));
	d(g_print ("<<<<<\n"));

	return TRUE;
}

static void
redo_groups_iterator (gpointer key, gpointer data, gpointer user_data)
{
	TasklistTask *task = data;
	Tasklist *tasklist = user_data;

	d (print_task (task));

	fixup_group (task);
	task->visible = is_task_really_visible (task);
	if (task->visible)
		tasklist->vtasks = g_slist_insert_sorted (tasklist->vtasks, task, tlsort);
}

void
tasklist_redo_vtasks (Tasklist *tasklist)
{
	TasklistTask *task;
	GList *item;
	
	if (tasklist->vtasks) {
		g_slist_free (tasklist->vtasks);
		tasklist->vtasks = NULL;
	}

	d(g_print ("\n\n\n\n\n\n\n\n\n\nredo_vtasks\n\n\n\n\n\n\n\n\n\n"));

	if (tasklist->config.enable_grouping)
		g_hash_table_foreach (tasklist->groups, redo_groups_iterator, tasklist);

	for (item = gwmh_task_list_get (); item; item = item->next) {
		task = g_hash_table_lookup (tasklist->tasks, item->data);

		/* this should never actually happen */
		if (!task) continue;

		task->visible = is_task_really_visible (task);
		
		if (task->visible)
			tasklist->vtasks = g_slist_insert_sorted (tasklist->vtasks, task, tlsort);
	}

#if 0
	if (!tasklist->config.sort_tasklist)
		tasklist->vtasks = g_slist_reverse (tasklist->vtasks);
#endif
	d(g_print ("\n\n\n\n\n\n\n\n\n\nvtasks: %d\n", g_slist_length (tasklist->vtasks)));

	d(g_print (">>>>>\tredo_vtasks:\n"));
	d(g_slist_foreach (tasklist->vtasks, print_task_iterator, NULL));
	d(g_print ("<<<<<\n"));
}

/* Check what task (if any) is at position x,y on the tasklist */
static TasklistTask *
task_get_xy (Tasklist *tasklist, gint x, gint y)
{
	GSList *temp_tasks, *temp;
	TasklistTask *task;

	temp_tasks = tasklist->vtasks;

	for (temp = temp_tasks; temp != NULL; temp = temp->next) {
		task = (TasklistTask *)temp->data;
		if (x > task->x &&
		    x < task->x + task->width &&
		    y > task->y &&
		    y < task->y + task->height)
			return task;
	}

	return NULL;
}

static void
draw_dot (GdkWindow *window, GdkGC *lgc, GdkGC *dgc, int x, int y)
{
	gdk_draw_point (window, dgc, x,   y);
	gdk_draw_point (window, lgc, x+1, y+1);
}

/* Draw a single task */
void
tasklist_draw_task (TasklistTask *task, GdkRectangle *rect)
{
	TasklistTask *real_task;
	gchar *tempstr;
	gint text_height, text_width;
	gboolean focused;

	/* For mini icons */
	TasklistIcon *icon;
	GdkPixbuf *pixbuf;
	
	Tasklist *tasklist;

	if (!is_task_really_visible (task))
		return;

	/* is_task_visible should return FALSE for task == NULL */
	g_assert (task != NULL);

	tasklist = task->tasklist;
	
	real_task = task;
	if (is_task_visible (task->focused_task) && GWMH_TASK_FOCUSED (task->focused_task->gwmh_task))
		task = task->focused_task;

	focused = GWMH_TASK_FOCUSED (task->gwmh_task) || real_task->menu != NULL;

	gtk_paint_box (tasklist->area->style, tasklist->area->window,
		       focused ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL,		       
		       focused ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
		       rect, tasklist->area, "button",
		       real_task->x, real_task->y,
		       real_task->width, real_task->height);

	tempstr = tasklist_task_get_label (task, real_task->width, real_task->task_group);
	if (tempstr) {
		text_height = gdk_string_height (tasklist->area->style->font, "1");
		text_width = gdk_string_width (tasklist->area->style->font, tempstr);
		gdk_draw_string (tasklist->area->window,
				 tasklist->area->style->font,
				 focused ?
				 tasklist->area->style->fg_gc[GTK_STATE_ACTIVE] :
				 tasklist->area->style->fg_gc[GTK_STATE_NORMAL],
				 real_task->x +
				 (tasklist->config.show_mini_icons ? 10 : 0) +
				 ((real_task->width - text_width) / 2),
				 real_task->y + ((real_task->height - text_height) / 2) + text_height,
				 tempstr);

		g_free (tempstr);
	}

	if (tasklist->config.show_mini_icons) {
		icon = task->icon;
		
		if ( GWMH_TASK_ICONIFIED (task->gwmh_task))
			pixbuf = icon->minimized;
		else
			pixbuf = icon->normal;

		gdk_pixbuf_render_to_drawable_alpha (
			pixbuf,
			tasklist->area->window,
			0, 0,
			real_task->x + 3 + (16 - gdk_pixbuf_get_width (pixbuf)) / 2,
			real_task->y + (real_task->height - gdk_pixbuf_get_height (pixbuf)) / 2,
			gdk_pixbuf_get_width (pixbuf),
			gdk_pixbuf_get_height (pixbuf),
			GDK_PIXBUF_ALPHA_BILEVEL,
			127,
			GDK_RGB_DITHER_NORMAL,
			gdk_pixbuf_get_width (pixbuf),
			gdk_pixbuf_get_height (pixbuf));

	}

	if (real_task->task_group) {
		GtkStyle *style;
		GdkWindow *window;
		GdkGC *lgc, *dgc;
		int x, y, i, j;

		style = tasklist->area->style;

		lgc = style->light_gc[focused ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL];
		dgc = style->dark_gc[focused ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL];

		window = tasklist->area->window;

		x = real_task->x + real_task->width - style->klass->ythickness - 10;
		y = real_task->y + style->klass->xthickness + 2;

		for (i = 0; i < 3; i++) {
			for (j = i; j < 3; j++) {
				draw_dot (window, lgc, dgc, x + j*3, y + i*3);
			}
		}

		

#if 0
		gtk_draw_arrow (tasklist->area->style, tasklist->area->window,
				focused ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL,
				GTK_SHADOW_ETCHED_IN,
				GTK_ARROW_DOWN, TRUE,
				real_task->x + real_task->width - 12,
				real_task->y + (real_task->height - 8) / 2,
				9, 8);
#endif
	}

}


static int
max_width (GSList *tasks)
{
	TasklistTask *task;
	GSList *li;
	int maxwidth = 0;
	char *s;

	for (li = tasks; li; li = li->next) {
		task = li->data;

		if (task->fullwidth < 0) {
			s = tasklist_task_get_label (task, -1, task->task_group);
			task->fullwidth = gdk_string_width (task->tasklist->area->style->font, s);
			g_free (s);
		}

		maxwidth = MAX (maxwidth,
				task->fullwidth + 
				(task->tasklist->config.show_mini_icons 
				 ? ROW_HEIGHT + 10 : 0));
	}
	return MIN (maxwidth, 512);
}

/* Layout the tasklist */
static gboolean
real_layout_tasklist (gpointer data)
{
	Tasklist *tasklist = data;
	gint j = 0, k = 0, num = 0, p = 0;
	TasklistTask *task;
	GSList *temp = NULL;
	gint num_rows = 0, num_cols = 0;
	gint curx = 0, cury = 0, curwidth = 0, curheight = 0;

	tasklist->layout_idle = 0;
	d(g_message ("Layout!"));

	if (!tasklist->vtasks) {
		d(g_message ("no tasks :(\n"));
		gtk_widget_draw (tasklist->area, NULL);
		return FALSE;
	}

	num = g_slist_length (tasklist->vtasks);
	
	switch (applet_widget_get_panel_orient (APPLET_WIDGET (tasklist->applet))) {
	case ORIENT_UP:
	case ORIENT_DOWN:
		if (num == 0) {
			if (tasklist->config.horz_fixed)
				tasklist->horz_width = tasklist->config.horz_width;
			else
				tasklist->horz_width = 4;

			tasklist_change_size (tasklist, FALSE, -1);
			
			gtk_widget_draw (tasklist->area, NULL);
			return FALSE;
		}

		while (p < num) {
			if (num < get_horz_rows(tasklist))
				num_rows = num;
			
			j++;
			if (num_cols < j)
				num_cols = j;
			if (num_rows < k + 1)
				num_rows = k + 1;
			
			if (get_horz_rows (tasklist) == 0 || j >= ((num + get_horz_rows(tasklist) - 1) / get_horz_rows(tasklist))) {
				j = 0;
				k++;
			}
			p++;
		}
		
		if (tasklist->config.horz_fixed) {
			curheight = (ROW_HEIGHT * get_horz_rows(tasklist) - (tasklist->config.sunken?4:0)) / num_rows;
			curwidth = (tasklist->config.horz_width - (tasklist->config.sunken?4:0)) / num_cols;

		} else {
			int width;

			width = tasklist->config.horz_taskwidth * num_cols + DRAG_HANDLE_SIZE;

			if (width > tasklist->config.horz_width)
				width = tasklist->config.horz_width;
			
			if (tasklist->config.horz_never_push)
				clamp_size (tasklist, &width);

			width -= DRAG_HANDLE_SIZE;

			curheight = (ROW_HEIGHT * get_horz_rows(tasklist) - (tasklist->config.sunken?4:0)) / num_rows;
#if 0
			/* If the total width is higher than allowed, 
			   we use the "fixed" way instead */
			if ((curwidth * num_cols) > tasklist->config.horz_width)
				curwidth = (tasklist->config.horz_width - 0) / num_cols;
#endif
			curwidth = width / num_cols;
		}


		curx = (tasklist->config.sunken?2:0);
		cury = (tasklist->config.sunken?2:0);


		for (temp = tasklist->vtasks; temp != NULL; temp = temp->next) {
			task = (TasklistTask *) temp->data;
			
			task->x = curx;
			task->y = cury;
			task->width = curwidth;
			task->height = curheight;
			
			if (tasklist->config.horz_fixed) {
				curx += curwidth;
			
				if (curx >= tasklist->config.horz_width ||
				    curx + curwidth > tasklist->config.horz_width) {
					cury += curheight;
					curx = (tasklist->config.sunken?2:0);
				}
			} else {

				curx += curwidth;

				if (curx >= num_cols * curwidth) {
					cury += curheight;
					curx = (tasklist->config.sunken?2:0);
				}
			}
		}

		if (tasklist->config.horz_fixed)
			tasklist->horz_width = tasklist->config.horz_width;
		else
			tasklist->horz_width = num_cols * curwidth + 4;

		tasklist_change_size (tasklist, FALSE, -1);

		break;

	case ORIENT_LEFT:
	case ORIENT_RIGHT:

		if (num == 0) {
			if (tasklist->config.vert_fixed)
				tasklist->vert_height = tasklist->config.vert_height;
			else
				tasklist->vert_height = 4;

			tasklist_change_size (tasklist, FALSE, -1);
			
			gtk_widget_draw (tasklist->area, NULL);
			return FALSE;
		}

		curheight = ROW_HEIGHT;
		if (tasklist->config.follow_panel_size)
			curwidth = tasklist->panel_size - (tasklist->config.sunken?4:0);
		else
			curwidth = tasklist->config.vert_width - (tasklist->config.sunken?4:0);
		
		if (tasklist->config.vert_width_full)
			curwidth = MAX (curwidth, max_width (tasklist->vtasks));

		num_cols = 1;
		num_rows = num;
		
		curx = (tasklist->config.sunken?2:0);
		cury = (tasklist->config.sunken?2:0);

		if (tasklist->config.vert_fixed) {
			tasklist->vert_height = tasklist->config.vert_height;
		} else {
			tasklist->vert_height = curheight * num_rows + 4 + DRAG_HANDLE_SIZE;
			if (tasklist->config.vert_never_push)
				clamp_size (tasklist, &tasklist->vert_height);
			tasklist->vert_height -= DRAG_HANDLE_SIZE;
		}

		tasklist_change_size (tasklist, FALSE, curwidth);
		
		for (temp = tasklist->vtasks; temp != NULL; temp = temp->next) {
			task = (TasklistTask *) temp->data;
			
			task->x = curx;
			task->y = cury;
			task->width = curwidth;
			task->height = curheight;
			
			curx += curwidth;

			if (curx >= (tasklist->config.follow_panel_size?
				     tasklist->panel_size:
				     tasklist->config.vert_width) - (tasklist->config.sunken?4:0)) {
				cury += curheight;
				curx = (tasklist->config.sunken?2:0);
			}
		}

		break;
	}

	gtk_widget_draw (tasklist->area, NULL);

	return FALSE;
}

/* this now actually just queues a relayout */
void
tasklist_layout_tasklist (Tasklist *tasklist)
{
	g_return_if_fail (tasklist);

	/* don't queue another timeout */
	if (tasklist->layout_idle) {
		d(g_message ("Skipped layout!"));
		return;
	}
	
	d(g_message ("Adding layout callback..."));
	tasklist->layout_idle = gtk_idle_add (real_layout_tasklist, tasklist);
}

#if 0
/* Get a task from the list that has got the given gwmh_task */
static TasklistTask *
find_gwmh_task (Tasklist *tasklist, GwmhTask *gwmh_task)
{
	GList *temp_tasks;
	TasklistTask *task;

	temp_tasks = tasklist->tasks;

	while (temp_tasks) {
		task = (TasklistTask *)temp_tasks->data;
		if (task->gwmh_task == gwmh_task)
			return task;
		temp_tasks = temp_tasks->next;
	}
	
	return NULL;
}
#endif

/* This routine gets called when desktops are switched etc */
static gboolean
desk_notifier (gpointer func_data, GwmhDesk *desk,
	       GwmhDeskInfoMask change_mask)
{
	Tasklist *tasklist = (Tasklist *) func_data;
	
	if (tasklist->config.all_desks_minimized && 
	    tasklist->config.all_desks_normal)
		return TRUE;

	tasklist_redo_vtasks (tasklist);
	tasklist_layout_tasklist (tasklist);

	return TRUE;
}

static void
tasklist_group_destroy (TasklistTask *group)
{
	d(g_print (" *** destroying group: %s\n", group->group_name));

	tasklist_icon_destroy (group);

	g_hash_table_remove (group->tasklist->groups, group->group_name);

	g_free (group->gwmh_task);
	group->gwmh_task = NULL;
	g_free (group->group_name);
	group->group_name = NULL;

	tasklist_clean_menu (group);

	g_free (group);
}

static void
tasklist_task_destroy (GwmhTask *gtask, Tasklist *tasklist)
{
	TasklistTask *ttask;
	
	g_return_if_fail (gtask != NULL);
	g_return_if_fail (tasklist != NULL);	
	
	ttask = g_hash_table_lookup (tasklist->tasks, gtask);
	
	if (!ttask) {
		g_warning ("Task not found in tasklist: %p; not destroying", gtask);
		return;
	}

	d(g_print (" *** removing: %p (%s)\n", ttask, gtask->name));
	
	ttask->destroyed = TRUE;

	g_hash_table_remove (tasklist->tasks, gtask);
	
	if (ttask == tasklist->motion_task)
		tasklist->motion_task = NULL;

	if (ttask->menuitem) {
		gtk_object_set_data (GTK_OBJECT (ttask->menuitem),
				     "task", NULL);
		gtk_widget_hide (ttask->menuitem);
		ttask->menuitem = NULL;
	}

	tasklist_icon_destroy (ttask);
	tasklist_clean_menu (ttask);

	if (ttask->group) {
		if (ttask->group->focused_task == ttask)
			ttask->group->focused_task = NULL;

		ttask->group->tasks = g_slist_remove (ttask->group->tasks, ttask);

		if (!ttask->group->tasks)
			tasklist_group_destroy (ttask->group);
		else
			fixup_vtask (ttask->group, NULL);
	}

	tasklist->vtasks = g_slist_remove (tasklist->vtasks, ttask);

	g_free (ttask);

	tasklist_layout_tasklist (tasklist);
}

static TasklistTask *
tasklist_group_new (TasklistTask *first_task, const char *group_name)
{
	TasklistTask *group;

	g_return_val_if_fail (first_task != NULL, NULL);
	g_return_val_if_fail (group_name != NULL, NULL);

	group = g_new0 (TasklistTask, 1);
	group->tasklist = first_task->tasklist;
	group->task_group = TRUE;
	group->group_name = g_strdup (group_name);
	group->fullwidth = -1;
	g_hash_table_insert (group->tasklist->groups,
			     group->group_name, group);

	gdk_pixbuf_ref (first_task->icon->normal);
	gdk_pixbuf_ref (first_task->icon->minimized);

	group->icon = g_new (TasklistIcon, 1);
	group->icon->normal    = first_task->icon->normal;
	group->icon->minimized = first_task->icon->minimized;

	group->tasks = g_slist_prepend (group->tasks, first_task);

	group->gwmh_task = g_new0 (GwmhTask, 1);
	group->gwmh_task->name = group->group_name;

	return group;
}

/* 
 * this is void since we don't need to get the return value when a
 * task is created and we can just create a lot of tasks from the gwmh
 * task glist
 */

static void
tasklist_task_new (GwmhTask *gtask, Tasklist *tasklist)
{
	TasklistTask *ttask;
	char *class;

	g_return_if_fail (gtask != NULL);
	g_return_if_fail (tasklist != NULL);
	g_return_if_fail (tasklist->tasks != NULL);

	d(g_print ("Adding task: %s\n", gtask->name));

	ttask = g_new0 (TasklistTask, 1);

	ttask->tasklist = tasklist;
	ttask->gwmh_task = gtask;

	g_hash_table_insert (tasklist->tasks, gtask, ttask);

	ttask->wmhints_icon = tasklist_icon_get_pixmap (ttask);

	tasklist_icon_set (ttask);
	ttask->fullwidth = -1;

	class = get_task_class (gtask);
	if (class == NULL)
		return;

	ttask->group = g_hash_table_lookup (tasklist->groups, class);

	if (ttask->group == NULL) {
		ttask->group = tasklist_group_new (ttask, class);
	} else {
		ttask->group->tasks = g_slist_prepend (ttask->group->tasks, ttask);
		fixup_vtask (ttask->group, NULL);
	}

	g_free (class);
}

static void
tasklist_task_new_iterator (gpointer data, gpointer user_data)
{
	GwmhTask *gtask = data;
	Tasklist *tasklist = user_data;

	tasklist_task_new (gtask, tasklist);
}


/* This routine gets called when tasks are created/destroyed etc */
static gboolean
task_notifier (gpointer func_data, GwmhTask *gwmh_task,
	       GwmhTaskNotifyType ntype,
	       GwmhTaskInfoMask imask)
{
	Tasklist *tasklist = (Tasklist *) func_data;
	TasklistTask *task;
	gboolean redraw = FALSE;
	gboolean relayout = FALSE;

	switch (ntype)
	{
	case GWMH_NOTIFY_INFO_CHANGED:
		/* if this is just an allocation change, then no we don't want
		 * to do anything */
		if ( ! (imask & ~GWMH_TASK_INFO_ALLOCATION))
			break;

		task = g_hash_table_lookup (tasklist->tasks, gwmh_task);
		if (!task) {
			g_warning ("Getting info about task we don't know about: %p", gwmh_task);
			break;
		}

		if ((imask & GWMH_TASK_INFO_FOCUSED) &&
		    task->gwmh_task->focused && task->group)
			task->group->focused_task = task;


		if (imask & GWMH_TASK_INFO_MISC) {
			if (tasklist->config.vert_width_full && 
			    (tasklist->orient == ORIENT_LEFT ||
			     tasklist->orient == ORIENT_RIGHT))
				relayout = TRUE;
			task->fullwidth = -1;
		}

		/* we only need to re-layout if the task has changed
		 * visibility status.  If it has, its group will also get
		 * fixed up
		 */
		if (fixup_vtask (task, &redraw)) {
			relayout = TRUE;
		}


		if (imask & GWMH_TASK_INFO_WM_HINTS) {
			if (tasklist_icon_get_pixmap (task) !=
			    task->wmhints_icon) {
				tasklist_icon_destroy (task);
				tasklist_icon_set (task);
				redraw = TRUE;
			}
		}
		if (imask & GWMH_TASK_INFO_GSTATE)
			relayout = TRUE;
		if (imask & GWMH_TASK_INFO_ICONIFIED)
			relayout = TRUE;

		if (imask & GWMH_TASK_INFO_FOCUSED)
			redraw = TRUE;
		if (imask & GWMH_TASK_INFO_MISC)
			redraw = TRUE;
		if (imask & GWMH_TASK_INFO_DESKTOP)
			relayout = TRUE;

		/* BTW, change_size is always called from the layout_tasklist
		 * since we pass TRUE */
		if (relayout)
			tasklist_layout_tasklist (tasklist);
		else if (redraw)
			tasklist_draw_task (task->group && is_task_really_visible (task->group)
					    ? task->group : task, NULL);

		break;
	case GWMH_NOTIFY_NEW:
		tasklist_task_new (gwmh_task, tasklist);
		tasklist_layout_tasklist (tasklist);
		break;
	case GWMH_NOTIFY_DESTROY:
		tasklist_task_destroy (gwmh_task, tasklist);
		break;
	default:
		d(g_print ("Unknown ntype: %d\n", ntype));
	}

	return TRUE;
}

/* Show the task if need. Return TRUE if so */
static gboolean
show_task (Tasklist *tasklist, TasklistTask *task)
{
	if (!GWMH_TASK_ICONIFIED (task->gwmh_task) && GWMH_TASK_FOCUSED (task->gwmh_task))
		return FALSE;

	
	if (!(tasklist->config.move_to_current && GWMH_TASK_ICONIFIED (task->gwmh_task))) {
		GwmhDesk *desk_info;
		desk_info = gwmh_desk_get_config ();
		
		if (task->gwmh_task->desktop != desk_info->current_desktop ||
		    task->gwmh_task->harea != desk_info->current_harea ||
		    task->gwmh_task->varea != desk_info->current_varea) {
			gwmh_desk_set_current_area (task->gwmh_task->desktop,
						    task->gwmh_task->harea,
						    task->gwmh_task->varea);
		}
	}
	
	gwmh_task_show (task->gwmh_task);
#if 0
	/* Why is a focus needed here?
	   gwmh_task_show is supposed to give focus */
	
	/*
	 * i think this is a sawfish bug: when "give
	 * uniconised windows focused" is unchecked it
	 * probably doesn't reassign focus. -- jacob
	 */
	
	gwmh_task_focus (task->gwmh_task);
	gwmh_task_focus (task->gwmh_task);
	
#endif 
	return TRUE; 
}

/* This routine gets called when the mouse is pressed */
static gboolean
cb_button_press_event (GtkWidget *widget, GdkEventButton *event, Tasklist *tasklist)
{
	TasklistTask *task;
	
	task = task_get_xy (tasklist, (gint)event->x, (gint)event->y);

	if (!task)
		return FALSE;

	if (event->button == 1) {
		if (task->task_group)
			tasklist_group_popup (task, event->button, event->time);
		else if (! show_task (tasklist, task))
			gwmh_task_iconify (task->gwmh_task);
		
		return TRUE;
	}
	
	if (event->button == 3) {
		gtk_signal_emit_stop_by_name (GTK_OBJECT (widget),
					      "button_press_event");
		tasklist_menu_popup (task, event->button, event->time);
		return TRUE;
	}

	return FALSE;
}

static void
cb_drag_leave (GtkWidget *widget, GdkDragContext *context, guint time, Tasklist *tasklist)
{
	if (tasklist->motion_timeout) {
		gtk_timeout_remove (tasklist->motion_timeout);
		tasklist->motion_timeout = 0;
		tasklist->motion_task = NULL;
	}
}

static gboolean 
cb_motion_timeout (gpointer user_data)
{
	Tasklist *tasklist = user_data;
	
	if (tasklist->motion_task && !(tasklist->motion_task->task_group)) {
		show_task (tasklist, tasklist->motion_task);
	}

	tasklist->motion_timeout = 0;
	tasklist->motion_task = NULL;
	
	return FALSE;	
}

/* This routine gets called when user drag something over the tasklist */
static gboolean 
cb_drag_motion (GtkWidget *widget, GdkDragContext *context, int x, int y, guint time, Tasklist *tasklist)
{
	TasklistTask *task;
	
	gdk_drag_status (context, 0, time);
	
	task = task_get_xy (tasklist, x, y);
	
	if (task != tasklist->motion_task) {
	
		if (tasklist->motion_timeout) {
			gtk_timeout_remove (tasklist->motion_timeout);
		}
	
		tasklist->motion_task = task;

		if (task) {	
			tasklist->motion_timeout = gtk_timeout_add (MOTION_TIMEOUT, cb_motion_timeout, tasklist);		
		} else {
			tasklist->motion_timeout = 0;
		}
	}
	
	return TRUE;
}

/* FIXME: This routine is one of the ugliest ones in existence,
 * gtk purists should not look at it.  This should get rewritten such
 * that each label has a widget with an inputonly window OR draw the
 * tooltips ourselves.  However for the time being this is better then
 * nothing I suppose.  Feel free to flame me for this.  Tooltips have
 * been one of the more requested features for a LONG time.
 *
 * -George
 */
static gboolean
cb_area_event (GtkWidget *widget, GdkEvent *event, Tasklist *tasklist)
{
	GSList *temp_tasks, *temp;
	TasklistTask *task;

	if ((event->type == GDK_LEAVE_NOTIFY ||
	     event->type == GDK_ENTER_NOTIFY) &&
	    event->crossing.detail == GDK_NOTIFY_INFERIOR)
		return FALSE;

	if (event->type == GDK_LEAVE_NOTIFY) {
		gtk_tooltips_set_tip (tasklist->tooltips,
				      tasklist->fake_tooltip_widget,
				      NULL, NULL);
		tasklist->tooltip_task = NULL;
		return FALSE;
	}

	if (event->type == GDK_MOTION_NOTIFY ||
	    event->type == GDK_ENTER_NOTIFY) {
		int x, y;

		if (event->type == GDK_MOTION_NOTIFY) {
			x = event->motion.x;
			y = event->motion.y;
		} else {
			x = event->crossing.x;
			y = event->crossing.y;
		}

		temp_tasks = tasklist->vtasks;
		for (temp = temp_tasks; temp != NULL; temp = temp->next) {
			task = (TasklistTask *)temp->data;

			if (!is_task_really_visible (task))
				continue;

			if (x >= task->x && 
			    x <= task->x + task->width &&
			    y >= task->y && 
			    y <= task->y + task->height) {
				/* This is it, and this is
				 * also the utterly evil part */
				if (tasklist->tooltip_task != task) {
					char *label;
					gboolean ignore;
					gpointer old_data;
					GdkEvent new_event = { 0 };
					GtkWidget *fake = tasklist->fake_tooltip_widget;

					tasklist->tooltip_task = task;

					fake->allocation.x = task->x;
					fake->allocation.y = task->y;
					fake->allocation.width = task->width;
					fake->allocation.height = task->height;
					fake->window = tasklist->area->window;

					label = tasklist_task_get_label
						(task, 0, task->task_group);

					gtk_tooltips_set_tip
						(tasklist->tooltips,
						 fake, label, NULL);

					g_free (label);

					if (event->type == GDK_ENTER_NOTIFY) {
						new_event = *event;
					} else {
						new_event.type = GDK_ENTER_NOTIFY;
						new_event.any.window = fake->window;
					}

					/* EEEEEEEEEEEEEVIL, this is the only
					 * way to make gtk draw the tooltips
					 * short of drawing them ourselves.
					 * The tooltips unfortunately do a whole
					 * bunch of internal checking so we abuse
					 * the area window.  This way it will
					 * appear almost as if the window belongs
					 * to the fake rather then to the area,
					 * at least for the duration of this call. */
					old_data = fake->window->user_data;
					fake->window->user_data = fake;
					gtk_signal_emit_by_name (GTK_OBJECT (fake),
								 "event",
								 &new_event,
								 &ignore);
					fake->window->user_data = old_data;
				}
				break;
			}
		}
		/* No task found, so we want to turn off the tooltip */
		if (temp == NULL) {
			gtk_tooltips_set_tip (tasklist->tooltips,
					      tasklist->fake_tooltip_widget,
					      NULL, NULL);
			tasklist->tooltip_task = NULL;
			return FALSE;
		}
	}

	return FALSE;
}

/* This routine gets called when the tasklist is exposed */
static gboolean
cb_expose_event (GtkWidget *widget, GdkEventExpose *event, Tasklist *tasklist)
{
	GSList *temp_tasks, *temp;
	TasklistTask *task;

	temp_tasks = tasklist->vtasks;

	gtk_paint_box (tasklist->area->style, tasklist->area->window,
		       tasklist->area->state, 
		       tasklist->config.sunken?GTK_SHADOW_IN:GTK_SHADOW_NONE,
		       &event->area, tasklist->area, "button",
		       0, 0, -1, -1);
	
	for (temp = temp_tasks; temp != NULL; temp = temp->next) {
		GdkRectangle rect, dest;
		task = (TasklistTask *)temp->data;

		rect.x = task->x;
		rect.y = task->y;
		rect.width = task->width;
		rect.height = task->height;

		if(gdk_rectangle_intersect(&event->area, &rect, &dest))
			tasklist_draw_task (task, &dest);
	}

	return FALSE;
}

/* This routine gets called when the user selects "properties" */
static void
cb_properties (AppletWidget *applet, Tasklist *tasklist)
{
	tasklist_display_properties (tasklist);
}

/* This routine gets called when the user selects "about" */
static void
cb_about (AppletWidget *applet, Tasklist *tasklist)
{

	const char *authors[] = {
		"Anders Carlsson (andersca@gnu.org)",
		"Miguel de Icaza (miguel@ximian.com)",
		"Jacob Berkman (jacob@ximian.com)",
		"George Lebl (jirka@5z.com)",
		NULL
	};

	/* Stop the about box from being shown twice at once */
	if (tasklist->about_dialog != NULL)
	{
		gtk_widget_show_now (tasklist->about_dialog);
		gdk_window_raise (tasklist->about_dialog->window);
		return;
	}
	
	tasklist->about_dialog = gnome_about_new (
		"Gnome Tasklist",
		VERSION,
		_("Copyright (C) 1999 Anders Carlsson"),
		authors,
		_("A tasklist for the GNOME desktop environment.\nIcons by Tuomas Kuosmanen (tigert@gimp.org)."),
		NULL);
	gtk_signal_connect (GTK_OBJECT(tasklist->about_dialog), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &tasklist->about_dialog);

	gtk_widget_show_now (tasklist->about_dialog);

	if (tasklist->about_dialog->window != NULL)
		gdk_window_raise (tasklist->about_dialog->window);
}

/* Ignore mouse button 1 clicks */
static gboolean
ignore_1st_click (GtkWidget *widget, GdkEvent *event, Tasklist *tasklist)
{
	GdkEventButton *buttonevent = (GdkEventButton *)event;

	if (event->type == GDK_BUTTON_PRESS &&
	    buttonevent->button == 1) {
		if (buttonevent->window != tasklist->area->window)
			buttonevent->button = 2;
	}
	if (event->type == GDK_BUTTON_RELEASE &&
	    buttonevent->button == 1) {
		if (buttonevent->window != tasklist->area->window)
			buttonevent->button = 2;
	}
	 
	return FALSE;
}

/* Changes size of the applet */
void
tasklist_change_size (Tasklist *tasklist, gboolean layout, int fullwidth)
{
	int handle_width = 0;
	int handle_height = 0;
	int width = 0;
	int height = 0;


	switch (applet_widget_get_panel_orient (APPLET_WIDGET (tasklist->applet))) {
	case ORIENT_UP:
	case ORIENT_DOWN:
		width = tasklist->horz_width;
		handle_width = tasklist->horz_width + DRAG_HANDLE_SIZE;
		height = handle_height = get_horz_rows (tasklist) * ROW_HEIGHT;

		GTK_HANDLE_BOX (tasklist->handle)->handle_position = GTK_POS_LEFT;
		break;
	case ORIENT_LEFT:
	case ORIENT_RIGHT:
		width = tasklist->config.follow_panel_size
			? tasklist->panel_size 
			: tasklist->config.vert_width;

		if (tasklist->config.vert_width_full) {
			if (fullwidth < 0)
				fullwidth = max_width (tasklist->vtasks);

			width = MAX (width, fullwidth);
		}

		handle_width = width;
		handle_height = tasklist->vert_height + DRAG_HANDLE_SIZE;
		height = tasklist->vert_height;
			
		GTK_HANDLE_BOX (tasklist->handle)->handle_position = GTK_POS_TOP;
	}

	gtk_widget_set_usize (tasklist->handle, handle_width, handle_height);
	gtk_drawing_area_size (GTK_DRAWING_AREA (tasklist->area), width, height);
	
	if (layout)
		tasklist_layout_tasklist (tasklist);
}

/* Called when the panel's orient changes */
static void
cb_change_orient (GtkWidget *widget, GNOME_Panel_OrientType orient, Tasklist *tasklist)
{
	tasklist->orient = orient;

	/* Change size accordingly */
	tasklist_change_size (tasklist, TRUE, -1);
}

static void
cb_change_pixel_size (GtkWidget *widget, int size, Tasklist *tasklist)
{
	tasklist->panel_size = size;
	
	/* Change size accordingly */
	if(tasklist->config.follow_panel_size)
		tasklist_change_size (tasklist, TRUE, -1);
}

static void
cb_help (AppletWidget *w, Tasklist *tasklist)
{
	GnomeHelpMenuEntry help_entry = { "tasklist_applet",
					  "index.html" };
	gnome_help_display(NULL, &help_entry);
}

static void
tasklist_destroy (GtkObject *applet_widget, Tasklist *tasklist)
{
	gwmh_task_notifier_remove (tasklist->task_notifier_id);
	gwmh_desk_notifier_remove (tasklist->desk_notifier_id);

	tasklist->task_notifier_id = -1;
	tasklist->desk_notifier_id = -1;

	if (tasklist->layout_idle != 0) {
		gtk_idle_remove (tasklist->layout_idle);
		tasklist->layout_idle = 0;
	}

	if (tasklist->motion_timeout) {
		gtk_timeout_remove (tasklist->motion_timeout);
		tasklist->motion_timeout = 0;
		tasklist->motion_task = NULL;
	}

	if (tasklist->prop)
		gtk_widget_destroy (tasklist->prop);
	tasklist->prop = NULL;

	if (tasklist->about_dialog)
		gtk_widget_destroy (tasklist->about_dialog);
	tasklist->about_dialog = NULL;

	if (tasklist->unknown_icon != NULL) {
		if (tasklist->unknown_icon->normal != NULL) {
			gdk_pixbuf_unref (tasklist->unknown_icon->normal);
			tasklist->unknown_icon->normal = NULL;
		}
		if (tasklist->unknown_icon->minimized != NULL) {
			gdk_pixbuf_unref (tasklist->unknown_icon->minimized);
			tasklist->unknown_icon->minimized = NULL;
		}
		g_free (tasklist->unknown_icon);
		tasklist->unknown_icon = NULL;
	}

	g_slist_free (tasklist->vtasks);
	tasklist->vtasks = NULL;

#ifndef APPLET_COMPILE_AS_PROCESS
        #warning Here we save peoples memory by making this a shlib applet and leaking whatever we possibly can!
#endif

	if (tasklist->fake_tooltip_widget != NULL) {
		tasklist->fake_tooltip_widget->window = NULL;
		gtk_widget_unref (tasklist->fake_tooltip_widget);
		tasklist->fake_tooltip_widget = NULL;
	}

	if (tasklist->tooltips != NULL) {
		gtk_object_destroy (GTK_OBJECT (tasklist->tooltips));
		tasklist->tooltips = NULL;
	}

	if (tasklist->groups != NULL) {
		/* FIXME: we leak EVERYTHING here */
		g_hash_table_destroy (tasklist->groups);
		tasklist->groups = NULL;
	}
	if (tasklist->tasks != NULL) {
		/* FIXME: we leak EVERYTHING here */
		g_hash_table_destroy (tasklist->tasks);
		tasklist->tasks = NULL;
	}

	g_free (tasklist);
}

/* Create the applet */
static Tasklist *
tasklist_new (void)
{
	Tasklist *tasklist;
	GtkWidget *hbox;

	tasklist = g_new0 (Tasklist, 1);
	tasklist->panel_size = 48;
	tasklist->horz_width = 0;
	tasklist->vert_height = 0;

	tasklist->applet = applet_widget_new ("tasklist_applet");
	if (!tasklist->applet) {
		g_warning (_("Tasklist: Unable to create applet widget"));
		g_free (tasklist);
		return NULL;
	}

	/* Some evil tooltip stuff, this is some pretty evil stuff */
	tasklist->tooltips = gtk_tooltips_new ();
	tasklist->fake_tooltip_widget = gtk_type_new (gtk_widget_get_type ());
	GTK_WIDGET_SET_FLAGS (tasklist->fake_tooltip_widget, GTK_NO_WINDOW);
	GTK_WIDGET_SET_FLAGS (tasklist->fake_tooltip_widget, GTK_VISIBLE);
	GTK_WIDGET_SET_FLAGS (tasklist->fake_tooltip_widget, GTK_MAPPED);
	gtk_widget_ref (tasklist->fake_tooltip_widget);
	gtk_object_sink (GTK_OBJECT (tasklist->fake_tooltip_widget));
	tasklist->tooltip_task = NULL;

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	
	tasklist->handle = gtk_handle_box_new ();
	gtk_signal_connect (GTK_OBJECT (tasklist->handle), "event",
			    GTK_SIGNAL_FUNC (ignore_1st_click), tasklist);

	tasklist->area = gtk_drawing_area_new ();

	gtk_widget_ensure_style (tasklist->area);
	tasklist->unknown_icon = g_new (TasklistIcon, 1);
	tasklist->unknown_icon->normal = gdk_pixbuf_new_from_xpm_data (unknown_xpm);
	tasklist->unknown_icon->minimized = tasklist_icon_create_minimized_icon (tasklist, tasklist->unknown_icon->normal);

#if 0
	gtk_container_add (GTK_CONTAINER (tasklist->handle), tasklist->area);
#endif

	/* we must bind signals BEFORE applet_widget_add to avoid
	 * a race */
	gtk_signal_connect (GTK_OBJECT (tasklist->applet), "change-orient",
			    GTK_SIGNAL_FUNC (cb_change_orient), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->applet), "save-session",
			    GTK_SIGNAL_FUNC (tasklist_write_config), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->applet), "change-pixel-size",
			    GTK_SIGNAL_FUNC (cb_change_pixel_size), tasklist);

	gtk_widget_set_events (tasklist->area, GDK_EXPOSURE_MASK | 
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK |
			       GDK_ENTER_NOTIFY_MASK |
			       GDK_LEAVE_NOTIFY_MASK |
			       GDK_POINTER_MOTION_MASK);
	gtk_signal_connect (GTK_OBJECT (tasklist->area), "event",
			    GTK_SIGNAL_FUNC (cb_area_event), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->area), "expose_event",
			    GTK_SIGNAL_FUNC (cb_expose_event), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->area), "button_press_event",
			    GTK_SIGNAL_FUNC (cb_button_press_event), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->area), "drag_motion",
			    GTK_SIGNAL_FUNC (cb_drag_motion), tasklist);
	gtk_signal_connect (GTK_OBJECT (tasklist->area), "drag_leave",
			    GTK_SIGNAL_FUNC (cb_drag_leave), tasklist);			    
			    			    
	gtk_drag_dest_set (GTK_WIDGET (tasklist->area), 0,
	 		   NULL, 0, GDK_ACTION_COPY);

	/*
	 * we add the area *after* the widget so that we get events on it
	 */
	gtk_container_add (GTK_CONTAINER (hbox), tasklist->handle);
	applet_widget_add (APPLET_WIDGET (tasklist->applet), hbox);
	gtk_container_add (GTK_CONTAINER (tasklist->handle), tasklist->area);
	
	tasklist_read_config (tasklist);

	if ( ! tasklist->config.enable_tooltips)
		gtk_tooltips_disable (tasklist->tooltips);
	
	applet_widget_register_stock_callback (
		APPLET_WIDGET (tasklist->applet),
		"properties",
		GNOME_STOCK_MENU_PROP,
		_("Properties..."),
		(AppletCallbackFunc) cb_properties,
		tasklist);

	applet_widget_register_stock_callback (
		APPLET_WIDGET (tasklist->applet),
		"help",
		GNOME_STOCK_PIXMAP_HELP,
		_("Help"),
		(AppletCallbackFunc) cb_help,
		tasklist);

	applet_widget_register_stock_callback (
		APPLET_WIDGET (tasklist->applet),
		"about",
		GNOME_STOCK_MENU_ABOUT,
		_("About..."),
		(AppletCallbackFunc) cb_about,
		tasklist);

	tasklist->panel_size = applet_widget_get_panel_pixel_size(APPLET_WIDGET(tasklist->applet));
	tasklist->orient = applet_widget_get_panel_orient(APPLET_WIDGET(tasklist->applet));

	tasklist->task_notifier_id = gwmh_task_notifier_add (task_notifier, tasklist);
	tasklist->desk_notifier_id = gwmh_desk_notifier_add (desk_notifier, tasklist);

	gtk_signal_connect (GTK_OBJECT (tasklist->applet), "destroy",
			    GTK_SIGNAL_FUNC (tasklist_destroy), tasklist);
	
	gtk_widget_show_all (tasklist->area);
	gtk_widget_show_all (tasklist->handle);

	tasklist_change_size (tasklist, TRUE, -1);

	tasklist->tasks = g_hash_table_new (g_direct_hash, g_direct_equal);
	tasklist->groups = g_hash_table_new (g_str_hash, g_str_equal);

	g_list_foreach (gwmh_task_list_get (),
			tasklist_task_new_iterator,
			tasklist);

	return tasklist;
}

static void
tasklist_init (void)
{
	gwmh_init ();
}

#ifdef APPLET_COMPILE_AS_PROCESS
gint
main (gint argc, gchar *argv[])
{
	Tasklist *tasklist;
	
	/* Initialize i18n */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("tasklist_applet",
			    VERSION,
			    argc, argv,
			    NULL, 0, NULL);

	gdk_rgb_init ();

	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-tasklist.png");

	gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
	gtk_widget_set_default_visual (gdk_rgb_get_visual ());

	tasklist_init ();
	
	tasklist = tasklist_new ();

	gtk_widget_show_all (tasklist->applet);

	applet_widget_gtk_main ();

	return 0;
}
#else
static GtkWidget *
make_new_applet (const char *goad_id)
{
	static int inited = 0;
	Tasklist *tasklist;
	
	if (!inited){
		tasklist_init ();
		inited = 1;
	}
	tasklist = tasklist_new ();

	if (!tasklist)
		return NULL;

	gtk_widget_show_all (tasklist->applet);

	return tasklist->applet;
}

static CORBA_Object
activator (PortableServer_POA poa,
	   const char *goad_id,
	   const char **params,
	   gpointer *impl_ptr,
	   CORBA_Environment *ev)
{
	GtkWidget *widget;

	widget = make_new_applet (goad_id);
	if (widget == NULL) {
		g_warning (_("Don't know how to activate `%s'\n"), goad_id);
		return CORBA_OBJECT_NIL;
	}

	return applet_widget_corba_activate (widget, poa, goad_id,
					     params, impl_ptr, ev);
}
static void
deactivator (PortableServer_POA poa,
	     const char *goad_id,
	     gpointer impl_ptr,
	     CORBA_Environment *ev)
{
	applet_widget_corba_deactivate (poa, goad_id, impl_ptr, ev);
}

static const char *repo_id[]={ "IDL:GNOME/Applet:1.0", NULL };
static GnomePluginObject applets_list[] = { 
	{ repo_id, "tasklist_applet", NULL, "Task list applet",
	  &activator, &deactivator },
	{ NULL }
};

GnomePlugin GNOME_Plugin_info = { 
	applets_list,
	NULL
};

#endif
