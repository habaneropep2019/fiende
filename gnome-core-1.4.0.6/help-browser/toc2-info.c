#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <glib.h>

#include "misc.h"
#include "toc2-info.h"
#include "toc2.h"

struct _info_node {
    gchar *filename;
    gchar *basename;

    gint len;
};

/* Pairs are used in the expansion code */
struct _pair {
    gchar *name;
    gint  val;
};

typedef struct _pair *Pair;

static gint compareItems(const void *a, const void *b);
static gchar *makeBaseName(gchar *name);
static gchar *findInfoFile(gchar *rootFile, gchar *name);

GList *newInfoTable(struct _toc_config *conf)
{
    char fullname[BUFSIZ];
    DIR *d;
    struct dirent *dirp;
    GList *list = NULL;
    GList *infoList = NULL;
    GList *listItem;
    struct _big_table_entry *entry;
    struct _info_node *p;
    char last[BUFSIZ];
    int tmp_array_size = 256, tmp_array_elems = 0;
    struct _info_node **tmp_array = g_new0 (struct _info_node *,
					    tmp_array_size);

    while (conf->path) {
	if (conf->type != TOC_INFO_TYPE) {
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

		p = g_malloc(sizeof(*p));
		p->basename = makeBaseName(dirp->d_name);
		g_snprintf(fullname, sizeof(fullname),
			 "%s/%s", conf->path, dirp->d_name);
		p->filename = g_strdup(fullname);
		p->len = strlen(fullname);

                if (tmp_array_elems >= tmp_array_size) {
                    tmp_array_size *= 2;
                    tmp_array = g_realloc(tmp_array,
                                          (sizeof(*tmp_array)
                                           * tmp_array_size));
                }
                tmp_array[tmp_array_elems++] = p;
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

        for (i = tmp_array_elems - 1; i >= 0; i--)
            infoList = g_list_prepend(infoList, tmp_array[i]);

        g_free(tmp_array);
    }

    /* XXX should also read /usr/info/dir */

    strcpy(last, "");
    while (infoList) {
	listItem = infoList;
	infoList = g_list_remove_link(infoList, listItem);

	/* Make item, link it in, show it */
	p = (struct _info_node *)listItem->data;
	if (strcmp(last, p->basename)) {
	    entry = g_malloc(sizeof(*entry));
	    entry->type = TOC_INFO_TYPE;
	    entry->name = g_strdup(p->basename);
	    entry->section = NULL;
	    entry->expanded = 0;
	    entry->indirect = 0;
	    entry->ext = 0;
	    entry->filename = g_strdup(p->filename);

	    list = g_list_append(list, entry);

	    strcpy(last, p->basename);
	}

	g_free(p->filename);
	g_free(p->basename);
	g_free(listItem->data);
	g_list_free(listItem);
    }

    g_list_free(infoList);

    return list;
}

static gchar *makeBaseName(gchar *name)
{
    gchar buf[BUFSIZ];
    gchar *end, *s, *ss;

    g_snprintf (buf, sizeof(buf), "%s", name);
    end = buf + strlen(buf);

    /* Strip off any trailing `.gz' */
    if ((strlen(buf) > 3) && !strcmp(end - 3, ".gz")) {
	end -= 3;
	*end = '\0';
    }else if((strlen(buf) > 4) && !strcmp(end - 4, ".bz2")) {
	end -= 4;
	*end = '\0';
    }

    /* Strip and trailing `-nnnn' where `n' are numbers */
    ss = s = strrchr(buf, '-');
    if (s) {
	s++;
	while (*s && isdigit(*s)) {
	    s++;
	}
	if (! *s) {
	    end = ss;
	    *end = '\0';
	}
    }

    /* Strip off any trailing `.info' */
    if ((strlen(buf) > 5) && !strcmp(end - 5, ".info")) {
	end -= 5;
	*end = '\0';
    }

    return g_strdup(buf);
}

static gint compareItems(const void *a, const void *b)
{
    gint res;
    struct _info_node *e1 = *(struct _info_node **)a;
    struct _info_node *e2 = *(struct _info_node **)b;

    res = strcmp(e1->basename, e2->basename);

    /* If they are the same, sort based on length of filename */
    if (!res) {
	res = e1->len - e2->len;
    }

    return res;
}


gint expandInfoTable(GList *table, gchar *name)
{
    /* *table points into the table, and we must remain valid    */
    /* whe we return.  Therefore, only insert items after table. */

    guchar *data;
    gchar *c;
    GList *indirect = NULL;
    GList *nextItem;
    GList *list;
    Pair  e;
    gchar *s;
    gchar *anchor;
    struct _big_table_entry *entry;
    gint val;
    gchar *rootFile;

    rootFile = ((struct _big_table_entry *)table->data)->filename;
    if (loadFileToBuf(rootFile, &data, NULL) || !data) {
	return -1;
    }

    ((struct _big_table_entry *)table->data)->expanded = 1;

    /* Find the indirect table */
    if (!(c = strstr(data, "\nIndirect:\n"))) {
	/* No indirect table - that's OK */
	g_free(data);
	return 0;
    }
    ((struct _big_table_entry *)table->data)->indirect = 1;

    /* Parse the indirect table */
    c += 12;
    while (*c != '') {
	e = g_malloc(sizeof(*e));
	s = strchr(c, ':');
	if (!s) {
	    g_warning("ran out of indirect entries");
	    break;
	}
	*s = '\0';
	e->name = c;
	c = s + 1;
	s = strchr(c, '\n');
	if (!s) {
	    g_warning("ran out of indirect entries");
	    break;
	}
	*s = '\0';
	e->val = strtol(c, NULL, 10);
	c = s + 1;

	indirect = g_list_append(indirect, e);
    }

    /* Find the tag table */
    if (!(c = strstr(c, "\nTag Table:\n(Indirect)\n"))) {
	g_warning("missing tag table");
	if (indirect) {
	    g_list_foreach(indirect, (GFunc)g_free, NULL);
	    g_list_free(indirect);
	}
	g_free(data);
	return -1;
    }

    /* Parse the tag table */
    c += 24;
    while (*c != '') {
	c += 6;
	s = strchr(c, 0x7f);
	if (!s) {
	    g_warning("Invalid table entry");
	    break;
	}
	*s = '\0';
	anchor = c;

	c = s + 1;
	s = strchr(c, '\n');
	if (!s) {
	    g_warning("Ran out of indirect entries!");
	    break;
	}
	*s = '\0';
	val = strtol(c, NULL, 10);
	c = s + 1;

	/* Create new entry */
	entry = g_malloc(sizeof(*entry));
	entry->type = TOC_INFO_TYPE;
	entry->name = g_strdup(name);
	entry->section = g_strdup(anchor);
	map_spaces_to_underscores(entry->section);
	entry->expanded = 0;
	entry->indirect = 0;
	entry->ext = 0;
	/* Find the right filename */
	list = g_list_last(indirect);
	while (list) {
	    if (((Pair)list->data)->val <= val) {
		entry->filename =
		    findInfoFile(rootFile, ((Pair)list->data)->name);
		break;
	    }
	    list = list->prev;
	}

	/* Splice it into the table, and update table */
	nextItem = table->next;
	table->next = NULL;
	g_list_append(table, entry);
	table = table->next;
	table->next = nextItem;
	if (nextItem)
		nextItem->prev = table;
    }

    if (indirect) {
	g_list_foreach(indirect, (GFunc)g_free, NULL);
	g_list_free(indirect);
    }
    g_free(data);

    return 0;
}

static gchar *findInfoFile(gchar *rootFile, gchar *name)
{
    gchar buf[BUFSIZ];
    gchar *s;

    g_snprintf (buf, sizeof(buf), "%s", rootFile);
    s = strrchr(buf, '/');
    s++;
    *s = '\0';
    strcat(buf, name);

    if (!access(buf, R_OK)) {
	return g_strdup(buf);
    }

    /* try a .gz on the end */
    strcat(buf, ".gz");
    if (!access(buf, R_OK)) {
	return g_strdup(buf);
    }
    *strstr(buf,".gz")=0;
    strcat(buf, ".bz2");
    if (!access(buf, R_OK)) {
	return g_strdup(buf);
    }

    /* XXX this is goofy */
    return g_strdup("missing");
}
