/*****
* XmHTMLfuncs.h : widely used functions and overall configuration settings.
*
* This file Version	$Revision: 1.6.10.1 $
*
* Creation date:		Tue Dec  3 15:00:14 GMT+0100 1996
* Last modification: 	$Date: 2001/10/20 06:52:12 $
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
* $Source: /cvs/gnome/gnome-libs/gtk-xmhtml/Attic/XmHTMLfuncs.h,v $
*****/
/*****
* ChangeLog 
* $Log: XmHTMLfuncs.h,v $
* Revision 1.6.10.1  2001/10/20 06:52:12  kmaraas
* 2001-10-20  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* *.*: Apply all the Red Hat patches.
*
* Revision 1.6  1998/12/01 21:10:06  harinath
* * XmHTMLfuncs.h: Avoid ordering problems by including stdlib.h and
* string.h.  #undef before #define to avoid warnings.
*
* Revision 1.5  1998/02/12 03:08:28  unammx
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
* Revision 1.4  1997/12/29 22:16:23  unammx
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
* Revision 1.2  1997/12/19 00:03:49  unammx
* gtk/xmhtml updates
*
* Revision 1.1  1997/11/28 03:38:56  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.16  1997/10/23 00:24:48  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.15  1997/08/30 00:43:02  newt
* HashTable stuff. Changed proto's for almost every routine in here.
*
* Revision 1.14  1997/08/01 12:56:02  newt
* Progressive image loading changes. Changes to debug memory alloc protos.
*
* Revision 1.13  1997/05/28 01:43:34  newt
* Added protos and defines for debug memory allocation functions.
*
* Revision 1.12  1997/04/29 14:23:32  newt
* Moved all XmHTML private functions to XmHTMLP.h
*
* Revision 1.11  1997/04/03 05:32:52  newt
* ImageInfoShared macro. _XmHTMLLoadBodyImage proto
*
* Revision 1.10  1997/03/28 07:06:42  newt
* Frame interface prototypes from frames.c
*
* Revision 1.9  1997/03/20 08:07:25  newt
* added external html_tokens definition, _XmHTMLReplaceOrUpdateImage
*
* Revision 1.8  1997/03/11 19:52:05  newt
* ImageBuffer; XmHTMLImage and XmImageInfo macros; new protos for animated Gifs
*
* Revision 1.7  1997/03/04 18:47:01  newt
* _XmHTMLDrawImagemapSelection proto added
*
* Revision 1.6  1997/03/04 00:57:29  newt
* Delayed Image Loading: _XmHTMLReplaceImage and _XmHTMLUpdateImage
*
* Revision 1.5  1997/03/02 23:14:13  newt
* malloc defines; function proto's for all private image/imagemap routines
*
* Revision 1.4  1997/02/11 02:02:57  newt
* Changes for NEED_STRCASECMP
*
* Revision 1.3  1997/01/09 06:55:59  newt
* expanded copyright marker
*
* Revision 1.2  1997/01/09 06:48:43  newt
* updated function definitions
*
* Revision 1.1  1996/12/19 02:17:18  newt
* Initial Revision
*
*****/ 

#ifndef _XmHTMLfuncs_h_
#define _XmHTMLfuncs_h_

#include <errno.h>

/* Since we override some of the symbols in the following headers,
   include them first, to prevent include ordering problems.  */
#include <stdlib.h>
#include <string.h>

#ifdef WITH_MOTIF
#include <X11/IntrinsicP.h>		/* for Widget definition & fast macros */
#endif

#ifndef BYTE_ALREADY_TYPEDEFED
#define BYTE_ALREADY_TYPEDEFED
typedef unsigned char Byte;
#endif /* BYTE_ALREADY_TYPEDEFED */

/*****
* The top section of this file contains a number of default values that
* can only be set at compile-time.
* These values modify the default behaviour of the XmHTML widget, so be
* carefull when modifying these values.
*****/

/***** 
* Time window in which button press & release trigger an anchor activation.
* (specified in milliseconds).
*****/
#define XmHTML_BUTTON_RELEASE_TIME			500		

/*****
* Default horizontal & vertical marginwidth.
*****/
#define XmHTML_DEFAULT_MARGIN				20

/*****
* Number of pixels to scroll when the scrollbar isn't being dragged but
* moved by either keyboard actions or arrow pressing. Be warned that
* using large values will cause a jumpy scrolling as XmHTML will have to
* render increasing amounts of data.
*****/
#define XmHTML_HORIZONTAL_SCROLL_INCREMENT	12		/* average char width */
#define XmHTML_VERTICAL_SCROLL_INCREMENT	18		/* average line height */

/*****
* Absolute maximum no of colors XmHTML may use. It is the maximum value for
* the XmNmaxImageColors resource.
* Increasing this number isn't a wise thing to do since all image routines
* are optimized for using paletted images. If you want XmHTML to handle more
* than 256 colors, you will have to modify the code.
*****/
#define MAX_IMAGE_COLORS					256

/*****
* Default font sizes
* Scalable size array: default,sub/superscript,h1,h2,h3,h4,h5,h6
* Fixed size array   : default,sub/superscript
*****/
#define XmHTML_DEFAULT_FONT_SCALABLE_SIZES	"14,8,24,16,16,14,14,8"
#define XmHTML_DEFAULT_FONT_FIXED_SIZES		"14,8"

/*****
* Default Table boder width, cell & row spacing.
*****/
#define XmHTML_DEFAULT_TABLE_BORDERWIDTH		1
#define XmHTML_DEFAULT_CELLSPACING			2
#define XmHTML_DEFAULT_ROWSPACING			2

/*****
* Maximum number of iterations the text-justification routines may reach.
* Decreasing the default value of 1500 will lead to an increasing amount
* of warnings.
*****/
#define MAX_JUSTIFY_ITERATIONS				1500

