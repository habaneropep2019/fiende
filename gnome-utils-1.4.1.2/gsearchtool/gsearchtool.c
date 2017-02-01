/* Gnome Search Tool 
 * (C) 1998,2000 the Free Software Foundation
 *
 * Author:   George Lebl
 */

#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>

#include "gsearchtool.h"
#include "outdlg.h"

#define STDIN  0
#define STDOUT 1
#define STDERR 2

typedef enum {
	NOT_RUNNING,
	RUNNING,
	MAKE_IT_STOP
} RunLevel;

const FindOptionTemplate templates[] = {
#define OPTION_FILENAME 0
	{ FIND_OPTION_TEXT, "-name '%s'", N_("File name") },
#define OPTION_NOSUBDIRS 1
	{ FIND_OPTION_BOOL, "-maxdepth 1", N_("Don't search subdirectories") },
	{ FIND_OPTION_TEXT, "-user '%s'", N_("File owner") },
	{ FIND_OPTION_TEXT, "-group '%s'", N_("File owner group") },
	{ FIND_OPTION_TIME, "-mtime '%s'", N_("Last modification time") },
#define OPTION_NOMOUNTED 5
	{ FIND_OPTION_BOOL, "-mount", N_("Don't search mounted filesystems") },
	{ FIND_OPTION_BOOL, "-empty", N_("Empty file") },
	{ FIND_OPTION_BOOL, "-nouser -o -nogroup",
				N_("Invalid user or group") },
	{ FIND_OPTION_TEXT, "'!' -name '%s'", N_("Filenames except") },
	{ FIND_OPTION_BOOL, "-follow", N_("Follow symbolic links") },
	{ FIND_OPTION_GREP, "fgrep -lZ '%s' --", N_("Simple substring search") },
	{ FIND_OPTION_GREP, "grep -lZ '%s' --", N_("Regular expression search") },
	{ FIND_OPTION_GREP, "egrep -lZ '%s' --",
				N_("Extended regular expression search") },
	{ FIND_OPTION_END, NULL,NULL}
};

/*this will not include the directories in search*/
const static char defoptions[] = "'!' -type d";
/*this should be done if the above is made optional*/
/*char defoptions[] = "-mindepth 0";*/

GList *criteria_find = NULL;
GList *criteria_grep = NULL;

static GtkWidget *find_box = NULL;
static GtkWidget *grep_box = NULL;

static GtkWidget *start_dir_e = NULL;

static GtkWidget *locate_entry = NULL;

static int current_template = 0;

GtkWidget *app; 

static RunLevel find_running = NOT_RUNNING;
static RunLevel locate_running = NOT_RUNNING;

static int
count_char(char *s, char p)
{
	int cnt = 0;
	for(;*s;s++) {
		if(*s == p)
			cnt++;
	}
	return cnt;
}

static char *
quote_quote_string(char *s)
{
	GString *gs;
	char *ret;
	if(count_char(s, '\'') == 0)
		return g_strdup(s);
	gs = g_string_new("");
	for(;*s;s++) {
		if(*s == '\'')
			g_string_append(gs,"'\\''");
		else
			g_string_append_c(gs,*s);
	}

	ret = gs->str;
	g_string_free(gs, FALSE);
	return ret;
}

static char *
makecmd(char *start_dir)
{
	GString *cmdbuf;
	GList *list;

	cmdbuf = g_string_new ("");

	if(start_dir)
		g_string_sprintfa(cmdbuf, "find %s %s ", start_dir, defoptions);
	else
		g_string_sprintfa(cmdbuf, "find . %s ", defoptions);

	for(list=criteria_find;list!=NULL;list=g_list_next(list)) {
		FindOption *opt = list->data;
		if(opt->enabled) {
			char *s;
			switch(templates[opt->templ].type) {
			case FIND_OPTION_BOOL:
				g_string_sprintfa(cmdbuf,"%s ",
						  templates[opt->templ].option);
				break;
			case FIND_OPTION_TEXT:
				s = quote_quote_string(opt->data.text);
				g_string_sprintfa(cmdbuf,
						  templates[opt->templ].option,
						  s);
				g_free(s);
				g_string_append_c(cmdbuf, ' ');
				break;
			case FIND_OPTION_NUMBER:
				g_string_sprintfa(cmdbuf,
						  templates[opt->templ].option,
						  opt->data.number);
				g_string_append_c(cmdbuf, ' ');
				break;
			case FIND_OPTION_TIME:
				s = quote_quote_string(opt->data.time);
				g_string_sprintfa(cmdbuf,
						  templates[opt->templ].option,
						  s);
				g_free(s);
				g_string_append_c(cmdbuf, ' ');
				break;
			case FIND_OPTION_GREP:
				g_warning(_("grep options found in find list bad bad!"));
				break;
			default:
			        break;
			}
		}
	}
	g_string_append(cmdbuf,"-print0 ");

	for(list = criteria_grep; list != NULL; list = g_list_next(list)) {
		FindOption *opt = list->data;
		if(opt->enabled) {
			g_string_sprintfa(cmdbuf, "| xargs -0 ");
			if(templates[opt->templ].type != FIND_OPTION_GREP)
				g_warning(_("non-grep option found in grep list, bad bad!"));
			else {
				char *s = quote_quote_string(opt->data.text);
				g_string_sprintfa(cmdbuf,
						  templates[opt->templ].option,
						  s);
				g_free(s);
			}
			g_string_append_c(cmdbuf, ' ');
		}
	}

	{
		char *ret = cmdbuf->str;
		g_string_free(cmdbuf, FALSE);
		return ret;
	}
}

