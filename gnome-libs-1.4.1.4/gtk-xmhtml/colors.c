/*****
* colors.c : XmHTML color allocation routines
*
* This file Version	$Revision: 1.13.6.2 $
*
* Creation date:		Mon Dec 16 13:57:41 GMT+0100 1996
* Last modification: 	$Date: 2002/01/24 22:00:57 $
* By:					$Author: kmaraas $
* Current State:		$State: Exp $
*
* Author:				newt
* (C)Copyright 1995-1996 Ripley Software Development
* All Rights Reserved
*
* This file is part of the XmHTML Widget Library.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/
/*****
* ChangeLog 
* $Log: colors.c,v $
* Revision 1.13.6.2  2002/01/24 22:00:57  kmaraas
* 2002-01-24  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* colors.c: Fix up earlier fix so things compile.
*
* Revision 1.13.6.1  2002/01/23 18:32:13  kmaraas
* 2002-01-23  Wayne Schuller  <k_wayne@linuxpower.org>
*
* 	* colors.c: (my_get_colors), (set_widget_colors): Just some error
* 	checking for NULL parameters. Helps to avoid
* 	http://bugzilla.gnome.org/show_bug.cgi?id=12493
*
* Revision 1.13  1999/07/29 01:26:28  sopwith
*
*
* Fix all warnings.
*
* Revision 1.12  1998/05/25 02:36:33  sopwith
*
*
* libgnomeui/gnome-dialog.[ch]:
* Fix gnome_dialog_run_modal() to do the "sane thing" (while incorporating
* Havoc's bug fixes):
* 	- Havoc, it is plausible that the programmer might want to
* 	get back a "the user did not press ANY button". Let them handle it,
* 	it's more flexible. Also removed other various overkills.
*
* I think there may still be a possible bug in using gtk_main_quit as a
* delete_event handler - perhaps need a gnome_dialog_delete_event_handler()
* that just calls gtk_main_quit() and returns FALSE (so the dialog doesn't
* get auto-destroyed).
*
* The other changes, I might wind up reverting because, while my tree
* compiles, I have no idea what the xmhtml change is, sorry :(
*
* Revision 1.11  1998/02/12 03:08:30  unammx
* Merge to Koen's XmHTML 1.1.2 + following fixes:
*
* Wed Feb 11 20:27:19 1998  Miguel de Icaza  <miguel@nuclecu.unam.mx>
*
* 	* gtk-forms.c (freeForm): gtk_destroy_widget is no longer needed
* 	with the refcounting changes.
*
* 	* gtk-xmhtml.c (gtk_xmhtml_remove): Only god knows why I was
* 	adding the just removed widget.
*
* Revision 1.10  1998/01/31 00:12:06  unammx
* Thu Jan 29 12:17:07 1998  Federico Mena  <federico@bananoid.nuclecu.unam.mx>
*
* 	* gtk-xmhtml.c (wrap_gdk_cc_get_pixels): Added wrapper function
* 	for gdk_color_context_get_pixels{_incremental}().  This function
* 	will first upscale the color information to 16 bits.  This
* 	function can be removed as described next.
*
* 	* XCC.c: I defined a USE_EIGHT_BIT_CHANNELS macro that makes the
* 	GetPixel functions expect color data to be in [0, 255].  Two
* 	macros, UPSCALE() and DOWNSCALE(), are used in those functions.
* 	When XmHTML is modified to use 16-bit color information, these
* 	macros and the #ifdef parts can be safely removed, as the
* 	functions already operate with 16-bit colors internally.
*
* 	* colors.c (XmHTMLAllocColor): Made this function use 16-bit
*  	values for color matching.
*
* 	* toolkit.h (XCCGetPixelsIncremental): Removed un-needed do{}while(0)
*
* 	* XCC.c (XCCGetPixel): _red/_green/_blue parameters are now
* 	expected to be in [0, 65535].  This is to be friendlier to the Gdk
* 	port of the XCC.
* 	(XCCGetPixels): Made it use 16-bit color values as well.  Fixed
* 	mdist=1000000 buglet (it should start with at least 0x1000000).
* 	(XCCGetPixelsIncremental): Same as for XCCGetPixels().
*
* Revision 1.9  1998/01/15 01:40:58  unammx
* Comment fixes - Federico
*
* Revision 1.8  1998/01/15 01:34:03  unammx
* Wed Jan 14 19:28:01 1998  Federico Mena  <federico@bananoid.nuclecu.unam.mx>
*
* 	* colors.c (my_get_colors): Now we allocate the colors, just as
* 	Motif does.  I think they should be freed sometime, but they are
* 	not.  Lesstif does not free them, either.  I don't know if OSF
* 	Motif ever frees these colors...
*
* Revision 1.7  1998/01/14 04:11:48  unammx
* Tue Jan 13 22:04:43 1998  Federico Mena  <federico@bananoid.nuclecu.unam.mx>
*
* 	* Lots of changes all over the place to fix colors.  Things are
* 	*almost* working right now.  I think I'm only missing setting the
* 	window backgrounds appropriately.  Several things were done:
*
* 		- Motif's color and gc fields from Core and XmManager were
* 		  replicated inside the GtkXmHTML widget structure.
*
* 		- Macros were created in toolkit.h to use these fields.
*
* 		- Instead of the old kludgy set_{fore,back}ground_internal
* 		  functions, we now set the window background directly.
* 		  This does not work perfectly; I'll look into it.
*
* 		- I created a shade_color() function in colors.c (ok, ok,
* 		  I stole it from gtkstyle.c) which mimics XmGetColors()
* 		  -- it calculates shaded colors for the 3D look.
*
* 	I hope to fix the remaining problems with window backgrounds real
* 	soon now.
*
* Revision 1.6  1998/01/10 02:27:02  unammx
* First attempt at fixing the RecomputeColors functions.  They are still
* not perfect, as scrollbar colors are affected, too. - Federico
*
* Revision 1.5  1998/01/07 01:45:36  unammx
* Gtk/XmHTML is ready to be used by the Gnome hackers now!
* Weeeeeee!
*
* This afternoon:
*
* 	- Changes to integrate gtk-xmhtml into an autoconf setup.
*
* 	- Changes to make gtk-xmhtml a library to be used by Gnome
* 	  (simply include <gtk-xmhtml/gtk-xmhtml.h and link
* 	   with -lgtkxmhtml and you are set).
*
* Revision 1.4  1997/12/29 22:16:24  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.3  1997/12/25 01:34:10  unammx
* Good news for the day:
*
*    I have upgraded our XmHTML sources to XmHTML 1.1.1.
*
*    This basically means that we got table support :-)
*
* Still left to do:
*
*    - Set/Get gtk interface for all of the toys in the widget.
*    - Frame support is broken, dunno why.
*    - Form support (ie adding widgets to it)
*
* Miguel.
*
* Revision 1.2  1997/12/18 00:39:21  unammx
* It compiles and links -miguel
*
* Revision 1.1  1997/12/17 04:40:28  unammx
* Your daily XmHTML code is here.  It almost links.  Only the
* images.c file is left to port.  Once this is ported we are all
* set to start debugging this baby.
*
* btw, Dickscrape is a Motif based web browser that is entirely
* based on this widget, I just tested it today, very impressive.
*
* Miguel.
*
* Revision 1.13  1997/10/23 00:24:51  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.12  1997/08/31 17:33:07  newt
* log edit
*
* Revision 1.11  1997/08/30 00:46:39  newt
* Changed _XmHTMLConfirmColor32 proto from void to Boolean. Added
* _XmHTMLRecomputeColors.
*
* Revision 1.10  1997/08/01 12:57:49  newt
* my_strdup -> strdup
*
* Revision 1.9  1997/05/28 01:45:14  newt
* Changes for the XmHTMLAllocColor and XmHTMLFreeColor functions.
*
* Revision 1.8  1997/04/29 14:24:51  newt
* Fix in _XmHTMLFreeColors
*
* Revision 1.7  1997/04/03 05:33:37  newt
* _XmHTMLGetPixelByName is much more robuster and fault tolerant
* (patch by Dick Ported, dick@cymru.net)
*
* Revision 1.6  1997/03/20 08:08:18  newt
* fixed a few bugs in BestPixel and _XmHTMLFreeColors
*
* Revision 1.5  1997/03/02 23:15:32  newt
* some obscure free() changes
*
* Revision 1.4  1997/02/11 02:06:32  newt
* Bugfixes related to color releasing.
*
* Revision 1.3  1997/01/09 06:55:20  newt
* expanded copyright marker
*
* Revision 1.2  1997/01/09 06:43:42  newt
* small fix on ConfirmColor32
*
* Revision 1.1  1996/12/19 02:17:07  newt
* Initial Revision
*
*****/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#ifndef WITH_MOTIF
#include <gdk/gdkx.h>
#endif

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"
#ifdef WITH_MOTIF
#    include "XCCP.h"
#endif

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/
static struct{
	String color;	/* for both #rrggbb and named color specs */
	TColor xcolor;
	int used;		/* color usage counter */
}color_cache[256];

