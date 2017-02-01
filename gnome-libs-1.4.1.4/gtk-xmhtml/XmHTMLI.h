/*****
* XmHTMLI.h : XmHTML internal function proto's.
*             Only required when building the XmHTML Library.
*             If you whish to include this file, it *must* be included
*             AFTER XmHTMLP.h as it references a number of structures defined
*             in that header.
*
* This file Version	$Revision: 1.7 $
*
* Creation date:		Tue Aug 19 16:03:22 GMT+0100 1997
* Last modification: 	$Date: 1999/07/29 01:26:28 $
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
* modify it under the terms of the GNU [Library] General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU [Library] General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/
/*****
* $Source: /cvs/gnome/gnome-libs/gtk-xmhtml/Attic/XmHTMLI.h,v $
*****/
/*****
* ChangeLog 
* $Log: XmHTMLI.h,v $
* Revision 1.7  1999/07/29 01:26:28  sopwith
*
*
* Fix all warnings.
*
* Revision 1.6  1999/04/08 19:32:52  unammx
*
*
* 1999-04-08  Miguel de Icaza  <miguel@nuclecu.unam.mx>
*
* 	* configure.in (LIBS): Test for existance of dlfcn.h header.  Make
* 	libgnorba/goad.c only use shared lib deps if the system has
* 	dlfcn.h.
*
* 	I am suprised that nobody ever fixed this.
*
* 	Add a test for seteuid, it is not available on HP-UX.
*
* 	* libgnorba/goad.c: Do include dlfcn.h only if available on this
* 	system
*
* 1999-04-08  Miguel de Icaza  <miguel@nuclecu.unam.mx>
*
* 	* XmHTMLI.h: Always use full prototypes.  Fixes compilation on
* 	solaris-2.6
*
* 1999-04-08  Miguel de Icaza  <miguel@nuclecu.unam.mx>
*
* 	* gnome-pty-helper.c (init_msg_pass): Return a value.
*
* Revision 1.5  1997/12/29 22:16:22  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.4  1997/12/25 01:34:09  unammx
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
* Revision 1.3  1997/12/18 23:00:16  unammx
* More fixes and added PNG support. - Federico
*
* Revision 1.2  1997/12/11 21:20:21  unammx
* Step 2: more gtk/xmhtml code, still non-working - mig
*
* Revision 1.1  1997/11/28 03:38:55  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.2  1997/10/23 00:24:46  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.1  1997/08/30 00:07:31  newt
* Initial Revision
*
*****/ 

#ifndef _XmHTMLI_h_
#define _XmHTMLI_h_

_XFUNCPROTOBEGIN

/*****
* When XmHTML_ERROR_FUNCS is defined, only the error functions (at the end
* of this include file) will be visible to the outside world.
*****/
#ifndef XmHTML_ERROR_FUNCS

/* usefull macros */
#define Abs(x)		((x) < 0 ? -(x) : (x))
#define Max(x,y)	(((x) > (y)) ? (x) : (y))
#define Min(x,y)	(((x) < (y)) ? (x) : (y))
#define FONTHEIGHT(f) ((f)->max_bounds.ascent + (f)->max_bounds.descent)
#define FnHeight(f) ((f)->ascent + (f)->descent)

/* RANGE forces a to be in the range b..c (inclusive) */
#define RANGE(a,b,c) { if (a < b) a = b;  if (a > c) a = c; }

/*********************************************************************
* @Module: parse.c 
* @Description: XmHTML HTML parser
*
* @Defines:
* struct Parser: parser object
*
* @Exports:
* _XmHTMLparseHTML    : raw HTML parser.
* _XmHTMLFreeObjects  : free the given parser tree.
* _XmHTMLTextGetSTring: create a HTML source document from the given
*                       parser tree.
* _ParserCreate       : create a parser Object.
* _ParserDelete       : delete a parser Object.
* _ParserIsBodyElement: check whether an element may appear inside
*                       the <BODY> tag.
* _ParserIsElementTerminated  : check if an element has a terminating
*                               counterpart.
* _ParserCheckElementOccurance: check whether presence of an element in
*                               in a certain state is allowed.
* _ParserCheckElementContent  : check whether an element is allowed to
*                               appear in a certain state.
* _ParserCutComment   : cut a HTML comment from the input stream.
* _ParserNewObject    : allocate a new object.
* _ParserPushState    : push a parser state on the stack.
* _ParserPopState     : pop a parser state from the stack.
* _ParserOnStack      : check if the given element is on the stack.
* _ParserTokenToId    : convert a string to an internal element id.
* _ParserStoreElement : store a real element
* _ParserStoreTextElement: store a text element.
* _ParserStoreTextElementRtoL: store a text element and reverse it's contents.
* _ParserInsertElement: insert a new element in the list of elements.
* _ParserTerminateElement: terminate the given element. Backtracks both stack
*                       and element list
* _ParserCopyElement  : copy an element
* _ParserVerify       : verify (and correct) the presence of an element.
* _ParserVerifyVerification: verify whether the parser successfully
*                       verified/repaired a document.
*
**********************************************************************/

extern XmHTMLObject *_XmHTMLparseHTML(XmHTMLWidget html, XmHTMLObject *old_list,
	char *input, XmHTMLWidget dest);

extern void _XmHTMLFreeObjects(XmHTMLObject *objects);

extern String _XmHTMLTextGetString(XmHTMLObject *objects);