static gboolean
kill_after_nth_nl (GString *str, int n)
{
	int i;
	int count = 0;
	for (i = 0; str->str[i] != '\0'; i++) {
		if (str->str[i] == '\n') {
			count++;
			if (count == n) {
				g_string_truncate (str, i);
				return TRUE;
			}
		}
	}
	return FALSE;
}

static void
really_run_command(char *cmd, char sepchar, RunLevel *running)
{
	static gboolean lock = FALSE;
	int idle;

	GString *string;
	char ret[PIPE_READ_BUFFER];
	int fd[2], fderr[2];
	int i,n;
	int pid;
	GString *errors = NULL;
	gboolean add_to_errors = TRUE;

	if( ! lock) {
		lock = TRUE;
	} else {
		gnome_app_error(GNOME_APP(app),
				_("Search already running on another page"));
		return;
	}

	/* running = NOT_RUNNING, RUNNING, MAKE_IT_STOP */
	*running = RUNNING;

	/*create the results box*/
	/*FIXME: add an option to autoclear result box and pass TRUE in that
	  case*/
	outdlg_makedlg(_("Search Results"), FALSE);
	
	pipe(fd);
	pipe(fderr);
	
	if ( (pid = fork()) == 0) {
		close(fd[0]);
		close(fderr[0]);
		
		close(STDOUT); 
		close(STDIN);
		close(STDERR);

		dup2(fd[1],STDOUT);
		dup2(fderr[1],STDERR);

		close(fd[1]);
		close(fderr[1]);

		execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
		_exit(0); /* in case it fails */
	}
	close(fd[1]);
	close(fderr[1]);

	outdlg_freeze();
	idle = gtk_idle_add((GtkFunction)gtk_true,NULL);

	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fderr[0], F_SETFL, O_NONBLOCK);

	string = g_string_new (NULL);

	while (*running == RUNNING) {
		n = read (fd[0], ret, PIPE_READ_BUFFER);
		for (i = 0; i < n; i++) {
			if(ret[i] == sepchar) {
				outdlg_additem (string->str);
				g_string_assign (string, "");
			} else {
				g_string_append_c (string, ret[i]);
			}
		}

		n = read (fderr[0], ret, PIPE_READ_BUFFER-1);
		if (n > 0) {
			ret[n] = '\0';
			if (add_to_errors) {
				if (errors == NULL)
					errors = g_string_new (ret);
				else
					errors = g_string_append (errors, ret);
				add_to_errors =
					! kill_after_nth_nl (errors, 20);
			}
			fprintf (stderr, "%s", ret);
		}
		
		if (waitpid (-1, NULL, WNOHANG) != 0)
			break;
		/*this for some reason doesn't work, I need to add an
		  idle handler and do iteration with TRUE*/
		/*if(gtk_events_pending())
			gtk_main_iteration_do(FALSE);*/
		gtk_main_iteration_do (TRUE);
		if (*running == MAKE_IT_STOP) {
			kill(pid, SIGKILL);
			wait(NULL);
		}
	}
	/* now we got it all ... so finish reading from the pipe */
	while ((n = read (fd[0], ret, PIPE_READ_BUFFER)) > 0) {
		for (i = 0; i < n; i++) {
			if (ret[i] == sepchar) {
				outdlg_additem (string->str);
				g_string_assign (string, "");
			} else {
				g_string_append_c (string, ret[i]);
			}
		}
	}
	while((n = read(fderr[0], ret, PIPE_READ_BUFFER-1)) > 0) {
		ret[n]='\0';
		if (add_to_errors) {
			if (errors == NULL)
				errors = g_string_new (ret);
			else
				errors = g_string_append (errors, ret);
			add_to_errors =
				! kill_after_nth_nl (errors, 20);
		}
		fprintf (stderr, "%s", ret);
	}

	close(fd[0]);
	close(fderr[0]);

	gtk_idle_remove(idle);
	outdlg_thaw();

	outdlg_showdlg();
	
	/* if any errors occured */
	if(errors) {
		if ( ! add_to_errors) {
			errors = g_string_append
				(errors,
				 _("\n... Too many errors to display ..."));
		}
		/* make an error message */
		gnome_app_error (GNOME_APP (app), errors->str);
		/* freeing allocated memory */
		g_string_free (errors, TRUE);
	}

	g_string_free (string, TRUE);

	*running = NOT_RUNNING;

	lock = FALSE;
}