/*** Private Function Prototype Declarations ****/
static int CreateColormap(XmHTMLWidget html, TColor *cmap);

/*** Private Variable Declarations ***/
static int last_color;
static Boolean confirm_warning = True;

/* HTML-3.2 color names */
static String html_32_color_names[16] = {"black", "silver", "gray", 
	"white", "maroon", "red", "purple", "fuchsia", "green", 
	"lime", "olive", "yellow", "navy", "blue", "teal", 
	"aqua"};

/* corresponding HTML-3.2 sRGB values */
static String html_32_color_values[16] = {"#000000", "#c0c0c0", "#808080", 
	"#ffffff", "#800000", "#ff0000", "#800080", "#ff00ff", "#008000", 
	"#00ff00", "#808000", "#ffff00", "#000080", "#0000ff", "#008080", 
	"#00ffff"};

/* for creating a 3/3/2 based palette */
#define RMASK		0xe0
#define RSHIFT		0
#define GMASK		0xe0
#define GSHIFT		3
#define BMASK		0xc0
#define BSHIFT		6

/* XXX: This function does an XQueryColors() the hard way, because there is
 * no corresponding function in Gdk.
 */
#ifndef WITH_MOTIF
static void
my_x_query_colors(GdkColormap *colormap,
		  GdkColor    *colors,
		  gint         ncolors)
{
	XColor *xcolors;
	gint    i;

	xcolors = g_new(XColor, ncolors);
	for (i = 0; i < ncolors; i++)
		xcolors[i].pixel = colors[i].pixel;

	XQueryColors(gdk_display, GDK_COLORMAP_XCOLORMAP(colormap), xcolors, ncolors);

	for (i = 0; i < ncolors; i++) {
		colors[i].red   = xcolors[i].red;
		colors[i].green = xcolors[i].green;
		colors[i].blue  = xcolors[i].blue;
	}

	g_free(xcolors);
}
#endif
/*****
* Name: 		tryColor
* Return Type: 	Boolean
* Description: 	verifies the validity of the given colorname and returns
*				the corresponding RGB components for the requested color.
* In: 
*	dpy:		Display on which color should be allocated;
*	colormap:	colormap in which color should be allocated;
*	color:		name of color to allocate. Either symbolic or an RGB triplet;
*	*defs:		TColor struct for allocated color. Filled upon return.
* Returns:
*	True when color name is valid, False if not.
* Note:
*	This routine tries to recover from incorrectly specified RGB triplets.
*	(not all six fields present).
*****/
static Boolean
tryColor(Display *dpy, TColormap colormap, String color, TColor *def)
{
	/*
	* backup color for stupid html writers that don't use a leading hash.
	* The 000000 will ensure we have a full colorspec if the user didn't
	* specify the full symbolic color name (2 red, 2 green and 2 blue).
	*/
	char hash[]="#000000";
	int i;

	/* first try original name */
	if(!(Toolkit_Parse_Color (dpy, colormap, color, def)))
	{
		/*
		* Failed, see if we have a leading hash. This doesn't work with
		* symbolic color names. Too bad then.
		*/
		if(color[0] != '#')
		{
			/*
			* Only prepend the hash sign by setting the second char to NULL.
			* Can't initialize hash this way (above that is) since the literal
			* copy below won't work that way. Don't ask me why, it just won't
			* work.
			*/
			hash[1] = '\0';
			strncat(hash, color, 6);
		}
		/*
		* Copy up to seven chars. This will make a valid color spec
		* even if the full triplet hasn't been specified. The strlen check
		* to prevent a buffer overflow.
		*/
		else
			/* literal copy so we get a valid triplet */
			if(strlen(color) < 7)
			{
				for(i = 0; i < strlen(color); i++)
					hash[i] = color[i];
			}
			else
				strncpy(hash, color, 7);
		/* NULL terminate */
		hash[7] = '\0';

		/* try again */
		if(!(Toolkit_Parse_Color(dpy, colormap, hash, def)))
			return(False);
	}
	return(True);
}

