/* This file should encapsulate all the HTML widget functionality. */
/* No other files should be accessing HTML widget functions!       */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gnome.h>
#include <libgnomeui/gnome-app.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnome/gnome-help.h>
#include <gdk/gdkkeysyms.h>
#include "gnome-helpwin.h"

#include "parseUrl.h"
#include "window.h"
#include "history.h"
#include "bookmarks.h"
#include "toc2.h"
#include "transport.h"
#include "docobj.h"
#include "queue.h"
#include "visit.h"
#include "misc.h"

/* Toolbar pixmaps */
#include "contents.xpm"

#ifdef HELP_USE_GTKHTML
#include <gtkhtml/htmlurl.h>
#include <errno.h>
void gtk_html_source (GtkHTML *html, char *url, char *source);
#endif

#define DEFAULT_HEIGHT 500
#define DEFAULT_WIDTH  600


struct _helpWindow {
    /* Main app widget */
    GtkWidget *app;

    /* Toolbar widgets - needed to set button states */
    GtkWidget *tb_back, *tb_forw;

    /* The HTML widget */
    GtkWidget *helpWidget;

    /* The forward/backward queue */
    Queue queue;

    /* The current page reference */
    gchar *currentRef;
    gchar *humanRef;
    gchar *Title;

    gboolean useCache;

    /* The entry box that shows the URL */
    GtkWidget *entryBox;

    /* status bar */
    GtkWidget *appBar;

    /* Bogus widgets used by accel table */
    GtkWidget *accelWidget;
    
    /* Passed to us by the main program */
    HelpWindowCB about_cb;
    HelpWindowCB new_window_cb;
    HelpWindowCB close_window_cb;
    HelpWindowCB set_current_cb;
    HelpWindowCB config_cb;
    History history;
    Toc toc;
    DataCache cache;
    Bookmarks bookmarks;
};


static void makeEntryArea(HelpWindow w);

/* Callbacks */
static void quit_cb(void);
static void about_cb (GtkWidget *w, HelpWindow win);
static void config_cb (GtkWidget *w, HelpWindow win);
static void bookmark_cb (GtkWidget *w, HelpWindow win);
static void close_cb (GtkWidget *w, HelpWindow win);
static gboolean delete_cb (GtkWidget *w, GdkEvent *ev, HelpWindow win);
static void new_window_cb (GtkWidget *w, HelpWindow win);
static void help_forward(GtkWidget *w, HelpWindow win);
static void help_backward(GtkWidget *w, HelpWindow win);
static void help_onhelp(GtkWidget *w, HelpWindow win);
static void help_gotoindex(GtkWidget *w, HelpWindow win);

#ifdef HELP_USE_GTKHTML
static void gtkhtml_activate (GtkWidget *w, const gchar *url, HelpWindow data);
static void gtkhtml_onurl (GtkWidget *w, const gchar *url, HelpWindow data);
static void on_set_base (GtkHTML *html, const gchar *url, gpointer data);
static void gtkhtml_formActivate (GtkWidget *w, const gchar *method,
				  const gchar *url, const gchar *encoding,
				  HelpWindow win);
#else
static void xmhtml_activate(GtkWidget *w, XmHTMLAnchorCallbackStruct *cbs,
			    HelpWindow win);
static void anchorTrack(GtkWidget *w, XmHTMLAnchorCallbackStruct *cbs,
			    HelpWindow win);
static void formActivate(GtkWidget *w, XmHTMLFormCallbackStruct *cbs,
			    HelpWindow win);
#endif

static void reload_page(GtkWidget *w, HelpWindow win);
static void ghelpShowHistory (GtkWidget *w, HelpWindow win);
static void ghelpShowBookmarks (GtkWidget *w, HelpWindow win);
static void entryChanged(GtkWidget *w, HelpWindow win);
static void setCurrent(HelpWindow w);

static void pageUp(GtkWidget *w, HelpWindow win);
static void pageDown(GtkWidget *w, HelpWindow win);
static void spaceUp(GtkWidget *w, HelpWindow win);
static void spaceDown(GtkWidget *w, HelpWindow win);

static void dndDrop(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
		    GtkSelectionData *data, guint info,
		    guint time, HelpWindow win);

static void init_toolbar(HelpWindow w);
static void update_toolbar(HelpWindow w);

#ifdef HELP_USE_GTKHTML
static void url_requested (GtkHTML *html, const char *url, GtkHTMLStream *s, HelpWindow win);
#else
XmImageInfo *load_image(GtkWidget *html_widget, gchar *ref);
#endif


/**********************************************************************/



/**********************************************************************/

/* Menu and toolbar structures */

GnomeUIInfo filemenu[] = {
        GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Window"),
				     N_("Open new browser window"),
				     new_window_cb, NULL),

	GNOMEUIINFO_SEPARATOR,

	{GNOME_APP_UI_ITEM, 
	 N_("_Add Bookmark"), N_("Add bookmark"),
         (gpointer)bookmark_cb, NULL, NULL, 
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE,
	 0, 0, NULL},

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_CLOSE_ITEM(close_cb, NULL),

	GNOMEUIINFO_MENU_EXIT_ITEM(quit_cb, NULL),

	GNOMEUIINFO_END
};

