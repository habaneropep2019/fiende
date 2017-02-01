/*****
* readXPM.c : XmHTML XPM image loading routines
*
* This file Version	$Revision: 1.6 $
*
* Creation date:		Wed Feb 19 03:19:23 GMT+0100 1997
* Last modification: 	$Date: 1999/07/29 01:26:29 $
* By:					$Author: sopwith $
* Current State:		$State: Exp $
*
* Author:				newt
*
* Copyright (C) 1994-1997 by Ripley Software Development 
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
* $Log: readXPM.c,v $
* Revision 1.6  1999/07/29 01:26:29  sopwith
*
*
* Fix all warnings.
*
* Revision 1.5  1998/02/12 03:09:48  unammx
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
* Revision 1.4  1998/01/14 04:12:11  unammx
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
* Revision 1.3  1998/01/10 03:26:24  unammx
* fix
*
* Revision 1.2  1998/01/07 01:45:42  unammx
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
* Revision 1.1  1997/11/28 03:38:59  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.9  1997/10/23 00:25:26  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.8  1997/08/30 01:34:07  newt
* Extended the check for transparent pixel names to include mask and background.
*
* Revision 1.7  1997/08/01 13:12:43  newt
* Much more readable: eliminated duplicate code.
*
* Revision 1.6  1997/05/28 01:56:09  newt
* Image depth support.
*
* Revision 1.5  1997/04/29 14:31:15  newt
* Header files modifications.
*
* Revision 1.4  1997/03/20 08:16:03  newt
* Transparency color bugfix: pixel is now index instead of background pixel
*
* Revision 1.3  1997/03/11 19:59:03  newt
* ImageBuffer changes
*
* Revision 1.2  1997/03/04 18:49:55  newt
* Removed dependency on work_area field of XmHTMLWidget
*
* Revision 1.1  1997/03/02 23:03:00  newt
* Initial Revision
*
*****/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* prevent Byte re-declaration */
#if defined(HAVE_LIBPNG) || defined(HAVE_LIBZ)
#include <zlib.h>
#endif

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"
#include "plc.h"

/* xpm checks whether Pixel has already been defined. IntrinsicP doesn't */
#include <X11/xpm.h>

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/

/*** Private Function Prototype Declarations ****/
static XmHTMLRawImageData *doXpm(TWidget html, ImageBuffer *ib,
	XpmImage *xpm_image);

/*** Private Variable Declarations ***/

/*****
* Name: 		doXpm
* Return Type: 	XmHTMLRawImageData*
* Description: 	converts the given xpm data to our own format
* In: 
*	html:		widget id;
*	ib:			current image buffer;
*	xpm_image:	xpm image data;
* Returns:
*
*****/
static XmHTMLRawImageData*
doXpm(TWidget html, ImageBuffer *ib, XpmImage *xpm_image)
{
	int i, is_gray = 0;
	TColor tmpcolr;
	String col_name;
	TColormap cmap;
	static XmHTMLRawImageData *img_data;
	register Byte *bptr;
	register unsigned int *ptr;

#ifdef WITH_MOTIF
	/*
	* get colormap. We need one to obtain the RGB components of the
	* selected colors, pixmaps define their colors using a symbolic name
	* instead of defining the wanted RGB components.
	*/
	if(XmIsHTML(html))
		cmap = ((XmHTMLWidget)html)->core.colormap;
	else	/* every widget is derived from core */
		XtVaGetValues(html, XmNcolormap, &cmap, NULL);
#else
	cmap = gtk_widget_get_colormap (html);
#endif
	
	/* allocate raw image */
	AllocRawImageWithCmap(img_data, (int)xpm_image->width,
		(int)xpm_image->height, (int)xpm_image->ncolors);

	/* little trick to compute image depth */
	if(ib != NULL)
	{
		ib->depth = 2;
		while((ib->depth << 2) < img_data->cmapsize && ib->depth < 12)
			ib->depth++;
	}

	/* fill colormap for this image */
	for(i = 0; i < img_data->cmapsize; i++)
	{
		/* pick up the name of the current color */
		col_name = xpm_image->colorTable[i].c_color;

		/* transparancy, these can *all* name a transparent color. */
		if(!(strcasecmp(col_name, "none")) ||
			!(strcasecmp(col_name, "mask")) ||
			!(strcasecmp(col_name, "background")))
		{
			Pixel bg_pixel;

			/*
			* Get current background index: use the given background pixel
			* index if we have one. Else get the current background color of
			* the given widget.
			*/
			if(_xmimage_cfg && (_xmimage_cfg->flags & ImageBackground))
				bg_pixel = _xmimage_cfg->bg_color;
			else
			{
#ifdef WITH_MOTIF
				if(XmIsHTML(html))
					bg_pixel = ((XmHTMLWidget)html)->html.body_bg;
				else
					XtVaGetValues(html, XtNbackground, &bg_pixel, NULL);
#else
				/* FIXME: I don't know if getting the color from macro is
				 * the same as getting it from the Xt resource.
				 */
				bg_pixel = Toolkit_StyleColor_Background(html);
#endif
			}

			/* get RGB components for this color. */
			tmpcolr.pixel = bg_pixel;
#ifdef WITH_MOTIF
			XQueryColor(Toolkit_Display(html), cmap, &tmpcolr);
#else
			my_x_query_colors(cmap, &tmpcolr, 1);
#endif
			/* store background pixel index */
			img_data->bg = i;
		}
		else /* get RGB components for this color */ {
#ifdef WITH_MOTIF
			(void)XParseColor(Toolkit_Display(html), cmap, col_name, &tmpcolr);
#else
			gdk_color_parse (col_name, &tmpcolr);
#endif
		}
		/* colorcomponents are in the range 0-255 */
		img_data->cmap[i].red   = tmpcolr.red   >> 8;
		img_data->cmap[i].green = tmpcolr.green >> 8;
		img_data->cmap[i].blue  = tmpcolr.blue  >> 8;
		is_gray &= (tmpcolr.red == tmpcolr.green) &&
					(tmpcolr.green == tmpcolr.blue);
	}
	/* no need to fill in remainder of colormap, gets done by AllocRawImage */
	img_data->color_class = (is_gray != 0 ? XmIMAGE_COLORSPACE_INDEXED :
		XmIMAGE_COLORSPACE_GRAYSCALE);

	/*****
	* convert xpm image data to our own internal format: array of indices
	* in the colormap for this image. First pixel at upper-left corner.
	* The XpmImage data is actually already in this format but as the
	* XpmImage data is unsigned int we need to check if the indices don't
	* exceed 255 (or we would get an out-of-bounds indexing leading to a
	* segmentation fault eventually).
	*****/
	ptr  = xpm_image->data;
	bptr = img_data->data;
	for(i = 0; i < (img_data->width * img_data->height); i++)
	{
		int pix;
		pix = (int)*ptr;
		if (pix > (MAX_IMAGE_COLORS - 1))
			pix = 0;
		*bptr++ = (Byte)pix;
		ptr++;
	}
	XpmFreeXpmImage(xpm_image);
	return(img_data);
}