/*****
* Name: 		_XmHTMLGetPixelByName
* Return Type: 	Pixel
* Description: 	retrieves the pixel value for a color name. Color can be
*				given as a color name as well as a color value.
* In: 
*	display:	display where color value should be retrieved from
*	color:		the color to allocate.
*	def_pixel:	default pixel to return if color allocation fails
* Returns:
*	The pixel value closest to the requested color
*****/
Pixel
_XmHTMLGetPixelByName(XmHTMLWidget html, String color, Pixel def_pixel)
{
	Display *dpy = Toolkit_Display((TWidget)html);
	TColor def;
	int i, slot = -1;
	unsigned short r[1], g[1], b[1];
	TColormap colormap;
	Pixel pixel[1];
	int success = False;	/* REQUIRED */

	/* sanity check */
	if(!color || *color == '\0')
		return(def_pixel);

	/* see if we have an xcc for this widget */
	_XmHTMLCheckXCC(html);
	colormap = Toolkit_Widget_Colormap (html);

	/***** 
	* See if the named color has already been allocated. If so, we don't
	* allocate it again but return the already obtained pixel value.
	* This way we don't have to do an XAllocColor for each color we have
	* already allocated.
	*****/
	for(i = 0 ; i < last_color; i++)
	{
		if(color_cache[i].used && !(strcmp(color_cache[i].color, color)))
		{
			_XmHTMLDebug(7, ("format.c: GetPixelFromName: color %s already "
				"allocated.\n", color));
			color_cache[i].used++;
			return(color_cache[i].xcolor.pixel);
		}
		if(!color_cache[i].used)
			slot = i;
	}
	/* stack full */
	if(last_color > 255 && slot == -1)
		return(def_pixel);

	/* no free slots */
	if(slot == -1)
		slot = last_color;

	if((!tryColor(dpy, colormap, color, &def)))
	{
		Boolean again;

		/* turn of warnings */
		confirm_warning = False;

		/* see if by chance it's one of the 16 appointed colors */
		again = _XmHTMLConfirmColor32(color);

		/* turn on warnings */
		confirm_warning = True;

		/* try it */
		if(!again || !tryColor(dpy, colormap, color, &def))
		{
			/* bad color spec, return */
			_XmHTMLWarning(__WFUNC__(html, "_XmHTMLGetPixelByName"),
				"Bad color name %s", color);
			return(def_pixel);
		}
	}

	r[0] = def.red   >> 8;  /* downscale */
	g[0] = def.green >> 8;
	b[0] = def.blue  >> 8;
	pixel[0] = None;		/* REQUIRED! */

	/* try to allocate it */
	XCCGetPixels(html->html.xcc, r, g, b, 1, pixel, &success);

	if(!success)
	{
		/* failed, return default pixel */
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLGetPixelByName"), 
			"XAllocColor failed for color %s", color);
		return(def_pixel);
	}
	def.pixel = pixel[0];

	/* store in color stack. */
	color_cache[slot].color = strdup(color);
	(void)memset((char*)&color_cache[slot].xcolor, 0, sizeof(TColor));
	(void)memcpy((char*)&color_cache[slot].xcolor, (char*)&def, sizeof(TColor));
	color_cache[slot].used++;

	/* don't forget to set colorstack depth */
	if(slot == last_color)
		last_color++;

	return(def.pixel);
}

