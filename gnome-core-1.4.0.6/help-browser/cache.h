#ifndef _GNOME_HELP_CACHE_H_
#define _GNOME_HELP_CACHE_H_

#include <glib.h>

typedef struct _data_cache *DataCache;

DataCache newDataCache(guint maxMemSize, guint maxDiskSize,
		       GCacheDestroyFunc destroyFunc, gchar *file);

void reconfigDataCache(DataCache cache, guint maxMemSize, guint maxDiskSize,
		       GCacheDestroyFunc destroyFunc, gchar *file);

void destroyDataCache(DataCache cache);

gpointer lookupInDataCache(DataCache cache, gchar *key);
gpointer lookupInDataCacheWithLen(DataCache cache, gchar *key, gchar **aux, gint *len);

/* addToDataCache() will strdup() the key, so you should free */
/* it if you need to.  The value is *not* copied, but it *is* */
/* passed to destroyFunc() when it falls off the stack.       */
/* If overWrite is TRUE, replace any existing entry.          */
/* Otherwise, make no change.                                 */
void addToDataCache(DataCache cache, gchar *key, gpointer value,
		    guint size, gchar *aux, gboolean overWrite);

void saveCache(DataCache cache);

#endif