GnomeUIInfo viewmenu[] = {
	{ GNOME_APP_UI_ITEM, N_("_Back"), NULL, help_backward, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BACK, 'B',
	  GDK_SHIFT_MASK, NULL },
	{ GNOME_APP_UI_ITEM, N_("_Forward"), NULL, help_forward, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_FORWARD, 'F',
	  GDK_SHIFT_MASK, NULL },
	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, N_("_Reload"), NULL, reload_page, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REFRESH, 'R',
	  0, NULL },

	GNOMEUIINFO_SEPARATOR,
	{ GNOME_APP_UI_ITEM, N_("_Index"), NULL, help_gotoindex, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 'I',
	  0, NULL },

	GNOMEUIINFO_END
};

/* Add this to gnome-convert 
+    s# (/[a-z]+/[a-z.-/]+)# <A HREF="file:$1">$1</A>#g;
 */

GnomeUIInfo helpmenu[] = {
  
    GNOMEUIINFO_HELP("help-browser"),

    GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb, NULL),

    GNOMEUIINFO_END
};
 
GnomeUIInfo windowmenu[] = {
	{GNOME_APP_UI_ITEM, 
	 N_("_History"), N_("Show History Window"),
         (gpointer)ghelpShowHistory, NULL, NULL, 
	 GNOME_APP_PIXMAP_NONE, NULL,
	 0, 0, NULL},
	{GNOME_APP_UI_ITEM, 
	 N_("_Bookmarks"), N_("Show Bookmarks Window"),
         (gpointer)ghelpShowBookmarks, NULL, NULL, 
	 GNOME_APP_PIXMAP_NONE, NULL,
	 0, 0, NULL},
	 GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu[] = {
        GNOMEUIINFO_MENU_PREFERENCES_ITEM(config_cb, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
    GNOMEUIINFO_MENU_FILE_TREE(filemenu),
    GNOMEUIINFO_SUBTREE(N_("_Window"), windowmenu),
    GNOMEUIINFO_SUBTREE(N_("_View"), viewmenu),
    GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),
    GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
    GNOMEUIINFO_END
};



GnomeUIInfo toolbar[] = {
    GNOMEUIINFO_ITEM_STOCK(N_("Back"), 
			   N_("Go to the previous location in the history list"),
			   help_backward, GNOME_STOCK_PIXMAP_BACK),
    GNOMEUIINFO_ITEM_STOCK(N_("Forward"),
			   N_("Go to the next location in the history list"),
			   help_forward, GNOME_STOCK_PIXMAP_FORWARD),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(N_("Reload"), N_("Reload"), reload_page,
			   GNOME_STOCK_PIXMAP_REFRESH),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(N_("Index"), N_("Show Documentation Index"), 
		     help_gotoindex, GNOME_STOCK_PIXMAP_HOME),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM(N_("History"), N_("Show History Window"),
		     ghelpShowHistory, contents_xpm),
    GNOMEUIINFO_ITEM_STOCK(N_("BMarks"), N_("Show Bookmarks Window"),
		     ghelpShowBookmarks, GNOME_STOCK_PIXMAP_BOOK_OPEN),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(N_("Help"), N_("Help on Help"), help_onhelp, GNOME_STOCK_PIXMAP_HELP),
    GNOMEUIINFO_END
};

/**********************************************************************/


/* stupid function to handle fact XmHTML doesn't un-escape & strings */
/* in URL we get passed back!                                        */
static gchar *
unescape_url ( gchar *ref ) {
    gchar *s, *ns;
    gchar *newref;


    /* go through and replace any of following with unescaped equivs */

    if (!ref)
	return NULL;

    newref = g_strdup (ref);
    for (s=ref, ns=newref; *s; ) {

	if (*s != '&') {
	    *ns = *s;
	    ns++;
	    s++;
	    continue;
	} else {

	    if (!g_strncasecmp (s, "&gt;", 4)) {
		*ns = '>';
		s+=4;
	    } else if (!g_strncasecmp (s, "&lt;", 4)) {
		*ns = '<';
		s+=4;
	    } else if (!g_strncasecmp (s, "&amp;", 5)) {
		*ns = '&';
		s+=5;
	    } else {
		*ns = *s;
		s++;
	    }

	    ns++;
	    continue;
	}
    }

    *ns = '\0';

    return newref;
}


/**********************************************************************/

/* Callbacks */

static void
about_cb (GtkWidget *w, HelpWindow win)
{
    if (win->about_cb)
	(win->about_cb)(win);
}

static void
config_cb (GtkWidget *w, HelpWindow win)
{
    if (win->config_cb)
	(win->config_cb)(win);
}

static void
bookmark_cb (GtkWidget *w, HelpWindow win)
{
    if (win->bookmarks)
	addToBookmarks(win->bookmarks, win->humanRef, win->Title);
}

static void
new_window_cb (GtkWidget *w, HelpWindow win)
{
    if (win->new_window_cb)
	(win->new_window_cb)(win);
}

static gboolean
delete_cb (GtkWidget *w, GdkEvent *ev, HelpWindow win)
{
    if (win->close_window_cb)
	(win->close_window_cb)(win);
    return FALSE;
}

static void
close_cb (GtkWidget *w, HelpWindow win)
{
    gtk_widget_destroy (win->app);
    win->app = NULL;
    if (win->close_window_cb)
	(win->close_window_cb)(win);
}

static void
ghelpShowHistory (GtkWidget *w, HelpWindow win)
{
    showHistory(win->history);
}

static void
ghelpShowBookmarks (GtkWidget *w, HelpWindow win)
{
    showBookmarks(win->bookmarks);
}

static void
quit_cb (void)
{
    gtk_main_quit ();
    return;
}

#ifdef HELP_USE_GTKHTML
static void
gtkhtml_activate (GtkWidget *w, const gchar *url, HelpWindow win)
{
    g_message("TAG CLICKED: %s", url);

    helpWindowShowURL(win, url, TRUE, TRUE);
}

static void
gtkhtml_onurl (GtkWidget *html, const gchar *url, HelpWindow win)
{
	gnome_appbar_set_status (GNOME_APPBAR (win->appBar), url ? url : "");
}

static void
gtkhtml_formActivate (GtkWidget *w, const gchar *method,
		      const gchar *action, const gchar *encoding,
		      HelpWindow win)
{
	GString *tmpstr = g_string_new (action);

	g_print("submitting '%s' to '%s' using method '%s'\n", encoding, action, method);

	if(g_strcasecmp(method, "GET") == 0) {

		tmpstr = g_string_append_c (tmpstr, '?');
		tmpstr = g_string_append (tmpstr, encoding);
		
		helpWindowShowURL ((HelpWindow)w, tmpstr->str, FALSE, TRUE);
		
		g_string_free (tmpstr, TRUE);
	} else {
		g_warning ("Unsupported submit method '%s'\n", method);
	}
}

#else /* !HELP_USE_GTKHTML */

static void
xmhtml_activate(GtkWidget *w, XmHTMLAnchorCallbackStruct *cbs, HelpWindow win)
{


    g_message("TAG CLICKED: %s", cbs->href);
    
    if (cbs->href) {
	gchar *s;

	s = unescape_url(cbs->href);
	
	helpWindowShowURL(win, s, TRUE, TRUE);
	
	g_free(s);
    }
}

static void
anchorTrack(GtkWidget *w, XmHTMLAnchorCallbackStruct *cbs, HelpWindow win)
{
	gnome_appbar_pop(GNOME_APPBAR(win->appBar));
	if (cbs->href) {
	    gchar *s;

	    s = unescape_url(cbs->href);
	    gnome_appbar_push(GNOME_APPBAR(win->appBar), s);
	    g_free (s);
	}
}


static void
formActivate(GtkWidget *w, XmHTMLFormCallbackStruct *cbs, HelpWindow win)
{
	gint i;

	g_message("Recvieved a GTK_XMHTML_FORM event, enctype = %s",
		  cbs->enctype);

	g_message("There are %d components.", cbs->ncomponents);
	for (i=0; i<cbs->ncomponents; i++) {
		g_message("Component %d: name = %s, value = %s",
			  i,
			  cbs->components[i].name,
			  cbs->components[i].value);
	}
}
#endif /* HELP_USE_GTKHTML */

static void
help_forward(GtkWidget *w, HelpWindow win)
{
	gchar *ref;
	gint pos;

	if (!(ref = queue_next(win->queue, &pos)))
		return;

	visitURL(win, ref, TRUE, FALSE, FALSE );
	queue_move_next(win->queue);
	
	g_message("jump to line: %d", pos);
	gnome_helpwin_jump_to_line(GNOME_HELPWIN(win->helpWidget), pos);

	update_toolbar(win);
	setCurrent(win);
}

static void
help_backward(GtkWidget *w, HelpWindow win)
{
	gchar *ref;
	gint pos;

	if (!(ref = queue_prev(win->queue, &pos)))
		return;

	visitURL(win, ref, TRUE, FALSE, FALSE);
	queue_move_prev(win->queue);
	
	g_message("jump to line: %d", pos);
	gnome_helpwin_jump_to_line(GNOME_HELPWIN(win->helpWidget), pos);

	update_toolbar(win);
	setCurrent(win);
}

static void
help_onhelp(GtkWidget *w, HelpWindow win)
{
	helpWindowShowURL(win, "ghelp:help-browser", TRUE, TRUE);
}

static void
help_gotoindex(GtkWidget *w, HelpWindow win)
{
	helpWindowShowURL(win, "toc:", TRUE, FALSE);
}



static void
reload_page(GtkWidget *w, HelpWindow win)
{
    gchar *s;
    gchar buf[BUFSIZ];
	
    /* Do a little shorthand processing */
    s = win->humanRef;
    if (!s || *s == '\0') {
	return;
    } else if (*s == '/') {
	g_snprintf(buf, sizeof(buf), "file:%s", s);
    } else {
	g_snprintf(buf, sizeof(buf), "%s", s);
    }
    
    g_message("RELOAD PAGE: %s", buf);
    /* make html widget believe we want to reload */
#ifdef HELP_USE_GTKHTML
    gtk_html_source (GTK_HTML (win->helpWidget), "toc:wanda", "<BODY>Hi</BODY>");
#else
    gtk_xmhtml_source(GTK_XMHTML(win->helpWidget), "<BODY>Hi</BODY>");
#endif
    helpWindowShowURL(win, buf, FALSE, FALSE);
}	

static void
entryChanged(GtkWidget *w, HelpWindow win)
{
    gchar *s;
    gchar buf[BUFSIZ];
    
    g_message("ENTRY BOX: %s", gtk_entry_get_text(GTK_ENTRY(w)));

    /* Do a little shorthand processing */
    s = gtk_entry_get_text(GTK_ENTRY(w));
    if (*s == '/') {
	g_snprintf(buf, sizeof(buf), "file:%s", s);
    } else {
	g_snprintf(buf, sizeof(buf), "%s", s);
    }
    
    helpWindowShowURL(win, buf, TRUE, TRUE);
}

/**********************************************************************/



/**********************************************************************/

/* Misc static routines */

static void
setCurrent(HelpWindow w)
{
    if (w->set_current_cb) {
	(w->set_current_cb)(w);
    }
}

static void
init_toolbar(HelpWindow w)
{
	gnome_app_create_toolbar_with_data(GNOME_APP(w->app), toolbar, w);

	w->tb_back = toolbar[0].widget;
	w->tb_forw = toolbar[1].widget;
	
	update_toolbar(w);
}

static void
update_toolbar(HelpWindow w)
{
	if (w->tb_back)
		gtk_widget_set_sensitive(w->tb_back, queue_isprev(w->queue));
	if (w->tb_forw)
		gtk_widget_set_sensitive(w->tb_forw, queue_isnext(w->queue));
}

static void
makeEntryArea(HelpWindow w)
{
    GtkWidget *dock, *hbox, *label, *entry;

    dock = gnome_dock_item_new ("gnome-help-browser-toolbar1",
				(GNOME_DOCK_ITEM_BEH_EXCLUSIVE
				 | GNOME_DOCK_ITEM_BEH_NEVER_VERTICAL));
    gtk_widget_show (dock);
    
    hbox = gtk_hbox_new(FALSE, 2);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
    gtk_widget_show(hbox);
    
    label = gtk_label_new(_("Location: "));
    gtk_widget_show(label);
    
    entry = gnome_entry_new(NULL);
    gtk_widget_show(entry);

    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(entry)->entry),
		       "activate", (GtkSignalFunc)entryChanged, w);

    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (dock), hbox);
    gnome_dock_add_item (GNOME_DOCK (GNOME_APP (w->app)->dock),
			 GNOME_DOCK_ITEM (dock), GNOME_DOCK_TOP, 1, 0, 0, FALSE);
    w->entryBox = GTK_COMBO(entry)->entry;
}


