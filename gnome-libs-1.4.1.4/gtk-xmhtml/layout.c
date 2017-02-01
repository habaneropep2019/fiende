/*****
* layout.c : XmHTML layout computation routines
*
* This file Version	$Revision: 1.6.6.1 $
*
* Creation date:		Thu Nov  6 01:35:46 GMT+0100 1997
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
* $Log: layout.c,v $
* Revision 1.6.6.1  2001/10/20 06:52:12  kmaraas
* 2001-10-20  Kjartan Maraas  <kmaraas@gnome.org>
*
* 	* *.*: Apply all the Red Hat patches.
*
* Revision 1.6  1999/07/29 01:26:29  sopwith
*
*
* Fix all warnings.
*
* Revision 1.5  1999/06/02 01:00:42  unammx
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
* Revision 1.4  1998/02/12 03:09:18  unammx
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
* Revision 1.3  1997/12/30 03:32:52  unammx
* More work on getting the frames working, still some bits are missing - Miguel
*
* Revision 1.2  1997/12/29 22:16:31  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.1  1997/12/25 01:34:13  unammx
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
*****/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/
typedef XmHTMLWord** (*WordProc)(XmHTMLObjectTable, XmHTMLObjectTable, int*);

/*****
* Object bounding box. Used for recursive layout computations in tables
* and text flowing around images.
*****/
typedef struct _PositionBox{
	Cardinal x;					/* absolute box upper left x position	*/
	Cardinal y;					/* absolute box upper left y position	*/
	int lmargin;				/* left margin							*/
	int rmargin;				/* right margin							*/
	int tmargin;				/* top margin							*/
	int bmargin;				/* bottom margin						*/
	int width;					/* absolute box width					*/
	int height;					/* absolute box height 					*/
	int min_width;				/* minimum box width					*/
	int min_height;				/* minimum box height					*/
	int left;					/* absolute left position				*/
	int right;					/* absolute right position				*/
	int idx;					/* index of cell using this box			*/
}PositionBox;

/*** Private Function Prototype Declarations ****/
static void SetText(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement start, XmHTMLObjectTableElement end,
	Boolean in_pre, Boolean precompute);
static void SetRule(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement data);
static void SetApplet(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement data);
static void SetBlock(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement data);
static void SetBullet(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement data);
static void SetBreak(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement data);
static XmHTMLObjectTableElement SetTable(XmHTMLWidget html,
	PositionBox *box, XmHTMLObjectTableElement data);

/*****
* Layout computation routines
*****/
static void ComputeTextLayout(XmHTMLWidget html, PositionBox *box,
	XmHTMLWord **words, int nstart, int *nwords, Boolean last_line,
	Boolean precompute);
static void ComputeTextLayoutPre(XmHTMLWidget html, PositionBox *box,
	XmHTMLWord **words, int nstart, int *nwords, Boolean last_line);

/*****
* Various helper routines
*****/
static XmHTMLWord **getWords(XmHTMLObjectTableElement start,
	XmHTMLObjectTableElement end, int *nwords);

static XmHTMLWord **getWordsRtoL(XmHTMLObjectTableElement start,
	XmHTMLObjectTableElement end, int *nwords);

static void JustifyText(XmHTMLWidget html, XmHTMLWord *words[],
	int word_start, int word_end, Dimension sw, int len, int line_len,
	int skip_id);

static void CheckAlignment(XmHTMLWidget html, XmHTMLWord *words[],
	int word_start, int word_end, int sw, int line_len, Boolean last_line,
	int skip_id);

static void CheckVerticalAlignment(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement start, XmHTMLObjectTableElement end,
	Alignment valign);

static void AdjustBaseline(XmHTMLWord *base_obj, XmHTMLWord **words,
	int start, int end, int *lineheight, Boolean last_line);

static void AdjustBaselinePre(XmHTMLWord *base_obj, XmHTMLWord **words,
	int start, int end, int *lineheight, Boolean last_line);

static void PreComputeTableLayout(XmHTMLWidget html, PositionBox *parent,
	XmHTMLObjectTableElement obj_start, XmHTMLObjectTableElement obj_end);

static void ComputeTableLayout(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement obj_start, XmHTMLObjectTableElement obj_end);

/* 
* characters that must be flushed against a word. Can't use ispunct since
* that are all printable chars that are not a number or a letter.
*/
#define IS_PUNCT(c) (c == '.' || c == ',' || c == ':' || c == ';' || \
	c == '!' || c == '?')

/*** Private Variable Declarations ***/
static int line, last_text_line;
static int max_width;
static XmHTMLWord *baseline_obj;
static Boolean had_break;		/* indicates a paragraph had a break */
static XmHTMLWord** (*get_word_func)(XmHTMLObjectTableElement,
	XmHTMLObjectTableElement, int *);
static int curr_anchor, named_anchor;

#ifdef DEBUG
static int lines_done;
static int total_iterations;
#endif

#define STORE_ANCHOR(DATA) { \
	if(DATA->text_data & TEXT_ANCHOR) \
	{ \
		/* save anchor data */ \
		for(i = 0 ; i < DATA->n_words; i++) \
		{ \
			/* sanity check */ \
			if(curr_anchor == html->html.anchor_words) \
			{ \
				_XmHTMLWarning(__WFUNC__(html, "_XmHTMLpaint"), \
					"I'm about to crash: exceeding anchor word count!"); \
				curr_anchor--; \
			} \
			/* copy worddata and adjust y position */ \
			html->html.anchors[curr_anchor] = DATA->words[i]; \
			if(DATA->words[i].type == OBJ_IMG) \
				html->html.anchors[curr_anchor].y =  DATA->words[i].y; \
			else \
				html->html.anchors[curr_anchor].y =  \
					DATA->words[i].y - DATA->words[i].font->xfont->ascent; \
			curr_anchor++; \
		} \
	} \
	if(DATA->text_data & TEXT_ANCHOR_INTERN) \
	{ \
		/* save named anchor location */ \
		if(named_anchor == html->html.num_named_anchors) \
		{ \
			_XmHTMLWarning(__WFUNC__(html, "_XmHTMLpaint"), \
				"I'm about to crash: exceeding named anchor count!"); \
			named_anchor--; \
		} \
		/* copy worddata and adjust y position */ \
		html->html.named_anchors[named_anchor] = *DATA; \
		named_anchor++; \
	} \
}

/*****
* Name:			_XmHTMLComputeLayout
* Return Type:	void
* Description:	displays every formatted object on to the screen.
* In:
*	w:			Widget to display 
* Returns:
*	nothing.
*****/
void
_XmHTMLComputeLayout(XmHTMLWidget html)
{
	XmHTMLObjectTableElement temp, end;
	PositionBox box;
	int i;

	_XmHTMLDebug(5, ("layout.c: _XmHTMLComputeLayout Start\n"));

	html->html.paint_start = temp = html->html.formatted;
	html->html.paint_x = 0;
	html->html.paint_width = html->html.work_width + html->html.margin_width;

	line = last_text_line = 0;
	baseline_obj = (XmHTMLWord*)NULL;
	max_width = 0;
	had_break = False;
	curr_anchor = 0, named_anchor = 0;

	/*****
	* work_width is core width minus one horizontal margin. 
	* Maximum useable width is core width minus two times the horizontal
	* margin.
	*****/
	box.x = html->html.margin_width;
	box.y = html->html.margin_height + html->html.default_font->xfont->ascent;
	box.lmargin = html->html.margin_width;		/* absolute left margin		*/
	box.rmargin = html->html.work_width;		/* absolute right margin	*/
	box.width  = box.rmargin - box.lmargin;		/* absolute box width		*/
	box.height = -1;
	box.tmargin = html->html.margin_height;		/* top margin			*/
	box.bmargin = html->html.margin_height;		/* bottom margin		*/
	box.left  = box.lmargin;					/* initial left offset	*/
	box.right = box.rmargin;					/* initial right offset	*/

	/* select appropriate word collector */
	if(html->html.string_direction == TSTRING_DIRECTION_R_TO_L)
		get_word_func = getWordsRtoL;
	else
		get_word_func = getWords;

#ifdef DEBUG
	lines_done = 0;
	total_iterations = 0;
	_XmHTMLDebug(5, ("layout.c: _XmHTMLComputeLayout:\n"
		"\tCore offset: %ix%i\n"
		"\tmargins: width = %i, height = %i\n"
		"\twidget offset: %ix%i\n",
		Toolkit_Widget_Dim (html).x, Toolkit_Widget_Dim (html).y,
			 box.lmargin, box.tmargin, box.x, box.y));
#endif

	/* sanity check */
	if(temp == NULL)
		return;		/* fix 01/28/97-06, kdh */

	_XmHTMLFullDebug(5, ("layout.c: _XmHTMLComputeLayout, x = %d, y = %d \n",
		box.x, box.y));

	while(temp != NULL)
	{
		switch(temp->object_type)
		{
			/*
			* To get a proper text layout, we need to do the layout for
			* whole blocks of text at a time.
			*/
			case OBJ_TEXT:
				for(end = temp; end->next->object_type == OBJ_TEXT;
					end = end->next);

				/* go and do text layout */
				SetText(html, &box, temp, end->next, False, False);

				for(; temp->object_type == OBJ_TEXT; temp = temp->next)
				{
					STORE_ANCHOR(temp);
				}
				/* back up one element */
				temp = end;
				break;

			case OBJ_PRE_TEXT:
				for(end = temp; end->next->object_type == OBJ_PRE_TEXT;
					end = end->next);

				my_assert(end != NULL);

				/* go and do text layout */
				SetText(html, &box, temp, end->next, True, False);

				for(; temp->object_type == OBJ_PRE_TEXT; temp = temp->next)
				{
					STORE_ANCHOR(temp);
				}
				/* back up one element */
				temp = end;
				break;
			case OBJ_BULLET:
				SetBullet(html, &box, temp);
				break;
			case OBJ_HRULE:
				SetRule(html, &box, temp);
				break;
			case OBJ_TABLE:
				end = SetTable(html, &box, temp);

				/*****
				* Now store anchor data in this table. We can't do this
				* in the table layout routine as the (recursive) computation
				* routines for nested tables will repeatedly store anchor data.
				*****/
				for(; temp != end; temp = temp->next)
				{
					if(temp->object_type == OBJ_TEXT ||
						temp->object_type == OBJ_PRE_TEXT)
					{
						STORE_ANCHOR(temp);
					}
					/* empty named anchors can cause this */
					else if(temp->text_data & TEXT_ANCHOR_INTERN)
					{
						/* save named anchor location */
						html->html.named_anchors[named_anchor] = *temp;
						named_anchor++;
					}
				}
				SetBlock(html, &box, temp);

				/* back up one element */
				temp = end->prev;
				break;
			case OBJ_TABLE_FRAME:
#ifdef DEBUG
				_XmHTMLWarning(__WFUNC__(html, "_XmHTMLComputeLayout"),
					"Invalid object OBJ_TABLE_FRAME! (debug)\n");
#endif
				break;
			case OBJ_APPLET:
				SetApplet(html, &box, temp);
				SetBreak(html, &box, temp);
				break;
			case OBJ_BLOCK:
				SetBlock(html, &box, temp);
				SetBreak(html, &box, temp);
				break;
			case OBJ_NONE:
				SetBlock(html, &box, temp);
				/* empty named anchors can cause this */
				if(temp->text_data & TEXT_ANCHOR_INTERN)
				{
					/* save named anchor location */
					html->html.named_anchors[named_anchor] = *temp;
					named_anchor++;
				}
				break;
			default:
				_XmHTMLWarning(__WFUNC__(html, "_XmHTMLComputeLayout"), 
					"Unknown object type!");
		}
		/* end command for painting the first page */
		if((box.y - temp->height > html->html.work_height) || 
			(box.y > html->html.work_height))
			html->html.paint_end = temp;
		if(box.x > max_width)
			max_width = box.x;
		temp = temp->next;
	}
	/***** 
	* Now adjust width of the anchors.
	* If the current anchor word and the next are on the same line, and these
	* words belong to the same anchor, the width of the current anchor word 
	* is adjusted so it will seem to be continue across the whole line when
	* the mouse pointer is moved over an anchor.
	* We can adjust the width field directly because the html.anchors field is
	* only used for anchor lookup, not for rendering.
	*****/
	for(i = 0 ; i < html->html.anchor_words; i++)
			html->html.anchors[i].x = html->html.anchors[i].self->x;
	for(i = 0 ; i < html->html.anchor_words - 1; i++)
	{
		if((html->html.anchors[i].owner == html->html.anchors[i+1].owner) &&
			(html->html.anchors[i].line == html->html.anchors[i+1].line))
		{
			html->html.anchors[i].width = 
				html->html.anchors[i+1].x - html->html.anchors[i].x + 2;
		}
		my_assert(html->html.anchors[i].base != NULL);
	}
	/*****
	* store total height for this document. We add the marginheight and
	* font descent to get the text nicely centered.
	*****/
	html->html.formatted_height = box.y + box.tmargin + 
		html->html.default_font->xfont->descent; 

	/* Preferred width for this document, includes horizontal margin once. */
	html->html.formatted_width = max_width;

	/* store new maximum line number */
	html->html.nlines = line;

	/*****
	* Never adjust top_line, scroll_x or scroll_y. This will make the
	* widget jump to the line in question and start drawing at the scroll_x 
	* and scroll_y positions.
	*****/

	_XmHTMLDebug(5, ("layout.c: _XmHTMLComputeLayout, x_max = %d, "
		"y_max = %d, total lines: %i.\n", html->html.formatted_width,
		html->html.formatted_height, line));
	_XmHTMLDebug(5, ("layout.c: _XmHTMLComputeLayout, stored %i/%i "
		"anchor words\n", curr_anchor, html->html.anchor_words));
	_XmHTMLDebug(5, ("layout.c: _XmHTMLComputeLayout, stored %i/%i "
		"named anchors \n", named_anchor, html->html.num_named_anchors));

	/* now process any images with an alpha channel (if any) */
	if(html->html.delayed_creation)
		_XmHTMLImageCheckDelayedCreation(html);

#ifdef DEBUG
	/* prevent divide by zero */
	if(lines_done)
	{
		_XmHTMLDebug(5, ("outlining stats\n"));
		_XmHTMLDebug(5, ("\tlines done: %i\n", lines_done));
		_XmHTMLDebug(5, ("\ttotal iterations: %i\n", total_iterations));
		_XmHTMLDebug(5, ("\taverage iterations per line: %f\n", 
			(float)(total_iterations/(float)lines_done)));
	}
#endif

	_XmHTMLDebug(5, ("layout.c: _XmHTMLComputeLayout End\n"));
	return;
}

