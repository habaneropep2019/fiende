#include "MCursor.h"
#include "objects.h"

void MCursor::load(const char *name, int masked) {
	static char *dir = NULL;
	GdkPixmap *pixmap, *bitmap, *mask;
	int width, height;
	char file[255];

	if (!dir) dir = gnome_datadir_file("xbill/cursors");

	sprintf (file, "%s/%s.xpm", dir, name);
	pixmap = gdk_pixmap_colormap_create_from_xpm(ui.display, ui.colormap,
						     &bitmap, NULL, file);
	if (pixmap == NULL) {
		printf ("cannot open %s\n", file);
		exit(1);
	} else
	  gdk_pixmap_unref(pixmap);
	if (masked == SEP_MASK) {
	  sprintf(file, "%s/%s_mask.xpm", dir, name);
	  pixmap = gdk_pixmap_colormap_create_from_xpm(ui.display, ui.colormap,
						       &mask, NULL, file);
	  if (pixmap == NULL) {
	    printf("cannot open %s\n", file);
	    exit(1);
	  } else
	    gdk_pixmap_unref(pixmap);
	} else
	  mask = bitmap;
	gdk_window_get_size(bitmap, &width, &height);
	cursor = gdk_cursor_new_from_pixmap(bitmap, mask, &ui.black, &ui.white,
					    width/2, height/2);
}

