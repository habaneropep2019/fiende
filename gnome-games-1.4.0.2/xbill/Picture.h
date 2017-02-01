#ifndef X11_PICTURE_H
#define X11_PICTURE_H

#include <gdk/gdk.h>

class Picture {
public:
	gint width, height;
	GdkPixmap *pix;
	GdkGC *gc;
	void draw (int x, int y);
	void draw_centered ();
	void load(const char *name, int index=-1);
};

#endif
