/*****
* XmHTML.h : XmHTML TWidget public header file
*
* This file Version	$Revision: 1.5 $
*
* Creation date:		Tue Nov 19 23:18:37 GMT+0100 1996
* Last modification: 	$Date: 1998/02/12 03:08:19 $
* By:					$Author: unammx $
* Current State:		$State: Exp $
*
* Author:				newt
* (C)Copyright 1995-1996 Ripley Software Development
* All Rights Reserved
*
* This file is part of the XmHTML TWidget Library.
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
* $Source: /cvs/gnome/gnome-libs/gtk-xmhtml/Attic/XmHTML.h,v $
*****/
/*****
* ChangeLog 
* $Log: XmHTML.h,v $
* Revision 1.5  1998/02/12 03:08:19  unammx
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
* Revision 1.4  1998/01/07 01:45:35  unammx
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
* Revision 1.3  1997/12/29 22:16:21  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.2  1997/12/25 01:34:09  unammx
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
* Revision 1.1  1997/11/28 03:38:55  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.19  1997/10/26 23:49:52  newt
* Moved internal symbol defines to XmHTMLP.h
*
* Revision 1.18  1997/10/23 00:24:44  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.17  1997/08/31 17:32:43  newt
* log edit
*
* Revision 1.16  1997/08/30 00:40:16  newt
* Changed proto's for XmHTMLImageReplace and XmHTMLImageUpdate. They now
* return an XmImageStatus instead of void.
*
* Revision 1.15  1997/08/01 12:54:55  newt
* Progressive image loading changes.
*
* Revision 1.14  1997/05/28 01:41:04  newt
* Added the XmHTMLImageGZFSupported, XmHTMLGIFtoGZF, XmHTMLGifReadOK and
* XmHTMLGifGetDataBlock convienience routines to convert GIF images to an
* alternate format. Added the XmHTMLAllocColor and XmHTMLFreeColor protos for
* easy color allocation. Modified the XmImageCreate routines to use a
* XmImageConfig structure.
*
* Revision 1.13  1997/04/29 14:22:35  newt
* Completely revised due to XmHTMLParserObject integration
*
* Revision 1.12  1997/04/03 05:31:30  newt
* XmHTMLFrameGetChild proto. XmIMAGE_SHARED_DATA option bit added.
*
* Revision 1.11  1997/03/28 07:06:14  newt
* XmNframeCallback, XmHTMLFrameCallbackStruct. XmHTMLParserCallback changes.
*
* Revision 1.10  1997/03/20 08:05:02  newt
* XmHTMLImageFreeImageInfo, XmNrepeatDelay
*
* Revision 1.9  1997/03/11 19:50:27  newt
* Changes in XmImageInfo; grouped and renamed convenience functions
*
* Revision 1.8  1997/03/04 18:45:55  newt
* XmNimagemapBoundingBoxForeground and XmCImagemapDrawBoundingBoxes 
* resources added
*
* Revision 1.7  1997/03/04 00:56:30  newt
* Delayed Image Loading: added the delayed field to the XmImageInfo structure. 
* Removed a number of obsolete defines
*
* Revision 1.6  1997/03/02 23:07:10  newt
* Image/Imagemap related changes. XmHTMLAnchorCallbackStruct changed
*
* Revision 1.5  1997/02/11 02:01:26  newt
* Added the XmNhandleShortTags resource
*
* Revision 1.4  1997/02/04 02:55:57  newt
* added the basefont element
*
* Revision 1.3  1997/01/09 06:55:53  newt
* expanded copyright marker
*
* Revision 1.2  1997/01/09 06:47:58  newt
* New resource: XmNparserCallback and corresponding error structure/defines
*
* Revision 1.1  1996/12/19 02:17:14  newt
* Initial Revision
*
*****/ 
/*****
* Public interfaces are listed at the end of this file
*****/

#ifndef _XmHTML_h_
#define _XmHTML_h_

#define XmHTMLVERSION	1
#define XmHTMLREVISION	1
#define XmHTMLUPDATE_LEVEL 2
#define XmHTMLVersion \
	(XmHTMLVERSION * 1000 + XmHTMLREVISION * 100 + XmHTMLUPDATE_LEVEL)

/* used by Imake to get Shared library version numbering */
#ifndef _LIBRARY

#define XmHTMLVERSION_STRING \
	"XmHTML Beta Version 1.1.2 (C)Ripley Software Development"

