#include "tasklist_applet.h"

gboolean
tasklist_write_config (GtkWidget *w, const gchar *privcfgpath, const gchar *globcfgpath, gpointer data)
{
	Tasklist *tasklist = (Tasklist *) data;

	gnome_config_push_prefix (privcfgpath 
				  ? privcfgpath
				  : APPLET_WIDGET (tasklist->applet)->privcfgpath);

	gnome_config_set_bool ("tasklist/follow_panel_size",   tasklist->config.follow_panel_size);
	gnome_config_set_bool ("tasklist/horz_fixed",          tasklist->config.horz_fixed);
	gnome_config_set_bool ("tasklist/horz_never_push",     tasklist->config.horz_never_push);
	gnome_config_set_int  ("tasklist/horz_width",          tasklist->config.horz_width);
	gnome_config_set_int  ("tasklist/horz_rows",           tasklist->config.horz_rows);
	gnome_config_set_int  ("tasklist/horz_taskwidth",      tasklist->config.horz_taskwidth);
	gnome_config_set_bool ("tasklist/vert_fixed",          tasklist->config.vert_fixed);
	gnome_config_set_int  ("tasklist/vert_height",         tasklist->config.vert_height);
	gnome_config_set_bool ("tasklist/vert_never_push",     tasklist->config.vert_never_push);
	gnome_config_set_int  ("tasklist/vert_width",          tasklist->config.vert_width);
	gnome_config_set_int  ("tasklist/vert_width_full",     tasklist->config.vert_width_full);       
	gnome_config_set_bool ("tasklist/show_mini_icons",     tasklist->config.show_mini_icons);
	gnome_config_set_bool ("tasklist/show_normal",         tasklist->config.show_normal);
	gnome_config_set_bool ("tasklist/show_minimized",      tasklist->config.show_minimized);
	gnome_config_set_bool ("tasklist/all_desks_normal",    tasklist->config.all_desks_normal);
	gnome_config_set_bool ("tasklist/all_desks_minimized", tasklist->config.all_desks_minimized);
	gnome_config_set_bool ("tasklist/confirm_before_kill", tasklist->config.confirm_before_kill);
	gnome_config_set_bool ("tasklist/move_to_current",     tasklist->config.move_to_current);
	gnome_config_set_bool ("tasklist/enable_grouping",     tasklist->config.enable_grouping);
	gnome_config_set_int  ("tasklist/grouping_min",        tasklist->config.grouping_min);
	gnome_config_set_bool ("tasklist/enable_tooltips",     tasklist->config.enable_tooltips);
	gnome_config_set_bool ("tasklist/sunken",              tasklist->config.sunken);
			       
	gnome_config_sync ();
	
	gnome_config_pop_prefix ();
	return FALSE;
}

void
tasklist_read_config (Tasklist *tasklist)
{
	gnome_config_push_prefix (APPLET_WIDGET (tasklist->applet)->privcfgpath);

	tasklist->config.follow_panel_size = gnome_config_get_bool ("tasklist/follow_panel_size=true");

	tasklist->config.horz_fixed = gnome_config_get_bool ("tasklist/horz_fixed=true");
	/* if the screen is not too wide, make it default to 300 */
	if (gdk_screen_width () <= 800)
		tasklist->config.horz_width = gnome_config_get_int ("tasklist/horz_width=300");
	else
		tasklist->config.horz_width = gnome_config_get_int ("tasklist/horz_width=450");
	tasklist->config.horz_never_push = gnome_config_get_bool ("tasklist/horz_never_push=false");
	tasklist->config.horz_rows = gnome_config_get_int ("tasklist/horz_rows=2");
	tasklist->config.horz_taskwidth = gnome_config_get_int ("tasklist/horz_taskwidth=150");
	tasklist->config.vert_fixed = gnome_config_get_bool ("tasklist/vert_fixed=true");
	tasklist->config.vert_never_push = gnome_config_get_bool ("tasklist/vert_never_push=false");
	tasklist->config.vert_width = gnome_config_get_int ("tasklist/vert_width=48");
	tasklist->config.vert_width_full = gnome_config_get_bool ("tasklist/vert_width_full=false");
	tasklist->config.vert_height = gnome_config_get_int ("tasklist/vert_height=300");

	tasklist->config.confirm_before_kill = gnome_config_get_bool ("tasklist/confirm_before_kill=true");
	
	tasklist->config.show_mini_icons = gnome_config_get_bool ("tasklist/show_mini_icons=true");
	tasklist->config.show_normal = gnome_config_get_bool ("tasklist/show_normal=true");
	tasklist->config.show_minimized = gnome_config_get_bool ("tasklist/show_minimized=true");
	tasklist->config.all_desks_normal = gnome_config_get_bool ("tasklist/all_desks_normal=false");
	tasklist->config.all_desks_minimized = gnome_config_get_bool ("tasklist/all_desks_minimized=false");
	tasklist->config.move_to_current = gnome_config_get_bool ("tasklist/move_to_current=false");
	tasklist->config.sunken = gnome_config_get_bool ("tasklist/sunken=false");
	
	tasklist->config.enable_grouping = gnome_config_get_bool ("tasklist/enable_grouping=true");
	tasklist->config.grouping_min    = gnome_config_get_int  ("tasklist/grouping_min=3");
	tasklist->config.enable_tooltips = gnome_config_get_bool ("tasklist/enable_tooltips=true");

	gnome_config_pop_prefix ();
}


