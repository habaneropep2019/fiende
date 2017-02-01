#include <gnome.h>
#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "bonobo-ui-toolbar.h"
#include "bonobo-ui-toolbar-button-item.h"
#include "bonobo-ui-toolbar-separator-item.h"

static GtkWidget *
prepend_item (BonoboUIToolbar *toolbar,
	      const char *icon_file_name,
	      const char *label,
	      gboolean expandable)
{
	GtkWidget *item;
	GdkPixbuf *pixbuf;

	if (icon_file_name)
		pixbuf = gdk_pixbuf_new_from_file (icon_file_name);
	else
		pixbuf = NULL;

	item = bonobo_ui_toolbar_button_item_new (pixbuf, label);

	if (pixbuf)
		gdk_pixbuf_unref (pixbuf);

	bonobo_ui_toolbar_insert (toolbar, BONOBO_UI_TOOLBAR_ITEM (item), 0);
	bonobo_ui_toolbar_item_set_expandable (BONOBO_UI_TOOLBAR_ITEM (item), expandable);
	gtk_widget_show (item);

	return item;
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *toolbar;
	GtkWidget *frame;
	GtkWidget *item;

	gnome_init ("bonobo-toolbar-test", "0.0", argc, argv);

	/* ElectricFence rules. */
	free (malloc (1));

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_widget_show (frame);
	gtk_container_add (GTK_CONTAINER (window), frame);

	toolbar = bonobo_ui_toolbar_new ();
	gtk_widget_show (toolbar);
	gtk_container_add (GTK_CONTAINER (frame), toolbar);

	/* bonobo_ui_toolbar_set_orientation (BONOBO_UI_TOOLBAR (toolbar), GTK_ORIENTATION_VERTICAL); */

	{ /* Test late icon adding */
		GdkPixbuf *pixbuf;

		item = prepend_item (BONOBO_UI_TOOLBAR (toolbar), NULL, "Debian", FALSE);

		pixbuf = gdk_pixbuf_new_from_file ("/usr/share/pixmaps/gnome-debian.png");
		bonobo_ui_toolbar_button_item_set_icon (
			BONOBO_UI_TOOLBAR_BUTTON_ITEM (item), pixbuf);
		gdk_pixbuf_unref (pixbuf);
	}

	{ /* Test late label setting */
		item = prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/gnome-emacs.png", NULL, FALSE);
		bonobo_ui_toolbar_button_item_set_label (
			BONOBO_UI_TOOLBAR_BUTTON_ITEM (item), "Emacs");
	}

	item = bonobo_ui_toolbar_separator_item_new ();
	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (toolbar), BONOBO_UI_TOOLBAR_ITEM (item), 1);
	gtk_widget_show (item);

	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/apple-green.png", "Green apple", FALSE);
	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/apple-red.png", "Red apple (exp)", TRUE);
	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/gnome-applets.png", "Applets", FALSE);
	prepend_item (BONOBO_UI_TOOLBAR (toolbar), "/usr/share/pixmaps/gnome-gimp.png", "Gimp (exp)", TRUE);

	gtk_widget_show (window);

	gtk_main ();

	return 0;
}