/**********************************************************************/


/**********************************************************************/

/* Public functions */

void
helpWindowQueueMark(HelpWindow w)
{
    gint pos;

    pos = gnome_helpwin_get_line(GNOME_HELPWIN(w->helpWidget));
    g_message("get_line = %d", pos);
    
    queue_mark_current(w->queue, pos ? pos : 1);
}

void
helpWindowQueueAdd(HelpWindow w, gchar *ref)
{
    queue_add(w->queue, ref, 1);
}

gchar
*helpWindowCurrentRef(HelpWindow w)
{
    return w->currentRef;
}

gchar
*helpWindowHumanRef(HelpWindow w)
{
    return w->humanRef;
}

void
helpWindowHistoryAdd(HelpWindow w, gchar *ref)
{
    addToHistory(w->history, ref);
}

void
helpWindowHTMLSource(HelpWindow w, gchar *s, gint len,
		     gchar *ref, gchar *humanRef)
{
    gchar *buf=NULL;

    /* First set the current ref (it may be used to load images) */
    g_free (w->currentRef);

    /* It's important to set this first because it used is to */
    /* resolve relative refs for images.                      */
    w->currentRef = g_strdup(ref);
    
    g_free (w->humanRef);

    w->humanRef = g_strdup(humanRef);

    /* Load it up */
    buf = g_malloc(len + 1);
    memcpy(buf, s, len);
    buf[len] = '\0';
#ifdef HELP_USE_GTKHTML
    on_set_base (NULL, w->currentRef, NULL);
    gtk_html_source (GTK_HTML (w->helpWidget), w->humanRef, buf);
#else
    gtk_xmhtml_source(GTK_XMHTML(w->helpWidget), buf);
#endif
    g_free(buf);
    gtk_entry_set_text(GTK_ENTRY(w->entryBox), humanRef);

    if (w->Title)
	g_free(w->Title);

#ifdef HELP_USE_GTKHTML
    buf = gtk_html_get_title (GTK_HTML (w->helpWidget));
#else
    buf = XmHTMLGetTitle((GTK_WIDGET(w->helpWidget)));
#endif
    if (!buf)
	w->Title = g_strdup("");
    else
	w->Title = g_strdup(buf);

    g_message("Title is ->%s<-",w->Title);
}