/*****
* Name: 		_XmHTMLConfirmColor32
* Return Type: 	void
* Description: 	converts the given named color to the corresponding sRGB value.
* In: 
*	color:		color name to check
* Returns:
*	nothing, but if a match is found, color is updated with the corresponding
*	sRGB value.
* Note:
*	This routine is here for consistency. The standard HTML 3.2 colors are
*	referred to as the ``standard 16 color Windows VGA pallete''. This
*	uttermost *dumb*, *stupid* (you name it) pallete does not only contain an
*	absolute minimum of 16 colors, but in addition, most of the color names 
*	used are unknown to almost all X servers! Can you imagine a greater m$
*	ignorance!!!! Yuck.
*****/
Boolean
_XmHTMLConfirmColor32(char *color)
{
	register int i;

	/* an sRGB spec, see if we know it */
	if(color[0] == '#')
	{
		for(i = 0 ; i < 16; i++)
		{
			if(!strcasecmp(color, html_32_color_values[i]))
				return(True);
		}
	}
	else
	{
		for(i = 0 ; i < 16; i++)
		{
			if(!strcasecmp(color, html_32_color_names[i]))
			{
				color = realloc(color, strlen(html_32_color_values[i]));
				strcpy(color, html_32_color_values[i]);
				color[strlen(html_32_color_values[i])] = '\0';
				return(True);
			}
		}
	}
	/* nope, don't know it. Use black */
	if(confirm_warning)
		_XmHTMLWarning(__WFUNC__(NULL, "_XmHTMLConfirmColor32"), 
			"HTML 3.2 color violation: color %s not known, ignoring.\n",
			color);
	return(False);
}

/*****
* Name: 		_XmHTMLFreeColors
* Return Type: 	void
* Description: 	frees all colors we have allocated in the color cache
* In: 
*	w:			XmHTMLWidget
* Returns:
*	nothing
* Note:
*	color releasing is done only if the usage counter of a certain color reaches
*	zero, in case no more widgets are using it and thus can be safely destroyed.
*****/
void
_XmHTMLFreeColors(XmHTMLWidget html)
{
	int i, freed = 0;

	/* free all colors we have allocated */
	for(i = 0 ; i < last_color; i++)
	{
		/* only decrease if this color is actively being used */
		if(color_cache[i].used)
			color_cache[i].used--;
		/* only destroy if the usage counter of this color reaches zero */
		if(color_cache[i].color && !color_cache[i].used)
		{
			free(color_cache[i].color);
			color_cache[i].color = NULL;
			freed++;
		}
	}
	/* no more colors in stack, reset depth */
	if(freed == last_color)
		last_color = 0;
	/* 
	* not required to reset color stack depth. first free slot allocation
	* is used. Stack growth direction is variable.
	*/
}

#ifndef WITH_MOTIF

/* This color shading method is taken from gtkstyle.c.  Some
 *  modifications made by me - Federico
 */

static void
rgb_to_hls (gdouble *r, gdouble *g, gdouble *b)
{
	gdouble min;
	gdouble max;
	gdouble red;
	gdouble green;
	gdouble blue;
	gdouble h, l, s;
	gdouble delta;

	red = *r;
	green = *g;
	blue = *b;

	if (red > green) {
		if (red > blue)
			max = red;
		else
			max = blue;

		if (green < blue)
			min = green;
		else
			min = blue;
	} else {
		if (green > blue)
			max = green;
		else
			max = blue;

		if (red < blue)
			min = red;
		else
			min = blue;
	}

	l = (max + min) / 2;
	s = 0;
	h = 0;

	if (max != min) {
		if (l <= 0.5)
			s = (max - min) / (max + min);
		else
			s = (max - min) / (2 - max - min);

		delta = max -min;
		if (red == max)
			h = (green - blue) / delta;
		else if (green == max)
			h = 2 + (blue - red) / delta;
		else if (blue == max)
			h = 4 + (red - green) / delta;

		h *= 60;
		if (h < 0.0)
			h += 360;
	}

	*r = h;
	*g = l;
	*b = s;
}

