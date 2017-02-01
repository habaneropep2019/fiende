/* GNOME Help Browser */
/* Copyright (C) 1998 Red Hat Software, Inc    */
/* Written by Michael Fulbright and Marc Ewing */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.
*/

#include <config.h>
#include <gnome.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <orb/orbit.h>
#include <libgnorba/gnorba.h>
#include <libgnomeui/gnome-window-icon.h>
#include <errno.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "window.h"
#include "history.h"
#include "bookmarks.h"
#include "toc2.h"
#include "cache.h"
#include "help-browser.h"

#ifdef HELP_USE_GTKHTML
#include <gtkhtml/gtkhtml.h>
#endif

extern char *program_invocation_name;

#define NAME "GnomeHelp"
#define HELP_VERSION "0.4"


static void aboutCallback(HelpWindow win);
static void newWindowCallback(HelpWindow win);
static void closeWindowCallback(HelpWindow win);
static void setCurrentCallback(HelpWindow win);
static void configCallback(HelpWindow win);

static void historyCallback(gchar *ref);
static void bookmarkCallback(gchar *ref);

void messageHandler(gchar *s);
void warningHandler(gchar *s);
void errorHandler(gchar *s);
void setErrorHandlers(void);
HelpWindow makeHelpWindow(gint x, gint y, gint w, gint h);
static void initConfig(void);
static void saveConfig(void);
static GnomeClient *newGnomeClient(void);

static void configApply(GtkWidget *w, int page, GtkWidget *window);
static int configCancel(GtkWidget *w, GtkWidget *window);

void Exception( CORBA_Environment*);
help_browser_simple_browser 
  impl_help_browser_simple_browser__create(PortableServer_POA poa,
					   HelpWindow window, 
					   CORBA_Environment * ev);
void destroy_server(HelpWindow win);

/* MANPATH should probably come from somewhere */
#define DEFAULT_MANPATH   GNOME_PREFIX "/man:/usr/man:/usr/share/man:/usr/local/man:/usr/X11R6/man"
#define DEFAULT_INFOPATH  GNOME_PREFIX "/info:/usr/info:/usr/share/info:/usr/local/info"
/* GHELPPATH probably needs some automatic additions inside toc */
#define DEFAULT_GHELPPATH GNOME_PREFIX "/share/gnome/help:" \
                          "/opt/gnome/share/gnome/help:" \
                          "/usr/local/share/gnome/help:" \
			  "/usr/local/gnome/share/gnome/help:" \
			  "/usr/share/gnome/help" 
#define DEFAULT_MEMCACHESIZE "1000000"
#define DEFAULT_HISTORYLENGTH "1000"
#define HELP_BROWSER_RC_DIR ".gnome-help-browser"
#define DEFAULT_HISTORYFILE "history"
#define DEFAULT_CACHEFILE "cache"
#define DEFAULT_BOOKMARKFILE "bookmarks"

/* Config data */			  
static gchar *manPath;			  
static gchar *infoPath;			  
static gchar *ghelpPath;
static gint memCacheSize;
static gint historyLength;
static gchar *historyFile;
static gchar *cacheFile;
static gchar *bookmarkFile;
static GnomeClient *smClient;

/* A few globals */
static Toc toc;
static History historyWindow;
static DataCache cache;
static Bookmarks bookmarkWindow;
static gint debugLevel = 0;

GList *windowList = NULL;

/* This is the name of the help URL from the command line.  */
static gchar *helpURL = NULL;

/* requested size/positiion of initial help browser */
static gint defposx=0;
static gint defposy=0;
static gint defwidth =0;
static gint defheight=0;