/*****
* Name: 		JustifyText
* Return Type: 	void
* Description: 	adjusts interword spacing to produce fully justified text.
*				justification is done on basis of the longest words.
* In: 
*	start:		starting text element
*	end:		ending text element
*	w_start:	index in starting text element
*	w_end:		index in ending text element
*	sw:			width of a space in the current font
*	len:		current line length for this text
*	line_len:	maximum length of a line.
* Returns:
*	nothing, but *items contains updated delta fields to reflect the 
*	required interword spacing.
* Note:
*	Words that start with a punctuation character are never adjusted,
*	they only get shoved to the right.
*	This routine could be much more efficient if the text to be justified 
*	would be sorted.
*****/
static void
JustifyText(XmHTMLWidget html, XmHTMLWord *words[], int word_start, 
	int word_end, Dimension sw, int len, int line_len, int skip_id)
{
	int word_len, longest_word = 0, nspace = 0, i, j, num_iter = 0;

	/* See how many spaces we have to add */
	nspace = (int)((line_len - len)/(sw == 0 ? (sw = 3) : sw));

	/* 
	* last line of a block or no spaces to add. Don't adjust it.
	* nspace can be negative if there are words that are longer than
	* the available linewidth
	*/
	if(nspace < 1) 
		return;

	/* we need at least two words if we want this to work */
	if((word_end - word_start) < 2)
		return;

	/* no hassling for a line with two words, fix 07/03/97-02, kdh */
	if((word_end - word_start) == 2)
	{
		/* just flush the second word to the right margin */
		words[word_start+1]->x += nspace*sw;
		return;
	}

	/* pick up the longest word */
	for(i = word_start; i < word_end; i++)
	{
		if(i == skip_id)
			continue;
		if(words[i]->len > longest_word)
			longest_word = words[i]->len;
	}

	word_len = longest_word;

	/* adjust interword spacing until we run out of spaces to add */
	while(nspace && num_iter < MAX_JUSTIFY_ITERATIONS)
	{
		/* walk all words in search of the longest one */
		for(i = word_start ; i < word_end && nspace; i++, num_iter++)
		{
			if(i == skip_id || words[i]->len == 0)
				continue;
			/* Found! */
			if(words[i]->len == word_len && 
					!IS_PUNCT(*(words[i]->word)) &&
					!(words[i]->spacing & TEXT_SPACE_NONE))
			{
				/* see if we are allowed to shift this word */
				if(!(words[i]->spacing & TEXT_SPACE_TRAIL) ||
					!(words[i]->spacing & TEXT_SPACE_LEAD))
					continue;

				/*****
				* Add a leading space if we may, but always shift all 
				* following words to the right.
				*
				* fix 07/03/97-01, kdh
				******/
				if(words[i]->spacing & TEXT_SPACE_LEAD && i != word_start)
				{
					for(j = i; j < word_end; j++)
					{
						if(j == skip_id)
							continue;
						words[j]->x += sw;
					}
					nspace--;
				}
				if(nspace)
				{
					for(j = i + 1; j < word_end; j++)
					{
						if(j == skip_id)
							continue;
						words[j]->x += sw;
					}

					/* we have only added a space if this is true */
					if(j != i+1)
						nspace--;
				}
			}
		}
		num_iter++;
		/* move on to next set of words eligible for space adjustement. */
		word_len = (word_len == 0 ? longest_word : word_len - 1);
	}
	if(num_iter == MAX_JUSTIFY_ITERATIONS)
	{
		_XmHTMLWarning(__WFUNC__(NULL, "JustifyText"),
			"Text justification: bailing out after %i iterations\n"
			"    (line %i of input)", MAX_JUSTIFY_ITERATIONS, 
			words[word_start]->owner->object->line);
	}
#ifdef DEBUG
	lines_done++;
	total_iterations += num_iter;
#endif
}

/*****
* Name: 		CheckAlignment
* Return Type: 	void
* Description: 	adjusts x-position of every word to reflect requested
*				alignment.
* In: 
*	w:			XmHTML widget
*	start:		starting text element
*	end:		ending text element
*	word_start:	starting word index in the start element.
*	sw:			current space width.
*	last_line:	indicates this is the last line in a text block;
* Returns:
*	nothing, but every word in start and end (and any object(s) in between
*	them) that belongs to the same line is updated to reflect the alignment.
*	This routine just returns if the current alignment matches the default
*	alignment.
*****/
static void
CheckAlignment(XmHTMLWidget html, XmHTMLWord *words[], int word_start,
	int word_end, int sw, int line_len, Boolean last_line, int skip_id)
{
	int i, width, offset;

	/* sanity */
	if(word_end < 1)
		return;

	/* total line width occupied by these words */
	width = words[word_end-1]->x + words[word_end-1]->width - 
			words[word_start]->x;

	_XmHTMLFullDebug(5, ("layout.c: CheckAlignment, start word: %s, index %i, "
		"end word: %s, index %i, width = %i\n", 
		words[word_start]->word, word_start, 
		words[word_end-1]->word, word_end-1, width));

	switch(words[word_start]->owner->halign)
	{
		case XmHALIGN_RIGHT:
			offset = line_len - width;
			break;
		case XmHALIGN_CENTER:
			offset = (line_len - width)/2;
			break;
		case XmHALIGN_LEFT:		/* layout computation is always left-sided */
			offset = 0;
			break;
		case XmHALIGN_JUSTIFY:
			/* sw == -1 when used for <pre> text */
			if(html->html.enable_outlining && !last_line && sw != -1)
			{
				JustifyText(html, words, word_start, word_end, sw, width,
					line_len, (word_start < skip_id ? skip_id : -1));
				offset = 0;
				break;
			}
			/* fall thru */
		case XmHALIGN_NONE:
		default:
			/* use specified alignment */
			switch(html->html.alignment)
			{
				case TALIGNMENT_END:
					offset = line_len - width;
					break;
				case TALIGNMENT_CENTER:
					offset = (line_len - width)/2;
					break;
				case TALIGNMENT_BEGINNING:
				default:
					offset = 0;
					break;
			}
			break;
	}
	/*****
	* only adjust with a positive offset. A negative offset indicates
	* that the current width is larger than the available width.
	* Will ignore alignment setting for pre text that is wider than the
	* available window width.
	*****/
	if(offset <= 0)
		return;
	for(i = word_start; i < word_end; i++)
		words[i]->x += offset;
}

/*****
* Name: 		getWords
* Return Type: 	XmHTMLWord**
* Description: 	creates an array containing all OBJ_TEXT elements between
*				start and end.
* In: 
*	start:		element at which to start collecting words;
*	end:		element at which to end collecting words;
*	nwords:		no of words collected. Updated upon return;
* Returns:
*	an array of XmHTMLWord.
* Note:
*	This routine is used by the text layout routines to keep layout computation
*	managable.
*****/
static XmHTMLWord**
getWords(XmHTMLObjectTableElement start, XmHTMLObjectTableElement end,
	int *nwords)
{
	static XmHTMLWord **words;
	XmHTMLObjectTableElement tmp;
	int i, k, cnt = 0;

	for(tmp = start; tmp != end ; tmp = tmp->next)
		cnt += tmp->n_words;

	words = (XmHTMLWord**)calloc(cnt, sizeof(XmHTMLWord*));

	for(tmp = start, k = 0; tmp != end; tmp = tmp->next)
	{
		for(i = 0 ; i < tmp->n_words; i++)
		{
			/* store word ptr & reset position to zero */
			words[k] = &(tmp->words[i]);
			words[k]->x = 0;
			words[k]->y = 0;
			words[k++]->line = 0;
		}
	}

	*nwords = cnt;
	return(words);
}

/*****
* Name: 		getWordsRtoL
* Return Type: 	XmHTMLWord**
* Description: 	see getWords
* In: 
*	see getWords
* Returns:
*	see getWords
* Note:
*	This routines reverses the objects to properly accomodate right-to-left
*	layout.
*	This is a seperate routine for performance reasons.
*****/
static XmHTMLWord**
getWordsRtoL(XmHTMLObjectTableElement start, XmHTMLObjectTableElement end,
	int *nwords)
{
	static XmHTMLWord **words;
	XmHTMLObjectTableElement tmp;
	int i, k, cnt = 0;

	for(tmp = start; tmp != end ; tmp = tmp->next)
		cnt += tmp->n_words;

	words = (XmHTMLWord**)calloc(cnt, sizeof(XmHTMLWord*));

	/* sanity */
	if(end == NULL)
		for(end = start; end->next != NULL; end = end->next);
	for(tmp = end->prev, k = 0; tmp != start->prev; tmp = tmp->prev)
	{
		for(i = 0; i < tmp->n_words; i++)
		{
			/* store word ptr & reset position to zero */
			words[k] = &(tmp->words[i]);
			words[k]->x = 0;
			words[k]->y = 0;
			words[k++]->line = 0;
		}
	}
	*nwords = cnt;
	return(words);
}

