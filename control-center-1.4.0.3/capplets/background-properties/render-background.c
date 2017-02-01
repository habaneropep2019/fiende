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
#include <gdk/gdkx.h>

#include <X11/Xatom.h>

#include <math.h>

#include "imlib-misc.h"
#include "render-background.h"

static void
fill_gradient (GdkPixbuf *pixbuf,
	       GdkColor *c1, GdkColor *c2, int vertical)
{
	int i, j;
	int dr, dg, db;
	int gs1;
	int vc = (!vertical || (c1 == c2));
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	guchar *b, *row;
	guchar *d = gdk_pixbuf_get_pixels (pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride (pixbuf);

#define R1 c1->red
#define G1 c1->green
#define B1 c1->blue
#define R2 c2->red
#define G2 c2->green
#define B2 c2->blue

	dr = R2 - R1;
	dg = G2 - G1;
	db = B2 - B1;

	gs1 = (vertical) ? h-1 : w-1;

	row = g_new (unsigned char, rowstride);

	if (vc) {
		b = row;
		for (j = 0; j < w; j++) {
			*b++ = (R1 + (j * dr) / gs1) >> 8;
			*b++ = (G1 + (j * dg) / gs1) >> 8;
			*b++ = (B1 + (j * db) / gs1) >> 8;
		}
	}

	for (i = 0; i < h; i++) {
		if (!vc) {
			unsigned char cr, cg, cb;
			cr = (R1 + (i * dr) / gs1) >> 8;
			cg = (G1 + (i * dg) / gs1) >> 8;
			cb = (B1 + (i * db) / gs1) >> 8;
			b = row;
			for (j = 0; j < w; j++) {
				*b++ = cr;
				*b++ = cg;
				*b++ = cb;
			}
		}
		memcpy (d, row, w * 3);
		d += rowstride;
	}

#undef R1
#undef G1
#undef B1
#undef R2
#undef G2
#undef B2

	g_free (row);
}

#define RMAX  (3*1024)
#define RMAX2 (RMAX*RMAX) 

static guchar
shade_pixel (guchar pixel, double shade)
{
  if (shade > 0)
    {
      int tmp = shade * pixel * sqrt(3);
      return MIN (tmp, 255);
    }
  else
    return 0;
}

static void
emboss (GdkPixbuf *image, GdkPixbuf *boss, int x_offset, int y_offset)
{
  int image_width = gdk_pixbuf_get_width (image);
  int image_height = gdk_pixbuf_get_height (image);
  int boss_width = gdk_pixbuf_get_width (boss);
  int boss_height = gdk_pixbuf_get_height (boss);
  int i,j;

  /* Average the three channels of the boss image */
  
  gushort *boss_gray = g_new (gushort, boss_width * boss_height);

  for (j=0; j<boss_height; j++)
    {
      guchar *pixels = gdk_pixbuf_get_pixels (boss) + j * gdk_pixbuf_get_rowstride (boss);
      gushort *line = boss_gray + boss_width * j;

      for (i=0; i<boss_width; i++)
	line[i] = *(pixels++) + *(pixels++) + *(pixels++);
    }

  for (j=1; j<boss_height - 1; j++)
    {
      if (j + y_offset >= 0 && j + y_offset < image_height)
	{
	  guchar *p = gdk_pixbuf_get_pixels (image) + (j + y_offset) * gdk_pixbuf_get_rowstride (image) + 3 * x_offset;
      
	  for (i=1; i<boss_width - 1; i++)
	    {
	      double shade;
	      
	      if (i + x_offset >= 0 && i + x_offset < image_width)
		{
		  int dx = (*(boss_gray + boss_width * j +       i + 1) -
			    *(boss_gray + boss_width * j +       i - 1));
		  int dy = (*(boss_gray + boss_width * (j - 1) + i) -
			    *(boss_gray + boss_width * (j + 1) + i));
		  int dz;
		  
		  int t = dx*dx + dy*dy;

		  if (t > RMAX2)
		    {
		      double sqrt_t = sqrt(t);
		      
		      dz = 0;
		      dx = RMAX * dx / sqrt_t;
		      dy = RMAX * dy / sqrt_t;
		    }
		  else
		    {
		      dz = sqrt (RMAX2 - t);
		    }
	      
		  shade = (double)(dz - dx + dy) / (RMAX * sqrt(3));
		  
		  *(p) = shade_pixel (*p, shade);
		  *(p+1) = shade_pixel (*(p+1), shade);
		  *(p+2) = shade_pixel (*(p+2), shade);
		}
	      p += 3;
	    }

	}
    }

  g_free (boss_gray);
}

GdkPixbuf *
make_background (BGState *state,
		 int      output_width,
		 int      output_height,
		 gboolean can_tile)
{
	GdkPixbuf *image = NULL;
	GdkPixbuf *result = NULL;
	int image_width = 0;
	int image_height = 0;
	
	if (state->bgType == BACKGROUND_WALLPAPER) {
		image = gdk_pixbuf_new_from_file (state->wpFileName);
		if (image) {
			image_width = gdk_pixbuf_get_width (image);
			image_height = gdk_pixbuf_get_height (image);
		} else {
			return NULL;
		}
	}

	if (can_tile) {
		if (state->bgType == WALLPAPER_TILED &&
		    (!state->grad || (image && !gdk_pixbuf_get_has_alpha (image)))) {

			guint32 packed_color = PACKED_COLOR (&state->bgColor1);
			
			result = gdk_pixbuf_composite_color_simple (image, image_width, image_height,
								    GDK_INTERP_NEAREST, 255, 16,
								    packed_color, packed_color);
			
			gdk_pixbuf_unref (image);
			
			return result;
		} else if (state->bgType == BACKGROUND_SIMPLE) {
			if (state->vertical)
				output_width = 128;
			else
				output_height = 128;
		}
	}

	result = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, output_width, output_height);

	if (state->grad)
		fill_gradient (result, &state->bgColor1, &state->bgColor2, state->vertical);
	else
		fill_gradient (result, &state->bgColor1, &state->bgColor1, FALSE);


	if (state->bgType == BACKGROUND_WALLPAPER) {
		double x_scale = (double)output_width / gdk_screen_width ();
		double y_scale = (double)output_height / gdk_screen_height ();
		
		if (state->wpType == WALLPAPER_TILED) {
			int w = image_width * x_scale;
			int h = image_height * y_scale;
			int xoff, yoff;

			for (yoff = 0; yoff < output_height; yoff += h)
				for (xoff = 0; xoff < output_width; xoff += w)
					gdk_pixbuf_composite (image, result,
							      xoff, yoff,
							      MIN (w, output_width-xoff), MIN (h, output_height-yoff),
							      xoff, yoff, x_scale, y_scale,
							      GDK_INTERP_BILINEAR, 255);
		} else if (state->wpType == WALLPAPER_EMBOSSED) {
			GdkPixbuf *boss;
			
			int xoff = output_width - image_width * x_scale - 16 * x_scale;
			int yoff = output_height - image_height * y_scale - 64 * y_scale;

			if (x_scale != 1 || y_scale != 1) 
				boss = gdk_pixbuf_scale_simple (image,
								       image_width * x_scale,
								       image_height * y_scale,
								       GDK_INTERP_BILINEAR);

			else
				boss = gdk_pixbuf_ref (image);

			emboss (result, boss, xoff, yoff);
			gdk_pixbuf_unref (boss);
			
		} else {
			int scaled_width, scaled_height;
			int xoff, yoff;
			
			switch (state->wpType) {
			case WALLPAPER_CENTERED:
				scaled_width = image_width * x_scale;
				scaled_height = image_height * y_scale;
				break;
			case WALLPAPER_SCALED:
				scaled_width = output_width;
				scaled_height = output_height;
				break;
			case WALLPAPER_SCALED_KEEP:
				if (image_height == 0 || image_width == 0) {
					scaled_width = image_width;
					scaled_height = image_height;
				} else {
					double aspect = (double)image_width / image_height;
					if (aspect * output_height <= output_width) {
						scaled_width = aspect * output_height;
						scaled_height = output_height;
					} else {
						scaled_width = output_width;
						scaled_height = output_width / aspect;
					}
				}
				break;
			default:
				g_assert_not_reached();
				scaled_width = 0; /* Quiet gcc */
				scaled_height = 0;
			}

			xoff = (output_width - scaled_width) / 2;
			yoff = (output_height - scaled_height) / 2;

			gdk_pixbuf_composite (image, result,
					      xoff, yoff, scaled_width, scaled_height,
					      xoff, yoff, (double)scaled_width/image_width, (double)scaled_height/image_height,
					      GDK_INTERP_BILINEAR, 255);
		}
	}
	
	if (image)
		gdk_pixbuf_unref (image);
	return result;
}