static void
run_command(GtkWidget *w, gpointer data)
{
	char *cmd;
	GtkWidget **buttons = data;

	char *start_dir;

	if (buttons[0] == w) { /*we are in the stop button*/
		if(find_running == RUNNING)
			find_running = MAKE_IT_STOP;
		return;
	}

	if(start_dir_e) {
		start_dir = gnome_file_entry_get_full_path(GNOME_FILE_ENTRY(start_dir_e), TRUE);
		if(!start_dir) {
			gnome_app_error (GNOME_APP(app),
					 _("Start directory does not exist"));
			return;
		}
	} else
		start_dir = NULL;

	gtk_widget_set_sensitive(buttons[0], TRUE);
	gtk_widget_set_sensitive(buttons[1], FALSE);
	
	cmd = makecmd(start_dir);
	g_free(start_dir);

	really_run_command(cmd, '\0', &find_running);
	g_free(cmd);

	gtk_widget_set_sensitive(buttons[0], FALSE);
	gtk_widget_set_sensitive(buttons[1], TRUE);
}

static void
run_cmd_dialog(GtkWidget *wid, gpointer data)
{
	char *cmd;
	char *start_dir;
	GtkWidget *dlg;
	GtkWidget *w;

	if(start_dir_e) {
		start_dir = gnome_file_entry_get_full_path(GNOME_FILE_ENTRY(start_dir_e), TRUE);
		if(!start_dir) {
			gnome_app_error (GNOME_APP(app),
					 _("Start directory does not exist"));
			return;
		}
	} else
		start_dir = NULL;

	cmd = makecmd(start_dir);
	g_free(start_dir);

	dlg = gnome_dialog_new(_("Search command line"),
			       GNOME_STOCK_BUTTON_CLOSE,
			       NULL);
	gnome_dialog_set_close(GNOME_DIALOG(dlg), TRUE);
	gnome_dialog_set_parent(GNOME_DIALOG(dlg), GTK_WINDOW(app));

	w = gtk_label_new(_("This is the command line that can be used to "
			    "execute this search from the console:\n"
			    "(Note: to print one file per line rather then "
			    "null separated,\nappend \"| tr '\\000' '\\n'\" "
			    "to the line below)"));
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dlg)->vbox), w, TRUE, TRUE, 0);

	w = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w), cmd);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dlg)->vbox), w, FALSE, FALSE, 0);

	gtk_widget_show_all(dlg);

	g_free(cmd);
}


static void
menu_toggled(GtkWidget *w, gpointer data)
{
	if(GTK_CHECK_MENU_ITEM(w)->active)
		current_template = (long)data;
}

static GtkWidget *
make_list_of_templates(void)
{
	GtkWidget *menu;
	GtkWidget *menuitem;
	GSList *group=NULL;
	int i;

	menu = gtk_menu_new ();

	for(i=0;templates[i].type!=FIND_OPTION_END;i++) {
		menuitem=gtk_radio_menu_item_new_with_label(group,
							    _(templates[i].desc));
		gtk_signal_connect(GTK_OBJECT(menuitem),"toggled",
				   GTK_SIGNAL_FUNC(menu_toggled),
				   (gpointer)(long)i);
		group=gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
		gtk_menu_append (GTK_MENU (menu), menuitem);
		gtk_widget_show (menuitem);
	}
	return menu;
}