typedef struct _Parser{
	String source;				/* text being parsed					*/
	int index;					/* last known position					*/
	int len;					/* length of input text					*/
	int num_lines;				/* current line count					*/
	Dimension line_len;			/* maximum line length so far			*/
	Dimension cnt;				/* current line length					*/
	/* running list of inserted elements */
	int num_elements;			/* no of tags inserted so far			*/
	int num_text;				/* no of text elements inserted so far	*/
	XmHTMLObject *head;			/* head of object list					*/
	XmHTMLObject *current;		/* lastly inserted element				*/
	stateStack state_base;		/* stack base point						*/
	stateStack *state_stack;	/* actual stack							*/
	int cstart;					/* current element start position		*/
	int cend;					/* current element end position			*/
	int inserted;				/* no of auto-inserted chars			*/
	Cardinal err_count;			/* no of errors so far					*/
	Cardinal loop_count;		/* no of loops made so far				*/
	Boolean strict_checking;	/* HTML 3.2 looseness flag				*/
	Boolean have_body;			/* indicates presence of <body> tag		*/
	Byte warn;				/* warn about bad html constructs		*/
	Boolean bad_html;			/* bad HTML document flag				*/
	Boolean html32;				/* HTML32 conforming document flag		*/
	Boolean	automatic;			/* when in automatic mode				*/
	TWidget widget;				/* for the warning messages				*/
}Parser;

extern Parser *_ParserCreate(TWidget w);

extern void _ParserDelete(Parser *parser);
 
extern Boolean _ParserIsBodyElement(htmlEnum id);

extern Boolean _ParserIsElementTerminated(htmlEnum id);

extern int _ParserCheckElementOccurance(Parser *parser, htmlEnum current,
	htmlEnum state);

extern Boolean _ParserCheckElementContent(Parser *parser, htmlEnum current,
	htmlEnum state);

extern String _ParserCutComment(Parser *parser, String start);

extern XmHTMLObject *_ParserNewObject(Parser *parser, htmlEnum id,
	char *element, char *attributes, Boolean is_end, Boolean terminated);

extern void _ParserPushState(Parser *parser, htmlEnum id);

extern htmlEnum _ParserPopState(Parser *parser);

extern Boolean _ParserOnStack(Parser *parser, htmlEnum id);

extern htmlEnum _ParserTokenToId(Parser *parser, String token, Boolean warn);

extern String _ParserStoreElement(Parser *parser, char *start, char *end);

extern void _ParserStoreTextElement(Parser *parser, char *start, char *end);

extern void _ParserStoreTextElementRtoL(Parser *parser, char *start, char *end);

extern void _ParserInsertElement(Parser *parser, String element,
	htmlEnum new_id, Boolean is_end);

extern Boolean _ParserTerminateElement(Parser *parser, String element,
	htmlEnum current, htmlEnum expect);

extern void _ParserCopyElement(Parser *parser, XmHTMLObject *src,
		Boolean is_end);

extern int _ParserVerify(Parser *parser, htmlEnum id, Boolean is_end);

extern XmHTMLObject *_ParserVerifyVerification(XmHTMLObject *objects);

/* set of macros used by both parse.c and Parser.c */

/* elements for which a closing counterpart is optional */
#define OPTIONAL_CLOSURE(id) ((id) == HT_DD || (id) == HT_DT || \
	(id) == HT_LI || (id) == HT_P || (id) == HT_OPTION || (id) == HT_TD || \
	(id) == HT_TH || (id) == HT_TR)

/* physical/logical markup elements */
#define IS_MARKUP(id) ((id) == HT_TT || (id) == HT_I || (id) == HT_B || \
	(id) == HT_U || (id) == HT_STRIKE || (id) == HT_BIG || (id) == HT_SMALL || \
	(id) == HT_SUB || (id) == HT_SUP || (id) == HT_EM || (id) == HT_STRONG || \
	(id) == HT_DFN || (id) == HT_CODE || (id) == HT_SAMP || (id) == HT_KBD || \
	(id) == HT_VAR || (id) == HT_CITE || (id) == HT_FONT)

/* text containers */
#define IS_CONTAINER(id) ((id) == HT_BODY || (id) == HT_DIV || \
	(id) == HT_CENTER || (id) == HT_BLOCKQUOTE || (id) == HT_FORM || \
	(id) == HT_TH || (id) == HT_TD || (id) == HT_DD || (id) == HT_LI || \
	(id) == HT_NOFRAMES)

/* all elements that may be nested */
#define NESTED_ELEMENT(id) (IS_MARKUP(id) || (id) == HT_APPLET || \
	(id) == HT_BLOCKQUOTE || (id) == HT_DIV || (id) == HT_CENTER || \
	(id) == HT_FRAMESET)

/* other elements */
#define IS_MISC(id) ((id) == HT_P || (id) == HT_H1 || (id) == HT_H2 || \
	(id) == HT_H3 || (id) == HT_H4 || (id) == HT_H5 || (id) == HT_H6 || \
	(id) == HT_PRE || (id) == HT_ADDRESS || (id) == HT_APPLET || \
	(id) == HT_CAPTION || (id) == HT_A || (id) == HT_DT)

/*********************************************************************
* @Module: callbacks.c 
* @Description: XmHTML callback routines
*
* @Exports:
* _XmHTMLLinkCallback : XmNlinkCallback driver.
* _XmHTMLTrackCallback: XmNanchorTrackCallback driver.
* _XmHTMLActivateCallback: XmNactivateCallback drivers
* _XmHTMLDocumentCallback: XmNdocumentCallback drivers.
*
**********************************************************************/

extern void _XmHTMLLinkCallback(XmHTMLWidget html);

extern void _XmHTMLTrackCallback(XmHTMLWidget html, TEvent *event, 
	XmHTMLAnchor *anchor);

extern void _XmHTMLActivateCallback(XmHTMLWidget html, TEvent *event, 
	XmHTMLAnchor *anchor);

