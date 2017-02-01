/*****
* format.c : XmHTML formatting routines: translates parsed HTML to 	info 
*			required for displaying a HTML page.
*
* This file Version	$Revision: 1.13.6.2 $
*
* Creation date:		Tue Nov 26 17:03:09 GMT+0100 1996
* Last modification: 	$Date: 2002/01/08 22:33:11 $
* By:					$Author: kmaraas $
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
* ChangeLog 
* $Log: format.c,v $
* Revision 1.13.6.2  2002/01/08 22:33:11  kmaraas
* 2002-01-06  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* *: Fix compiler warnings.
* 	* images.c: Fix missing X color context ref that was causing lots
* 	of crashes. Fixes #60237, #61638, #63439, #65040, #66913 and more.
* 	* test.c: do not use %s for a boolean use %d instead.
*
* Revision 1.13.6.1  2001/10/20 06:52:12  kmaraas
* 2001-10-20  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* *.*: Apply all the Red Hat patches.
*
* Revision 1.13  1999/07/29 01:26:28  sopwith
*
*
* Fix all warnings.
*
* Revision 1.12  1999/06/02 01:00:39  unammx
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
* Revision 1.11  1998/06/23 18:45:55  unammx
* James Henstridge's signal fixes to GtkXmHTML
*
* Revision 1.10  1998/02/12 03:08:39  unammx
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
* Revision 1.9  1998/01/14 05:49:51  unammx
* Tue Jan 13 22:04:43 1998  Federico Mena  <federico@bananoid.nuclecu.unam.mx>
*
* 	* gtk-xmhtml.c (gtk_xmhtml_new): The widget starts up frozen and
*  	thaws itself when it is realized.  This fixes all of the problems
*  	regarding realization, gc creation, and window background setting.
*
* (Federico and Miguel)
*
* Revision 1.8  1998/01/14 04:11:52  unammx
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
* Revision 1.7  1998/01/09 06:10:23  unammx
* Fixed (?) background colors of the HTML widget.  I'm not 100% sure I did it
* the right way, but it seems to work.
*
* - Federico
*
* Revision 1.6  1997/12/29 22:16:25  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.5  1997/12/27 20:58:17  unammx
* More access functions to the widget internals.  I missed these
* yesterday (ie, those that did not require SetValues validation
* now have an explicit routine to change the values).
*
* Frame support depends on the client of the widget, we should catch
* that signal and do something with it, I have not figured out exacly
* how it works, but example_2 in the XmHTML-1.1.1 distribution has an
* example of this working.
*
* Miguel.
*
* Revision 1.4  1997/12/25 01:34:11  unammx
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
* Revision 1.3  1997/12/13 01:49:15  unammx
* your daily dose of ported XmHTML code, non functional as usual -mig
*
* Revision 1.2  1997/12/11 21:20:21  unammx
* Step 2: more gtk/xmhtml code, still non-working - mig
*
* Revision 1.1  1997/11/28 03:38:56  gnomecvs
* Work in progress port of XmHTML;  No, it does not compile, don't even try -mig
*
* Revision 1.17  1997/10/23 00:24:56  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.16  1997/08/31 17:34:24  newt
* renamed _rec structures to Rec.
*
* Revision 1.15  1997/08/30 00:55:24  newt
* Completed <form></form> support.
* Bugfix in _XmHTMLInitializeFontSizeLists, the default font is now properly
* changed.
* Made the font loading routines again robuster.
* ParseBodyTags now always attempts to load a body image.
* Made XmHTMLGetURLType a bit stricter.
*
* Revision 1.14  1997/08/01 13:00:21  newt
* Bugfixes in font switching (<b>...<font><i>...</i></font>...</b>) now
* properly handler. Enhanced form support.
*
* Revision 1.13  1997/05/28 01:46:35  newt
* Added support for the XmNbodyImage resource: it's now used but only if no
* bgcolor resource has been set.
*
* Revision 1.12  1997/04/29 14:26:18  newt
* HTML forms changes
*
* Revision 1.11  1997/04/03 05:34:25  newt
* _XmHTMLLoadBodyImage added.
* Placed a large number of warnings between a #ifdef PEDANTIC/#endif
*
* Revision 1.10  1997/03/28 07:12:43  newt
* Fixed buffer overrun in TexToPre. 
* Fixed font resolution: x and y resolution are now always equal. 
* XmHTML now ignores the ending body tag.
*
* Revision 1.9  1997/03/20 08:10:04  newt
* Split font cache in a true cache and a font stack.
* Added stack checks when document has been formatted.
*
* Revision 1.8  1997/03/11 19:52:17  newt
* added ImageToWord
*
* Revision 1.7  1997/03/04 00:59:26  newt
* ?
*
* Revision 1.6  1997/03/02 23:17:46  newt
* Way too many changes. Most important: font loading/switching scheme; anchor 
* treatment; image/imagemap treatment
*
* Revision 1.5  1997/02/11 02:08:44  newt
* Way to many. Anchor treatment has been changed completely. 
* Bugfixes in anchor parsing. Potential buffer overruns eliminated.
*
* Revision 1.4  1997/02/04 02:56:49  newt
* Bugfix in LoadQueryFont. 
* Added code to deal with the basefont element. 
* Changed the font element handling.
*
* Revision 1.3  1997/01/09 06:55:39  newt
* expanded copyright marker
*
* Revision 1.2  1997/01/09 06:44:42  newt
* lots of changes: linebreaking and changes related to changed XmHTMLWord
*
* Revision 1.1  1996/12/19 02:17:10  newt
* Initial Revision
*
*****/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>	/* isspace, tolower */

#include <gdk/gdki18n.h>
#if !defined(G_HAVE_BROKEN_WCTYPE) && (defined(G_HAVE_WCTYPE_H) || defined(G_HAVE_WCHAR_H)) && !defined(X_LOCALE)
#define is_ideograph(wc) \
	(iswalpha (wc) && (!iswupper (wc) && !iswlower (wc) )) || \
	(!(iswalnum(wc) || iswspace(wc) || iswpunct(wc) || iswcntrl(wc)))
#else
#define is_ideograph(wc) FALSE
#endif

/* Local includes */
#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/
#define MAX_NESTED_LISTS	26	/* maximum no of nested lists */
#define IDENT_SPACES		3	/* length of each indent, in no of spaces */

typedef struct{
	String name;
	Marker type;	/* Marker is an enumeration type defined in XmHTMLP.h */
}listMarkers;

/* for nesting of ordered and unordered lists */
typedef struct{
	Boolean isindex;	/* propagate index numbers? */
	int level;			/* item number */
	htmlEnum type;		/* ol or ul, used for custom markers */
	Marker marker;		/* marker to use */
}listStack;

/* for nesting of colors */
typedef struct colorRec{
	unsigned long color;
	struct colorRec *next;
}colorStack;

typedef struct fontRec{
	int size;
	XmHTMLfont *font;	/* ptr to cached font */
	struct fontRec *next;
}fontStack;

/* for nesting of alignments */
typedef struct alignRec{
	Alignment align;
	struct alignRec *next;
}alignStack;

/*** Private Function Prototype Declarations ****/

/****
* Formatting routines 
*****/
/* Created a new element for the formatted element table */
static XmHTMLObjectTableElement NewTableElement(XmHTMLObject *data);

/* Insert and element into the formatted element table */
static void InsertTableElement(XmHTMLWidget html, 
	XmHTMLObjectTableElement element, Boolean is_anchor);

/* Release the formatted element table */
static void FreeObjectTable(XmHTMLObjectTable *list);

/* Free the given list of tables */
static void freeTables(XmHTMLTable *table);

/* Initialize the formatted element table */
static void InitObjectTable(XmHTMLObjectTable *list, XmHTMLAnchor *anchors);

/* push/pop a font on font stack */
static void PushFont(XmHTMLfont *font, int size);
static XmHTMLfont *PopFont(int *size);

/* copy given text into an internal buffer */
static String CopyText(XmHTMLWidget html, String text, Boolean formatted,
	int *text_data, Boolean expand_escapes);

/* collapse all consecutive whitespace into a single space */
static void CollapseWhiteSpace(String text);

/* Split raw text into an array of words */
static XmHTMLWord* TextToWords(String text, int *num_words, Dimension *height, 
	XmHTMLfont *font, Byte line_data, int text_data, 
	XmHTMLObjectTableElement owner);

/* Split an image into an array of words ;-) */
static XmHTMLWord *ImageToWord(XmHTMLWidget html, String attributes, 
	int *num_words, Dimension *height, XmHTMLObjectTableElement owner,
	Boolean formatted);

static XmHTMLWord *allocFormWord(XmHTMLForm *form, Dimension *width,
	Dimension *height, XmHTMLObjectTableElement owner, Boolean formatted);

static XmHTMLWord *InputToWord(XmHTMLWidget html, String attributes, 
	int *num_words, Dimension *width, Dimension *height,
	XmHTMLObjectTableElement owner, Boolean formatted);

static XmHTMLWord *SelectToWord(XmHTMLWidget html, XmHTMLObject *start,
	int *num_words, Dimension *width, Dimension *height,
	XmHTMLObjectTableElement owner, Boolean formatted);

static XmHTMLWord *TextAreaToWord(XmHTMLWidget html, XmHTMLObject *start,
	int *num_words, Dimension *width, Dimension *height,
	XmHTMLObjectTableElement owner, Boolean formatted);

static XmHTMLWord *BreakToWord(Dimension *height, XmHTMLfont *font,
	int linefeed, XmHTMLObjectTableElement owner);

static XmHTMLWord *MakeDummyWord(Dimension *height, XmHTMLfont *font,
	Byte line_data, XmHTMLObjectTableElement owner);

/* Split raw text into a chunk of preformatted lines */
static XmHTMLWord *TextToPre(String text, int *num_words, XmHTMLfont *font, 
	Byte line_data, XmHTMLObjectTableElement owner);
 
/* Insert a horizontal tab */
static XmHTMLWord* SetTab(int size, Dimension *height, XmHTMLfont *font, 
	XmHTMLObjectTableElement owner);  

/* Initialize a bullet (a real bullet or some number) */
static void FillBullet(XmHTMLWidget html, XmHTMLObjectTableElement owner);

/* get properties of a table element */
static TableProperties *tableCheckProperties(XmHTMLWidget html,
	String attributes, TableProperties *parent, Alignment halign, Pixel bg);

/* open a new table */
static XmHTMLTable *tableOpen(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj, Alignment *halign,
	Pixel *bg);

/* close the current table */
static XmHTMLTable *tableClose(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end);

/* open a caption in the current table */
static void tableOpenCaption(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj,
	Pixel *bg);

/* close the current caption */
static void tableCloseCaption(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end);

/* open a row in the current table */
static void tableOpenRow(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj, Alignment *halign,
	Pixel *bg);

/* close the current row */
static void tableCloseRow(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end);

/* open a cell in the current row */
static void tableOpenCell(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj, Alignment *halign,
	Pixel *bg);

/* close the current cell in the current row */
static void tableCloseCell(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end);

/* Parse body tags and update the TWidget */
static void ParseBodyTags(XmHTMLWidget html, XmHTMLObject *data);

/* Check whether a linefeed is required or not */
static int CheckLineFeed(int this, Boolean force);

/* push/pop a color on the color stack */
static void PushFGColor(Pixel color);
static void PushBGColor(Pixel color);
static Pixel PopFGColor(void);
static Pixel PopBGColor(void);

/* push/pop an alignment on/from the alignment stack */
static void PushAlignment(Alignment align);
static Alignment PopAlignment(void);

/* split the given anchor spec into a href, target and other stuff */
static void parseHref(String text, XmHTMLAnchor *anchor); 

/*** Private Variable Declarations ***/
/* Element data bits */
#define ELE_ANCHOR				(1<<1)
#define ELE_ANCHOR_TARGET		(1<<2)
#define ELE_ANCHOR_VISITED		(1<<3)
#define ELE_ANCHOR_INTERN		(1<<4)
#define ELE_UNDERLINE			(1<<5)
#define ELE_UNDERLINE_TEXT		(1<<6)
#define ELE_STRIKEOUT			(1<<7)
#define ELE_STRIKEOUT_TEXT		(1<<8)

/* Private formatted element table data */
static struct{
	unsigned long num_elements;
	unsigned long num_anchors;
	XmHTMLObjectTableElement head;
	XmHTMLObjectTableElement current;
	XmHTMLAnchor *anchor_head;
	XmHTMLAnchor *anchor_current;
}list_data;

/* color and alignment stacks */
static colorStack fg_color_base, *fg_color_stack;
static colorStack bg_color_base, *bg_color_stack;
static alignStack align_base, *align_stack;
static fontStack font_base, *font_stack;

/* Marker information for HTML lists, ordered list. */
#define OL_ARRAYSIZE	5
static listMarkers ol_markers[OL_ARRAYSIZE] = {
	{"1", XmMARKER_ARABIC},
	{"a", XmMARKER_ALPHA_LOWER},
	{"A", XmMARKER_ALPHA_UPPER},
	{"i", XmMARKER_ROMAN_LOWER},
	{"I", XmMARKER_ROMAN_UPPER},
};

/* Unordered list. */
#define UL_ARRAYSIZE	3
static listMarkers ul_markers[UL_ARRAYSIZE] = {
	{"disc", XmMARKER_DISC},
	{"square", XmMARKER_SQUARE},
	{"circle", XmMARKER_CIRCLE},
};

#ifdef DEBUG
static int allocated;
#endif

/*****
* Name: 		NewTableElement
* Return Type: 	XmHTMLObjectTableElement
* Description: 	creates a ObjectTableElement and fills it.
* In: 
*	data:		raw data for this element.
* Returns:
*	the newly created element.
*****/
static XmHTMLObjectTableElement
NewTableElement(XmHTMLObject *data)
{
	static XmHTMLObjectTableElement element = NULL;

	element = (XmHTMLObjectTableElement)malloc(sizeof(XmHTMLObjectTable));

	/* initialise to zero */
	(void)memset(element, 0, sizeof(XmHTMLObjectTable));
	/* fill in appropriate fields */
	element->object = data;

#ifdef DEBUG
	allocated++;
#endif
	return(element);
}

/*****
* Name: 		InsertTableElement
* Return Type: 	void
* Description: 	inserts a given formatted element in the list of elements.
* In: 
*	w:			XmHTMLWidget to which this element belongs.
*	element:	element to add
*	is_anchor:	true if this element is an anchor.
* Returns:
*	nothing.
*****/
static void
InsertTableElement(XmHTMLWidget html, XmHTMLObjectTableElement element, 
	Boolean is_anchor) 
{
	/* attach prev and next ptrs to the appropriate places */
	element->prev = list_data.current;
	list_data.current->next = element;
	list_data.current = element;
	/* increment element counter */
	list_data.num_elements++;
#ifdef DEBUG
	if(is_anchor)
		list_data.num_anchors++;
#endif
}

/*****
* Name: 		parseHref
* Return Type: 	void
* Description: 	returns the url specification found in the given anchor.
* In: 
*	text:		full anchor spec.
*	href:		url found in given anchor. Filled upon return.
*	target:		any target attribute found. Filled upon return.
*	extra:		any additional attributes found. Filled upon return.
* Returns:
*	nothing.
*****/
static void
parseHref(String text, XmHTMLAnchor *anchor) 
{
	if(text == NULL ||
		(anchor->href = _XmHTMLTagGetValue(text, "href")) == NULL)
	{
		/* allocate empty href field so later strcmps won't explode */
		anchor->href = (char *)malloc(1);
		anchor->href[0] = '\0'; /* fix 02/03/97-05, kdh */
		/*
		* Could be a named anchor with a target spec. Rather impossible but
		* allow for it anyway (I can imagine this to be true for a
		* split-screen display).
		*/
		if(text == NULL)
			return;
	}

	/* Check if there is a target specification */
	anchor->target= _XmHTMLTagGetValue(text, "target");

	/* Also check for rel, rev and title */
	anchor->rel = _XmHTMLTagGetValue(text, "rel");
	anchor->rev = _XmHTMLTagGetValue(text, "rev");
	anchor->title  = _XmHTMLTagGetValue(text, "title");
}

/*****
* Name: 		FreeObjectTable
* Return Type: 	void
* Description: 	releases all memory occupied by the formatted list of elements.
* In: 
*	list:		previous list to be released.
* Returns:
*	nothing.
* Note:
*	Images are freed in XmHTML.c, which calls XmHTMLFreeAllImages to do the
*	job.
*****/
static void 
FreeObjectTable(XmHTMLObjectTable *list)
{
	XmHTMLObjectTableElement temp;

#ifdef DEBUG
	int i = 0, j = 0;
#endif

	/* free all parsed objects */
	while(list != NULL)
	{
		temp = list->next;
		if(list->text)	/* space occupied by text to display */
			free(list->text);

		/* free list of words. Can't be done above, <pre> doesn't have this! */
		if(list->n_words)
		{
			/* 
			* only the first word contains a valid ptr, all others point to
			* some char in this buffer, so freeing them *will* cause a
			* segmentation fault eventually.
			*/
			free(list->words[0].word);
			/* Free raw word data */
			free(list->words);
		}
		free(list);
		list = temp;
#ifdef DEBUG
		i++;
#endif
	}
	_XmHTMLDebug(2, ("format.c: FreeObjectTable End, freed %i elements and "
		"%i anchors.\n", i, j));
}