/* Argument parsing.  */
static struct poptOption options[] = 
{
  {NULL, 'x', POPT_ARG_INT, &defposx, 0, N_("X position of window"), N_("X")},
  {NULL, 'y', POPT_ARG_INT, &defposy, 0, N_("Y position of window"), N_("Y")},
  {NULL, 'w', POPT_ARG_INT, &defwidth, 0, N_("Width of window"), N_("WIDTH")},
  {NULL, 'h', POPT_ARG_INT, &defheight, 0, N_("Height of window"), N_("HEIGHT")},
  {"debug", 'd', POPT_ARG_INT, &debugLevel, 0, N_("Debug level"), NULL},
  {NULL, '\0', 0, NULL, 0}
};

#ifdef UGLY_LE_HACK

void
show_requested_url(char *url)
{
  helpWindowShowURL(ahelpwindow, url, TRUE, TRUE);
}
#endif

void Exception( CORBA_Environment* ev )
{
  switch( ev->_major )
    {
    case CORBA_SYSTEM_EXCEPTION:
      g_log("GNOME Help", G_LOG_LEVEL_DEBUG, "CORBA system exception %s.\n",
	    CORBA_exception_id(ev));
      exit ( 1 );
    case CORBA_USER_EXCEPTION:
      g_log("GNOME Help", G_LOG_LEVEL_DEBUG, "CORBA user exception: %s.\n",
	    CORBA_exception_id( ev ) );
      exit ( 1 );
    default:
      break;
    }
}

int
main(int argc, char *argv[])
{
    HelpWindow                  window = 0;
    gchar                       buf[BUFSIZ];
    CORBA_ORB                   orb;
    CORBA_Environment           ev;
    PortableServer_POA          root_poa;
    PortableServer_POAManager   pm;
    help_browser_simple_browser browser_object;
    const gchar **leftovers;
    poptContext ctx;
    
    
    CORBA_exception_init(&ev);
    
    /* Initialize the i18n stuff */
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    orb = gnome_CORBA_init_with_popt_table(NAME, VERSION, &argc, argv,
					   options, 0, &ctx, GNORBA_INIT_SERVER_FUNC, &ev);

#ifdef HELP_USE_GTKHTML
#ifdef GTKHTML_HAVE_GCONF
    gconf_init (argc, argv, NULL);
#endif /* GTKHTML_HAVE_GCONF */
#endif /* HELP_USE_GTKHTML*/

    gdk_rgb_init ();
 
    gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
    gtk_widget_set_default_visual (gdk_rgb_get_visual ());

    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-help.png");

    leftovers = poptGetArgs(ctx);
    if(leftovers && leftovers[0]) {
      const char *url = leftovers[0];
      if(*url == '/')
	helpURL = g_strconcat ("file:", url, NULL);
      else
	helpURL = g_strdup (url);
    }
    poptFreeContext(ctx);

    Exception(&ev);
    
    root_poa = (PortableServer_POA) 
      CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
    Exception(&ev);
    
/* enable session management here */
    smClient = newGnomeClient();

    initConfig();

    setErrorHandlers();
	
    if (smClient && smClient->client_id)
	    g_message("SM client ID is %s", smClient->client_id );
    else
	    g_message("Session Manager not detected");

    g_snprintf(buf, sizeof(buf), "%s/%s", HELP_BROWSER_RC_DIR, historyFile);
    historyWindow = newHistory(historyLength, historyCallback, buf);

    g_snprintf(buf, sizeof(buf), "%s/%s", HELP_BROWSER_RC_DIR, cacheFile);
    cache = newDataCache(memCacheSize, 0, (GCacheDestroyFunc)g_free, buf);

    toc = newToc(manPath, infoPath, ghelpPath);

    g_snprintf(buf, sizeof(buf), "%s/%s", HELP_BROWSER_RC_DIR, bookmarkFile);
    bookmarkWindow = newBookmarks(bookmarkCallback, NULL, buf);

    window = makeHelpWindow(defposx, defposy, defwidth, defheight);
    if (!helpURL)
      helpURL = "toc:";

    gtk_widget_show (helpWindowGetAppWindow (window));
    helpWindowShowURL(window, helpURL, TRUE, TRUE);

    browser_object =  impl_help_browser_simple_browser__create(root_poa, 
							       window, &ev);
    Exception(&ev);
    
    pm = PortableServer_POA__get_the_POAManager(root_poa, &ev);
    Exception(&ev);

    PortableServer_POAManager_activate(pm, &ev);
    Exception(&ev);

    goad_server_register(NULL, browser_object, "gnome-help-browser", "object", &ev);
#if 0
    /* define if you need to debug output, else squelch it */
#ifdef CREATE_LOGFILE
    output_fd = open("/tmp/gnome-help-browser.log", O_CREAT | O_WRONLY
		     | O_APPEND, 0666);
#else
    output_fd = open("/dev/null", O_WRONLY, 0666);
#endif
    setvbuf(stderr, 0, _IOLBF, 0);
    setvbuf(stdout, 0, _IOLBF, 0);
    dup2(output_fd, fileno(stdout));
    dup2(output_fd, fileno(stderr));
    close(output_fd);
#endif
    gtk_main();

    saveHistory(historyWindow);
    saveBookmarks(bookmarkWindow);
    saveCache(cache);

    goad_server_unregister(NULL, "help-browser", "object", &ev);

    Exception(&ev);
    
    return 0;
}

