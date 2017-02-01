#ifndef _GNOME_HELP_BROWSER_WINDOW_H_
#define _GNOME_HELP_BROWSER_WINDOW_H_

#include <gtk/gtk.h>
#include <glib.h>

#include "queue.h"
#include "history.h"
#include "bookmarks.h"
#include "cache.h"
#include "toc2.h"

typedef struct _helpWindow *HelpWindow;

typedef void (*HelpWindowCB)(HelpWindow win);

HelpWindow helpWindowNew(gchar *name,
			 gint x, gint y, gint width, gint height,
			 HelpWindowCB about_callback,
			 HelpWindowCB new_window_callback,
			 HelpWindowCB close_window_callback,
			 HelpWindowCB set_current_callback,
			 HelpWindowCB config_callback);
void helpWindowClose(HelpWindow win);
void helpWindowShowURL(HelpWindow win, const gchar *ref, 
		       gboolean useCache, gboolean addToQueue);
void helpWindowSetHistory(HelpWindow win, History history);
void helpWindowSetToc(HelpWindow win, Toc toc);
void helpWindowSetBookmarks(HelpWindow win, Bookmarks bookmarks);
Toc helpWindowGetToc(HelpWindow win);
void helpWindowSetCache(HelpWindow win, DataCache cache);
DataCache helpWindowGetCache(HelpWindow win);

void helpWindowQueueMark(HelpWindow w);
void helpWindowQueueAdd(HelpWindow w, gchar *ref);
void helpWindowHistoryAdd(HelpWindow w, gchar *ref);

gchar *helpWindowCurrentRef(HelpWindow w);
gchar *helpWindowHumanRef(HelpWindow w);

void helpWindowHTMLSource(HelpWindow w, gchar *s, gint len,
			  gchar *ref, gchar *humanRef);
void helpWindowJumpToAnchor(HelpWindow w, gchar *s);
void helpWindowJumpToLine(HelpWindow w, gint n);

GtkWidget *helpWindowGetAppWindow(HelpWindow w);

#endif