/*****
* Name:			freeTables
* Return Type: 	void
* Description: 	frees all data allocated for HTML table support.
* In: 
*	table:		list of tables to be freed.
* Returns:
*	nothing.
*****/
static void
freeTables(XmHTMLTable *table)
{
	XmHTMLTable *tab, *tmp = table;
	TableRow *row;
	int i, j, k;

	while(table)
	{
		tmp = table->next;

		/*****
		* Free all child tables (first table in the childs array is the
		* table itself)
		*****/
		for(i = 0; i < table->nchilds; i++)
		{
			tab = &table->childs[i];

			/* free all rows */
			for(j = 0; j < tab->nrows; j++)
			{
				row = &tab->rows[j];
				/* free all cells in this row */
				for(k = 0; k < row->ncells; k++)
				{
					free(row->cells[k].properties);
				}
				free(row->cells);
				free(row->properties);
			}
			free(tab->rows);
			free(tab->properties);
		}
		free(table->childs);
		free(table);
		table = tmp;
	}
}

/*****
* Name: 		FreeAnchors
* Return Type: 	void
* Description: 	frees the memory occupied by the anchor data
* In: 
*	anchors:	list of anchors to be freed
* Returns:
*	nothing.
*****/
static void
FreeAnchors(XmHTMLAnchor *anchors)
{
	XmHTMLAnchor *tmp;
	int i = 0;

	while(anchors)
	{
		tmp = anchors->next;
		/* href field is always allocated */
		free(anchors->href);
		if(anchors->target)
			free(anchors->target);
		if(anchors->rel)
			free(anchors->rel);
		if(anchors->rev)
			free(anchors->rev);
		if(anchors->title)
			free(anchors->title);
		if(anchors->name)		/* fix 07/09/97-01, kdh */
			free(anchors->name);
		if(anchors->events)
			free(anchors->events);
		free(anchors);
		anchors = NULL;
		anchors = tmp;
		i++;
	}
	_XmHTMLDebug(2, ("format.c: FreeAnchors, freed %i XmHTMLAnchor objects\n", 
		i));
}

/*****
* Name: 		InitObjectTable
* Return Type: 	void
* Description: 	initializes the list of formatted elements.
* In: 
*	list:		previous list to be released.
* Returns:
*	nothing
* Note:
*	The list head is a dummy element and is never used. It is done to gain
*	some performance (a test on an empty head is not required now in the
*	InsertTableElement routine).
*****/
static void
InitObjectTable(XmHTMLObjectTable *list, XmHTMLAnchor *anchors)
{
	if(list != NULL)
	{
		FreeObjectTable(list);
		list = NULL;
	}

	if(anchors != NULL)
	{
		FreeAnchors(anchors);
		anchors = NULL;
	}
	if(list_data.head)
		free(list_data.head);
	list_data.head = NewTableElement(NULL);
	list_data.current = list_data.head;
	list_data.anchor_head = (XmHTMLAnchor*)NULL;
	list_data.anchor_current = (XmHTMLAnchor*)NULL;
	list_data.num_elements = 1;
	list_data.num_anchors  = 0;
}

/*****
* Name: 		CollapseWhiteSpace
* Return Type: 	void
* Description: 	collapses whitespace in the given text
* In: 
*	text:		text for which multiple whitespace has to be collapsed.
* Returns:
*	nothing, but text is updated when this function returns.
*****/
static void
CollapseWhiteSpace(String text)
{
	register char *outPtr = text;

	/* 
	* We only collapse valid text and text that contains more than whitespace
	* only. This should never be true since CopyText will filter these
	* things out. It's just here for sanity.
	*/
	if(*text == '\0' || !strlen(text))
		return;

	_XmHTMLDebug(2, ("format.c: CollapseWhiteSpace, text in is:\n%s\n", text));

	/*
	* Now collapse each occurance of multiple whitespaces.
	* This may produce different results on different systems since
	* isspace() might not produce the same on each and every platform.
	*/
	while(True)
	{
		switch(*text)
		{
			case '\f':
			case '\n':
			case '\r':
			case '\t':
			case '\v':
				*text = ' ';	/* replace by a single space */
				/* fall through */
			case ' ':
				/* skip past first space */
				*(outPtr++) = *(text++);	
				/* collapse every space following */
				while(*text != '\0' && isspace(*text))
					*text++ = '\0';
				break;
			default:
				*(outPtr++) = *(text++);
				break;
		}
		if(*text == 0)
		{
			*outPtr = '\0';
			return;
		}
	}
}

/*****
* Name: 		TextToWords
* Return Type: 	XmHTMLWord*
* Description: 	splits the given text into an array of words.
* In: 
*	text:		text to split
*	num_words:	number of words in the given text. Filled upon return;
*	font:		font to use for this text.
* Returns:
*	an array of words. When allocation fails, this routine exits.
*****/
static XmHTMLWord* 
TextToWords(String text, int *num_words, Dimension *height, XmHTMLfont *font, 
	Byte line_data, int text_data, XmHTMLObjectTableElement owner)
{
	int n_words, n_words_alloc, raw_numwc, i;
	GdkWChar *raw, *start;
	XmHTMLWord *words;
	register int j;
	register GdkWChar *chPtr;
	int leading_space;
	char *wordsbuf;
	int size_wordsbuf;

	/* sanity check */
	if(text == NULL)
	{
		*height = *num_words = 0;
		return(NULL);
	}

	_XmHTMLFullDebug(2, ("format.c: TextToWords, text in is:\n%s\n", text));

	/* copy text */
	raw = (GdkWChar *)calloc(strlen(text) + 1, sizeof(GdkWChar));
	raw_numwc = gdk_mbstowcs(raw, text, strlen(text));
	if (raw_numwc < 1)
		raw_numwc = 0;

	/* allocate memory for all words */
	n_words = 0;
	n_words_alloc = 10;
	words = (XmHTMLWord*)calloc(n_words_alloc, sizeof(XmHTMLWord));

	/* Split the text in words and fill in the appropriate fields */
	*height = font->height;
	chPtr = start = raw;

	for(i = 0, j = 0, leading_space = 0; ; chPtr++, j++)
	{
		/* expand words[] if needed */
		if (n_words_alloc < i + 1)
		{
			n_words_alloc += 10;
			words = (XmHTMLWord *)realloc(
				words, n_words_alloc * sizeof(XmHTMLWord));
		}

		/* also pick up the last word! */
		if(*chPtr == ' ' || *chPtr == '\0' ||
		   (chPtr > start &&
		    (is_ideograph(*chPtr) || is_ideograph(chPtr[-1]))))
		{
			GdkWChar saved_char;
			int trailing_space = 0;
			if(*chPtr == ' ')
			{
				chPtr++;			/* nuke the space */
				raw[j++] = '\0';
				trailing_space = 1;
			}
			/* set the first char of the next word 0 temporally.
			 * gdk_wcstombs requires this. */
			saved_char = raw[j];
			raw[j] = '\0';
			/* fill in required fields */
			words[i].self      = NULL;	/* set later */
			words[i].word      = gdk_wcstombs(start);
			words[i].len       = words[i].word ? strlen(words[i].word) : 0;
			words[i].height    = *height;
			words[i].width     = Toolkit_Text_Width(
				font->xfont, words[i].word, strlen(words[i].word));
			words[i].owner     = owner;
			words[i].font      = font;
			words[i].spacing   = 0;		/* set later */
			words[i].type      = OBJ_TEXT;
			words[i].line_data = line_data;

			if (leading_space)
				words[i].spacing |= TEXT_SPACE_LEAD;
			else
				words[i].spacing |= TEXT_SPACE_LEAD_ZEROWIDTH;

			if (trailing_space)
				words[i].spacing |= TEXT_SPACE_TRAIL;
			else
				words[i].spacing |= TEXT_SPACE_TRAIL_ZEROWIDTH;

			leading_space = trailing_space; /* for the next word */

			raw[j] = saved_char;

			_XmHTMLFullDebug(2, ("format.c: TextToWords, word is %s, len is "
				"%i, width is %i, height is %i\n", words[i].word, words[i].len,
				words[i].width, words[i].height));

			start = chPtr;
			i++;
		}
		if(*chPtr == '\0')
			break;
	}
	free(raw);
	n_words = i;

	/* because the pointer 'words' may be realloc()ed, we need to set the
	* 'self' field here.
	*/
	for (i = 0; i < n_words; i++)
		words[i].self = &words[i];

	/* words[0].word must be a malloc()ed pointer, and word[i].word
	* (for i > 0) a reference (i.e., it should not be freed).
	*/
	for (size_wordsbuf = 0, i = 0; i < n_words; i++)
		size_wordsbuf += words[i].len + 1;
	wordsbuf = (char *)malloc(size_wordsbuf);
	for (i = j = 0; i < n_words; i++)
	{
		/* copies trailing nil also */
		memcpy(wordsbuf + j, words[i].word, words[i].len + 1);
		free(words[i].word);
		words[i].word = wordsbuf + j;
		j += words[i].len + 1;
	}

	/* 
	* when there is more than one word in this block, the first word
	* _always_ has a trailing space.
	* Likewise, the last word always has a leading space.
	*/
	if(n_words > 1)
	{
		words[0].spacing &= ~TEXT_SPACE_LEAD_ZEROWIDTH;
		words[n_words-1].spacing &= ~TEXT_SPACE_TRAIL_ZEROWIDTH;

		words[0].spacing |= text_data &
			(TEXT_SPACE_LEAD | TEXT_SPACE_LEAD_ZEROWIDTH);
		words[n_words-1].spacing |= text_data &
			(TEXT_SPACE_TRAIL | TEXT_SPACE_TRAIL_ZEROWIDTH);
	}
	else
		words[0].spacing = text_data;

	_XmHTMLFullDebug(2, ("format.c: TextToWords counted %i words\n", n_words));

	*num_words = i; /* n_words */;
	return(words);	
}

/*****
* Name: 		ImageToWord
* Return Type: 	XmHTMLWord*
* Description: 	converts an image to a word
* In: 
*	w:			XmHTMLWidget id
*	attributes:	raw <img> specification
*	height:		object height, updated upon return
*	owner:		owning object
*	formatted:	True when this image is part of a block of <pre></pre> text.
* Returns:
*	a word representing the image
*****/
static XmHTMLWord*
ImageToWord(XmHTMLWidget html, String attributes, int *num_words, 
	Dimension *height, XmHTMLObjectTableElement owner, Boolean formatted)
{
	static XmHTMLWord *word;
	static XmHTMLImage *image;
	Dimension width = 0;

	*num_words = 0;

	/* sanity check */
	if(attributes == NULL || 
		(image = _XmHTMLNewImage(html, attributes, &width, height)) == NULL)
	{
		*height = 0;
		return(NULL);
	}

	_XmHTMLFullDebug(2, ("format.c: ImageToWord, image in is: %s\n",
		image->url));

	word = (XmHTMLWord*)calloc(1, sizeof(XmHTMLWord));

	/* required for image anchoring/replace/update */
	image->owner = owner;

	/* fill in required fields */
	word->self   = word;
	word->word   = strdup(image->alt);	/* we always have this */
	word->len    = strlen(image->alt);
	word->width  = width + 2*image->hspace + 2*image->border;
	word->height = *height + 2*image->vspace + 2*image->border;
	word->owner  = owner;
	word->font   = font_base.font;		/* always use the default font */
	/*****
	* if image support is disabled, add width of the alt text to the
	* image width (either from default image or specified in the doc).
	* This is required for proper exposure handling when images are disabled.
	*****/
	if(!html->html.images_enabled)
		word->width += Toolkit_Text_Width(word->font->xfont, word->word, word->len);

	/*****
	* No spacing if part of a chunk of <pre></pre> text
	* Fix 07/24/97, kdh
	*****/
	word->spacing = formatted ? 0 : TEXT_SPACE_LEAD | TEXT_SPACE_TRAIL;
	word->type = OBJ_IMG;
	word->line_data = NO_LINE;	/* no underlining for images */
	word->image = image;

	_XmHTMLFullDebug(2, ("format.c: TextToWords, word is %s, len is %i, "
		"width is %i, height is %i\n", word->word, word->len,
		word->width, word->height));

	*num_words = 1;
	return(word);
}

/*****
* Name:			allocFormWord
* Return Type: 	XmHTMLWord*
* Description: 	allocates a default XmHTMLWord for use within a HTML form.
* In: 
*	form:		form entry for which this word should be allocated;
*	*width:		object's width, updated upon return;
*	*height:	object's height, updated upon return;
*	owner:		owning object.
*	formatted:	true when allocating a form component present in <pre></pre>
* Returns:
*	a newly allocated word.
*****/
static XmHTMLWord*
allocFormWord(XmHTMLForm *form, Dimension *width, Dimension *height,
	XmHTMLObjectTableElement owner, Boolean formatted)
{
	static XmHTMLWord *word;

	/* allocate new entry */
	word = (XmHTMLWord*)calloc(1, sizeof(XmHTMLWord));

	/* fill in required fields */
	word->self    = word;
	word->word    = strdup(form->name); 	/* we always have this */
	word->len     = strlen(form->name);
	word->height  = *height = form->height;
	word->width   = *width  = form->width;
	word->owner   = owner;
	word->font    = font_base.font; 		/* always use default font */
	word->spacing = formatted ? 0 : TEXT_SPACE_LEAD | TEXT_SPACE_TRAIL;
	word->type    = OBJ_FORM;
	word->form    = form;

	return(word);
}

/*****
* Name: 		InputToWord
* Return Type: 	XmHTMLWord*
* Description: 	converts a HTML form <input> element to a word
* In: 
*	w:			XmHTMLWidget id
*	attributes:	raw form element specification
*	width:		object width, updated upon return
*	height:		object height, updated upon return
*	owner:		owning object
*	formatted:	true when this form component is placed in a <pre></pre> tag.
* Returns:
*	a word representing the image
*****/
static XmHTMLWord*
InputToWord(XmHTMLWidget html, String attributes, int *num_words, 
	Dimension *width, Dimension *height, XmHTMLObjectTableElement owner,
	Boolean formatted)
{
	static XmHTMLForm *form_entry;
	XmHTMLWord *word;

	*num_words = 0;

	/* sanity check */
	if(attributes == NULL ||
		(form_entry = _XmHTMLFormAddInput(html, attributes)) == NULL)
		return(NULL);

	/* save owner, we need it in the paint routines */
	form_entry->data = owner;

	/* image buttons are treated as anchored images */
	if(form_entry->type == FORM_IMAGE)
	{
		word = ImageToWord(html, attributes, num_words, height, owner,
				formatted);
		/* remove alt text */
		free(word->word);
		/* use form member name instead */
		word->word = strdup(form_entry->name);
		word->len  = strlen(form_entry->name);
		word->form = form_entry;

		_XmHTMLFullDebug(2, ("format.c: InputToWord, word is %s, len is %i, "
			"width is %i, height is %i (type = image)\n", word->word,
			word->len, word->width, word->height));

		return(word);
	}

	/* allocate new word for this form member */
	word = allocFormWord(form_entry, width, height, owner, formatted);

	_XmHTMLFullDebug(2, ("format.c: InputToWord, word is %s, len is %i, "
		"width is %i, height is %i\n", word->word, word->len,
		word->width, word->height));

	*num_words = 1;
	return(word);
}

