/*****
* XmHTMLP.h : XmHTML Widget private header file
*
* This file Version	$Revision: 1.11.6.1 $
*
* Creation date:		Tue Nov 19 23:18:41 GMT+0100 1996
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
* $Source: /cvs/gnome/gnome-libs/gtk-xmhtml/Attic/XmHTMLP.h,v $
*****/
/*****
* ChangeLog 
* $Log: XmHTMLP.h,v $
* Revision 1.11.6.1  2001/10/20 06:52:12  kmaraas
* 2001-10-20  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* *.*: Apply all the Red Hat patches.
*
* Revision 1.11  1999/06/02 01:00:37  unammx
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
* Revision 1.10  1998/02/12 03:08:23  unammx
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
* Revision 1.9  1998/02/11 00:02:29  unammx
* Tue Feb 10 18:02:19 1998  Miguel de Icaza  <miguel@nuclecu.unam.mx>
*
* 	* XmHTMLP.h: Debugging fields on the structure are always
* 	included, no matter if DEBUG was defined or not.  Otherwise
* 	applications linked with XmHTML would required to use the same
* 	defines that were used originally.
*
* Revision 1.8  1998/01/07 01:45:36  unammx
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
* Revision 1.7  1997/12/30 03:32:51  unammx
* More work on getting the frames working, still some bits are missing - Miguel
*
* Revision 1.6  1997/12/29 22:16:22  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.5  1997/12/25 01:34:10  unammx
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
* Revision 1.4  1997/12/24 17:53:54  unammx
* Fun stuff:
*
* 	The widget now handles mouse motion, mouse clicks, anchors can
* 	be clicked.
*
* 	The widget emits signals for all of the interesting events
* 	(the same events that were used by the Motif port, we just use
* 	signals instead of XtCallbacks).
*
* Boring stuff:
*
* 	The widget now handles focusin/focusout/enternotif/leavenotify
*
* 	More code sharing between the Motif frontend an the Gtk
* 	frontned;   More portability macros;
*
* 	Cleaned up some more the privte widget header files.
*
* Revision 1.3  1997/12/18 00:39:21  unammx
* It compiles and links -miguel
*
* Revision 1.2  1997/12/17 04:40:28  unammx
* Your daily XmHTML code is here.  It almost links.  Only the
* images.c file is left to port.  Once this is ported we are all
* set to start debugging this baby.
*
* btw, Dickscrape is a Motif based web browser that is entirely
* based on this widget, I just tested it today, very impressive.
*
* Miguel.
*
* Revision 1.1  1997/11/28 03:38:56  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.18  1997/10/26 23:50:04  newt
* Added internal symbol defines (_LIBRARY stuff)
*
* Revision 1.17  1997/10/23 00:24:47  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.16  1997/08/31 17:32:58  newt
* log edit
*
* Revision 1.15  1997/08/30 00:42:16  newt
* Reorganized a number of things and moved all private XmHTML functions to
* the XmHTMLI.h include file.
*
* Revision 1.14  1997/08/01 12:55:01  newt
* Progressive image loading changes.
*
* Revision 1.13  1997/05/28 01:43:05  newt
* Made all image options a long instead of a Byte. Additional protos for the
* XmNdecodeGIFProc resource.
*
* Revision 1.12  1997/04/29 14:23:03  newt
* Moved all XmHTML private functions in.
*
* Revision 1.11  1997/04/03 05:31:52  newt
* Added an additional base field to the XmHTMLWord structure.
*
* Revision 1.10  1997/03/28 07:06:26  newt
* Frame interface support.
*
* Revision 1.9  1997/03/20 08:06:11  newt
* XmNrepeatDelay, moved HTML enumeration table from XmHTML.h in here
*
* Revision 1.8  1997/03/11 19:50:50  newt
* Changes in XmHTMLImage, XmHTMLWord and XmHTMLObjectTable
*
* Revision 1.7  1997/03/04 18:46:42  newt
* XmHTMLImage changed for animation support; imagemap_fg and imagemap_draw added
*
* Revision 1.6  1997/03/04 00:56:58  newt
* Removed some obsolete fields
*
* Revision 1.5  1997/03/02 23:09:30  newt
* XmHTMLImage, XmHTMLImageMap, XmHTMLAnchor structs modified.
*
* Revision 1.4  1997/02/11 02:02:11  newt
* Removed obsolete fields from XmHTMLWord, XmHTMLObjectTable. 
* Added stuff for new anchor treatment.
*
* Revision 1.3  1997/01/09 06:55:56  newt
* expanded copyright marker
*
* Revision 1.2  1997/01/09 06:48:29  newt
* changes in XmHTMLWord. Added all image structures and enums.
*
* Revision 1.1  1996/12/19 02:17:16  newt
* Initial Revision
*
*****/ 

#ifndef _XmHTMLP_h_
#define _XmHTMLP_h_

#include <X11/Xatom.h>		/* property defines */
/* Required includes */

#ifdef WITH_MOTIF
#     include <XmHTML/toolkit.h>
#     include <XmHTML/XmHTML.h>
#else
#     include <gtk-xmhtml/toolkit.h>
#     include <gtk-xmhtml/XmHTML.h>
#endif