static void
hls_to_rgb (gdouble *h, gdouble *l, gdouble *s)
{
	gdouble hue;
	gdouble lightness;
	gdouble saturation;
	gdouble m1, m2;
	gdouble r, g, b;

	lightness = *l;
	saturation = *s;

	if (lightness <= 0.5)
		m2 = lightness * (1 + saturation);
	else
		m2 = lightness + saturation - lightness * saturation;
	m1 = 2 * lightness - m2;

	if (saturation == 0) {
		*h = lightness;
		*l = lightness;
		*s = lightness;
	} else {
		hue = *h + 120;
		while (hue > 360)
			hue -= 360;
		while (hue < 0)
			hue += 360;

		if (hue < 60)
			r = m1 + (m2 - m1) * hue / 60;
		else if (hue < 180)
			r = m2;
		else if (hue < 240)
			r = m1 + (m2 - m1) * (240 - hue) / 60;
		else
			r = m1;

		hue = *h;
		while (hue > 360)
			hue -= 360;
		while (hue < 0)
			hue += 360;

		if (hue < 60)
			g = m1 + (m2 - m1) * hue / 60;
		else if (hue < 180)
			g = m2;
		else if (hue < 240)
			g = m1 + (m2 - m1) * (240 - hue) / 60;
		else
			g = m1;

		hue = *h - 120;
		while (hue > 360)
			hue -= 360;
		while (hue < 0)
			hue += 360;

		if (hue < 60)
			b = m1 + (m2 - m1) * hue / 60;
		else if (hue < 180)
			b = m2;
		else if (hue < 240)
			b = m1 + (m2 - m1) * (240 - hue) / 60;
		else
			b = m1;

		*h = r;
		*l = g;
		*s = b;
	}
}

static void
shade_color(GdkColor *a, GdkColor *b, gdouble k)
{
	gdouble red;
	gdouble green;
	gdouble blue;

	/* Special case for black, so that it looks pretty... */

	if ((a->red == 0) && (a->green == 0) && (a->blue == 0)) {
		a->red = 32768;
		a->green = 32768;
		a->blue = 32768;
	}

	red = (gdouble) a->red / 65535.0;
	green = (gdouble) a->green / 65535.0;
	blue = (gdouble) a->blue / 65535.0;

	rgb_to_hls (&red, &green, &blue);

	green *= k;

	if (green > 1.0)
		green = 1.0;
	else if (green < 0.0)
		green = 0.0;

	blue *= k;

	if (blue > 1.0)
		blue = 1.0;
	else if (blue < 0.0)
		blue = 0.0;

	hls_to_rgb (&red, &green, &blue);

	b->red = (int) (red * 65535.0 + 0.5);
	b->green = (int) (green * 65535.0 + 0.5);
	b->blue = (int) (blue * 65535.0 + 0.5);
}

#define HIGHLIGHT_MULT 0.7
#define LIGHT_MULT     1.5
#define DARK_MULT      0.5

static void
my_get_colors(GdkColormap *colormap, gulong background, gulong *top, gulong *bottom, gulong *highlight)
{
	GdkColor cbackground;
	GdkColor ctop, cbottom, chighlight;

	if (colormap == NULL)
		return;

	/* I think this should use a ColorContext instead of allocating colors itself... - Federico */

	/* FIXME: The colors that are allocated here are never freed.
	 * I think we can save the pixel values in static variables so that we
	 * can call gdk_colors_free() on the next invocation of the function.
	 */

	cbackground.pixel = background;
	my_x_query_colors(colormap, &cbackground, 1);

	if (top) {
		shade_color(&cbackground, &ctop, LIGHT_MULT);
		gdk_color_alloc(colormap, &ctop);
		*top = ctop.pixel;
	}

	if (bottom) {
		shade_color(&cbackground, &cbottom, DARK_MULT);
		gdk_color_alloc(colormap, &cbottom);
		*bottom = cbottom.pixel;
	}

	if (highlight) {
		shade_color(&cbackground, &chighlight, HIGHLIGHT_MULT);
		gdk_color_alloc(colormap, &chighlight);
		*highlight = chighlight.pixel;
	}
}

static void
set_widget_colors(GtkXmHTML *html, gulong *top, gulong *bottom, gulong *highlight)
{
	GdkColor c;

	if (html == NULL)
		return;

	if (top) {
		c.pixel = *top;
		gdk_gc_set_foreground(html->top_shadow_gc, &c);
	}

	if (bottom) {
		c.pixel = *bottom;
		gdk_gc_set_foreground(html->bottom_shadow_gc, &c);
	}

	if (highlight) {
		c.pixel = *highlight;
		gdk_gc_set_foreground(html->highlight_gc, &c);
		html->highlight_color = *highlight;
	}
}

#endif

/*****
* Name: 		_XmHTMLRecomputeColors
* Return Type: 	void
* Description: 	computes new values for top and bottom shadows and the
*				highlight color based on the current background color.
* In: 
*	html:		XmHTMLWidget id;
* Returns:
*	nothing.
*****/
void
_XmHTMLRecomputeColors(XmHTMLWidget html) 
{
	/* 
	* We can only compute the colors when we have a GC. If we don't
	* have a GC, the widget is not yet realized. Use managers defaults
	* then.
	*/
	if(html->html.gc != NULL)
	{
#ifdef WITH_MOTIF
		Pixel top = None, bottom = None, highlight = None;
		Arg args[3];

		XmGetColors(XtScreen((Widget)html), html->core.colormap,
			html->html.body_bg, NULL, &top, &bottom, &highlight);
		XtSetArg(args[0], XmNtopShadowColor, top);
		XtSetArg(args[1], XmNbottomShadowColor, bottom);
		XtSetArg(args[2], XmNhighlightColor, highlight);
		XtSetValues((Widget)html, args, 3);
#else
		gulong top, bottom, highlight;
		if(!GTK_WIDGET_REALIZED(html))
		  gtk_widget_realize(GTK_WIDGET(html));
		my_get_colors(gtk_widget_get_colormap(GTK_WIDGET(html)), html->html.body_bg, &top, &bottom, &highlight);
		set_widget_colors(html, &top, &bottom, &highlight);
#endif
	}
}