/*****
* Name:			AdjustBaseline
* Return Type: 	void
* Description: 	adjusts the baseline for each word between start and end.
* In: 
*	base_obj:	object which controls the baseline offset;
*	**words:	array of all words being laid out;
*	start:		starting word index;
*	end:		ending word index;
*	lineheight:	new lineheight (= spacing between to consecutive lines of text)
*	last_line:	True when called for the last line in paragraph. Used for
*				computing the proper vertical offset between the end of this
*				paragraph and the object following it.
* Returns:
*	nothing, but all words between start and end have their baseline adjusted.
*****/
static void
AdjustBaseline(XmHTMLWord *base_obj, XmHTMLWord **words, int start, int end, 
	int *lineheight, Boolean last_line)
{
	int i, y_offset = 0;

	my_assert(base_obj != NULL);

	if(base_obj->type == OBJ_IMG)
	{
		switch(base_obj->image->align)
		{
			case XmVALIGN_MIDDLE:
				y_offset = (*lineheight - base_obj->font->xfont->ascent)/2.;
				/* adjust return value from SetText */
				/* fix 07/03/97-04, kdh */
				if(last_line && base_obj != words[end-1])
					*lineheight = y_offset;
				break;
			case XmVALIGN_BASELINE:
			case XmVALIGN_BOTTOM:
				y_offset = *lineheight - base_obj->font->xfont->ascent;
				*lineheight += base_obj->font->xfont->ascent/2.;
				break;
			case XmVALIGN_TOP:
			default:
				break;
		}
	}
	else if(base_obj->type == OBJ_FORM)
	{
		/* fix 07/04/97-01, kdh */
		/* form elements are always aligned in the middle */
		y_offset = (*lineheight - base_obj->font->xfont->ascent)/2.;

		/* But they move the baseline down */
		*lineheight += base_obj->font->xfont->ascent/2.;
	}
	else if(!last_line) /* sanity */
		*lineheight = words[end]->height;

	/*****
	* Now adjust the baseline for every word on this line.
	* Split into a y_offset and non y_offset part for performance reasons.
	*****/
	if(y_offset)
	{
		for(i = start; i < end; i++)
		{
			/* only move text objects */
			if(words[i]->type == OBJ_TEXT)
				words[i]->y += y_offset;
			words[i]->base = base_obj;
		}
	}
	else
	{
		for(i = start; i < end; i++)
			words[i]->base = base_obj;
	}
}

#define UPDATE_WORD(W) { \
	/* images and forms need to have the font ascent substracted to get a */ \
	/* proper vertical alignment. */ \
	(W)->line = line; \
	(W)->x = x_pos + e_space; \
	if((W)->type != OBJ_TEXT && (W)->type != OBJ_BLOCK) \
	{ \
		(W)->y = y_pos + (W)->owner->y_offset - (W)->font->xfont->ascent; \
		have_object = True; \
	} \
	/* regular text, no additional adjustment required */ \
	else (W)->y = y_pos + (W)->owner->y_offset; \
	x_pos = (W)->x + (W)->width + (W)->owner->x_offset; \
}

/*****
* Name: 		SetText
* Return Type: 	void
* Description: 	main text layout driver;
* In: 
*	html:		XmHTMLWidget id;
*	*x:			initial x position, updated to new x position upon return;
*	*y:			initial y position, updated to new y position upon return;
*	start:		starting object id;
*	end:		ending object id;
*	in_pre:		True if called for preformatted text;
*	precompute:	True if we are pre-computing the box dimensions (Tables!)
* Returns:
*	nothing
*****/
static void
SetText(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement start,
	XmHTMLObjectTableElement end, Boolean in_pre, Boolean precompute)
{
	XmHTMLWord **words;
	int nwords, word_start, i;
	PositionBox my_box;
	XmHTMLObjectTableElement current = NULL;

	/***** 
	* to make it ourselves _much_ easier, put all the words starting from
	* start and up to end in a single block of words.
	*****/
	words = get_word_func(start, end, &nwords);

	/* sanity */
	if(nwords == 0)
		return;

	/*****
	* Set up the initial PositionBox to be used for text layout.
	*****/
	my_box.x         = box->x;
	my_box.y         = box->y;
	my_box.lmargin   = box->lmargin;
	my_box.rmargin   = box->rmargin;
	my_box.left      = box->left;
	my_box.right     = box->rmargin;
	my_box.width     = my_box.right - my_box.left;
	my_box.min_width = -1;
	my_box.height    = -1;

	/* do text layout */
	if(in_pre)
		ComputeTextLayoutPre(html, &my_box, words, 0, &nwords, True);
	else
		ComputeTextLayout(html, &my_box, words, 0, &nwords, True, precompute);

	if(precompute)
	{
		/* update return values */
		box->x = my_box.x;
		box->y = my_box.y;
		if(my_box.width > box->width || box->width == -1)
			box->width = my_box.width;
		if(my_box.min_width < box->min_width || box->min_width == -1)
			box->min_width = my_box.min_width;

		/* no longer needed */
		free(words);

		/* done precomputing */
		return;
	}

	/* Update all ObjectTable elements for these words */
	current = NULL;
	for(i = 0; i < nwords; i++)
	{
		if(current != words[i]->owner)
		{
			word_start = i;
			current    = words[i]->owner;
			current->x = words[i]->x;
			current->width = words[i]->width;
			current->line  = words[i]->line;
			/*****
			* To get correct screen updates, the vertical position and height
			* of this object are that of the baseline object.
			* The font is also changed to the font used by the baseline
			* object.
			*****/
			current->y      = words[i]->base->y;
			current->height = words[i]->base->height;
			current->font   = words[i]->base->font;
			
			/* get index of last word on the first line of this object. */
			for(; i < word_start + current->n_words-1 &&
				words[i]->line == words[i+1]->line; i++);
			/*****
			* Total line width is given by end position of last word on this
			* line minus the starting position of the first word on this line.
			* (ensures we take interword spacing into account)
			*****/
			current->width = words[i]->x + words[i]->width - current->x;

			/*****
			* Lineheight of this object is given by vertical position of last
			* word minus vertical position of first word in this block. Only
			* valid when this object spans multiple lines.
			*****/
			if(i != word_start + current->n_words-1)
			{
				current->height = words[word_start + current->n_words - 1]->y -
					words[word_start]->y;
			}
			else if(in_pre && words[i]->base->spacing)
			{
				/* vertical line spacing in preformatted text */ 
				current->height = ((int)words[i]->base->spacing) *
						words[i]->base->font->height;
			}
			/* and set i to last word of this object */
			i = word_start + current->n_words-1;
			_XmHTMLDebug(5, ("layout.c: SetText, object data: x = %d, y = %d, "
				"width = %d, height = %d, line = %i\n", current->x,
				current->y, current->width, current->height, current->line));
		}
	}

	/* and update return values */
	box->x = my_box.x;
	box->y = my_box.y;

	/* free words */
	free(words);
}