/*****
* Name:			SelectToWord
* Return Type: 	XmHTMLWord*
* Description:	converts a HTML form <select></select> to a HTMLWord.
*				Also processes any <option></option> items within this select.
* In: 
*	html:		XmHTMLWidget id;
*	start:		object at which <select> starts;
*	*num_words:	no of words allocated. Updated upon return;
*	*width:		width of returned object. Updated upon return;
*	*height:	height of returned object. Updated upon return;
*	owner:		owning element.
*	formatted:	true when this form component is placed in a <pre></pre> tag.
* Returns:
*	a newly allocated word upon success. NULL on failure.
*****/
static XmHTMLWord*
SelectToWord(XmHTMLWidget html, XmHTMLObject *start, int *num_words, 
	Dimension *width, Dimension *height, XmHTMLObjectTableElement owner,
	Boolean formatted)
{
	static XmHTMLForm *form_entry;
	XmHTMLWord *word;
	XmHTMLObject *tmp = start;

	*num_words = 0;

	/* sanity check */
	if(start->attributes == NULL ||
		(form_entry = _XmHTMLFormAddSelect(html, start->attributes)) == NULL)
		return(NULL);

	/* save owner */
	form_entry->data = owner;

	/* move to next element */
	tmp = tmp->next;

	/* add all option tags */
	for(; tmp != NULL && tmp->id != HT_SELECT; tmp = tmp->next)
	{
		if(tmp->id == HT_OPTION && !tmp->is_end)
		{
			XmHTMLObject *sel_start = tmp;
			int foo;
			String text = NULL;

			/*
			* The next object should be plain text, if not it's an
			* error and we should ignore it
			*/
			tmp = tmp->next;
			if(tmp->id != HT_ZTEXT)
			{
				if(html->html.bad_html_warnings)
				{
					/* empty option tag, ignore it */
					if(tmp->id == HT_OPTION)
						_XmHTMLWarning(__WFUNC__(html, "SelectToWord"),
							"Empty <OPTION> tag, ignored (line %i in input).",
							tmp->line);
					else
						_XmHTMLWarning(__WFUNC__(html, "SelectToWord"),
							"<%s> not allowed inside <OPTION> tag, ignored "
							"(line %i in input).", html_tokens[tmp->id],
							tmp->line);
				}
				continue;
			}
			/* get text */
			if((text = CopyText(html, tmp->element, False, &foo, True)) == NULL)
				continue;

			CollapseWhiteSpace(text);
			if(strlen(text))
			{
				_XmHTMLFormSelectAddOption(html, form_entry,
					sel_start->attributes, text);
				/* no longer needed */
				free(text);
			}
		}
	}
	/* close this selection and get width and height */
	_XmHTMLFormSelectClose(html, form_entry);

	/* allocate new word for this form member */
	word = allocFormWord(form_entry, width, height, owner, formatted);

	_XmHTMLFullDebug(2, ("format.c: SelectToWord, word is %s, len is %i, "
		"width is %i, height is %i\n", word->word, word->len,
		word->width, word->height));

	*num_words = 1;
	return(word);
}

/*****
* Name:			TextAreaToWord
* Return Type: 	XmHTMLWord*
* Description:	converts a HTML form <textarea> to a HTMLWord.
* In: 
*	html:		XmHTMLWidget id;
*	start:		object at which <textarea> starts;
*	*num_words:	no of words allocated. Updated upon return;
*	*width:		width of returned object. Updated upon return;
*	*height:	height of returned object. Updated upon return;
*	owner:		owning element.
*	formatted:	true when this form component is placed in a <pre></pre> tag.
* Returns:
*	a newly allocated word upon success. NULL on failure.
*****/
static XmHTMLWord*
TextAreaToWord(XmHTMLWidget html, XmHTMLObject *start, int *num_words,
	Dimension *width, Dimension *height, XmHTMLObjectTableElement owner,
	Boolean formatted)
{
	static XmHTMLForm *form_entry;
	XmHTMLWord *word;
	String text = NULL;
	int foo;

	*num_words = 0;
	*height = *width = 0;

	/* sanity check */
	if(start->attributes == NULL)
		return(NULL);

	/* get text between opening and closing <textarea>, if any */
	if(start->next->id == HT_ZTEXT)
		text = CopyText(html, start->next->element, True, &foo, False);

	/* create new form entry. text will serve as the default content */
	if((form_entry = _XmHTMLFormAddTextArea(html, start->attributes,
		text)) == NULL)
	{
		if(text)
			free(text);
		return(NULL);
	}
	form_entry->data = owner;

	/* allocate new word for this form member */
	word = allocFormWord(form_entry, width, height, owner, formatted);

	_XmHTMLFullDebug(2, ("format.c: TextAreaToWord, word is %s, len is %i, "
		"width is %i, height is %i\n", word->word, word->len,
		word->width, word->height));

	*num_words = 1;
	return(word);
}

/*****
* Name:			indexToWord
* Return Type: 	XmHTMLWord
* Description: 	creates a prefix for numbered lists with the ISINDEX
*				attribute set.
* In: 
*	html:		XmHTMLWidget id;
*	list_stack:	stack of all lists;
*	current...:	current list id;
*	owner:		owning element.
*	formatted:	true when this form component is placed in a <pre></pre> tag.
* Returns:
*	a newly allocated word.	
* Note:
*	This routine creates the prefix based on the type and depth of the
*	current list. All types can be intermixed, so this routine is capable
*	of returning something like 1.A.IV.c.iii for a list nested five levels,
*	the first with type `1', second with type `A', third with type `I',
*	fourth with type `a' and fifth with type `i'.
*****/
static XmHTMLWord* 
indexToWord(XmHTMLWidget html, listStack list_stack[], int current_list,
	XmHTMLObjectTableElement owner, Boolean formatted)
{
	static XmHTMLWord *word;
	int i;
	char index[128], number[42];	/* enough for a zillion numbers & depths */

	word = (XmHTMLWord*)calloc(1, sizeof(XmHTMLWord));

	(void)memset(&index, '\0', 128);
	for(i = 0; i < current_list; i++)
	{
		if(list_stack[i].type == HT_OL)
		{
			switch(list_stack[i].marker)
			{
				case XmMARKER_ALPHA_LOWER: 
					sprintf(number, "%s.", ToAsciiLower(list_stack[i].level));
					break;
				case XmMARKER_ALPHA_UPPER: 
					sprintf(number, "%s.", ToAsciiUpper(list_stack[i].level));
					break;
				case XmMARKER_ROMAN_LOWER:
					sprintf(number, "%s.", ToRomanLower(list_stack[i].level));
					break;
				case XmMARKER_ROMAN_UPPER:
					sprintf(number, "%s.", ToRomanUpper(list_stack[i].level));
					break;
				case XmMARKER_ARABIC:
				default:
					sprintf(number, "%i.", list_stack[i].level);
					break;
			}
			/* no buffer overflow */
			if(strlen(index) + strlen(number) > 128)
				break;
			strcat(index, number);
		}
	}

	/* fill in required fields */
	word->word = strdup(index);
	word->len  = strlen(index);
	word->self      = word;					/* unused */
	word->owner     = owner;				/* unused */
	word->font      = font_base.font;		/* unused */
	word->spacing   = formatted ? 0 : TEXT_SPACE_NONE;
	word->type      = OBJ_TEXT;				/* unused */
	word->line_data = NO_LINE;				/* unused */

	return(word);
}

/*****
* Name: 		TextToPre
* Return Type: 	XmHTMLWord*
* Description: 	splits the given text into an array of preformatted lines
* In: 
*	text:		text to split
*	num_words:	number of words in the given text. Filled upon return;
*	font:		font to use for this text.
* Returns:
*	an array of words. When allocation fails, this routine exits.
* Note:
*	the static var nchars is used to propagate the tab index to another
*	chunk of preformatted text if the current text is a block of preformatted
*	text with whatever formatting. It is only reset if an explicit newline
*	is encountered.
*****/
static XmHTMLWord* 
TextToPre(String text, int *num_words, XmHTMLfont *font, Byte line_data, 
	XmHTMLObjectTableElement owner)
{
	int nwords, len, i, j, ntabs, max_width, in_word, size, nfeeds;
	static char *raw;
	static XmHTMLWord *words;
	static int nchars = 0;
	register char *chPtr, *start, *end;
#ifdef DEBUG
	int used;
#endif

	/* sanity check */
	if(text == NULL)
	{
		*num_words = 0;
		return(NULL);
	}
 
	_XmHTMLFullDebug(2, ("format.c: TextToPre, text in is:\n%s\n", text));

	chPtr = text;
	raw = NULL;
		
	/***** 
	* compute how many words we have. A preformatted word is started
	* with a printing char and is terminated by either a newline or a
	* sequence of whitespaces. Multiple newlines are collapsed into a 
	* single word where the height of the word indicates the number of
	* newlines to insert.
	* The in_word logic comes from GNU wc.
	*****/
	in_word = nwords = ntabs = 1;	/* fix 01/30/97-02, kdh */
	while(True)
	{
		switch(*chPtr)
		{
			/* tabs and single spaces are collapsed */
			case '\t':	/* horizontal tab */
			case ' ':
				if(in_word)
				{
					while(*chPtr != '\0' && (*chPtr == ' ' || *chPtr == '\t'))
					{
						if(*chPtr == '\t')
							ntabs++;	/* need to know how many to expand */
						chPtr++;
					}
					nwords++;
					in_word = False;
				}
				else
				{
					/* fix 03/23/97-01, kdh */
					if(*chPtr == '\t')
						ntabs++;	/* need to know how many to expand */
					chPtr++;
				}
				break;
			/* newlines reset the tab index and are collapsed */
			case '\n':
				while(*chPtr != '\0' && *chPtr == '\n')
					chPtr++;
				nwords++;	/* current word is terminated */
				nchars = 1;
				break;
			default:
				chPtr++;
				in_word = True;
				break;
		}
		if(*chPtr == '\0')
			break;
	}

	/* sanity check */
	if(nwords == 0)
	{
		*num_words = 0;
		return(NULL);
	}

	/* add an extra word and tab for safety */
	nwords++;	/* preformatted text with other formatting *needs* this */
	ntabs++;

	/* compute amount of memory to allocate */
	size = ((ntabs*8)+strlen(text)+1)*sizeof(char);

	raw = (char*)malloc(size);

	_XmHTMLDebug(2, ("format.c: TextToPre, allocated %i bytes\n", size));

	/* allocate memory for all words */
	words = (XmHTMLWord*)calloc(nwords, sizeof(XmHTMLWord));

	chPtr = text;
	end = raw;
#ifdef DEBUG
	used = 0;
#endif
	/* first filter out all whitespace and other non-printing characters */
	while(True)
	{
		switch(*chPtr)
		{
			case '\f':	/* formfeed, ignore */
			case '\r':	/* carriage return, ignore */
			case '\v':	/* vertical tab, ignore */
				chPtr++;
#ifdef DEBUG
				used++;
#endif
				break;	
			case '\t':	/* horizontal tab */
				/* no of ``floating spaces'' to emulate a tab */
				len = ((nchars / 8) + 1) * 8;
				for(j = 0; j < (len - nchars); j++)
				{
					*end++ = ' ';		/* insert a tab */
#ifdef DEBUG
					used++;
#endif
				}
				nchars = len;
#ifdef DEBUG
				used++;
#endif
				chPtr++;
				break;
			/* newlines reset the tab index */
			case '\n':
				nchars = 0;	/* reset tab spacing index */
				/* fall thru */
			default:
				nchars++;
				*end++ = *chPtr++;
#ifdef DEBUG
				used++;
#endif
				break;
		}
		if(*chPtr == '\0')	/* terminate loop */
		{
			*end = '\0';
			break;
		}
	}
	_XmHTMLDebug(2, ("format.c: TextToPre, %i bytes actually used\n", used));

	/* Now go and fill all words */
	start = end = raw;
	max_width = i = len = 0;
	nfeeds = 0;

	while(True)
	{
		/* also pick up the last word! */
		if(*end == ' ' || *end == '\n' || *end == '\0')
		{
			if(*end)
			{
				/* skip past all spaces */
				while(*end != '\0' && *end != '\n' && *end == ' ')
				{
					end++;
					len++;
				}

				/***** 
				* if this word is ended by a newline, remove the newline.
				* X doesn't know how to interpret them.
				* We also want to recognize multiple newlines, so we must
				* skip past them.
				*****/
				if(*end == '\n')
				{
					while(*end != '\0' && *end == '\n')
					{
						nfeeds++;
						*end++ = '\0';
					}
					/*****
					* Since the no of newlines to add is stored in a
					* Byte, we need to limit the no of newlines to the
					* max. value a Byte can have: 255 (= 2^8)
					*****/
					if(nfeeds > 255)
						nfeeds = 255;
				}
			}

			words[i].type      = OBJ_TEXT;
			words[i].self      = &words[i];
			words[i].word      = start;
			words[i].height    = font->height;
			words[i].owner     = owner;
			words[i].spacing   = (Byte)nfeeds;	/* no of newlines */
			words[i].font      = font;
			words[i].line_data = line_data;
			words[i].len       = len;
			words[i].width     = Toolkit_Text_Width(font->xfont, words[i].word, len);
			start = end;
			i++;
			len = 0;
			nfeeds = 0;
		}
		if(*end == '\0')	/* terminate loop */
			break;
		end++;	/* move to the next char */
		len++;
	}

	_XmHTMLDebug(2, ("format.c: TexToPre, allocated %i words, %i actually "
		"used\n", nwords, i));

	/* total no of words */
	*num_words = i;
	return(words);
}

static XmHTMLWord*
BreakToWord(Dimension *height, XmHTMLfont *font, int linefeed,
	XmHTMLObjectTableElement owner)
{
	static XmHTMLWord *word;

	word = (XmHTMLWord*)calloc(1, sizeof(XmHTMLWord));

	word->type      = OBJ_BLOCK;
	word->self      = word;
	word->word      = (String)malloc(1);	/* needs an empty word */
	word->word[0]   = '\0';
	word->len       = 0;
	word->height    = font->height;			/* height of current font */
	word->owner     = owner;
	word->spacing   = TEXT_SPACE_LEAD | TEXT_SPACE_TRAIL;
	word->line_data = linefeed+1;	/* how much lines to break */
	word->font      = font;

	return(word);
}

static XmHTMLWord*
MakeDummyWord(Dimension *height, XmHTMLfont *font, Byte line_data,
	XmHTMLObjectTableElement owner)
{
	static XmHTMLWord *word;

	word = (XmHTMLWord*)calloc(1, sizeof(XmHTMLWord));

	word->type      = OBJ_TEXT;
	word->self      = word;
	word->word      = (String)malloc(1);		/* needs an empty word */
	word->word[0]   = '\0';
	word->len       = 0;
	word->height    = *height = font->height;	/* height of current font */
	word->owner     = owner;
	word->line_data = line_data;
	word->font      = font;
	/* an empty word acts as a single space */
	word->spacing   = TEXT_SPACE_LEAD|TEXT_SPACE_TRAIL;

	return(word);
}

/*****
* Name: 		SetTab
* Return Type: 	XmHTMLWord*
* Description: 	returns a XmHTMLWord with spaces required for a tab.
* In: 
* Returns:
*	the tab.
*****/
static XmHTMLWord* 
SetTab(int size, Dimension *height, XmHTMLfont *font, 
	XmHTMLObjectTableElement owner)
{
	static XmHTMLWord *tab;
	static char *raw;

	/* The tab itself */
	raw = (char*)malloc((size+1)*sizeof(char));

	/* fill with spaces */
	(void)memset(raw, ' ', size);
	raw[size] = '\0'; /* NULL terminate */

	tab = (XmHTMLWord*)calloc(1, sizeof(XmHTMLWord));

	/* Set all text fields for this tab */
	tab->self      = tab;
	tab->word      = raw;
	tab->len       = size;
	tab->height    = *height = font->height;
	tab->width     = Toolkit_Text_Width(font->xfont, raw, size);
	tab->owner     = owner;
	tab->spacing   = TEXT_SPACE_NONE;	/* a tab is already spacing */
	tab->font      = font;
	tab->type      = OBJ_TEXT;
	tab->line_data = NO_LINE;

	return(tab);	
}