/*****
* Name: 		_XmHTMLRecomputeHighlightColor
* Return Type: 	void
* Description: 	computes the select color based upon the given color.
* In: 
*	html:		XmHTMLWidget id;
* Returns:
*	nothing.
*****/
void
_XmHTMLRecomputeHighlightColor(XmHTMLWidget html, Pixel bg_color) 
{
	/* 
	* We can only compute the colors when we have a GC. If we don't
	* have a GC, the widget is not yet realized. Use managers defaults
	* then.
	*/
	if(html->html.gc != NULL)
	{
#ifdef WITH_MOTIF
		Pixel highlight = None;
		Arg args[1];

		XmGetColors(XtScreen((Widget)html), html->core.colormap,
			bg_color, NULL, NULL, NULL, &highlight);
		XtSetArg(args[0], XmNhighlightColor, highlight);
		XtSetValues((Widget)html, args, 1);
#else
		gulong highlight;

		my_get_colors(gtk_widget_get_colormap(GTK_WIDGET(html)), html->html.body_bg, NULL, NULL, &highlight);
		set_widget_colors(html, NULL, NULL, &highlight);
#endif
	}
}

/*****
* Name: 		XmHTMLAllocColor
* Return Type: 	Pixel
* Description: 	allocates the named color and takes the XmNmaxImageColors
*				resource into account.
* In: 
*	w:			XmHTMLWidget id;
*	color:		colorname, either symbolic or an RGB triplet.
*	def_pixel:	pixel to return when allocation of "color" fails.
* Returns:
*	allocated pixel upon success or def_pixel upon failure to allocate "color".
*****/
Pixel
XmHTMLAllocColor(TWidget w, String color, Pixel def_pixel)
{
	Display *dpy = Toolkit_Display(w);
	TColor def;
	TColormap colormap;
	int success = True;

	/* sanity check */
	if(!w || !color || *color == '\0')
	{
		_XmHTMLWarning(__WFUNC__(w, "XmHTMLAllocColor"), "%s passed to "
			"XmHTMLAllocColor.", w ? "NULL color name" : "NULL parent");
		return(def_pixel);
	}

	/*
	* Get colormap for this widget. Will always succeed as all widgets
	* (and gadgets as well) are subclassed from core.
	*/
	colormap = Toolkit_Widget_Colormap (w);

	if((!tryColor(dpy, colormap, color, &def)))
	{
		/* bad color spec, return */
		_XmHTMLWarning(__WFUNC__(w, "XmHTMLAllocColor"), "Bad color "
			"name %s", color);
		return(def_pixel);
	}

	/* try to allocate it */
	if(!Toolkit_Alloc_Color(dpy, colormap, &def))
	{
		/*
		* Initial allocation failed, try to find a close match from any
		* colors already allocated in the colormap
		*/
		TColor *cmap;
		int cmapsize;
		TVisual *visual = NULL;
		int d, mdist, close, ri, gi, bi;
		register int i, rd, gd, bd;

		_XmHTMLDebug(7, ("colors.c: XmHTMLAllocColor: first stage allocation "
			"for %s failed, trying to match it.\n", color));

		Toolkit_Get_Visual (w, visual);

		/*
		* Get parent visual if current widget doesn't have one. This will
		* *always* return a visual.
		*/
		if(!visual)
			visual = XCCGetParentVisual(w);

		/* we only use up to MAX_IMAGE_COLORS */
#ifdef WITH_MOTIF
		cmapsize = (visual->map_entries > MAX_IMAGE_COLORS ?
			    MAX_IMAGE_COLORS : visual->map_entries);
#else
		cmapsize = MIN(GDK_VISUAL_XVISUAL(visual)->map_entries, MAX_IMAGE_COLORS);
#endif

		cmap = (TColor*)malloc(cmapsize*sizeof(TColor));

		/* initialise pixels */
		for(i = 0; i < cmapsize; i++)
		{
			cmap[i].pixel = (Pixel)i;
			cmap[i].red = cmap[i].green = cmap[i].blue = 0;
#ifdef WITH_MOTIF
			cmap[i].flags = DoRed|DoGreen|DoBlue;
#endif
		}

		/* read the colormap */
#ifdef WITH_MOTIF
		XQueryColors(dpy, colormap, cmap, cmapsize);
#else
		my_x_query_colors(colormap, cmap, cmapsize);
#endif

		mdist = 0x1000000;
		close = -1;

		ri = def.red;
		gi = def.green;
		bi = def.blue;

		/* 
		* walk all colors in the colormap and see which one is the 
		* closest. Uses plain least squares.
		*/
		for(i = 0; i < cmapsize && mdist != 0; i++)
		{
			/* Don't replace these by shifts; the sign may get clobbered */

			rd = (ri - cmap[i].red) / 256;
			gd = (gi - cmap[i].green) / 256;
			bd = (bi - cmap[i].blue) / 256;

			if((d = (rd*rd) + (gd*gd) + (bd*bd)) < mdist)
			{
				close = i;
				mdist = d;
			}
		}
		if(close != -1)
		{
			/* we got a match, try to allocate this color */
			def.red   = cmap[close].red;
			def.green = cmap[close].green;
			def.blue  = cmap[close].blue;
#ifdef WITH_MOTIF
			def.flags = DoRed|DoGreen|DoBlue;
#endif
			if(!Toolkit_Alloc_Color(dpy, colormap, &def))
				success = False;
		}
		else
			success = False;

		/* no longer needed */
		free(cmap);
	}
	if(success == False)
	{
		/* failed, return default pixel */
		_XmHTMLWarning(__WFUNC__(w, "_XmHTMLGetPixelByName"), 
			"XmHTMLAllocColor failed for color %s", color);
		return(def_pixel);
	}
	_XmHTMLDebug(7, ("colors.c: XmHTMLAllocColor: %s allocated!\n", color));
	return(def.pixel);
}