HelpWindow
makeHelpWindow(gint x, gint y, gint w, gint h)
{
    HelpWindow window;
    
    window = helpWindowNew(NAME, defposx, defposy, defwidth, defheight,
			   aboutCallback, newWindowCallback,
			   closeWindowCallback, setCurrentCallback,
			   configCallback);
    helpWindowSetHistory(window, historyWindow);
    helpWindowSetCache(window, cache);
    helpWindowSetToc(window, toc);
    helpWindowSetBookmarks(window, bookmarkWindow);

    windowList = g_list_append(windowList, window);
    return window;
}

/********************************************************************/

/* Callbacks */

static void
setCurrentCallback(HelpWindow win)
{
  windowList = g_list_remove(windowList, win);
  windowList = g_list_append(windowList, win);
}

static void
closeWindowCallback(HelpWindow win)
{
  destroy_server(win);
  helpWindowClose(win);
  windowList = g_list_remove(windowList, win);
  
  if (!windowList) {
    gtk_main_quit();
  }
}

static void
newWindowCallback(HelpWindow win)
{
    HelpWindow window;
    
    window = makeHelpWindow(0,0,0,0);
    helpWindowShowURL(window, "toc:", TRUE, TRUE);
    gtk_widget_show (helpWindowGetAppWindow (window));
}

static void
historyCallback (gchar *ref)
{
    g_message("HISTORY: %s", ref);
    helpWindowShowURL((HelpWindow)g_list_last(windowList)->data, 
		      ref, TRUE, TRUE);
}

static void
bookmarkCallback (gchar *ref)
{
    g_message("BOOKMARKS: %s", ref);
    helpWindowShowURL((HelpWindow)g_list_last(windowList)->data,
		      ref, TRUE, TRUE);
}