/*****
* Name: 		CopyText
* Return Type: 	String
* Description: 	copies the given text to a newly malloc'd buffer.
* In: 
*	html:		XmHTMLWidget id;
*	text:		text to clean out.
*	formatted:	true when this text occurs inside <pre></pre>
*	text_data:	text option bits, spacing and such
*	expand_escapes:	
*				True -> expand escape sequences in text. Only viable when
*				copying pre-formatted text (plain text documents are handled
*				internally as consisting completely of preformatted text for
*				which the escapes may not be expanded).
* Returns:
*	cleaned up text. Terminates if malloc fails.
*****/
static String 
CopyText(XmHTMLWidget html, String text, Boolean formatted, int *text_data,
	Boolean expand_escapes)
{
	static String ret_val;
	GdkWChar *wtext, *start;
	int len;
	static Boolean have_space = False;
	static Boolean have_space_zerowidth = False;

	/* sanity check */
	if(*text == '\0' || !strlen(text))
		return(NULL);

	/* preformatted text, just copy and return */
	if(formatted)
	{
		*text_data = TEXT_SPACE_NONE;
		ret_val    = strdup(text);
		/* expand all escape sequences in this text */
		if(expand_escapes)
			_XmHTMLExpandEscapes(ret_val, html->html.bad_html_warnings);
		have_space = have_space_zerowidth = False;
		return(ret_val);
	}

	_XmHTMLFullDebug(2, ("format.c: CopyText, text in is:\n%s\n", text));

	/* convert to wide characters */
	wtext = (GdkWChar *)calloc(strlen(text) + 1, sizeof(GdkWChar));
	len = gdk_mbstowcs(wtext, text, strlen(text));
	if (len < 0) len = 0;
	start = wtext;

	*text_data = 0;

	/* see if we have any leading/trailing spaces */
	if(gdk_iswspace(*wtext) || have_space)
		*text_data = TEXT_SPACE_LEAD;
	else if(is_ideograph(*wtext) || have_space_zerowidth)
		*text_data = TEXT_SPACE_LEAD_ZEROWIDTH;

	if(len > 0 && gdk_iswspace(wtext[len-1]))
		*text_data |= TEXT_SPACE_TRAIL;
	else if(len > 0 && (is_ideograph(wtext[len-1])))
		*text_data |= TEXT_SPACE_TRAIL_ZEROWIDTH;

	/*****
	* Remove leading/trailing spaces
	* very special case: spaces between different text formatting
	* elements must be retained
	*****/
	/* remove all leading space */
	while(*start != '\0' && gdk_iswspace(*start))
	{
		start++;
		len--;
	}
	/* remove all trailing space */
	while(len > 0 && gdk_iswspace(start[len-1]))
		len--; 

	/*****
	* Spaces *can* appear between different text formatting elements. 
	* We want to retain this spacing since the above whitespace checking
	* only yields the current element, and does not take the previous text
	* element into account.
	* So when the current element doesn't have any leading or trailing spaces,
	* we use the spacing from the previous full whitespace element.
	* Obviously we must reset this data if we have text to process.
	*
	* Very special case: this text only contains whitespace and its therefore
	* most likely just spacing between formatting elements.
	* If the next text elements are in the same paragraph as this single
	* whitespace, we need to add a leading space if that text doesn't have
	* leading spaces. That's done above. If we have plain text, we reset
	* the prev_spacing or it will mess up the layout later on.
	*****/
	if(!len)
	{
		have_space = True;
		return(NULL);
	}
	have_space = have_space_zerowidth = False;
	if(len > 0 && (is_ideograph(wtext[len-1])))
		have_space_zerowidth = True;

	/* convert to multibyte form */
	start[len] = '\0';
	ret_val = gdk_wcstombs(start);
	free(wtext);

	/* expand all escape sequences in this text */
	if(expand_escapes)
		_XmHTMLExpandEscapes(ret_val, html->html.bad_html_warnings);

	return(ret_val);
}

/*****
* Name: 		ParseBodyTags
* Return Type: 	void
* Description: 	checks the <BODY> element for additional tags
* In: 
*	w:			HTML TWidget to check
*	data:		body element data.
* Returns:
*	nothing, but the HTML TWidget is updated to reflect the body stuff.
*****/
static void 
ParseBodyTags(XmHTMLWidget html, XmHTMLObject *data)
{
	char *chPtr;
	Boolean bg_color_set = False;	/* flag for bodyImage substitution */

	/* check all body color tags */
	if(html->html.body_colors_enabled)
	{
		Boolean doit = True;

		if((chPtr = _XmHTMLTagGetValue(data->attributes, "text")))
		{
			if(html->html.strict_checking)
				doit = _XmHTMLConfirmColor32(chPtr);

			if(doit)
				html->html.body_fg = _XmHTMLGetPixelByName(html, chPtr, 
					html->html.body_fg_save);
			free(chPtr);

			Toolkit_StyleColor_Foreground(html) = html->html.body_fg;
		}

		if(doit && (chPtr = _XmHTMLTagGetValue(data->attributes, "bgcolor")))
		{
			bg_color_set = True;

			if(html->html.strict_checking)
				doit = _XmHTMLConfirmColor32(chPtr);

			/* only change if we had success */
			if(doit)
			{
				html->html.body_bg = _XmHTMLGetPixelByName(html, chPtr, 
					html->html.body_bg_save);

				/* also set as background for the entire TWidget */

				Toolkit_StyleColor_Background(html) = html->html.body_bg;
#ifdef WITH_MOTIF
				XtVaSetValues(html->html.work_area,
					XmNbackground, html->html.body_bg, NULL);
#else
				{
					/* FIXME: I don't know if this is what we want */
					GdkColor c;
					c.pixel = html->html.body_bg;
					gdk_window_set_background(html->html.work_area->window, &c);
				}
#endif
				/* get new values for top, bottom & highlight */
				_XmHTMLRecomputeColors(html);
			}

			free(chPtr);
		}

		if(doit && (chPtr = _XmHTMLTagGetValue(data->attributes, "link")))
		{
			if(html->html.strict_checking)
				doit = _XmHTMLConfirmColor32(chPtr);

			if(doit)
				html->html.anchor_fg = _XmHTMLGetPixelByName(html, chPtr, 
					html->html.anchor_fg_save);
			free(chPtr);
		}

		if(doit && (chPtr = _XmHTMLTagGetValue(data->attributes, "vlink")))
		{
			if(html->html.strict_checking)
				doit = _XmHTMLConfirmColor32(chPtr);
			if(doit)
				html->html.anchor_visited_fg = _XmHTMLGetPixelByName(html,
					chPtr, html->html.anchor_visited_fg_save);
			free(chPtr);
		}

		if(doit && (chPtr = _XmHTMLTagGetValue(data->attributes, "alink")))
		{
			if(html->html.strict_checking)
				doit = _XmHTMLConfirmColor32(chPtr);
			if(doit)
				html->html.anchor_activated_fg = _XmHTMLGetPixelByName(html,
					chPtr, html->html.anchor_activated_fg_save);
			free(chPtr);
		}
		/*****
		* an invalid color spec, ignore them all together and revert to
		* saved settings
		*****/
		if(doit == False)
		{
			/* first check if we changed the background color */
			if (Toolkit_StyleColor_Background(html) != html->html.body_bg_save)
			{
				html->html.body_fg = html->html.body_fg_save;
				html->html.body_bg = html->html.body_bg_save;
				Toolkit_StyleColor_Foreground(html) = html->html.body_fg;
				Toolkit_StyleColor_Background(html) = html->html.body_bg;

#ifdef WITH_MOTIF
				XtVaSetValues(html->html.work_area,
					XmNbackground, html->html.body_bg, NULL);
#else
				{
					/* FIXME: I don't know if this is what we want */
					GdkColor c;
					c.pixel = html->html.body_bg;
					printf("3\n");
					gdk_window_set_background(html->html.work_area->window, &c);
				}
#endif
				
				/* restore values for top, bottom & highlight */
				_XmHTMLRecomputeColors(html);
			}
			
			html->html.body_fg             = html->html.body_fg_save;
			html->html.body_bg             = html->html.body_bg_save;
			html->html.anchor_fg           = html->html.anchor_fg_save;
			html->html.anchor_visited_fg   = html->html.anchor_visited_fg_save;
			html->html.anchor_activated_fg = html->html.anchor_activated_fg_save;
			Toolkit_StyleColor_Foreground(html) = html->html.body_fg;

			bg_color_set = False;
		}
	}

	/* Check background image spec. First invalidate any existing body image */
	if(html->html.body_image)
		html->html.body_image->options |= IMG_ORPHANED;
	html->html.body_image = (XmHTMLImage*)NULL;
	html->html.body_image_url = NULL;

	/*
	* *ALWAYS* load the body image if we want the SetValues method to
	* behave itself.
	*/
	if((chPtr = _XmHTMLTagGetValue(data->attributes, "background")))
	{
		_XmHTMLLoadBodyImage(html, chPtr);
		/* store document's body image location */
		if(html->html.body_image)
			html->html.body_image_url = html->html.body_image->url;
		free(chPtr);
	}
	/*
	* Use default body image if present *and* if no background color
	* has been set.
	*/
	else if(!bg_color_set && html->html.def_body_image_url)
		_XmHTMLLoadBodyImage(html, html->html.def_body_image_url);

	/*****
	* Now nullify it if we aren't to show the background image.
	* makes sense huh? (the list of images is a global resource so the
	* storage occupied by this unused image is freed when all document
	* data is freed).
	*****/
	if(!html->html.images_enabled || !html->html.body_images_enabled)
	{
		if(html->html.body_image)
			html->html.body_image->options |= IMG_ORPHANED;
		html->html.body_image = NULL;
	}
	/*****
	* When a body image is present it is very likely that a highlight
	* color based upon the current background actually makes an anchor
	* invisible when highlighting is selected. Therefore we base the
	* highlight color on the activated anchor background when we have a body
	* image, and on the document background when no body image is present.
	*****/
	if(html->html.body_image)
		_XmHTMLRecomputeHighlightColor(html, html->html.anchor_activated_fg);
	else
		_XmHTMLRecomputeHighlightColor(html, html->html.body_bg);
}

static void
FillBullet(XmHTMLWidget html, XmHTMLObjectTableElement owner)
{
	Dimension radius;
	char number[42];	/* large enough buffer for a zillion numbers */
	XmHTMLfont *font = html->html.default_font;
	String prefix;

	/***** 
	* x-offset for any marker and radius for a bullet or length of a 
	* side for a square marker.
	*****/
	radius = (Dimension)(0.5*(font->width));

	if(owner->marker == XmMARKER_DISC || owner->marker == XmMARKER_SQUARE ||
		owner->marker == XmMARKER_CIRCLE)
	{
		/* y-offset for this marker */
		owner->height = (Dimension)(0.5*font->lineheight + 0.25*radius);
		owner->width = radius;
	}
	else
	{
		/*****
		* If we have a word, this is an ordered list for which the index
		* should be propageted.
		*****/
		if(owner->words)
			prefix = owner->words[0].word;
		else
			prefix = "";
		switch(owner->marker)
		{
			case XmMARKER_ALPHA_LOWER: 
				sprintf(number, "%s%s.", prefix,
					ToAsciiLower(owner->list_level));
				break;
			case XmMARKER_ALPHA_UPPER: 
				sprintf(number, "%s%s.", prefix,
					ToAsciiUpper(owner->list_level));
				break;
			case XmMARKER_ROMAN_LOWER:
				sprintf(number, "%s%s.", prefix,
					ToRomanLower(owner->list_level));
				break;
			case XmMARKER_ROMAN_UPPER:
				sprintf(number, "%s%s.", prefix,
					ToRomanUpper(owner->list_level));
				break;
			case XmMARKER_ARABIC:
			default:
				sprintf(number, "%s%i.", prefix, owner->list_level);
				break;
		}
		owner->text  = strdup(number);
		owner->len   = strlen(number);
		owner->width = radius + Toolkit_Text_Width (font->xfont, owner->text, owner->len);
		owner->height = html->html.default_font->height;
	}
}

/*****
* Name: 		CheckLineFeed
* Return Type: 	int
* Description: 	checks wether the requested newline is honored.
* In: 
*	this:		newline to add.
*	force:		add the requested newline anyway
* Returns:
*	computed vertical pixel offset.
* Note:
*	any of CLEAR_NONE, CLEAR_SOFT or CLEAR_HARD
*****/
static int
CheckLineFeed(int this, Boolean force)
{
	static int prev_state = CLEAR_NONE;
	int ret_val = this;

	if(force)
	{
		prev_state = this;
		return(ret_val);
	}

	/* multiple soft and hard returns are never honored */
	switch(this)
	{
		case CLEAR_HARD:
			if(prev_state == CLEAR_SOFT)
			{
				ret_val = CLEAR_SOFT;
				prev_state = CLEAR_HARD;
				break;
			}
			if(prev_state == CLEAR_HARD)
			{
				/* unchanged */
				ret_val = CLEAR_NONE;
				break;
			}
			prev_state = ret_val = this;
			break;
		case CLEAR_SOFT:
			if(prev_state == CLEAR_SOFT)
			{
				ret_val = CLEAR_NONE;
				prev_state = CLEAR_SOFT;
				break;
			}
			if(prev_state == CLEAR_HARD)
			{
				/* unchanged */
				ret_val = CLEAR_NONE;
				break;
			}
			ret_val = prev_state = this;
			break;
		case CLEAR_NONE:
			ret_val = prev_state = this;
			break;
	}
	return(ret_val);
}

/*****
* Name:			tableCheckProperties
* Return Type: 	TableProperties
* Description: 	scans a table element for common properties (border,
*				alignment, background and border styles).
* In: 
*	html:		XmHTMLWidget id;
*	attributes:	attributes to be checked;
*	parent:		properties of parent table element. Properties not found
*				in the attributes are inherited from this parent.
* Returns:
*	a new property.
*****/
static TableProperties*
tableCheckProperties(XmHTMLWidget html, String attributes,
	TableProperties *parent, Alignment halign, Pixel bg)
{
	TableProperties prop;
	static TableProperties *prop_ret;
	String chPtr;

	prop_ret = (TableProperties*)malloc(sizeof(TableProperties));
	memset(prop_ret, 0, sizeof(TableProperties));

	if(parent)
		memcpy(&prop, parent, sizeof(TableProperties));
	else
	{
		/* defaults assume a table without any borders */
		prop.border   = -1;
		prop.halign   = halign;		 	/* unused */
		prop.valign   = XmVALIGN_TOP;		/* contents on top */
		prop.bg       = bg;			/* propagate */
		prop.bg_image = NULL;
		prop.framing  = TFRAME_VOID;		/* no border */
		prop.ruling   = TRULE_NONE;		/* no ruling */
		prop.nowrap   = False;			/* wrap long lines */
	}

	/*****
	* Horizontal alignment is only inherited through the halign argument:
	* the align attribute on the table tag applies to the table in a whole,
	* not to any of it's members.
	*****/
	prop_ret->halign   = _XmHTMLGetHorizontalAlignment(attributes, halign);
	prop_ret->valign   = _XmHTMLGetVerticalAlignment(attributes, prop.valign);
	prop_ret->nowrap   = _XmHTMLTagCheck(attributes, "nowrap");

	/*****
	* Border value. If -1 is returned, check for the presence of the word
	* ``border'' in the attributes. If it exists, we assume a non-zero
	* border width.
	*****/
	prop_ret->border   = _XmHTMLTagGetNumber(attributes, "border", prop.border);
	/* if the word ``border'' is present, use default border width */
	if(prop_ret->border == -1 && _XmHTMLTagCheck(attributes, "border"))
		prop_ret->border = XmHTML_DEFAULT_TABLE_BORDERWIDTH;

	/*****
	* Framing applies per-table. If border is non-zero, the default is
	* to render a fully framed table.
	* If a TFRAME_VOID is returned, discard the border width.
	*****/
	prop_ret->framing  = _XmHTMLGetFraming(attributes,
							prop_ret->border > 0 ? TFRAME_BOX : prop.framing);
	if(prop_ret->framing == TFRAME_VOID)
		prop_ret->border = 0;

	/*****
	* Ruling applies per-cell. If border is non-zero, the default is to
	* render full borders around this cell.
	* If a TRULE_NONE is returned, discard the border width.
	*****/
	prop_ret->ruling   = _XmHTMLGetRuling(attributes,
							prop_ret->border ? TRULE_ALL : prop.ruling);

	if(prop_ret->ruling == TRULE_NONE)
		prop_ret->border = 0;

	/* only pick up background color if we are allowed to honor this attrib */
	if(html->html.allow_color_switching &&
		(chPtr = _XmHTMLTagGetValue(attributes, "bgcolor")) != NULL)
	{
		Boolean doit = True;
		if(html->html.strict_checking)
			doit = _XmHTMLConfirmColor32(chPtr);
		if(doit)
			prop_ret->bg = _XmHTMLGetPixelByName(html, chPtr, prop.bg);
		free(chPtr);
	}
	else
		prop_ret->bg = prop.bg;

	/* table background image? */
	if((chPtr = _XmHTMLTagGetValue(attributes, "background")))
	{
		String buf;
		Dimension width, height;
		XmHTMLImage *image;

		/* kludge so _XmHTMLNewImage recognizes it */
		buf = malloc(strlen(chPtr)+7);
		sprintf(buf, "src=\"%s\"", chPtr);

		/* load it */
		if((image = _XmHTMLNewImage(html, buf, &width, &height)) != NULL)
		{
			/* animations are not allowed as background images */
			if(ImageIsAnim(image))
				image = NULL;
			/* and we sure won't have the default image as background */
			else if(ImageIsInternal(image))
				image = NULL;
		}
		prop_ret->bg_image = image;
		free(buf);
		free(chPtr);
	}
	else
		prop_ret->bg_image = prop.bg_image;
	return(prop_ret);
}