/*****
* NEVER define VERSION *or* _LIBRARY yourself. These are defines required
* for compiling the library. When defined, they pull in a number of other
* header files which are normally *not* installed.
*****/
#ifdef GNOME_LIB
#   ifndef VERSION
#       define VERSION
#   endif
#endif

#ifdef VERSION
# ifndef _LIBRARY
#  define _LIBRARY
# endif
#endif/* VERSION */

/*****
* prevent multiple decls when building the lib.
* zconf.h (which gets included by png.h) also typedefs Byte to unsigned char.
* The compiler warning is annoying, so just don't typedef it again.
* XCC is an opaque object defined in XCCP.h
*****/
#ifdef _LIBRARY
# ifdef WITH_MOTIF
#  include "XCC.h"		/* XColorContext definitions and protos */
# else
typedef GdkColorContext *XCC;
# endif
# ifdef _ZCONF_H
#  ifndef BYTE_ALREADY_TYPEDEFED
#   define BYTE_ALREADY_TYPEDEFED
#  endif /* BYTE_ALREADY_TYPEDEFED */
# endif /* _ZCONF_H */
#else
  typedef struct _XColorContext *XCC;
#endif /* _LIBRARY */

#ifndef BYTE_ALREADY_TYPEDEFED
#define BYTE_ALREADY_TYPEDEFED
typedef unsigned char Byte;
#endif /* BYTE_ALREADY_TYPEDEFED */

_XFUNCPROTOBEGIN

/*****
* A whole slew of states for all kinds of internal objects.
*****/

/*****
* Line styles
*****/
#define LINE_SOLID		(1<<1)	/* paint a solid line	*/
#define LINE_DASHED		(1<<2)	/* paint a dashed line	*/
#define LINE_SINGLE		(1<<3)	/* paint a single line	*/
#define LINE_DOUBLE		(1<<4)	/* paint a double line	*/
#define LINE_STRIKE		(1<<5)	/* render as strikeout	*/
#define LINE_UNDER		(1<<6)	/* render as underline	*/
#define NO_LINE			0		/* no lines at all		*/

/*****
* Spacing and anchor text data bits
*****/
#define TEXT_SPACE_NONE			(1<<0)	/* no spacing at all			*/
#define TEXT_SPACE_LEAD			(1<<1)	/* add a leading space			*/
#define TEXT_SPACE_TRAIL 		(1<<2)	/* add a trailing space			*/
#define TEXT_ANCHOR				(1<<3)	/* a regular anchor				*/
#define TEXT_ANCHOR_INTERN		(1<<4)	/* internal anchor flag			*/
#define TEXT_IMAGE				(1<<5)	/* indicates an image member	*/
#define TEXT_FORM				(1<<6)	/* indicates a form member		*/
#define TEXT_BREAK				(1<<7)	/* indicates a linebreak		*/
#define TEXT_SPACE_LEAD_ZEROWIDTH	(1<<8)	/* no spacing, but can break before the word	*/
#define TEXT_SPACE_TRAIL_ZEROWIDTH	(1<<9)	/* no spacing, but can break after the word	*/

/***** 
* HTML list marker enumeration type 
*****/
typedef enum{
	XmMARKER_NONE = 0, 
	XmMARKER_ARABIC = 10, 
	XmMARKER_ALPHA_LOWER, 
	XmMARKER_ALPHA_UPPER, 
	XmMARKER_ROMAN_LOWER,
	XmMARKER_ROMAN_UPPER,
	XmMARKER_DISC = 15,
	XmMARKER_SQUARE,
	XmMARKER_CIRCLE
}Marker;

/***** 
* Horizontal/Vertical alignment data 
*****/
typedef enum{
	XmHALIGN_NONE = 0,	/* horizontal alignment */
	XmHALIGN_LEFT,
	XmHALIGN_CENTER,
	XmHALIGN_RIGHT,
	XmHALIGN_JUSTIFY,	/* extension for fully justified text */
	XmVALIGN_NONE = 8,	/* vertical alignment */
	XmVALIGN_TOP,
	XmVALIGN_MIDDLE,
	XmVALIGN_BOTTOM,
	XmVALIGN_BASELINE
}Alignment;

/***** 
* Possible types of HTML objects. 
* All text types are only used when computing the screen layout
*****/
typedef enum{
	OBJ_NONE = 0,
	OBJ_TEXT,			/* text element							*/
	OBJ_PRE_TEXT,		/* preformatted text					*/
	OBJ_BULLET,			/* all types of markers for lists		*/
	OBJ_HRULE,			/* horizontal rule						*/
	OBJ_TABLE,			/* table elements						*/
	OBJ_TABLE_FRAME,	/* table caption, row, cell elements	*/
	OBJ_IMG,			/* image elements						*/
	OBJ_FORM,			/* form elements						*/
	OBJ_APPLET,			/* applet elements						*/
	OBJ_BLOCK			/* other block level elements			*/
}ObjectType;

/*****
* linefeed types
*****/
#define CLEAR_NONE			-1		/* stay on the same line				*/
#define CLEAR_SOFT			0		/* return + move single line downard	*/
#define CLEAR_HARD			1		/* return + move two lines downward		*/
#define CLEAR_ALL			2		/* return + move baseline fully down	*/

