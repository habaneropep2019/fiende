#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>

static GList *hintlist = NULL;
static int hintnum = 0;

static GList *curhint = NULL;
static char *curfortune = NULL;

static GtkWidget *canvas;
static GtkWidget *cb;
static GtkWidget *sw;
static GnomeCanvasItem *hint_item;
static GnomeCanvasItem *blue_background;
static GnomeCanvasItem *white_background;
static int width = 400,height = 200;

static gboolean browse_hints = FALSE;
static gboolean browse_motd = FALSE;
static gboolean browse_fortune = FALSE;
static gboolean session_login = FALSE;

struct poptOption options[] = {
  { "browse-hints", '\0', POPT_ARG_NONE, &browse_hints, 0,
	  N_("Start in hint browsing mode"),
	  NULL
  },
  { "browse-motd", '\0', POPT_ARG_NONE, &browse_motd, 0,
	  N_("Start in motd mode"),
	  NULL
  },
  { "browse-fortune", '\0', POPT_ARG_NONE, &browse_fortune, 0,
	  N_("Start in fortune mode"),
	  NULL
  },
  { "session-login", '\0', POPT_ARG_NONE, &session_login, 0,
	  N_("Start in session login mode (used from gsm)"),
	  NULL
  },
  { NULL }
};



static char *
default_hint(void)
{
	return  g_strdup(_("Click on the GNOME foot icon to open the\n"
			   "Main Menu.  This menu contains all\n"
			   "GNOME applications, tools, and commands."));
}

/*find the language, but only look as far as gotlang*/
static GList *
find_lang (GList *langlist, const GList *gotlang, const char *lang)
{
	while (langlist != NULL &&
	       langlist != gotlang) {
		if (strcmp (langlist->data, lang) == 0)
			return langlist;
		langlist = langlist->next;
	}
	return NULL;
}

/*parse all children and pick out the best language one*/
static char *
get_i18n_string (xmlDocPtr doc, xmlNodePtr child, const char *name)
{
	GList *langlist;
	char *current;
	xmlNodePtr cur;
	GList *gotlang = NULL; /*the highest language we got*/
	
	langlist = gnome_i18n_get_language_list("LC_ALL");
	
	current = NULL;

	/*find C the locale string*/
	for(cur = child->childs; cur; cur = cur->next) {
		char *lang;
		if (cur->name == NULL ||
		    g_strcasecmp (cur->name, name) != 0)
			continue;

		lang = xmlGetProp (cur, "xml:lang");
		if (lang == NULL ||
		    lang[0] == '\0') {
			if (lang != NULL)
				xmlFree (lang);
			if (gotlang != NULL)
				continue;
			if (current != NULL)
				xmlFree (current);
			current = xmlNodeListGetString (doc, cur->childs, 1);
		} else {
			GList *ll = find_lang (langlist, gotlang, lang);
			xmlFree (lang);
			if (ll != NULL) {
				if (current != NULL)
					xmlFree (current);
				current = xmlNodeListGetString (doc, cur->childs, 1);
				gotlang = ll;
				if (ll == langlist) /*we can't get any better then this*/
					break;
			}
		}
	}
	return current;
}

static void
read_hints_from_file (const char *file)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	doc = xmlParseFile(file);
	if (doc == NULL)
		return;
	
	if (doc->root == NULL ||
	    doc->root->name == NULL ||
	    g_strcasecmp (doc->root->name, "GnomeHints") != 0) {
		xmlFreeDoc (doc);
		return;
	}

	for (cur = doc->root->childs; cur; cur = cur->next) {
		char *str;
		if (cur->name == NULL ||
		    g_strcasecmp (cur->name, "Hint") != 0)
			continue;
		str = get_i18n_string (doc, cur, "Content");
		if (str != NULL) {
			hintlist = g_list_prepend (hintlist, str);
			hintnum++;
		}
	}
	
	xmlFreeDoc (doc);
}

static void
read_in_hints(void)
{
	DIR *dir;
	struct stat s;
	struct dirent *dent;
	char * name;
       
	/*add the default hint*/
	hintlist = g_list_prepend (hintlist, default_hint());
	hintnum++;

	/* see if we find the directory with the hints */
	name = gnome_datadir_file ("gnome/hints");
	if (name == NULL)
		return;
	if (stat (name, &s) != 0 ||
	    !S_ISDIR(s.st_mode)) {
		g_free (name);
		return;
	}
	
	dir = opendir (name);
	if (dir == NULL) {
		g_free (name);
		return;
	}

	/* read the directory file by file */
	while((dent = readdir(dir)) != NULL) {
		char *file;
		/* Skip over dot files */
		if(dent->d_name[0] == '.')
			continue;
		file = g_concat_dir_and_file(name,dent->d_name);
		if(stat(file, &s) != 0 ||
		   !S_ISREG(s.st_mode))
			continue;
		read_hints_from_file (file);
		g_free (file);
	}
	g_free (name);
	closedir (dir);
	
	if (hintlist != NULL)
		/*we read it all in reverse in fact,
		  so now we just put it back*/
		hintlist = g_list_reverse (hintlist);
}