/*****
* Name:			tableOpen
* Return Type: 	XmHTMLTable
* Description: 	opens a new table.
* In: 
*	html;		XmHTMLWidget id;
*	parent:		parent table. Storage for nested tables is always allocated
*				in this table. It is NULL when a truely new table is needed.
*	start:		start of objects contained in this table.
*	obj:		XmHTMLObject starting this table.
* Returns:
*	a newly opened table.
*****/
static XmHTMLTable*
tableOpen(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj, Alignment *halign,
	Pixel *bg)
{
	static XmHTMLTable *table;
	XmHTMLTable *parent_table;
	XmHTMLObject *tmp;
	int nrows = 0;				/* no of rows in table				*/
	int depth = 0;
	int nchilds = 0;			/* no of table childs in this table */
	Alignment caption_position = XmVALIGN_TOP; /* where is the caption? */
	Boolean have_caption = False;

	if(parent)
	{
		/* get to the absolute parent of this table */
		parent_table = parent;
		while(parent_table->parent != NULL)
			parent_table = parent_table->parent;

		/* get correct ptr */
		parent_table = &(parent_table->childs[0]);

		/* sanity check */
		if(parent_table->lastchild+1 == parent_table->nchilds)
		{
			_XmHTMLError(__WFUNC__(html, "tableOpen"),
				"Bad table count!!!");
		}
		/* get next available table */
		parent_table->lastchild++;
		table = &(parent_table->childs[parent_table->lastchild]);
	}
	else
	{
		table = (XmHTMLTable*)calloc(1, sizeof(XmHTMLTable));
	}

	/*****
	* Get table attributes.
	*****/
	table->width    = _XmHTMLTagCheckNumber(obj->attributes, "width", 0);
	table->hmargin  = _XmHTMLTagGetNumber(obj->attributes, "cellspacing",
						XmHTML_DEFAULT_CELLSPACING);
	table->vmargin  = _XmHTMLTagGetNumber(obj->attributes, "rowspacing", 
						XmHTML_DEFAULT_ROWSPACING);
	table->hpadding = _XmHTMLTagGetNumber(obj->attributes, "cellpadding", 0);
	table->vpadding = _XmHTMLTagGetNumber(obj->attributes, "rowpadding", 0);
	table->ncols    = _XmHTMLTagGetNumber(obj->attributes, "cols", 0);
	table->start    = start;	/* starting object */
	table->owner    = start;	/* owning object */
	table->parent	= NULL;		/* parent table */

	/* table properties */
	table->properties = tableCheckProperties(html, obj->attributes,
		parent ? parent->properties : NULL, *halign, *bg);

	/* set return alignment */
	*halign = table->properties->halign;

	/* set return background */
	*bg = table->properties->bg;

	/* count how many rows this table has */
	for(tmp = obj->next; tmp != NULL; tmp = tmp->next)
	{
		/* check for end of table and child tables */
		if(tmp->id == HT_TABLE)
		{
			if(tmp->is_end)
			{
				if(depth == 0)
					break;
				else
					depth--;
			}
			else	/* new table opens */
			{
				depth++;
				nchilds++;
			}
		}
		/*****
		* only count a row when it belongs to the top-level table.
		* A caption is considered a special row that spans the entire table
		* and has only one cell: the row itself.
		*****/
		if((tmp->id == HT_TR || tmp->id == HT_CAPTION) && depth == 0 &&
			!tmp->is_end)
		{
			if(tmp->id == HT_CAPTION)
			{
				/*****
				* see where the caption should be inserted: as the first
				* or last row for this table.
				*****/
				String chPtr = _XmHTMLTagGetValue(tmp->attributes, "align");
				if(chPtr == NULL)
					caption_position = XmVALIGN_TOP;
				else
				{
					if(!(strcasecmp(chPtr, "bottom")))
						caption_position = XmVALIGN_BOTTOM;
					else
						caption_position = XmVALIGN_TOP;
					free(chPtr);
				}
				have_caption = True;
			}
			nrows++;
		}
	}
	/* sanity, should never happen */
	if(!nrows)
		nrows++;

	/* allocate all rows for this table */
	table->rows = (TableRow*)calloc(nrows, sizeof(TableRow));
	table->nrows = nrows;
	table->lastrow = 0;

	/* set caption ptr. */
	if(have_caption)
	{
		if(caption_position == XmVALIGN_TOP)
		{
			table->caption = &(table->rows[0]);
			table->lastrow = 1;
		}
		else
			table->caption = &(table->rows[nrows-1]);
	}

	/* The master table contains all tables */
	if(parent == NULL)
	{
		nchilds++;
		table->childs = (XmHTMLTable*)calloc(nchilds, sizeof(XmHTMLTable));
		table->nchilds = nchilds;
		table->childs[0] = *table;		/* we are the first table */
		table->lastchild = 0;
	}
	else
	{
		table->childs = (XmHTMLTable*)NULL;
		table->nchilds = 0;
		table->lastchild = 0;
		/* set parent table */
		table->parent = parent;
	}

	/* and set as table in the element given to us */
	start->table = table;

	return(table);
}

/*****
* Name:			tableClose
* Return Type: 	int
* Description: 	performs required table wrapup actions
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	end:		last object to be used by this table;
* Returns:
*	-1 when this was the last table to be closed, a 0 or a positive integer
*	when there are still tables open.
*****/
static XmHTMLTable*
tableClose(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end)
{
	/* current table */
	XmHTMLTable *real_table, *table = parent;
	int i, ncols = 0;

	/* sanity */
	if(parent == NULL)
		return(NULL);

	/* bad hack */
	real_table = parent->owner->table;
	real_table->start = parent->owner->next;
	real_table->end   = end;

	/* pick up correct ptr */
	if(table->parent == NULL)
		table = &(parent->childs[0]);

	table->start = table->owner->next;
	table->end   = end;

	/*****
	* Sanity Check: check all cells in search of a rowspan attribute. If
	* we detect a rowspan in the *last* cell of a row, we must add a bogus
	* cell to this row. If we don't do this, any cells falling in this row
	* will be skipped, causing text to disappear (at the least, in the worst
	* case it will cause a crash).
	*****/

	/* See how many columns we have (if not already set by the COLS attr.) */
	for(i = 0; i < table->nrows; i++)
	{
		if(ncols < table->rows[i].ncells)
			ncols = table->rows[i].ncells;
	}
	if(ncols > table->ncols)
		table->ncols = ncols;

	/* move to current table */
	return(table->parent);
}

/*****
* Name:			tableOpenCaption
* Return Type: 	void
* Description: 	adds a caption to the given table.
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	start:		start of objects contained in this caption.
*	obj:		XmHTMLObject starting this caption.
* Returns:
*	nothing, but upon return the current table will have a caption.
*****/
static void
tableOpenCaption(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj,
	Pixel *bg)
{
	XmHTMLTable *table = parent;
	TableRow *caption;
	TableCell *cell;

	/* sanity */
	if(parent == NULL)
		return;

	if(table->parent == NULL)
		table = &(parent->childs[0]);

	/* get caption */
	caption = table->caption;

	/* only one caption allowed */
	if(caption->lastcell)
		return;

	/*****
	* Get properties for this caption.
	* The global table properties are *never* propagated since, officially,
	* a Caption doesn't have any attributes...
	*****/
	caption->properties = tableCheckProperties(html, obj->attributes,
						NULL, html->html.default_halign, *bg);

	/* starting object */
	caption->start = start;
	caption->owner = start;
	
	/* set parent table */
	caption->parent = table;

	/* one cell: the caption itself */
	caption->cells = (TableCell*)calloc(1, sizeof(TableCell));
	caption->ncells   = 1;
	caption->lastcell = 1;

	/* fill in used fields */
	cell = &(caption->cells[0]);

	cell->header = False;	/* unused */
	cell->width = 0;		/* unused */
	cell->height = 0;		/* unused */
	cell->rowspan = 1;		/* unused */
	cell->colspan = 0;		/* spans the full table width */

	/* get properties for this cell */
	cell->properties = tableCheckProperties(html, obj->attributes,
						NULL, caption->properties->halign,
						caption->properties->bg);
	/* set return background */
	*bg = caption->properties->bg;

	/* starting object */
	cell->start = start;
	cell->owner = start;
	
	/* ending object unknown */
	cell->end = NULL;

	/* set parent caption */
	cell->parent = caption;

	/* all done */
}

/*****
* Name:			tableCloseCaption
* Return Type: 	int
* Description: 	performs required caption wrapup actions
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	end:		last object to be used by this caption;
* Returns:
*	nothing.
*****/
static void
tableCloseCaption(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end)
{
	XmHTMLTable *table = parent;
	TableRow *caption;
	TableCell *cell;

	/* sanity */
	if(parent == NULL)
		return;

	if(table->parent == NULL)
		table = &(parent->childs[0]);

	caption = table->caption;

	/* sanity */
	if(caption->ncells == 0)
		return;

	cell = &(caption->cells[0]);

	cell->start = cell->start->next;
	cell->end   = end;
}

/*****
* Name:			tableOpenRow
* Return Type: 	void
* Description: 	adds a row to the current table.
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	start:		start of objects contained in this row.
*	obj:		XmHTMLObject starting this row, used to compute no of cells
*				in a row.
* Returns:
*	Nothing, but a new row has been prepared in the current table.
*****/
static void
tableOpenRow(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj, Alignment *halign,
	Pixel *bg)
{
	XmHTMLTable *table = parent;
	TableRow *row;
	XmHTMLObject *tmp;
	int ncells = 0;
	int depth  = 0;

	/* sanity */
	if(parent == NULL)
		return;

	if(table->parent == NULL)
		table = &(parent->childs[0]);

	/* sanity */
	if(table->lastrow == table->nrows)
	{
		_XmHTMLError(__WFUNC__(html, "tableRowOpen"),
			"Bad tablerow count!!!");
	}

	/* get next available row in this table */
	row = &(table->rows[table->lastrow]);

	/* get properties for this row */
	row->properties = tableCheckProperties(html, obj->attributes,
						table->properties, html->html.default_halign, *bg);
	/* set return alignment */
	*halign = row->properties->halign;

	/* set return background */
	*bg = row->properties->bg;

	/* starting object */
	row->start = start;
	row->owner = start;
	
	/* set parent table */
	row->parent = table;

	/* count how many cells this row has */
	for(tmp = obj->next; tmp != NULL; tmp = tmp->next)
	{
		/* check for end of row and child rows (in child tables) */
		if(tmp->id == HT_TR)
		{
			if(tmp->is_end)
			{
				if(depth == 0)
					break;
				else
					depth--;
			}
			else	/* new row opens */
				depth++;
		}
		/* only count a cell when it belongs to the top-level row */
		if((tmp->id == HT_TH || tmp->id == HT_TD) && depth == 0 && !tmp->is_end)
			ncells++;
	}
	/* empty rows don't have cells */
	if(ncells)
		row->cells = (TableCell*)calloc(ncells, sizeof(TableCell));
	else	/* allocate an empty cell */
		row->cells = (TableCell*)calloc(1, sizeof(TableCell));
	row->ncells   = ncells;
	row->lastcell = 0;

	/* move to next available row */
	table->lastrow++;
}

/*****
* Name:			tableCloseRow
* Return Type: 	int
* Description: 	performs required row wrapup actions
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	end:		last object to be used by this row;
* Returns:
*	nothing.
*****/
static void
tableCloseRow(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end)
{
	XmHTMLTable *table = parent;
	TableRow *row;
	int i, ncols = 0;

	/* sanity */
	if(parent == NULL)
		return;

	if(table->parent == NULL)
		table = &(parent->childs[0]);

	/* get current row in this table */
	row = &(table->rows[table->lastrow-1]);

	/*****
	* Count how many columns this row has (including cells spanning multiple
	* columns).
	*****/
	for(i = 0; i < row->ncells; i++)
		ncols += row->cells[i].colspan;

	if(ncols > table->ncols)
		table->ncols = ncols;

	row->start = row->start->next;
	row->end   = end;
}

/*****
* Name:			tableOpenCell
* Return Type: 	void
* Description: 	adds a cell to the current row in the current table.
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	start:		start of objects contained in this cell;
*	obj:		XmHTMLObject starting this cell;
* Returns:
*	Nothing, but a new cell has been prepared in the current row.
*****/
static void
tableOpenCell(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement start, XmHTMLObject *obj, Alignment *halign,
	Pixel *bg)
{
	XmHTMLTable *table = parent;
	TableRow *row;
	TableCell *cell;

	/* sanity */
	if(parent == NULL)
		return;

	if(table->parent == NULL)
		table = &(parent->childs[0]);

	/* get current row in this table */
	row = &(table->rows[table->lastrow-1]);

	/* sanity */
	if(row->lastcell == row->ncells)
	{
		_XmHTMLError(__WFUNC__(html, "tableCellOpen"),
			"Bad tablerow cell count!!!");
	}

	/* get next available cell in this row */
	cell = &(row->cells[row->lastcell]);

	/* get cell-specific properties */
	cell->header = (obj->id == HT_TH ? True : False);
	cell->width = _XmHTMLTagCheckNumber(obj->attributes, "width", 0);
	cell->height = _XmHTMLTagCheckNumber(obj->attributes, "height", 0);
	cell->rowspan = _XmHTMLTagGetNumber(obj->attributes, "rowspan", 1);
	cell->colspan = _XmHTMLTagGetNumber(obj->attributes, "colspan", 1);

	/* [row/cell]span = 0 : span entire table in requested direction */
	if(cell->rowspan <= 0 || cell->rowspan > table->nrows)
		cell->rowspan = table->nrows;

	/*****
	* colspan <= 0 gets handled in SetTable when we now how many columns
	* this table has
	*****/

	/* get global properties for this cell */
	cell->properties = tableCheckProperties(html, obj->attributes,
						row->properties, row->properties->halign, *bg);
	/* set return alignment */
	*halign = cell->properties->halign;

	/* set return background */
	*bg = cell->properties->bg;

	/* starting object */
	cell->start = start;
	cell->owner = start;
	
	/* set parent row */
	cell->parent = row;

	/* move to next available cell */
	row->lastcell++;
}

/*****
* Name:			tableCloseCell
* Return Type: 	int
* Description: 	performs required cell wrapup actions
* In: 
*	html:		XmHTMLWidget id;
*	parent:		parent table;
*	end:		last object to be used by this cell;
* Returns:
*	nothing.
*****/
static void
tableCloseCell(XmHTMLWidget html, XmHTMLTable *parent,
	XmHTMLObjectTableElement end)
{
	XmHTMLTable *table = parent;
	TableRow *row;
	TableCell *cell;

	/* sanity */
	if(parent == NULL)
		return;

	if(table->parent == NULL)
		table = &(parent->childs[0]);

	/* get current row in this table */
	row = &(table->rows[table->lastrow-1]);

	/* get current cell in this row */
	cell = &(row->cells[row->lastcell-1]);

	cell->start = cell->start->next;
	cell->end   = end;
}

static void
PushAlignment(Alignment align)
{
	alignStack *tmp;

	tmp = (alignStack*)malloc(sizeof(alignStack));
	tmp->align = align;
	tmp->next = align_stack;
	align_stack = tmp;
}

static Alignment
PopAlignment(void)
{
	Alignment align;
	alignStack *tmp;

	if(align_stack->next != NULL)
	{
		tmp = align_stack;
		align_stack = align_stack->next;
		align = tmp->align;
		free(tmp);
	}
	else
	{
		_XmHTMLDebug(2, ("XmHTML: negative alignment stack!\n"));
		align = align_stack->align;
	}
	return(align);
}

static void
PushFGColor(Pixel color)
{
	colorStack *tmp;

	tmp = (colorStack*)malloc(sizeof(colorStack));
	tmp->color = color;
	tmp->next = fg_color_stack;
	fg_color_stack = tmp;
}

static Pixel
PopFGColor(void)
{
	Pixel color;
	colorStack *tmp;

	if(fg_color_stack->next != NULL)
	{
		tmp = fg_color_stack;
		fg_color_stack = fg_color_stack->next;
		color = tmp->color;
		free(tmp);
	}
	else
	{
		_XmHTMLDebug(2, ("XmHTML: negative foreground color stack!\n"));
		color = fg_color_stack->color;
	}
	return(color);
}

static void
PushBGColor(Pixel color)
{
	colorStack *tmp;

	tmp = (colorStack*)malloc(sizeof(colorStack));
	tmp->color = color;
	tmp->next = bg_color_stack;
	bg_color_stack = tmp;
}

static Pixel
PopBGColor(void)
{
	Pixel color;
	colorStack *tmp;

	if(bg_color_stack->next != NULL)
	{
		tmp = bg_color_stack;
		bg_color_stack = bg_color_stack->next;
		color = tmp->color;
		free(tmp);
	}
	else
	{
		_XmHTMLDebug(2, ("XmHTML: negative background color stack!\n"));
		color = bg_color_stack->color;
	}
	return(color);
}

static void
PushFont(XmHTMLfont *font, int size)
{
	fontStack *tmp;

	tmp = (fontStack*)malloc(sizeof(fontStack));
	tmp->font = font;
	tmp->size = size;
	tmp->next = font_stack;
	font_stack = tmp;
#ifdef DEBUG
	_XmHTMLDebug(2, ("format.c: PushFont, pushed font %s\n", font->font_name));
#endif
}

