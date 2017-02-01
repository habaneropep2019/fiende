/* meat-grinder, maker of tarballs */

#include <config.h>
#include <gnome.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

static GtkWidget *app = NULL;
static GtkWidget *icon_list = NULL;
static GtkWidget *status_label = NULL;
static GtkWidget *compress_button = NULL;
static GHashTable *file_ht = NULL;
static int number_of_files = 0;
static int number_of_dirs = 0;
static char *file_icon = NULL;
static char *folder_icon = NULL;
static char *compress_icon = NULL;
static char *tar_prog = NULL;
static char *filename = NULL;
static pid_t temporary_pid = 0;
static char *temporary_file = NULL;

#define ERRDLG(error) gnome_dialog_run_and_close (GNOME_DIALOG (gnome_error_dialog_parented (error, GTK_WINDOW (app))))
#define ERRDLGP(error,parent) gnome_dialog_run_and_close (GNOME_DIALOG (gnome_error_dialog_parented (error, GTK_WINDOW (parent))))

static GtkTargetEntry drop_targets [] = {
  { "text/uri-list", 0, 0 }
};
static guint n_drop_targets = sizeof (drop_targets) / sizeof(drop_targets[0]);

static GtkTargetEntry drag_targets [] = {
  { "x-special/gnome-icon-list", 0, 0 },
  { "text/uri-list", 0, 0 }
};
static guint n_drag_targets = sizeof(drag_targets) / sizeof(drag_targets[0]);

enum {
	TYPE_FOLDER,
	TYPE_REGULAR
};


typedef struct _File File;
struct _File {
	int type;
	char *name;
	char *base_name;
};

static void
free_file (File *f)
{
	f->type = -1;
	g_free (f->name);
	f->name = NULL;
	g_free (f);
}

static char *
make_temp_dir (void)
{
	char *name = NULL;
	char *gname;
	do {
		if (name != NULL)
			free (name);
		name = tempnam (NULL, "gmg");
	} while (mkdir (name, 0755) < 0);

	gname = g_strdup (name);
	free (name);
	return gname;
}

static gboolean
query_dialog (const gchar *msg)
{
	int ret;
	GtkWidget *req;

	req = gnome_message_box_new (msg,
				     GNOME_MESSAGE_BOX_QUESTION,
				     GNOME_STOCK_BUTTON_YES,
				     GNOME_STOCK_BUTTON_NO,
				     NULL);

	gtk_window_set_modal (GTK_WINDOW (req), TRUE);

	ret = gnome_dialog_run (GNOME_DIALOG (req));

	if (ret == 0)
		return TRUE;
	else /* this includes -1 which is "destroyed" */
		return FALSE;
}

static void
update_status (void)
{
	if (number_of_files > 0 || number_of_dirs > 0) {
		char *msg;
		msg = g_strdup_printf (_("Number of files: %d\nNuber of folders: %d"),
				       number_of_files, number_of_dirs);
		gtk_label_set_text (GTK_LABEL (status_label), msg);
		g_free (msg);
	} else {
		gtk_label_set_text (GTK_LABEL (status_label), 
				    _("No files or folders, drop files or folders\n"
				      "above to add them to the archive"));
	}
}

/* removes icon, positions should not be trusted after this */
static void
remove_pos (int pos)
{
	File *f;

	f = gnome_icon_list_get_icon_data (GNOME_ICON_LIST (icon_list), pos);
	gnome_icon_list_remove (GNOME_ICON_LIST (icon_list), pos);
	if (f == NULL)
		return;

	if (f->type == TYPE_REGULAR)
		number_of_files --;
	else if (f->type == TYPE_FOLDER)
		number_of_dirs --;
	g_hash_table_remove (file_ht, f->base_name);
	free_file (f);

	if (number_of_files + number_of_dirs <= 0) {
		gtk_widget_set_sensitive (compress_button, FALSE);
	}

	update_status ();
}

struct argv_adder {
	char *link_dir;
	char **argv;
	int pos;
};

static void
make_argv_fe (gpointer key, gpointer value, gpointer user_data)
{
	File *f = value;
	struct argv_adder *argv_adder = user_data;
	char *file = g_concat_dir_and_file (argv_adder->link_dir, f->base_name);

	argv_adder->argv[argv_adder->pos ++] = f->base_name;

	/* FIXME: catch errors */
	symlink (f->name, file);
}

