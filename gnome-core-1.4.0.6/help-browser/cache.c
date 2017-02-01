/* Caching functions for ghelp */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "cache.h"

struct _data_cache {
    guint maxMemSize;
    guint maxDiskSize;

    guint memSize;
    guint diskSize;

    GHashTable *hashTable;
    GList *queue;

    GCacheDestroyFunc destroyFunc;

    gchar *file;
};

struct _data_cache_entry {
    gchar *key;
    gpointer value;
    guint size;
    gchar *aux;
};

static void freeEntry(struct _data_cache_entry *entry, DataCache cache);
static void removeElement(DataCache cache);

DataCache newDataCache(guint maxMemSize, guint maxDiskSize,
		       GCacheDestroyFunc destroyFunc, gchar *file)
{
    DataCache res;

    res = (DataCache)malloc(sizeof *res);
    res->file = NULL;
    reconfigDataCache(res, maxMemSize, maxDiskSize, destroyFunc, file);

    res->memSize = 0;
    res->diskSize = 0;

    res->hashTable = g_hash_table_new(g_str_hash, g_str_equal);
    res->queue = NULL;

    return res;
}

void reconfigDataCache(DataCache cache, guint maxMemSize, guint maxDiskSize,
		       GCacheDestroyFunc destroyFunc, gchar *file)
{
    gchar filename[BUFSIZ];
    
    cache->maxMemSize = maxMemSize;
    cache->maxDiskSize = maxDiskSize;
    cache->destroyFunc = destroyFunc;
    if (cache->file) {
	g_free(cache->file);
    }
    if (file) {
	if (*(file) != '/') {
	    g_snprintf(filename, sizeof(filename), "%s/%s",
		       getenv("HOME"), file);
	} else {
	    g_snprintf(filename, sizeof(filename), "%s", file);
	}
	cache->file = g_strdup(filename);
    } else {
	cache->file = NULL;
    }
}

void saveCache(DataCache cache)
{
}

static void freeEntry(struct _data_cache_entry *entry, DataCache cache)
{
    if (cache->destroyFunc) {
	(cache->destroyFunc)(entry->value);
    }
    g_free(entry->key);
    g_free(entry);
}

void destroyDataCache(DataCache cache)
{
    g_hash_table_destroy(cache->hashTable);
    g_list_foreach(cache->queue, (GFunc)freeEntry, (gpointer)cache);
    g_list_free(cache->queue);
    free(cache);
}

gpointer lookupInDataCache(DataCache cache, gchar *key)
{
    return lookupInDataCacheWithLen(cache, key, NULL, NULL);
}

gpointer lookupInDataCacheWithLen(DataCache cache, gchar *key, gchar **aux, gint *len)
{
    struct _data_cache_entry *hit;
    
    if (! (hit = g_hash_table_lookup(cache->hashTable, key))) {
	return NULL;
    }

    /* Let's move this element to the end of the list */
    /* so it won't get tossed soon.                   */
    cache->queue = g_list_remove(cache->queue, hit);
    cache->queue = g_list_append(cache->queue, hit);

    if (len) {
	*len = hit->size;
    }
    if (aux)
        *aux = hit->aux;
    return hit->value;
}

void addToDataCache(DataCache cache, gchar *key, gpointer value,
		    guint size, gchar *aux, gboolean overWrite)
{
    struct _data_cache_entry *hit;

    if (size > cache->maxMemSize) {
	return;
    }

    /* If we have too much stuff in the cache, clean up a bit */
    while (cache->memSize + size > cache->maxMemSize) {
	removeElement(cache);
    }

    /* See if entry is already there */
    hit = g_hash_table_lookup(cache->hashTable, key);
    if (hit) {
	if (! overWrite) {
	    return;
	}

	/* Replace the value */
	if (cache->destroyFunc) {
	    (cache->destroyFunc)(hit->value);
	}
	hit->value = value;
	hit->size = size;
	hit->aux = g_strdup(aux);

	/* Let's move this element to the end of the list */
	/* so it won't get tossed soon.                   */
	cache->queue = g_list_remove(cache->queue, hit);
	cache->queue = g_list_append(cache->queue, hit);

	return;
    }

    /* It is not there - make a new entry */
    hit = g_new0(struct _data_cache_entry, 1);
    hit->key = g_strdup(key);
    hit->value = value;
    hit->size = size;
    hit->aux = g_strdup(aux);

    cache->queue = g_list_append(cache->queue, hit);
    g_hash_table_insert(cache->hashTable, hit->key, hit);
    cache->memSize += size;
}

static void removeElement(DataCache cache)
{
    GList *top;
    struct _data_cache_entry *topItem;

    /* Remove from queue */
    top = cache->queue;
    cache->queue = g_list_remove_link(cache->queue, top);

    /* Remove from hash table */
    topItem = top->data;
    g_hash_table_remove(cache->hashTable, topItem->key);
    
    cache->memSize -= topItem->size;

    /* Free all associated memory */
    g_list_free(top);
    freeEntry(topItem, cache);
}
