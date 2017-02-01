/* -*- mode: C; c-file-style: "linux" -*- */

/*
 * Background display property module.
 * (C) 1997 the Free Software Foundation
 * (C) 2000 Helix Code, Inc.
 * (C) 1999, 2000 Red Hat, Inc.
 *
 * Authors: Miguel de Icaza.
 *          Federico Mena.
 *          Radek Doulik
 *          Michael Fulbright
 *          Justin Maurer
 *          Owen Taylor
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>

#define PACKED_COLOR(c) ((((c)->red & 0xff00) << 8) | ((c)->green & 0xff00) | ((c)->blue >> 8))

typedef struct _BGState BGState;

enum {
	WALLPAPER_TILED,
	WALLPAPER_CENTERED,
	WALLPAPER_SCALED,
	WALLPAPER_SCALED_KEEP,
	WALLPAPER_EMBOSSED
};

enum {
	BACKGROUND_SIMPLE,
	BACKGROUND_WALLPAPER
};

struct _BGState {
	GdkColor   bgColor1, bgColor2;
	gint       enabled;
	gint       grad;
	gint       vertical;
	gint       bgType;
	gint       wpType;
	gchar      *wpFileName;
	gchar      *wpFileSelName;
};

GdkPixbuf *make_background (BGState *state,
			    int      output_width,
			    int      output_height,
			    gboolean can_tile);


GdkPixmap *make_root_pixmap (gint width, gint height);
void       dispose_root_pixmap (GdkPixmap *pixmap);
void       set_root_pixmap (GdkPixmap *pixmap);

void render_to_drawable (GdkPixbuf *pixbuf,
			 GdkDrawable *drawable, GdkGC *gc,
			 int src_x, int src_y,
			 int dest_x, int dest_y,
			 int width, int height,
			 GdkRgbDither dither,
			 int x_dither, int y_dither);

gulong xpixel_from_color (GdkColor *color);