/* Create a persistant pixmap. We create a separate display
 * and set the closedown mode on it to RetainPermanent
 */
GdkPixmap *
make_root_pixmap (gint width, gint height)
{
	Pixmap result;
	Display *display;

	gdk_flush ();

	display = XOpenDisplay (gdk_display_name);
	XSetCloseDownMode (display, RetainPermanent);

	result = XCreatePixmap (display,
				DefaultRootWindow (display),
				width, height,
				DefaultDepthOfScreen (DefaultScreenOfDisplay (GDK_DISPLAY())));
	XCloseDisplay (display);

	return gdk_pixmap_foreign_new (result);
}

void
dispose_root_pixmap (GdkPixmap *pixmap)
{
	/* Unrefing a foreign pixmap causes it to be destroyed - so we include
	 * this bad hack, that will work for GTK+-1.2 until the problem
	 * is fixed in the next release
	 */

	GdkWindowPrivate *private = (GdkWindowPrivate *)pixmap;
	
	gdk_xid_table_remove (private->xwindow);
	g_dataset_destroy (private);
	g_free (private);

}

/* Set the root pixmap, and properties pointing to it. We
 * do this atomically with XGrabServer to make sure that
 * we won't leak the pixmap if somebody else it setting
 * it at the same time. (This assumes that they follow the
 * same conventions we do
 */
