/*****
* fonts.c : XmHTML font loading & caching routines.
*
* This file Version	$Revision: 1.10.6.1 $
*
* Creation date:		Mon Sep 22 09:46:00 GMT+0100 1997
* Last modification: 	$Date: 2001/10/20 06:52:12 $
* By:					$Author: kmaraas $
* Current State:		$State: Exp $
*
* Author:				newt
*
* Copyright (C) 1994-1997 by Ripley Software Development 
* All Rights Reserved
*
* This file is part of the XmHTML Widget Library
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
* $Log: fonts.c,v $
* Revision 1.10.6.1  2001/10/20 06:52:12  kmaraas
* 2001-10-20  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* *.*: Apply all the Red Hat patches.
*
* Revision 1.10  1999/07/29 01:26:28  sopwith
*
*
* Fix all warnings.
*
* Revision 1.9  1999/06/02 01:00:38  unammx
*
* 1999-06-01  Akira Higuchi <a-higuti@math.sci.hokudai.ac.jp>
*
* 	* libgnomeui/gnome-canvas-text.c:
* 	* libgnomeui/gnome-icon-item.c:
* 	* libgnomeui/gnome-less.c: Replace some gdk_font_load() calls with
* 	gdk_fontset_load.    Use a more open fontset rule to load the fonts.
*
* 1999-06-01  Akira Higuchi <a-higuti@math.sci.hokudai.ac.jp>
*
* 	* gtk-xmhtml/XmHTMLP.h: Add three members lbearing, rbearing,
* 	and width. These members are computed in allocFont().
*
* 	* gtk-xmhtml/toolkit.h: Remove Toolkit_XFont() macro.
*
* 	* gtk-xmhtml/XmHTML.c:
* 	* gtk-xmhtml/fonts.c:
* 	* gtk-xmhtml/format.c:
* 	* gtk-xmhtml/gtk-xmhtml.c:
* 	* gtk-xmhtml/layout.c:
* 	* gtk-xmhtml/paint.c: Add fontset support. We use gdk_fontset_load()
* 	instead of gdk_font_load() iff a fontset is supplied for label
* 	widgets.
*
* 	* gtk-xmhtml/test.c: Add gtk_set_locale() call before gtk_init().
*
* Revision 1.8  1999/02/25 01:05:06  unammx
* Missing bit of the strtok patches from Ulrich
*
* Revision 1.7  1998/05/11 07:54:07  kmaraas
* 1998-05-11  Kjartan Maraas  <kmaraas@fib.hl.no>
*
* 	* Removed two compiler warnings in gnome-libs/gtk-xmhtml/
* 	fonts.c and gtk-forms.c.
*
* Revision 1.6  1998/02/12 03:08:37  unammx
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
* Revision 1.3  1997/12/25 01:38:29  unammx
* Small bug fixes
*
* Revision 1.2  1997/12/25 01:34:10  unammx
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
* Revision 1.1  1997/11/28 03:38:56  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.1  1997/10/23 00:23:09  newt
* Initial Revision
*
*****/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>		/* isspace */
#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"

#ifndef WITH_MOTIF
#    include <gdk/gdkprivate.h>
#endif

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/
int xmhtml_fn_sizes[8], xmhtml_basefont_sizes[7], xmhtml_fn_fixed_sizes[2];

/*** Private Datatype Declarations ****/
/*****
* A single font cache entry.
* The font cache is a binary tree, sorted according to the name of a font.
* The name field points to a field in the font field.
*****/
typedef struct _fontCacheEntry{
	XmHTMLfont *font;				/* font data */
	String name;					/* font name */
	Boolean is_map;					/* true when this is a font mapping */
	XmHTMLfont *map_to;				/* ptr to real font data */
	struct _fontCacheEntry *left;
	struct _fontCacheEntry *right;
}fontCacheEntry;

/*****
* Definition of a display-bound font cache.
* This structure contains the ID of the display this cache is used for,
* the actual font cache, the default font for this display and a list of
* widgets referencing this cache.
*****/
typedef struct _fontCache{
	Display *dpy;					/* display were fonts come from */
	int res_x;						/* horizontal screen resolution */
	int res_y;						/* vertical screen resolution */
	fontCacheEntry *cache;			/* cached fonts for this display */
	XmHTMLfont *default_font;		/* default font */
	int nwidgets;					/* no of widgets referring this cache */
	TWidgetList widgets;				/* array of widget referring this cache */
	struct _fontCache *next;		/* ptr to next cache */
	int nentries;					/* no of cached fonts */
	int nmaps;						/* no of mapped fonts */
	int nlookups;					/* no of search actions */
	int requests;					/* no of requests made */
	int hits;						/* no of hits */
	int misses;						/* no of missed requests */
}fontCache;

/*** Private Function Prototype Declarations ****/
/* create a fully valid XLFD */
static String makeFontName(String name, String foundry, String family,
	String weight, String slant, int points, String charset, String fam_return);

static fontCacheEntry *insertFont(fontCacheEntry *entry, String name,
	XmHTMLfont *font, XmHTMLfont *map_to);

/* get a font from the cache */
static XmHTMLfont *getFont(fontCacheEntry *entry, String name, Byte style);

/* create a map for the given font */
static XmHTMLfont *mapFont(XmHTMLfont *font, String name);

/* allocate a new XmHTMLfont entry */
static XmHTMLfont *allocFont(TFontStruct *xfont, String name, String family,
	Byte style);

/* load or get a font from the cache */
static XmHTMLfont *loadAndCacheFont(TWidget w, String name, String family,
	Byte style);

/* free all cached fonts for the given display */
static void freeFontEntries(Display *dpy, fontCacheEntry *fonts);

/* (re-)initialize a font cache */
static void initializeFontSizeLists(XmHTMLWidget html);

/*** Private Variable Declarations ***/
static fontCache *master_cache;		/* master font cache */
static fontCache *curr_cache;		/* current font cache */

/* Backup lists when sizes are not specified */
static int def_fn_sizes[8] = {110,80,240,160,160,140,140,140};
static int def_fn_fixed_sizes[2] = {110,80};