extern Boolean _XmHTMLDocumentCallback(XmHTMLWidget html, Boolean html32,
	Boolean verified, Boolean balanced, Boolean terminated, int pass_level);

/*********************************************************************
* @Module: format.c 
* @Description : XmHTML formatting routines, translate the parse output
*                to a set of objects suitable for displaying a HTML page.
*
* @Exports:
* _XmHTMLformatObjects: create a list of formatted objects.
* _XmHTMLNewAnchor    : allocate and fill a new anchor.
*
**********************************************************************/

extern void _XmHTMLformatObjects(XmHTMLWidget old, XmHTMLWidget html);

extern XmHTMLAnchor* _XmHTMLNewAnchor(XmHTMLWidget html, XmHTMLObject *object);

/*********************************************************************
* @Module: frames.c 
* @Description: XmHTML frame support
*
* @Exports:
* _XmHTMLCreateFrames  : create all required HTML frame widgets.
* _XmHTMLCheckForFrames: check for a frameset definition, destroying
*                        any previous frame lists.
* _XmHTMLReconfigureFrames   : recompute the frame layout after a widget
*                              resize.
* _XmHTMLFrameCreateCallback : XmNframeCallback driver for frame creation.
* _XmHTMLFrameDestroyCallback: XmNframeCallback driver for frame
*                              destruction.
* 
**********************************************************************/

extern Boolean _XmHTMLCreateFrames(XmHTMLWidget old, XmHTMLWidget html);

extern void _XmHTMLDestroyFrames(XmHTMLWidget html, int nframes);

extern TWidget _XmHTMLFrameCreateCallback(XmHTMLWidget html,
	XmHTMLFrameWidget *frame);

extern void _XmHTMLFrameDestroyCallback(XmHTMLWidget html, 
	XmHTMLFrameWidget *frame);

extern int _XmHTMLCheckForFrames(XmHTMLWidget html, XmHTMLObject *objects);

extern void _XmHTMLReconfigureFrames(XmHTMLWidget html);

/*********************************************************************
* @Module: forms.c 
* @Description: XmHTML HTML form support.
*
* @Exports:
* _XmHTMLStartForm       : start a new form.
* _XmHTMLEndForm         : terminate the form opened with _XmHTMLStartForm.
* _XmHTMLFreeForm        : destroy a given form.
* _XmHTMLFormAddInput    : add an input field to the current form.
* _XmHTMLFormAddSelect   : add a select field to the current form.
* _XmHTMLFormSelectAddOption: add an option to select opened with
*                          _XmHTMLFormAddSelect.
* _XmHTMLFormSelectClose : close the select opened with
*                          _XmHTMLFormAddSelect.
* _XmHTMLFormAddTextArea : add a text area to the current form.
* _XmHTMLFormActivate    : XmNformCallback driver.
* _XmHTMLFormReset       : reset a given form to it's default values.
* _XmHTMLProcessTraversal: form widget traversal handler.
*
**********************************************************************/

extern void _XmHTMLStartForm(XmHTMLWidget html, String attributes);

extern void _XmHTMLEndForm(XmHTMLWidget html);

extern XmHTMLForm *_XmHTMLFormAddInput(XmHTMLWidget html, String attributes);

extern XmHTMLForm *_XmHTMLFormAddSelect(XmHTMLWidget html, String attributes);

extern XmHTMLForm *_XmHTMLFormAddTextArea(XmHTMLWidget html,
	String attributes, String text);

extern void _XmHTMLFormSelectAddOption(XmHTMLWidget html, XmHTMLForm *entry,
	String attributes, String label);

extern void _XmHTMLFormSelectClose(XmHTMLWidget html, XmHTMLForm *entry);

extern void _XmHTMLFreeForm(XmHTMLWidget html, XmHTMLFormData *form);

extern void _XmHTMLFormActivate(XmHTMLWidget html, TEvent *event,
	XmHTMLForm *entry);

extern void _XmHTMLFormReset(XmHTMLWidget html, XmHTMLForm *entry);

extern void _XmHTMLProcessTraversal(TWidget w, int direction);

extern XmHTMLObjectTableElement _XmHTMLGetAnchorByValue(XmHTMLWidget html, 
	int anchor_id);

/*********************************************************************
* @Module: XmHTML.c 
* @Description: XmHTML Widget definition, widget methods and public
*               functions.
*
* @Exports:
* _XmHTMLGetAnchorByValue: finds the object of a named anchor by it's id.
* _XmHTMLGetAnchorByName : finds the object of a named anchor by it's name.
* _XmHTMLMoveToPos       : scroll the display area by a given amount.
*                          Scroll direction is given by the widget id.
* _XmHTMLCheckXCC        : create a XColorContext for the a HTML widget.
* _XmHTMLClearArea       : XClearArea for a XmHTML widget.
* _XmHTMLCvtStringToWarning: convert a XmCHTMLWarningType to it's internal
*                          representation.
*
**********************************************************************/

extern XmHTMLObjectTableElement _XmHTMLGetAnchorByName(XmHTMLWidget html, 
	String anchor);

extern void _XmHTMLMoveToPos(TWidget w, XmHTMLWidget html, int value);

extern void _XmHTMLCheckXCC(XmHTMLWidget html);

extern void _XmHTMLClearArea(XmHTMLWidget html, int x, int y, int width,
	int height);

extern Boolean _XmHTMLCvtStringToWarning(Display *dpy, XrmValuePtr args,
	Cardinal *num_args, XrmValuePtr from_val, XrmValuePtr to_val,
	XtPointer *converter_data);