static void
whack_links_fe (gpointer key, gpointer value, gpointer user_data)
{
	File *f = value;
	char *link_dir = user_data;
	char *file = g_concat_dir_and_file (link_dir, f->base_name);

	unlink (file);
	g_free (file);
}

static void
setup_busy (GtkWidget *w, gboolean busy)
{
	GdkCursor *cursor;

	if (busy) {
		/* Change cursor to busy */
		cursor = gdk_cursor_new (GDK_WATCH);
		gdk_window_set_cursor (w->window, cursor);
		gdk_cursor_destroy (cursor);
	} else {
		gdk_window_set_cursor (w->window, NULL);
	}

	gdk_flush ();
}

static gboolean
create_archive (const char *fname,
		const char *dir,
		gboolean gui_errors)
{
	gboolean status = TRUE;
	pid_t pid;
	struct argv_adder argv_adder;

	if (dir == NULL)
		argv_adder.link_dir = make_temp_dir ();
	else
		argv_adder.link_dir = (char *)dir;

	argv_adder.argv = g_new (char *, number_of_files + number_of_dirs + 4);
	argv_adder.argv[number_of_files + number_of_dirs + 3] = NULL;
	argv_adder.argv[0] = tar_prog;
	argv_adder.argv[1] = "-chzf";
	argv_adder.argv[2] = (char *)fname;
	argv_adder.pos = 3;

	g_hash_table_foreach (file_ht, make_argv_fe, &argv_adder);

	/* FIXME: get error output, check for errors, etc... */

	pid = fork ();
	if (pid < 0) {
		if (gui_errors) {
			ERRDLG (_("Cannot start the archive (tar) program"));
		} else {
			fprintf (stderr,
				 _("Cannot start the archive (tar) program"));
		}
		status = FALSE;
	} else if (pid == 0) {
		umask (022);
		/* FIXME: handler errors */
		chdir (argv_adder.link_dir);
		execv (tar_prog, argv_adder.argv);
		fprintf (stderr, _("Cannot run tar/gtar"));
		_exit (1);
	}

	/* FIXME: Do some progress bar or what not rather then block */
	if (pid > 0)
		waitpid (pid, 0, 0);

	/* FIXME: handle errors */
	g_hash_table_foreach (file_ht, whack_links_fe, argv_adder.link_dir);

	if (dir == NULL) {
		/* FIXME: handle errors */
		rmdir (argv_adder.link_dir);
		g_free (argv_adder.link_dir);
	}

	g_free (argv_adder.argv);

	return status;
}

static void
start_temporary (void)
{
	char *dir;
	char *file = NULL;

	if (temporary_file != NULL) {
		if (access (temporary_file, F_OK) == 0)
			return;

		/* Note: nautilus is a wanker and will happily do a move when
		 * we explicitly told him that we just support copy, so in case
		 * this file is missing, we let nautilus have it and hope
		 * he chokes on it */

		dir = g_dirname (temporary_file);
		rmdir (dir);
		g_free (dir);

		g_free (temporary_file = NULL);
		temporary_file = NULL;

		/* just paranoia */
		if (temporary_pid > 0)
			kill (temporary_pid, SIGKILL);
	}

	/* make a temporary dirname */
	dir = make_temp_dir ();
	file = g_concat_dir_and_file (dir, _("Archive.tar.gz"));

	temporary_pid = fork ();

	if (temporary_pid == 0) {
		if ( ! create_archive (file, dir, FALSE /* gui_errors */)) {
			_exit (1);
		} else {
			_exit (0);
		}
	}

	/* can't fork? don't dispair, do synchroniously */
	if (temporary_pid < 0) {
		setup_busy (app, TRUE);
		if ( ! create_archive (file, dir, TRUE /* gui_errors */)) {
			setup_busy (app, FALSE);
			g_free (file);
			g_free (dir);
			temporary_pid = 0;
			return;
		}
		setup_busy (app, FALSE);
		temporary_pid = 0;
	}
	g_free (dir);

	temporary_file = file;
}

static gboolean
ensure_temporary (void)
{
	int status;

	start_temporary ();

	if (temporary_file == NULL)
		return FALSE;

	if (temporary_pid == 0)
		return TRUE;

	/* ok, gotta wait */
	waitpid (temporary_pid, &status, 0);

	temporary_pid = 0;

	if (WIFEXITED (status) &&
	    WEXITSTATUS (status) == 0) {
		return TRUE;
	} else {
		g_free (temporary_file);
		temporary_file = NULL;
		temporary_pid = 0;
		return FALSE;
	}
}