/*****
* Server/client side and map type values 
*****/
typedef enum{
	XmMAP_NONE = 1,
	XmMAP_SERVER,
	XmMAP_CLIENT
}Imagemap;

/*****
* Image option bits.
* Each of these bits represents certain state information about an image.
*****/
#define IMG_ISBACKGROUND		(1L<<1)	/* is a background image			*/
#define IMG_ISINTERNAL			(1L<<2)	/* is an internal image				*/
#define IMG_ISCOPY				(1L<<3)	/* is a referential copy			*/
#define IMG_ISANIM				(1L<<4)	/* is an animation					*/
#define IMG_FRAMEREFRESH		(1L<<5)	/* set when running an animation	*/
#define IMG_HASDIMENSIONS		(1L<<6)	/* dimensions are given in <IMG>	*/
#define IMG_HASSTATE			(1L<<7)	/* current state pixmap present		*/
#define IMG_INFOFREED			(1L<<8)	/* imageinfo has been freed			*/
#define IMG_DELAYED_CREATION	(1L<<9)	/* create when needed				*/
#define IMG_ORPHANED			(1L<<10)/* indicates orphaned image			*/
#define IMG_PROGRESSIVE			(1L<<11)/* indicates image is being loaded	*/

/*****
* Possible colorclass an image can have.
*****/
#define COLOR_CLASS_GRAYSCALE	0
#define COLOR_CLASS_INDEXED		1
#define COLOR_CLASS_RGB			2

/*****
* What type of scrolling a frame should employ.
*****/
typedef enum{
	FRAME_SCROLL_NONE = 1,
	FRAME_SCROLL_AUTO,
	FRAME_SCROLL_YES
}FrameScrolling;

/*****
* Possible types of frame sizes
*****/
typedef enum{
	FRAME_SIZE_FIXED = 1,			/* size specified in pixels	*/
	FRAME_SIZE_RELATIVE,			/* size is relative			*/
	FRAME_SIZE_OPTIONAL				/* size is optional			*/
}FrameSize;

/*****
* The three possible anchor selection states
*****/
#define ANCHOR_UNSELECTED	(Byte)0		/* default anchor state			*/
#define ANCHOR_INSELECT		(Byte)1		/* anchor is gaining selection	*/
#define ANCHOR_SELECTED		(Byte)2		/* anchor is selected			*/

/*****
* XmHTML font style bits
*****/
#define FONT_BOLD		(1<<1)
#define FONT_MEDIUM		(1<<2)
#define FONT_ITALIC		(1<<3)
#define FONT_REGULAR	(1<<4)
#define FONT_FIXED		(1<<5)
#define FONT_SCALABLE	(1<<6)

/*****
* A XmHTML font. XmHTML uses it's own font definition for performance
* reasons (the layout routines use a *lot* of font properties).
*****/
typedef struct{
 	Byte style;					/* this font's style						*/
 	String font_name;			/* full XLFD								*/
 	String font_family;			/* fontFamily (foundry-family-sw-spacing)	*/
 	TFontStruct *xfont;			/* ptr to font definition					*/
 	int lbearing;					/* lbearing of largest character			*/
 	int rbearing;					/* rbearing of largest character			*/
 	int width;					/* width of largest character				*/
 	int height;					/* height of largest character				*/
 	int lineheight;				/* suggested lineheight						*/
 	Cardinal isp;				/* normal interword spacing					*/
 	Cardinal eol_sp;			/* additional end-of-line spacing			*/
 	int sup_xoffset;			/* additional superscript x-offset			*/
 	int sup_yoffset;			/* additional superscript y-offset			*/
 	int sub_xoffset;			/* additional subscript x-offset			*/
 	int sub_yoffset;			/* additional subscript y-offset			*/
 	int ul_offset;				/* additional underline offset				*/
 	Cardinal ul_thickness;		/* underline thickness						*/
 	int st_offset;				/* additional strikeout offset				*/
 	Cardinal st_thickness;		/* strikeout thickness						*/
}XmHTMLfont;

/**********
****** Definition of all possible Objects
**********/

/*****
* HTML4.0 Events
*****/
typedef struct _HTEvent{
	int			type;			/* HTML4.0 event type	*/
	XtPointer	data;			/* event user data		*/
}HTEvent;

/*****
* All possible events that HTML4.0 defines
* All fields are ptrs into XmHTML HTEvent array.
*****/
typedef struct _AllEvents{
	/* Document/Frame specific events */
	HTEvent		*onLoad;
	HTEvent		*onUnload;

	/* HTML Form specific events */
	HTEvent		*onSubmit;
	HTEvent		*onReset;
	HTEvent		*onFocus;
	HTEvent		*onBlur;
	HTEvent		*onSelect;
	HTEvent		*onChange;

	/* object events */
	HTEvent		*onClick;
	HTEvent		*onDblClick;
	HTEvent		*onMouseDown;
	HTEvent		*onMouseUp;
	HTEvent		*onMouseOver;
	HTEvent		*onMouseMove;
	HTEvent		*onMouseOut;
	HTEvent		*onKeyPress;
	HTEvent		*onKeyDown;
	HTEvent		*onKeyUp;
}AllEvents;

