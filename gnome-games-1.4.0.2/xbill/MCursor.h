#ifndef X11_MCURSOR_H
#define X11_MCURSOR_H

#include <gdk/gdk.h>

class MCursor {
public:
	static const int SEP_MASK = 0;
	static const int OWN_MASK = 1;
	GdkCursor *cursor;
	void load(const char *name, int masked);
};

#endif
