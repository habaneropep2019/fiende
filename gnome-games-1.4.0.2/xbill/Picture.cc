#include <libgnome/libgnome.h>
#include "Picture.h"
#include "objects.h"

void Picture::load(const char *name, int index) {
	char *logo;
	char file[255];
	GdkBitmap *mask;
	gint gcmask;
	GdkGCValues gcval;

	gcmask = (GDK_GC_FOREGROUND | GDK_GC_BACKGROUND | GDK_GC_EXPOSURES);
	gcval.graphics_exposures = FALSE;
	gcval.foreground.pixel = ui.black.pixel;
	gcval.background.pixel = ui.white.pixel;
	if (index>=0)
		sprintf (file, "xbill/pixmaps/%s_%d.xpm", name, index);
	else
	        sprintf(file, "xbill/pixmaps/%s.xpm", name);
	logo = gnome_datadir_file(file);
	pix = gdk_pixmap_colormap_create_from_xpm(ui.display, ui.colormap,
						  &mask, &ui.white, logo);
	g_free(logo);

	if (pix == NULL) {
		printf ("cannot open %s\n", file);
		exit(1);
	}
	gc = gdk_gc_new_with_values(ui.display, &gcval,
				    (GdkGCValuesMask)gcmask);
	gdk_gc_set_clip_mask(gc, mask);
	gdk_window_get_size(pix, &width, &height);
}