void
helpWindowJumpToAnchor(HelpWindow w, gchar *s)
{
    gnome_helpwin_jump_to_anchor(GNOME_HELPWIN(w->helpWidget), s);
}

void
helpWindowJumpToLine(HelpWindow w, gint n)
{
    gnome_helpwin_jump_to_line(GNOME_HELPWIN(w->helpWidget), n);
}

void
helpWindowClose(HelpWindow win)
{
    if (win->currentRef)
	g_free(win->currentRef);
    win->currentRef = NULL;
    if (win->humanRef)
	g_free(win->humanRef);
    win->humanRef = NULL;
    queue_free(win->queue);
    win->queue = NULL;
    /*
      gtk_widget_destroy(win->accelWidget);
    */
    g_free(win);
}

static void init_accel(HelpWindow win)
{
    static gint page_up_signal = 0;
    static gint page_down_signal = 0;
    static gint up_signal = 0;
    static gint down_signal = 0;

#ifdef HELP_USE_GTKHTML
    g_message ("Skipping accel init...");
#else
    GtkAccelGroup *accel_group = gtk_accel_group_get_default();

    if(!page_up_signal) {
      page_up_signal = gtk_object_class_user_signal_new 
	(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb)->klass,
	 "page_up",
	 GTK_RUN_FIRST,
	 gtk_marshal_NONE__NONE,
	 GTK_TYPE_NONE, 0);
      page_down_signal = gtk_object_class_user_signal_new
	(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb)->klass,
	 "page_down",
	 GTK_RUN_FIRST,
	 gtk_marshal_NONE__NONE,
	 GTK_TYPE_NONE, 0);
      up_signal = gtk_object_class_user_signal_new
	(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb)->klass,
	 "up",
	 GTK_RUN_FIRST,
	 gtk_marshal_NONE__NONE,
	 GTK_TYPE_NONE, 0);
      down_signal = gtk_object_class_user_signal_new
	(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb)->klass,
	 "down",
	 GTK_RUN_FIRST,
	 gtk_marshal_NONE__NONE,
	 GTK_TYPE_NONE, 0);
    }
    gtk_signal_connect(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb), 
		       "page_up", 
		       GTK_SIGNAL_FUNC(pageUp), win);
    gtk_signal_connect(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb), 
		       "page_down", 
		       GTK_SIGNAL_FUNC(pageDown), win);
    gtk_signal_connect(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb), 
		       "up", 
		       GTK_SIGNAL_FUNC(spaceUp), win);
    gtk_signal_connect(GTK_OBJECT(GTK_XMHTML (win->helpWidget)->html.vsb), 
		       "down", 
		       GTK_SIGNAL_FUNC(spaceDown), win);
    gtk_widget_add_accelerator(GTK_XMHTML (win->helpWidget)->html.vsb, 
			       "page_up", accel_group, 
			       'b', 0, 0);
    gtk_widget_add_accelerator(GTK_XMHTML (win->helpWidget)->html.vsb, 
			       "page_down", accel_group, 
			       ' ', 0, 0);

    gtk_widget_add_accelerator(GTK_XMHTML (win->helpWidget)->html.vsb, 
			       "page_up", accel_group, 
			       GDK_Page_Up, 0, 0);
    gtk_widget_add_accelerator(GTK_XMHTML (win->helpWidget)->html.vsb, 
			       "page_down", accel_group, 
			       GDK_Page_Down, 0, 0);
    gtk_widget_add_accelerator(GTK_XMHTML (win->helpWidget)->html.vsb, 
			       "up", accel_group, 
			       GDK_Up, 0, 0);
    gtk_widget_add_accelerator(GTK_XMHTML (win->helpWidget)->html.vsb, 
			       "down", accel_group, 
			       GDK_Down, 0, 0);
    gtk_widget_add_accelerator(win->entryBox, 
			       "grab_focus", accel_group, 
			       'g', 0, 0);
