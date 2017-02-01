#include <gnome.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include <glib.h>

#include "toc2.h"
#include "toc2-ghelp.h"

static gint compareItems(const void *a, const void *b);

GList *newGhelpTable(struct _toc_config *conf)
{
    char filename[BUFSIZ];
    DIR *d;
    struct dirent *dirp;
    GList *list = NULL;
    struct _big_table_entry *entry;
    char *lang;
    GList *lang_list = NULL, *new_lang_list = NULL;
    GList *temp = NULL;
    struct stat buf;
    int tmp_array_size = 256, tmp_array_elems = 0;
    struct _big_table_entry **tmp_array = g_new0 (struct _big_table_entry *,
                                                tmp_array_size);

    lang_list = gnome_i18n_get_language_list ("LC_MESSAGE");

    /* stick in possible variants of languages in lang_list */
    temp= lang_list;
    while (temp) {
      gchar *p;

      lang = g_strdup((gchar*) temp->data);
      p = strchr(lang, '_');
      if (p) {
	new_lang_list = g_list_append(new_lang_list, lang);
	lang = g_strdup((gchar*) temp->data);
	p = strchr(lang, '_');
	*p = '\0';
	new_lang_list = g_list_append(new_lang_list, lang);
      } else {
	new_lang_list = g_list_append(new_lang_list, lang);
      }
      temp= temp->next;
    }

    while (conf->path) {
	if (conf->type != TOC_GHELP_TYPE) {
	    conf++;
	    continue;
	}

	d = opendir(conf->path);
	if (d) {
	    while ((dirp = readdir(d))) {
	        if (! (strcmp("..", dirp->d_name) &&
		       strcmp(".", dirp->d_name))) {
		    continue;
		}

		temp= new_lang_list;
		while (temp)
		  {
		    lang= (gchar*) temp->data;
		    g_snprintf (filename, sizeof(filename),
				"%s/%s/%s/index.html",
				conf->path, dirp->d_name, lang);

		    if (stat (filename, &buf) == 0)
		      break;

		    temp= temp->next;
		  }

		if (temp)
		  {
		    entry = g_malloc(sizeof(*entry));
		    entry->type = TOC_GHELP_TYPE;
		    entry->name = g_strdup(dirp->d_name);
		    entry->section = NULL;
		    entry->expanded = 1;
		    entry->ext = 0;
		    entry->filename = g_strdup(filename);

                    if (tmp_array_elems >= tmp_array_size) {
                        tmp_array_size *= 2;
                        tmp_array = g_realloc(tmp_array,
                                              (sizeof(*tmp_array)
                                               * tmp_array_size));
                    }
                    tmp_array[tmp_array_elems++] = entry;
		  }
	    }
	    closedir(d);
	}

	conf++;
    }

    if (tmp_array_elems == 0) {
	    g_free (tmp_array);
	    return NULL;
    }
    /* FIXME: If would be much cooler if glib had a function to sort
       lists.  */
    qsort(tmp_array, (size_t) tmp_array_elems, sizeof(*tmp_array),
          compareItems);

    {
        int i;

        for (i = tmp_array_elems - 1; i > 0; i--) {
	    if (compareItems(&tmp_array[i],&tmp_array[i-1]) != 0) 
                list = g_list_prepend(list, tmp_array[i]);
	}
	list = g_list_prepend(list, tmp_array[0]);

        g_free(tmp_array);
    }

    /* free up lang lists */
    g_list_foreach(new_lang_list, (GFunc) g_free, NULL);

    return list;
}

static gint compareItems(const void *a, const void *b)
{
    struct _big_table_entry *e1 = *(struct _big_table_entry **)a;
    struct _big_table_entry *e2 = *(struct _big_table_entry **)b;

    return strcmp(e1->name, e2->name);
}