static void
aboutCallback (HelpWindow win)
{
	static GtkWidget *about = NULL;
	const gchar *authors[] = {
		"Mike Fulbright",
		"Marc Ewing",
		"Jacob Berkman",
		NULL
	};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	
	}
	about = gnome_about_new ( _("Gnome Help Browser"), HELP_VERSION,
				  _("Copyright (c) 1998 Red Hat Software, Inc."),
				  authors,
				  _("GNOME Help Browser allows easy access to "
				  "various forms of documentation on your "
				  "system"),
				  NULL);
	gtk_signal_connect (GTK_OBJECT (about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
	gtk_widget_show (about);
	
	return;
}

/**********************************************************************/

/* Error handlers */

void messageHandler(gchar *s)
{
    if (debugLevel > 2) {
	printf("M: %s\n", s);
    }
}

void errorHandler(gchar *s)
{
    if (debugLevel > 0) {
	fprintf(stderr, "E: %s\n", s);
    }
}

void warningHandler(gchar *s)
{
    if (debugLevel > 1) {
	printf("W: %s\n", s);
    }
}

void setErrorHandlers(void)
{
    g_set_error_handler((GErrorFunc) errorHandler);
    g_set_warning_handler((GErrorFunc) warningHandler);
    g_set_message_handler((GErrorFunc) messageHandler);
    g_set_print_handler((GErrorFunc) printf);
}


/**********************************************************************/

/* Sesssion stuff */

static int
save_state (GnomeClient        *client,
            gint                phase,
            GnomeRestartStyle   save_style,
            gint                shutdown,
            GnomeInteractStyle  interact_style,
            gint                fast,
            gpointer            client_data)
{
        gchar *argv[20];
	gchar *s;
        gint i = 0, j;
	HelpWindow win;
	GtkWidget  *appwin;
        gint xpos, ypos;
	gint xsize, ysize;


	/* for now we worry about first window in window list */
	/* but we want to save state for all window browsers  */
	/* in the future                                      */
	if (!windowList)
		return FALSE;
	win = windowList->data;
	if (!win)
		return FALSE;
	appwin = helpWindowGetAppWindow(win);
	if (!appwin)
		return FALSE;

	g_message("Saving myself");
        gdk_window_get_origin(appwin->window, &xpos, &ypos);
	gdk_window_get_size(appwin->window, &xsize, &ysize);

        argv[i++] = program_invocation_name;
        argv[i++] = (char *) "-x";
	s = alloca(20);
	g_snprintf(s, 20, "%d", xpos);
        argv[i++] = s;
        argv[i++] = (char *) "-y";
	s = alloca(20);
	g_snprintf(s, 20, "%d", ypos);
        argv[i++] = s;
        argv[i++] = (char *) "-w";
	s = alloca(20);
	g_snprintf(s, 20, "%d", xsize);
        argv[i++] = s;
        argv[i++] = (char *) "-h";
	s = alloca(20);
	g_snprintf(s, 20, "%d", ysize);
        argv[i++] = s;
	s = alloca(512);
	g_snprintf(s, 512, "%s", helpWindowHumanRef(win));
	argv[i++] = s;

	s = alloca(512);
	g_snprintf(s, 512, "restart command is");
	for (j=0; j<i; j++) {
		strncat(s, " ", 511);
		strncat(s, argv[j], 511);
	}
	g_message("%s", s);

        gnome_client_set_restart_command (client, i, argv);
        /* i.e. clone_command = restart_command - '--sm-client-id' */
        gnome_client_set_clone_command (client, 0, NULL);

        return TRUE;
}


/**********************************************************************/

static void
session_die (gpointer client_data)
{
	gtk_main_quit ();
}

/* Session Management stuff */
static GnomeClient
*newGnomeClient()
{
	gchar *buf[1024];

	GnomeClient *client;

        client = gnome_master_client ();

	if (!client)
		return NULL;

	getcwd((char *)buf,sizeof(buf));
	gnome_client_set_current_directory(client, (char *)buf);

        gtk_object_ref(GTK_OBJECT(client));
        gtk_object_sink(GTK_OBJECT(client));

        gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
                            GTK_SIGNAL_FUNC (save_state), NULL);
        gtk_signal_connect (GTK_OBJECT (client), "die",
                            GTK_SIGNAL_FUNC (session_die), NULL);
        return client;
}

/**********************************************************************/

/* Configure stuff */