/*****
* Name: 		ComputeTextLayout
* Return Type: 	void
* Description: 	orders the given textdata into single lines, breaking and
*				moving up to the next line if necessary.
* In: 
*	w:			widget for which to do this;
*	box:		bounding box to be used for computing text layout;
*	words:		array of words to be laid out;
*	nstart:		starting idx;
*	nwords:		ending idx, can be updated upon return;
*	last_line:	indicates that this routine is called for the the last line in
*				a paragraph.
* Returns:
*	nothing
* Note:
*	This function does the layout of complete paragraphs at once.
*	A paragraph is given by all text elements between start and end.
*
*	This is a rather complex routine. Things it does are the following:
*	- considers images, HTML form members and text as the same objects;
*	- adjusts baseline according to the highest object on a line;
*	- adjusts space width if font changes;
*	- performs horizontal alignment;
*	- performs text outlining if required;
*	- glues words together if required (interword spacing);
*****/
static void
ComputeTextLayout(XmHTMLWidget html, PositionBox *box, XmHTMLWord **words,
	int nstart, int *nwords, Boolean last_line, Boolean precompute)
{
	XmHTMLfont *basefont, *font;
	XmHTMLWord *base_obj;
	Cardinal x_pos, y_pos, x_start, y_start;
	int i, sw, e_space = 0, word_start, word_width;
	int lineheight = 0, p_height = 0;
	Boolean have_object = False, first_line = True, done = False;
	Boolean in_line = True;
	int skip_id = -1, left, right, width, height;
	int min_box_width = 0, max_box_width = 0, max_box_height;

	/* initial offsets */
	left    = box->left;
	right   = box->right;
	x_start = left;
	x_pos   = x_start;
	y_pos   = box->y;
	width   = box->width;
	height  = box->height;

	basefont = font = words[nstart]->font;
	/* interword spacing */
	e_space = sw = font->isp;

	had_break = False;

	/*****
	* Proper baseline continuation of lines consisting of words with different
	* properties (font, fontstyle, images, form members or anchors) require us
	* to check if we are still on the same line. If we are, we use the baseline
	* object of that line. If we are on a new line, we take the first word of
	* this line as the baseline object.
	*****/
	if(!baseline_obj)
		base_obj = words[nstart];
	else
		base_obj = (last_text_line == line ? baseline_obj : words[nstart]);

	/* lineheight always comes from the current baseline object */
	max_box_height = lineheight = base_obj->height;

	word_start = nstart;

	/*****
	* Text layout:
	* we keep walking words until we are about to exceed the available
	* linewidth. When we are composing a line in this way, we keep track
	* of the highest word (which will define the maximum lineheight).
	* If a linefeed needs to be inserted, the lineheight is added to
	* every word for a line. We then move to the next line (updating the
	* vertical offset as we do) and the whole process repeats itself.
	*****/
	for(i = nstart; i <  *nwords && !done; i++)	
	{
		if(words[i]->type == OBJ_BLOCK && x_pos == right)
		{
			_XmHTMLDebug(5, ("layout.c: ComputeTextLayout, skipping, "
				"<BR>, lineheight = %i\n", lineheight));
			/* just skip it */
			e_space = 0;
			UPDATE_WORD(words[i]);
			continue;
		}
		/*****
		* We must flow text around a left-aligned image. First finish the
		* the current line, then adjust the left margin and available
		* linewidth and the height we should use.
		* We can only honor this attribute if the width of this image is
		* less than the available width.
		* Multiple left/right aligned images aren't supported (yet).
		*****/
		if(words[i]->type == OBJ_IMG &&
			(words[i]->image->align == XmHALIGN_LEFT ||
			words[i]->image->align == XmHALIGN_RIGHT))
		{
			if(skip_id == -1 && words[i]->width < width)
			{
				skip_id = i;
				/* we are already busy with a line, finish it first */
				if(in_line)
					continue;
				/* start of a line, just proceed */
			}
		}
		in_line = True;	/* we are busy with a line of text */
		had_break = False;

		/* get new space width if font changes */
		if(font != words[i]->font)
		{
			font = words[i]->font;
			sw = font->isp;		/* new interword spacing */

			/*****
			* If this font is larger than the current font it will become
			* the baseline font for non-text objects.
			*****/
			if(font->lineheight > basefont->lineheight)
				basefont = font;
		}

		/*****
		* Sigh, need to check if we may break words before we do the
		* check on current line width: if the current word doesn't have
		* a trailing space, walk all words which don't have a leading
		* and trailing space as well and end if we encounter the first word
		* which *does* have a trailing space. We then use the total width
		* of this word to check against available line width.
		*****/
		if(
			!(words[i]->spacing & TEXT_SPACE_TRAIL) && 
			!(words[i]->spacing & TEXT_SPACE_TRAIL_ZEROWIDTH) && 
			i+1 < *nwords &&
			!(words[i+1]->spacing & TEXT_SPACE_LEAD) &&
			!(words[i+1]->spacing & TEXT_SPACE_LEAD_ZEROWIDTH)
		)
		{
			int j = i+1;
			word_width = words[i]->width;
			while(j < *nwords)
			{
				if(!(words[j]->spacing & TEXT_SPACE_LEAD) &&
					!(words[j]->spacing & TEXT_SPACE_LEAD_ZEROWIDTH))
					word_width += words[j]->width;

				/* see if this word has a trail space and the next a leading */
				if(
					!(words[j]->spacing & TEXT_SPACE_TRAIL) && 
					!(words[j]->spacing & TEXT_SPACE_TRAIL_ZEROWIDTH) && 
					j+1 < *nwords &&
					!(words[j+1]->spacing & TEXT_SPACE_LEAD) &&
					!(words[j+1]->spacing & TEXT_SPACE_LEAD_ZEROWIDTH)
				)
					j++;
				else
					break;
			}
		}
		else
			word_width = words[i]->width;

		/* minimum box width must fit the longest non-breakable word */
		if(min_box_width < word_width)
			min_box_width = word_width;

		/* Check if we are about to exceed the viewing width */
		if((i && x_pos + word_width + e_space >= right) ||
			words[i]->type == OBJ_BLOCK)
		{
			/*****
			* If this is a forced linebreak we act as this is the last
			* line in a paragraph: no implicit lineheight adjustment and no
			* text justification in CheckAlignment.
			*****/
			Boolean is_break = words[i]->type == OBJ_BLOCK;

			/*****
			* set font of non-text objects to the largest font of the
			* text objects (required for proper anchor drawing)
			*****/
			if(base_obj->type != OBJ_TEXT)
				base_obj->font = basefont;

			/* adjust baseline for all words on the current line */
			AdjustBaseline(base_obj, words, word_start, i, &lineheight,
				is_break);

			/* Adjust for alignment */
			CheckAlignment(html, words, word_start, i, sw, width, is_break,
				skip_id);

			/* increment absolute height */
			y_pos += lineheight;

			/* increment absolute box height */
			max_box_height += lineheight;

			/* insert linebreak */
			if(is_break)
			{
				int h;
				h = (int)((words[i]->line_data)*base_obj->font->lineheight);

				/* no negative linebreaks! */
				if((h -= lineheight) < 0)
					h = 0; /* no negative breaks! */
				y_pos += h;
				max_box_height += h;
				/*****
				* INSERT CODE
				* if we are flowing around an image, check if break = clear
				* was given. If so, terminate flow around the current
				* image.
				*****/

				/*****
				* This word was a break and therefore the next word can't have
				* a leading space (if it has it will mess up text
				* justification).
				* Fix 12/15/97-02, kdh
				*****/
				if(i+1 != *nwords)
					words[i+1]->spacing &= ~TEXT_SPACE_LEAD;
			}

			/* update maximum box width */
			if(x_pos - x_start > max_box_width)
				max_box_width = x_pos - x_start;

			x_pos = x_start;
			line++;
			word_start  = i;		/* next word starts on a new line */
			lineheight  = words[i]->height;
			base_obj    = words[i];
			have_object = False;	/* object has been done */
			first_line  = False;	/* no longer the first line */
			in_line     = False;	/* done with current line */

			_XmHTMLFullDebug(5, ("layout.c: ComputeTextLayout, linefeed, "
				"x = %d, y = %d.\n", x_pos, y_pos));

			/* line is finished, set all margins for proper text flowing */
			if(skip_id != -1)
			{
				/* start of text flowing */
				if(height == -1)
				{
					/* save all info for this word */
					words[skip_id]->line = line;
					have_object = True;
					words[skip_id]->y = y_pos +
						words[skip_id]->owner->y_offset -
						words[skip_id]->font->xfont->ascent;

					/* this word sets the baseline for itself */
					words[skip_id]->base = words[skip_id];

					/* set appropriate margins */
					if(words[skip_id]->image->align == XmHALIGN_RIGHT)
					{
						/* flush to the right margin */
						words[skip_id]->x = right - words[skip_id]->width;
						right = words[skip_id]->x;
					}
					else
					{
						/*****
						* Flush to the left margin, it's the first word on
						* this line, so no leading space is required.
						*****/
						words[skip_id]->x = x_pos;
						x_pos = words[skip_id]->x + words[skip_id]->width;
						left = x_pos + e_space;
					}
					p_height = 0;
					height = words[skip_id]->height;
					width = box->width - words[skip_id]->width - sw - e_space;
				}
				else /* increment height of this paragraph */
					p_height += lineheight;

				/*****
				* If this is True, we are at the bottom of the image
				* Restore margins and continue.
				*****/
				if(p_height >= height)
				{
					skip_id = -1;
					height = -1;

					left  = box->left;
					right = box->right;
					width = box->width;
					x_pos = x_start;
				}
			}
		}

		/* save maximum lineheight */
		if(lineheight < words[i]->height)
		{
			/*****
			* Shift all words already placed on this line down. Don't do it
			* for the first line in a paragraph and if this word is actually
			* an image as this is already taken into account (paragraph
			* spacing)
			******/
			if(!first_line && words[i]->type != OBJ_IMG)
			{
				/* fix 07/03/97-03, kdh */
				int k = word_start;

				/* new vertical position of all words in the current line */
				y_pos += (words[i]->height - lineheight);
				max_box_height += (words[i]->height - lineheight);

				/* shift 'em down. No need to check skip_id */
				for(; k < i; k++)
					words[k]->y = y_pos;
			}
			/* save new lineheight */
			lineheight = words[i]->height;
			base_obj   = words[i];
		}

		/*****
		* Interword Spacing.
		* 	1. word starts at beginning of a line, don't space it at all.
		*	   (box->lmargin includes indentation as well)
		* 	2. previous word does not have a trailing spacing:
		* 		a. current word does have leading space, space it.
		*		b. current word does not have a leading space, don't space it.
		* 	3. previous word does have a trailing space:
		*		a. always space current word.
		* 	4. previous word does not have any spacing:
		*		a. current word has leading space, space it.
		*		b. current word does not have a leading space, don't space it.
		* Note: if the previous word does not have a trailing space and the
		*	current word does not have a leading space, these words are
		*	``glued'' together.
		*****/
		e_space = 0;
		if(i != 0 && x_pos != left)
		{
			if(!(words[i-1]->spacing & TEXT_SPACE_TRAIL))
			{
				if(words[i]->spacing & TEXT_SPACE_LEAD)
					e_space = sw;
			}
			else if(words[i-1]->spacing & TEXT_SPACE_TRAIL)
				e_space = sw;
			else if(words[i]->spacing & TEXT_SPACE_LEAD)
				e_space = sw;

			/* additional end-of-line spacing? */
			if(e_space && words[i]->word[words[i]->len-1] == '.')
				e_space += font->eol_sp;
		}
		/*****
		* save linenumber, x and y positions for this word or for
		* multiple words needing to be ``glued'' together.
		*****/
		if(
			!(words[i]->spacing & TEXT_SPACE_TRAIL) && 
			!(words[i]->spacing & TEXT_SPACE_TRAIL_ZEROWIDTH) && 
			i+1 < *nwords &&
			!(words[i+1]->spacing & TEXT_SPACE_LEAD) &&
			!(words[i+1]->spacing & TEXT_SPACE_LEAD_ZEROWIDTH)
		)
		{
			/* first word must take spacing into account */
			UPDATE_WORD(words[i]);
			/* all other words are glued, so no spacing! */
			e_space = 0;
			i++;
			while(i < *nwords)
			{
				/* don't take left/right flushed image into account */
				if(i == skip_id)
					continue;
				/* connected word, save line, x and y pos. */
				if(!(words[i]->spacing & TEXT_SPACE_LEAD))
					UPDATE_WORD(words[i])

				/* this word has a trailing and the next a leading space? */
				if(
					!(words[i]->spacing & TEXT_SPACE_TRAIL) && 
					!(words[i]->spacing & TEXT_SPACE_TRAIL_ZEROWIDTH) && 
					i+1 < *nwords &&
					!(words[i+1]->spacing & TEXT_SPACE_LEAD) &&
					!(words[i+1]->spacing & TEXT_SPACE_LEAD_ZEROWIDTH)
				)
					i++;
				else
					break;
			}
		}
		else /* save line, x and y pos for this word. */
			UPDATE_WORD(words[i])

		my_assert(base_obj != NULL);

	}
	/*****
	* If we've got an image left, update it. We only have an image left if
	* it's position hasn't been updated in the above loop, it will be
	* positioned otherwise, but we ran out of text before we reached the
	* box's height. So we need to update y_pos to move the baseline properly
	* down. The box itself isn't restored as we have to check the alignment
	* for this last line as well.
	*****/
	if(skip_id != -1)
	{
		if(words[skip_id]->x == 0 && words[skip_id]->y == 0)
		{
			UPDATE_WORD(words[skip_id]);
		}
		else	/* update y_pos */
			y_pos += height - p_height;
	}

	/*****
	* How do we know we are at the end of this block of text objects??
	* If the calling routine set last_line to True, we know we are done
	* and we can consider the layout computation done.
	* If last_line is False, we can be sure that other text is coming so
	* we must continue layout computation on the next call to this routine.
	* If we haven't finished computing the layout for all words, we were
	* flowing text around an object (currently only images), and we need
	* to adjust the number of words done *and* be able to restart computation
	* on the next call to this routine.
	*****/
	if(i == *nwords)
	{
		if(last_line)
			done = True;
		else
			done = False;
	}
	else if(done)
	{
		*nwords = i;
		done = False;
	}

	/* also adjust baseline for the last line */
	if(base_obj->type != OBJ_TEXT)
		base_obj->font = basefont;

	AdjustBaseline(base_obj, words, word_start, i, &lineheight, done);

	/* also adjust alignment for the last line */
	CheckAlignment(html, words, word_start, *nwords, sw, box->width, done,
		skip_id);

	/* save initial vertical offset */
	y_start = box->y;

	/* non-text objects (images & form members) move the baseline downward */
	if(have_object || precompute)
	{
		box->y = y_pos;
		/*
		* If we are precomputing for table layout, we need to add an
		* artificial linebreak ourselves
		*/
		if(precompute && (in_line || word_start != *nwords))
			box->y += lineheight;
		else	/* objects always cause a linebreak */
			box->y += lineheight;
		had_break = True;
	}
	else
		box->y = y_pos;
	box->x = x_pos;

	/* store final box height */
	if(first_line || (box->height = box->y - y_start) == 0)
	{
		if(lineheight > base_obj->font->height)
			box->height = lineheight;
		else
			box->height = base_obj->height;
	}
	else
		box->height = max_box_height;

	/* check maximum box width again */
	if(x_pos - x_start > max_box_width)
		max_box_width = x_pos - x_start;

	/* store minimum and maximum box width */
	box->width = max_box_width > min_box_width ? max_box_width : min_box_width;
	box->min_width = min_box_width;

	box->width += base_obj->font->lbearing;
	box->min_width += base_obj->font->lbearing;

	/* and check against document maximum width */
	if(max_box_width > max_width)
		max_width = max_box_width;

	/* last text line and baseline object for this piece of text */
	last_text_line = line;
	baseline_obj   = base_obj;

	/*****
	* If we haven't done a full line, we must increase linenumbering
	* anyway as we've inserted a linebreak.
	*****/
	if(first_line)
		line++;
}

/*****
* Name:			AdjustBaselinePre
* Return Type: 	void
* Description: 	see AdjustBaseline
* In: 
*
* Returns:
*	nothing.
*****/
static void
AdjustBaselinePre(XmHTMLWord *base_obj, XmHTMLWord **words, int start, int end, 
	int *lineheight, Boolean last_line)
{
	int i, y_offset = 0;

	if(base_obj->type == OBJ_IMG)
	{
		switch(base_obj->image->align)
		{
			case XmVALIGN_MIDDLE:
				y_offset = (*lineheight - base_obj->font->xfont->ascent)/2.;
				/* adjust return value from SetText */
				/* fix 07/03/97-04, kdh */
				if(last_line && base_obj != words[end-1])
					*lineheight = y_offset;
				break;
			case XmVALIGN_BASELINE:
			case XmVALIGN_BOTTOM:
				y_offset = *lineheight - base_obj->font->xfont->ascent;
				*lineheight += 0.5*base_obj->font->xfont->ascent;
				break;
			case XmVALIGN_TOP:
			default:
				break;
		}
	}
	else if(base_obj->type == OBJ_FORM)
	{
		/* fix 07/04/97-01, kdh */
		/* form elements are always aligned in the middle */
		y_offset = 0.5*(*lineheight - base_obj->font->xfont->ascent);

		/* But they move the baseline down */
		*lineheight += 0.5*base_obj->font->xfont->ascent;
	}
	else  /* sanity */
		return;

	/* Now adjust the baseline offset for every word on this line. */
	if(y_offset)
	{
		for(i = start; i < end; i++)
		{
			/* only move text objects */
			if(words[i]->type == OBJ_TEXT)
				words[i]->y += y_offset;
		}
	}
}