/*****
* Name: 		makeFontName
* Return Type: 	String
* Description: 	creates a full 14 field font name from the given args.
* In: 
*	name:		font base name, required
*	foundry:	font foundry, optional
*	family:		font family, optional
*	weight:		font weight, required
*	slant:		font slant, required
*	points:		font pointsize (tenths of a point), required
*	charset:	current ISO character set encoding
*	fam_return: XmHTML fontFamily spec, updated upon return.
* Returns:
*	the composed font name
* Note:
*	The name argument contains the following string:
*		foundry-family-width-spacing
*	The charset contains the last two fields of a valid font name:
*		iso9959-1
*	When family is specified, foundry will be replaced by a wildcard
*
*	this routine will compose the following fontname given the basename:
*	-foundry-family-weight-slant-width--*-points-res_x-res_y-spacing-*-charset
*****/
static String 
makeFontName(String name, String foundry, String family, String weight, 
	String slant, int points, String charset,
	String fam_return)
{
	int i;
	static char fontfam[512], new_name[1024];
	String fndry, fam, wd, sp;
	GtkWidget *dummy_label;
	gboolean use_fontset = 0;

	strncpy(fontfam, name, 511);
	fontfam[strlen(name)] = '\0';

	/* foundry */
	fndry = &fontfam[0];

	/* family */
	for(i = 0; fontfam[i] != '\0' && fontfam[i] != '-'; i++);
	fontfam[i++] = '\0';
	fam = &fontfam[i];

	/* set width */
	for(; fontfam[i] != '\0' && fontfam[i] != '-'; i++);
	fontfam[i++] = '\0';
	wd = &fontfam[i];

	/* spacing */
	for(; fontfam[i] != '\0' && fontfam[i] != '-'; i++);
	fontfam[i++] = '\0';
	sp = &fontfam[i];

	_XmHTMLFullDebug(8, ("fonts.c: makeFontName, split fontFamily "
		"value %s:\nfoundry : %s\nfamily : %s\nwidth : %s\nspacing : %s\n",
		name, fndry, fam, wd, sp));

	/* determine whether we use a font or a fontset */
	dummy_label = gtk_label_new("Dummy");
	gtk_widget_ensure_style(dummy_label);
	use_fontset = (dummy_label->style->font->type == GDK_FONT_FONTSET);
	gtk_object_destroy(GTK_OBJECT(dummy_label));

	/* screen resolutions are stored in the display-bound font cache */
	if (use_fontset) {
		/* we try to load fonts close to the specified one */
		/* FIXME: better way to determine the fontset name? */
		sprintf(new_name,
			"-%s-%s-%s-%s-%s-*-*-%i-%i-%i-%s-*-%s,"
			"-*-*-%s-%s-%s-*-%i-*-*-*-*-*-*,"
			"-*-*-*-*-*-*-%i-*-*-*-*-*-*,*",
			foundry != NULL ? foundry : fndry,
			family != NULL ? family : fam, 
			weight, slant, wd, points,
			curr_cache->res_x, curr_cache->res_y, sp, charset, 
			weight, slant, wd, points / 10,
			points / 10);
	} else {
		sprintf(new_name,
			"-%s-%s-%s-%s-%s-*-*-%i-%i-%i-%s-*-%s",
			foundry != NULL ? foundry : fndry,
			family != NULL ? family : fam, 
			weight, slant, wd, points,
			curr_cache->res_x, curr_cache->res_y, sp, charset);
	}

	/* create XmHTML fontFamily spec for this font */
	sprintf(fam_return, "%s-%s-%s-%s", (foundry != NULL ? foundry : fndry),
		(family != NULL ? family : fam), wd, sp);

	/* make it all lowercase */
	my_locase(new_name);

	return(new_name);
}

/*****
* Name: 		insertFont
* Return Type: 	fontCacheEntry
* Description: 	inserts the given font in the given font cache.
* In: 
*	entry:		current font cache;
*	name:		name of font to insert;
*	font:		font data to be inserted;
*	map_to:		original font data if this is a mapping;
* Returns:
*	updated cache entry;
*****/
static fontCacheEntry*
insertFont(fontCacheEntry *entry, String name, XmHTMLfont *font, 
	XmHTMLfont *map_to)
{
	if(entry == NULL)
	{
		/* allocate new font entry */
		entry = (fontCacheEntry*)malloc(sizeof(fontCacheEntry));
		entry->name   = font->font_name;
		entry->font   = font;
		entry->is_map = map_to != NULL;
		entry->map_to = map_to;
		entry->left   = (fontCacheEntry*)NULL;
		entry->right  = (fontCacheEntry*)NULL;
	}
	else
	{
		int ret_val = strncmp(name, entry->name, strlen(name));

		/* duplicate font entries can exist, so check the style as well */
		if(ret_val == 0 && entry->font->style == font->style)
			return(entry);
		if(ret_val < 0)
			entry->left = insertFont(entry->left, name, font, map_to);
		else
			entry->right = insertFont(entry->right, name, font, map_to);
	}
	return(entry);
}

/*****
* Name:			getFont
* Return Type: 	XmHTMLfont
* Description: 	looks for a font in the fontcache;
* In: 
*	entry:		current font cache;
*	name:		name of font to locat;
* Returns:
*	a valid font if name is found, NULL if not found.
*****/
static XmHTMLfont*
getFont(fontCacheEntry *entry, String name, Byte style)
{
	if(entry != NULL)
	{
		int ret_val = strncmp(name, entry->name, strlen(name));
		curr_cache->nlookups++;

		/*****
		* We want the styles to match as well, _XmHTMLloadQueryFont is
		* a bit too smart sometimes.
		*****/
		if(ret_val == 0 && style == entry->font->style)
		{
			_XmHTMLDebug(8,("already cached.\n"));

			if(entry->map_to)
			{
				_XmHTMLDebug(8, ("\t(mapped to %s)\n",
					entry->map_to->font_name));
				return(entry->map_to);
			}
			else
				return(entry->font);
		}
		if(ret_val < 0)
			return(getFont(entry->left, name, style));
		else
			return(getFont(entry->right, name, style));
	}
	return((XmHTMLfont*)NULL);
}

/*****
* Name: 		mapFont
* Return Type:	XmHTMLfont*
* Description: 	creates a font mapping;
* In: 
*	font:		data to which ``name'' is mapped;
*	name:		name of font
* Returns:
*	a new font entry;
*****/
static XmHTMLfont*
mapFont(XmHTMLfont *font, String name)
{
	static XmHTMLfont *map;

	map = (XmHTMLfont*)malloc(sizeof(XmHTMLfont));

	/* copy everything */
	memcpy(map, font, sizeof(XmHTMLfont));

	/* override name */
	map->font_name = strdup(name);
	return(map);
}