static void
pick_random_hint(void)
{
	srandom(time(NULL));
	
	/*the random is not truly random thanks to me %'ing it,
	  but you know what ... who cares*/
	curhint = g_list_nth (hintlist, random()%hintnum);
}

/*returns a newly allocated expanded string, or the original pointer
  if there are no tabs*/
static char *
expand_str(char *s)
{
	int tab_count = 0;
	char *p,*pp;
	char *n;
	int i;
	p = s;
	while((p=strchr(p,'\t'))) {
		tab_count++;
		p++;
	}
	if(tab_count == 0)
		return s;
	n = g_new(char,strlen(s)+(tab_count*4)+1);
	for(i=0,p=s,pp=n;*p;p++,pp++,i++) {
		if(*p=='\t') {
			*pp = ' ';
			while((i+1)%5>0) {
				*(++pp) = ' ';
				i++;
			}
		} else
			*pp = *p;
	}
	*pp = '\0';
	return n;
}

static char *
get_a_fortune(void)
{
	char *fortune_command;
       
	fortune_command = g_file_exists("/usr/games/fortune")?
		g_strdup("/usr/games/fortune"):
		gnome_is_program_in_path("fortune");

	if(fortune_command) {
		FILE *fp;
		fp = popen(fortune_command,"r");
		g_free(fortune_command);
		if(fp) {
			char buf[256];
			char *ret;
			GString *gs = g_string_new("");
			while(fgets(buf,256,fp)) {
				char *p = expand_str(buf);
				g_string_append(gs,p);
				if(p!=buf) g_free(p);
			}
			fclose(fp);
			ret = gs->str;
			g_string_free(gs,FALSE);
			return ret;
		}
	}

	return g_strdup(_("You do not have fortune installed."));

}

static void
grow_text_if_necessary(void)
{
	double w,h;
	int ww,hh;
	int changed = FALSE;
	
	gtk_object_get(GTK_OBJECT(hint_item),
		       "text_width",&w,
		       "text_height",&h,
		       NULL);
	/*add border, and 10 pixels around*/
	w+=75+10;
	h+=50+10;
	/*some sanity limits*/
	/*if(w>800) w = 800;
	if(h>600) h = 600;*/

	if(w>width) {
		width = w;
		changed = TRUE;
	}
	if(h>height) {
		height = h;
		changed = TRUE;
	}

	if(!changed)
		return;
	
	/*limits on size*/
	ww = width; hh = height;
	if(ww>720) ww = 720;
	if(hh>450) hh = 450;

	if(ww != width || hh != height)
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
					       GTK_POLICY_AUTOMATIC,
					       GTK_POLICY_AUTOMATIC);
	else
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
					       GTK_POLICY_NEVER,
					       GTK_POLICY_NEVER);

	/*here we grow the canvas*/
	gtk_widget_set_usize(canvas,ww,hh);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas),
				       0.0,0.0,width,height);
	
	gnome_canvas_item_set(blue_background,
			      "x2",(double)width,
			      "y2",(double)height,
			      NULL);
	gnome_canvas_item_set(white_background,
			      "x2",(double)width,
			      "y2",(double)height,
			      NULL);
	gnome_canvas_item_set(hint_item,
			      "x",(double)(((width-75)/2)+75),
			      "y",(double)(((height-50)/2)+50),
			      "clip_width",(double)(width-75),
			      "clip_height",(double)(height-50),
			      NULL);
}

/* evil lies herein */
static gboolean
title_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
	static int clicks = 0;

	if (event->type == GDK_BUTTON_PRESS) {
		clicks ++;
		if (clicks == 3) {
			gnome_canvas_item_set
				(item,
				 "text", "ALL YOUR BASE ARE BELONG TO US",
				 NULL);
		}
	}

	return FALSE;
}