/*****
* Name:			ComputeTextLayoutPre
* Return Type: 	void
* Description: 	main text layout engine for preformatted text.
* In: 
*	html:		XmHTMLWidget id;
*	box:		bounding box to be used for computing text layout;
*	words:		array of words to be laid out;
*	nstart:		starting idx;
*	nwords:		ending idx, can be updated upon return;
*	last_line:	indicates that this routine is called for the the last line in
*				a paragraph.
* Returns:
*	nothing, but all words between start and end now have a screen position
*	and a line number.
*****/
static void
ComputeTextLayoutPre(XmHTMLWidget html, PositionBox *box, XmHTMLWord **words,
	int nstart, int *nwords, Boolean last_line)
{
	XmHTMLfont *basefont, *font;
	XmHTMLWord *base_obj;
	Cardinal x_pos, y_pos, y_start;
	int i, word_start;
	int lineheight = 0, y_offset, p_height = 0;
	Boolean have_object = False, first_line = True, done = False;
	int max_box_width = 0;

	/* initial starting point */
	x_pos  = box->left;
	y_pos  = box->y;

	/*****
	* Baseline stuff. Always initialize to the first word of this para.
	*****/
	base_obj   = words[0];
	basefont   = font = words[0]->font;
	word_start = 0;
 	y_offset   = basefont->height;

	if(base_obj->spacing != 0)
		lineheight = y_offset;
	else
		lineheight = base_obj->height;

	/*****
	* Text layout:
	* we keep walking words until we are about to insert a newline.
	* Newlines are marked by words width a non-zero spacing field.
	* When we are composing a line in this way, we keep track
	* of the highest word (which will define the maximum lineheight).
	* If a linefeed needs to be inserted, the lineheight is added to
	* every word for a line. We then move to the next line (updating the
	* vertical offset as we do) and the whole process repeats itself.
	*****/
	for(i = nstart; i < *nwords && !done; i++)	
	{
		/* compute new line spacing if font changes */
		if(font != words[i]->font)
		{
			font = words[i]->font;
			/*
			* If this font is larger than the current font it will become
			* the baseline font for non-text objects.
			* Must use maxbounds fontheight for fixed width fonts.
			*/
			if(font->lineheight > basefont->lineheight)
			{
				basefont = font;
 				y_offset = basefont->lineheight;
			}
		}

		/* check maximum lineheight */
		if(lineheight < words[i]->height)
		{
			/* this becomes the new baseline object */
			base_obj = words[i];
			/*****
			* Shift all words already placed on this line down. Don't do
			* it for the first line in a paragraph and if this word is
			* actually an image as this is already taken into account
			* (paragraph spacing)
			******/
			if(!first_line && words[i]->type != OBJ_IMG)
			{
				/* fix 07/03/97-03, kdh */
				int k = word_start;	/* idx of first word on this line */

				/* new vertical position of all words in the current line */
				y_pos += (words[i]->height - lineheight);

				/* shift 'em down */
				for(; k < i; k++)
				{
					words[k]->y = y_pos;
					words[k]->base = base_obj;
				}
			}
			/* Store new lineheight */
			if(words[i]->spacing != 0)
				lineheight = ((int)words[i]->spacing) * y_offset;
			else
				lineheight = words[i]->height;
		}
		/*****
		* save line, x and y pos for this word.
		* We don't do any interword spacing for <PRE> objects, they
		* already have it. Images and forms need to have the font ascent
		* substracted to get a proper vertical alignment.
		*****/
		words[i]->line = line;	/* fix 04/26/97-01, kdh */
		words[i]->x    = x_pos;
		words[i]->base = base_obj;
		if(words[i]->type != OBJ_TEXT)
		{
			words[i]->y = y_pos + words[i]->owner->y_offset -
							words[i]->font->xfont->ascent;
			have_object = True;
		}
		/* regular text, no additional adjustment required */
		else
			words[i]->y = y_pos + words[i]->owner->y_offset;

		x_pos += words[i]->width;

		/* we must insert a newline */
		if(words[i]->spacing != 0)
		{
			/*****
			* Adjust font of non-text objects to the largest font of the
			* text objects (required for proper anchor drawing)
			*****/
			if(base_obj->type != OBJ_TEXT)
				base_obj->font = basefont;

			/*****
			* Adjust baseline for all words on the current line if the
			* current baseline object is an image or a form component.
			* For all plain text objects we only need the lineheight.
			*****/
			if(base_obj->type == OBJ_IMG || base_obj->type == OBJ_FORM)
				AdjustBaselinePre(base_obj, words, word_start, i, &lineheight,
					False);
			else
				lineheight = (((int)words[i]->spacing) * y_offset);

			y_pos += lineheight;

			/* increment height of this paragraph */
			p_height += lineheight;

			/* Adjust for alignment */
			CheckAlignment(html, words, word_start, i, -1, box->width, False,
				-1);

			if(x_pos > max_box_width)
				max_box_width = x_pos;

			x_pos = box->left;
			line++;
			word_start  = i+1;		/* next word starts on a new line */
			base_obj    = words[i];
			basefont    = base_obj->font;
 			y_offset    = basefont->lineheight;
			lineheight  = y_offset;		/* default lineheight */
			have_object = False;
			first_line  = False;

			_XmHTMLFullDebug(5, ("layout.c: ComputeTextLayoutPre, linefeed, "
				"x = %d, y = %d.\n", x_pos, y_pos));

			if(box->height != -1 && p_height >= box->height)
				done = True;
		}
	}
	/* sanity, can be true for short <pre></pre> paragraphs. */
	if(word_start == *nwords)
		word_start--;

	if(i == *nwords)
	{
		if(last_line)
			done = True;
		else
			done = False;
	}
	else if(done)
	{
		*nwords = i;
		done = False;
	}

	/* also adjust baseline for the last line */
	if(base_obj->type == OBJ_IMG || base_obj->type == OBJ_FORM)
		AdjustBaselinePre(base_obj, words, word_start, i, &lineheight, done);

	/* also adjust alignment for the last line */
	CheckAlignment(html, words, word_start, *nwords, -1, box->width, done, -1);

	y_start = box->y;

	/* non-text objects (images & form members) move the baseline downward */
	if(have_object)
	{
		box->y = y_pos + lineheight;
		had_break = True;
	}
	else
		box->y = y_pos;
	box->x = x_pos;

	/* store final box height */
	if((box->height = box->y - y_start) == 0)
		box->height = lineheight;

	/* check maximum box width again */
	if(x_pos > max_box_width)
		max_box_width = x_pos;

	/*
	* store box width. Minimum width is same as maximum width for
	* preformatted text.
	*/
	box->width = box->min_width = max_box_width;

	/* and check against document maximum width */
	if(max_box_width > max_width)
		max_width = max_box_width;

	/* make sure we have a linefeed */
	if(first_line)
		line++;

	/* all done */
}

static void
SetApplet(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement data)
{
	data->x = box->x;
	data->y = box->y;
	data->height = html->html.default_font->height;
	data->line   = line;
}

static void
SetBlock(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement data)
{
	XmHTMLfont *font = (data->font ? data->font : html->html.default_font);
	data->x = box->x;
	data->y = box->y;
	data->height = font->lineheight;	/* fix 01/25/97-01; kdh */
	data->line   = line;
}

/*****
* Name: 		SetRule
* Return Type: 	computes the offsets & position of a horizontal rule.
* Description: 
* In: 
*	html:		XmHTMLWidget id;
*	box:		current minipage;
*	data:		element data.
* Returns:
*	nothing.
* Note:
*	rules always start at the *left* margin of the current page and extend
*	across the full available width.
*****/
static void
SetRule(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement data)
{
	int width = box->width;
	int left = box->lmargin;
	int h;

	/* horizontal offset */
	box->x = left + data->ident;

	/* See if we have an width specification */
	if(data->len != 0)
	{
		if(data->len < 0)	/* % spec */
			width *= (float)(-1*data->len/100.);
		else	/* pixel spec, cut if wider than available */
			width = (data->len > width ? width : data->len);
		/* alignment is only honored if there is a width spec */
		switch(data->halign)
		{
			case XmHALIGN_RIGHT:
				box->x = left + box->width - width;
				break;
			case XmHALIGN_CENTER:
				box->x = left + (box->width - width - left)/2;
			default:	/* shutup compiler */
				break;
		}
	}
	/* Save position and width */
	data->x = box->x;
	data->y = box->y;
	data->line  = line;
	data->width = width;

	/* Adjust x and y */
	box->x = left;

	/*
	* This might seem funny, but it is a hack to correct the linefeeding
	* mechanism in format.c. Horizontal rules are a real pain in the you
	* know what to deal with.
	* If we do not increase y, text will appear above and on (or even in)
	* a rule.
	*/
	if(data->linefeed)
		h = 2*data->linefeed + data->height;
	else
		h = (int)(2*(html->html.default_font->lineheight)) + data->height;

	/* linefeed */
	line += 2;
	box->y += h;
}

/*****
* Name: 		SetBullet
* Return Type: 	void
* Description: 	computes the position & offsets for a list leader (can be a
*				bullet or number).
* In: 
*	html:		XmHTMLWidget id;
*	box:		current minipage;
*	data:		element data.
* Returns:
*	nothing. Upon return the current element has it's position set.
* Note:
*	Bullets always carry indentation within them. This indentation is the
*	*total* indentation from the left margin and therefore the x-position
*	is computed using the left margin, and not the left position. As we
*	want all bullets to be right aligned, the x-position of a bullet is
*	computed as the left margin *plus* any indentation. When being rendered,
*	the real position is the computed position *minus* the width of the bullet.
*****/
static void
SetBullet(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement data)
{
	/* save vertical position */
	data->y = box->y;

	/* linefeed if not at left margin */
	if(box->x != box->lmargin)
		line++;
	box->y += data->linefeed; 
	box->x = box->lmargin + data->ident;

	/* we have a left offset */
	box->left = box->x;

	data->x = box->x;
	data->line = line;
}

static void
SetBreak(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement data)
{
	int linefeed = data->linefeed;

	/* position of this break */
	data->y = box->y;
	data->x = box->x;

	/* check if this is a real linebreak or just a margin reset */
	if(linefeed)
	{
		/* if we already had a linefeed, we can substract one */
		if(had_break && baseline_obj)
		{
			linefeed -= baseline_obj->font->lineheight;
			had_break = False;
		}
		/* no negative linefeeds!! */
		if(linefeed > 0)
		{
			line++;
			box->y += data->linefeed;
			/* update box height */
			box->height = linefeed;
		}
	}

	/* reset margin */
	box->x = box->lmargin + data->ident;
	box->left = box->x;

	data->line = line;
	data->height = box->y - data->y;	/* height of this linefeed */
}