/*****
* Name: 		allocFont
* Return Type: 	XmHTMLfont
* Description: 	allocates a new font entry and retrieves all required
*				font properties;
* In: 
*	xfont:		ptr to an X font;
*	name:		name of this font;
*	family:		family to which this font belongs;
*	style:		style of this font, see the FONT_ defines in XmHTMLP.h
* Returns:
*	a new font entry;
*****/
static XmHTMLfont*
allocFont(TFontStruct *xfont, String name, String family, Byte style)
{
	static XmHTMLfont *font;
	unsigned long value = 0;
	XFontStruct *xfs = NULL;
	int mb_ascent, mb_descent;

#ifdef WITH_MOTIF
	xfs = xfont;
#else	
	switch (xfont->type) {
	case GDK_FONT_FONT:
		xfs = (XFontStruct *)(((GdkFontPrivate *)xfont)->xfont);
		break;
	case GDK_FONT_FONTSET:
		xfs = NULL;
		break;
	default:
		printf ("Passed a non-font to allocFont!\n");
		exit (1);
	}
#endif

	font = (XmHTMLfont*)malloc(sizeof(XmHTMLfont));

	/* default items */
	font->xfont = xfont;
	font->font_name = strdup(name);
	font->font_family = strdup(family);
	font->style = style;

	/* size of largest character */
	if (xfs)
	{
		mb_ascent = xfs->max_bounds.ascent;
		mb_descent = xfs->max_bounds.descent;
		font->width = xfs->max_bounds.width;
		font->lbearing = xfs->max_bounds.lbearing;
		font->rbearing = xfs->max_bounds.rbearing;
	}
#ifndef WITH_MOTIF
	else
	{
		XFontStruct **fsl;
		char **names;
		int n, i;
		XFontSet fset;
		fset = (XFontSet)(((GdkFontPrivate *)xfont)->xfont);
		n = XFontsOfFontSet(fset, &fsl, &names);
		font->width = font->rbearing = font->lbearing = 1;
		mb_ascent = mb_descent = 0;
		for (i = 0; i < n; i++) {
			XCharStruct mb;
			mb = fsl[i]->max_bounds;
			font->lbearing = MIN(font->lbearing, mb.lbearing);
			font->rbearing = MAX(font->rbearing, mb.rbearing);
			font->width = MAX(font->width, mb.width);
			mb_ascent = MAX(mb.ascent, mb_ascent);
			mb_descent = MAX(mb.descent, mb_descent);
		}
	}
#endif
	font->height = mb_ascent + mb_descent;

	/* suggested lineheight */
	font->lineheight = xfont->ascent + xfont->descent;

	/* now go get a bunch of properties */

	/* normal interword spacing */
	if (xfs && XGetFontProperty(xfs, XA_NORM_SPACE, &value) == True)
		font->isp = (Cardinal)value;
	else
        {
                /* use width of a single space */
#ifdef WITH_MOTIF
		int dir, ascent, descent;
		XCharStruct sp;
		XTextExtents(XF, " ", 1, &dir, &ascent, &descent, &sp);
		font->isp = sp.width;
#else
		font->isp = gdk_char_width(xfont, ' ');
#endif
        }

	/* additional end-of-line spacing */
	if (xfs && XGetFontProperty(xfs, XA_END_SPACE, &value) == True)
		font->eol_sp = (Cardinal)value;
	else
		font->eol_sp = 0;

	/* superscript x-offset */
		font->sup_xoffset = 0;

	/* superscript y-offset */
	if (xfs && XGetFontProperty(xfs, XA_END_SPACE, &value) == True)
		font->sup_yoffset = (int)value;
	else
		font->sup_yoffset = (int)(mb_ascent  * -.4);

	/* subscript x-offset */
	if (xfs && XGetFontProperty(xfs, XA_SUBSCRIPT_X, &value) == True)
		font->sub_xoffset = (int)value;
	else
		font->sub_xoffset = 0;

	/* subscript y-offset */
	if (xfs && XGetFontProperty(xfs, XA_SUBSCRIPT_Y, &value) == True)
		font->sub_yoffset = (int)value;
	else
		font->sub_yoffset = (int)(mb_descent * .8);

	/* underline offset */
	if (xfs && XGetFontProperty(xfs, XA_UNDERLINE_POSITION, &value) == True)
		font->ul_offset = (int)value;
	else
		font->ul_offset = (int)(mb_descent-2);

	/* underline thickness */
	if (xfs && XGetFontProperty(xfs, XA_UNDERLINE_THICKNESS, &value) == True)
		font->ul_thickness = (Cardinal)value;
	else
		font->ul_thickness = (Cardinal)1;

	/* strikeout offset */
	if(xfs && XGetFontProperty(xfs, XA_STRIKEOUT_ASCENT, &value) == True)
		font->st_offset = (int)value;
	else
		font->st_offset = (int)(0.5*(mb_ascent))+3;

	/* strikeout descent */
	if(xfs && XGetFontProperty(xfs, XA_STRIKEOUT_DESCENT, &value) == True)
		font->st_thickness = font->st_offset + (Cardinal)value;
	else
		font->st_thickness = 1;

	return(font);
} 

/*****
* Name:			loadAndCacheFont
* Return Type: 	XmHTMLfont
* Description: 	retrieves a font from the cache or loads one when it isn't
*				already available.
* In: 
*	w:			XmHTMLWidget id;
*	name:		name of font to be loaded;
*	family:		family to which this font belongs;
*	style:		style of this font, see the FONT_ defines in XmHTMLP.h;
* Returns:
*	a valid font if the font was loaded successfully, NULL if not (which
*	should never happen);
*****/
static XmHTMLfont*
loadAndCacheFont(TWidget w, String name, String family, Byte style)
{
	XmHTMLfont *font;
	TFontStruct *xfont;

	_XmHTMLDebug(8,( "fonts.c: loadAndCacheFont: checking fontcache for\n%s: ",
		name));

	curr_cache->requests++;

	/* check if we have loaded this font before */
	if((font = getFont(curr_cache->cache, name, style)) != NULL)
	{
		curr_cache->hits++;
		return(font);
	}
	curr_cache->misses++;

	_XmHTMLDebug(8,( "not cached,\ntrying to load..."));

#ifdef WITH_MOTIF
	/* A new font, try to load it */
	xfont = XLoadQueryFont(XtDisplay(w), name);
#else
	if (strchr(name, ',')) {
		_XmHTMLDebug(8, ( "a fontset..." ));
		xfont = gdk_fontset_load (name);
	} else {
		_XmHTMLDebug(8, ( "a font..." ));
		xfont = gdk_font_load (name);
	}
#endif
	/* store it if successfull */
	if(xfont != NULL)
	{
		_XmHTMLDebug(8,( "found.\n"));

		/* get a new fontentry */
		font = allocFont(xfont, name, family, style);

		/* store in the cache */
		curr_cache->nentries++;
		curr_cache->cache = insertFont(curr_cache->cache, name, font, NULL);

		/* return the new font */
		return(font);
	}
#ifdef DEBUG
	else
		_XmHTMLDebug(8,( "failed.\n"));
#endif
	return((XmHTMLfont*)NULL);
}