/* Required includes */
#ifdef WITH_MOTIF
#    include <X11/Intrinsic.h>
#    include <Xm/Xm.h>
#else
#    include <gtk/gtk.h>
#endif

#ifdef WITH_MOTIF
#    include <XmHTML/HTML.h>
#else
#    include <gtk-xmhtml/HTML.h>
#endif
	
_XFUNCPROTOBEGIN

#ifdef WITH_MOTIF
#    include "XmHTML-motif.h"
#else
typedef struct _GtkXmHTML *XmHTMLWidget;
#endif
	
/********    Public Function Declarations    ********/

/*****
* Convenience/public functions
* There are four categories of them:
* - image related
* - anchor related
* - text related
* - functions that don't fit in any of the above three
*****/

/*****
* Image related convenience functions
*****/
extern XmImageInfo *XmHTMLImageDefaultProc(TWidget w, String file,
	unsigned char *buf, int size);

/* Return image type */
extern unsigned char XmHTMLImageGetType(String file, unsigned char *buf,
	int size);

/* returns True if XmHTMLImageDefaultProc supports JPEG images */
extern Boolean XmHTMLImageJPEGSupported(void);

/* returns True if XmHTMLImageDefaultProc supports PNG images */
extern Boolean XmHTMLImagePNGSupported(void);

/* returns True if XmHTMLImageDefaultProc supports GZF images */
extern Boolean XmHTMLImageGZFSupported(void);

/* Replace image with new_image */
extern XmImageStatus XmHTMLImageReplace(TWidget w, XmImageInfo *image, 
		XmImageInfo *new_image);

/* update image */
extern XmImageStatus XmHTMLImageUpdate(TWidget w, XmImageInfo *image);

/* release all memory occupied by the images */
extern void XmHTMLImageFreeAllImages(TWidget w);

/* add an imagemap to a HTML TWidget. */
extern void XmHTMLImageAddImageMap(TWidget w, String image_map);

/* free an XmImageInfo structure */
extern void	XmHTMLImageFreeImageInfo(TWidget w, XmImageInfo *info);

/* return the total size of a given XmImageInfo structure */
extern int XmHTMLImageGetImageInfoSize(XmImageInfo *info);

/* suspend progressive image loading */
extern void XmHTMLImageProgressiveSuspend(TWidget w);

/* reactivate progressive image loading */
extern void XmHTMLImageProgressiveContinue(TWidget w);

/* terminate progressive image loading */
extern void XmHTMLImageProgressiveKill(TWidget w);

/*****
* Special image functions
*****/
/*****
* Create and return a XmImage for use other than with XmHTML.
* When width and height are non-zero, the image read is scaled to the specified
* dimensions. 
*****/
extern XmImage *XmImageCreate(TWidget w, String file,
	Dimension width, Dimension height, XmImageConfig *config);

/*****
* Create an XmImage from the given XmImageInfo. When the image type is
* IMAGE_UNKNOWN, but the url field represents a local file, this routine
* calls XmImageCreate with the url field as the file argument.
* Only honors the ImageFrameSelect, ImageCreateGC and ImageBackground
* XmImageConfig flag and appropriate fields of that structure.
*****/
extern XmImage *XmImageCreateFromInfo(TWidget w, XmImageInfo *info,
	Dimension width, Dimension height, XmImageConfig *config);

/* destroy a XmImage */
extern void XmImageDestroy(XmImage *image);

/*****
* Tiles "dest" with the given XmImage. Please note that "src" *must* have been
* created with the ImageCreateGC flag, otherwise this function does nothing and
* returns -1. Returns 0 upon success.
* Internally, this routine is more or less a combination of XSetTile,
* XSetTSOrigin and XFillRectangle in one.
*
* (UNTESTED)
*****/
extern int XmImageSetBackgroundImage(XmImage *src, TDrawable dest, int src_x,
	int src_y, int width, int height, int dest_x, int dest_y);

/*****
* XCopyArea for an XmImage which also takes a possible clipmask into account.
* Please note that "image" *must* have been created with the ImageCreateGC
* flag, otherwise this function does nothing and returns -1.
* Returns 0 upon success.
*
* (UNTESTED)
*****/
extern int XmImageDrawImage(XmImage *image, TDrawable dest, int src_x,
	int src_y, int dest_x, int dest_y);

