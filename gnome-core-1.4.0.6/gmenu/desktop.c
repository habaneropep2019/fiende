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
 * Desktop_Data utils
 *-----------------------------------------------------------------------------
 */

Desktop_Data *desktop_data_new(const gchar *path, const gchar *name, const gchar *comment, GtkWidget *pixmap)
{
	Desktop_Data *dd;

	dd = g_new0 (Desktop_Data, 1);

	if (path != NULL) {
		dd->path = g_strdup(path);
		dd->editable = file_is_editable(dd->path);
		dd->isfolder = isdir(dd->path);
	}

	/* g_strdup Handles NULLs well */
	dd->name = g_strdup(name);
	dd->comment = g_strdup(comment);

	dd->pixmap = pixmap;
	if (pixmap != NULL)
		gtk_widget_ref (pixmap);

	dd->expanded = FALSE;

	return dd;
}

Desktop_Data *desktop_data_new_from_path(const gchar *path)
{
	Desktop_Data *dd;
	GnomeDesktopEntry *dentry = NULL;

	g_return_val_if_fail (path != NULL, NULL);

	dd = g_new0(Desktop_Data, 1);

	dd->path = g_strdup(path);

	if (isdir(path)) {
		gchar *dir_file = g_concat_dir_and_file(dd->path, ".directory");

		dd->isfolder = TRUE;

		if (isfile(dir_file)) {
			dd->editable = file_is_editable(dir_file);
			dentry = gnome_desktop_entry_load_unconditional(dir_file);
		} else {
			dd->editable = file_is_editable(dd->path);
		}

		g_free(dir_file);
	} else {
		dd->isfolder = FALSE;
		dd->editable = file_is_editable(dd->path);
		dentry = gnome_desktop_entry_load_unconditional(path);
	}

	if (dentry != NULL &&
	    dentry->name != NULL)
		dd->name = g_strdup(dentry->name);
	else
		dd->name = g_strdup (g_basename (dd->path));

	if (dentry != NULL &&
	    dentry->comment != NULL) {
		dd->comment = g_strdup(dentry->comment);
	} else {
		if (dd->isfolder)
			dd->comment = g_strconcat(dd->name , _(" Folder"), NULL);
		else
			dd->comment = g_strdup("");
	}

	if (dentry != NULL &&
	    dentry->icon != NULL)
		dd->pixmap = pixmap_load(dentry->icon);
	else
		dd->pixmap = pixmap_unknown();

	if (dentry != NULL)
		gnome_desktop_entry_free(dentry);

	return dd;
}

void desktop_data_free(Desktop_Data *dd)
{
	if (dd != NULL) {
		g_free(dd->path);
		dd->path = NULL;
		g_free(dd->name);
		dd->name = NULL;
		g_free(dd->comment);
		dd->comment = NULL;

		if (dd->pixmap != NULL) {
			gtk_widget_destroy (GTK_WIDGET(dd->pixmap));
			gtk_widget_unref (GTK_WIDGET(dd->pixmap));
			dd->pixmap = NULL;
		}

		g_free(dd);
	}
}