/*****
* Maximum number of iterations the table layout computation routines may
* reach. This only occurs when the minimum suggested table width is smaller
* than the available width and the maximum suggested table width is larger
* than the available width. In this case, the layout routines have to compute
* an optimum balance between the different colum widths within the available
* width. The algorithm used should be convergent, but could be divergent for
* nested tables or tables prefixed with an extreme indentation (nested lists
* and such). Hence the safeguard.
*****/
#define MAX_TABLE_ITERATIONS				128

/*****
* Default gamma correction value for your display. This is only used for
* images that support gamma correction (JPEG and PNG).
* 2.2 is a good assumption for almost every X display.
* For a Silicon Graphics displays, change this to 1.8
* For Macintosh displays (MkLinux), change this to 1.4 (so I've been told)
* If you change this value, it *must* be a floating point value.
*****/
#define XmHTML_DEFAULT_GAMMA				2.2

/*****
* Maximum size of the PLC get_data() buffer. This is the maximum amount
* of data that will be requested to a function installed on the
* XmNprogressiveReadProc. Although this define can have any value, using
* a very small value will make progressive loading very slow, while using
* a large value will make the response of XmHTML slow while any PLC's are
* active.
* The first call to the get_data() routine will request PLC_MAX_BUFFER_SIZE
* bytes, while the size requested by any following calls will depend on the
* type of image being loaded and the amount of data left in the current input
* buffer.
*****/
#define PLC_MAX_BUFFER_SIZE					2048

/*****
* The default timeout value for the Progressive Loader Context. This
* timeout is the default value for the XmNprogressiveInitialDelay and
* specifies the polling interval between subsequent PLC calls.
* 
* Specified in milliseconds (1 second = 1000 milliseconds)
* XmHTML dynamically adjusts the timeout value as necessary and recomputes
* it after each PLC call.
* PLC_MIN_DELAY is the minimum value XmHTML can reduce the timeout to while
* PLC_MAX_DELAY is the maximum value XmHTML can increase the timeout to.
*****/
#define PLC_DEFAULT_DELAY					250
#define PLC_MIN_DELAY						5
#define PLC_MAX_DELAY						1000

/***************** End of User configurable section *****************/ 

/*****
* magic number for the XmHTMLImage structure. XmHTML uses this field to verify
* the return value from a user-installed primary image cache.
*****/
#define XmHTML_IMAGE_MAGIC		0xce

/* lint kludge */
#ifdef lint
#undef True
#undef False
#define True ((Boolean)1)
#define False ((Boolean)0)
#endif /* lint */

/*****
* When X was written, noboby ever heard of C++, let alone what words
* would be reserved for this language, and as a result of this, a few
* structures in X contain words that could cause a problem when compiling
* XmHTML with a C++ compiler. 
*****/
#if defined(__cplusplus) || defined(c_plusplus)
#define MEMBER_CLASS	c_class
#define MEMBER_NEW		c_new
#define MEMBER_DELETE	c_delete
#else
#define MEMBER_CLASS	class
#define MEMBER_NEW		new
#define MEMBER_DELETE	delete
#endif

/*****
* None can be undefined if we aren't compiled for Xt/Motif.
*****/
#ifndef None
#define None		0
#endif
#define IdleKeep					False
#define IdleRemove					True
#define NullTimeout					None

/****
* debug.c
* Must include this before anything else to prevent inconsistencies.
****/
#include "debug.h"

/*****
* Using #if !defined(DMALLOC) && !defined(DEBUG) seems to trigger a bug
* on SparcWorks CPP, so we use the old #ifndef combi's to work around it.
* Fix 10/27/97-01, shl.
*****/
#ifndef DMALLOC

/* These may be #defined.  */
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup

#ifndef DEBUG

/* Normal builds use Xt memory functions */

#    ifdef WITH_MOTIF
#        define malloc(SZ)		XtMalloc((SZ))
#        define calloc(N,SZ)		XtCalloc((N),(SZ))
#        define realloc(PTR,SZ)		XtRealloc((char*)(PTR),(SZ))
#        define free(PTR)		XtFree((char*)(PTR))
#        define strdup(STR)		XtNewString((STR))
#    else
#        define malloc(SZ)              g_malloc(SZ)
#        define calloc(N,SZ)            g_malloc0((SZ)*(N))
#        define realloc(PTR,SZ)         g_realloc(PTR,SZ)
#        define free(PTR)               g_free(PTR)
#        define strdup(STR)             g_strdup(STR)
#    endif
#else /* DEBUG defined */

/* debug builds use asserted functions unless DMALLOC is defined */
extern char *__rsd_malloc(size_t size, char *file, int line);
extern char *__rsd_calloc(size_t nmemb, size_t size, char *file, int line);
extern char *__rsd_realloc(void *ptr, size_t size, char *file, int line);
extern char *__rsd_strdup(const char *s1, char *file, int line);
extern void  __rsd_free(void *ptr, char *file, int line);

/* every source file has a static variable called src_file */
#define malloc(SZ)			__rsd_malloc((SZ), __FILE__, __LINE__)
#define calloc(N,SZ)		__rsd_calloc((N),(SZ), __FILE__, __LINE__)
#define realloc(PTR,SZ)		__rsd_realloc((PTR),(SZ), __FILE__, __LINE__)
#define free(PTR)			__rsd_free((PTR), __FILE__, __LINE__)
#define strdup(STR)			__rsd_strdup((STR), __FILE__, __LINE__)

#endif /* DEBUG */
#else /* DMALLOC */

/* let dmalloc.h define it all */
#include <dmalloc.h>

#endif /* DMALLOC */

/* Don't add anything after this endif! */
#endif /* _XmHTMLfuncs_h_ */
