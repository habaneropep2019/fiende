#include <config.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "toc.h"
#include "toc-man.h"

static struct _man_sections {
    char ch;
    char *name;
    int flag;
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

static int compareItems(char *a, char *b)
{
    char *pa, *pb;

    pa = strrchr(a, '/');
    pb = strrchr(b, '/');
    pa++;
    pb++;
    return strcmp(pa, pb);
}

void expandManPagesIndex(GtkWidget *item, struct _man_sections *ext)
{
    GtkWidget *tree;
    GtkWidget *newitem;
    struct _toc_config *toc;
    char dirname[BUFSIZ];
    DIR *d;
    struct dirent *dirp;
    GList *list = NULL;
    GList *listItem;
    char *last = "";
    char *this;
    char fullname[BUFSIZ];

    if (gtk_object_get_data(GTK_OBJECT(item), "expanded")) {
	return;
    }

    setWatch(gtk_widget_get_toplevel(item));
    
    tree = GTK_TREE_ITEM_SUBTREE(item);

    /* Actually need to read, sort, then insert here */
    list = NULL;

    toc = toc_config;
    while (toc->path) {
	if (toc->type != TOC_MAN_TYPE) {
	    toc++;
	    continue;
	}
	g_snprintf(dirname, sizeof(dirname), "%s/man%c", toc->path, ext->ch);

	d = opendir(dirname);
	if (d) {
	    while ((dirp = readdir(d))) {
	        char *fname;
	        if (! (strcmp("..", dirp->d_name)
		       && strcmp(".", dirp->d_name))) {
		    continue;
		}
		fname = g_strdup_printf ("%s/%s", dirname, dirp->d_name);
		list = g_list_insert_sorted(list, fname,
					    (GCompareFunc)compareItems);
	    }
	    closedir(d);
	}

	toc++;
    }

    while (list) {
	listItem = list;
	list = g_list_remove_link(list, listItem);

	/* Make item, link it in, show it */
	this = strrchr(listItem->data, '/') + 1;
	if (! strcmp(this, last)) {
	    g_snprintf(fullname, sizeof(fullname),
		       "%s (%s)", this, (char *)listItem->data);
	    newitem = gtk_tree_item_new_with_label(fullname);
	} else {
	    newitem = gtk_tree_item_new_with_label(this);
	}
	last = this;
	gtk_tree_append(GTK_TREE(tree), newitem);
	gtk_widget_show(newitem);
	
	/* Set the URL for this item */
	gtk_object_set_data(GTK_OBJECT(newitem), "URL", listItem->data);
	gtk_signal_connect_object(GTK_OBJECT(newitem), "destroy",
				  (GtkSignalFunc)g_free, listItem->data);
	
	g_list_free(listItem);
    }

    unsetWatch(gtk_widget_get_toplevel(item));
    gtk_object_set_data(GTK_OBJECT(item), "expanded", (gpointer)1);
}

void expandManPagesRoot(GtkWidget *item)
{
    GtkWidget *tree;
    GtkWidget *newitem;
    GtkWidget *subtree;
    struct _man_sections *p;
    struct _toc_config *toc;
    char dirname[BUFSIZ];
    DIR *d;
    struct dirent *dirp;

    if (gtk_object_get_data(GTK_OBJECT(item), "expanded")) {
	return;
    }

    setWatch(gtk_widget_get_toplevel(item));

    tree = GTK_TREE_ITEM_SUBTREE(item);

    /* Set the flags to 0 */
    p = man_sections;
    while (p->ch) {
	p->flag = 0;
	p++;
    }

    /* Iterate, setting flags to 1 for those that exist */
    toc = toc_config;
    while (toc->path) {
	if (toc->type != TOC_MAN_TYPE) {
	    toc++;
	    continue;
	}
	p = man_sections;
	while (p->ch) {
	    g_snprintf(dirname, sizeof(dirname), "%s/man%c", toc->path, p->ch);
	    d = opendir(dirname);
	    if (d) {
	        while (d && (dirp = readdir(d))) {
		    if (! (strcmp("..", dirp->d_name) &&
			   strcmp(".", dirp->d_name))) {
		        continue;
		    }
		    break;
		}
		if (d && dirp) {
		    p->flag = 1;
		}
		closedir(d);
	    }
	    
	    p++;
	}
	toc++;
    }

    p = man_sections;
    while (p->ch) {
	if (! p->flag) {
	    p++;
	    continue;
	}
	newitem = gtk_tree_item_new_with_label(_(p->name));
	gtk_tree_append(GTK_TREE(tree), newitem);
	gtk_signal_connect(GTK_OBJECT(newitem), "expand",
			   (GtkSignalFunc)expandManPagesIndex, p);
	subtree = gtk_tree_new();
	gtk_tree_set_selection_mode(GTK_TREE(subtree), GTK_SELECTION_SINGLE);
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(newitem), subtree);
	gtk_widget_show(newitem);

	p++;
    }

    unsetWatch(gtk_widget_get_toplevel(item));
    gtk_object_set_data(GTK_OBJECT(item), "expanded", (gpointer)1);
}