/*****
* Definition of an anchor 
* URLType is an enumeration type defined in HTML.h
*****/
typedef struct _XmHTMLAnchor{
	URLType				url_type;		/* url type of anchor			*/
	String				name;			/* name if it's a named anchor	*/
	String				href;			/* referenced URL				*/
	String				target;			/* target spec					*/
	String				rel;			/* possible rel					*/
	String				rev;			/* possible rev					*/
	String				title;			/* possible title				*/
	AllEvents			*events;		/* events to be served			*/
	Cardinal 			line;			/* location of this anchor		*/
	Boolean				visited;		/* true when anchor is visited	*/
	struct _XmHTMLAnchor *next;			/* ptr to next anchor			*/
}XmHTMLAnchor;

/*****
* Definition of a word (a word can be plain text, an image, a form member
* or a linebreak).
*****/
typedef struct _XmHTMLWord{
	int 				x;			/* x-position for this word				*/
	int					y;			/* y-position for this word				*/
	Dimension 			width;		/* pixel width of word					*/
	Dimension 			height;		/* pixel height of word					*/
	Cardinal 			line;		/* line for this word					*/
	ObjectType 			type;		/* type of word, used by <pre>,<img>	*/
	String 				word;		/* word to display						*/
	int 				len;		/* string length of word				*/
	XmHTMLfont	 		*font;		/* font to use							*/
	Byte 				line_data;	/* line data (underline/strikeout)		*/
	int				spacing;	/* leading/trailing/nospace allowed		*/
	AllEvents			*events;	/* events to be served					*/
	struct _XmHTMLImage *image;		/* when this is an image				*/
	struct _XmHTMLForm	*form;		/* when this is a form element			*/
	struct _XmHTMLWord	*base;		/* baseline word for a line				*/
	struct _XmHTMLWord	*self; 		/* ptr to itself, for anchor adjustment	*/
	struct _XmHTMLObjectTable *owner;	/* owner of this worddata			*/
}XmHTMLWord;

/* area definition. See map.c for the full definition */
typedef struct _mapArea mapArea;

/*****
* Client-side imagemap information
* mapArea is a transparent object and is defined in map.c
*****/
typedef struct _XmHTMLImageMap{
	String				name;		/* name of map			*/
	int					nareas;		/* no of areas			*/
	mapArea				*areas;		/* list of areas		*/
	struct _XmHTMLImageMap *next;	/* ptr to next imagemap */
}XmHTMLImageMap;

/*****
* XmHTML's internal image format.
* One very important thing to note is that the meaning of the (width,height)
* and (swidth,sheight) members of this structure is exactly *OPPOSITE* to
* the members with the same name in the public structures (XmImageInfo and
* XmImage).
*****/
typedef struct _XmHTMLImage{
	/* Normal image data */
	Byte			magic;			/* structure identifier */
	String			url;			/* raw src specification */
	XmImageInfo		*html_image;	/* local image data */
	TPixmap			pixmap;			/* actual image */
	TPixmap			clip;			/* for transparant pixmaps */
	unsigned long	options;		/* image option bits */
	int				width;			/* resulting image width */
	int				height;			/* resulting image height */
	int				npixels;		/* no of allocated pixels */
	XCC             xcc;			/* a lot of visual info */

	/* Possible <IMG> attributes */
	int				swidth;			/* requested image width */
	int				sheight;		/* requested image height */
	String			alt;			/* alternative image text */
	Alignment		align;			/* image alignment */
	Imagemap		map_type;		/* type of map to use */
	String			map_url;		/* image map url/name */
	Dimension		border;			/* image border thickness */
	Dimension		hspace;			/* horizontal spacing */
	Dimension		vspace;			/* vertical spacing */

	struct _XmHTMLObjectTable *owner;	/* owner of this image */
	struct _XmHTMLImage *child;		/* ptr to copies of this image */
	struct _XmHTMLImage *next;		/* ptr to next image */

	/* animation data */
	XmImageFrame 	*frames;		/* array of animation frames */
	int				nframes;		/* no of frames following */
	int				current_frame;	/* current frame count */
	int				current_loop;	/* current loop count */
	int				loop_count;		/* maximum loop count */
	TIntervalId	proc_id;		/* timer id for animations */
	XmHTMLWidget	html;			/* image owner */
	TAppContext	context;		/* Application context for animations */
 
 	/* other data */
	AllEvents			*events;	/* events to be served */
}XmHTMLImage;

/*****
* The following structure is used to mimic file access in memory.
*****/
typedef struct{
	char *file;					/* name of file */
	Byte *buffer;				/* memory buffer */
	Byte *curr_pos;				/* current position in buffer */
	size_t next;				/* current block count */
	size_t size;				/* total size of in-memory file */
	Boolean may_free;			/* True when we must free this block */
	unsigned char type;			/* type of image */
	int depth;					/* depth of this image */
}ImageBuffer;

