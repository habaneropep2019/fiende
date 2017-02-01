#ifndef _GNOME_HELP_HISTORY_H_
#define _GNOME_HELP_HISTORY_H_

#include <gtk/gtk.h>
#include <glib.h>

typedef struct _history_struct *History;

typedef void (*HistoryCB) (gchar *ref);

History newHistory(gint length, HistoryCB callback, gchar *file);
void reconfigHistory(History h, gint length, HistoryCB callback, gchar *file);
void destroyHistory(History h);
void addToHistory(History h, gchar *ref);
void showHistory(History h);
void hideHistory(History h);
void saveHistory(History h);

#endif /* _GNOME_HELP_HISTORY_H_ */