static void
remove_option(GtkWidget *w, gpointer data)
{
	FindOption *opt = data;
	if(templates[opt->templ].type == FIND_OPTION_GREP) {
		gtk_container_remove(GTK_CONTAINER(grep_box),w->parent);
		criteria_grep = g_list_remove(criteria_grep,opt);
	} else {
		gtk_container_remove(GTK_CONTAINER(find_box),w->parent);
		criteria_find = g_list_remove(criteria_find,opt);
	}
}

static void
enable_option(GtkWidget *w, gpointer data)
{
	FindOption *opt = data;
	GtkWidget *frame = gtk_object_get_user_data(GTK_OBJECT(w));
	gtk_widget_set_sensitive(GTK_WIDGET(frame),
				 GTK_TOGGLE_BUTTON(w)->active);
	opt->enabled = GTK_TOGGLE_BUTTON(w)->active;
}

static void
entry_changed(GtkWidget *w, gpointer data)
{
	FindOption *opt = data;
	switch(templates[opt->templ].type) {
	case FIND_OPTION_TEXT:
	case FIND_OPTION_GREP:
		opt->data.text = gtk_entry_get_text(GTK_ENTRY(w));
		break;
	case FIND_OPTION_NUMBER:
		sscanf(gtk_entry_get_text(GTK_ENTRY(w)),"%d",
		       &opt->data.number);
		break;
	case FIND_OPTION_TIME:
		opt->data.time = gtk_entry_get_text(GTK_ENTRY(w));
		break;
	default:
		g_warning(_("Entry changed called for a non entry option!"));
		break;
	}
}

char empty_str[]="";

static void
set_option_defaults(FindOption *opt)
{
	switch(templates[opt->templ].type) {
	case FIND_OPTION_BOOL:
		break;
	case FIND_OPTION_TEXT:
	case FIND_OPTION_GREP:
		opt->data.text = empty_str;
		break;
	case FIND_OPTION_NUMBER:
		opt->data.number = 0;
		break;
	case FIND_OPTION_TIME:
		opt->data.time = empty_str;
		break;
	default:
	        break;
	}
}

static GtkWidget *
create_option_box(FindOption *opt, gboolean enabled)
{
	GtkWidget *hbox;
	GtkWidget *option;
	GtkWidget *frame;
	GtkWidget *w;

	hbox = gtk_hbox_new(FALSE,5);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_OUT);
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

	switch(templates[opt->templ].type) {
	case FIND_OPTION_BOOL:
		option = gtk_label_new(_(templates[opt->templ].desc));
		gtk_misc_set_alignment(GTK_MISC(option), 0.0, 0.5);
		break;
	case FIND_OPTION_TEXT:
	case FIND_OPTION_NUMBER:
	case FIND_OPTION_TIME:
	case FIND_OPTION_GREP:
		option = gtk_hbox_new(FALSE,5);
		w = gtk_label_new(_(templates[opt->templ].desc));
		gtk_box_pack_start(GTK_BOX(option),w,FALSE,FALSE,0);
		w = gtk_entry_new();
		gtk_signal_connect(GTK_OBJECT(w),"changed",
				   GTK_SIGNAL_FUNC(entry_changed),opt);
		gtk_box_pack_start(GTK_BOX(option),w,TRUE,TRUE,0);
		break;
	default:
		/* This should never happen, if it does, there is a bug */
		option = gtk_label_new("???");
	        break;
	}
	gtk_container_add(GTK_CONTAINER(frame), option);

	w = gtk_check_button_new_with_label(_("Enable"));
	gtk_object_set_user_data(GTK_OBJECT(w), frame);
	gtk_signal_connect(GTK_OBJECT(w), "toggled",
			   GTK_SIGNAL_FUNC(enable_option), opt);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), enabled);
	enable_option(w, opt);
	gtk_box_pack_start(GTK_BOX(hbox), w, FALSE,FALSE,0);

	w = gtk_button_new_with_label(_("Remove"));
	gtk_signal_connect(GTK_OBJECT(w), "clicked",
			   GTK_SIGNAL_FUNC(remove_option), opt);
	gtk_box_pack_start(GTK_BOX(hbox), w, FALSE,FALSE, 0);

	return hbox;
}