/*****
* Definition of HTML form components
*****/
typedef struct _XmHTMLForm{
	int	 			x;				/* x-position for this widget */
	int 			y;				/* y-position for this widget */
	Dimension 		width;			/* width of this widget */
	Dimension 		height;			/* height of this widget */
	Cardinal		line;			/* starting line number of this object */
	TWidget 			w;				/* TWidget ID */
	TWidget			child;			/* child id for scrolled stuff */
	String 			name;			/* name for this TWidget */
	Byte 			type;			/* TWidget type (see HTML.h) */
	int				size;			/* cols in text(field)/items in select */
	int				maxlength;		/* max chars to enter/rows in textarea */
	String 			value;			/* default text */
	String			content;		/* entered text(field) contents */
	Alignment		align;			/* image/file browse button position */
	Boolean			multiple;		/* multiple select flag */
	int				selected;		/* default state */
	Boolean 		checked;		/* check value for option/radio buttons. */
	Boolean 		mapped;			/* True when displayed, false otherwise */
	struct _XmHTMLForm *options;	/* option items for select */
	struct _XmHTMLObjectTable *data;/* owning data object */
	struct _XmHTMLFormData *parent;	/* parent form */
	struct _XmHTMLForm *prev;		/* ptr to previous record */
	struct _XmHTMLForm *next;		/* ptr to next record */
}XmHTMLForm;

/*****
* Definition of form data
*****/
typedef struct _XmHTMLFormData{
	Widget html;					/* owner of this form */
	String action;					/* destination url/cgi-bin */
	int method;						/* XmHTML_FORM_GET,POST,PIPE */
	String enctype;					/* form encoding */
	int ncomponents;				/* no of items in this form */
	Widget fileSB;					/* input == file */
	XmHTMLForm *components;			/* list of form items */
	struct _XmHTMLFormData *prev;	/* ptr to previous form */
	struct _XmHTMLFormData *next;	/* ptr to next form */
}XmHTMLFormData;

/*****
* Definition of XmHTML tables
*
* Dimensions:
* positive -> absolute number;
* negative -> relative number;
* 0        -> no dimension specified;
*
* Each component in a table has a set of core properties. Properties are
* inherited from top to bottom and can be overriden.
*
* Content containers render the contents of all objects between
* start (inclusive) and end (exclusive).
*****/
/* possible framing types */
typedef enum{
	TFRAME_VOID = 0,			/* no borders		*/
	TFRAME_ABOVE,				/* only top side	*/
	TFRAME_BELOW,				/* only bottom side	*/
	TFRAME_LEFT,				/* only left side	*/
	TFRAME_RIGHT,				/* only right side	*/
	TFRAME_HSIDES,				/* top & bottom		*/
	TFRAME_VSIDES,				/* left & right		*/
	TFRAME_BOX,					/* all sides		*/
	TFRAME_BORDER				/* all sides		*/
}TableFraming;

/* possible ruling types */
typedef enum{
	TRULE_NONE = 0,				/* no rules			*/
	TRULE_GROUPS,				/* only colgroups	*/
	TRULE_ROWS,					/* only rows		*/
	TRULE_COLS,					/* only columns		*/
	TRULE_ALL					/* all cells		*/
}TableRuling;

/*****
* Properties shared by all table elements. These are inherited from top to
* bottom and can be overriden by the appropriate tag attributes.
*****/
typedef struct _TableProperties{
	int				border;				/* border width, 0 = noborder	*/
	Alignment		halign;				/* content horizontal alignment	*/
	Alignment		valign;				/* content vertical alignment	*/
	Pixel			bg;					/* content background color		*/
	XmHTMLImage		*bg_image;			/* content background image		*/
	TableFraming	framing;			/* what frame should we use?	*/
	TableRuling		ruling;				/* what rules should we draw?	*/
	Boolean                 nowrap;                         /* don't break lines            */
}TableProperties;

/*****
* a Cell, can be a header cell or a simple cell.
*****/
typedef struct _TableCell{
	Boolean			header;				/* True if a header cell	*/
	int				width;				/* suggested cell width		*/
	int				height;				/* suggested cell height	*/
	int				rowspan;			/* no of rows spanned		*/
	int				colspan;			/* no of cells spanned		*/
	TableProperties	*properties;		/* properties for this cell	*/
	struct _XmHTMLObjectTable *start;	/* first object to render	*/
	struct _XmHTMLObjectTable *end;		/* last object to render	*/
	struct _XmHTMLObjectTable *owner;	/* owning object			*/
	struct _TableRow *parent;			/* parent of this cell		*/
}TableCell;

/* A row. A row consists of a number of Cells */
typedef struct _TableRow{
	TableCell		*cells;				/* all cells in this row	*/
	int				ncells;				/* no of cells in row		*/
	int				lastcell;			/* last used cell			*/
	TableProperties	*properties;		/* properties for this row	*/
	struct _XmHTMLObjectTable *start;	/* first object to render	*/
	struct _XmHTMLObjectTable *end;		/* last object to render	*/
	struct _XmHTMLObjectTable *owner;	/* owning object			*/
	struct _XmHTMLTable *parent;		/* parent of this row		*/
}TableRow;