#endif
}

static void pageUp(GtkWidget *w, HelpWindow win)
{
#ifdef HELP_USE_GTKHTML
    g_message ("page up!");
#else
    GtkAdjustment *adj;

    adj = GTK_ADJUSTMENT(GTK_XMHTML(win->helpWidget)->vsba);
    gtk_adjustment_set_value(adj, adj->value - (adj->page_size));
#endif
}

static void pageDown(GtkWidget *w, HelpWindow win)
{
#ifdef HELP_USE_GTKHTML
    g_message ("page down!");
#else
    GtkAdjustment *adj;

    adj = GTK_ADJUSTMENT(GTK_XMHTML(win->helpWidget)->vsba);
    gtk_adjustment_set_value(adj, adj->value + (adj->page_size));
#endif
}

static void spaceUp(GtkWidget *w, HelpWindow win)
{
#ifdef HELP_USE_GTKHTML
    g_message ("space up");
#else
    GtkAdjustment *adj;

    adj = GTK_ADJUSTMENT(GTK_XMHTML(win->helpWidget)->vsba);
    gtk_adjustment_set_value(adj, adj->value - (adj->step_increment));
#endif
}

static void spaceDown(GtkWidget *w, HelpWindow win)
{
#ifdef HELP_USE_GTKHTML
    g_message ("space down");
#else
    GtkAdjustment *adj;

    adj = GTK_ADJUSTMENT(GTK_XMHTML(win->helpWidget)->vsba);
    gtk_adjustment_set_value(adj, adj->value + (adj->step_increment));
#endif
}