static void
add_option(int templ, gboolean enabled)
{
	FindOption *opt = g_new(FindOption,1);
	GtkWidget *w;

	opt->templ = templ;
	opt->enabled = TRUE;

	set_option_defaults(opt);

	w = create_option_box(opt, enabled);
	gtk_widget_show_all(w);

	/*if it's a grep type option (criterium)*/
	if(templates[templ].type == FIND_OPTION_GREP) {
		criteria_grep = g_list_append(criteria_grep,opt);
		gtk_box_pack_start(GTK_BOX(grep_box),w,FALSE,FALSE,0);
	} else {
		criteria_find = g_list_append(criteria_find,opt);
		gtk_box_pack_start(GTK_BOX(find_box),w,FALSE,FALSE,0);
	}
}

static void
add_option_cb(GtkWidget *w, gpointer data)
{
	add_option(current_template, TRUE);
}

static GtkWidget *
create_find_page(void)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *w;
	char *s;
	static GtkWidget *buttons[2];

	vbox = gtk_vbox_new(FALSE,GNOME_PAD_SMALL);
	gtk_container_border_width(GTK_CONTAINER(vbox),GNOME_PAD_SMALL);

	hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	w = gtk_label_new(_("Start in directory:"));
	gtk_box_pack_start(GTK_BOX(hbox),w,FALSE,FALSE,0);
	start_dir_e = gnome_file_entry_new("directory", _("Browse"));
	gnome_file_entry_set_directory(GNOME_FILE_ENTRY(start_dir_e), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),start_dir_e,TRUE,TRUE,0);
	w = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(start_dir_e));
	s = g_get_current_dir();
	gtk_entry_set_text(GTK_ENTRY(w), s);
	g_free(s);

	find_box = gtk_vbox_new(TRUE,GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox),find_box,FALSE,FALSE,0);
	grep_box = gtk_vbox_new(TRUE,GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox),grep_box,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	w = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(w), make_list_of_templates());
	gtk_box_pack_start(GTK_BOX(hbox),w,FALSE,FALSE,0);

	w = gtk_button_new_with_label(_("Add"));
	gtk_signal_connect(GTK_OBJECT(w),"clicked",
			   GTK_SIGNAL_FUNC(add_option_cb),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),w,FALSE,FALSE,0);

	hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
	gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	w = gtk_hseparator_new();
	gtk_box_pack_end(GTK_BOX(vbox),w,FALSE,FALSE,0);

	w = gtk_button_new_with_label(_("Show Command"));
	gtk_signal_connect(GTK_OBJECT(w), "clicked",
			   GTK_SIGNAL_FUNC(run_cmd_dialog), NULL);
	gtk_box_pack_end(GTK_BOX(hbox), w, FALSE, FALSE, 0);

	buttons[1] = gtk_button_new_with_label(_("Start"));
	buttons[0] = gtk_button_new_with_label(_("Stop"));
	gtk_signal_connect(GTK_OBJECT(buttons[1]),"clicked",
			   GTK_SIGNAL_FUNC(run_command), buttons);
	gtk_signal_connect(GTK_OBJECT(buttons[0]),"clicked",
			   GTK_SIGNAL_FUNC(run_command), buttons);
	gtk_box_pack_end(GTK_BOX(hbox),buttons[0],FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),buttons[1],FALSE,FALSE,0);
	gtk_widget_set_sensitive(buttons[0],FALSE);

	add_option(OPTION_FILENAME, TRUE);
	add_option(OPTION_NOSUBDIRS, FALSE);
	add_option(OPTION_NOMOUNTED, FALSE);

	return vbox;
}

static void
run_locate_command(GtkWidget *w, gpointer data)
{
	char *cmd;
	GtkWidget **buttons = data;

	char *locate_string;

	if (buttons[0] == w) { /*we are in the stop button*/
		if(locate_running == RUNNING)
			locate_running = MAKE_IT_STOP;
		return;
	}

	locate_string = gtk_entry_get_text(GTK_ENTRY(locate_entry));
	if(!locate_string || !*locate_string) {
		gnome_app_error (GNOME_APP(app),
				 _("Nothing to locate"));
		return;
	}
	locate_string = quote_quote_string(locate_string);

	gtk_widget_set_sensitive(buttons[0], TRUE);
	gtk_widget_set_sensitive(buttons[1], FALSE);

	cmd = g_strdup_printf("locate '%s'", locate_string);

	g_free(locate_string);

	really_run_command(cmd, '\n', &locate_running);
	g_free(cmd);

	gtk_widget_set_sensitive(buttons[0], FALSE);
	gtk_widget_set_sensitive(buttons[1], TRUE);
}