/*****
* Name:			_XmHTMLloadQueryFont
* Return Type:	XFontStruct*
* Description:	loads a font from the given family in given size, weight and 
*				slant. Loaded fonts are cached to minimize the overhead spent 
*				in XLoadQueryFont().
* In:
*	w:			Widget for which this font is to be loaded.
*	name:		XmHTML fontFamily spec. Only used when family is NULL.
*	family:		font family name of the font to load. When non-null this
*				contains the typeface of the font, e.i.: helvetica, symbol,
*				etc...
*	ptsz:		size of font to load, in tenths of a point
*	style:		style of this font.
*	*loaded:	indicates whether the requested font was loaded or the current 
*				font was returned. When loaded is initially True, a warning
*				message is displayed if the font can't be loaded.
* Returns:
*	A XFontStruct* for the font in the requested family/size and loaded
*	set to True or the current font and loaded set to False.
* Note: 
*	This routine was based on the LoadQueryScalableFont() routine found in
*	O'Reilly's Xlib Programming Manual by Adrian Nye, but that's no longer
*	recognizable...
*
*	This routine goes through *GREAT* lengths to find the requested font, and
*	it will almost never fail (unless the XmNfontFamily/XmNcharset resources
*	form an invalid pair, but then XmHTML will give up on startup immediatly).
*****/
XmHTMLfont*
_XmHTMLloadQueryFont(TWidget w, String name, String family, int ptsz,
	Byte style, Boolean *loaded)
{
	String weight, slant, fontname = NULL, charset = NULL;
	XmHTMLWidget html = (XmHTMLWidget)w;
	char fontfamily[1024], font_mapping[1024];
	XmHTMLfont *font = NULL;

	font_mapping[0] = '\0';

	/***** 
	* Okay, now we are going to try and load a font. 
	* Check weight & slant styles. The order in which they are treated
	* is important: bold overrides any FONT_MEDIUM settings and italic 
	* overrides any FONT_REGULAR settings.
	* First attempts are all made with given charset. If no font is found
	* we wildcard it and try all over again.
	*****/
	if(style & FONT_BOLD)
	{
		int num_total = 0;
		
		while(num_total != 2 && font == NULL)
		{
			int num_outer = 0;

			charset = (num_total == 0 ? html->html.charset : "*-*");

			num_total++;

			while(num_outer != 4 && font == NULL)
			{
				int num_inner = 0;

				/* weight can vary between bold, demibold, medium, regular */
				switch(num_outer)
				{
					case 0 : weight = "bold"    ; break;
					case 1 : weight = "demibold"; break;
					case 2 : weight = "medium"  ; break;
					default: weight = "regular" ; break;
				}

				num_outer++;

				/* slant can vary between italic, oblique and roman */
				if(style & FONT_ITALIC)
				{
					while(num_inner < 3 && font == NULL)
					{
						switch(num_inner)
						{
							case 0 : slant = "i"; break; /* italic */
							case 1 : slant = "o"; break; /* oblique */
							default: slant = "r"; break; /* roman */
						}

						num_inner++;

						fontname = makeFontName(name, family ? "*" : NULL,
							family, weight, slant, ptsz, charset, fontfamily);

						font = loadAndCacheFont(w, fontname, fontfamily, style);
						if(font == NULL && font_mapping[0] == '\0')
						{
							strcpy(font_mapping, fontname);
							font_mapping[strlen(fontname)] = '\0';
						}
					}
				}
				else
				{
					slant = "r"; /* roman */

					fontname = makeFontName(name, family ? "*" : NULL,
						family, weight, slant, ptsz, charset, fontfamily);

					font = loadAndCacheFont(w, fontname, fontfamily, style);
					if(font == NULL && font_mapping[0] == '\0')
					{
						strcpy(font_mapping, fontname);
						font_mapping[strlen(fontname)] = '\0';
					}
				}
			}
		}
	}
	/* regular font style */
	else
	{
		int num_total = 0;

		while(num_total != 2 && font == NULL)
		{
			int num_outer = 0;

			charset = (num_total == 0 ? html->html.charset : "*-*");

			num_total++;

			while(num_outer != 2 && font == NULL)
			{
				int num_inner = 0;

				/* weight can vary between medium and regular */
				if(num_outer == 0)
					weight = "medium";
				else
					weight = "regular";

				num_outer++;

				/* slant can vary between italic, oblique and roman */
				if(style & FONT_ITALIC)
				{
					while(num_inner < 3 && font == NULL)
					{
						switch(num_inner)
						{
							case 0 : slant = "i"; break; /* italic */
							case 1 : slant = "o"; break; /* oblique */
							default: slant = "r"; break; /* roman */
						}

						num_inner++;

						fontname = makeFontName(name, family ? "*" : NULL,
							family, weight, slant, ptsz, charset, fontfamily);

						font = loadAndCacheFont(w, fontname, fontfamily, style);
						if(font == NULL && font_mapping[0] == '\0')
						{
							strcpy(font_mapping, fontname);
							font_mapping[strlen(fontname)] = '\0';
						}
					}
				}
				else
				{
					slant = "r"; /* roman */

					fontname = makeFontName(name, family ? "*" : NULL,
						family, weight, slant, ptsz, charset, fontfamily);

					font = loadAndCacheFont(w, fontname, fontfamily, style);
					if(font == NULL && font_mapping[0] == '\0')
					{
						strcpy(font_mapping, fontname);
						font_mapping[strlen(fontname)] = '\0';
					}
				}
			}
		}
	}

	if(font)
	{
		/*****
		* If the requested font was mapped to another font, store the
		* mapping as well since it will be the same for all subsequent
		* requests for this font.
		* loaded is False only when we are just attempting to load a font
		* and we want this thing to fail, so we sure as hell don't want a
		* default mapping then.
		*****/
		if(font_mapping[0] != '\0' && *loaded == False)
		{
			XmHTMLfont *map = mapFont(font, font_mapping);
			curr_cache->nentries++;
			curr_cache->nmaps++;
			curr_cache->cache = insertFont(curr_cache->cache, font_mapping,
									map, font);
		}
		/* we have the font */
		*loaded = True;

		/* return the new font */
		return(font);
	}

	/* we don't have the font */
	if(*loaded)
	{
		_XmHTMLWarning(__WFUNC__(w, "_XmHTMLloadQueryFont"), "Failed to load "
			"font %s\n    Font probably doesn't exist. Ignored.", fontname);
	}

	*loaded = False;

	/* fix 02/03/07-01, dp */
	return(curr_cache->default_font);
}