static void dndDrop(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
		    GtkSelectionData *data, guint info,
		    guint time, HelpWindow win)
{
    if (data->data) {
	GList *urls = gnome_uri_list_extract_uris ((gchar *)data->data);

	if (urls)
	    helpWindowShowURL(win, (char *)urls->data, TRUE, TRUE);
	
	gnome_uri_list_free_strings (urls);
    }
}

HelpWindow
helpWindowNew(gchar *name,
	      gint x, gint y, gint width, gint height,
	      HelpWindowCB about_callback,
	      HelpWindowCB new_window_callback,
	      HelpWindowCB close_window_callback,
	      HelpWindowCB set_current_callback,
	      HelpWindowCB config_callback)
{
        HelpWindow w;
	GtkWidget *vbox;
	static GtkTargetEntry target_table[] = {
		{ "text/uri-list", 0, 0 },
	};

	w = (HelpWindow)g_malloc(sizeof(*w));

	w->queue= queue_new();
	w->about_cb = about_callback;
	w->new_window_cb = new_window_callback;
	w->close_window_cb = close_window_callback;
	w->set_current_cb = set_current_callback;
	w->config_cb = config_callback;
	w->history = NULL;
	w->bookmarks = NULL;
	w->cache = NULL;
	w->currentRef = NULL;
	w->humanRef = NULL;
	w->Title    = NULL;

	w->app = gnome_app_new (name, _("Gnome Help Browser"));
	gtk_window_set_wmclass (GTK_WINDOW (w->app), "GnomeHelpBrowser",
				"GnomeHelpBrowser");

	gtk_signal_connect (GTK_OBJECT (w->app), "delete_event",
			    GTK_SIGNAL_FUNC (delete_cb), w);

	gnome_app_create_menus_with_data(GNOME_APP(w->app), mainmenu, w);

	/* do the toolbar */
	init_toolbar(w);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
	gtk_widget_show(vbox);

	makeEntryArea(w);

	/* make the help window */
	w->helpWidget = gnome_helpwin_new();
#ifdef HELP_USE_GTKHTML
	g_warning ("skipping some xmhtml stuff...");
#else
	gtk_xmhtml_set_anchor_underline_type(GTK_XMHTML(w->helpWidget),
					    GTK_ANCHOR_SINGLE_LINE);
	gtk_xmhtml_set_anchor_buttons(GTK_XMHTML(w->helpWidget), FALSE);
#endif
	gtk_widget_show(w->helpWidget);

	/* add a status bar */
	w->appBar = gnome_appbar_new(FALSE, TRUE,
				     GNOME_PREFERENCES_USER);

	gnome_app_set_statusbar(GNOME_APP(w->app), GTK_WIDGET(w->appBar));
	gnome_app_install_menu_hints(GNOME_APP (w->app), mainmenu);
	
	/* trap clicks on tags so we can stick requested link in browser */
#ifdef HELP_USE_GTKHTML
	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "link_clicked",
			   GTK_SIGNAL_FUNC(gtkhtml_activate), w);

	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "on_url",
			   GTK_SIGNAL_FUNC(gtkhtml_onurl), w);

	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "submit",
					     GTK_SIGNAL_FUNC(gtkhtml_formActivate), w);
	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "set_base",
			   GTK_SIGNAL_FUNC (on_set_base), w);
#else /* !HELP_USE_GTKHTML */
	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "activate",
			   GTK_SIGNAL_FUNC(xmhtml_activate), w);

	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "anchor_track",
					     GTK_SIGNAL_FUNC(anchorTrack), w);

	gtk_signal_connect(GTK_OBJECT(w->helpWidget), "form",
					     GTK_SIGNAL_FUNC(formActivate), w);
#endif /* HELP_USE_GTKHTML */

#ifdef HELP_USE_GTKHTML
	{
		GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
		gtk_widget_show (scrolled_window);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		
		gtk_container_add (GTK_CONTAINER (scrolled_window), w->helpWidget);
		gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
	}
#else
	gtk_box_pack_start(GTK_BOX(vbox), w->helpWidget, TRUE, TRUE, 0);
#endif
	gnome_app_set_contents(GNOME_APP(w->app), vbox);

#ifdef HELP_USE_GTKHTML
	gtk_signal_connect (GTK_OBJECT (w->helpWidget), "url_requested",
			    GTK_SIGNAL_FUNC (url_requested), w);
