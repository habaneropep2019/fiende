#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>

#include "toc2.h"
#include "toc2-man.h"

static gint compareItems(const void *a, const void *b);

static struct _man_sections {
    gchar ch;
    gchar *name;
    gint flag;
} man_sections[] = {
    { '1', N_("User Commands"), 0 },
    { '2', N_("System Calls"), 0 },
    { '3', N_("Library Functions"), 0 },
    { '4', N_("Special Files"), 0 },
    { '5', N_("File Formats"), 0 },
    { '6', N_("Games"), 0 },
    { '7', N_("Miscellaneous"), 0 },
    { '8', N_("Administration"), 0 },
    { '9', N_("man9"), 0 },
    { 'n', N_("mann"), 0 },
    { 'x', N_("manx"), 0 },
    { 0, NULL, 0 }
};

gchar *getManSection(gchar ch)
{
    struct _man_sections *p = man_sections;

    while (p->ch) {
	if (p->ch == ch) {
	    return _(p->name);
	}
	p++;
    }

    return (gchar *) "Unknown Section";
}

GList *newManTable(struct _toc_config *conf)
{
    struct _man_sections *p;
    gchar dirname[BUFSIZ];
    DIR *d;
    struct dirent *dirp = NULL;
    char *lang;
    GList *lang_list = NULL;
    GList *list = NULL;
    GList *temp = NULL;
    struct _big_table_entry *entry;
    gchar *s;
    int tmp_array_size = 256, tmp_array_elems = 0;
    struct _big_table_entry **tmp_array = g_new0 (struct _big_table_entry *,
                                                tmp_array_size);

    lang_list = gnome_i18n_get_language_list ("LC_MESSAGE");

    while (conf->path) {
	if (conf->type != TOC_MAN_TYPE) {
	    conf++;
	    continue;
	}
	p = man_sections;
	while (p->ch) {
	  temp = lang_list;
	  while (temp)
	  {
	    lang = (gchar*) temp->data;
	    if (strcmp(lang,"C") == 0) {
	    	g_snprintf(dirname, sizeof(dirname), "%s/man%c",
				conf->path, p->ch);
	    } else {
		g_snprintf(dirname, sizeof(dirname), "%s/%s/man%c",
				conf->path, lang, p->ch);
	    }
	    temp = temp->next;	
	    d = opendir(dirname);
	    if (d) {
	        while (d && (dirp = readdir(d))) {
		    if (! (strcmp("..", dirp->d_name) &&
			   strcmp(".", dirp->d_name))) {
		        continue;
		    }
		    /* Add to table */
		    entry = g_malloc0 (sizeof(*entry));
		    entry->filename = g_strdup_printf ("%s/%s",
						       dirname, dirp->d_name);

		    entry->name = g_strdup(dirp->d_name);
		    /* first delete any possible compression extension */
		    if ((s = strrchr(entry->name, '.'))) {
			if ((strcmp(s,".gz") == 0) ||
			    (strcmp(s,".Z") == 0) ||
			    (strcmp(s,".bz2") == 0) ||
			    (strcmp(s,".bz") == 0) ||
			    (strcmp(s,".z") == 0))
				*s = '\0';
		    }
		    /* and then the section or .man extension */
		    if ((s = strrchr(entry->name, '.'))) {
			*s = '\0';
		    }

		    entry->type = TOC_MAN_TYPE;
		    entry->ext = p->ch;
		    entry->expanded = 1;
		    entry->section = NULL;

                    if (tmp_array_elems >= tmp_array_size) {
                        tmp_array_size *= 2;
                        tmp_array = g_realloc(tmp_array,
                                              (sizeof(*tmp_array)
                                               * tmp_array_size));
                    }
                    tmp_array[tmp_array_elems++] = entry;
		}
		if (d && dirp) {
		    p->flag = 1;
		}
		closedir(d);
	      }
	    }

	    p++;
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

    return list;
}

/* Sort according to section/name */
static gint compareItems(const void *a, const void *b)
{
    gint res;
    struct _big_table_entry *e1 = *(struct _big_table_entry **)a;
    struct _big_table_entry *e2 = *(struct _big_table_entry **)b;

    res = e1->ext - e2->ext;

    /* If they are the same, sort based on length of filename */
    if (!res) {
 	res = strcmp(e1->name, e2->name);
    }

    return res;
}