static XmHTMLfont* 
PopFont(int *size)
{
	XmHTMLfont *font;
	fontStack *tmp;

	if(font_stack->next != NULL)
	{
		tmp = font_stack;
		font_stack = font_stack->next;
		font = tmp->font;
		*size = tmp->size;
		free(tmp);
	}
	else
	{
		_XmHTMLDebug(2, ("XmHTML: negative font stack!\n"));
		font = font_stack->font;
		*size = font_stack->size;
	}
#ifdef DEBUG
	_XmHTMLDebug(2, ("format.c: PopFont, popped font %s\n", font->font_name));
#endif
	return(font);
}

#define PUSH_COLOR(TWidget) { \
	char *chPtr; \
	/* check for color spec */ \
	PushFGColor(fg); \
	chPtr = _XmHTMLTagGetValue(temp->attributes, "color"); \
	if(chPtr != NULL) \
	{ \
		Boolean doit = True; \
		if(html->html.strict_checking) \
			doit = _XmHTMLConfirmColor32(chPtr); \
		if(doit) fg = _XmHTMLGetPixelByName(html, chPtr, fg); \
		free(chPtr); \
	} \
}

#define CHECK_LINE { \
	if(element_data & ELE_ANCHOR) { \
		if(element_data & ELE_ANCHOR_TARGET) \
			line_data = html->html.anchor_target_line; \
		else if(element_data & ELE_ANCHOR_VISITED) \
			line_data = html->html.anchor_visited_line; \
		else \
			line_data = html->html.anchor_line; \
	} \
	/* ignore <u> for anchors */ \
	else { \
		if(element_data & ELE_UNDERLINE) \
			line_data  = LINE_SOLID | LINE_UNDER; \
	} \
	/* check strikeout flag */ \
	if(element_data & ELE_STRIKEOUT) \
		line_data |= LINE_STRIKE; \
}

/********
****** Private XmHTML Functions
********/

/*****
* Name: 		_XmHTMLNewAnchor
* Return Type:	XmHTMLAnchor *
* Description: 	allocates and fills an anchor object
* In: 
*	html:		owning TWidget
*	object:		raw anchor data
* Returns:
*	the allocated object
*****/
XmHTMLAnchor*
_XmHTMLNewAnchor(XmHTMLWidget html, XmHTMLObject *object)
{
	static XmHTMLAnchor *anchor;

	/* stupid sanity check */
	if(object->attributes == NULL)
		return(NULL);
	
	anchor = (XmHTMLAnchor*)malloc(sizeof(XmHTMLAnchor));

	/* set all fields to zero */
	(void)memset(anchor, 0, sizeof(XmHTMLAnchor));

	/* anchors can be both named and href'd at the same time */
	anchor->name = _XmHTMLTagGetValue(object->attributes, "name");

	/* get the url specs */
	parseHref(object->attributes, anchor); 

	/* get the url type */
	anchor->url_type = XmHTMLGetURLType(anchor->href);

	/* promote to named if necessary */
	if(anchor->url_type == ANCHOR_UNKNOWN && anchor->name)
		anchor->url_type = ANCHOR_NAMED;

	/* see if we need to watch any events for this anchor */
	anchor->events = _XmHTMLCheckCoreEvents(html, object->attributes);

#ifdef PEDANTIC
	if(anchor->url_type == ANCHOR_UNKNOWN)
	{
		_XmHTMLWarning(__WFUNC__(html, "_XmHTMLNewAnchor"), "Could "
			"not determine URL type for anchor %s (line %i of input)\n",
			object->attributes, object->line);
	}
#endif /* PEDANTIC */

	/* 
	* If we have a proc available for anchor testing, call it and 
	* set the visited field.
	*/
#ifdef WITH_MOTIF
	if(html->html.anchor_visited_proc)
 		anchor->visited = html->html.anchor_visited_proc((TWidget)html, 
 				anchor->href, html->html.client_data);
#else
	gtk_signal_emit (GTK_OBJECT (html),
			 gtk_xmhtml_signals[GTK_XMHTML_ANCHOR_VISITED],
			 anchor->href, html->html.client_data,
			 &(anchor->visited));
#endif
	/* insert in the anchor list */
	if(list_data.anchor_head)
	{
		/*****
		* We can't do anything about duplicate anchors. Removing them
		* would mess up the named anchor lookup table.
		*****/
		list_data.anchor_current->next = anchor;
		list_data.anchor_current = anchor;
	}
	else
	{
		list_data.anchor_head = list_data.anchor_current = anchor;
	}
	return(anchor);
}