#else
	/* HACKHACKHACK this will grab images via http */
	gtk_object_set_data(GTK_OBJECT(w->helpWidget), "HelpWindow", w);
	gtk_xmhtml_set_image_procs(GTK_XMHTML(w->helpWidget),
				   (XmImageProc)load_image,
				   NULL,NULL,NULL);
#endif
	/* size should be auto-determined, or read from gnomeconfig() */
	if (width && height)
		gtk_window_set_default_size(GTK_WINDOW(w->app), width, height); 
	else
		gtk_window_set_default_size(GTK_WINDOW(w->app), 
				     DEFAULT_WIDTH, DEFAULT_HEIGHT); 

	if (x != y)
		gtk_widget_set_uposition(GTK_WIDGET(w->app), x, y);

	gtk_window_set_policy(GTK_WINDOW(w->app), TRUE, TRUE, FALSE);

	/* Add accelerators */
	init_accel(w);

#ifdef HELP_USE_GTKHTML
	gtk_signal_connect(GTK_OBJECT(w->helpWidget),
#else
	gtk_signal_connect(GTK_OBJECT(GTK_XMHTML(w->helpWidget)->html.work_area),
#endif
			   "drag_data_received",
			   GTK_SIGNAL_FUNC(dndDrop), w);

#ifdef HELP_USE_GTKHTML
        gtk_drag_dest_set (w->helpWidget,
#else
	gtk_drag_dest_set (GTK_XMHTML(w->helpWidget)->html.work_area,
#endif
			   GTK_DEST_DEFAULT_MOTION |
			   GTK_DEST_DEFAULT_HIGHLIGHT |
			   GTK_DEST_DEFAULT_DROP,
			   target_table, 1,
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);

	return w;
}

void
helpWindowSetHistory(HelpWindow win, History history)
{
    win->history = history;
}

void
helpWindowSetBookmarks(HelpWindow win, Bookmarks bookmarks)
{
    win->bookmarks = bookmarks;
}

void
helpWindowSetToc(HelpWindow win, Toc toc)
{
    win->toc = toc;
}

Toc
helpWindowGetToc(HelpWindow win)
{
    return win->toc;
}

void
helpWindowSetCache(HelpWindow win, DataCache cache)
{
    win->cache = cache;
}

DataCache
helpWindowGetCache(HelpWindow win)
{
    return win->cache;
}

void
helpWindowShowURL(HelpWindow win, const gchar *ref, 
		  gboolean useCache, gboolean addToQueue)
{
	gchar err[1024];

	win->useCache = useCache;
	if (visitURL(win, ref, useCache, addToQueue, TRUE)) {
		GtkWidget *msg;

		g_snprintf(err, sizeof(err), _("Error loading document:\n\n%s\n\nYou probably don't\nhave this documentation\ninstalled on your system."),
			 ref);
		msg = gnome_message_box_new(err, GNOME_MESSAGE_BOX_ERROR,
					    GNOME_STOCK_BUTTON_OK, NULL);
		gtk_window_set_modal (GTK_WINDOW (msg), TRUE);
		gtk_widget_show(msg);

		gtk_entry_set_text(GTK_ENTRY(win->entryBox), win->humanRef);
		return;
	} else {
		/* clear the status bar */
		gnome_appbar_pop(GNOME_APPBAR(win->appBar));
	}
	update_toolbar(win);
	setCurrent(win);
	win->useCache = TRUE;

	/* XXX This should work, but it doesn't */
	{
#ifdef HELP_USE_GTKHTML
	    const char *title = gtk_html_get_title (GTK_HTML (win->helpWidget));
#else
	    const char *title = XmHTMLGetTitle(GTK_WIDGET(win->helpWidget));
#endif
	    if (!title) title = "";
	}
#ifdef HELP_USE_GTKHTML
#warning grab focus on the scroll
#else
	gtk_widget_grab_focus(GTK_XMHTML(win->helpWidget)->html.vsb);
#endif
}

GtkWidget 
*helpWindowGetAppWindow(HelpWindow w)
{
	g_return_val_if_fail( w != NULL, NULL );

	return w->app;
}

/**********************************************************************/

#ifdef HELP_USE_GTKHTML
static HTMLURL *baseURL;

static void
on_set_base (GtkHTML *html, const gchar *url, gpointer data)
{
	HTMLURL *tmpurl;
	
	if (baseURL)
		html_url_destroy (baseURL);

	tmpurl = html_url_new (url);
	baseURL = html_url_dup (tmpurl, HTML_URL_DUP_NOREFERENCE);
	html_url_destroy (tmpurl);
}

static gchar *
parse_href (const gchar *s)
{
	gchar *retval;
	gchar *tmp;
	HTMLURL *tmpurl;

	if(s == NULL || *s == 0)
		return NULL;

	if (s[0] == '#') {
		tmpurl = html_url_dup (baseURL, HTML_URL_DUP_NOREFERENCE);
		html_url_set_reference (tmpurl, s + 1);

		tmp = html_url_to_string (tmpurl);
		html_url_destroy (tmpurl);

		return tmp;
	}

	tmpurl = html_url_new (s);
	if (html_url_get_protocol (tmpurl) == NULL) {
		if (s[0] == '/') {
			if (s[1] == '/') {
				gchar *t;

				/* Double slash at the beginning.  */

				/* FIXME?  This is a bit sucky.  */
				t = g_strconcat (html_url_get_protocol (baseURL),
						 ":", s, NULL);
				tmpurl = html_url_new (t);
				retval = html_url_to_string (tmpurl);
				html_url_destroy (tmpurl);
				g_free (t);
			} else {
				/* Single slash at the beginning.  */

				tmpurl = html_url_dup (baseURL,
						       HTML_URL_DUP_NOPATH);
				html_url_set_path (tmpurl, s);
				retval = html_url_to_string (tmpurl);
				html_url_destroy (tmpurl);
			}
		} else {
			tmpurl = html_url_append_path (baseURL, s);
			retval = html_url_to_string (tmpurl);
			html_url_destroy (tmpurl);
		}
	} else {
		retval = html_url_to_string (tmpurl);
	}

	return retval;
}

static void
url_requested (GtkHTML *html, const char *url, GtkHTMLStream *s, HelpWindow win)
{
	char *full_url = url;
	GtkHTMLStreamStatus status;
	docObj obj;
	guchar *buf;
	gint buflen;

	full_url = parse_href (url);

	if (GNOME_HELPWIN (html)->writing) {
		return;
	}

	obj = docObjNew(full_url, win->useCache);
	g_free (full_url);

	docObjResolveURL(obj, helpWindowCurrentRef(win));

	if (strstr(docObjGetAbsoluteRef(obj), "file:")) {
		int fd, tmperrno;
		char buf2[8192];
		
		fd = open(docObjGetAbsoluteRef(obj) + 5, O_RDONLY | O_NONBLOCK);
		if (fd < 0) {
			return;
		}
		
		do {
			buflen = read(fd, buf2, 8192);
			tmperrno = errno;
			if (buflen > 0)
				gtk_html_write(html, s, buf2, buflen);
		} while (buflen || tmperrno == EAGAIN);
		close(fd);
		gtk_html_end(html, s, GTK_HTML_STREAM_OK);
		docObjFree(obj);
		return;
	}

	if (transport(obj, helpWindowGetCache(win))) {
	    docObjFree(obj);
	    gtk_html_end (html, s, GTK_HTML_STREAM_ERROR);
	    return;
	}

	docObjGetRawData(obj, &buf, &buflen);
	if (buflen > 0)
		gtk_html_write (html, s, buf, buflen);
	gtk_html_end (html, s, GTK_HTML_STREAM_OK);
	docObjFree(obj);
}
#else
/* HACK HACK HACK */
/*
 * Among problems this routine has - it leaves temp files in /tmp 
 */
XmImageInfo *
load_image(GtkWidget *html_widget, gchar *ref)
{
        HelpWindow win;
	docObj obj;
	guchar *buf;
	XmImageInfo *info;
	gint buflen;
	gint fd;
	char tmpnam[]="/tmp/GnomeHelpBrowser-tmp.XXXXXX";

	win = gtk_object_get_data(GTK_OBJECT(html_widget), "HelpWindow");
	obj = docObjNew(ref, win->useCache);
	docObjResolveURL(obj, helpWindowCurrentRef(win));
	if (strstr(docObjGetAbsoluteRef(obj), "file:")) {
	    info = XmHTMLImageDefaultProc(html_widget,
					  docObjGetAbsoluteRef(obj) + 5,
					  NULL, 0);
	    docObjFree(obj);
	    return info;
	}
	if (transport(obj, helpWindowGetCache(win))) {
	    docObjFree(obj);
	    return NULL;
	}
	
	fd = mkstemp(tmpnam);
	if (fd >= 0) {
	    docObjGetRawData(obj, &buf, &buflen);
	    write(fd, buf, buflen);
	    close(fd);
	}
		
	docObjFree(obj);
	return XmHTMLImageDefaultProc(html_widget, tmpnam, NULL, 0);
}
#endif

void
statusMsg(gchar *msg)
{
    HelpWindow win;
    extern GList *windowList;
    if (!windowList) 
      puts(msg);
    else {
      win = (HelpWindow)g_list_first(windowList)->data;
      gnome_appbar_pop(GNOME_APPBAR(win->appBar));
      gnome_appbar_push(GNOME_APPBAR(win->appBar), msg);
      gnome_appbar_refresh(GNOME_APPBAR(win->appBar));
      while (gtk_events_pending ())
        gtk_main_iteration ();
      gdk_flush();
    }
}

/* Change first parameter of gnome_appbar_new for this to work */
void
statusPerc(gfloat percent)
{
    HelpWindow win;
    extern GList *windowList;

    if (!windowList) 
      ;
    else {
      win = (HelpWindow)g_list_first(windowList)->data;
      gnome_appbar_set_progress(GNOME_APPBAR(win->appBar), percent);
      gnome_appbar_refresh(GNOME_APPBAR(win->appBar));
      while (gtk_events_pending ())
        gtk_main_iteration ();
      gdk_flush();
    }
}