static void initConfig(void)
{
    gchar buf[BUFSIZ];
    struct stat statb;
    gchar *s;
    
    manPath = gnome_config_get_string("/" NAME "/paths/manpath="
				      DEFAULT_MANPATH);
    infoPath = gnome_config_get_string("/" NAME "/paths/infopath="
				       DEFAULT_INFOPATH);
    ghelpPath = gnome_config_get_string("/" NAME "/paths/ghelppath="
					DEFAULT_GHELPPATH);

    memCacheSize = gnome_config_get_int("/" NAME "/cache/memsize="
					DEFAULT_MEMCACHESIZE);
    cacheFile = gnome_config_get_string("/" NAME "/cache/file="
					DEFAULT_CACHEFILE);
    historyLength = gnome_config_get_int("/" NAME "/history/length="
					 DEFAULT_HISTORYLENGTH);
    historyFile = gnome_config_get_string("/" NAME "/history/file="
					  DEFAULT_HISTORYFILE);
    bookmarkFile = gnome_config_get_string("/" NAME "/bookmarks/file="
					   DEFAULT_BOOKMARKFILE);

    /* Clean up from when we moved to require .gnome-help-browser */

    if (! strncmp(cacheFile, HELP_BROWSER_RC_DIR,
		  strlen(HELP_BROWSER_RC_DIR))) {
	s = cacheFile;
	cacheFile = g_strdup(cacheFile + strlen(HELP_BROWSER_RC_DIR) + 1);
	g_free(s);
    }
    
    if (! strncmp(historyFile, HELP_BROWSER_RC_DIR,
		  strlen(HELP_BROWSER_RC_DIR))) {
	s = historyFile;
	historyFile = g_strdup(historyFile + strlen(HELP_BROWSER_RC_DIR) + 1);
	g_free(s);
    }
    
    if (! strncmp(bookmarkFile, HELP_BROWSER_RC_DIR,
		  strlen(HELP_BROWSER_RC_DIR))) {
	s = bookmarkFile;
	bookmarkFile = g_strdup(bookmarkFile +
				strlen(HELP_BROWSER_RC_DIR) + 1);
	g_free(s);
    }

    errno = 0;
    g_snprintf(buf, sizeof(buf), "%s/%s",
	       g_getenv("HOME"), HELP_BROWSER_RC_DIR);
    if (stat(buf, &statb)) {
	if (mkdir(buf, 0755)) {
	    g_error("Unable to mkdir $HOME/%s: %s",
		    HELP_BROWSER_RC_DIR, strerror(errno));
	}
    }
    
    saveConfig();
}

static void saveConfig(void)
{
    gnome_config_set_string("/" NAME "/paths/manpath", manPath);
    gnome_config_set_string("/" NAME "/paths/infopath", infoPath);
    gnome_config_set_string("/" NAME "/paths/ghelppath", ghelpPath);
    gnome_config_set_int("/" NAME "/cache/memsize", memCacheSize);
    gnome_config_set_string("/" NAME "/cache/file", cacheFile);
    gnome_config_set_int("/" NAME "/history/length", historyLength);
    gnome_config_set_string("/" NAME "/history/file", historyFile);
    gnome_config_set_string("/" NAME "/bookmarks/file", bookmarkFile);
    
    gnome_config_sync();
}

/* Given the current config settings, reset all the widgets and stuff */
static void setConfig(void)
{
    reconfigHistory(historyWindow, historyLength,
		    historyCallback, historyFile);
    reconfigDataCache(cache, memCacheSize, 0, (GCacheDestroyFunc)g_free,
		      cacheFile);
    /*toc = newToc(manPath, infoPath, ghelpPath, tocCallback);*/
    reconfigBookmarks(bookmarkWindow, bookmarkCallback, NULL, bookmarkFile);
}

/* XXX This stuff should all be abstracted somewhere else. */
/* There are globals around where there shouldn't be.      */

#define CONFIG_INT  1
#define CONFIG_TEXT 2

struct _config_entry {
    gchar *name;
    gint type;
    gpointer var;
    GtkWidget *entry;
};

GtkWidget *configWindow = NULL;
struct _config_entry config_sizes [] = {
    { N_("History size"),    CONFIG_INT, &historyLength, NULL },
    { N_("History file"),    CONFIG_TEXT, &historyFile, NULL },
    { N_("Cache size"),      CONFIG_INT, &memCacheSize, NULL },
    { N_("Cache file"),      CONFIG_TEXT, &cacheFile, NULL },
    { N_("Bookmark file"),   CONFIG_TEXT, &bookmarkFile, NULL },
    