/*****
* Name: 		XmHTMLFreeColor
* Return Type: 	void
* Description: 	releases an allocated pixel
* In: 
*	w:			XmHTMLWidget id;
*	pixel:		pixel to be freed.
* Returns:
*	nothing.
*****/
void
XmHTMLFreeColor(TWidget w, Pixel pixel)
{
	/* sanity check */
	if(!w)
	{
		_XmHTMLBadParent(w, "XmHTMLFreeColor");
		return;
	}

	/*
	* ->core.colormap will always yield a colormap, all widgets are
	* subclassed from core.
	*/
#ifdef WITH_MOTIF
	XFreeColors(XtDisplay(w), w->core.colormap, &pixel, 1, 0L);
#else
	gdk_colors_free(gtk_widget_get_colormap(w), &pixel, 1, 0L);
#endif
}

Boolean
_XmHTMLAddPalette(XmHTMLWidget html)
{
	TColor cmap[MAX_IMAGE_COLORS];
	int ncolors = 0, nlines = 0;
	int i,r,g,b;
	String chPtr;

	if(html->html.palette != NULL)
	{
		chPtr = html->html.palette;
		/* skip leading whitespace */
		while(*chPtr != '\0' && isspace(*chPtr))
		{
			if(*chPtr == '\n')
				nlines++;
			chPtr++;
		}
		while(*chPtr != '\0' && ncolors < MAX_IMAGE_COLORS)
		{
			if((sscanf(chPtr, "%x %x %x", &r, &g, &b)) != 3)
			{
				_XmHTMLWarning(__WFUNC__(html, "_XmHTMLAddPalette"),
					"Bad color entry on line %i of palette.", nlines);
				/* skip to next entry */
				while(*chPtr != '\0' && !isspace(*chPtr))
					chPtr++;
			} 
			else
			{
				RANGE(r,0,255);
				RANGE(g,0,255);
				RANGE(b,0,255);
				cmap[ncolors].red   = r;
				cmap[ncolors].green = g;
				cmap[ncolors].blue  = b;
				ncolors++;
				/* skip them */
				for(i = 0; i < 3; i++)
				{
					while(*chPtr != '\0' && isalnum(*chPtr))
						chPtr++;
					while(*chPtr != '\0' && isspace(*chPtr))
					{
						if(*chPtr == '\n')
							nlines++;
						chPtr++;
					}
				}
			}
			/* move to next slot */
			while(*chPtr != '\0' && isspace(*chPtr))
			{
				if(*chPtr == '\n')
					nlines++;
				chPtr++;
			}
		}

		/* check against maxImageColors */
		if(ncolors != html->html.max_image_colors)
		{
			if(ncolors < html->html.max_image_colors)
				html->html.max_image_colors = ncolors;
			else
			{
				/* check how many colors are really allowed on this display */
				if(ncolors < html->html.xcc->num_colors)
					html->html.max_image_colors = ncolors;
				else
					ncolors = html->html.max_image_colors;
			}
		}
	}
	else
		ncolors = CreateColormap(html, &cmap[0]);

	/* allocate this palette */
	ncolors = XCCAddPalette(html->html.xcc, cmap, ncolors);

	/* check if we need to initialize dithering */
	if(html->html.map_to_palette == XmBEST ||
		html->html.map_to_palette == XmFAST)
		XCCInitDither(html->html.xcc);

	_XmHTMLDebug(7, ("colors.c, _XmHTMLAddPalette, added a palette with %i "
		"colors\n", ncolors));

	return(True);
}