/*****
* Name: 		_XmHTMLReadXPM
* Return Type: 	XmHTMLRawImageData*
* Description: 	reads an xpm image of any type from xpm data read from a file.
* In: 
*	html:		widget id;
*	ib:			image data;
* Returns:
*	allocated image upon success. NULL on failure.
*****/
XmHTMLRawImageData*
_XmHTMLReadXPM(TWidget html, ImageBuffer *ib)
{
	XpmImage xpm_image;
	XpmInfo foo;
	int i;

	(void)memset(&xpm_image, 0, sizeof(xpm_image));
	(void)memset(&foo, 0, sizeof(foo));

	if((i = XpmCreateXpmImageFromBuffer((String)ib->buffer, &xpm_image, 
		&foo)) != XpmSuccess)
	{
		/* spit out appropriate error message */
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLReadXPM"), "libXpm "
			"failed on image %s:\n    %s", ib->file, XpmGetErrorString(i));

		/* release everything */
		XpmFreeXpmInfo(&foo);
		XpmFreeXpmImage(&xpm_image);
		return(NULL);
	}
	/* we don't use the returned info so free it */
	XpmFreeXpmInfo(&foo);

	/* convert xpm data to raw image data */
	return(doXpm(html, ib, &xpm_image));
}

/*****
* Name: 		_XmHTMLCreateXpmFromData
* Return Type: 	XmHTMLRawImageData*
* Description: 	reads an xpm image of any type from raw xpm data
* In: 
*	html:		widget id;
*	data:		xpm data
* Returns:
*	allocated image upon success. NULL on failure.
*****/
XmHTMLRawImageData*
_XmHTMLCreateXpmFromData(TWidget html, char **data, String src)
{
	XpmImage xpm_image;
	XpmInfo foo;
	int i;

	(void)memset(&xpm_image, 0, sizeof(xpm_image));
	(void)memset(&foo, 0, sizeof(foo));

	if((i = XpmCreateXpmImageFromData(data, &xpm_image, &foo)) != XpmSuccess)
	{
		/* spit out appropriate error message */
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLCreateXpmFromData"),
			"libXpm failed on image %s:\n    %s", src, XpmGetErrorString(i));

		/* release everything */
		XpmFreeXpmInfo(&foo);
		XpmFreeXpmImage(&xpm_image);
		return(NULL);
	}
	/* we don't use the returned info so free it */
	XpmFreeXpmInfo(&foo);

	/* convert xpm data to raw image data */
	return(doXpm(html, NULL, &xpm_image));
}

/*****
* Progressive Pixmap loading routines
*****/

void
_PLC_XPM_Init(PLC *plc)
{
	plc->plc_status = PLC_ABORT;
}

void
_PLC_XPM_ScanlineProc(PLC *plc)
{
	plc->plc_status = PLC_ABORT;
}

void
_PLC_XPM_Destructor(PLC *plc)
{
	plc->plc_status = PLC_ABORT;
}