static XmHTMLObjectTableElement
SetTable(XmHTMLWidget html, PositionBox *box, XmHTMLObjectTableElement data)
{
	XmHTMLTable *table;
	TableRow *row = NULL;
	TableCell *cell = NULL;
	PositionBox **boxes = NULL;	/* 2D position matrix		*/
	int *rows = NULL;			/* maximum row heights		*/
	int *min_cols = NULL;		/* minimum width column dimensions	*/
	int *max_cols = NULL;		/* maximum width column dimensions	*/
	int i, j, k, idx;
	int max_twidth = 0, min_twidth = 0, max_theight = 0;
	int twidth, ncols, nrows, hspace, vspace, hpad, vpad, bwidth;
	int full_max_twidth, full_min_twidth, usable_twidth;
	Cardinal x_pos, y_pos, x_start;
	int save_line, max_line;
	Boolean needs_height_adjustment = False;
	static int depth;

	/* pick up table data */
	table = data->table;

	/*****
	* The first table in a stack of tables contains all data for all
	* table childs it contains. The first table child is the master
	* table itself. So when a table doesn't have a child table it *is*
	* a child table itself and thus we should add the left offset
	* to the initial horizontal position.
	*****/
	if(table->childs)
	{
		table = &(table->childs[0]);
		x_start = box->x;
	}
	else
		x_start = box->left;

	/* table position */
	data->x = x_start;
	data->y = box->y;

	_XmHTMLDebug(17, ("layout.c: ------ Table Layout Starts -----\n"));
	_XmHTMLDebug(17, ("layout.c: table starts near line %i and ends at line "
		"%i in input).\n", table->start->object->line,
		table->end->object->line));
	_XmHTMLDebug(17, ("layout.c: initial box position: %i, %i\n",
		box->x, box->y));
	_XmHTMLDebug(17, ("layout.c: table depth: %i\n", depth + 1));
	_XmHTMLDebug(17, ("layout.c: table has %i rows and %i columns.\n", 
		table->nrows, table->ncols));
	depth++;

	/* set maximum table width */
	if(table->width)
	{
		/* relative to current box width */
		if(table->width < 0)
			twidth = (-table->width * box->width)/100.;
		else
			twidth = table->width;
	}
	else
	{
		/* if this is a child table, assume a 100% width */
		if(!table->childs)
			twidth = box->width;
		else
		{
			/* parent table, use whatever we can get our hands on */
			if((twidth = box->width - Abs(box->x - data->ident)) <= 0)
				twidth = -1;
		}
	}

	/* save current line number count */
	save_line = line;

	/* total no of rows and columns this table is made up of */
	nrows = table->nrows;
	ncols = table->ncols;
	if(ncols == 0 || nrows == 0)
	{
		_XmHTMLWarning(__WFUNC__(html, "SetTable"), "Empty table, ignoring.");
		return(table->end);
	}
	/*****
	* Sanity Check: check all cells in search of a rowspan attribute. If
	* we detect a rowspan in the *last* cell of a row, we must add a bogus
	* cell to this row. If we don't do this, any cells falling in this row
	* will be skipped, causing text to disappear (at the least, in the worst
	* case it will cause a crash 'cause any in the skipped text anchors
	* are never detected).
	*****/
	for(i = 0; i < table->nrows; i++)
	{
		row = &(table->rows[i]);

		for(j = 0; j < row->ncells; j++)
		{
			if(row->cells[j].rowspan > 1 && (j+1) == ncols)
				ncols++;
		}
	}

	/* allocate a box for each row in this table */
	boxes = (PositionBox**)calloc(nrows, sizeof(PositionBox*));

		/* allocate a range of boxes spanning all columns */
	for(i = 0; i < nrows; i++)
		boxes[i] = (PositionBox*)calloc(ncols, sizeof(PositionBox));

	/*****
	* Step One: check if we have cells spanning multiple rows or columns.
	* We always assume the table is rectangular: each row has the same
	* amount of columns. If a cell is really being used, it will have a 
	* positive box index. A box with a negative index value means that
	* this is a bogus cell spanned by it's neighbouring cells (which can be
	* in another row).
	*****/
	for(i = 0; i < nrows; i++)
	{
		row = &(table->rows[i]);

		for(j = 0, idx = 0; j < ncols && idx < row->ncells; j++)
		{
			/* can happen when a cell spans multiple rows */
			if(boxes[i][j].idx == -1)
				continue;

			cell = &(row->cells[idx]);

			/* adjust col & rowspan if not set or incorrect */
			if(cell->colspan <= 0 || cell->colspan > ncols)
				cell->colspan = ncols;
			if(cell->rowspan <= 0 || cell->rowspan > nrows)
				cell->rowspan = nrows;

			boxes[i][j].idx = idx;
			if(cell->colspan != 1)
			{
				/* subsequent cells are spanned by this cell */
				for(k = 1; (j + k) < ncols && k < cell->colspan; k++)
					boxes[i][j + k].idx = -1;
				/* update cell counter to last spanned cell */
				j += (k-1);
			}
			if(cell->rowspan != 1)
			{
				/* subsequent rows are spanned by this cell */
				for(k = 1; (i + k) < nrows && k < cell->rowspan; k++)
					boxes[i + k][j].idx = -1;
			}
			idx++;
		}
		if(j != ncols)
			for(; j < ncols; j++)
				boxes[i][j].idx = -1;
	}

	/*****
	* Step Two: compute minimum and maximum width of each cell.
	* All precomputation is done without *any* margins, they will be
	* added later.
	*****/
	_XmHTMLDebug(17, ("layout.c: ------ Table Precomputation Starts -----\n"));

	_XmHTMLDebug(17, ("layout.c: Precomputing table dimensions:\n"));

	for(i = 0; i < nrows; i++)
	{
		row = &(table->rows[i]);

		/* compute desired cell dimensions */
		for(j = 0; j < ncols; j++)
		{
			/* skip if this is a spanned cell */
			if((idx = boxes[i][j].idx) != -1)
				cell = &(row->cells[idx]);
			else
			{
				_XmHTMLDebug(17, ("Cell(%i,%i,%i): spanned\n", depth, i, j));
				continue;
			}

			/* offsets unused */

			/*****
			* Preset margins. Layout precomputation *excludes* any spacing,
			* it is added later since it doesn't apply to the cell contents.
			* Defaults are for unknown cell dimensions.
			*****/
			boxes[i][j].lmargin = 0;
			boxes[i][j].width = boxes[i][j].height = -1;
			boxes[i][j].rmargin = 0xfffffff;

			/* do we have a width? */
			if(cell->width)
			{
				/* is it relative to table width? */
				if(cell->width < 0)
				{
					/* yes it is, do we have a table width? */
					if(twidth != -1)
						boxes[i][j].width = (-cell->width * twidth)/100.;
					else
						boxes[i][j].width = -1;
				}
				else /* absolute cell width */
					boxes[i][j].width = cell->width;
			}
			/* substract padding and check if we still have a valid width */
			if((boxes[i][j].width -= 2*table->hpadding) < 0)
				boxes[i][j].width = -1;
			else
				boxes[i][j].rmargin = boxes[i][j].width;

			/* set minimum width */
			boxes[i][j].min_width = boxes[i][j].width;

			/*****
			* Do we have a cell height? If so, it must be an absolute
			* number. Can't do anything with relative heights.
			*****/
			if(cell->height > 0 && (cell->height - 2*table->vpadding) > 0)
				boxes[i][j].height = cell->height - 2*table->vpadding;
			else
				boxes[i][j].height = -1;	/* leave open ended */

			/*****
			* Precompute the required dimensions for this cell.
			* Upon return, the PositionBox will have updated values for
			* width, min_width and height.
			*****/
			PreComputeTableLayout(html, &boxes[i][j], cell->start,
				cell->end);

			_XmHTMLDebug(17, ("Cell(%i,%i,%i): width = %i, min_width = %i, "
				"height = %i\n", depth, i, j, boxes[i][j].width,
				boxes[i][j].min_width, boxes[i][j].height));
		}
	}

	/*****
	* Step Three: compute minimum and maximum row widths.
	* The table layout is contained in the PositionBox matrix.
	*****/

	/* allocate room for minimum row dimensions */
	rows = (int*)malloc(nrows * sizeof(int));

	/* allocate room to store min & max column dimensions */
	min_cols = (int*)malloc(ncols * sizeof(int));
	max_cols = (int*)malloc(ncols * sizeof(int));

	/* initialize to unknown sizes */
	rows = memset(rows, -1, nrows * sizeof(int));
	min_cols = memset(min_cols, -1, ncols * sizeof(int));
	max_cols = memset(max_cols, -1, ncols * sizeof(int));

	/* compute minimum & maximum column widths and row heights */
	for(i = 0; i < nrows; i++)
	{
		int row_max_height = 0;

		/* get current row */
		row = &(table->rows[i]);

		/* walk all cells in this row */
		for(j = 0; j < ncols; j++)
		{
			/* skip if this is a spanned cell */
			if((idx = boxes[i][j].idx) != -1)
				cell = &(row->cells[idx]);
			else
				continue;

			/*****
			* Height & width are useless for cells spanning multiple rows or
			* cells, both will get proper values once all cell widths have
			* been set. The one exception is when the minimum width
			* of this cell equals the maximum width, in which case the cell
			* can't possibly be broken.
			*
			* As we are computing *total* cell dimensions, we must take
			* border width & cell spacing into account as well.
			*****/
			if((cell->colspan == 1 && cell->rowspan == 1) ||
				boxes[i][j].width == boxes[i][j].min_width)
			{
				/*****
				* Get largest minimum column width.
				* If nowrap is in effect for this cell, compare against
				* maximum cell width.
				*****/
				if(cell->properties->nowrap)
				{
					if(min_cols[j] < boxes[i][j].width)
						min_cols[j] = boxes[i][j].width;
				}
				else
				{
					if(min_cols[j] < boxes[i][j].min_width)
						min_cols[j] = boxes[i][j].min_width;
				}

				/* Get smallest maximum column width */
				if(max_cols[j] < boxes[i][j].width)
					max_cols[j] = boxes[i][j].width;

				/* get maximum row height */
				if(row_max_height < boxes[i][j].height)
					row_max_height = boxes[i][j].height;
			}
			else
			{
				boxes[i][j].height = -1;
				boxes[i][j].width  = -1;
				needs_height_adjustment = True;
			}
		}
		/* store height for this row */
		rows[i] = row_max_height;

		/* and update table height */
		max_theight += row_max_height;
	}

	/* check if we have any open-ended columns */
	if(needs_height_adjustment)
	{
		_XmHTMLDebug(17, ("layout.c: checking for unknown column widths\n"));
		for(i = 0; i < ncols; i++)
		{
			if(max_cols[i] == -1 || min_cols[i] == -1)
			{
				int min_col_width = 0xfffffff;

				/* get smallest column */
				for(k = 0; k < nrows; k++)
				{
					if(boxes[k][i].min_width > 0 &&
						min_col_width > boxes[k][i].min_width)
						min_col_width = boxes[k][i].min_width;
				}
				/* hmm, try smallest widest column */
				if(min_col_width == 0xfffffff)
				{
					for(k = 0; k < nrows; k++)
					{
						if(boxes[k][i].width > 0 &&
							boxes[k][i].width < min_col_width)
							min_col_width = boxes[k][i].width;
					}
					/* not found, author intended to have an empty row */
					if(min_col_width == 0xfffffff)
						min_col_width = 0;
				}
				if(max_cols[i] == -1)
				{
					_XmHTMLDebug(17, ("layout.c: column %i, computed maximum "
						"width as %i\n", i, min_col_width));
					max_cols[i] = min_col_width;
				}
				if(min_cols[i] == -1)
				{
					_XmHTMLDebug(17, ("layout.c: column %i, computed minimum "
						"width as %i\n", i, min_col_width));
					min_cols[i] = min_col_width;
				}
			}
		}
	}


	/*****
	* Compute *full* minimum & maximum table widths. Full table width takes
	* all all spacing into account.
	* The used table width is the table width minus all spacing.
	*****/
	hspace = table->hmargin;
	vspace = table->vmargin;
	hpad   = table->hpadding;
	vpad   = table->vpadding;
	bwidth = table->properties->border;
	full_max_twidth = 0;
	full_min_twidth = 0;
	max_twidth      = 0;
	min_twidth      = 0;
	usable_twidth   = twidth;
	
	for(i = 0; i < ncols; i++)
	{
		/* max_ and min_twidth exclude any spacing */
		max_twidth += max_cols[i];
		min_twidth += min_cols[i];

		/* full_ includes all spacing */
		full_max_twidth += max_cols[i] + hspace + 2*hpad;
		full_min_twidth += min_cols[i] + hspace + 2*hpad;
		usable_twidth   -= (hspace + 2*hpad);
	}
	/* full widths include left & right borders as well */
	full_max_twidth += 2*bwidth;
	full_min_twidth += 2*bwidth;
	usable_twidth   -= 2*bwidth;

#ifdef DEBUG
	_XmHTMLDebug(17, ("layout.c: Computed table dimensions:\n"));
	_XmHTMLDebug(17, ("layout.c: twidth     = %i\n", twidth));
	_XmHTMLDebug(17, ("layout.c: max_twidth = %i\n", max_twidth));
	_XmHTMLDebug(17, ("layout.c: min_twidth = %i\n", min_twidth));
	_XmHTMLDebug(17, ("layout.c: full_max_twidth = %i\n", full_max_twidth));
	_XmHTMLDebug(17, ("layout.c: full_min_twidth = %i\n", full_min_twidth));
	_XmHTMLDebug(17, ("layout.c: max_theight= %i\n", max_theight));
	_XmHTMLDebug(17, ("layout.c: Column widths:\n"));

	for(i = 0; i < ncols; i++)
	{
		_XmHTMLDebug(17, ("layout.c: column %i, min_width = %i, max_width = "
			"%i\n", i, min_cols[i], max_cols[i]));
	}
	_XmHTMLDebug(17, ("layout.c: Row heights:\n"));
	for(i = 0; i < nrows; i++)
	{
		_XmHTMLDebug(17, ("layout.c: row %i, height = %i\n", i, rows[i]));
	}
#endif

	/*****
	* Step Four: compute column widths.
	*****/
	/*****
	* case 1: minimum width equal or wider than total width
	* For nested tables, twidth can be -1 if the table with wasn't set
	* to an absolute number. In this case set all columns to their minimum
	* width.
	*****/
	if(twidth == -1 || full_min_twidth >= twidth)
	{
#ifdef DEBUG
		if(twidth == -1)
		{
			_XmHTMLDebug(17, ("layout.c: twidth unknown, using minimum "
				"column widths.\n"));
			twidth = full_min_twidth;
		}
		else
			_XmHTMLDebug(17, ("layout.c: full_min_twidth > twidth\n"));
#endif
		if(twidth == -1)
			twidth = full_min_twidth;

		/* assign each column it's minimum width */
		for(i = 0; i < ncols; i++)
			max_cols[i] = min_cols[i];

		/* maximum table width */
		max_twidth = min_twidth;
		full_max_twidth = twidth;
		
		/* needs a re-evaluation of row heights */
		needs_height_adjustment = True;
	}
	/* case 2: maximum width less than total width */
	else if(full_max_twidth < twidth)
	{
		_XmHTMLDebug(17, ("layout.c: full_max_twidth < twidth\n"));

		/*****
		* re-evaluation of cell heights only required if col or row spanning
		* is in effect. If so, the appropriate flag has already been set
		* in the above logic.
		*****/

		/*****
		* When a table has an absolute width (table->width nonzero), we
		* stretch all columns so the available width is used up
		* almost entirely (roundoff errors).
		*****/
		if(table->width)
		{
			min_twidth = 0;
			full_min_twidth = 0;
			for(i = 0; i < ncols; i++)
			{
				/* compute width percentage used by this column */
				float pwidth = (float)max_cols[i]/(float)max_twidth;
				max_cols[i] = (int)(pwidth*usable_twidth);
				min_twidth += max_cols[i];
				full_min_twidth += max_cols [i] + hspace + 2*hpad;
			}
			/* save new table width */
			max_twidth = min_twidth;
			full_max_twidth = full_min_twidth;
		}
	}
	/* case 3: max width exceeds available width while min width fits.*/
	else
	{
		int nloop = 0;

		/*****
		* Loop this until the table fits the available width or we
		* exceed the allowed no of iterations.
		*****/
		while(full_max_twidth > twidth && nloop < MAX_TABLE_ITERATIONS)
		{
			/* Difference between available space and minimum table width */
			float w_diff = (float)(usable_twidth - min_twidth);

			/* Difference between maximum and minimum table width */
			float m_diff = (float)(max_twidth - min_twidth);

			/* prevent divide by zero */
			if(m_diff == 0.0)
				m_diff = 1.0;

			_XmHTMLDebug(17, ("layout.c: min_twidth < usable_twidth && "
				"max_twidth > usable_twidth\n"));
			_XmHTMLDebug(17, ("layout.c: min_twidth = %i, max_twidth = %i, "
				"usable_twidth = %i\n", min_twidth, max_twidth, usable_twidth));
			_XmHTMLDebug(17, ("layout.c: w_diff = %.3f, m_diff = %.3f\n",
				w_diff, m_diff));

			/*****
			* For each column, get the difference between minimum and maximum
			* column width and scale using the above differences.
			*****/
			max_twidth = 0;
			full_max_twidth = 0;
			for(i = 0; i < ncols; i++)
			{
				float c_diff = max_cols[i] - min_cols[i];
				max_cols[i]  = min_cols[i] + (int)(c_diff * (w_diff/m_diff));

				/* update maximum width: add spacing */
				max_twidth += max_cols[i];
				full_max_twidth += max_cols[i] + hspace + 2*hpad;

				_XmHTMLDebug(17, ("layout.c: Column %i, c_diff = %.3f, "
					"width = %i\n", i, c_diff, max_cols[i]));
			}
			nloop++;
		}
		if(nloop == MAX_TABLE_ITERATIONS)
		{
			_XmHTMLWarning(__WFUNC__(NULL, "SetTable"),
				"Cell Dimension: bailing out after %i iterations\n"
				"    (near line %i in input)", MAX_TABLE_ITERATIONS,
				table->start->object->line);
		}
		/* needs a re-evaluation of row heights */
		needs_height_adjustment = True;
	}

	/*****
	* Step Five: recompute row heights if required.
	* For a number of cells, the width will be less than the maximum cell
	* width and thus lines will be broken. If we don't recompute the
	* required box height, the table will overflow vertically.
	* As we are recomputing the cell dimensions, we must substract the
	* horizontal spacing we added above.
	*****/
	if(needs_height_adjustment)
	{
		_XmHTMLDebug(17, ("layout.c: adjusting cell heights:\n"));

		for(i = 0; i < nrows; i++)
		{
			int row_max_height = 0;

			row = &(table->rows[i]);
			for(j = 0; j < ncols; j++)
			{
				int row_max_width  = 0;

				/* skip if this is a spanned cell */
				if((idx = boxes[i][j].idx) != -1)
					cell = &(row->cells[idx]);
				else
				{
					_XmHTMLDebug(17,("Cell(%i,%i,%i): spanned\n",
						depth, i, j));
					continue;
				}

				/* box to wide, will be broken */
				if(boxes[i][j].width > max_cols[j] ||
					boxes[i][j].height == -1 ||
					boxes[i][j].width  == -1)
				{
					/* offsets unused */
					boxes[i][j].x       = 0;
					boxes[i][j].y       = 0;
					boxes[i][j].left    = 0;
					boxes[i][j].right   = 0;

					/* set margins */
					boxes[i][j].lmargin = 0;

					/* set right margin */
					if(cell->colspan == 1)
						boxes[i][j].rmargin = max_cols[j];
					else
					{
						/* spans multiple columns, add up column widths */
						int k;
						boxes[i][j].rmargin = 0;
						for(k = j; k < j + cell->colspan && k < ncols; k++)
							boxes[i][j].rmargin += max_cols[k];
					}
					boxes[i][j].width = boxes[i][j].rmargin -
											boxes[i][j].lmargin;
					boxes[i][j].height= -1;

					PreComputeTableLayout(html, &boxes[i][j], cell->start,
						cell->end);

					_XmHTMLDebug(17,("Cell(%i,%i,%i): "
						"width = %i, height = %i\n", depth, i, j,
						boxes[i][j].width, boxes[i][j].height));
				}
				else
					_XmHTMLDebug(17, ("Cell(%i,%i,%i): no change\n",
						depth, i, j));

				/* update maximum row width */
				row_max_width += boxes[i][j].width;

				/* update maximum row height, taking spacing into account */
				if(cell->rowspan == 1 && row_max_height < boxes[i][j].height)
					row_max_height = boxes[i][j].height;

				/*****
				* Update table width if we're exceeding it, which should
				* not be really happening as the cell width will only
				* decrease: each cell already has it's minimum width.
				*****/
				if(max_twidth < row_max_width)
				{
					_XmHTMLDebug(17, ("layout.c: updating maximum table "
						"width from %i to %i\n", max_twidth, row_max_width));
					max_twidth = row_max_width;
				}
			}
			rows[i] = row_max_height;
		}
#ifdef DEBUG
		for(i = 0; i < nrows; i++)
		{
			_XmHTMLDebug(17, ("layout.c: row %i, height = %i\n", i, rows[i]));
		}
#endif
	}
	/*****
	* Step Six: adjust row heights to account for rowspan attributes.
	*
	* Each (filled) cell has it's dimensions set. We now need to adjust
	* the row heights to properly account for any rowspan attributes set.
	* The way we do this is as follows: for each row, check if it contains
	* a cell with the rowspan attribute set and get the one which spans the
	* most rows. When found, compute the total height of all spanned rows
	* and then compute the difference between this height and the height of
	* the spanning cell. If this difference is negative, we can keep the
	* current row heights. If it's positive however, we distribute this
	* difference evenly accross the height of all spanned rows.
	*****/
	if(needs_height_adjustment)
	{
		_XmHTMLDebug(17, ("layout.c: rowspan in effect, re-adjusting row "
			"heights :\n"));

		for(i = 0; i < nrows; i++)
		{
			int max_span = 1;
			int span_cell = -1;

			row = &(table->rows[i]);
			for(j = 0; j < ncols; j++)
			{
				/* skip if this is a spanned cell */
				if((idx = boxes[i][j].idx) != -1)
					cell = &(row->cells[idx]);
				else
					continue;
				if(cell->rowspan > max_span)
				{
					max_span = cell->rowspan;
					span_cell = j;
				}
			}
			if(span_cell != -1 && span_cell < ncols)
			{
				/* height of spanning cell */
				int max_h = boxes[i][span_cell].height;

				/* compute height of all spanned rows */
				int span_h = 0;

				for(k = i; k < nrows && k < i + max_span; k++)
					span_h += rows[k];

				/* spanned height greater than occupied height, adjust rows */
				if((max_h - span_h) > 0)
				{
					int extra = (max_h - span_h)/(float)max_span;
					for(k = i; k < nrows && k < i + max_span; k++)
						rows[k] += extra;
				}
			}
		}
#ifdef DEBUG
		for(i = 0; i < nrows; i++)
		{
			_XmHTMLDebug(17, ("layout.c: row %i, height = %i\n", i, rows[i]));
		}
#endif
	}

	_XmHTMLDebug(17, ("layout.c: ------ Table Precomputation End -----\n"));

	/*****
	* Step Seven: assign box dimensions for each cell.
	*****/
	_XmHTMLDebug(17, ("layout.c: Final Cell Dimensions:\n"));

	/*****
	* Check horizontal *table* alignment and compute initial
	* horizontal offset.
	*****/
	_XmHTMLDebug(17, ("layout.c: checking horizontal alignment.\n"));
	_XmHTMLDebug(17, ("layout.c: x_start = %i, max_twidth = %i, "
		"twidth = %i\n", x_start, max_twidth, twidth));

	switch((table->properties->halign == XmHALIGN_NONE ?
		html->html.alignment : table->properties->halign))
	{
		case XmHALIGN_RIGHT:
			x_start += twidth > full_max_twidth ?
						twidth - full_max_twidth : 0;
			break;
		case XmHALIGN_CENTER:
			x_start += twidth > full_max_twidth ?
						(twidth - full_max_twidth)/2 : 0;
			break;
		case XmHALIGN_LEFT:		/* computation is always left-oriented */
		case XmHALIGN_JUSTIFY:	/* useless for tables */
		default:
			break;
	}
	_XmHTMLDebug(17, ("layout.c: computed x_start to be %i\n", x_start));

	max_theight = 0;
	max_twidth  = 0;

	/* adjust upper left corner of table */
	x_start += bwidth;
	y_pos = box->y + bwidth;

	for(i = 0; i < nrows; i++)
	{
		int tw = 0;

		/* pick up current row */
		row = &(table->rows[i]);

		/* top-left row positions */
		x_pos = x_start;
		row->owner->x = x_pos;
		row->owner->y = y_pos;

		for(j = 0; j < ncols; j++)
		{
			int cwidth = 0, cheight = 0;

			/* pick up current cell if not a spanned cell */
			if((idx = boxes[i][j].idx) != -1)
				cell = &(row->cells[idx]);

			/* initial start positions for this box */
			boxes[i][j].x = x_pos + hspace;
			boxes[i][j].y = y_pos + vspace + vpad;

			/*****
			* Set correct left & right margin
			* Left & right margin take horizontal spacing into account.
			*****/
			boxes[i][j].lmargin = x_pos + hpad + hspace;
			if(idx == -1 || cell->colspan == 1)
			{
				/* set right margin */
				boxes[i][j].rmargin = boxes[i][j].lmargin + max_cols[j] + hpad;

				/* total cell width */
				cwidth = max_cols[j] + 2 * hpad + hspace;
			}
			else
			{
				/* spans multiple columns, add up column sizes */
				int k;
				boxes[i][j].rmargin = boxes[i][j].lmargin;
				for(k = j; k < j + cell->colspan && k < ncols; k++)
				{
					/* left & right padding */
					boxes[i][j].rmargin += (max_cols[k] + 2*hpad);

					/* total cell width */
					cwidth += max_cols[k] + 2 * hpad + hspace;
				}
				/* above loop adds one to many */
				boxes[i][j].rmargin -= hpad;
			}

			/* set available cell width */
			boxes[i][j].width = boxes[i][j].rmargin - boxes[i][j].lmargin;
			boxes[i][j].left  = boxes[i][j].lmargin;
			boxes[i][j].right = boxes[i][j].rmargin;

			/* Set correct cell height. */
			if(idx == -1 || cell->rowspan == 1)
			{
				boxes[i][j].height = rows[i] + 2*vpad;
				cheight = boxes[i][j].height + vspace;
			}
			else
			{
				/* spans multiple rows, add up the row heights it occupies */
				int k;
				boxes[i][j].height = 0;
				for(k = i; k < i + cell->rowspan && k < nrows; k++)
				{
					boxes[i][j].height += rows[k] + 2*vpad;
					cheight += (rows[k] + 2*vpad + vspace);
				}

			}
			/* set vertical margins */
			boxes[i][j].tmargin = vpad;
			boxes[i][j].bmargin = boxes[i][j].height;

			/*****
			* Store bounding box dimensions for proper frame rendering, but
			* *never* do this if the current cell is a spanned one. If we
			* would do this the offsets would be horribly wrong...
			*****/
			if(idx != -1)
			{
				cell->owner->x = x_pos;
				cell->owner->y = y_pos;
				cell->owner->width  = cwidth;
				cell->owner->height = cheight;
			}

			/*****
			* advance x position to next column. Must include any padding &
			* spacing.
			*****/
			x_pos += (max_cols[j] + 2*hpad + hspace + (bwidth ? 1 : 0));
			tw += (max_cols[j] + 2*hpad + hspace + (bwidth ? 1 : 0));

			_XmHTMLDebug(17, ("Cell(%i,%i,%i): x = %i, y = %i, w = %i, "
				"h = %i, left = %i, right = %i %s\n", depth, i, j,
				boxes[i][j].x, boxes[i][j].y, boxes[i][j].width,
				boxes[i][j].height, boxes[i][j].left, boxes[i][j].right,
				idx == -1 ? "(spanned)" : ""));
		}
		/* update max_width if necessary */
		if(x_pos > max_width)
		{
			/* adjust maximum document width */
			max_width = x_pos;
			_XmHTMLDebug(17, ("layout.c: adjusted max_width to %i\n",
				max_width));
		}
		if(max_twidth < tw)
			max_twidth = tw;

		/* move to next row, row height already includes padding */
		y_pos += rows[i] + 2*hpad + vspace;
		max_theight += rows[i] + 2*hpad + vspace;

		/* save row dimensions */
		row->owner->width = x_pos - row->owner->x;
		row->owner->height = y_pos - row->owner->y;
	}
	max_twidth  += 2*bwidth;
	max_theight += 2*bwidth;

	/* Final table height */
#ifdef DEBUG
	if(max_theight != (y_pos - box->y))
	{
		_XmHTMLWarning(__WFUNC__(html, "SetTable"), "adjusted table height "
			"from %i to %i", max_theight, y_pos - box->y);
		max_theight = y_pos - box->y;
	}
	/* Final table height */
	if(full_max_twidth != max_twidth)
	{
		_XmHTMLWarning(__WFUNC__(html, "SetTable"), "adjusted table width "
			"from %i to %i", full_max_twidth, max_twidth);
		full_max_twidth = max_twidth;
	}
#else
	max_theight = y_pos - box->y;
	full_max_twidth = max_twidth;
#endif

	/* restore line count */
	line = save_line;

	/*****
	* Step Eight: compute real text layout using the computed box dimensions.
	*****/
	for(i = 0; i < table->nrows; i++)
	{
		row = &(table->rows[i]);

		/* restore line count for each row */
		line = save_line;

		max_line = 0;

		/* layout all cells in this row */
		for(j = 0; j < ncols; j++)
		{
			/* skip if this is a spanned cell */
			if((idx = boxes[i][j].idx) != -1)
				cell = &(row->cells[idx]);
			else
				continue;

			/* same line count for each cell */
			line = save_line;

			_XmHTMLDebug(17, ("layout.c: Cell(%i,%i,%i): computing final "
				"layout for box at (%i,%i) with size %ix%i\n", depth, i, j,
				boxes[i][j].x, boxes[i][j].y,
				boxes[i][j].width, boxes[i][j].height));

			/* compute final layout for the current cell */
			ComputeTableLayout(html, &boxes[i][j], cell->start, cell->end);

			_XmHTMLDebug(17, ("layout.c: Cell(%i,%i,%i): done computing final "
				"layout\n", depth, i, j));

			/* adjust cell contents vertically if not aligned at cell top */
			if(cell->properties->valign != XmVALIGN_TOP)
				CheckVerticalAlignment(html, &boxes[i][j], cell->start,
					cell->end, cell->properties->valign);

			/* store maximum line count in this row */
			if(max_line < (save_line - line))
				max_line = (save_line - line);
		}
		/* row done, adjust linecount */
		save_line += max_line;
	}
	/* all done, set correct linenumber */
	line = save_line;

	/* All done, free the allocated boxes */
	for(i = 0; i < table->nrows; i++)
	{
		/* all cells in this row */
		free(boxes[i]);
	}
	free(boxes);

	/* free row & column size storage */
	free(rows);
	free(max_cols);
	free(min_cols);

	/* store return dimensions, box->x is not touched */
	box->y += max_theight;
	table->end->height = max_theight;

	/*****
	* update x position of owning object, it might have shifted due to
	* horizontal alignment adjustment at table level.
	*****/
	data->x = x_start - bwidth;
	data->y -= bwidth;
	/* final (absolute) table dimensions */
	data->height = max_theight;
	data->width  = full_max_twidth;

	/* adjust maximum document width */
	if(box->x + full_max_twidth > max_width)
	{
		max_width = box->x + full_max_twidth;
		_XmHTMLDebug(17, ("layout.c: adjusted max_width to %i\n", max_width));
	}

	_XmHTMLDebug(17, ("layout.c: table at depth %i finished, table width: %i, "
		"table height: %i\n", depth, max_twidth, max_theight));
	_XmHTMLDebug(17, ("layout.c: next box starts at: %i, %i\n",
		box->x, box->y));

	depth--;

	_XmHTMLDebug(17, ("layout.c: ------ Table Layout Ends -----\n"));

	/* all done! */
	return(table->end);
}