static void
locate_activate (GtkWidget *entry, gpointer data)
{
	GtkWidget **buttons = data;
	run_locate_command (buttons[1], buttons);
}

static GtkWidget *
create_locate_page(void)
{
	GtkWidget *w, *vbox, *hbox;
	static GtkWidget *buttons[2];

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

	w = gtk_label_new(_("This is an interface to locate.  If you type in "
			    "a simple string it\nwill be matched as a subset "
			    "of the full path, and if you type\nin a string "
			    "with wildcards, it will have to match the full "
			    "path."));
	gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	w = gtk_label_new(_("Locate file: "));
	gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 0);

	locate_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), locate_entry, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (locate_entry), "activate",
			    GTK_SIGNAL_FUNC (locate_activate),
			    buttons);

	hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
	gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	w = gtk_hseparator_new();
	gtk_box_pack_end(GTK_BOX(vbox),w,FALSE,FALSE,0);

	buttons[1] = gtk_button_new_with_label(_("Start"));
	buttons[0] = gtk_button_new_with_label(_("Stop"));
	gtk_signal_connect(GTK_OBJECT(buttons[1]),"clicked",
			   GTK_SIGNAL_FUNC(run_locate_command), buttons);
	gtk_signal_connect(GTK_OBJECT(buttons[0]),"clicked",
			   GTK_SIGNAL_FUNC(run_locate_command), buttons);
	gtk_box_pack_end(GTK_BOX(hbox),buttons[0],FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),buttons[1],FALSE,FALSE,0);
	gtk_widget_set_sensitive(buttons[0],FALSE);

	return vbox;
}

static GtkWidget *
create_window(void)
{
	GtkWidget *nbook;

	nbook = gtk_notebook_new();
	gtk_container_border_width(GTK_CONTAINER(nbook),GNOME_PAD_SMALL);

	gtk_notebook_append_page(GTK_NOTEBOOK(nbook),create_find_page(),
				 gtk_label_new(_("Full find (find)")));
	gtk_notebook_append_page(GTK_NOTEBOOK(nbook),create_locate_page(),
				 gtk_label_new(_("Quick find (locate)")));

	return nbook;
}

static void
about_cb (GtkWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	static const char *authors[] = {
		"George Lebl <jirka@5z.com>",
		NULL
	};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}

	about = gnome_about_new(_("The Gnome Search Tool"), VERSION,
				_("(C) 1998,2000 the Free Software Foundation"),
				authors,
				_("Frontend to the unix find/grep/locate "
				  "commands"),
				NULL);
	gtk_signal_connect (GTK_OBJECT (about), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);
	gtk_widget_show (about);
}

/* thy evil easter egg */
static gboolean
window_click(GtkWidget *w, GdkEventButton *be)
{
	static int foo = 0;
	if(be->button == 3 && (++foo)%3 == 0)
		gnome_ok_dialog("9\\_/_-\n  /\\ /\\\n\nGEGL!");
	return TRUE;
}

static void
quit_cb(GtkWidget *w, gpointer data)
{
	if(find_running == RUNNING)
		find_running = MAKE_IT_STOP;
	if(locate_running == RUNNING)
		locate_running = MAKE_IT_STOP;
	gtk_main_quit();
}

static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_EXIT_ITEM(quit_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {  
	GNOMEUIINFO_HELP("gsearchtool"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo gsearch_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
        GNOMEUIINFO_END
};


int
main(int argc, char *argv[])
{
	GtkWidget *search;

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init ("gsearchtool", VERSION, argc, argv);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-searchtool.png");

        app = gnome_app_new("gsearchtool", _("Gnome Search Tool"));
	gtk_window_set_wmclass (GTK_WINDOW (app), "gsearchtool", "gsearchtool");
	gtk_window_set_policy (GTK_WINDOW (app), TRUE, TRUE, TRUE);

        gtk_signal_connect(GTK_OBJECT(app), "delete_event",
			   GTK_SIGNAL_FUNC(quit_cb), NULL);

	gtk_signal_connect(GTK_OBJECT(app), "button_press_event",
			   GTK_SIGNAL_FUNC(window_click), NULL);

	/*set up the menu*/
        gnome_app_create_menus(GNOME_APP(app), gsearch_menu);

	search = create_window();
	gtk_widget_show_all(search);

	gnome_app_set_contents(GNOME_APP(app), search);

	gtk_widget_show(app);
	
	gtk_main ();

	return 0;
}