static void
cleanup_temporary (void)
{
	char *file = temporary_file;
	pid_t pid = temporary_pid;

	temporary_file = NULL;
	temporary_pid = 0;

	if (pid > 0) {
		if (kill (pid, SIGTERM) == 0)
			waitpid (pid, NULL, 0);
	}
	
	if (file != NULL) {
		char *dir;

		unlink (file);

		dir = g_dirname (file);
		rmdir (dir);
		g_free (dir);
	}

	g_free (file);
}

static void
about_cb (GtkWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	gchar *authors[] = {
		"George Lebl <jirka@5z.com>",
		NULL
	};

	if (about != NULL) {
		gtk_widget_show_now (about);
		gdk_window_raise (about->window);
		return;
	}
	about = gnome_about_new (_("The GNOME Archive Generator"), VERSION,
				 "(C) 2001 George Lebl",
				 (const char **)authors,
				 _("Drag files in to make archives"),
				 NULL);
	gtk_signal_connect (GTK_OBJECT (about), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &about);
	gtk_widget_show (about);
}

static void
quit_cb (GtkWidget *item)
{
	gtk_main_quit ();
}

static void
select_all_cb (GtkWidget *w, gpointer data)
{
	int i;

	for (i = 0; i < GNOME_ICON_LIST (icon_list)->icons; i++) {
		gnome_icon_list_select_icon (GNOME_ICON_LIST (icon_list), i);
	}
}

static gboolean
file_remove_fe (gpointer key, gpointer value, gpointer user_data)
{
	File *f = value;
	free_file (f);
	return TRUE;
}

static void
clear_cb (GtkWidget *w, gpointer data)
{
	g_hash_table_foreach_remove (file_ht, file_remove_fe, NULL);
	number_of_files = number_of_dirs = 0;
	gnome_icon_list_clear (GNOME_ICON_LIST (icon_list));
	gtk_widget_set_sensitive (compress_button, FALSE);
	update_status ();
}

static void
remove_cb (GtkWidget *w, gpointer data)
{
	while (GNOME_ICON_LIST (icon_list)->selection != NULL) {
		int pos = GPOINTER_TO_INT (GNOME_ICON_LIST (icon_list)->selection->data);
		remove_pos (pos);
	}
}

static void
save_ok (GtkWidget *widget, GtkFileSelection *fsel)
{
	char *fname;

	g_return_if_fail (GTK_IS_FILE_SELECTION(fsel));

	setup_busy (GTK_WIDGET (fsel), TRUE);

	fname = gtk_file_selection_get_filename (fsel);
	if (fname == NULL || fname[0] == '\0') {
		ERRDLGP (_("No filename selected"), fsel);
		setup_busy (GTK_WIDGET (fsel), FALSE);
		return;
	} else if (access (fname, F_OK) == 0 &&
		   ! query_dialog (_("File exists, overwrite?"))) {
		setup_busy (GTK_WIDGET (fsel), FALSE);
		return;
	} else if ( ! create_archive (fname,
				      NULL /*dir*/,
				      TRUE /* gui_errors */)) {
		/* the above should do error dialog itself */
		setup_busy (GTK_WIDGET (fsel), FALSE);
		return;
	}

	setup_busy (GTK_WIDGET (fsel), FALSE);

	g_free (filename);
	filename = g_strdup (fname);
	gtk_widget_destroy (GTK_WIDGET (fsel));
}

static void
archive_cb (GtkWidget *w, gpointer data)
{
	GtkFileSelection *fsel;

	if (number_of_files + number_of_dirs <= 0) {
		ERRDLG (_("No items to archive"));
		return;
	}

	fsel = GTK_FILE_SELECTION(gtk_file_selection_new(_("Save Results")));
	if (filename != NULL)
		gtk_file_selection_set_filename (fsel, filename);

	gtk_signal_connect (GTK_OBJECT (fsel->ok_button), "clicked",
			    GTK_SIGNAL_FUNC (save_ok), fsel);
	gtk_signal_connect_object
		(GTK_OBJECT (fsel->cancel_button), "clicked",
		 GTK_SIGNAL_FUNC (gtk_widget_destroy), 
		 GTK_OBJECT (fsel));

	gtk_window_position (GTK_WINDOW (fsel), GTK_WIN_POS_MOUSE);

	gtk_window_set_transient_for (GTK_WINDOW (fsel),
				      GTK_WINDOW (app));

	gtk_widget_show (GTK_WIDGET (fsel));
}