void 
set_root_pixmap (GdkPixmap *pixmap)
{
	GdkAtom type;
	gulong nitems, bytes_after;
	gint format;
	guchar *data_esetroot;
	Pixmap pixmap_id;
	int result;

	XGrabServer (GDK_DISPLAY());

	result = XGetWindowProperty (GDK_DISPLAY(), GDK_ROOT_WINDOW(),
				     gdk_atom_intern("ESETROOT_PMAP_ID", FALSE),
				     0L, 1L, False, XA_PIXMAP,
				     &type, &format, &nitems, &bytes_after,
				     &data_esetroot);

	if (result == Success && type == XA_PIXMAP && format == 32 && nitems == 1) {
		XKillClient(GDK_DISPLAY(), *(Pixmap*)data_esetroot);
	}

	if (data_esetroot != NULL) {
		XFree (data_esetroot);
	}

	if (pixmap != NULL) {
		pixmap_id = GDK_WINDOW_XWINDOW (pixmap);

		XChangeProperty (GDK_DISPLAY(), GDK_ROOT_WINDOW(),
				 gdk_atom_intern("ESETROOT_PMAP_ID", FALSE), 
				 XA_PIXMAP, 32, PropModeReplace,
				 (guchar *) &pixmap_id, 1);
		XChangeProperty (GDK_DISPLAY(), GDK_ROOT_WINDOW(),
				 gdk_atom_intern("_XROOTPMAP_ID", FALSE), 
				 XA_PIXMAP, 32, PropModeReplace,
				 (guchar *) &pixmap_id, 1);

		XSetWindowBackgroundPixmap (GDK_DISPLAY(), GDK_ROOT_WINDOW(), 
					    pixmap_id);
	} else {
		XDeleteProperty (GDK_DISPLAY (), GDK_ROOT_WINDOW (),
				 gdk_atom_intern("ESETROOT_PMAP_ID", FALSE));
		XDeleteProperty (GDK_DISPLAY(), GDK_ROOT_WINDOW(),
				 gdk_atom_intern("_XROOTPMAP_ID", FALSE));
	}

	XClearWindow (GDK_DISPLAY (), GDK_ROOT_WINDOW ());
	XUngrabServer (GDK_DISPLAY());

	XFlush(GDK_DISPLAY());
}