/*****
* Name:			_XmHTMLformatObjects
* Return Type:	XmHTMLObjectTable*
* Description:	creates a list of formatted HTML objects.
* In:
*	old:		previous XmHTMLWidget id (contains all items to be freed);
*	html:		current XmHTMLWidget id, containing raw (parser) html object
*				data and will receive new formatted data.
* Returns:
*	nothing, but list of formatted objects, anchors, tables (and images)
*	is updated upon return.
*****/
void
_XmHTMLformatObjects(XmHTMLWidget old, XmHTMLWidget html)
{
	/* text level variables */
	String text;
	int linefeed, n_words, anchor_words, named_anchors;
	int x_offset = 0, y_offset = 0;
	int text_data;		/* text data bits */
	Byte line_data;		/* line data bits */
	unsigned long element_data = 0;
	XmHTMLWord *words;
	XmHTMLAnchor *anchor_data, *form_anchor_data;
#ifdef DEBUG
	int num_ignore = 0;
	static String func = "_XmHTMLformatObjects";
#endif

	/* list variables */
	int ul_level, ol_level, ident_level, current_list;
	listStack list_stack[MAX_NESTED_LISTS];
	int in_dt = 0;

	/* remaining object variables */
	Pixel fg, bg;
	Dimension width, height;
	Alignment halign, valign; 
	ObjectType object_type;
	XmHTMLfont *font;
	static XmHTMLObjectTableElement element;
	XmHTMLObjectTableElement previous_element = NULL;
	int basefont;

	/* imagemap and area stuff */
	XmHTMLImageMap *imageMap = NULL;

	/* local flags */
	Boolean ignore = False, in_pre = False;

	/* HTML tables */
	XmHTMLTable *table = NULL;
	XmHTMLTable *current_table = NULL;

	/* misc. variables */
	XmHTMLObject *temp;
	int i, new_anchors = 0;
	Boolean anchor_data_used = False;

#ifdef DEBUG
	allocated = 0;
#endif

	/* Free any previous lists and initialize it */
	InitObjectTable(old->html.formatted, old->html.anchor_data);
	html->html.formatted   = (XmHTMLObjectTable*)NULL;
	html->html.anchor_data = (XmHTMLAnchor*)NULL;

	/* free table data */
	freeTables(old->html.tables);
	html->html.tables = (XmHTMLTable*)NULL;

	/* 
	* Nothing to do, just return. Should only happen if we get called
	* from Destroy().
	*/
	if(html->html.elements == NULL)
	{
		/* free top of list */
		if(list_data.head)
			free(list_data.head);
		list_data.head = NULL;
		return;
	}

	/* Move to the body element */
	for(temp = html->html.elements; temp != NULL && temp->id != HT_BODY; 
		temp = temp->next);

	/*
	* No <body> element found. This is an error since the parser will
	* *always* add a <body> element if none is present in the source
	* document.
	*/
	if(temp == NULL)
	{
		/*****
		* The only exception is a document only containing plain text and no
		* BODY tag was present in the input. In this case, check the input and
		* start outputting at the end of the first text element not belonging
		* to the head of a document.
		* Fix 01/04/98-01, kdh
		*****/
		XmHTMLObject *last_obj = NULL;

		for(temp = html->html.elements; temp != NULL; temp = temp->next)
		{
			switch(temp->id)
			{
				case HT_DOCTYPE:
				case HT_BASE:
					/* these two have no ending tag */
					last_obj = temp;
					break;
				case HT_HTML:
					/* don't use the closing tag for this, it's the end... */
					if(!temp->is_end)
						last_obj = temp;
					break;
				case HT_HEAD:
				case HT_TITLE:
				case HT_SCRIPT:
				case HT_STYLE:
					/* pick up the last closing tag */
					if(temp->is_end)
						last_obj = temp;
				default:
					break;
			}
		}

		if(last_obj == NULL || last_obj->next == NULL)
		{
			_XmHTMLWarning(__WFUNC__(html, func), "Nothing to display: no "
				"<BODY> tag found (HTML parser failure)!");
			return;
		}
		/* we move to the correct object a bit further down */
		temp = last_obj;
	}

	/* initialize font stack */
	font_stack = &font_base;
	font_stack->font = font = _XmHTMLSelectFontCache(html, False);
	font_stack->size = basefont = 4;
	font_stack->next = NULL;

	/* Reset anchor count */
	anchor_words = 0;
	named_anchors = 0;
	anchor_data = form_anchor_data = NULL;

	/* initialize list variables */
	ul_level = 0;
	ol_level = 0;
	ident_level = 0;
	current_list = 0;

	/* reset stacks */
	for(i = 0; i < MAX_NESTED_LISTS; i++)
	{
		list_stack[i].isindex = False;
		list_stack[i].marker  = XmMARKER_NONE;
		list_stack[i].level   = 0;
		list_stack[i].type    = HT_ZTEXT;
	}

	/* Initialize linefeeding mechanism */
	linefeed = CheckLineFeed(CLEAR_SOFT, True);

	/* Initialize alignment */
	align_stack = &align_base;
	align_stack->align = halign = html->html.default_halign;
	align_stack->next = NULL;
	valign = XmVALIGN_NONE;
	object_type = OBJ_NONE;

	/* check for background stuff */
	ParseBodyTags(html, temp);

	/* foreground color to use */
	fg = html->html.body_fg;
	/* background color to use */
	bg = html->html.body_bg;

	/* Initialize foreground color stack */
	fg_color_stack = &fg_color_base;
	fg_color_stack->color = fg;
	fg_color_stack->next = NULL;

	/* Initialize background color stack */
	bg_color_stack = &bg_color_base;
	bg_color_stack->color = fg;
	bg_color_stack->next = NULL;

	/* move to the next element */
	temp = temp->next;

 	/*
	* Insert a dummy element at the head of the list to prevent incorrect
	* handling of the first real element to be rendered.
	* fix 12/14/97-01, kdh
	*/
	element = NewTableElement(temp);
	element->object_type = OBJ_NONE;
	element->font = html->html.default_font;
	InsertTableElement(html, element, False);

	/*
	* Only elements between <BODY></BODY> elements are really interesting.
	* BUT: if the HTML verification/reparation routines in parse.c should
	* fail, we might have a premature </body> element, so we don't check on
	* it but walk thru every item found.
	*/
	while(temp != NULL)
	{
		/* create new element */
		if(!ignore)
			element = NewTableElement(temp);
		else
		{
			/*****
			* reuse current element if it wasn't needed on the previous
			* pass.
			* fix 11/12/97-01, kdh
			*****/
			(void)memset(element, 0, sizeof(XmHTMLObjectTable));
			/* fill in appropriate fields */
			element->object = temp;
		}

		/* Initialize all fields changed in here */
		text = NULL;
		ignore = False;
		object_type = OBJ_NONE;
		n_words = 0;
		width = height = 0;
		words = NULL;
		linefeed = CLEAR_NONE;
		line_data = NO_LINE;
		text_data = TEXT_SPACE_NONE;

		_XmHTMLDebug(2, ("format.c, _XmHTMLformatObjects, object data:\n"
			"\telement id: %s\n\tattributes: %s\n\tis_end: %s\n",
			html_tokens[temp->id], 
			temp->attributes ? temp->attributes : "<none>",
			temp->is_end ? "Yes" : "No"));

		switch(temp->id)
		{
			/* plain text */
			case HT_ZTEXT:
				object_type = OBJ_TEXT;
				/*
				* text_data gets completely reset in CopyText.
				* We do not want escape expansion if we are loading a plain
				* text document.
				*/
				if((text = CopyText(html, temp->element, in_pre, &text_data,
					html->html.mime_id != XmHTML_NONE)) == NULL)
				{
					/***** 
					* named anchors can be empty, so keep them by inserting
					* a dummy word, but only do that once (prevents a margin
					* reset as well).
					*****/
					if((element_data & ELE_ANCHOR_INTERN) && !anchor_data_used)
					{
						words = MakeDummyWord(&height, font, line_data,
									element);
						n_words = 1;
					}
					else
						ignore = True; /* ignore empty text fields */
					break;
				}
				if(!in_pre)
				{
					CollapseWhiteSpace(text);
					/*****
					* If this turns out to be an empty block, ignore it,
					* but only if it's not an empty named anchor.
					*****/
					if(strlen(text) == 0)
					{
						if((element_data & ELE_ANCHOR_INTERN) &&
							!anchor_data_used)
						{
							object_type = OBJ_NONE;
							words = MakeDummyWord(&height, font, line_data,
										element);
							n_words = 1;
						}
						else
							ignore = True;
						free(text);
						break;
					}
					/* check line data */
					CHECK_LINE;
					/* convert text to a number of words */
					words = TextToWords(text, &n_words, &height, font, 
						line_data, text_data, element);

					/* Plain text does a hard reset */
					linefeed = CheckLineFeed(CLEAR_NONE, True);
				}
				else
				{
					object_type = OBJ_PRE_TEXT;
					/* check line data */
					CHECK_LINE;
					/* convert text to a number of words, keep formatting. */
					words = TextToPre(text, &n_words, font, line_data, element);

					/* Preformatted text does a soft reset */
					linefeed = CheckLineFeed(CLEAR_NONE, False);
				}
				break;

			/* images */
			case HT_IMG:
				if((words = ImageToWord(html, temp->attributes, &n_words,
					&height, element, in_pre)) == NULL)
				{
					ignore = True;
					break;
				}
				text_data |= TEXT_IMAGE;
				object_type = in_pre ? OBJ_PRE_TEXT : OBJ_TEXT;
				/* No explicit returns for images, reset */
				linefeed = CheckLineFeed(CLEAR_NONE, False);
				break;

			/* anchors */
			case HT_A:
				if(temp->is_end)
				{
					/*
					* this is a very sneaky hack: since empty named anchors
					* are allowed, we must store it somehow. And this is how
					* we do it: insert a dummy word (to prevent margin reset)
					* and back up one element.
					*/
					if(!anchor_data_used && (element_data & ELE_ANCHOR_INTERN))
					{
						_XmHTMLDebug(2, ("format.c: _XmHTMLformatObjects, "
							"adding bogus named anchor %s\n", 
							anchor_data->name));

						/* insert a dummy word to prevent margin reset */
						words = MakeDummyWord(&height, font, line_data,
									element);
						n_words = 1;
						object_type = OBJ_TEXT;

						anchor_data_used = True;
						temp = temp->prev;
						break;
					}
					/* unset anchor bitfields */
					element_data &= ( ~ELE_ANCHOR & ~ELE_ANCHOR_TARGET & 
							~ELE_ANCHOR_VISITED & ~ELE_ANCHOR_INTERN);
					fg = PopFGColor();
					bg = html->html.body_bg;
					anchor_data = NULL;
					ignore = True;	/* only need anchor data */
					_XmHTMLDebug(2,("format.c: _XmHTMLformatObjects: anchor "
						"end\n"));
				}
				else
				{
					/* allocate a new anchor */
					anchor_data = _XmHTMLNewAnchor(html, temp);

					/* sanity check */
					if(!anchor_data)
					{
						ignore = True;
						break;
					}
					/* save current color */
					PushFGColor(fg);

					new_anchors++;
					anchor_data_used = False;

					/* set proper element bits */

					/* maybe it's a named one */
					if(anchor_data->name)
						element_data |= ELE_ANCHOR_INTERN;

					/*
					* maybe it's also a plain anchor. If so, see what 
					* foreground color we have to use to render this
					* anchor.
					*/
					if(anchor_data->href[0] != '\0')
					{
						element_data |= ELE_ANCHOR;
						fg = html->html.anchor_fg;

						/* maybe it's been visited */
						if(anchor_data->visited)
						{
							element_data |= ELE_ANCHOR_VISITED;
							fg = html->html.anchor_visited_fg;
						}
						/* maybe it's a target */
						else if(anchor_data->target)
						{
							element_data |= ELE_ANCHOR_TARGET;
							fg = html->html.anchor_target_fg;
						}
					}
					_XmHTMLDebug(2,("format.c: _XmHTMLformatObjects: anchor "
						"start\n"));
					ignore = True;	/* only need anchor data */
				}
				break;

			/* font changes */
			case HT_CITE:		/* italic */
			case HT_I:			/* italic */
			case HT_EM:			/* italic */
			case HT_DFN:		/* italic */
			case HT_STRONG:		/* bold */
			case HT_B:			/* bold */
			case HT_SAMP:		/* fixed width */
			case HT_TT:			/* fixed width */
			case HT_VAR:		/* fixed width */
			case HT_CODE:		/* fixed width */
			case HT_KBD:		/* fixed width */
				if(temp->is_end)
				{
					if(html->html.allow_color_switching)
						fg = PopFGColor();
					font = PopFont(&basefont);
				}
				else
				{
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, temp->id, basefont, font);
				}
				ignore = True; /* only need font data */
				break;

			case HT_U:
				if(temp->is_end) /* unset underline bitfields */
					element_data &= (~ELE_UNDERLINE & ~ELE_UNDERLINE_TEXT);
				else
					element_data |= ELE_UNDERLINE;
				ignore = True; /* only need underline data */
				break;

			case HT_STRIKE:
				if(temp->is_end) /* unset strikeout bitfields */
					element_data &= (~ELE_STRIKEOUT & ~ELE_STRIKEOUT_TEXT);
				else
					element_data |= ELE_STRIKEOUT;
				ignore = True; /* only need strikeout data */
				break;

			case HT_BASEFONT:
				{
					basefont = _XmHTMLTagGetNumber(temp->attributes, "size", 0);
					/* take absolute value */
					basefont = Abs(basefont);
					if(basefont < 1 || basefont > 7)
					{
						if(html->html.bad_html_warnings)
							_XmHTMLWarning(__WFUNC__(html, func),
								"Invalid basefont size %i at line %i of "
								"input.", basefont, temp->line);
						basefont = 4;
					}
				}
				ignore = True;	/* only need font data */
				break;

			/*****
			* <font> is a bit performance hit. We always need to push & pop
			* the font *even* if only the font color has been changed as we
			* can't keep track of what has actually been changed.
			*****/
			case HT_FONT:
				if(temp->is_end)
				{
					if(html->html.allow_font_switching)
						font = PopFont(&basefont);
					if(html->html.allow_color_switching)
						fg = PopFGColor();
				}
				else
				{
					char *chPtr;
					int size = xmhtml_basefont_sizes[basefont - 1];

					if(html->html.allow_color_switching)
						PUSH_COLOR(html);

					if(html->html.allow_font_switching)
						PushFont(font, basefont);
					else
						break;

					/* can't use TagGetNumber: fontchange can be relative */
					chPtr = _XmHTMLTagGetValue(temp->attributes, "size");
					if(chPtr != NULL)
					{
						int f_inc = atoi(chPtr);

						/* check wether size is relative or not */
						if(chPtr[0] == '-' || chPtr[0] == '+')
						{
							f_inc = basefont + f_inc; /* + 1; */
						}
						/* sanity check */
						if(f_inc < 1 || f_inc > 7)
						{
							if(f_inc < 1)
								f_inc = 1;
							else
								f_inc = 7;
						}

						basefont = f_inc;
						/* minus one: zero based array */
						size = xmhtml_basefont_sizes[f_inc-1];
						free(chPtr); /* fix 01/28/98-02, kdh */
						chPtr = NULL;
					}
					/*****
					* Font face changes only allowed when not in preformatted
					* text.
					* Only check when not being pedantic.
					*****/
#ifndef PEDANTIC
					if(!in_pre)
#endif
						chPtr = _XmHTMLTagGetValue(temp->attributes, "face");

					if(chPtr != NULL)
					{
#ifdef PEDANTIC
						if(in_pre)
						{
							_XmHTMLWarning(__WFUNC__(html, func),
								"<FONT FACE=\"%s\"> not allowed inside <PRE>,"
								" ignored.\n    (line %i in input)", chPtr,
								temp->line);
							/*****
							* Ignore face but must allow for size change.
							* (Font stack will get unbalanced otherwise!)
							*****/
							font = _XmHTMLLoadFont(html, HT_FONT, size, font);
						}
						else
#endif
							font = _XmHTMLLoadFontWithFace(html, size, chPtr,
										font);

						free(chPtr);
					}
					else
						font = _XmHTMLLoadFont(html, HT_FONT, size, font);
				}
				ignore = True; /* only need font data */
				break;

			case HT_BIG:
				if(temp->is_end)
					font = PopFont(&basefont);
				else /* multiple big elements are not honoured */
				{
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, HT_FONT,
								xmhtml_basefont_sizes[4], font);
				}
				ignore = True; /* only need font data */
				break;

			case HT_SMALL:
				if(temp->is_end)
					font = PopFont(&basefont);
				else /* multiple small elements are not honoured */
				{
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, HT_FONT,
								xmhtml_basefont_sizes[2], font);
				}
				ignore = True; /* only need font data */
				break;

			case HT_SUB:
			case HT_SUP:
				if(temp->is_end)
				{
					/* restore vertical offset */
					y_offset = 0;
					x_offset = 0;
					font = PopFont(&basefont);
				}
				else /* multiple small elements are not honoured */
				{
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, HT_FONT,
								xmhtml_basefont_sizes[2], font);
					y_offset = (temp->id == HT_SUB ? 
						font->sub_yoffset : font->sup_yoffset);
					x_offset = (temp->id == HT_SUB ? 
						font->sub_xoffset : font->sup_xoffset);
				}
				ignore = True; /* only need font data */
				break;

			case HT_H1:
			case HT_H2:
			case HT_H3:
			case HT_H4:
			case HT_H5:
			case HT_H6:
				if(temp->is_end)
				{
					if(html->html.allow_color_switching)
						fg = PopFGColor();
					halign = PopAlignment();
					font = PopFont(&basefont);
				}
				else
				{
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);

					PushAlignment(halign);
					halign = _XmHTMLGetHorizontalAlignment(temp->attributes,
						halign);
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, temp->id, basefont, font);
					/*
					* Need to update basefont size as well so font face changes
					* *inside* these elements use the correct font size as
					* well.
					* The sizes used by the headers are in reverse order.
					*/
					basefont = (int)(HT_H6 - temp->id) + 1;
				}
				linefeed = CheckLineFeed(CLEAR_HARD, False);
				object_type = OBJ_BLOCK;
				break;

			/* lists. The COMPACT tag is ignored */
			case HT_UL:
			case HT_DIR:
			case HT_MENU:
				if(temp->is_end)
				{

					ul_level--;
					ident_level--;
					if(ident_level < 0)
					{
						if(html->html.bad_html_warnings)
							_XmHTMLWarning(__WFUNC__(html, func),
								"Negative indentation at line %i of input. "
								"Check your document.", temp->line);
						ident_level = 0;
					}
					current_list = (ident_level ? ident_level - 1 : 0);
				}
				else
				{
					int mark_id;
					/* avoid overflow of mark id array */
					mark_id = ul_level % UL_ARRAYSIZE;

					/* set default marker & list start */
					list_stack[ident_level].marker = ul_markers[mark_id].type;
					list_stack[ident_level].level = 0;
					list_stack[ident_level].type = temp->id;

					if(ident_level == MAX_NESTED_LISTS)
					{
  						_XmHTMLWarning(__WFUNC__(html, func), "Exceeding"
							" maximum nested list depth for nested lists (%i) "
							"at line %i of input.", MAX_NESTED_LISTS,
							temp->line);
						ident_level = MAX_NESTED_LISTS-1;
					}
					current_list = ident_level;

					if(temp->id == HT_UL)
					{
						char *chPtr;
						/* check if user specified a custom marker */
						chPtr = _XmHTMLTagGetValue(temp->attributes, "type");
						if(chPtr != NULL)
						{
							/*
							* Walk thru the list of possible markers. If a 
							* match is found, store it so we can switch back
							* to the correct marker once this list terminates.
							*/
							for(i = 0 ; i < UL_ARRAYSIZE; i++)
							{
								if(!(strcasecmp(ul_markers[i].name, chPtr)))
								{
									list_stack[ident_level].marker = 
										ul_markers[i].type;
									break;
								}
							}
							free(chPtr);
						}
					}
					ul_level++;
					ident_level++;
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			case HT_OL:
				if(temp->is_end)
				{
					/* must be reset properly, only possible for <ol> lists. */
					list_stack[current_list].isindex = False;

					ol_level--;
					ident_level--;
					if(ident_level < 0)
					{
						if(html->html.bad_html_warnings)
							_XmHTMLWarning(__WFUNC__(html, func),
								"Negative indentation at line %i of input. "
								"Check your document.", temp->line);
						ident_level = 0;
					}
					current_list = (ident_level ? ident_level - 1 : 0);
				}
				else
				{
					int mark_id;
					char *chPtr;

					/* avoid overflow of mark id array */
					mark_id = ol_level % OL_ARRAYSIZE;

					/* set default marker & list start */
					list_stack[ident_level].marker = ol_markers[mark_id].type;
					list_stack[ident_level].level = 0;
					list_stack[ident_level].type = temp->id;

					if(ident_level == MAX_NESTED_LISTS)
					{
  						_XmHTMLWarning(__WFUNC__(html, func), "Exceeding"
							" maximum nested list depth for nested lists (%i) "
							"at line %i of input.", MAX_NESTED_LISTS,
							temp->line);
						ident_level = MAX_NESTED_LISTS-1;
					}
					current_list = ident_level;

					/* check if user specified a custom marker */
					chPtr = _XmHTMLTagGetValue(temp->attributes, "type");
					if(chPtr != NULL)
					{
						/*
						* Walk thru the list of possible markers. If a 
						* match is found, store it so we can switch back
						* to the correct marker once this list terminates.
						*/
						for(i = 0 ; i < OL_ARRAYSIZE; i++)
						{
							if(!(strcmp(ol_markers[i].name, chPtr)))
							{
								list_stack[ident_level].marker = 
									ol_markers[i].type;
								break;
							}
						}
						free(chPtr);
					}

					/* see if a start tag exists */
					if(_XmHTMLTagCheck(temp->attributes, "start"))
					{
						/* pick up a start spec */
						list_stack[ident_level].level = 
							_XmHTMLTagGetNumber(temp->attributes, "start", 0);
						list_stack[ident_level].level--;
					}

					/* see if we have to propage the current index number */
					list_stack[ident_level].isindex =
						_XmHTMLTagCheck(temp->attributes, "isindex");
					ol_level++;
					ident_level++;
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			case HT_DL:
				if(temp->is_end)
					ident_level--;
				else
					ident_level++;
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			case HT_LI:
				if(temp->is_end)	/* optional termination */
					object_type = OBJ_BLOCK;
				else
				{
					char *chPtr;

					/* increase list counter */
					list_stack[current_list].level++;

					/* check if user specified a custom marker */
					chPtr = _XmHTMLTagGetValue(temp->attributes, "type");
					if(chPtr != NULL)
					{
						/*
						* depending on current list type, check and set
						* the marker.
						*/
						if(list_stack[current_list].type == HT_OL)
						{
							for(i = 0 ; i < OL_ARRAYSIZE; i++)
							{
								if(!(strcmp(ol_markers[i].name, chPtr)))
								{
									list_stack[current_list].marker = 
										ol_markers[i].type;
									break;
								}
							}
						}
						else if(list_stack[current_list].type == HT_UL)
						{
							for(i = 0 ; i < UL_ARRAYSIZE; i++)
							{
								if(!(strcmp(ul_markers[i].name, chPtr)))
								{
									list_stack[current_list].marker = 
										ul_markers[i].type;
									break;
								}
							}
						}
						free(chPtr);
					}
					/* check if user specified a custom number for ol lists */
					if(list_stack[current_list].type == HT_OL && 
						_XmHTMLTagCheck(temp->attributes, "value"))
					{
						list_stack[current_list].level = 
							_XmHTMLTagGetNumber(temp->attributes, "value", 0);
					}
					/*
					* If the current list is an index, create a prefix for
					* the current item
					*/
					if(list_stack[current_list].isindex)
					{
						words = indexToWord(html, list_stack, current_list,
									element, in_pre);
						n_words = 1;
					}
					object_type = OBJ_BULLET;
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				break;

			case HT_DT:
				if(temp->is_end)
					in_dt--;
				else
					in_dt++;
			case HT_DD:
				object_type = OBJ_BLOCK;
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				break;

			/* block commands */
			case HT_ADDRESS:
				if(temp->is_end)
				{
					if(html->html.allow_color_switching)
						fg = PopFGColor();
					font = PopFont(&basefont);
				}
				else
				{
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, temp->id, basefont, font);
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			/* <BR> is a special word */
			case HT_BR:
#if 0
				{
					String chPtr;
					
					if((chPtr = _XmHTMLTagGetValue(temp->attributes,
						"clear")) != NULL && locase(chPtr[0]) != 'n')
					{
						/* all clear attribs but none reset the linefeeder */
						(void)CheckLineFeed(CLEAR_HARD, True);
						linefeed = CLEAR_ALL;

						if(locase(chPtr[0]) == 'l')	/* clear = left */
							halign = XmHALIGN_LEFT;
						else if(locase(chPtr[0]) == 'r')	/* clear = right */
							halign = XmHALIGN_RIGHT;
						/* no default */

						free(chPtr);
					}
					else /* fix 01/20/97-02, kdh */
						linefeed = CheckLineFeed(CLEAR_SOFT, False);
				}
#endif
				if((linefeed = CheckLineFeed(CLEAR_SOFT, False)) != CLEAR_NONE)
				{
					words = BreakToWord(&height, font, linefeed, element); 
					n_words = 1;
					object_type = OBJ_TEXT;
					text_data |= TEXT_BREAK;	/* it's a linebreak */
				}
				else
				{
					/* linebreak follows a stronger linebreak, ignore */
					object_type = OBJ_NONE;
					ignore = True;
				}

				break;

			case HT_TAB:
				{
					char *chPtr;

					object_type = OBJ_TEXT;

					element->len = 8; /* default tabsize */

					/* see if we have a width spec */
					chPtr = _XmHTMLTagGetValue(temp->attributes, "size");
					if(chPtr != NULL)
					{
						element->len = atoi(chPtr);
						free(chPtr);
					}
					n_words = 1;
					words = SetTab(element->len, &height, font, element);
				}
				break;
				
			case HT_PRE:
				if(temp->is_end)
				{
					if(html->html.allow_color_switching)
						fg = PopFGColor();
					font = PopFont(&basefont);
					in_pre = False;
				}
				else
				{
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);
					PushFont(font, basefont);
					font = _XmHTMLLoadFont(html, temp->id, basefont, font);
					in_pre = True;
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			case HT_BLOCKQUOTE:
				if(temp->is_end)
				{
					ident_level--;
					if(html->html.allow_color_switching)
						fg = PopFGColor();
				}
				else
				{
					ident_level++;
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			/*****
			* <P> and <DIV> are equal all except for the amount of
			* vertical whitespace added: <P> causes a hard linebreak whereas
			* <DIV> causes a soft linebreak.
			*****/
			case HT_P:
			case HT_DIV:
				if(temp->is_end)
				{
					halign = PopAlignment();
					/*
					* Paragraph ending also adds linespacing (natural flow
					* of text between paragraphs).
					*/
					linefeed = CheckLineFeed(
						(temp->id == HT_P ? CLEAR_HARD : CLEAR_SOFT), False);
					
					/* do we have a color attrib? */
					if(html->html.allow_color_switching)
						fg = PopFGColor();
				}
				else
				{
					PushAlignment(halign);
					halign = _XmHTMLGetHorizontalAlignment(temp->attributes,
						halign);
					linefeed = CheckLineFeed(
						(temp->id == HT_P ? CLEAR_HARD : CLEAR_SOFT), False);
					/* do we have a color attrib? */
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);
				}
				object_type = OBJ_BLOCK;
				break;

			case HT_CENTER:
				if(temp->is_end)
				{
					halign = PopAlignment();
					/* do we have a color attrib? */
					if(html->html.allow_color_switching)
						fg = PopFGColor();
				}
				else
				{
					PushAlignment(halign);
					halign = XmHALIGN_CENTER;
					/* do we have a color attrib? */
					if(html->html.allow_color_switching)
						PUSH_COLOR(html);
				}
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_BLOCK;
				break;

			case HT_HR:
				{
					/* 
					* horizontal rules don't have an ending counterpart,
					* so the alignment is never pushed. If we should do that, 
					* we would get an unbalanced stack.
					*/
					element->halign =
						_XmHTMLGetHorizontalAlignment(temp->attributes,
							halign);
					/* see if we have a width spec */
					element->len = _XmHTMLTagCheckNumber(temp->attributes,
										"width", 0);

					/* check height */
					height = _XmHTMLTagGetNumber(temp->attributes, "size", 0);
					/* sanity check */
					if(height <= 0 )
						height = 2;
					/* y_offset is used as a flag for the NOSHADE attr. */
					element->y_offset = (int)_XmHTMLTagCheck(temp->attributes, 
						"noshade");

					/* do we have a color attrib? */
					if(html->html.allow_color_switching &&
						!html->html.strict_checking)
						PUSH_COLOR(html);
				}
				/* horizontal rules always have a soft return */
				linefeed = CheckLineFeed(CLEAR_SOFT, False);
				object_type = OBJ_HRULE;
				break;

			/* forms */
			case HT_FORM:
				if(temp->is_end)
					_XmHTMLEndForm(html);
				else
					_XmHTMLStartForm(html, temp->attributes);

				/* only need form data */
				ignore = True;
				break;

			case HT_SELECT:
				/* this form component can only contain option tags */
				if((words = SelectToWord(html, temp, &n_words, &width, &height,
					element, in_pre)) == NULL)
				{
					ignore = True;
					break;
				}
				/* walk to the end of this select */
				temp = temp->next;
				for(; temp != NULL && temp->id != HT_SELECT;
					temp = temp->next);

				text_data |= TEXT_FORM;
				object_type = in_pre ? OBJ_PRE_TEXT : OBJ_TEXT;
				break;

			/*****
			* It's an error if we get this, SelectToWord deals with these
			* tags.
			*****/
			case HT_OPTION:
				if(!temp->is_end)
				{
					if(html->html.bad_html_warnings)
						_XmHTMLWarning(__WFUNC__(html, func), 
							"Bad <OPTION> tag: outside a <SELECT> tag, "
							"ignoring (line %i in input).", temp->line);
				}
				ignore = True;
				break;

			case HT_TEXTAREA:
				if((words = TextAreaToWord(html, temp, &n_words, &width,
					&height, element, in_pre)) == NULL)
				{
					ignore = True;
					break;
				}
				/*
				* Walk to the end of this textarea. If there was any text
				* provided, we've already picked it up.
				*/
				temp = temp->next;
				for(; temp != NULL && temp->id != HT_TEXTAREA;
					temp = temp->next);

				text_data |= TEXT_FORM;
				object_type = in_pre ? OBJ_PRE_TEXT : OBJ_TEXT;
				break;

			case HT_INPUT:
				if((words = InputToWord(html, temp->attributes, &n_words,
					&width, &height, element, in_pre)) == NULL)
				{
					ignore = True;
					break;
				}
				/* type=image is promoted to a true image */
				if(words->form->type == FORM_IMAGE)
				{
					text_data |= TEXT_IMAGE;

					/* allocate a new anchor */
					if((form_anchor_data = _XmHTMLNewAnchor(html, temp))
						== NULL)
						break;

					/* promote to internal form anchor */
					form_anchor_data->url_type = ANCHOR_FORM_IMAGE;

					new_anchors++;

					/* set proper element bits, we assume it's a plain one */
					element_data |= ELE_ANCHOR;
				}
				else
				{
					text_data |= TEXT_FORM;
				}
				object_type = in_pre ? OBJ_PRE_TEXT : OBJ_TEXT;
				break;

			/* applets */
			case HT_APPLET:
				if(temp->is_end)
				{
					/*
					* INSERT CODE
					* to end this applet
					*/
				}
				else
				{
					if(html->html.bad_html_warnings)
						_XmHTMLWarning(__WFUNC__(html, func), 
							"<APPLET> element not supported yet.");
					/*
					* INSERT CODE
					* to start this applet
					*/
				}
				object_type = OBJ_APPLET;
				ignore = True;
				break;

			case HT_PARAM:		/* applet params */
				if(html->html.bad_html_warnings)
					_XmHTMLWarning(__WFUNC__(html, func), 
						"<PARAM> element not supported yet.");
				object_type = OBJ_APPLET;
				ignore = True;
				break;

			case HT_MAP:
				if(temp->is_end)
				{
					_XmHTMLStoreImagemap(html, imageMap);
					imageMap = NULL;
				}
				else
				{
					String chPtr;

					chPtr = _XmHTMLTagGetValue(temp->attributes, "name");
					if(chPtr != NULL)
					{
						imageMap = _XmHTMLCreateImagemap(chPtr);
						free(chPtr);
					}
					else if(html->html.bad_html_warnings)
						_XmHTMLWarning(__WFUNC__(html, func), "unnamed "
							"map, ignored (line %i in input).", temp->line);
				}
				ignore = True;	/* only need imagemap name */
				break;

			case HT_AREA:
				if(imageMap)
					_XmHTMLAddAreaToMap(html, imageMap, temp);
				else if(html->html.bad_html_warnings)
					_XmHTMLWarning(__WFUNC__(html, func), "<AREA> "
						"element outside <MAP>, ignored (line %i in input).",
						temp->line);
				ignore = True;	/* only need area data */
				break;

			/* tables */
			case HT_TABLE:
				if(temp->is_end)
				{
					/* wrapup current table */
					table = tableClose(html, table, element);

					halign = PopAlignment();
					bg = PopBGColor();
					object_type = OBJ_NONE;
				}
				else
				{
					/* table start always causes a linebreak */
					linefeed = CheckLineFeed(CLEAR_SOFT, False);

					PushAlignment(halign);
					PushBGColor(bg);

					/*****
					* Open a new table. Returns a parent or a child table.
					*****/
					table = tableOpen(html, table, element, temp, &halign, &bg);

					/* new master table, insert */
					if(table->parent == NULL)
					{
						/* insert this table in the list of tables */
						if(html->html.tables)
						{
							current_table->next = table;
							current_table = table;
						}
						else
						{
							html->html.tables = table;
							current_table = table;
						}
					}
					object_type = OBJ_TABLE;
				}
				break;

			case HT_CAPTION:		/* table caption */
				if(temp->is_end)
				{
					/* close the caption */
					tableCloseCaption(html, table, element);

					halign = PopAlignment();
					bg = PopBGColor();
					object_type = OBJ_NONE;
				}
				else
				{
					PushAlignment(halign);
					PushBGColor(bg);

					/* captions are always centered */
					halign = XmHALIGN_CENTER;

					/* open the caption */
					tableOpenCaption(html, table, element, temp, &bg);

					object_type = OBJ_TABLE_FRAME;
				}
				break;

			case HT_TR:		/* table row */
				if(temp->is_end)	/* optional termination */
				{
					/* close current row */
					tableCloseRow(html, table, element);

					halign = PopAlignment();
					bg = PopBGColor();
					object_type = OBJ_NONE;
				}
				else
				{
					/* open a row */
					PushAlignment(halign);
					PushBGColor(bg);

					tableOpenRow(html, table, element, temp, &halign, &bg);

					object_type = OBJ_TABLE_FRAME;
				}
				break;

			case HT_TH:		/* header cell */
			case HT_TD:		/* regular cell */
				if(temp->is_end)	/* optional termination */
				{
					/* header cell used a bold font, restore */
					if(temp->id == HT_TH)
						font = PopFont(&basefont);

					/* close current cell */
					tableCloseCell(html, table, element);

					halign = PopAlignment();
					bg = PopBGColor();

					object_type = OBJ_NONE;
				}
				else
				{
					/* header cell uses a bold font */
					if(temp->id == HT_TH)
					{
						PushFont(font, basefont);
						font = _XmHTMLLoadFont(html, HT_B, basefont, font);
					}

					PushAlignment(halign);
					PushBGColor(bg);

					/* open a cell */
					tableOpenCell(html, table, element, temp, &halign, &bg);

					/* table cell always resets linefeeding */
					(void)CheckLineFeed(CLEAR_SOFT, True);

					object_type = OBJ_TABLE_FRAME;
				}
				break;

			/*****
			* According to HTML3.2, the following elements may not occur
			* inside the body content, but a *lot* of HTML documents are
			* in direct violation with this and the parser isn't always
			* successfully in removing them. So we need to handle these
			* elements as well and skip all data between the opening and
			* closing element.
			*****/
			case HT_STYLE:
			case HT_SCRIPT:
				{
					htmlEnum end_id = temp->id;
					/* move past element */
					temp = temp->next;
					/* skip it entirely */
					for(; temp != NULL; temp = temp->next)
						if(temp->id == end_id && temp->is_end)
							break;
					ignore = True;
				}
				break;

			default:
				_XmHTMLDebug(2, ("format.c: _XmHTMLformatObjects; "
					"Unused element %s.\n", temp->element));
				ignore = True;
		}
		if(!ignore)
		{
			/* adjust anchor count */
			if(element_data & ELE_ANCHOR)
			{
				text_data |= TEXT_ANCHOR;
				anchor_words += n_words; 
				anchor_data_used = True;
			}
			/* mark object as internal anchor */
			if(element_data & ELE_ANCHOR_INTERN)
			{
				text_data |= TEXT_ANCHOR_INTERN;
				named_anchors++;
				anchor_data_used = True;
			}
			element->text = text;
			element->text_data = text_data;
			element->words = words;
			element->n_words = n_words;
			element->width = width;
			element->height = height;
			element->fg = fg;
			element->bg = bg;
			element->font = font;
			element->marker = list_stack[current_list].marker;
			element->list_level = list_stack[current_list].level;
			element->table = table;
			/* 
			* <dt> elements have an identation one less than the current.
			* all identation must use the default font (consistency).
			*/
			if(in_dt && ident_level)
				element->ident = (ident_level-1) * IDENT_SPACES * 
					html->html.default_font->width;
			else
				element->ident = ident_level * IDENT_SPACES * 
					html->html.default_font->width;

			element->linefeed = (int)((1+linefeed)*font->lineheight);

			/* stupid hack so HT_HR won't mess up alignment and color stack */
			if(temp->id != HT_HR)
			{
				element->halign = halign;
				element->y_offset = y_offset;
				element->x_offset = x_offset;
			}
			else if(html->html.allow_color_switching &&
						!html->html.strict_checking)
				fg = PopFGColor();

			element->object_type = object_type;

			if(object_type == OBJ_BULLET)
				FillBullet(html, element);

			/*****
			* If we have a form component of type <input type="image">, we
			* have promoted it to an anchor. Set this anchor data as the
			* anchor for this element and, as it is used only once, reset
			* it to NULL. In all other case we have a plain anchor.
			*
			* Note: as form components are allowed inside anchors, this is
			* the only place in which we can possibly have nested anchors.
			* This is a problem we will have to live with...
			*****/
			if(form_anchor_data)
			{
				element->anchor = form_anchor_data;
				form_anchor_data = NULL;
				element_data &= ~ELE_ANCHOR;
			}
			else
				element->anchor = anchor_data;

			/* add an anchor id if this data belongs to a named anchor */
			if(element_data & ELE_ANCHOR_INTERN) 
				element->id = named_anchors;

			InsertTableElement(html, element, element_data & ELE_ANCHOR);
			previous_element = element;
		}
		/* element will be reused if it was ignored */
#ifdef DEBUG
		else 
			num_ignore++;
#endif

		/* move to next element */
		temp = temp->next;
	}
	/* if there still is an open table, close it now */
	if(table)
		tableClose(html, table, element);

	/*****
	* Insert a dummy element at the end of the list, saves some NULL tests
	* in the layout routines.
	*****/
	if(ignore)
	{
		/* reuse ignored element */
		memset(element, 0, sizeof(XmHTMLObjectTable));
		element->object_type = OBJ_NONE;
	}
	else
		element = NewTableElement(NULL);
	InsertTableElement(html, element, False);

	/*
	* Some sucker forget to terminate a list and parser failed to repair it.
	* Spit out a warning.
	*/
	if(html->html.bad_html_warnings && ident_level != 0)
		_XmHTMLWarning(__WFUNC__(html, func), "non-zero indentation at "
			"end of input. Check your document.");

	/* clear foreground colorstack */
	if(fg_color_stack->next)
	{
		int i=0;
		while(fg_color_stack->next)
		{
			(void)PopFGColor();
			i++;
		}
		_XmHTMLDebug(2, ("unbalanced foreground color stack (%i colors "
			"remained).\n", i));
	}
	/* clear background colorstack */
	if(bg_color_stack->next)
	{
		int i=0;
		while(bg_color_stack->next)
		{
			(void)PopBGColor();
			i++;
		}
		_XmHTMLDebug(2, ("unbalanced background color stack (%i colors "
			"remained).\n", i));
	}

	/* clear alignment stack */
	if(align_stack->next)
	{
		int i = 0;
		while(align_stack->next)
		{
			(void)PopAlignment();
			i++;
		}
		_XmHTMLDebug(2, ("unbalanced alignment stack (%i alignments "
			"remain).\n", i));
	}

	/* clear font stack */
	if(font_stack->next)
	{
		int i = 0;
		while(font_stack->next)
		{
			(void)PopFont(&basefont);
			i++;
		}
		_XmHTMLDebug(2, ("unbalanced font stack (%i fonts remain).\n", i));
	}

	/* 
	* allocate memory for all anchor words in this document, gets filled in
	* paint.c
	*/
	if(anchor_words)
	{
		html->html.anchors= (XmHTMLWord*)calloc(anchor_words+1,
			sizeof(XmHTMLWord));

		_XmHTMLDebug(2,("_XmHTMLFormatObjects: anchors contain %i words\n",
			anchor_words));
		html->html.anchor_words = anchor_words;
	}
	html->html.num_anchors = list_data.num_anchors;

	/* allocated memory for all named anchors. Gets filled in paint.c */
	if(named_anchors)
	{
		html->html.named_anchors = 
			(XmHTMLObjectTable*)calloc(named_anchors+1, 
				sizeof(XmHTMLObjectTable));

		html->html.num_named_anchors = named_anchors;
	}

	_XmHTMLDebug(2,("_XmHTMLFormatObjects: formatted %li elements of which %li"
		" anchors.\n", list_data.num_elements, list_data.num_anchors));
	_XmHTMLDebug(2,("_XmHTMLFormatObjects: found %i named anchors.\n",
		named_anchors));
	_XmHTMLDebug(2, ("_XmHTMLformatObjects, allocated %i elements and "
		"ignored %i objects.\n", allocated, num_ignore));

	_XmHTMLDebug(2, ("_XmHTMLformatObjects, allocated %i XmHTMLAnchor "
		"objects\n", new_anchors));

	/* store the anchor list */
	html->html.anchor_data = list_data.anchor_head;

	/* Since the table head is a dummy element, we return the next one */
	list_data.current = list_data.head->next;

	/* this is *required* */
	if(list_data.current)
		list_data.current->prev = NULL;

	/* free top of the list */
	free(list_data.head);
	list_data.head = NULL;

	/* store it */
	html->html.formatted = list_data.current;

	/* all done! */
}