static void
add_file (const char *file)
{
	struct stat s;
	File *f, *oldf;
	int type;
	const char *icon;
	int pos;
	int i;
	const char *base_name;
	char *fullname;

	if (file == NULL ||
	    file[0] == '\0')
		return;

	fullname = g_strdup (file);

	while (strcmp (fullname, "/") != 0 &&
	       fullname[0] != '\0' &&
	       fullname[strlen(fullname)-1] == '/') {
		fullname[strlen(fullname)-1] = '\0';
	}

	if (strcmp (fullname, "/") != 0)
		base_name = g_basename (fullname);
	else
		base_name = "/";

	oldf = g_hash_table_lookup (file_ht, base_name);
	i = 1;
	while (oldf != NULL) {
		char *temp;
		if (strcmp (oldf->name, fullname) == 0) {
			g_free (fullname);
			return;
		}
		temp = g_strdup_printf ("%s-%d", base_name, i++);
		oldf = g_hash_table_lookup (file_ht, temp);
		g_free (temp);
	}

	if (access (fullname, R_OK) != 0) {
		/* FIXME: perhaps an error of some sort ??? */
		g_free (fullname);
		return;
	}

	if (stat (fullname, &s) < 0) {
		/* FIXME: perhaps an error of some sort ??? */
		g_free (fullname);
		return;
	}

	type = 0;
	icon = NULL;
	if (S_ISDIR (s.st_mode)) {
		type = TYPE_FOLDER;
		icon = folder_icon;
		number_of_dirs ++;
	} else if (S_ISREG (s.st_mode) ||
		   S_ISLNK (s.st_mode)) {
		type = TYPE_REGULAR;
		icon = file_icon;
		number_of_files ++;
	} else {
		/* FIXME: error of some sort */
		g_free (fullname);
		return;
	}

	gtk_widget_set_sensitive (compress_button, TRUE);

	f = g_new0 (File, 1);
	f->type = type;
	f->name = g_strdup (fullname);
	if (oldf == NULL) {
		f->base_name = g_strdup (base_name);
	} else {
		i = 1;
		do {
			g_free (f->base_name);
			f->base_name = g_strdup_printf ("%s-%d",
							base_name, i++);
		} while (g_hash_table_lookup (file_ht, f->base_name) != NULL);
	}

	pos = gnome_icon_list_append (GNOME_ICON_LIST (icon_list), icon,
				      f->base_name);
	gnome_icon_list_set_icon_data (GNOME_ICON_LIST (icon_list), pos, f);

	g_hash_table_insert (file_ht, f->base_name, f);

	update_status ();

	g_free (fullname);
}

static void
add_ok (GtkWidget *widget, GtkFileSelection *fsel)
{
	char *fname;

	g_return_if_fail (GTK_IS_FILE_SELECTION(fsel));

	fname = gtk_file_selection_get_filename (fsel);
	if (fname == NULL || fname[0] == '\0') {
		ERRDLGP (_("No filename selected"), fsel);
		return;
	} else if (access (fname, F_OK) != 0) {
		ERRDLGP (_("File does not exists"), fsel);
		return;
	}
	
	add_file (fname);

	gtk_widget_destroy (GTK_WIDGET (fsel));
}

static void
add_cb (GtkWidget *w, gpointer data)
{
	GtkFileSelection *fsel;

	fsel = GTK_FILE_SELECTION(gtk_file_selection_new(_("Add file or folder")));
	if (filename != NULL)
		gtk_file_selection_set_filename (fsel, filename);

	gtk_signal_connect (GTK_OBJECT (fsel->ok_button), "clicked",
			    GTK_SIGNAL_FUNC (add_ok), fsel);
	gtk_signal_connect_object
		(GTK_OBJECT (fsel->cancel_button), "clicked",
		 GTK_SIGNAL_FUNC (gtk_widget_destroy), 
		 GTK_OBJECT (fsel));

	gtk_window_position (GTK_WINDOW (fsel), GTK_WIN_POS_MOUSE);

	gtk_window_set_transient_for (GTK_WINDOW (fsel),
				      GTK_WINDOW (app));

	gtk_widget_show (GTK_WIDGET (fsel));
}