/*****
* A table. A table consists of a Caption and a number of Rows
* The caption is a special row: it has only one cell that stretches
* across the entire table: itself.
*****/
typedef struct _XmHTMLTable{
	/* overall table properties */
	int				width;				/* suggested table width	*/
	int				hmargin;			/* horizontal cell margin	*/
	int				vmargin;			/* vertical cell margin		*/
	int				hpadding;			/* horizontal cell padding	*/
	int				vpadding;			/* vertical row padding		*/
	int				ncols;				/* no of columns			*/
	TableProperties *properties;		/* master table properties	*/

	TableRow		*caption;			/* table caption			*/
	TableRow		*rows;				/* all table rows			*/
	int				nrows;				/* no of rows in table		*/
	int				lastrow;			/* last used row			*/

	struct _XmHTMLTable *parent;		/* parent table (for childs)*/
	struct _XmHTMLTable *childs;		/* table child				*/
	int				nchilds;			/* no of child tables		*/
	int				lastchild;			/* last used table			*/

	struct _XmHTMLObjectTable *start;	/* first object in table	*/
	struct _XmHTMLObjectTable *end;		/* last object in table		*/

	struct _XmHTMLObjectTable *owner;	/* owner of this table		*/

	struct _XmHTMLTable *next;			/* ptr to next table		*/
}XmHTMLTable;

/*****
* Definition of formatted HTML elements
*****/
typedef struct _XmHTMLObjectTable{
	int				x;				/* x position for this element		*/
	int				y;				/* y position for this element		*/
	Dimension		width;			/* width of this element			*/
	Dimension		height;			/* height of this element			*/
	Cardinal		line;			/* starting line number of this object */
	Cardinal		id;				/* object identifier (anchors only)	*/
	ObjectType		object_type;	/* element type						*/
	String			text;			/* cleaned text						*/
	Byte			text_data;		/* text/image/anchor data bits		*/
	int				len;			/* length of text or width of a rule*/
	int				y_offset;		/* offset for sub/sup, <hr> noshade flag */
	int				x_offset;		/* additional offset for sub/sup	*/
	XmHTMLObject	*object;		/* object data (raw text)			*/
	XmHTMLAnchor	*anchor;		/* ptr to anchor data				*/
	XmHTMLWord		*words;			/* words to be displayed			*/
	XmHTMLForm		*form;			/* form data						*/
	XmHTMLTable		*table;			/* table data						*/
	int				n_words;		/* number of words					*/
	Byte			anchor_state;	/* anchor selection state identifier*/
	Alignment		halign;			/* horizontal line alignment		*/
	int				linefeed;		/* linebreak type					*/
	Dimension		ident;			/* xoffset for list indentation		*/
	Marker			marker;			/* marker to use in lists			*/		
	int				list_level;		/* current count of list element.	*/
	XmHTMLfont		*font;			/* font to be used for this object	*/
	Pixel			fg;				/* foreground color for this object	*/
	Pixel			bg;				/* background color for this object	*/
	struct _XmHTMLObjectTable *next;
	struct _XmHTMLObjectTable *prev;
}XmHTMLObjectTable, *XmHTMLObjectTableElement;

/*****
*
*****/
typedef struct _XmHTMLSelection{
	XmHTMLObjectTable *start;		/* selection start object */
	int start_word;					/* first word index */
	int start_nwords;				/* word count */

	XmHTMLObjectTable *end;			/* ending start object (inclusive) */
	int end_word;					/* first word index */
	int end_nwords;					/* word count */
}XmHTMLSelection;

/*****
* definition of frame childs
*****/
typedef struct _XmHTMLFrameWidget{
	int				x;				/* computed frame x-position */
	int				y;				/* computed frame y-position */
	Dimension		width;			/* computed frame width */
	Dimension		height;			/* computed frame height */
	int				xs;				/* saved x-position */
	int				ys;				/* saved y-position */
	Dimension		width_s;		/* saved frame width */
	Dimension		height_s;		/* saved frame height */
	FrameSize		width_type;		/* horizontal frame size specification */
	FrameSize		height_type;	/* vertical frame size specification */
	FrameScrolling	scroll_type;	/* frame scrolling */
	String			src;			/* source document */
	String			name;			/* internal frame name */
	Dimension		margin_width;	/* frame margin width */
	Dimension		margin_height;	/* frame margin height */
	Boolean			resize;			/* may we resize this frame? */
	int				border;			/* add a border around this frame? */
	TWidget			frame;			/* XmHTMLWidget id for this frame */
}XmHTMLFrameWidget;

/*****
* Parser state stack object
*****/
typedef struct _stateStack{
	htmlEnum id;					/* current state id */
	struct _stateStack *next;		/* ptr to next record */
}stateStack;

/*****
* Progressive Loading Context. This is an opaque object fully defined in
* plc.h. It's a rather complex thing for all objects than can be loaded
* progressively. It maintains the state of each object (data as well as data
* processing functions) and does a bunch of nifty things.
*****/ 
typedef struct _PLC *PLCPtr;

/*****
* This struct is required to properly perform alpha channel processing.
* It contains information about the current background setting.
*
* Alpha channel processing is done for PNG images with (obviously) an alpha
* channel. The data used for creating the pixmap is a merger of the original
* image data with the current background setting (fixed color or an image).
* When a document with such an image contains a background image, XmHTML needs
* to redo this alpha processing whenever the document layout is changed: the
* exact result of this merger depends on the position of the image. This can
* be a rather slow process (alpha channels require floating point ops), and
* by at least storing the current background info we can achieve some
* performance increase. 
*****/
typedef struct _AlphaChannelInfo{
	int fb_maxsample;			/* frame buffer maximum sample value */
	int background[3];			/* solid background color: R, G, B */
	int ncolors;				/* size of background image colormap */
	TColor *bg_cmap;			/* background image colormap */
}AlphaChannelInfo, *AlphaPtr;