static void
PreComputeTableLayout(XmHTMLWidget html, PositionBox *parent,
	XmHTMLObjectTableElement obj_start, XmHTMLObjectTableElement obj_end)
{
	XmHTMLObjectTableElement tmp, end;
	int max_width_save;
	PositionBox box, box_return;
	int y_start = parent->y;

	memcpy(&box, parent, sizeof(PositionBox));
	memcpy(&box_return, parent, sizeof(PositionBox));

	/*****
	* Save current max_width, it's possible it will be changed in
	* SetText (or any of the routines it calls) but if it does it's a bogus
	* value (width is ignored when precomputing cell widths).
	*****/
	max_width_save = max_width;
	had_break = False;
	baseline_obj = NULL;
	box.y = 0;
	box.x = 0;

	for(tmp = obj_start; tmp && tmp != obj_end; tmp = tmp->next)
	{
		switch(tmp->object_type)
		{
			case OBJ_TEXT:
				/* collect all words */
				for(end = tmp; end->next->object_type == OBJ_TEXT; 
					end = end->next);

				/* go and do text layout */
				SetText(html, &box, tmp, end->next, False, True);

				/* back up one element */
				tmp = end;
				break;

			case OBJ_PRE_TEXT:
				/* collect all words */
				for(end = tmp; end->next->object_type == OBJ_PRE_TEXT;
					end = end->next);

				/* go and do text layout */
				SetText(html, &box, tmp, end->next, True, True);

				/* back up one element */
				tmp = end;
				break;
			case OBJ_BULLET:
				SetBullet(html, &box, tmp);
				break;
			case OBJ_HRULE:
				SetRule(html, &box, tmp);
				break;
			case OBJ_TABLE:
				SetBlock(html, &box, tmp);
				tmp = SetTable(html, &box, tmp);
				break;
			case OBJ_TABLE_FRAME:
				break;
			case OBJ_APPLET:
				SetApplet(html, &box, tmp);
				SetBreak(html, &box, tmp);
				break;
			case OBJ_BLOCK:
				SetBlock(html, &box, tmp);
				SetBreak(html, &box, tmp);
				break;
			case OBJ_NONE:
				SetBlock(html, &box, tmp);
				break;
			default:
				_XmHTMLWarning(__WFUNC__(html, "PreComputeLayout"), 
					"Unknown object type!");
		}
		/* store maximum box width */
		if(box_return.width < box.width)
			box_return.width = box.width;
		/* store minimum box width (ignore empty boxes) */
		if(box.min_width > 0 && box_return.min_width < box.min_width)
			box_return.min_width = box.min_width;
	}
	/* Done precomputing, update return values */

	/* maximum box width */
	if(box_return.width > parent->lmargin && box_return.width != -1)
		parent->width = box_return.width - parent->lmargin;
	else if(box_return.width != -1)	/* empty cells/rows */
		parent->width = box_return.width;
	else
		parent->width = parent->lmargin;

	/* minimum box width */
	if(box_return.min_width > parent->lmargin)
		parent->min_width = box_return.min_width - parent->lmargin;
	else
		parent->min_width = box_return.min_width;

	/*****
	* Update box height if the computed height is larger than the set
	* height (can only happen for cells with the height attribute set).
	*****/
	if(box_return.height != -1)
	{
		/* don't ask me how this is possible, but it *does* happen */
		if(box_return.height < 0)
			parent->height = box.y - (y_start + box_return.height);
		else
			if(box.y - y_start > parent->height)
				parent->height = box.y - y_start;
	}
	else
		parent->height = box.y - y_start;

	box.y = 0;
	box.x = 0;

	/* and restore max_width */
	max_width = max_width_save;
}