/*****
* Write an image to file. Returns False upon failure.
* Saving an image in the GIF format is *not* possible due to Unisys's
* stupid LZW licensing policy. Exporting an image as PNG, JPEG or GZF is only
* possible if support for the required libraries has been compiled in.
*
* (UNIMPLEMENTED, always returns False)
*****/
extern Boolean XmImageExport(XmImageInfo *info, String file, Dimension width,
	Dimension height, unsigned char type);

/* convert a GIF image to a GZF image */
extern Boolean XmHTMLGIFtoGZF(String infile, unsigned char *buf, int size,
	String outfile);

/*****
* Anchor related convenience functions
* These routines can be used to jump to named anchors.
*****/
/* return the internal id of a named anchor given it's name or -1. */
extern int XmHTMLAnchorGetId(TWidget w, String anchor);

/* scroll to a named anchor, given it's id */
extern void XmHTMLAnchorScrollToId(TWidget w, int anchor_id);

/* scroll to a named anchor, given it's name */
extern void XmHTMLAnchorScrollToName(TWidget w, String anchor);

/*****
* Text related convenience functions
*****/
/* This macro sets the given text into a HTML TWidget */
#define XmHTMLTextSet(TWidget,TEXT) XtVaSetValues((TWidget), \
		XmNvalue, (TEXT), NULL)

/* scroll to the requested line number */
extern void XmHTMLTextScrollToLine(TWidget w, int line);

/* set text into a html TWidget */
extern void XmHTMLTextSetString(TWidget w, String text);

/*****
* same as XmHTMLTextSetString with one fundamental difference: text doesn't
* have to be NULL terminated. The size of the input string is instead
* given by len. If text is NULL or len is 0, the current contents are
* cleared.
*****/
extern void XmHTMLTextSetStringWithLength(TWidget w, String text, size_t len);

/* return a *pointer* to the original text */
extern String XmHTMLTextGetSource(TWidget w);

/* return a copy of the current parser output */
extern String XmHTMLTextGetString(TWidget w);


/* text search functions */

/* search for the string to_find, possibly starting from curr_pos */
extern XmHTMLPosition XmHTMLTextFindString(TWidget w, String to_find,
		XmHTMLDirection dir, XmHTMLPosition curr_pos);

/*****
* Return a formatted copy of the current TWidget contents
* (UNIMPLEMENTED, always returns NULL)
*****/
extern String XmHTMLTextGetFormatted(TWidget w, unsigned char papertype,
	XmHTMLPaperSize *paperdef, unsigned char type, unsigned char PSoptions);

/*****
* Miscelleneous convenience functions
*****/
/* return the library version number */
extern int XmHTMLGetVersion(void);

/* return the URL type of the given href */
extern URLType XmHTMLGetURLType(String href);

/* return the value of the <TITLE></TITLE> element */
extern String XmHTMLGetTitle(TWidget w);

/* return an info structure for the specified location */
extern XmHTMLInfoStructure *XmHTMLXYToInfo(TWidget w, int x, int y);

/****
* Return the contents of the document head. Returns True when a <head></head>
* section is present in the current document, False if not. When mask_bits
* only contains HeadClear, the given attribute structure is wiped clean and
* this function will return False immediatly.
* The only exception concerns the <!DOCTYPE> tag and the HeadDocType mask bit:
* if this bit is set, the value of this tag is returned whether or not a
* head is present.
****/
extern Boolean XmHTMLGetHeadAttributes(TWidget w, XmHTMLHeadAttributes *head,
	unsigned char mask_bits);

/* return the TWidget id of a framechild given its name */
extern TWidget XmHTMLFrameGetChild(TWidget w, String name);

/* force a recomputation of screen layout and trigger a redisplay */
extern void XmHTMLRedisplay(TWidget w);

/* return info about the font cache for display of the given TWidget */
extern XmHTMLFontCacheInfo *XmHTMLGetFontCacheInfo(TWidget w);

/* free the given font cache info */
extern void XmHTMLFreeFontCacheInfo(XmHTMLFontCacheInfo *info);

/*
* Allocate given color (symbolic name of rgb triplet) using the TWidget's
* colormap. Works with the XmNmaxImageColors resource.
*/
extern Pixel XmHTMLAllocColor(TWidget w, String color, Pixel def_pixel);

/* free a color allocated with XmHTMLAllocColor */
extern void XmHTMLFreeColor(TWidget w, Pixel pixel);

_XFUNCPROTOEND

#endif /* _LIBRARY */

/* Don't add anything after this endif! */
#endif /* _XmHTML_h_ */