static gboolean
get_use_gdkrgb (void)
{
	static gboolean inited = FALSE;
	static gboolean use_gdkrgb = FALSE;

	if (!inited) {
		gdk_rgb_init ();

		if (gdk_rgb_get_cmap() == gdk_colormap_get_system ()) 
			use_gdkrgb = TRUE;
		else 
			background_imlib_init ();

		inited = TRUE;
	}

	return use_gdkrgb;
}

void 
render_to_drawable (GdkPixbuf *pixbuf,
		    GdkDrawable *drawable, GdkGC *gc,
		    int src_x, int src_y,
		    int dest_x, int dest_y,
		    int width, int height,
		    GdkRgbDither dither,
		    int x_dither, int y_dither)
{
	gboolean use_gdkrgb = get_use_gdkrgb();
	int render_type;

	if (use_gdkrgb) {
		gdk_pixbuf_render_to_drawable (pixbuf, drawable, gc,
					       src_x, src_y, dest_x, dest_y,
					       width, height,
					       dither, x_dither, y_dither);
	} else {
		
		guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
		int width = gdk_pixbuf_get_width (pixbuf);
		int height = gdk_pixbuf_get_height (pixbuf);
		int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
		Pixmap pixmap;

		ImlibImage *image;

		if (width*3 == rowstride)
			image = Imlib_create_image_from_data (imlib_data,
							      pixels, NULL,
							      width, height);
		else {
			/* Hopefully this will never be hit for the actual screen background*/
			
			guchar *data = g_malloc (width * height * 3);
			int i, j;

			if (gdk_pixbuf_get_n_channels (pixbuf) == 3) {
				char *p = pixels;
				char *q = data;
				
				for (i=0; i<height; i++) {
					memcpy (q, p, width*3);
					
					q += width*3;
					p += rowstride;
				}
			} else {
				char *q = data;
				char *p;
				
				for (i=0; i<height; i++) {
					p = pixels + rowstride * i;

					for (j=0; j<width; j++) {
						*q = *p;
						*(q + 1) = *(p + 1);
						*(q + 2) = *(p + 2);
						p += 4;
						q += 3;
					}
				}
			}

			image = Imlib_create_image_from_data (imlib_data,
							      data, NULL,
							      width, height);

			g_free (data);
		}

		render_type = Imlib_get_render_type(imlib_data);
		Imlib_set_render_type (imlib_data,RT_DITHER_TRUECOL);
		Imlib_render (imlib_data, image, width, height);
		Imlib_set_render_type (imlib_data, render_type);
		
		pixmap = Imlib_move_image (imlib_data, image);

		XCopyArea (GDK_DISPLAY (),
			   pixmap, GDK_WINDOW_XWINDOW (drawable),
			   GDK_GC_XGC (gc),
			   src_x, src_y, width, height, dest_x, dest_y);

		Imlib_free_pixmap (imlib_data, pixmap);
		Imlib_destroy_image (imlib_data, image);
	}
}

gulong 
xpixel_from_color (GdkColor *color)
{
	gboolean use_gdkrgb = get_use_gdkrgb();

	if (use_gdkrgb) {
		return gdk_rgb_xpixel_from_rgb (PACKED_COLOR (color));
	} else {
		int r = color->red >> 8;
		int g = color->green >> 8;
		int b = color->blue >> 8;
		
		return Imlib_best_color_match (imlib_data, &r, &g, &b);
	}
}
