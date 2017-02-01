/*
 * GNOME menu editor revision 2
 * (C)1999
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"

/*
 *-----------------------------------------------------------------------------
 * .desktop save functions
 *-----------------------------------------------------------------------------
 */

gboolean save_desktop_entry_file(GnomeDesktopEntry *dentry, const gchar *path,
			gboolean prompt_first, gboolean prompt_about_overwrite,
			gboolean error_on_overwrite_conflict)
{
	/*
	 * (1) Pop up some dialogs if necessary.
	 */
	if ((dentry->name == NULL) || (strlen(dentry->name) == 0))
		{
		gnome_warning_dialog(_("The menu item must have a name"));
		return FALSE;
		}

	if ((path == NULL) || (strlen(path) == 0))
		{
		gnome_warning_dialog(_("The menu entry must have a filename"));
		return FALSE;
		}

	if (prompt_first)
		{
		GtkWidget *question;
		question = gnome_dialog_new(_("Save changes?"),
				GNOME_STOCK_BUTTON_OK,
				GNOME_STOCK_BUTTON_CANCEL, NULL);
		if (gnome_dialog_run_and_close(GNOME_DIALOG(question)) == 1)
			return FALSE;
		}

	/*
	 * (2) Check to see if we're overwriting a file.
	 */
	if (isfile(path) || isdir(path))
		{
		if (error_on_overwrite_conflict)
			{
			gnome_warning_dialog(_("This change conflicts with an existing menu item"));
			return FALSE;
			}

		if (prompt_about_overwrite)
			{
			GtkWidget *question;
			question = gnome_dialog_new(_("Overwrite existing menu entry?"),
					GNOME_STOCK_BUTTON_OK,
					GNOME_STOCK_BUTTON_CANCEL, NULL);
			if (gnome_dialog_run_and_close(GNOME_DIALOG(question)) == 1)
				return FALSE;
			}
		}

	g_free (dentry->location);
	dentry->location = g_strdup(path);
	gnome_desktop_entry_save (dentry);

	return TRUE;
}


void save_desktop_entry(GnomeDesktopEntry *dentry, const gchar *old_path, gboolean isfolder)
{
	gchar *new_path;
	gchar *save_path;
	gchar *name;
	gchar *buf;

	if ((dentry->name == NULL) || (strlen(dentry->name) == 0))
		{
		gnome_warning_dialog(_("The menu item must have a name"));
		return;
		}

	name = validate_filename(dentry->name);
	buf = remove_level_from_path(old_path);
	if (isfolder)
		{
		if (strcmp(old_path, user_apps_dir) == 0 ||
		    strcmp(old_path, system_apps_dir) == 0 ||
		    (system_apps_merge_dir && strcmp(old_path, system_apps_merge_dir) == 0) ||
		    strcmp(old_path, system_applets_dir) == 0 )
			new_path = g_strdup(old_path);
		else
			new_path = g_strconcat(buf, "/", name, NULL);
		}
	else
		{
		new_path = g_strconcat(buf, "/", name, ".desktop", NULL);
		}
	g_free(buf);
	g_free(name);

	if (strcmp(old_path, new_path) != 0)
		{
		/* different name, rename old first */
		if (g_file_exists(new_path))
			{
			gnome_warning_dialog(_("This change conflicts with an existing menu item.\nNo two menu items in a submenu can have the same name."));
			g_free(new_path);
			return;
			}

		if (rename(old_path, new_path) < 0)
			g_warning("Unable to rename file %s to %s: %s\n",
				  old_path, new_path, g_strerror(errno));
		}

	if (isfolder)
		save_path = g_concat_dir_and_file(new_path, ".directory");
	else
		save_path = g_strdup(new_path);

	if (!save_desktop_entry_file(dentry, save_path, FALSE, FALSE, FALSE))
		{
		gnome_warning_dialog("Unable to write new menu data");
		g_free(save_path);
		g_free(new_path);
		return;
		}

	/* we made it here, we were successful, so notify tree of update */
	menu_tree_path_updated (tree, old_path, new_path, dentry);

	g_free (new_path);
	g_free (save_path);
}