/*****
* Name:			_XmHTMLaddFontMapping
* Return Type: 	void
* Description: 	add a fontmapping for the the given font. The name of the
*				font to be mapped is created in such a way that it will
*				be the very first match when the _XmHTMLloadQueryFont routine
*				is called. It's primary use is to reduce loading times when
*				switching between documents (we already know which font we will
*				get).
* In: 
*	font:		actual font.
* Returns:
*	nothing.
*****/
void
_XmHTMLaddFontMapping(XmHTMLWidget html, String name, String family,
	int ptsz, Byte style, XmHTMLfont *font)
{
	String fontname = NULL;
	char fontfamily[1024];
	XmHTMLfont *map;

	/*****
	* Create an XLFD that will match on the first run of _XmHTMLloadQueryFont.
	* !!!INCREDIBLE SPEEDUP!!!
	*****/
	fontname = makeFontName(name, family ? "*" : NULL, family,
			style & FONT_BOLD ? "bold" : "medium", 
			style & FONT_ITALIC ? "i"  : "r", ptsz, html->html.charset,
			fontfamily);

	/* add a mapping */
	map = mapFont(font, fontname);

	curr_cache->nentries++;
	curr_cache->nmaps++;
	curr_cache->cache = insertFont(curr_cache->cache, fontname, map, font);
}

/*****
* Name:			freeFontEntries
* Return Type: 	void
* Description: 	releases all fonts in the given cache;
* In: 
*	dpy:		display on which the fonts were allocated;
*	fonts:		cache to be freed;
* Returns:
*	nothing.
*****/
static void
freeFontEntries(Display *dpy, fontCacheEntry *fonts)
{
	if(fonts)
	{
		freeFontEntries(dpy, fonts->left);
		freeFontEntries(dpy, fonts->right);

		_XmHTMLDebug(8, ("fonts.c: freeFontEntries, releasing font\n\t%s\n",
			fonts->font->font_name));

		/* only free the font if it isn't a mapped one */
		if(!fonts->is_map)
		{
			Toolkit_Free_Font(dpy, fonts->font->xfont);
			free(fonts->font->font_family);
		}

		/* free all allocated strings */
		free(fonts->font->font_name);

		/* free XmHTMLfont entry */
		free(fonts->font);

		/* free cache entry */
		free(fonts);
	}
}

/*****
* Name: 		initializeFontSizeLists
* Return Type: 	void
* Description: 	fills all arrays of font sizes.
* In: 
*	w:			widget containing font size specs.
* Returns:
*	nothing, but the font lists are updated to reflect the new sizes.
*
* The static size lists can cause unexpected results when multiple instances
* of the Widget with different sizelists are being used.
*****/
static void
initializeFontSizeLists(XmHTMLWidget html)
{
	char *chPtr;
	char size_list[64];
	int i;
	Boolean ok;
	char *tokp;

	_XmHTMLDebug(8,( "fonts.c: initializeFontSizeLists Start\n"));

	/*** Scalable font size list ***/

	/* copy name, it gets destroyed */
	(void)memset(&size_list, 0, 64);
	strncpy(size_list, html->html.font_sizes, 63);	

	/* This list has 8 elements */
	for(chPtr = strtok_r(size_list, ",", &tokp), i = 0; i < 8 && chPtr != NULL;
		chPtr = strtok_r(NULL, ",", &tokp), i++)
	{
		if((xmhtml_fn_sizes[i] = 10*atoi(chPtr)) == 0)
			xmhtml_fn_sizes[i] = def_fn_sizes[i];
	}
	/* fill up list if it is not complete */
	if(i != 8)
	{
		for(; i < 8; i++)
			xmhtml_fn_sizes[i] = def_fn_sizes[i];
	}

#ifdef DEBUG
	_XmHTMLDebug(8, ( "fonts.c: initializeFontSizeLists, scalable font "
		"size list:\n"));
	for(i = 0 ; i < 8 ; i++)
		_XmHTMLDebug(8,("%i ", xmhtml_fn_sizes[i]));
#endif

	/*** Fixed font size list ***/

	/* copy name, it gets destroyed */
	(void)memset(&size_list, 0, 64);
	strncpy(size_list, html->html.font_sizes_fixed, 63);

	/* This list has 2 elements */
	for(chPtr = strtok_r(size_list, ",", &tokp), i = 0; i < 2 && chPtr != NULL;
		chPtr = strtok_r(NULL, ",", &tokp), i++)
	{
		if((xmhtml_fn_fixed_sizes[i] = 10*atoi(chPtr)) == 0)
			xmhtml_fn_fixed_sizes[i] = def_fn_fixed_sizes[i];
	}
	/* fill up list if it is not complete */
	if(i != 2)
	{
		for(; i < 2; i++)
			xmhtml_fn_fixed_sizes[i] = def_fn_fixed_sizes[i];
	}

	_XmHTMLDebug(8, ( "\nfonts.c: initializeFontSizeLists, fixed font "
		"size list:\n"));
	_XmHTMLDebug(8, ( "%i %i", xmhtml_fn_fixed_sizes[0],
		xmhtml_fn_fixed_sizes[1]));

	/* list of possible font de/increments using the <FONT SIZE=""> element */
	xmhtml_basefont_sizes[0] = xmhtml_fn_sizes[1];	/* sub/superscript size */
	xmhtml_basefont_sizes[1] = xmhtml_fn_sizes[7];	/* H6 size */
	xmhtml_basefont_sizes[2] = xmhtml_fn_sizes[6];	/* H5 size */
	xmhtml_basefont_sizes[3] = xmhtml_fn_sizes[5];	/* H4 size */
	xmhtml_basefont_sizes[4] = xmhtml_fn_sizes[4];	/* H3 size (def font size)*/
	xmhtml_basefont_sizes[5] = xmhtml_fn_sizes[3];	/* H2 size */
	xmhtml_basefont_sizes[6] = xmhtml_fn_sizes[2];	/* H1 size */

#ifdef DEBUG
	_XmHTMLDebug(8, ( "\nfonts.c: initializeFontSizeLists, fallback "
		"font size list:\n"));
	for(i = 0 ; i < 7 ; i++)
		_XmHTMLDebug(8,( "%i ", xmhtml_basefont_sizes[i]));
	_XmHTMLDebug(8, ("\n"));
#endif

	/* First try to load the default font as specified by the resources */
	ok = False;

	html->html.default_font = _XmHTMLloadQueryFont((TWidget)html,
		html->html.font_family, NULL, xmhtml_fn_sizes[0],
		FONT_SCALABLE|FONT_REGULAR|FONT_MEDIUM, &ok);

	/***** 
	* We can't load the default font, try again with a wildcarded family.
	* This time die if it fails
	*****/
	if(html->html.default_font == NULL)
	{
		_XmHTMLWarning(__WFUNC__(html, "initializeFontSizeLists"),
			"Failed to load default font %s\n    Guessing for a default font.",
			html->html.font_family);

		ok = True;
		html->html.default_font = _XmHTMLloadQueryFont((TWidget)html,
			html->html.font_family, "*", xmhtml_fn_sizes[0],
			FONT_SCALABLE|FONT_REGULAR|FONT_MEDIUM, &ok);

		/* too bad, we absolutely need it */
		if(ok == False)
		{
			/* die */
			_XmHTMLError(__WFUNC__(html,"initializeFontSizeLists"),
				"Failed to find a default font for %s\n    Check previous "
				"messages and adjust default font", html->html.font_family);
		}
	}

	_XmHTMLDebug(8,( "\nfonts.c: initializeFontSizeLists end.\n"));
}

