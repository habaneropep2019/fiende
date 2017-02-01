#ifndef _GNOME_HELP_BOOKMARKS_H_
#define _GNOME_HELP_BOOKMARKS_H_

#include <gtk/gtk.h>
#include <glib.h>

typedef struct _bookmarks_struct *Bookmarks;

typedef void (*BookmarksCB) (gchar *ref);

Bookmarks newBookmarks(BookmarksCB callback, gpointer data, gchar *file);
void reconfigBookmarks(Bookmarks b, BookmarksCB callback,
		       gpointer data, gchar *file);
void destroyBookmarks(Bookmarks b);
void addToBookmarks(Bookmarks b, gchar *ref, gchar *title);
void showBookmarks(Bookmarks b);
void hideBookmarks(Bookmarks b);
void saveBookmarks(Bookmarks b);

#endif /* _GNOME_HELP_BOOKMARKS_H_ */