/********
****** Public XmHTML Functions
********/

/*****
* Name: 		XmHTMLGetURLType
* Return Type: 	URLType
* Description: 	tries to figure out what type of url the given href is
* In: 
*	href:		url specification
* Returns:
*	type of url when we know it, ANCHOR_UNKNOWN otherwise.
* Note:
*	this routine is quite forgiving on typos of any url spec: only the
*	first character is checked, the remainder doesn't matter.
*****/
URLType
XmHTMLGetURLType(String href)
{
	char *chPtr;

	if(href == NULL || *href == '\0')
		return(ANCHOR_UNKNOWN);

	_XmHTMLDebug(2, ("format.c: XmHTMLGetURLType; checking url type of %s\n",
		href));

	/* first pick up any leading url spec */
	if((chPtr = strstr(href, ":")) != NULL)
	{
		/* check for URL types we know of. Do in most logical order(?) */
		if(!strncasecmp(href, "https", 5))	/* must be before http */
			return(ANCHOR_SECURE_HTTP);
		if(!strncasecmp(href, "http", 4))
			return(ANCHOR_HTTP);
		if(!strncasecmp(href, "mailto", 6))
			return(ANCHOR_MAILTO);
		if(!strncasecmp(href, "ftp", 3))
			return(ANCHOR_FTP);
		if(!strncasecmp(href, "file", 4))
			return(ANCHOR_FILE_REMOTE);
		if(!strncasecmp(href, "news", 4))
			return(ANCHOR_NEWS);
		if(!strncasecmp(href, "telnet", 6))
			return(ANCHOR_TELNET);
		if(!strncasecmp(href, "gopher", 6))
			return(ANCHOR_GOPHER);
		if(!strncasecmp(href, "wais", 4))
			return(ANCHOR_WAIS);
		if(!strncasecmp(href, "exec", 4) ||
			!strncasecmp(href, "xexec", 5))
			return(ANCHOR_EXEC);
		if(!strncasecmp(href, "pipe", 4))
			return(ANCHOR_PIPE);
		if(!strncasecmp(href, "about", 4))
			return(ANCHOR_ABOUT);
		if(!strncasecmp(href, "man", 4))
			return(ANCHOR_MAN);
		if(!strncasecmp(href, "info", 4))
			return(ANCHOR_INFO);
		_XmHTMLDebug(2, ("format.c: XmHTMLGetURLType; unknown type\n"));
		return(ANCHOR_UNKNOWN);
	}
	return(href[0] == '#' ? ANCHOR_JUMP : ANCHOR_FILE_LOCAL);
}