/*********************************************************************
* @Module: paint.c 
* @Description: XmHTML rendering routines.
*
* @Exports:
* _XmHTMLPaint            : display a list of objects.
* _XmHTMLRestartAnimations: freeze or restart all animations in a document.
* _XmHTMLDrawImage        : refresh a single image.
*
**********************************************************************/
 
/* Pour given paint commands onto the display. */
extern void _XmHTMLPaint(XmHTMLWidget html, XmHTMLObjectTable *start,
	XmHTMLObjectTable *end);

extern void _XmHTMLRestartAnimations(XmHTMLWidget html);

extern void _XmHTMLDrawImage(XmHTMLWidget html, XmHTMLObjectTableElement data,
	int y_offset, Boolean from_timerCB);

/*********************************************************************
* @Module: layout.c 
* @Description : XmHTML layout computation routines
*
* @Exports:
* _XmHTMLComputeLayout: computes the full screen layout for a XmHTML
*                       widget.
*
**********************************************************************/

extern void _XmHTMLComputeLayout(XmHTMLWidget html);

/*********************************************************************
* @Module: StringUtil.c 
* @Description: string manipulators and HTML tag analyzers.
*
* @Exports:
* __my_translation_table   : lowercase translation table.
* my_upcase                : make a string all uppercase.
* my_locase                : make a string all lowercase.
* my_strcasestr            : case insensitive strstr.
* ToAsciiLower             : convert a number to all lowercase ASCII.
* ToAsciiUpper             : convert a number to all uppercase ASCII.
* ToRomanLower             : convert a number to all uppercase roman numerals.
* ToRomanUpper             : convert a number to all lowercase roman numerals.
* _XmHTMLExpandEscapes     : expand all escape sequences in the given text.
* _XmHTMLTagCheck          : Check the existance of a tag.
* _XmHTMLTagGetValue       : Get the value of a tag.
* _XmHTMLTagGetNumber      : Get the numerical value of a tag.
* _XmHTMLTagCheckNumber    : Get the absolute (positive no returned) or
*                            relative (negative no returned) value of a tag.
* _XmHTMLTagCheckValue     : check if the given tag exists.
* _XmHTMLGetImageAlignment : Retrieve the value of the ALIGN attribute on images.
* _XmHTMLGetHorizontalAlignment: Retrieve the value of the ALIGN attribute.
* _XmHTMLGetVerticalAlignment: Retrieve the value of the VALIGN attribute.
* _XmHTMLGetFraming        : Retrieve the value of the FRAME table attribute.
* _XmHTMLGetRuling         : Retrieve the value of the RULE table attribute
* _XmHTMLGetMaxLineLength  : Returns maximum width of a line in pixels of
*                            the current document or 75% of screen width,
*                            whatever is the smallest.
* 
**********************************************************************/

extern void my_upcase(char *string);
extern void my_locase(char *string);
extern char *my_strcasestr(const char *s1, const char *s2);
extern char *my_strndup(const char *s1, size_t len);
extern const Byte __my_translation_table[];
#define _FastLower(x) (__my_translation_table[(unsigned int)x])

#ifdef NEED_STRERROR
extern char *sys_errlist[];
extern int errno;
#define strerror(ERRNUM) sys_errlist[ERRNUM]
#endif

#ifdef NEED_STRCASECMP
# include <sys/types.h>
extern int my_strcasecmp(const char *s1, const char *s2);
extern int my_strncasecmp(const char *s1, const char *s2, size_t n);
#define strcasecmp(S1,S2) my_strcasecmp(S1,S2)
#define strncasecmp(S1,S2,N) my_strncasecmp(S1,S2,N)
#endif

extern String ToAsciiLower(int val);

extern String ToAsciiUpper(int val);

extern String ToRomanUpper(int val);

extern String ToRomanLower(int val);

extern void _XmHTMLExpandEscapes(char *string, Boolean warn);

extern Boolean _XmHTMLTagCheck(char *attributes, char *tag);

extern char *_XmHTMLTagGetValue(char *attributes, char *tag);

extern int _XmHTMLTagGetNumber(char *attributes, char *tag, int def);

extern int _XmHTMLTagCheckNumber(char *attributes, char *tag, int def);

extern Boolean _XmHTMLTagCheckValue(char *attributes, 
	char *tag, char *check);

extern Alignment _XmHTMLGetImageAlignment(char *attributes);

extern Alignment _XmHTMLGetHorizontalAlignment(char *attributes, 
	Alignment def_align);

extern Alignment _XmHTMLGetVerticalAlignment(char *attributes,
	Alignment def_align);

extern TableFraming _XmHTMLGetFraming(char *attributes, TableFraming def);

extern TableRuling _XmHTMLGetRuling(char *attributes, TableRuling def);

extern Dimension _XmHTMLGetMaxLineLength(XmHTMLWidget html);

/*********************************************************************
* @Module: colors.c 
* @Description : XmHTML *text* color allocation routines.
*
* @Exports:
* _XmHTMLGetPixelByName  : allocate and return the named pixel.
* _XmHTMLConfirmColor32  : check name of the given color. Only when
*                          XmNstrictHTMLChecking is True.
* _XmHTMLFreeColors      : free the colors allocated for the given widget.
* _XmHTMLRecomputeColors : Recompute top shadow, bottom shadow & highlight
*                          colors.
* _XmHTMLRecomputeHighlightColor: Recompute the highlight color given a
*                          background pixel.
* _XmHTMLAddPalette      : add a palette to the widget (used for dithering)
*
**********************************************************************/

extern Pixel _XmHTMLGetPixelByName(XmHTMLWidget html, String color,
	Pixel def_pixel);

extern Boolean _XmHTMLConfirmColor32(char *color);