/*****
* Name:			_XmHTMLSelectFontCache
* Return Type: 	XmHTMLfont*
* Description: 	selects a cache according to the display a widget is
*				being displayed on (or creates one if it isn't present yet)
* In: 
*	html:		XmHTMLWidget id;
*	reset:		hard reset flag (used by SetValues when any of the font
*				resources changes);
* Returns:
*	the id of the default font for this cache;
*****/
XmHTMLfont*
_XmHTMLSelectFontCache(XmHTMLWidget html, Boolean reset)
{
	Display *dpy = Toolkit_Display((TWidget)html);
	fontCache *cache;

	_XmHTMLDebug(8, ("fonts.c: _XmHTMLSelectFontCache start\n"));

	for(cache = master_cache; cache != NULL && cache->dpy != dpy;
		cache = cache->next);

	if(cache == NULL)
	{
		int screen = DefaultScreen(dpy);
		cache = (fontCache*)malloc(sizeof(fontCache));
		cache->dpy = dpy;
		cache->cache = (fontCacheEntry*)NULL;
		cache->default_font = (XmHTMLfont*)NULL;
		cache->nwidgets = 1;
		cache->widgets = (TWidgetList)malloc(sizeof(TWidget));
		cache->widgets[0] = (TWidget)html;
		cache->next = (fontCache*)NULL;

		/* obtain screen resolution in dpi */
		cache->res_x =
			DisplayWidth(dpy, screen)/(DisplayWidthMM(dpy,screen)/25.4);
		cache->res_y =
			DisplayHeight(dpy, screen)/(DisplayHeightMM(dpy,screen)/25.4);

		/* adjust resolutions */
		cache->res_x = (cache->res_x < 87 ? 75 : 100);
		cache->res_y = (cache->res_y < 87 ? 75 : 100);

		/* make sure we have the same resolution in both directions */
		if(cache->res_x != cache->res_y)
		{
			if(cache->res_x > cache->res_y)
				cache->res_y = cache->res_x;
			else
				cache->res_x = cache->res_y;
		}

		cache->nentries = 0;
		cache->nmaps    = 0;
		cache->nlookups = 0;
		cache->requests = 0;
		cache->hits     = 0;
		cache->misses   = 0;

		if(master_cache)
		{
			fontCache *tmp;
			for(tmp = master_cache; tmp->next != NULL; tmp = tmp->next);
			tmp->next = cache;
		}
		else
			master_cache = cache;
		_XmHTMLDebug(8, ("fonts.c: _XmHTMLSelectFontCache, created first "
			"entry.\n"));
	}
	else
	{
		int i;

		/* see if we have got a reference for this widget */
		for(i = 0; i < cache->nwidgets && cache->widgets[i] != (TWidget)html;
			i++);

		if(i == cache->nwidgets)
		{
			_XmHTMLDebug(8, ("fonts.c: _XmHTMLSelectFontCache, adding "
				"reference entry for widget %s.\n", Toolkit_Widget_Name((TWidget)html)));

			cache->widgets = (TWidgetList)realloc(cache->widgets,
				(cache->nwidgets+1)*sizeof(TWidget));
			cache->widgets[cache->nwidgets++] = (TWidget)html;
		}
	}

	/*****
	* Only initialize font lists if the cache has changed, when we
	* are forced to do a reset or we haven't got a default font.
	*****/
	if(curr_cache != cache || reset || html->html.default_font == NULL)
	{
		curr_cache = cache;
		initializeFontSizeLists(html);
	}
	curr_cache->default_font = html->html.default_font;

	return(curr_cache->default_font);
}