static void
drag_data_received (GtkWidget          *widget,
		    GdkDragContext     *context,
		    gint                x,
		    gint                y,
		    GtkSelectionData   *data,
		    guint               info,
		    guint               time)
{
	if (data->length >= 0 &&
	    data->format == 8) {
		int i;
		char **files = g_strsplit ((char *)data->data, "\r\n", -1);
		for (i = 0; files != NULL && files[i] != NULL; i++) {
			/* FIXME: EVIL!!!! */
			if (strncmp (files[i], "file:", strlen ("file:")) == 0)
				add_file (files[i] + strlen ("file:"));
		}
		g_strfreev (files);
		gtk_drag_finish (context, TRUE, FALSE, time);
	} else {
		gtk_drag_finish (context, FALSE, FALSE, time);
	}
}

static void
drag_data_get (GtkWidget          *widget,
	       GdkDragContext     *context,
	       GtkSelectionData   *selection_data,
	       guint               info,
	       guint               time,
	       gpointer            data)
{
	char *string;

	if ( ! ensure_temporary ()) {
		/*FIXME: cancel the drag*/
		return;
	}

	string = g_strdup_printf ("file:%s\r\n", temporary_file);
	gtk_selection_data_set (selection_data,
				selection_data->target,
				8, string, strlen (string)+1);
	g_free (string);
}

/* Menus */
static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_ITEM_NONE
		(N_("Add file or folder..."),
		 N_("Add a file or folder to the archive"),
		 add_cb),
	GNOMEUIINFO_ITEM_NONE
		(N_("Create archive..."),
		 N_("Create a new archive from the items"),
		 archive_cb),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_EXIT_ITEM (quit_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu[] = {
	GNOMEUIINFO_ITEM_NONE (N_("Remove"),
			       N_("Remove the selected item(s)"),
			       remove_cb),
	GNOMEUIINFO_MENU_CLEAR_ITEM (clear_cb, NULL),
	GNOMEUIINFO_MENU_SELECT_ALL_ITEM (select_all_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_HELP ("meat-grinder"),
	GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo grinder_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE (file_menu),
	GNOMEUIINFO_MENU_EDIT_TREE (edit_menu),
	GNOMEUIINFO_MENU_HELP_TREE (help_menu),
        GNOMEUIINFO_END
};

static void
init_gui (void)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *sw;
	GtkWidget *w;
	GtkTooltips *tips;

	tips = gtk_tooltips_new ();

        app = gnome_app_new ("meat-grinder",
			     _("GNOME Archive Generator"));
	gtk_window_set_wmclass (GTK_WINDOW (app),
				"meat-grinder",
				"meat-grinder");
	gtk_window_set_policy (GTK_WINDOW (app), FALSE, TRUE, FALSE);

        gtk_signal_connect (GTK_OBJECT (app), "destroy",
			    GTK_SIGNAL_FUNC (quit_cb), NULL);

	/*set up the menu*/
        gnome_app_create_menus (GNOME_APP (app), grinder_menu);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_usize (sw, 250, 150);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_widget_show (sw);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

	icon_list = gnome_icon_list_new (/*evil*/66, NULL, 0);
	gnome_icon_list_set_selection_mode  (GNOME_ICON_LIST (icon_list),
					     GTK_SELECTION_MULTIPLE);
	gtk_widget_show (icon_list);
	gtk_container_add (GTK_CONTAINER (sw), icon_list);

	gtk_tooltips_set_tip (tips, icon_list,
			      _("Drop files here to add them to the archive."),
			      NULL);

	w = gtk_hseparator_new ();
	gtk_widget_show (w);
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	/* tarball icon */
	compress_button = gtk_button_new ();
	gtk_widget_show (compress_button);
	w = NULL;
	if (compress_icon != NULL)
		w = gnome_stock_pixmap_widget (NULL, compress_icon);
	if (w == NULL)
		w = gtk_label_new (_("Archive"));
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (compress_button), w);
	gtk_box_pack_start (GTK_BOX (hbox), compress_button, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (compress_button), "clicked",
			    GTK_SIGNAL_FUNC (archive_cb), NULL);
	gtk_tooltips_set_tip (tips, compress_button,
			      _("Drag this button to the destination where you "
				"want the archive to be created or press it to "
				"pop up a file selection dialog."),
			      NULL);

	gtk_widget_set_sensitive (compress_button, FALSE);

	/* setup dnd */
	gtk_drag_source_set (compress_button,
			     GDK_BUTTON1_MASK|GDK_BUTTON3_MASK,
			     drag_targets, n_drag_targets,
			     GDK_ACTION_COPY);
	/* just in case some wanker like nautilus took our image */
	gtk_signal_connect (GTK_OBJECT (compress_button), "drag_begin",
			    GTK_SIGNAL_FUNC (start_temporary), NULL);
	gtk_signal_connect (GTK_OBJECT (compress_button), "drag_data_get",
			    GTK_SIGNAL_FUNC (drag_data_get), NULL);


	status_label = gtk_label_new ("");
	gtk_label_set_justify (GTK_LABEL (status_label), GTK_JUSTIFY_LEFT);
	gtk_widget_show (status_label);
	gtk_box_pack_start (GTK_BOX (hbox), status_label, FALSE, FALSE, 0);
	update_status ();

	gtk_container_border_width (GTK_CONTAINER (vbox), 5);

	/* setup dnd */
	gtk_drag_dest_set (app,
			   GTK_DEST_DEFAULT_ALL,
			   drop_targets, n_drop_targets,
			   GDK_ACTION_LINK);
	gtk_signal_connect (GTK_OBJECT (app), "drag_data_received",
			    GTK_SIGNAL_FUNC (drag_data_received), NULL);

	gnome_app_set_contents (GNOME_APP (app), vbox);

	gtk_widget_show (app);
}