static void
ComputeTableLayout(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement obj_start, XmHTMLObjectTableElement obj_end)
{
	XmHTMLObjectTableElement tmp, end;
	int max_width_save;

	/*****
	* max_width also to be restored, it's final value is governed by the
	* total width of the enclosing table.
	* We also *must* reset the baseline object: it only applies *per*
	* box (= cell). Not resetting it would transfer the baseline object
	* to another cell, which is not very desirable...
	*****/
	max_width_save = max_width;
	had_break = False;
	baseline_obj = NULL;

	for(tmp = obj_start; tmp && tmp != obj_end; tmp = tmp->next)
	{
		switch(tmp->object_type)
		{
			/* collect all words */
			case OBJ_TEXT:
				for(end = tmp; end->next->object_type == OBJ_TEXT; 
					end = end->next);

				/* go and do text layout */
				SetText(html, box, tmp, end->next, False, False);

				/* back up one element */
				tmp = end;
				break;

			case OBJ_PRE_TEXT:
				for(end = tmp; end->next->object_type == OBJ_PRE_TEXT;
					end = end->next);

				/* go and do text layout */
				SetText(html, box, tmp, end->next, True, False);

				/* back up one element */
				tmp = end;
				break;
			case OBJ_BULLET:
				SetBullet(html, box, tmp);
				break;
			case OBJ_HRULE:
				SetRule(html, box, tmp);
				break;
			case OBJ_TABLE:
				/* nested table hehehehehehe */
				SetBlock(html, box, tmp);
				tmp = SetTable(html, box, tmp);
				break;
			case OBJ_TABLE_FRAME:
				SetBlock(html, box, tmp);
				break;
			case OBJ_APPLET:
				SetApplet(html, box, tmp);
				SetBreak(html, box, tmp);
				break;
			case OBJ_BLOCK:
				SetBlock(html, box, tmp);
				SetBreak(html, box, tmp);
				break;
			case OBJ_NONE:
				SetBlock(html, box, tmp);
				break;
			default:
				_XmHTMLWarning(__WFUNC__(html, "PreComputeLayout"), 
					"Unknown object type!");
		}
	}
	max_width = max_width_save;
}

static void
CheckVerticalAlignment(XmHTMLWidget html, PositionBox *box,
	XmHTMLObjectTableElement start, XmHTMLObjectTableElement end,
	Alignment valign)
{
	/*****
	* INSERT CODE
	* to shift all objects down
	*****/
}