static void
draw_on_canvas(GtkWidget *canvas, gboolean is_fortune, gboolean is_motd, const char *hint)
{
	GnomeCanvasItem *item;
	char *title_text;
	
	blue_background = gnome_canvas_item_new(
		gnome_canvas_root(GNOME_CANVAS(canvas)),
		gnome_canvas_rect_get_type(),
		"x1",(double)0.0,
		"y1",(double)0.0,
		"x2",(double)400.0,
		"y2",(double)200.0,
		"fill_color","blue",
		NULL);

	white_background =
		gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS(canvas)),
				       gnome_canvas_rect_get_type(),
				       "x1",(double)75.0,
				       "y1",(double)50.0,
				       "x2",(double)400.0,
				       "y2",(double)200.0,
				       "fill_color","white",
				       NULL);

	hint_item = gnome_canvas_item_new
		(gnome_canvas_root (GNOME_CANVAS(canvas)),
		 gnome_canvas_text_get_type (),
		 "x", (double)237.5,
		 "y", (double)125.0,
		 "fill_color", "black",
		 "clip_width", (double)325.0,
		 "clip_height", (double)150.0,
		 "clip", TRUE,
		 "text", hint,
		 NULL);
	
	if (is_fortune || is_motd) {
		gnome_canvas_item_set
			(GNOME_CANVAS_ITEM (hint_item),
			 /* the fixed font should be a font that is of a fixed
			  * spacing, such as would be one in a terminal */
			 "fontset", _("fixed"),
			 NULL);
	} else {
		gnome_canvas_item_set (GNOME_CANVAS_ITEM (hint_item),
				       "font_gdk", canvas->style->font,
				       NULL);
	}

	if (is_fortune)
		title_text = _("Fortune");
	else if (is_motd)
		title_text = _("Message of The Day");
	else
		title_text = _("GNOME Hints");

	item = gnome_canvas_item_new(
		gnome_canvas_root(GNOME_CANVAS(canvas)),
		gnome_canvas_text_get_type(),
		"x",(double)200.0,
		"y",(double)25.0,
		"fill_color","white",
		"fontset",_("-*-helvetica-bold-r-normal-*-*-180-*-*-p-*-*-*"),
		"text", title_text,
		NULL);
	gtk_signal_connect (GTK_OBJECT (item), "event",
			    GTK_SIGNAL_FUNC (title_event),
			    NULL);

	grow_text_if_necessary();
}

static void
exit_clicked(GtkWidget *window)
{
	GtkWidget *message_box;
	gboolean old_run_hints;
	gboolean new_run_hints;

	/* if not in session login mode, just quit */
	if(!session_login) {
		gtk_main_quit();
		return;
	}

	old_run_hints = gnome_config_get_bool ("/Gnome/Login/RunHints=true");
	new_run_hints = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb));
	/* show the dialog ONLY when the user toggled the hints off just
	 * now, and not if they were off before */
	if (old_run_hints && !new_run_hints) {
		message_box = gnome_message_box_new (_("You've chosen to disable the startup hint.\n"
						       "To re-enable it, choose \"Startup Hint\"\n"
						       "in the GNOME Control Center"),
						     GNOME_MESSAGE_BOX_INFO,
						     GNOME_STOCK_BUTTON_OK,
						     NULL);
		gnome_dialog_set_parent(GNOME_DIALOG(message_box),
					GTK_WINDOW(window));

		/* show now guarantees that the window will be realized and
		 * shown before it returns */
		gtk_widget_show_now (message_box);

		gnome_win_hints_set_layer(message_box, WIN_LAYER_ONTOP);

		/* raise window above the hints dialog if obscured */
		gdk_window_raise(message_box->window);
		
		gnome_dialog_run (GNOME_DIALOG (message_box));
	}
	gnome_config_set_bool ("/Gnome/Login/RunHints", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cb)));
	gnome_config_sync ();
	
	gtk_main_quit ();
}

static void
fortune_clicked(GtkWidget *window, int button, gpointer data)
{
	switch(button) {
	case 0:
		g_free(curfortune);
		curfortune = get_a_fortune();
		gnome_canvas_item_set(hint_item,
				      "text",curfortune,
				      NULL);
		grow_text_if_necessary();
		break;
	default:
		exit_clicked (window);
	}
}

static void
hints_clicked(GtkWidget *window, int button, gpointer data)
{
	switch(button) {
	case 0:
		if (curhint == NULL)
			return;
		curhint = curhint->prev;
		if (curhint == NULL)
			curhint = g_list_last (hintlist);
		gnome_canvas_item_set(hint_item,
				      "text",(char *)curhint->data,
				      NULL);
		grow_text_if_necessary();
		break;
	case 1:
		if (curhint == NULL)
			return;
		curhint = curhint->next;
		if (curhint == NULL)
			curhint = hintlist;
		gnome_canvas_item_set(hint_item,
				      "text",(char *)curhint->data,
				      NULL);
		grow_text_if_necessary();
		break;
	default:
		exit_clicked (window);
	}
}