/*****
* Name: 		_XmHTMLLoadFont
* Return Type: 	XmHTMLfont*
* Description: 	loads a new font, with the style determined by the current 
*				font: if current font is bold, and new is italic then a 
*				bold-italic font will be returned.
* In: 
*	w:			Widget for which to load a font
*	font_id:	id describing type of font to load.
*	size:		size of font to load. Only used for HT_FONT.
*	curr_font:	current font, required for propagating font style info.
* Returns:
*	the loaded font.
*****/
XmHTMLfont*
_XmHTMLLoadFont(XmHTMLWidget html, htmlEnum font_id, int size,
	XmHTMLfont *curr_font)
{
	XmHTMLfont *new_font = NULL;
	String family;
	int ptsz;
	Byte new_style = (Byte)0, font_style;
	Boolean ok = True;	/* enable font warnings */

	/* curr_font *must* always have a value as it references a cached font */
	my_assert(curr_font != NULL);

	/* pick up style of the current font */
	font_style = curr_font->style;

	_XmHTMLDebug(8,("_XmHTMLLoadFont: current font is %s %s %s.\n",
		(font_style & FONT_FIXED  ? "fixed"  : "scalable"),
		(font_style & FONT_BOLD   ? "bold"   : "medium"),
		(font_style & FONT_ITALIC ? "italic" : "regular"))); 

	/* See if we need to proceed with bold font */
	if(font_style & FONT_BOLD)
		new_style = FONT_BOLD;
	else
		new_style &= ~FONT_BOLD;

	/* See if we need to proceed with italic font */
	if(font_style & FONT_ITALIC)
		new_style |= FONT_ITALIC;
	else
		new_style &= ~FONT_ITALIC;

	/* See if we need to proceed with a fixed font */
	if(font_style & FONT_FIXED)
	{
		new_style |= FONT_FIXED;
		family = html->html.font_family_fixed;
		ptsz = xmhtml_fn_fixed_sizes[0];
	}
	else
	{
		new_style &= ~FONT_FIXED;
		family = curr_font->font_family;
		ptsz = xmhtml_fn_sizes[0];
	}

	_XmHTMLDebug(8,("_XmHTMLLoadFont: next font is %s %s %s (inherited).\n",
		(new_style & FONT_FIXED  ? "fixed"  : "scalable"),
		(new_style & FONT_BOLD   ? "bold"   : "medium"),
		(new_style & FONT_ITALIC ? "italic" : "regular"))); 

	switch(font_id)
	{
		case HT_CITE:
		case HT_I:
		case HT_EM:
		case HT_DFN:
		case HT_ADDRESS:
			new_font = _XmHTMLloadQueryFont((TWidget)html, family, NULL, 
				xmhtml_basefont_sizes[size-1], new_style|FONT_ITALIC, &ok);  
			break;
		case HT_STRONG:
		case HT_B:
		case HT_CAPTION:
			new_font = _XmHTMLloadQueryFont((TWidget)html, family, NULL, 
				xmhtml_basefont_sizes[size-1], new_style | FONT_BOLD, &ok);
			break;

		/*****
		* Fixed fonts always use the font specified by the value of the
		* fontFamilyFixed resource.
		*****/
		case HT_SAMP:
		case HT_TT:
		case HT_VAR:
		case HT_CODE:
		case HT_KBD:
 		case HT_PRE:	/* fix 01/20/97-03, kdh */
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family_fixed, NULL, xmhtml_fn_fixed_sizes[0],
				new_style |FONT_FIXED, &ok);
			break;

		/* The <FONT> element is useable in *every* state */
		case HT_FONT:
			new_font = _XmHTMLloadQueryFont((TWidget)html, family, NULL, size,
				new_style, &ok);
			break;

		/*****
		* Since HTML Headings may not occur inside a <font></font> declaration,
		* they *must* use the specified document font, and not derive their
		* true font from the current font.
		*****/
		case HT_H1:
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family, NULL, xmhtml_fn_sizes[2],
				FONT_SCALABLE|FONT_BOLD, &ok);
			break;
		case HT_H2:
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family, NULL, xmhtml_fn_sizes[3],
				FONT_SCALABLE|FONT_BOLD, &ok);
			break;
		case HT_H3:
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family, NULL, xmhtml_fn_sizes[4],
				FONT_SCALABLE|FONT_BOLD, &ok);
			break;
		case HT_H4:
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family, NULL, xmhtml_fn_sizes[5],
				FONT_SCALABLE|FONT_BOLD, &ok);
			break;
		case HT_H5:
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family, NULL, xmhtml_fn_sizes[6],
				FONT_SCALABLE|FONT_BOLD, &ok);
			break;
		case HT_H6:
			new_font = _XmHTMLloadQueryFont((TWidget)html,
				html->html.font_family, NULL, xmhtml_fn_sizes[7],
				FONT_SCALABLE|FONT_BOLD, &ok);
			break;

		/* should never be reached */
		default:
#ifdef PEDANTIC
			_XmHTMLWarning(__WFUNC__(html, "_XmHTMLLoadFont"), 
				"Unknown font switch. Using default font.");
#endif /* PEDANTIC */
			/* this will always succeed */
			ok = False;
			new_font = _XmHTMLloadQueryFont((TWidget)html, family, NULL, ptsz, 
				FONT_SCALABLE|FONT_REGULAR|FONT_MEDIUM, &ok);
			break;
	}
	return(new_font);
}

/*****
* Name: 		_XmHTMLLoadFontWithFace
* Return Type: 	XmHTMLfont*
* Description: 	load a new font with given pixelsize and face. 
*				Style is determined by the current font: if current font
*				is bold, and new is italic then a bold-italic font will be 
*				returned.
* In: 
*	w:			Widget for which to load a font
*	size:		size of font to load. Only used for HT_FONT.
*	face:		a comma separated list of font faces to use, contents are 
*				destroyed when this function returns.
*	curr_font:	current font, required for propagating font style info.
* Returns:
*	A new font with a face found in the list of faces given upon success
*	or the default font on failure.
*****/
XmHTMLfont*
_XmHTMLLoadFontWithFace(XmHTMLWidget html, int size, String face,
	XmHTMLfont *curr_font)
{
	XmHTMLfont *new_font = NULL;
	String chPtr, family, all_faces, first_face = NULL;
	Byte new_style = (Byte)0, font_style;
	int try;
	char *tokp;

	/* curr_font *must* always have a value as it references a cached font */
	my_assert(curr_font != NULL);

	/* pick up style of the current font */
	font_style = curr_font->style;

	/* See if we need to proceed with bold font */
	if(font_style & FONT_BOLD)
		new_style = FONT_BOLD;
	else
		new_style &= ~FONT_BOLD;

	/* See if we need to proceed with italic font */
	if(font_style & FONT_ITALIC)
		new_style |= FONT_ITALIC;
	else
		new_style &= ~FONT_ITALIC;

	/***** 
	* See if we need to proceed with a fixed font, only used to determine
	* initial font family.
	*****/
	if(font_style & FONT_FIXED)
	{
		new_style |= FONT_FIXED;
		family = html->html.font_family_fixed;
	}
	else
	{
		new_style &= ~FONT_FIXED;
		family = html->html.font_family;
	}

	/* we must have a ``,'' or strtok will fail */
	if((strstr(face, ",")) == NULL)
	{
		all_faces = (String)malloc(strlen(face) + 2);
		strcpy(all_faces, face);
		strcat(all_faces, ",\0");
	}
	else
		all_faces = strdup(face);

	/* walk all possible spaces */
	try = 0;
	for(chPtr = strtok_r(all_faces, ",", &tokp); chPtr != NULL;
		chPtr = strtok_r(NULL, ",", &tokp))
	{
		Boolean ok = False;

		try++;

		/* skip any leading spaces */
		while(isspace(*chPtr))
			chPtr++;

		_XmHTMLDebug(8, ("format.c: _XmHTMLLoadFontWithFace, trying with "
			"face %s\n", chPtr));

		/***** 
		* Disable font not found warning message, we are trying to find
		* a font of which we don't know if it exists.
		*****/
		ok = False;
		new_font = _XmHTMLloadQueryFont((TWidget)html, family, chPtr, size,
			new_style, &ok);
		if(new_font && ok)
		{
			_XmHTMLDebug(8, ("format.c: _XmHTMLLoadFontWithFace, font "
				"loaded.\n"));
			break;
		}
		if(try == 1)
			first_face = strdup(chPtr);
	}
	free(all_faces);
	/*****
	* hmm, the first font in this face specification didn't yield a valid
	* font. To speed up things considerably, we add a font mapping for the
	* first face in the list of given spaces. There's no sense in doing this
	* when there is only one face specified as this will always get us the
	* default font. We only add a mapping if the name of the returned font
	* contains at least one of the allowed faces. Not doing this check would
	* ignore face specs which do have a face we know. We also want the font
	* styles to match as well.
	* BTW: this is a tremendous speedup!!!
	*****/
	if(first_face)
	{
		/*****
		* Only add a mapping if the returned name contains one of the allowed
		* faces. No need to check for the presence of a comma: we only take
		* lists that have multiple face specifications.
		*****/
		if(try > 1)
		{
			/*****
			* Walk all possible faces. Nukes the face array but that's not
			* bad as we are the only ones using it.
			*****/
			for(chPtr = strtok_r(face, ",", &tokp); chPtr != NULL;
				chPtr = strtok_r(NULL, ",", &tokp))
			{
				/* skip any leading spaces */
				while(isspace(*chPtr))
					chPtr++;
				/* caseless 'cause fontnames ignore case */
				if(my_strcasestr(new_font->font_name, chPtr) &&
					new_font->style == new_style)
				{
					_XmHTMLaddFontMapping(html, family, first_face, size,
						new_style, new_font);
					break;
				}
			}
		}
		free(first_face);
	}
	return(new_font);
}

