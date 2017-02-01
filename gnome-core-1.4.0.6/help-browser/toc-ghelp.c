#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "toc.h"
#include "toc-ghelp.h"

void expandGHelpRoot(GtkWidget *item)
{
    GtkWidget *tree;
    GtkWidget *newitem;
    struct _toc_config *toc;
    char fullname[BUFSIZ];
    DIR *d;
    struct dirent *dirp;
    char *s, *lang;

    if (gtk_object_get_data(GTK_OBJECT(item), "expanded")) {
	return;
    }

    setWatch(gtk_widget_get_toplevel(item));

    tree = GTK_TREE_ITEM_SUBTREE(item);

    /* Iterate, setting flags to 1 for those that exist */
    toc = toc_config;
    while (toc->path) {
	if (toc->type != TOC_GHELP_TYPE) {
	    toc++;
	    continue;
	}

	d = opendir(toc->path);
	if (d) {
	    while ((dirp = readdir(d))) {
	        if (! (strcmp("..", dirp->d_name) &&
		       strcmp(".", dirp->d_name))) {
		    continue;
		}
			
		/* XXX should be sorted */
		
		newitem = gtk_tree_item_new_with_label(dirp->d_name);
		gtk_tree_append(GTK_TREE(tree), newitem);
		
		lang = getenv("LANGUAGE");
		if (!lang)
		    lang = getenv("LANG");
		if (!lang)
		    lang = "C";
		g_snprintf(fullname, sizeof(fullname), "%s/%s/%s/index.html",
			   toc->path, dirp->d_name, lang);
		
		/* XXX need to traverse LANGUAGE to find right
		   topic.dat. In fact, this LANG stuff doesn't really
		   belong here at all */
			
		s = g_strdup(fullname);
		gtk_object_set_data(GTK_OBJECT(newitem), "URL", s);
		gtk_signal_connect_object(GTK_OBJECT(newitem), "destroy",
					  (GtkSignalFunc)g_free, (gpointer)s);
		gtk_widget_show(newitem);
	    }
	    closedir(d);
	}
	
	toc++;
    }

    unsetWatch(gtk_widget_get_toplevel(item));
    gtk_object_set_data(GTK_OBJECT(item), "expanded", (gpointer)1);
}