static char *
get_motd(void)
{
	char *motd;
	FILE *fp;

	motd = gnome_config_get_string("/Gnome/Login/MotdFile=/etc/motd");

	fp = fopen(motd, "r");
	g_free(motd);
	if(fp) {
		char buf[256];
		GString *gs = g_string_new("");
		while(fgets(buf, 256, fp)) {
			char *p = expand_str(buf);
			g_string_append(gs, p);
			if(p!=buf) g_free(p);
		}
		fclose(fp);
		if(*gs->str) {
			char *ret = gs->str;
			g_string_free(gs, FALSE);
			return ret;
		} else
			g_string_free(gs, TRUE);
	}
	return g_strdup(_("No message of the day found!"));
}

static void
window_realize(GtkWidget *win)
{
	/* this is done because on startup gnome-hint needs to be on
	   top without other starting apps overlaying it, since it's
	   what the user should see (especially on the first run) so
	   we can't have it obscured */
	if(session_login)
		gnome_win_hints_set_layer(win, WIN_LAYER_ONTOP);
}

int
main(int argc, char *argv[])
{
	GtkWidget *win;
	char *hint;
	GnomeClient *client;
        GnomeClientFlags flags;
	gboolean is_fortune;
	gboolean is_motd;
	poptContext ctx;

	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	gnome_init_with_popt_table ("gnome-hint", VERSION, argc, argv,
				    options, 0, &ctx);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-hint.png");
	/* never let us be restarted */
	client = gnome_master_client();
	flags = gnome_client_get_flags(client);

	if (flags & GNOME_CLIENT_IS_CONNECTED) {
		gnome_client_set_restart_style (client, 
						GNOME_RESTART_NEVER);
                gnome_client_flush (client);
        }

	if(browse_hints) {
		/* if we are in hint browsing mode, ignore the fortunes and
		 * motd */
		is_fortune = FALSE;
		is_motd = FALSE;
	} else {
		/* we can give fortune instead of hints in fact */
		is_fortune = gnome_config_get_bool("/Gnome/Login/HintsAreFortune=false");

		/* actually give a MOTD, not a fortune nor a hint */
		is_motd = gnome_config_get_bool("/Gnome/Login/HintsAreMotd=false");
	}

	if(is_motd) {
		win = gnome_dialog_new(_("Message of The Day"),
				       GNOME_STOCK_BUTTON_CLOSE,
				       NULL);
		gtk_signal_connect(GTK_OBJECT(win), "clicked",
				   GTK_SIGNAL_FUNC(exit_clicked),
				   NULL);
	} else if(is_fortune) {
		win = gnome_dialog_new(_("Fortune"),
				       GNOME_STOCK_BUTTON_NEXT,
				       GNOME_STOCK_BUTTON_CLOSE,
				       NULL);
		gtk_signal_connect(GTK_OBJECT(win), "clicked",
				   GTK_SIGNAL_FUNC(fortune_clicked),
				   NULL);
	} else {
		win = gnome_dialog_new(_("Gnome hint"),
				       GNOME_STOCK_BUTTON_PREV,
				       GNOME_STOCK_BUTTON_NEXT,
				       GNOME_STOCK_BUTTON_CLOSE,
				       NULL);
		gtk_signal_connect(GTK_OBJECT(win), "clicked",
				   GTK_SIGNAL_FUNC(hints_clicked),
				   NULL);
	}
	gtk_signal_connect(GTK_OBJECT(win), "delete_event",
			   GTK_SIGNAL_FUNC(exit_clicked),
			   NULL);
	gtk_window_set_position(GTK_WINDOW(win),GTK_WIN_POS_CENTER);
	gtk_signal_connect_after(GTK_OBJECT(win), "realize",
				 GTK_SIGNAL_FUNC(window_realize), NULL);

	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_NEVER);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(win)->vbox),sw,TRUE,TRUE,0);

	/* only add the display this dialog next time if we are in session
	 * login mode */
	if(session_login) {
		cb = gtk_check_button_new_with_label (_("Display this dialog next time"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cb),
					      gnome_config_get_bool("/Gnome/Login/RunHints=true"));
		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(win)->vbox),cb,TRUE,TRUE,0);
	}

	canvas = gnome_canvas_new();
	gtk_container_add(GTK_CONTAINER(sw),canvas);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas),
				       0.0,0.0,width,height);
	gtk_widget_set_usize(canvas,width,height);
	gtk_widget_ensure_style(canvas);
	
	if(is_motd) {
		hint = get_motd();
	} else if(is_fortune) {
		curfortune = hint = get_a_fortune();
	} else {
		read_in_hints();
		if(gnome_config_get_bool("/gnome-hint/stuff/first_run=TRUE")) {
			curhint = hintlist;
		} else {
			pick_random_hint();
		}
		hint = curhint->data;

		gnome_config_set_bool("/gnome-hint/stuff/first_run",FALSE);
		gnome_config_sync();
	}

	draw_on_canvas(canvas, is_fortune, is_motd, hint);
	
	gtk_widget_show_all(win);

	gtk_main();

	return 0;
}