    { NULL, 0, NULL }
};

struct _config_entry config_paths [] = {
    { N_("Man Path"),        CONFIG_TEXT, &manPath, NULL },
    { N_("Info Path"),       CONFIG_TEXT, &infoPath, NULL },
    { N_("GNOME Help Path"), CONFIG_TEXT, &ghelpPath, NULL },
    
    { NULL, 0, NULL }
};

static void
signal_change (GtkWidget *widget, GnomePropertyBox *property)
{
	gnome_property_box_changed (property);
}

static void
generateConfigWidgets(GnomePropertyBox *property, struct _config_entry *configs, char *title)
{
    GtkWidget *table, *label, *entry;
    struct _config_entry *p;
    gchar buf[BUFSIZ];
    gint rows;
    
    if (! configs)
	return;

    rows = 0;
    p = configs;
    while (p->name) {
	rows++;
	p++;
    }

    table = gtk_table_new(0, 0, FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD);
    gnome_property_box_append_page (property, table, gtk_label_new (title));

    rows = 0;
    p = configs;
    while (p->name) {
	label = gtk_label_new(_(p->name));
	gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.5);
	gtk_widget_show(label);
	
	entry = gtk_entry_new();
	gtk_widget_show(entry);
	p->entry = entry;

	if (p->type == CONFIG_INT) {
	    g_snprintf(buf, sizeof(buf), "%d", *(gint *)(p->var));
	    gtk_entry_set_text(GTK_ENTRY(entry), buf);
	} else if (p->type == CONFIG_TEXT) {
	    gtk_entry_set_text(GTK_ENTRY(entry), *(gchar **)(p->var));
	}
	gtk_signal_connect (GTK_OBJECT(entry), "changed",
			    GTK_SIGNAL_FUNC(signal_change), property);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, rows, rows + 1,
			 GTK_FILL, GTK_FILL,
			 5, 0);
	gtk_table_attach(GTK_TABLE(table), entry, 1, 2, rows, rows + 1,
			 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			 0, 0);

	rows++;
	p++;
    }
}

static void
configCallback(HelpWindow win)
{
    GtkWidget *window;

    if (configWindow) {
	return;
    }

    /* Main Window */
    window = gnome_property_box_new ();
    gtk_window_set_title(GTK_WINDOW(window), _("Gnome Help Configure"));
    configWindow = window;

    generateConfigWidgets(GNOME_PROPERTY_BOX (window), config_sizes, _("History and cache"));
    generateConfigWidgets(GNOME_PROPERTY_BOX (window), config_paths, _("Paths"));

    gtk_signal_connect(GTK_OBJECT(window), "apply",
		       (GtkSignalFunc)configApply, window);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		       (GtkSignalFunc)configCancel, window);
    gtk_signal_connect(GTK_OBJECT(window), "destroy",
		       (GtkSignalFunc)configCancel, window);

    gtk_widget_show_all(window);
}

static void
load_config_from_widgets (struct _config_entry *p)
{
    gchar *s;
    gint x;
    
    while (p->name) {
	s = gtk_entry_get_text(GTK_ENTRY(p->entry));

	if (p->type == CONFIG_INT) {
	    sscanf(s, "%d", &x);
	    *(gint *)(p->var) = x;
	} else if (p->type == CONFIG_TEXT) {
	    if (*(gchar **)(p->var)) {
		g_free(*(gchar **)(p->var));
	    }
	    *(gchar **)(p->var) = g_strdup(s);;
	}
	p++;
    }
}

static void
configApply(GtkWidget *w, int page, GtkWidget *window)
{
    if (page != -1)
        return;
    load_config_from_widgets (config_paths);
    load_config_from_widgets (config_sizes);

    saveConfig();
    setConfig();
}

static int
configCancel(GtkWidget *w, GtkWidget *window)
{
    configWindow = NULL;
    return FALSE;
}