static void
got_signal (int sig)
{
	cleanup_temporary ();
	
	/* whack thyself */
	signal (sig, SIG_DFL);
	kill (getpid (), sig);
}

int
main (int argc, char *argv [])
{
	int i;
	poptContext ctx;
	const char **files;
	
	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init_with_popt_table ("meat-grinder", VERSION,
				    argc, argv, NULL, 0, &ctx);
	/* no icon yet */
	/*gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-meat-grinder.png");*/

	file_icon = gnome_pixmap_file ("mc/i-regular.png");
	if (file_icon == NULL)
		file_icon = gnome_pixmap_file ("nautilus/i-regular.png");
	if (file_icon == NULL)
		file_icon = gnome_pixmap_file ("nautilus/gnome/i-regular.png");
	if (file_icon == NULL)
		file_icon = gnome_pixmap_file ("gnome-file.png");
	if (file_icon == NULL)
		file_icon = gnome_pixmap_file ("gnome-unknown.png");

	folder_icon = gnome_pixmap_file ("gnome-folder.png");
	if (folder_icon == NULL)
		folder_icon = gnome_pixmap_file ("nautilus/gnome-folder.png");
	if (folder_icon == NULL)
		folder_icon = gnome_pixmap_file ("nautilus/gnome/gnome/gnome-folder.png");
	if (folder_icon == NULL)
		folder_icon = gnome_pixmap_file ("mc/i-directory.png");
	if (folder_icon == NULL)
		folder_icon = gnome_pixmap_file ("gnome-unknown.png");

	compress_icon = gnome_pixmap_file ("gnome-compressed.png");
	if (compress_icon == NULL)
		compress_icon = gnome_pixmap_file ("nautilus/gnome-compressed.png");
	if (compress_icon == NULL)
		compress_icon = gnome_pixmap_file ("mc/gnome-compressed.png");

	if (folder_icon == NULL || file_icon == NULL) {
		ERRDLG (_("Cannot find proper icons anywhere!"));
		exit (1);
	}

	tar_prog = gnome_is_program_in_path ("gtar");
	if (tar_prog == NULL)
		tar_prog = gnome_is_program_in_path ("tar");
	if (tar_prog == NULL) {
		ERRDLG (_("Cannot find the archive (tar) program!\n"
			  "This is the program used for creating archives."));
		exit (1);
	}

	files = poptGetArgs (ctx);

	init_gui ();

	file_ht = g_hash_table_new (g_str_hash, g_str_equal);

	gnome_icon_list_freeze (GNOME_ICON_LIST (icon_list));

	for (i = 0; files != NULL && files[i] != NULL; i++) {
		if (files[i][0] == '/') {
			add_file (files[i]);
		} else {
			char *curdir = g_get_current_dir ();
			char *file = g_concat_dir_and_file (curdir, files[i]);
			add_file (file);
			g_free (file);
			g_free (curdir);
		}
	}

	gnome_icon_list_thaw (GNOME_ICON_LIST (icon_list));

	signal (SIGINT, got_signal);
	signal (SIGTERM, got_signal);

	gtk_main ();

	cleanup_temporary ();
	
	return 0;
}