#ifdef DITHER_SIMPLE_COLORMAP
static int
CreateColormap(XmHTMLWidget html, TColor *cmap)
{
	int i, idx, ncolors;
	float mul;

	ncolors = html->html.max_image_colors;
	mul = (float)MAX_IMAGE_COLORS/(float)html->html.max_image_colors;

	if(html->html.xcc->mode == MODE_BW || html->html.xcc->mode == MODE_MY_GRAY)
	{
		/* grayscale visual */
		for (i = 0;  i < ncolors;  ++i)
		{
			idx = (int)(i * mul);
			cmap[i].red   = idx;
			cmap[i].green = idx;
			cmap[i].blue  = idx;
		}
	}
	else
	{
		for(i = 0; i < ncolors; i++)
		{
			idx = (int)(i * mul);
			cmap[i].red  = (((idx << 0) & 0xe0)*255 + 0.5*0xe0)/0xe0;
			cmap[i].green= (((idx << 3) & 0xe0)*255 + 0.5*0xe0)/0xe0;
			cmap[i].blue = (((idx << 6) & 0xc0)*255 + 0.5*0xc0)/0xc0;
		}
	}
	return(ncolors);
}

#else
/*****
* Name:			CreateColormap
* Return Type: 	int
* Description: 	creates a colormap (with equally spaced color components)
* In: 
*	html:		XmHTMLWidget id;
*	cmap:		colormap storage room. Filled upon return.
* Returns:
*	actual size of colormap. This is at most equal to the value of the
*	XmNmaxImageColors resource (unless the value of this resource is less than
*	8, in which case it will be set to 8).
*****/
static int
CreateColormap(XmHTMLWidget html, TColor *cmap)
{
	int iroot, nc, max_colors, blksize, blkdist, nci, val, maxsample;
	int i, j, k, l, total_colors, Ncolors[3], temp;
	static int RGB[3] = {1, 0, 2};
	Boolean changed;
	Byte **colormap;

#ifdef WITH_MOTIF
	/* number of components per color */
	if(html->html.xcc->mode == MODE_BW || html->html.xcc->mode == MODE_MY_GRAY)
		nc = 1;	/* grayscale */
	else
		nc = 3;	/* color */
#else
	{
		GdkColorContextMode mode;
		
		mode = html->html.xcc->mode;

		if (mode == GDK_CC_MODE_BW || mode == GDK_CC_MODE_MY_GRAY)
			nc = 1;
		else
			nc = 3;
	}
#endif
	
	/*****
	* requested colormap size.
	* To get an even distribution of the colors, we require this value to be
	* a triple power, with a minumum of 8 colors. 
	*****/
	max_colors = html->html.max_image_colors;
	if(max_colors < 8)
		max_colors = 8;

	iroot = 1;
	do
	{
		iroot++;
		temp = iroot;
		for(i = 1; i < nc; i++)
			temp *= iroot;
	}while(temp <= max_colors);
	iroot--;

	/* Set number of distinct values for each color component */
	total_colors = 1;
	for(i = 0; i < nc; i++)
	{
		Ncolors[i] = iroot;
		total_colors *= iroot;
	}

	/*****
	* See if we can increase the number of distinct color components
	* without exceeding the allowed number of colors.
	*****/
	do
	{
		changed = False;
		for(i = 0; i < nc; i++)
		{
			j = (nc == 1 ? 0 : RGB[i]);
			temp = total_colors/Ncolors[j];
			temp *= Ncolors[j]+1;
			if(temp > max_colors)
				break;	/* too large, done with this pass */
			Ncolors[j]++;
			total_colors = (int)temp;
			changed = True;
		}
	}while(changed);

	if(total_colors != html->html.max_image_colors)
	{
		_XmHTMLWarning(__WFUNC__(html, "makeDitherCmap"),
			"Requested XmNmaxImageColors value of %i could not be matched "
			"exactly.\n    Using %i colors out of %i total.",
			html->html.max_image_colors, total_colors, MAX_IMAGE_COLORS);
		html->html.max_image_colors = total_colors;
	}

	/* temporary storage */
	colormap = (Byte**)calloc(nc, sizeof(Byte*));
	for(i = 0; i < nc; i++)
		colormap[i] = (Byte*)calloc(total_colors, sizeof(Byte));

	/* distance between groups of identical entries for a component */
	blkdist = total_colors;

	/* maximum value of a single color component */
	maxsample = MAX_IMAGE_COLORS-1;

	/* now go and fill the palette */
	for(i = 0; i < nc; i++)
	{
		/* fill in entries for i'th color */
		nci = Ncolors[i];	/* no of distinct values for this color */
		blksize = blkdist/nci;
		for(j = 0; j < nci; j++)
		{
			/* get color value */
			val = (int)(((unsigned long)(j * maxsample + (nci-1)/2))/ (nci-1));
			/* fill all entries that have this value for this component. */
			for(k = j * blksize; k < total_colors; k+= blkdist)
			{
				/* fill in blksize entries starting at k */
				for(l = 0; l < blksize; l++)
					colormap[i][k+l] = (Byte)val;
			}
		}
		blkdist = blksize;	/* size of this color is offset to next color */
	}
	/* now save colormap in private storage */
	if(nc == 1) /* grayscale */
	{
		for(i = 0; i < total_colors; i++)
			cmap[i].red = cmap[i].green = cmap[i].blue = colormap[0][i];
	}
	else	/* rgb map */
	{
		for(i = 0; i < total_colors; i++)
		{
			cmap[i].red   = colormap[0][i];
			cmap[i].green = colormap[1][i];
			cmap[i].blue  = colormap[2][i];
		}
	}
	/* no longer needed */
	for(i = 0; i < nc; i++)
		free(colormap[i]);
	free(colormap);

	/* all done */
	return(total_colors);
}
#endif