extern void _XmHTMLFreeColors(XmHTMLWidget html);

extern void _XmHTMLRecomputeColors(XmHTMLWidget html);

extern void _XmHTMLRecomputeHighlightColor(XmHTMLWidget html, Pixel bg_color);

extern Boolean _XmHTMLAddPalette(XmHTMLWidget html);

/*********************************************************************
* @Module: images.c 
* @Description: XmHTML image loading/manipulation routines.
*
* @Defines:
* struct XmHTMLRawImageData: intermediate image data structure.
*
* @Exports:
* bitmap_bits[]           : bit array required for creating depth 1
*                           images (bitmaps).
* XmImageGifProc_plugin   : external gif decoder hook.
* XmImageGifzCmd_plugin   : external decompressor hook.
* _xmimage_cfg            : XmImage configuration hook.
* _XmHTMLGetImageType     : return type of image
* _XmHTMLImageFileToBuffer: read a file in a buffer
* _XmHTMLNewImage         : create a new image
* _XmHTMLLoadBodyImage    :load and set the body image.
* _XmHTMLImageUpdateChilds: update all copies of the given parent image.
* _XmHTMLImageCheckDelayedCreation: process all images that need rereading
*                           (alpha channel processing)
* _XmHTMLMakeAnimation    : create an animation for the given image.
* _XmHTMLInfoToPixmap     : create a pixmap from the given ImageInfo data.
* _XmHTMLReplaceOrUpdateImage: replace or update an image.
* _XmHTMLFreeImage        : Free private image data.
* _XmHTMLFreeImageInfo    : Free external image data.
* _XmHTMLReleaseImage     : Free an image and adjust the internal list of
*                           images.
* _XmHTMLCreateXImage     : create a new but empty XImage with given
*                           dimensions
* _XmHTMLFillXImage       : fill the given XImage.
* _XmHTMLReadBitmap       : read an X11 bitmap.
* _XmHTMLReadGIF          : read a GIF file.
* _XmHTMLReadFLG          : read a FLG file (Fast Loadable Graphic).
* _XmHTMLReadXPM          : read an X11 XPM image.
* _XmHTMLReadPNG          : read a PNG image.
* _XmHTMLReadJPEG         : read a JPEG image.
* _XmHTMLGifReadOK        : read a number of bytes from a GIF datafile.
* _XmHTMLGifGetDataBlock  : read a GIF datablock from a GIF datafile.
* _XmHTMLIsGifAnimated    : check whether a GIF is animated or not
* _XmHTMLGifAnimInit      : Initialize gif animation reading
* _XmHTMLGifAnimNextFrame : read a frame from an animated gif file.
* _XmHTMLGifAnimTerminate : wrap up animated gif reading.
* _XmHTMLCreateXpmFromData: read an X11 XPM image from raw XPM data.
* _XmHTMLReReadPNG        : process alpha channelled PNG images.
*
**********************************************************************/
extern Byte bitmap_bits[];
extern XmImageGifProc XmImageGifProc_plugin;
extern String XmImageGifzCmd_plugin;
extern XmImageConfig *_xmimage_cfg;

typedef struct _XmHTMLRawImageData{
	Byte			*data;			/* raw image data */
	Byte			*alpha;			/* alpha channel data */
	int				width;			/* image width in pixels */
	int				height;			/* image height in pixels */
	int				bg;				/* transparent pixel index */
	TColor			*cmap;			/* colormap for this image */
	int				cmapsize;		/* actual no of colors in image colormap */
	Byte			type;			/* type of image */
	Byte			color_class;	/* color class for this image */
	Boolean			delayed_creation;
	float			fg_gamma;		/* image foreground gamma */
}XmHTMLRawImageData;

#define FreePixmap(DPY,PIX) if((PIX)!= None) Toolkit_Free_Pixmap((DPY),(PIX))

/* check whether the body image is fully loaded */
#define BodyImageLoaded(IMAGE) \
	((IMAGE) ? (!ImageInfoDelayed((IMAGE)) && \
		!ImageInfoProgressive((IMAGE))) : True)

/* XmHTMLImage macros */
#define ImageIsBackground(IMG)		((IMG)->options & IMG_ISBACKGROUND)
#define ImageIsInternal(IMG)		((IMG)->options & IMG_ISINTERNAL)
#define ImageIsCopy(IMG)			((IMG)->options & IMG_ISCOPY)
#define ImageIsAnim(IMG)			((IMG)->options & IMG_ISANIM)
#define ImageFrameRefresh(IMG)		((IMG)->options & IMG_FRAMEREFRESH)
#define ImageHasDimensions(IMG)		((IMG)->options & IMG_HASDIMENSIONS)
#define ImageHasState(IMG)			((IMG)->options & IMG_HASSTATE)
#define ImageInfoFreed(IMG)			((IMG)->options & IMG_INFOFREED)
#define ImageDelayedCreation(IMG)	((IMG)->options & IMG_DELAYED_CREATION)
#define ImageIsOrphaned(IMG)		((IMG)->options & IMG_ORPHANED)
#define ImageIsProgressive(IMG)		((IMG)->options & IMG_PROGRESSIVE)

/* XmImageInfo macros */
#define ImageInfoDelayed(INFO)		((INFO)->options & XmIMAGE_DELAYED)
#define ImageInfoFreeLater(INFO)	((INFO)->options & XmIMAGE_DEFERRED_FREE)
#define ImageInfoFreeNow(INFO)		((INFO)->options & XmIMAGE_IMMEDIATE_FREE)
#define ImageInfoScale(INFO)		((INFO)->options & XmIMAGE_ALLOW_SCALE)
#define ImageInfoRGBSingle(INFO)	((INFO)->options & XmIMAGE_RGB_SINGLE)
#define ImageInfoShared(INFO)		((INFO)->options & XmIMAGE_SHARED_DATA)
#define ImageInfoClipmask(INFO)		((INFO)->options & XmIMAGE_CLIPMASK)
#define ImageInfoDelayedCreation(INFO) \
									((INFO)->options & XmIMAGE_DELAYED_CREATION)