/*****
* XmHTMLPart definition 
*****/
typedef struct _XmHTMLPart {
	/* Original document Resources */
	String				value;		/* raw HTML text, copied to the parser */
	String				source;		/* copy used by XmHTML */
	String				mime_type;	/* mime type of this text/image (?) */
	Byte				mime_id;	/* internal mime id */

	/* Anchor resources */
	TCursor				anchor_cursor;
	Boolean				anchor_display_cursor;
	Boolean				anchor_buttons;

	/* anchor colors */
	Pixel				anchor_fg;
	Pixel				anchor_visited_fg;
	Pixel				anchor_target_fg;
	Pixel				anchor_activated_fg;
	Pixel				anchor_activated_bg;
	Boolean				highlight_on_enter;		/* anchor highlighting */

	/* anchor underlining styles */
	Byte				anchor_underline_type;
	Byte				anchor_visited_underline_type;
	Byte				anchor_target_underline_type;

	/* internal underlining styles, translated from above */
	Byte				anchor_line;
	Byte				anchor_target_line;
	Byte				anchor_visited_line;

	Position			anchor_position_x;	/* for server-side imagemaps */
	Position			anchor_position_y;	/* for server-side imagemaps */
	XmHTMLObjectTable	*armed_anchor;		/* current anchor */
	XmHTMLAnchor		*anchor_current_cursor_element;
	XmHTMLAnchorProc        anchor_visited_proc;
	
	/* background image/color and text color resources */
	Boolean				body_colors_enabled;
	Boolean				body_images_enabled;
	Boolean				allow_color_switching;
	Boolean				allow_form_coloring;	/* body colors on HTML forms */
	Boolean				freeze_animations;
	Pixel				body_bg;		/* current background color */
	Pixel				body_fg;		/* current foreground color */
	String				body_image_url;	/* background image location */
	String				def_body_image_url;	/* default bg image location */
	XmHTMLImage         *body_image;	/* background image data */
	TGC					bg_gc;			/* background render gc */

	/* Font resources */
	String				charset;
	String				font_family;
	String				font_family_fixed;
	String				font_sizes;
	String				font_sizes_fixed;
	XmHTMLfont			*default_font;
	Byte				string_direction;
	Byte				alignment;
	Alignment			default_halign;
	Boolean				allow_font_switching;

	/* Image resources */
	Boolean				images_enabled;	/* True -> show images */
	int					max_image_colors; /* 0 -> as much as possible */
	float				screen_gamma;	/* gamma correction for this display */
	XmImageProc			image_proc;		/* external image loader */
	XmImageGifProc		gif_proc;		/* external gif decoder */
	String				zCmd;			/* uncompress command for LZWStream */
	XmHTMLImage			*images;		/* list of images in current doc */
	Boolean				delayed_creation;	/* delayed image creation */
	XCC					xcc;			/* a lot of visual info */

	Byte				map_to_palette;	/* if and how to map to palette */
	String				palette;		/* palette to use */

	/* Imagemap resources */
	XmHTMLImageMap		*image_maps;	/* array of client-side imagemaps */
	Pixel				imagemap_fg;	/* bounding box color */
	Boolean				imagemap_draw;	/* draw imagemap bounding boxes */

	/* Frame resources */
	Boolean				is_frame;		/* true when this is a frame */
	FrameScrolling		scroll_type;	/* frame scrolling */
	int					frame_border;	/* add a border to the frames? */
	int					nframes;		/* no of frames managed */
	XmHTMLFrameWidget	**frames;		/* list of frame childs */

	/* Document resources */
	Boolean				strict_checking;
	Boolean				enable_outlining;
	Byte				bad_html_warnings;
	XtPointer			client_data;	/* client_data for functional res. */

	/* Private Resources */
	Dimension			margin_width;	/* document margins */
	Dimension			margin_height;
	TWidget				work_area;		/* render area */
	Dimension			work_width;		/* render area dimensions */
	Dimension			work_height;

	Boolean				resize_height;	/* True -> autosize vertically */
	Boolean				resize_width;	/* True -> autosize horizontally */

	/* Progressive Loader Context buffer and interval */
	PLCPtr				plc_buffer;		/* PLC ringbuffer */
	int					num_plcs;		/* no of PLC's in ringbuffer */
	int					plc_def_delay;	/* default PLC timeout delay */
	int					plc_delay;		/* PLC timeout delay */
	int					plc_min_delay;	/* PLC minimum timeout delay */
	int					plc_max_delay;	/* PLC maximum timeout delay */
	TIntervalId		plc_proc_id;	/* timer id for main plc cycler */
	XmHTMLGetDataProc	get_data;		/* PLC data request function */
	XmHTMLEndDataProc	end_data;		/* PLC end signal function */
	Boolean				plc_suspended;	/* Global PLC suspension flag */
	TGC					plc_gc;			/* gc used by all plc's */

	/* perform final dithering pass/use image colors */
	Byte				perfect_colors;

	/* Internal stuff for alpha channelled PNG images */
	AlphaPtr			alpha_buffer;
	Byte				rgb_conv_mode;	/* 24 to 8bit conversion method */
	Byte				alpha_processing;	/* do alpha channel stuff? */

	/* 
	* Fallback colors, required for proper color resetting between documents
	* with and without a <body> color spec.
	*/
	Pixel				anchor_fg_save;
	Pixel				anchor_visited_fg_save;
	Pixel				anchor_target_fg_save;
	Pixel				anchor_activated_fg_save;
	Pixel				anchor_activated_bg_save;
	Pixel				body_bg_save;
	Pixel				body_fg_save;

	/* Scrollbar resources */
	TWidget				hsb;			/* vertical scrollbar TWidget id */
	TWidget				vsb;			/* horizontal scrollbar TWidget id */
	Byte				sb_policy;		/* scrollbar display policy */
	Byte				sb_placement;	/* scrollbar placement policy */
	int					scroll_x;		/* current horizontal position */
	int					scroll_y;		/* current vertical position */
	Boolean				needs_hsb;		/* True -> hsb required */
	Boolean				needs_vsb;		/* True -> vsb required */

	/* Callback resources */
	TCallbackList		activate_callback;
	TCallbackList		arm_callback;
	TCallbackList		anchor_track_callback;
	TCallbackList		frame_callback;
	TCallbackList		form_callback;
	TCallbackList		input_callback;
	TCallbackList		link_callback;
	TCallbackList		motion_track_callback;
	TCallbackList		imagemap_callback;
	TCallbackList		document_callback;
	TCallbackList		focus_callback;
	TCallbackList		losing_focus_callback;
	TCallbackList		event_callback;
	Boolean				need_tracking;		/* serve mouse/focus tracking? */

 	XmHTMLEventProc		event_proc;	/* HTML4.0 script processing proc */
 	HTEvent				*events;		/* HTML4.0 event data	*/
 	int					nevents;		/* no of events watched	*/

	/* Formatted document resources */
	int					formatted_width;	/* total width of document */
	int					formatted_height;	/* total height of document */
	int					num_anchors;		/* total no of anchors in doc */
	int					num_named_anchors;	/* total no of named anchors */
	int					anchor_words;	/* total no of anchor words */
	XmHTMLWord			*anchors;		/* for quick anchor lookup */
	XmHTMLObject		*elements;		/* unfiltered parser output */
	XmHTMLObjectTable	*named_anchors; /* for quick named anchor lookup */
	XmHTMLAnchor		*anchor_data;	/* parsed anchor data */
	XmHTMLObjectTable	*formatted;		/* display object data */
	XmHTMLObjectTable	*paint_start;	/* first paint command */
	XmHTMLObjectTable	*paint_end;		/* last paint command */
	int					paint_x;		/* horizontal paint start x-pos */
	int					paint_y;		/* vertical paint start y-pos */
	int					paint_width;	/* horizontal paint end x-pos */
	int					paint_height;	/* vertical paint end y-pos */
	Cardinal			top_line;		/* current topline */
	Cardinal			nlines;			/* no of lines in document */

	/* Table resources */
	XmHTMLTable			*tables;		/* list of all tables */

	/* Anchor activation resources */
	int					press_x;		/* ptr coordinates */
	int					press_y;
	Time				pressed_time;	/* time of anchor activation */
	Time				selected_time;	/* unused for now */
	XmHTMLAnchor		*selected;		/* selected anchor */
	XmHTMLObjectTable	*current_anchor;/* selected object */

	/* Text selection resources */
	XmHTMLObjectTable	*selection;		/* reserved for future use */
	int					select_start;	/* reserved for future use */
	int					select_end;		/* reserved for future use */

	/* HTML Form resources */
	XmHTMLFormData		*form_data;		/* all forms in the current document */

	/* Misc. resources */
	int					repeat_delay;	/* keyboard and scrollbar delay */
	TGC					gc;				/* main rendering gc */
	Boolean				in_layout;		/* layout blocking flag.
										 * Also used as SetValues blocking
										 * flag by the parser.
										 */
	int					visibility;		/* visibility state of work_area */

	Boolean				debug_disable_warnings;	/* warning msg blocking flag */
	Boolean				debug_full_output;	/* allow output from FullDebug */
	Boolean				debug_save_clipmasks; /* save clipmasks as bitmaps */
	Boolean				debug_no_loopcount;	/* ignore loop_count in anims */
	String				debug_prefix;		/* debug file prefix */
	String				debug_levels;		/* debug levels to enable */

	unsigned char pad[64];				/* reserved for future use */

}XmHTMLPart;

#ifdef WITH_MOTIF
typedef struct _XmHTMLRec
{
	CorePart		core;
	CompositePart	composite;
	ConstraintPart	constraint;
	XmManagerPart	manager;
	XmHTMLPart		html;
} XmHTMLRec;

/* Define subclassing level index to be used with ResolvePartOffset */
#define XmHTMLIndex		(XmManagerIndex+1)

externalref XmHTMLClassRec xmHTMLClassRec;
#endif

/*****
* Pull in internal function proto's when building the library.
*****/
#ifdef _LIBRARY
#include "XmHTMLI.h"
#endif

#ifndef WITH_MOTIF
#    include "gtk-xmhtml-p.h"
#endif

/* Don't add anything after this endif! */
#endif /* _XmHTMLP_h_ */