/*****
* Name:			_XmHTMLUnloadFonts
* Return Type: 	void
* Description: 	removes a widget from the widget list of a display-bound
*				font cache. When the reference count of this cache reaches
*				zero, the cache is released;
* In: 
*	html:		XmHTMLWidget id;
* Returns:
*	nothing.
*****/
void
_XmHTMLUnloadFonts(XmHTMLWidget html)
{
	Display *dpy = Toolkit_Display((TWidget)html);
	fontCache *cache;
	int i;

	/* get current font cache */
	for(cache = master_cache; cache != NULL && cache->dpy != dpy;
		cache = cache->next);

	if(cache == NULL)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLUnloadFonts"),
			"Font cache corrupted: could not find an entry for this display!");
			return;
	}
	for(i = 0; i < cache->nwidgets && cache->widgets[i] != (TWidget)html; i++);

	if(i == cache->nwidgets)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLUnloadFonts"),
			"Font cache corrupted: could find not an entry for this widget!");
			return;
	}

	_XmHTMLDebug(8,( "\nfonts.c: _XmHTMLUnloadFonts, removing reference for "
		"widget %s.\n", Toolkit_Widget_Name((TWidget)html)));

	/* invalidate current cache? */
	if(cache == curr_cache)
		curr_cache = (fontCache*)NULL;

	/* remove this widget */
	cache->widgets[i] = (TWidget)NULL;
	for(; i < cache->nwidgets - 1; i++)
		cache->widgets[i] = cache->widgets[i+1];

	cache->nwidgets--;

	/* if this was the last widget, free it */
	if(cache->nwidgets == 0)
	{
		fontCache *tmp;

		_XmHTMLDebug(8, ("fonts.c: _XmHTMLUnloadFonts, releasing font "
			"cache.\n"));

		/* special case, first entry is to be freed */
		if(cache == master_cache)
			master_cache = cache->next;
		else
		{
			for(tmp = master_cache; tmp->next != cache; tmp = tmp->next);

			/* connect next entry */
			tmp->next = cache->next;
		}
		/* free the entire list of cached fonts */
		freeFontEntries(dpy, cache->cache);
		free(cache->widgets);
		free(cache);
	}
#ifdef DEBUG
	else
	{
		_XmHTMLDebug(8, ("fonts.c: _XmHTMLUnloadFonts, cache still "
			"referenced by the following widgets:\n"));
		for(i = 0; i < cache->nwidgets; i++)
			_XmHTMLDebug(8, ("\t%s\n", Toolkit_Widget_Name(cache->widgets[i])));
	}
#endif
}

static void
fillCacheInfo(fontCacheEntry *entry, XmHTMLFontCacheInfo *info)
{
	if(entry)
	{
		fillCacheInfo(entry->left, info);

		info->fonts[info->nentries] = entry->name;
		if(entry->is_map)
			info->mapping[info->nentries] = entry->map_to->font_name;
		else
			info->mapping[info->nentries] = NULL;
		info->nentries++;

		fillCacheInfo(entry->right, info);
	}
}

XmHTMLFontCacheInfo*
XmHTMLGetFontCacheInfo(TWidget w)
{
	Display *dpy;
	fontCache *cache;
	static XmHTMLFontCacheInfo *info;

	dpy = Toolkit_Display(w);

	/* sanity check */
	if(dpy == NULL)
	{
		_XmHTMLWarning(__WFUNC__(NULL, "XmHTMLGetFontCacheInfo"),
			"XmHTMLGetFontCacheInfo: can't find display (<NULL>)");
		return(NULL);
	}

	/* pick up correct master cache */
	for(cache = master_cache; cache != NULL && cache->dpy != dpy;
		cache = cache->next);

	/* not found */
	if(cache == NULL)
	{
		_XmHTMLWarning(__WFUNC__(NULL, "XmHTMLGetFontCacheInfo"),
			"XmHTMLGetFontCacheInfo: can't find info for display %s.",
			DisplayString(dpy));
		return(NULL);
	}

	info = (XmHTMLFontCacheInfo*)malloc(sizeof(XmHTMLFontCacheInfo));

	info->nentries  = cache->nentries;		/* no of cached fonts */
	info->nmaps     = cache->nmaps;			/* no of mapped fonts */
	info->nlookups  = cache->nlookups;		/* no of search actions */
	info->nrequests = cache->requests;		/* no of requests made */
	info->hits      = cache->hits;			/* no of hits */
	info->misses    = cache->misses;		/* no of font cache misses */
	info->nwidgets  = cache->nwidgets;		/* no of widgets using this cache */
	info->widgets   = (WidgetList)cache->widgets;
	info->fonts     = (String*)calloc(info->nentries, sizeof(String));
	info->mapping   = (String*)calloc(info->nentries, sizeof(String));

	info->nentries = 0;
	fillCacheInfo(cache->cache, info);

	return(info);
}

void
XmHTMLFreeFontCacheInfo(XmHTMLFontCacheInfo *info)
{
	if(info == NULL)
		return;

	free(info->fonts);
	free(info->mapping);
	free(info);
}