#define ImageInfoProgressive(INFO)	((INFO)->options & XmIMAGE_PROGRESSIVE)

/* rewind the given image buffer */
#define	RewindImageBuffer(IB)	do{ \
	(IB)->next = (size_t)0; \
	(IB)->curr_pos = (IB)->buffer; \
}while(0)

/* free the given image buffer */
#define FreeImageBuffer(IB) { \
	if((IB)->may_free) { \
		free((IB)->file); \
		free((IB)->buffer); \
		free((IB)); \
		(IB) = NULL; \
	} \
}

/* allocate and initialize a rawImageData structure */
#define AllocRawImage(IMG, W, H) do { \
	IMG = (XmHTMLRawImageData*)malloc(sizeof(XmHTMLRawImageData)); \
	memset(IMG, 0, sizeof(XmHTMLRawImageData)); \
	IMG->cmapsize = 0; \
	IMG->bg = -1; \
	IMG->width = W; \
	IMG->height = H; \
	IMG->data = (Byte*)calloc(W*H, sizeof(Byte)); \
	IMG->delayed_creation = False; \
	IMG->color_class = XmIMAGE_COLORSPACE_INDEXED; \
}while(0)

#ifdef WITH_MOTIF
/* allocate a colormap for the given rawImageData */
#define AllocRawImageCmap(IMG,SIZE) do { \
	int i; \
	IMG->cmap = (XColor*)calloc(SIZE, sizeof(XColor)); \
	for(i = 0; i < SIZE; i++) { \
		IMG->cmap[i].pixel = i; IMG->cmap[i].flags = DoRed|DoGreen|DoBlue; } \
	IMG->cmapsize = SIZE; \
}while(0)
#else
	
#define AllocRawImageCmap(IMG,SIZE) do { \
	int i; \
	IMG->cmap = (GdkColor*)calloc(SIZE, sizeof(GdkColor)); \
	for(i = 0; i < SIZE; i++) { \
		IMG->cmap[i].pixel = i; } \
	IMG->cmapsize = SIZE; \
}while(0)
#endif
	
/* allocate and initialize a rawImageData structure with a colormap */
#define AllocRawImageWithCmap(IMG, W, H, SIZE) do { \
	IMG = (XmHTMLRawImageData*)malloc(sizeof(XmHTMLRawImageData)); \
	memset(IMG, 0, sizeof(XmHTMLRawImageData)); \
	AllocRawImageCmap(IMG,SIZE); \
	IMG->bg = -1; \
	IMG->width = W; \
	IMG->height = H; \
	IMG->data = (Byte*)calloc(W*H, sizeof(Byte)); \
	IMG->delayed_creation = False; \
}while(0)

/* destroy allocated image. Only to be called upon error */
#define FreeRawImage(IMG) do{ \
	if(IMG != NULL) { \
		if(IMG->data) free(IMG->data); \
		if(IMG->cmap) free(IMG->cmap); \
		free(IMG); \
		IMG = NULL; \
	}\
}while(0)

/* reset a rawImageData structure */
#define ResetRawImage(IMG) do { \
	memset(IMG, 0, sizeof(XmHTMLRawImageData)); \
	if(IMG->cmap) free(IMG->cmap); /* erase existing colormap */ \
	IMG->cmap = (TColor*)NULL; \
	IMG->cmapsize = 0; \
	IMG->bg = -1; \
	IMG->width = 0; \
	IMG->height = 0; \
	IMG->data = (Byte*)NULL; \
	IMG->delayed_creation = False; \
}while(0)

extern Byte _XmHTMLGetImageType(ImageBuffer *ib);

extern ImageBuffer *_XmHTMLImageFileToBuffer(String file);

extern XmHTMLRawImageData *_XmHTMLReadBitmap(TWidget html, ImageBuffer *ib);

extern XmHTMLRawImageData *_XmHTMLReadGIF(TWidget html, ImageBuffer *ib);

extern XmImageInfo *_XmHTMLReadFLG(XmHTMLWidget html, ImageBuffer *ib);

extern size_t _XmHTMLGifReadOK(ImageBuffer *ib, unsigned char *buf, int len);

extern size_t _XmHTMLGifGetDataBlock(ImageBuffer *ib, unsigned char *buf);

extern int _XmHTMLIsGifAnimated(ImageBuffer *fd);

extern int _XmHTMLGifAnimInit(TWidget html, ImageBuffer *ib,
	XmHTMLRawImageData *data);

extern Boolean _XmHTMLGifAnimNextFrame(ImageBuffer *ib,
	XmHTMLRawImageData *data, int *x, int *y, int *timeout, int *dispose);

extern void _XmHTMLGifAnimTerminate(ImageBuffer *ib);

extern XmHTMLRawImageData *_XmHTMLReadXPM(TWidget html, ImageBuffer *ib);

extern XmHTMLRawImageData *_XmHTMLCreateXpmFromData(TWidget html, char **data,
	String src);

extern XmHTMLRawImageData *_XmHTMLReadPNG(TWidget html, ImageBuffer *ib);

extern XmHTMLRawImageData *_XmHTMLReReadPNG(XmHTMLWidget html,
	XmHTMLRawImageData *raw_data, int x, int y, Boolean is_body_image);

