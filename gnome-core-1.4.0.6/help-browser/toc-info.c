#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "toc.h"
#include "toc-man.h"

struct _info_node {
    char *filename;
    char *basename;

    int len;
};

static int compareItems(struct _info_node *a, struct _info_node *b);
static char *makeBaseName(char *name);

static int compareItems(struct _info_node *a, struct _info_node *b)
{
    int res;
    
    res = strcmp(a->basename, b->basename);

    /* If they are the same, sort based on length of filename */
    if (!res) {
	res = a->len - b->len;
    }
    
    return res;
}

static char *makeBaseName(char *name)
{
    char buf[BUFSIZ];
    char *end, *s, *ss;

    g_snprintf (buf, sizeof (buf), "%s", name);
    end = buf + strlen(buf);

    /* Strip off any trailing `.gz' */
    if ((strlen(buf) > 3) && !strcmp(end - 3, ".gz")) {
	end -= 3;
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

void expandInfoRoot(GtkWidget *item)
{
    GtkWidget *tree;
    GtkWidget *newitem;
    struct _info_node *p;
    struct _toc_config *toc;
    char last[BUFSIZ];
    DIR *d;
    struct dirent *dirp;
    GList *list = NULL;
    GList *listItem;
    char *s;

    if (gtk_object_get_data(GTK_OBJECT(item), "expanded")) {
	return;
    }

    setWatch(gtk_widget_get_toplevel(item));

    tree = GTK_TREE_ITEM_SUBTREE(item);

    /* Iterate, setting flags to 1 for those that exist */
    toc = toc_config;
    while (toc->path) {
	if (toc->type != TOC_INFO_TYPE) {
	    toc++;
	    continue;
	}

	d = opendir(toc->path);
	if (d) {
	    while (dirp = readdir(d)) {
	        if (! (strcmp("..", dirp->d_name) &&
		       strcmp(".", dirp->d_name))) {
		    continue;
		}
		
		p = malloc(sizeof(*p));
		p->filename = g_strdup_printf ("%s/%s",
					       toc->path, dirp->d_name);
		p->len = strlen(p->filename);
		p->basename = makeBaseName(dirp->d_name);
		
		list = g_list_insert_sorted(list, p,
					    (GCompareFunc)compareItems);
	    }
	    closedir(d);
	}
	
	toc++;
    }

    strcpy(last, "");
    while (list) {
	listItem = list;
	list = g_list_remove_link(list, listItem);

	/* Make item, link it in, show it */
	p = (struct _info_node *)listItem->data;
	if (strcmp(last, p->basename) != 0) {
	    newitem = gtk_tree_item_new_with_label(p->basename);
	    gtk_tree_append(GTK_TREE(tree), newitem);
	    gtk_widget_show(newitem);
	
	    /* Set the URL for this item */
	    s = g_strdup_printf ("info:%s", p->basename);
	    gtk_object_set_data_full (GTK_OBJECT(newitem), "URL", s,
				      (GtkDestroyNotify) g_free);

	    strncpy(last, p->basename, sizeof (last));
	    last[sizeof (last) - 1] = '\0';
	}
	
	g_free(p->filename);
	g_free(p->basename);
	free(listItem->data);
	g_list_free(listItem);
    }

    g_list_free(list);

    unsetWatch(gtk_widget_get_toplevel(item));
    gtk_object_set_data(GTK_OBJECT(item), "expanded", (gpointer)1);
}