extern XmHTMLRawImageData *_XmHTMLReadJPEG(TWidget html, ImageBuffer *ib);

extern TXImage *_XmHTMLCreateXImage(XmHTMLWidget html, XCC xcc, Dimension width,
	Dimension height, String url);

extern void _XmHTMLFillXImage(XmHTMLWidget html, TXImage *ximage, XCC xcc,
	Byte *data, unsigned long *xcolors, int *start, int *end);

extern XmHTMLImage *_XmHTMLNewImage(XmHTMLWidget html, String attributes,
	Dimension *width, Dimension *height);

extern void _XmHTMLImageUpdateChilds(XmHTMLImage *image);

extern void _XmHTMLImageCheckDelayedCreation(XmHTMLWidget html);

extern void _XmHTMLMakeAnimation(XmHTMLWidget html, XmHTMLImage *image, 
	Dimension width, Dimension height);

extern TPixmap _XmHTMLInfoToPixmap(XmHTMLWidget html, XmHTMLImage *image, 
	XmImageInfo *info, Dimension width, Dimension height,
	unsigned long *global_cmap, TPixmap *clip);

extern XmImageStatus _XmHTMLReplaceOrUpdateImage(XmHTMLWidget html, 
	XmImageInfo *info, XmImageInfo *new_info, XmHTMLObjectTableElement *elePtr);

extern void _XmHTMLFreeImage(XmHTMLWidget html, XmHTMLImage *image);

extern void _XmHTMLFreeImageInfo(XmHTMLWidget html, XmImageInfo *info,
		Boolean external);

extern void _XmHTMLReleaseImage(XmHTMLWidget html, XmHTMLImage *image);

extern void _XmHTMLLoadBodyImage(XmHTMLWidget html, String url);

/*********************************************************************
* @Module: quantize.c 
* @Description : XmHTML color quantization and dithering routines
*
* @Exports:
* _XmHTMLQuantizeImage: quantize the given image data down to max_colors.
* _XmHTMLConvert24to8 : convert a 24bit image to an 8bit paletted one,
*                       quantizing if required.
* _XmHTMLPixelizeRGB  : convert an RGB image to a 8bit paletted image.
* _XmHTMLDitherImage  : dither the given image to a fixed palette.
*
**********************************************************************/
extern void _XmHTMLConvert24to8(Byte *data, XmHTMLRawImageData *img_data,
	int max_colors, Byte mode);

extern void _XmHTMLQuantizeImage(XmHTMLRawImageData *img_data, int max_colors);

extern void _XmHTMLPixelizeRGB(Byte *rgb, XmHTMLRawImageData *img_data);

extern void _XmHTMLDitherImage(XmHTMLWidget html, XmHTMLRawImageData *img_data);

/*********************************************************************
* @Module: map.c 
* @Description: XmHTML imagemap routines
*
* @Defines:
* struct _mapArea: structure identifying the shape and size of a HTML
*                  AREA definition.
*
* @Exports:
* _XmHTMLCreateImagemap  : create an imagemap.
* _XmHTMLStoreImagemap   : store an imagemap
* _XmHTMLAddAreaToMap    : add an area to an imagemap.
* _XmHTMLGetImagemap     : get the named imagemap
* _XmHTMLGetAnchorFromMap: return anchor data referenced by the given
*                          positions and imagemap.
* _XmHTMLCheckImagemaps  : check for possible external imagemaps.
* _XmHTMLFreeImageMaps   : free all imagemaps for a XmHTMLWidget.
* _XmHTMLDrawImagemapSelection: draw selection areas around each area
*                          in an imagemap.
*
**********************************************************************/
extern XmHTMLImageMap* _XmHTMLCreateImagemap(String name);

extern void _XmHTMLStoreImagemap(XmHTMLWidget html, XmHTMLImageMap *map);

extern void _XmHTMLAddAreaToMap(XmHTMLWidget html, XmHTMLImageMap *map, 
	XmHTMLObject *object);

extern XmHTMLImageMap *_XmHTMLGetImagemap(XmHTMLWidget html, String name);

extern XmHTMLAnchor *_XmHTMLGetAnchorFromMap(XmHTMLWidget html, int x, int y,
	XmHTMLImage *image, XmHTMLImageMap *map);

extern void _XmHTMLCheckImagemaps(XmHTMLWidget html);

extern void _XmHTMLFreeImageMaps(XmHTMLWidget html);

extern void _XmHTMLDrawImagemapSelection(XmHTMLWidget html, 
	XmHTMLImage *image);

/*********************************************************************
* @Module: plc.c 
* Description: XmHTML Progressive Loader Context interfacing routines.
*
* @Exports:
* _XmHTMLPLCCreate: create a PLC object suitable for progressive loading.
* _XmHTMLPLCCycler: main PLC cycler with dynamic timeout recalculation.
* _XmHTMLKillPLCCycler: kill and remove any outstanding PLC's
*
**********************************************************************/

extern PLCPtr _XmHTMLPLCCreate(XmHTMLWidget html, TPointer priv_data,
	String url, Byte type);

#ifdef WITH_MOTIF
extern void _XmHTMLPLCCycler(TPointer call_data, TIntervalId *proc_id);
#else
gint
_XmHTMLPLCCycler(gpointer call_data);
#endif

extern void _XmHTMLKillPLCCycler(XmHTMLWidget html);

/*********************************************************************
* @Module: fonts.c 
* @Description: XmHTML font loading & caching routines.
*
* @Exports:
* xmhtml_fn_sizes       : array with scalable font sizes.
* xmhtml_basefont_sizes : array with basefont sizes.
* xmhtml_fn_fixed_sizes : array with fixed font sizes.
* _XmHTMLSelectFontCache: initialize and/or select a font cache
*                         (each display has a seperate one).
* _XmHTMLloadQueryFont  : load or get a font from the font cache.
* _XmHTMLaddFontMapping : alias a known font to an unknown font.
* _XmHTMLLoadFont       : load a font as specified by id and size.
*                         Properties are inherited from a given
*                         font.
* _XmHTMLLoadFontWithFace: load a font with a named face and size.
*                         Properties are inherited from a given
*                         font.
* _XmHTMLUnloadFonts    : Release all fonts used by widget. Fonts
*                         are only unloaded when the last widget using
*                         a font cache has unloaded it's fonts.
*
**********************************************************************/

extern int xmhtml_fn_sizes[8];

extern int xmhtml_basefont_sizes[7];

extern int xmhtml_fn_fixed_sizes[2];


extern XmHTMLfont *_XmHTMLloadQueryFont(TWidget w, String name, String family,
	int ptsz, Byte style, Boolean *loaded);

extern XmHTMLfont *_XmHTMLSelectFontCache(XmHTMLWidget html, Boolean reset);

extern void _XmHTMLaddFontMapping(XmHTMLWidget html, String name,
	String family, int ptsz, Byte style, XmHTMLfont *font);

extern XmHTMLfont *_XmHTMLLoadFont(XmHTMLWidget html, htmlEnum font_id,
	int size, XmHTMLfont *curr_font);

extern XmHTMLfont *_XmHTMLLoadFontWithFace(XmHTMLWidget html, int size,
	String face, XmHTMLfont *curr_font);

extern void _XmHTMLUnloadFonts(XmHTMLWidget html);

/*********************************************************************
* @Module: events.c 
* @Description: HTML4.0 event routines
*
* @Exports:
* _XmHTMLCheckCoreEvents: check for the HTML 4.0 core events.
* _XmHTMLCheckBodyEvents: check for the HTML 4.0 body events as well
*                         as the core events.
* _XmHTMLCheckFormEvents: check for the HTML 4.0 <FORM> events as well
*                         as the core events.
* _XmHTMLProcessEvent   : XmNeventCallback driver.
* _XmHTMLFreeEventDatabase: release all storage allocated for the
*                         HTML 4.0 event handling.
*
**********************************************************************/

extern AllEvents *_XmHTMLCheckCoreEvents(XmHTMLWidget html, String attributes);

extern AllEvents *_XmHTMLCheckBodyEvents(XmHTMLWidget html, String attributes);

extern AllEvents *_XmHTMLCheckFormEvents(XmHTMLWidget html, String attributes);

extern void _XmHTMLProcessEvent(XmHTMLWidget html, TEvent *event,
	HTEvent *ht_event);

extern void _XmHTMLFreeEventDatabase(XmHTMLWidget old, XmHTMLWidget html);

#endif /* XmHTML_ERROR_FUNCS */

/*********************************************************************
* @Module: error.c 
* @Description: XmHTML warning/error functions
*
* @Exports:
* _XmHTMLWarning   : Displays a warning message and continues.
* _XmHTMLError     : Displays an error message and exits.
* __XmHTMLBadParent: Display a NULL/invalid parent warning message and
*                    continue.
* _XmHTMLAllocError: Displays an error message due to allocation
*                    problems and exits.
*
* @Note: There are two separate versions of XmHTML's error & warning
* functions. The debug versions include full location information while
* the normal build versions only contain the warning/error message.
* This allows us to reduce the data size of the normal build.
*
**********************************************************************/

#ifdef DEBUG

#define __WFUNC__(WIDGET_ID, FUNC)	(TWidget)WIDGET_ID, __FILE__, \
	 __LINE__, FUNC

extern void __XmHTMLWarning(
	TWidget w, String module, int line, String routine,
	String fmt, ...
);

extern void __XmHTMLError(
	TWidget w, String module, int line, String routine, 
	String fmt, ...
);

extern void __XmHTMLBadParent(TWidget w, String src_file, int line, String func);

#define _XmHTMLBadParent(W,FUNC)	__XmHTMLBadParent(W,__FILE__,__LINE__,FUNC)

#else	/* !DEBUG */

#define __WFUNC__(WIDGET_ID, FUNC)	(TWidget)WIDGET_ID

extern void __XmHTMLWarning(
	TWidget w, String fmt, ...
);

extern void __XmHTMLError(
	TWidget w, String fmt, ...
);

extern void __XmHTMLBadParent(TWidget w, String func);

#define _XmHTMLBadParent(W,FUNC)	__XmHTMLBadParent(W,FUNC)

#endif /* DEBUG */

#define _XmHTMLWarning __XmHTMLWarning
#define _XmHTMLError   __XmHTMLError

extern void _XmHTMLAllocError(TWidget w, char *module, char *routine, 
	char *func, int size);

#include "gtk-xmhtml.h"
#include "gtk-xmhtml-p.h"

void gtk_xmhtml_set_outline (GtkXmHTML *html, int flag);
void my_x_query_colors(GdkColormap *colormap,
		       GdkColor    *colors,
		       gint         ncolors);
void
gtk_xmhtml_set_colors (GtkXmHTML *html,
		       Pixel foreground,
		       Pixel background,
		       Pixel anchor_fg,
		       Pixel anchor_target_fg,
		       Pixel anchor_visited_fg,
		       Pixel anchor_activated_fg,
		       Pixel anchor_activated_bg);

void _XmHTMLRecomputeShadowColors(XmHTMLWidget html, Pixel base);

/* Don't add anything after this endif! */
#endif /* _XmHTMLI_h_ */
