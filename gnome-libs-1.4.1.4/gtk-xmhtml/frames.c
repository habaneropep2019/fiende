/*****
* frames.c : XmHTML frame support
*
* This file Version	$Revision: 1.15 $
*
* Creation date:		Tue Mar 25 18:53:12 GMT+0100 1997
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
* $Log: frames.c,v $
* Revision 1.15  1999/07/29 01:26:28  sopwith
*
*
* Fix all warnings.
*
* Revision 1.14  1998/02/12 03:08:57  unammx
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
* Revision 1.13  1998/01/06 04:48:39  unammx
* More forms work, today we got:
*
* 	Input file selectors.
* 	Listboxes.
* 	Option menus.
* 	Working input lines.
*
* Missing:
*
* 	Text entries (really, really, really trivial).
*
* Enjoy,
* Miguel.
*
* Revision 1.12  1997/12/31 20:22:31  unammx
* Frames work.
*
* There is a bug in the frame engine from Koen, it is not a bug of the
* port anymore, hopefully it will get fixed soonish.
*
* Miguel.
*
* Revision 1.11  1997/12/31 05:20:55  unammx
* Frames work.  Resizing a window still does not propagate to child
* frames, will fix that next.
*
* Elliot: check how the frames are loaded, you need to attach to the
* frame signal and load the contents when the widget asks for them.
*
* Miguel.
*
* Revision 1.10  1997/12/30 03:32:52  unammx
* More work on getting the frames working, still some bits are missing - Miguel
*
* Revision 1.9  1997/12/29 22:16:28  unammx
* This version does:
*
*    - Sync with Koen to version Beta 1.1.2c of the XmHTML widget.
*      Includes various table fixes.
*
*    - Callbacks are now properly checked for the Gtk edition (ie,
*      signals).
*
* Revision 1.8  1997/12/29 03:17:10  sopwith
* amessagetosatisfyCVS
*
* Revision 1.7  1997/12/28 17:45:51  unammx
* Make it compile
*
* Revision 1.6  1997/12/27 20:58:18  unammx
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
* Revision 1.5  1997/12/26 21:03:32  sopwith
* A few miscellaneous XmHTML bug fixes, including a note to miguel so he can fix frames ;-)
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
* Revision 1.3  1997/12/24 17:53:54  unammx
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
* Revision 1.1  1997/12/16 00:34:49  unammx
* More progress on the XmHTML work.  This time, I did frames.c, not
* as nice as I would like it to be.
*
* It still does not link.
*
* Revision 1.8  1997/10/23 00:25:01  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.7  1997/08/31 17:35:37  newt
* Several fixes in form creation & destruction and widget reuse. kd & rr
*
* Revision 1.6  1997/08/30 01:04:16  newt
* _XmHTMLWarning proto & color changes: XmHTML now uses manager's color fields.
*
* Revision 1.5  1997/08/01 13:01:40  newt
* my_strdup -> strdup, minor bugfixes and updated comments.
*
* Revision 1.4  1997/05/28 01:48:13  newt
* Sped up _XmHTMLCheckForFrames considerably.
*
* Revision 1.3  1997/04/29 14:27:00  newt
* Header files modifications.
*
* Revision 1.2  1997/04/03 05:35:36  newt
* Changed default name from _top to _frame appended with a number
*
* Revision 1.1  1997/03/28 07:02:46  newt
* Initial Revision
*
*****/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"
#include "toolkit.h"

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/
/* how many times may we retry frame alignment? */
#define MAX_FRAME_ITERATIONS	100
#define ROW	1
#define COL	2
#define ROW_DONE (1<<1)
#define COL_DONE (1<<2)

/*** Private Datatype Declarations ****/
/* definition of a HTML frameset */
typedef struct _frameSet{
	int type;					/* type of this set, either ROW or COL */
	int border;					/* frame border value */
	int *sizes;					/* array of child sizes */
	int curr_x;					/* current x insertion position */
	int curr_y;					/* current y insertion position */
	FrameSize *size_types;		/* array of possible size specifications */
	int nchilds;				/* max no of childs */
	int childs_done;			/* no of childs processed so far */
	int insert_pos;				/* insertion position of current child */
	struct _frameSet *parent;	/* parent frameset of this frameset */
	struct _frameSet *childs;	/* list of childs */
	struct _frameSet *next;		/* ptr to next frameSet */
}frameSet;

/* stack of framesets */
typedef struct _frameStack{
	frameSet *frame_set;
	struct _frameStack *next;
}frameStack;

/*** Private Function Prototype Declarations ****/
static frameSet *popFrameSet(void);
static frameSet *doFrameSet(String attributes);
static void insertFrameSetChild(frameSet *parent, frameSet *child);
static void insertFrameChild(frameSet *current_set, XmHTMLFrameWidget *frame);
static void pushFrameSet(frameSet *frame_set);
static void makeFrameSets(XmHTMLWidget html, XmHTMLObject *frameset);
static void destroyFrameSets(frameSet *set);
static XmHTMLFrameWidget *doFrame(XmHTMLWidget html, String attributes);
static void mapFrames(XmHTMLWidget html);
static void adjustConstraints(XmHTMLWidget html);
static void frameDoneCallback(XmHTMLWidget html, XmHTMLFrameWidget *frame, 
	TWidget widget);

/*** Private Variable Declarations ***/
static int current_frame;		/* running frame counter */
static frameSet *frame_sets;	/* list of all framesets processed */
static frameStack frame_base, *frame_stack;

/*****
* Name: 		doFrameSet
* Return Type: 	frameSet*
* Description: 	creates and fills a frameSet structure with the info in it's
*				attributes
* In: 
*	attributes:	attributes for this frameset
* Returns:
*	a newly created frameset.
* Note:
*	this routine inserts each frameset it creates in a linked list which
*	is used for stack purposes.
*****/
static frameSet*
doFrameSet(String attributes)
{
	frameSet *list, *tmp;
	String chPtr, tmpPtr, ptr;
	int i;

	/* create new entry */
	list = (frameSet*)malloc(sizeof(frameSet));
	(void)memset(list, 0, sizeof(frameSet));

	list->type = ROW;

	if((chPtr = _XmHTMLTagGetValue(attributes, "rows")) == NULL)
	{
		if((chPtr = _XmHTMLTagGetValue(attributes, "cols")) == NULL)
			return(frame_sets);
		else
			list->type = COL;
	}

	/*
	* count how many childs this frameset has: the no of childs is given by
	* the no of entries within the COLS or ROWS tag
	* Note that childs can be frames and/or framesets as well.
	*/
	for(tmpPtr = chPtr; *tmpPtr != '\0'; tmpPtr++)
		if(*tmpPtr == ',')
			list->nchilds++;
	list->nchilds++;

	list->sizes = (int*)calloc(list->nchilds, sizeof(int));
	list->size_types = (FrameSize*)calloc(list->nchilds, sizeof(FrameSize));
	list->childs = (frameSet*)calloc(list->nchilds, sizeof(frameSet));

	/*
	* get dimensions: when we encounter a ``*'' in a size definition it
	* means we are free to choose any size we want. When its a number 
	* followed by a ``%'' we must choose the size relative against the total
	* width of the render area. When it's a number not followed by anything
	* we have an absolute size.
	*/
	tmpPtr = ptr = chPtr;
	i = 0;
	while(True)
	{
		if(*tmpPtr == ',' || *tmpPtr == '\0')
		{
			if(*(tmpPtr-1) == '*')
				list->size_types[i] = FRAME_SIZE_OPTIONAL;
			else if(*(tmpPtr-1) == '%')
				list->size_types[i] = FRAME_SIZE_RELATIVE;
			else
				list->size_types[i] = FRAME_SIZE_FIXED;

			list->sizes[i++] = atoi(ptr);

			if(*tmpPtr == '\0')
				break;
			ptr = tmpPtr+1;
		}
		tmpPtr++;
		/* sanity */
		if(i == list->nchilds)
			break;
	}
	free(chPtr);

	/*
	* Frame borders can be specified by both frameborder or border, they
	* are equal.
	*/
	if((chPtr = _XmHTMLTagGetValue(attributes, "frameborder")) != NULL)
	{
		/*
		* Sigh, stupid Netscape frameset definition allows a tag to have
		* a textvalue or a number.
		*/
		if(!(strcasecmp(chPtr, "no")) || *chPtr == '0')
			list->border = 0;
		else
			list->border = atoi(chPtr);
		free(chPtr);
	}
	else
		list->border = _XmHTMLTagGetNumber(attributes, "border", 5);

	/* insert this new frame in the overal frameset list. */
	if(frame_sets == NULL)
		frame_sets = list;
	else
	{
		for(tmp = frame_sets; tmp != NULL && tmp->next != NULL; 
			tmp = tmp->next);
		tmp->next = list;
	}
	return(list);
}

/*****
* Name: 		doFrame
* Return Type: 	XmHTMLFrameWidget*
* Description: 	fills a HTML frame structure with data from it's attributes
* In: 
*	html:		XmHTMLWidget id;
*	attributes:	frame attributes
* Returns:
*	updated frame
* Note:
*	this routine takes the frame to update from an already allocated list
*	of frames and increments the running frame counter when it returns.
*****/
static XmHTMLFrameWidget*
doFrame(XmHTMLWidget html, String attributes)
{
	XmHTMLFrameWidget *frame;
	String chPtr;

	frame = html->html.frames[current_frame];

	/* default frame sizing */
	frame->height_type = FRAME_SIZE_FIXED;
	frame->width_type = FRAME_SIZE_FIXED;

	/* pick up all remaining frame attributes */
	frame->src = _XmHTMLTagGetValue(attributes, "src");

	/* get frame name, default to _frame if not present */
	if((frame->name = _XmHTMLTagGetValue(attributes, "name")) == NULL)
	{
		char buf[24];
		sprintf(buf, "_frame%i", current_frame);
		frame->name = strdup(buf);
	}

	frame->margin_width = (Dimension)_XmHTMLTagGetNumber(attributes,
		"marginwidth", 5);
	frame->margin_height = (Dimension)_XmHTMLTagGetNumber(attributes,
		"marginheight", 5);
	if(!frame->margin_width)
		frame->margin_width = html->html.margin_width;
	if(!frame->margin_height)
		frame->margin_height = html->html.margin_height;

	/*
	* This is useless as we don't support frame resizing. I think this is
	* a thing the caller must be able to do. A possible way could be to
	* overlay the render area with a PanedWidget and store these HTML widgets
	* as childs of this paned widget.
	*/
	frame->resize = !_XmHTMLTagCheck(attributes, "noresize");

	frame->scroll_type = FRAME_SCROLL_AUTO;

	if((chPtr = _XmHTMLTagGetValue(attributes, "scrolling")) != NULL)
	{
		if(!(strcasecmp(chPtr, "yes")))
			frame->scroll_type = FRAME_SCROLL_YES;
		else if(!(strcasecmp(chPtr, "no")))
			frame->scroll_type = FRAME_SCROLL_NONE;
		free(chPtr);
	}

	_XmHTMLDebug(11, ("frames.c: doFrame, frame %i created\n"
		"\tname: %s\n"
		"\tsrc : %s\n"
		"\tmargin width : %i\n"
		"\tmargin height: %i\n"
		"\tresize       : %s\n"
		"\tscrolling    : %s\n", current_frame, frame->name,
		frame->src ? frame->src : "<none>", frame->margin_width,
		frame->margin_height, frame->resize ? "yes" : "no",
		frame->scroll_type == FRAME_SCROLL_AUTO ? "auto" :
		(frame->scroll_type == FRAME_SCROLL_YES ? "always" : "none")));
		
	/*
	* Actual widget creation is postponed until the very last moment
	* of _XmHTMLCreateFrames
	*/

	/* increment running frame counter */
	current_frame++;
	return(frame);
}

/*****
* Name: 		insertFrameSetChild
* Return Type: 	void
* Description: 	inserts a child frameset in it's parent list
* In: 
*	parent:		parent of this frameset
*	child:		obvious
* Returns:
*	nothing
*****/
static void
insertFrameSetChild(frameSet *parent, frameSet *child)
{
	if(parent && parent->childs_done < parent->nchilds)
	{
		int idx = parent->childs_done;

		child->parent = parent;
		child->insert_pos = idx;

		parent->childs[parent->childs_done] = *child;
		parent->childs_done++;
	}
}

/*****
* Name: 		insertFrameChild
* Return Type: 	void
* Description: 	sets the geometry constraints on a HTML frame
* In: 
*	frame_set:	frameset parent of this frame;
*	frame:		frame for which to set the constraints
* Returns:
*	nothing, but frame is updated.
*****/
static void
insertFrameChild(frameSet *frame_set, XmHTMLFrameWidget *frame)
{
	frameSet *parent = frame_set->parent;
	int insert_pos = frame_set->childs_done;

	if(frame_set->type == COL)
	{
		frame->width_s = frame_set->sizes[insert_pos];
		frame->width_type = frame_set->size_types[insert_pos];
		frame->xs = frame_set->curr_x; 
		/* advance to next x position */
		frame_set->curr_x += frame->width_s;
		if(parent)
		{
			frame->height_s = parent->sizes[frame_set->insert_pos];
			frame->height_type = parent->size_types[frame_set->insert_pos];
			frame->ys = parent->curr_y;
		}
	}
	else
	{
		frame->height_s = frame_set->sizes[insert_pos];
		frame->height_type = frame_set->size_types[insert_pos];
		frame->ys = frame_set->curr_y;
		/* advance to next y position */
		frame_set->curr_y += frame->height_s;
		if(parent)
		{
			frame->width_s = parent->sizes[frame_set->insert_pos];
			frame->width_type = parent->size_types[frame_set->insert_pos];
			frame->xs = parent->curr_x;
		}
	}
	if(frame->height_s == 0)
		frame->height_type = FRAME_SIZE_OPTIONAL;
	if(frame->width_s == 0)
		frame->width_type = FRAME_SIZE_OPTIONAL;

	/* set additional constraints for this frame */
	frame->border = frame_set->border;

	/* disable resizing if we don't have a border */
	if(!frame->border)
		frame->resize = False;

	frame_set->childs_done++;
}

/*****
* Name: 		pushFrameSet
* Return Type: 	void
* Description: 	pushes a frameset on the stack
* In: 
*	frame_set:	frameset to push
* Returns:
*	nothing
*****/
static void
pushFrameSet(frameSet *frame_set)
{
	frameStack *tmp;

	tmp = (frameStack*)malloc(sizeof(frameStack));
	tmp->frame_set = frame_set;
	tmp->next = frame_stack;
	frame_stack = tmp;
}

/*****
* Name: 		popFrameSet
* Return Type: 	frameSet*
* Description: 	pops a frameset of the stack
* In: 
*	nothing
* Returns:
*	the next frameset on the stack, or NULL when stack is empty
*****/
static frameSet*
popFrameSet(void)
{
	frameStack *tmp;
	frameSet *frame_set;

	if(frame_stack->next)
	{
		tmp = frame_stack;
		frame_stack = frame_stack->next;
		frame_set = tmp->frame_set;
		free(tmp);
		return(frame_set);
	}
	return(NULL);
}

/*****
* Name: 		makeFrameSets
* Return Type: 	void
* Description: 	creates all HTML framesets and sets the geometry constraints
*				on each frame.
* In: 
*	html:		XmHTMLWidget id;
*	frameset:	XmHTMLObject data;
* Returns:
*	nothing
* Note:
*	This routine was *very* difficult to conceive, so don't let the simplicity
*	of it deceive you.
*****/
static void
makeFrameSets(XmHTMLWidget html, XmHTMLObject *frameset)
{
	XmHTMLObject *tmp;
	XmHTMLFrameWidget *frame;
	frameSet *current_set = NULL, *parent_set = NULL;
	int idx = 0, i;

	for(tmp = frameset; tmp != NULL; tmp = tmp->next)
	{
		switch(tmp->id)
		{
			case HT_FRAMESET:
				if(tmp->is_end)
				{
					/* frameset terminated, pop from stack */
					current_set = popFrameSet();
					/*
					* no more sets left on the stack: we've reached the
					* end of the outermost frameset and are done here.
					*/
					if(current_set == NULL)
						return;
					/*
					* if this test evaluates to False, it's a bad frameset
					* spec: it has more childs than originally specified
					* with the ROWS or COLS attribute. We just ignore
					* these things. This normally shouldn't happen as
					* unspecified framesets are skipped when a new
					* frameset is encountered.
					*/
					if((i = current_set->childs_done) < current_set->nchilds)
					{
						if(current_set->type == ROW)
							current_set->curr_y += current_set->sizes[i];
						else
							current_set->curr_x += current_set->sizes[i];
					}
				}
				else
				{
					/* A new frameset, push the current frameset on the stack */
					pushFrameSet(current_set);
					parent_set = frame_stack->frame_set;

					/* Check if we still have room for this thing. */
					if(!parent_set ||
						parent_set->childs_done < parent_set->nchilds)
					{
						/* create a new frameset */
						current_set = doFrameSet(tmp->attributes);
						insertFrameSetChild(parent_set, current_set);
						idx = 0;
					}
					else
					{
						/*
						* No more room available, this is an unspecified
						* frameset, kill it and all childs it might have.
						*/
						int depth = 1;
						int start_line = tmp->line;
						for(tmp = tmp->next; tmp != NULL; tmp = tmp->next)
						{
							if(tmp->id == HT_FRAMESET)
							{
								if(tmp->is_end)
								{
									if(--depth == 0)
										break;
								}
								else	/* child frameset */
									depth++;
							}
						}
						_XmHTMLWarning(__WFUNC__(html, "doFrameSets"),
							"Bad <FRAMESET> tag: missing COLS or ROWS "
							"attribute on parent set\n    Skipped all "
							"frame declarations between line %i and %i "
							"in input", start_line, tmp ? tmp->line : -1);
					}
				}
				break;
			case HT_FRAME:
				/* check if we have room left */
				if(current_set->childs_done < current_set->nchilds)
				{
					/* insert child in current frameset */
					frame = doFrame(html, tmp->attributes);
					insertFrameChild(current_set, frame);
					idx++;
				}
				else
					_XmHTMLWarning(__WFUNC__(html, "doFrameSets"),
						"Bad <FRAME> tag on line %i of input: missing COLS "
						"or ROWS attribute on parent set, skipped.",
						tmp->line);
				/*****
				* Note: </FRAME> doesn't exist. The parser is smart enough
				* to kick these out.
				*****/
				/* fall thru */
			default:
				break;
		}
		if(idx == html->html.nframes)
			return;
	}
}

/*****
* Name: 		adjustConstraints
* Return Type: 	void
* Description: 	computes XmHTML's frame childs positions
* In: 
*	html:		XmHTMLWidget id;
* Returns:
*	nothing, but each frame child has its position and dimension set correctly.
*****/
static void
adjustConstraints(XmHTMLWidget html)
{
	int i, j, k;
	int width, height, max_width, max_height;
	int rel_width, rel_height, max_rel_width, max_rel_height;
	int work_width, work_height;
	int max_x, max_y, curr_x, curr_y;
	int *visited = NULL;
	Byte *done;
	float x_adjust, y_adjust;
	XmHTMLFrameWidget *frame;

	/* this uses the core dimensions */
	work_width  = Toolkit_Widget_Dim (html).width;
	work_height = Toolkit_Widget_Dim (html).height;

	_XmHTMLDebug(11, ("frames.c: adjustConstraints, work_width: %i "
		"work_height: %i\n", work_width, work_height));

	/*
	* first restore all original dimensions 
	*/
	for(i = 0; i < html->html.nframes; i++)
	{
		frame = html->html.frames[i];
		frame->x = frame->xs;
		frame->y = frame->ys;
		frame->width  = frame->width_s;
		frame->height = frame->height_s;
	}
	/*
	* the ``done'' array is an array of two flags for each frame:
	* one flag is set when it's width has been set (ROW_DONE flag) and
	* one when it's height has been set (COL_DONE flag).
	* We can't use a fixed-size array as we don't know how many frames we
	* have in advance.
	*/
	done = (Byte*)calloc(html->html.nframes, sizeof(Byte));

	/*
	* Frame sizes can be specified in the following ways:
	* 1. fixed: frame dimensions are in pixels
	* 2. relative: frame dimensions are given in percentages
	* 3. optional: I get to determine the frame's dimensions
	*
	* We first check all fixed and relative widths and heights and see if
	* their sum doesn't exceed the available dimensions. If it does, the frame
	* dimensions are weighted by the amount their sum exceeds the available
	* dimensions. The same thing applies when the sum stays beneath the
	* available dimensions.
	* This procedure is repeated until all frames fit *exactly* within the
	* available screen dimensions.
	*/
	/* row stride */
	for(i = 0; i < html->html.nframes; i++)
	{
		max_width = max_rel_width = 0;
		width = rel_width = 0;
		max_y = curr_y = 0;
		x_adjust = 0.0;

		frame = html->html.frames[i];

		/* relative or fixed width */
		if(frame->width_type != FRAME_SIZE_OPTIONAL && !(done[i] & ROW_DONE))
		{
			/* y position of this frame */
			curr_y = frame->y;

			if(frame->width_type == FRAME_SIZE_RELATIVE)
				rel_width = frame->width;
			if(frame->width_type == FRAME_SIZE_FIXED)
				width = frame->width;

			for(j = i+1; j < html->html.nframes; j++)
			{
				frame = html->html.frames[j];

				if(frame->y == curr_y)
				{
					if(frame->width_type == FRAME_SIZE_FIXED)
						width += frame->width;
					if(width > max_width)
					{
						max_width = width;
						max_y = curr_y;
					}
					if(frame->width_type == FRAME_SIZE_RELATIVE)
						rel_width += frame->width;
					if(rel_width > max_rel_width)
					{
						max_rel_width = rel_width;
						max_y = curr_y;
					}
				}
			}
			/* compute corrections if we exceed or stay below the margins */
			if(max_rel_width && max_rel_width != 100)
				x_adjust = (float)(100./max_rel_width);

			if(max_width && max_width != work_width)
				x_adjust = (float)(work_width/(float)max_width);
		
			_XmHTMLDebug(11, ("frames.c, adjustConstraints, x_adjust: %f at "
				"y: %i\n", x_adjust, max_y));

			/* make adjustments */
			if(x_adjust != 0.)
			{
				for(j = 0; j < html->html.nframes; j++)
				{
					frame = html->html.frames[j];
					if(frame->y == max_y)
					{
						frame->width = (int)(x_adjust * frame->width);
						frame->x = (int)(x_adjust * frame->x);
						done[j] |= ROW_DONE;
					}
				}
			}
		}
	}
	/* column stride */
	for(i = 0; i < html->html.nframes; i++)
	{
		max_height = max_rel_height = 0;
		height = rel_height = 0;
		max_x = curr_x = 0;
		y_adjust = 0.0;

		frame = html->html.frames[i];

		/* relative or fixed height */
		if(frame->height_type != FRAME_SIZE_OPTIONAL && !(done[i] & COL_DONE))
		{
			/* x position of this frame */
			curr_x = frame->x;

			if(frame->height_type == FRAME_SIZE_RELATIVE)
				rel_height = frame->height;
			if(frame->height_type == FRAME_SIZE_FIXED)
				height = frame->height;

			for(j = i+1; j < html->html.nframes; j++)
			{
				frame = html->html.frames[j];

				if(frame->x == curr_x)
				{
					if(frame->height_type == FRAME_SIZE_FIXED)
						height += frame->height;
					if(height > max_height)
					{
						max_height = height;
						max_x = curr_x;
					}
					if(frame->height_type == FRAME_SIZE_RELATIVE)
						rel_height += frame->height;
					if(rel_height > max_rel_height)
					{
						max_rel_height = rel_height;
						max_x = curr_x;
					}
				}
			}
			/* compute corrections if we exceed or stay below the margins */
			if(max_rel_height && max_rel_height != 100)
				y_adjust = (float)(100./max_rel_height);

			if(max_height && max_height != work_height)
				y_adjust = (float)(work_height/(float)max_height);

			_XmHTMLDebug(11, ("frames.c, adjustConstraints, y_adjust: %f at "
				"x: %i\n", y_adjust, max_x));

			/* make adjustments */
			if(y_adjust != 0.)
			{
				for(j = 0; j < html->html.nframes; j++)
				{
					frame = html->html.frames[j];
					if(frame->x == max_x)
					{
						frame->height = (int)(y_adjust * frame->height);
						frame->y = (int)(y_adjust * frame->y);
						done[j] |= COL_DONE;
					}
				}
			}
		}
	}
	/*
	* Second stage: each frame now fits within the screen dimension, and
	* we now convert the relative and optional dimensions to real dimensions.
	*/
	j = 0;
	for(i = 0; i < html->html.nframes; i++)
	{
		frame = html->html.frames[i];
		if(frame->width_type == FRAME_SIZE_RELATIVE)
		{
			frame->x *= work_width/100.;
			frame->width *= work_width/100.;
		}
		if(frame->height_type == FRAME_SIZE_RELATIVE)
		{
			frame->y *= work_height/100.;
			frame->height *= work_height/100.;
		}
		if(frame->height_type == FRAME_SIZE_OPTIONAL ||
			frame->width_type == FRAME_SIZE_OPTIONAL)
			j++;
	}
	/* no more frames left to adjust */
	if(j == 0)
	{
		free(done);
		return;
	}

	/*
	* Third and final stage: we now fill in the optional dimensions.
	* We store the id's of the optional dimensions in an array (= the
	* visited array).
	*/
	k = j;	/* store this value */
	visited = (int*)calloc(j, sizeof(int));
	(void)memset(done, 0, html->html.nframes*sizeof(Byte));
	curr_x = curr_y = 0;

	/* row stride */
	for(i = 0; i < html->html.nframes; i++)
	{
		frame = html->html.frames[i];
		if(frame->width_type == FRAME_SIZE_OPTIONAL && !(done[i] & ROW_DONE))
		{
			/* y position of this frame, correction is in x direction */
			curr_y = frame->y;
			curr_x = frame->x;

			rel_width = width = max_x = 0;
			for(j = i; j < html->html.nframes; j++)
			{
				frame = html->html.frames[j];
				if(frame->width_type == FRAME_SIZE_OPTIONAL)
				{
					if(frame->y == curr_y)
					{
						/* optional relative height, sigh */
						if(frame->width)
							width += frame->width;
						else
							width++;
						/* running counter */
						visited[rel_width] = j;
						rel_width++;
					}
				}
				else
				{
					/* already occupied? */
					if(frame->y == curr_y)
						max_x += frame->width;
				}
			}
			x_adjust = (float)((work_width - max_x)/width);
			/* adjust in one go */
			for(j = 0; j < rel_width; j++)
			{
				frame = html->html.frames[visited[j]];
				frame->x = curr_x;
				frame->width = x_adjust * (frame->width ? frame->width : 1); 
				curr_x += frame->width;
				done[visited[j]] |= ROW_DONE;
			}
		}
	}
	(void)memset(visited, 0, k*sizeof(int));
	(void)memset(done, '\0', html->html.nframes*sizeof(Byte));
	/* column stride */
	for(i = 0; i < html->html.nframes; i++)
	{
		frame = html->html.frames[i];

		if(frame->height_type == FRAME_SIZE_OPTIONAL && !(done[i] & COL_DONE))
		{
			/* x position of this frame, correction is in y direction */
			curr_x = frame->x;
			curr_y = frame->y;
			rel_height = height = max_y = 0;

			for(j = i; j < html->html.nframes; j++)
			{
				frame = html->html.frames[j];
				if(frame->height_type == FRAME_SIZE_OPTIONAL)
				{
					if(frame->x == curr_x)
					{
						/* optional relative width, sigh */
						if(frame->height)
							height += frame->height;
						else
							height++;
						/* running counter */
						visited[rel_height] = j;
						rel_height++;
					}
				}
				else
				{
					/* already occupied? */
					if(frame->x == curr_x)
						max_y += frame->height;
				}
			}
			y_adjust = (float)((work_height - max_y)/height);
			/* adjust in one go */
			for(j = 0; j < rel_height; j++)
			{
				frame = html->html.frames[visited[j]];
				frame->y = curr_y;
				frame->height = y_adjust * (frame->height ? frame->height : 1); 
				curr_y += frame->height;
				done[visited[j]] |= COL_DONE;
			}
		}
	}
	free(visited);
	free(done);
}

/*****
* Name: 		destroyFrameSets
* Return Type: 	void
* Description: 	destroys the memory used by the framesets
* In: 
*	set:		list of framesets to be destroyed
* Returns:
*	nothing
*****/
static void
destroyFrameSets(frameSet *set)
{
	frameSet *tmp;

	while(set)
	{
		tmp = set->next;
		if(set->sizes)
			free(set->sizes);
		if(set->size_types)
			free(set->size_types);
		if(set->childs)
			free(set->childs);
		free(set);
		set = tmp;
	}
	set = NULL;
}

/*****
* Name: 		mapFrames
* Return Type: 	void
* Description: 	map's all XmHTML frame childs to screen
* In: 
*	html:		XmHTMLWidget id
* Returns:
*	nothing
*****/
static void
mapFrames(XmHTMLWidget html)
{
	XmHTMLFrameWidget *frame;
	int i;

	/* map all XmHTML frame childs */
	for(i = 0; i < html->html.nframes; i++)
	{
		frame = html->html.frames[i];
#ifdef WITH_MOTIF
		/* map to screen */
		XtSetMappedWhenManaged(frame->frame, True);
#else
		gtk_xmhtml_manage (GTK_CONTAINER (html), frame->frame);
		gtk_widget_show (frame->frame);
#endif
		/* call notifier */
		frameDoneCallback(html, frame, frame->frame);
		
	}
	/* resync */
	if(html->html.gc)
		Toolkit_Flush (Toolkit_Display(html->html.work_area), False);
}

/*****
* Name: 		frameDoneCallback
* Return Type: 	void
* Description: 	frame child creation finished notifier
* In: 
*	html:		XmHTMLWidget id;
*	frame:		XmHTMLFrameWidget data that is created.
*	widget:		frame child widget id
* Returns:
*	nothing
*****/
static void
frameDoneCallback(XmHTMLWidget html, XmHTMLFrameWidget *frame, 
	TWidget widget)
{
	XmHTMLFrameCallbackStruct cbs;

	if (!CHECK_CALLBACK(html, frame_callback, FRAME))
		return;
			
	/* inform user that this frame is finished */

	cbs.reason = XmCR_HTML_FRAME;
	cbs.event = NULL;
	cbs.src = frame->src;
	cbs.name = frame->name;
	cbs.html = widget;
	cbs.doit = False;

	/* call the callback list */
	Toolkit_Call_Callback((TWidget)html, html->html.frame_callback, FRAME, &cbs);
}

/********
****** Public Functions
********/

/*****
* Name: 		_XmHTMLCheckForFrames
* Return Type: 	int
* Description: 	checks if the given list of objects contains HTML frames
* In: 
*	html:		XmHTMLWidget id;
*	objects:	parser output to check
* Returns:
*	no of frames found in the current document.
*****/
int
_XmHTMLCheckForFrames(XmHTMLWidget html, XmHTMLObject *objects)
{
	XmHTMLObject *tmp;
	int nframes = 0;

	if (!CHECK_CALLBACK(html, frame_callback, FRAME))
		return 0;

	/*
	* frames are not allowed to appear inside the BODY tag.
	* So we never have to walk the entire contents of the current document
	* but simply break out of the loop once we encounter the <BODY> tag.
	* This is a fairly huge performance increase.
	*/
	for(tmp = objects; tmp != NULL && tmp->id != HT_BODY; tmp = tmp->next)
		if(tmp->id == HT_FRAME)
			nframes++;

	return(nframes);
}

/*****
* Name: 		_XmHTMLFrameDestroyCallback
* Return Type: 	void
* Description: 	frame destruction notifier
* In: 
*	html:		XmHTMLWidget id;
*	frame:		frame data;
* Returns:
*	nothing
*****/
void
_XmHTMLFrameDestroyCallback(XmHTMLWidget html, XmHTMLFrameWidget *frame)
{
	XmHTMLFrameCallbackStruct cbs;

	/* inform user that this frame is about to be destroyed */
	if(!CHECK_CALLBACK (html, frame_callback, FRAME))
		return;

	cbs.reason = XmCR_HTML_FRAMEDESTROY;
	cbs.event = NULL;
	cbs.src = frame->src;
	cbs.name = frame->name;
	cbs.html = frame->frame;
	cbs.doit = True;

	/* call the callback list */
	Toolkit_Call_Callback((Widget)html, html->html.frame_callback, FRAME, &cbs);

	/* always destroy this */
	if(frame->src)
		free(frame->src);
	if(frame->name)
		free(frame->name);

	/* return if we may not destroy this frame */
	if(!cbs.doit)
	{
		/* destroy frame data, but keep the widget alive */
		free(frame);
		frame = NULL;
		return;
	}

	/* destroy everything */
	if(frame->frame)
		Toolkit_Widget_Destroy(frame->frame);
	free(frame);
	frame = NULL;
}

/*****
* Name: 		_XmHTMLFrameCreateCallback
* Return Type: 	void
* Description:  frame creation notifier
* In: 
*	html:		XmHTMLWidget id
*	frame:		frame data
* Returns:
*	Widget id of frame to use, NULL otherwise
*****/
TWidget
_XmHTMLFrameCreateCallback(XmHTMLWidget html, XmHTMLFrameWidget *frame)
{
	XmHTMLFrameCallbackStruct cbs;
#ifdef WITH_MOTIF
	Arg args[20];
	Dimension argc = 0;
#endif
	XmHTMLWidget html_widget;
	static TWidget widget;

	/* inform user that this frame is about to be created */
	if(!CHECK_CALLBACK (html, frame_callback, FRAME))
		return(NULL);

	cbs.reason = XmCR_HTML_FRAMECREATE;
	cbs.event = NULL;
	cbs.src = frame->src;
	cbs.name = frame->name;
	cbs.html = NULL;
	cbs.doit = True;

	/* call the callback list */
	Toolkit_Call_Callback((TWidget)html, html->html.frame_callback, FRAME, &cbs);

#ifdef WITH_MOTIF
	/* set constraints and other frame stuff */
	XtSetArg(args[argc], XmNx, frame->x); argc++;
	XtSetArg(args[argc], XmNy, frame->y); argc++;
	XtSetArg(args[argc], XmNwidth, frame->width - frame->border); argc++;
	XtSetArg(args[argc], XmNheight, frame->height - frame->border); argc++;
	XtSetArg(args[argc], XmNmarginWidth, frame->margin_width); argc++;
	XtSetArg(args[argc], XmNmarginHeight, frame->margin_height); argc++;
	XtSetArg(args[argc], XmNborderWidth, frame->border); argc++;
	XtSetArg(args[argc], XmNborderColor, html->manager.top_shadow_color);argc++;
	XtSetArg(args[argc], XmNmappedWhenManaged, False); argc++;
	
	/* scrolling gets handled in the widget code itself, so don't set it */

	/*
	* Create when we have to, the widget is NULL or the widget isn't a
	* XmHTML widget.
	*/
	if(cbs.doit == True || cbs.html == NULL)
		widget = XmCreateHTML(html->html.work_area, cbs.name, args, argc);
	else if(!XmIsHTML(cbs.html))
	{
		/* not a HTML widget, spit out a warning and create one ourselves */
		_XmHTMLWarning(__WFUNC__(cbs.html, "_XmHTMLFrameCreateCallback"),
			"Bad HTML frame widget: not a XmHTMLWidget.");
		widget = XmCreateHTML(html->html.work_area, cbs.name, args, argc);
	}
	else
	{
		widget = cbs.html;

		/* first unmanage if it's still up */
		if(XtIsManaged(widget))
			XtUnmanageChild(widget);

		/* check if we need to clear any existing source */
		if(((XmHTMLWidget)widget)->html.source != NULL)
		{
			XtSetArg(args[argc], XmNvalue, NULL);
			argc++;
		}

		/* reconfigure this widget so it'll fit our purposes */
		XtSetValues(widget, args, argc);

		/* unmanage scrollbars as well */
		((XmHTMLWidget)widget)->html.needs_vsb = False;
		((XmHTMLWidget)widget)->html.needs_hsb = False;
		XtUnmanageChild(((XmHTMLWidget)widget)->html.hsb);
		XtUnmanageChild(((XmHTMLWidget)widget)->html.vsb);
	}
#else
	if (cbs.doit == True || cbs.html == NULL){
		widget = gtk_xmhtml_new ();
	} else if (!XmIsHTML (cbs.html)){
		/* not a HTML widget, spit out a warning and create one ourselves */
		_XmHTMLWarning(__WFUNC__(cbs.html, "_XmHTMLFrameCreateCallback"),
			"Bad HTML frame widget: not a XmHTMLWidget.");
		widget = gtk_xmhtml_new ();
	} else {
		widget = cbs.html;

		gtk_widget_hide (GTK_WIDGET (html));
		if (html->html.source)
			gtk_xmhtml_source (html, NULL);
		
		/* unmanage scrollbars as well */
		((XmHTMLWidget)widget)->html.needs_vsb = False;
		((XmHTMLWidget)widget)->html.needs_hsb = False;
		gtk_widget_hide (html->html.hsb);
		gtk_widget_hide (html->html.vsb);
	}
	gtk_widget_show (widget);
	gtk_xmhtml_manage (GTK_CONTAINER (html), widget);
	gtk_xmhtml_set_geometry (widget,
				 frame->x,
				 frame->y,
				 frame->width - frame->border,
				 frame->height - frame->border);
/* GtkFIXME: I am not handling borders yet */
				 
#endif
	html_widget = (XmHTMLWidget)widget;
	html_widget->html.is_frame = True;
	html_widget->html.frame_border = frame->border;
	html_widget->html.scroll_type = frame->scroll_type;

#ifdef WITH_MOTIF
	/* manage it */
	XtManageChild(widget);
#endif
	return(widget);
}

/*****
* Name: 		_XmHTMLDestroyFrames
* Return Type: 	void
* Description: 	frame destroyer
* In: 
*	html:		XmHTMLWidget id
*	nframes:	no of frames to destroy;
* Returns:
*	nothing, but the frames list of the widget is destroyed.
*****/
void
_XmHTMLDestroyFrames(XmHTMLWidget html, int nframes)
{
	int i = 0;

#ifdef WITH_MOTIF
	/* unmap all XmHTML frame childs */
	for(i = 0; i < html->html.nframes; i++)
		XtSetMappedWhenManaged(html->html.frames[i]->frame, False);
#endif
	
	/* free them */
	for(i = 0; i < nframes; i++)
		_XmHTMLFrameDestroyCallback(html, html->html.frames[i]);

	free(html->html.frames);
	html->html.frames = NULL;
	html->html.nframes = 0;
}

/*****
* Name: 		_XmHTMLReconfigureFrames
* Return Type: 	void
* Description: 	resize method for XmHTML frame childs
* In: 
*	html:		XmHTMLWidget id
* Returns:
*	nothing
*****/
void
_XmHTMLReconfigureFrames(XmHTMLWidget html)
{
	XmHTMLFrameWidget *frame;
	int i;

	_XmHTMLDebug(11, ("frames.c: _XmHTMLReconfigureFrames Start\n"));
	/* compute new screen positions */
	adjustConstraints(html);

	/* reconfigure all widgets */
	for(i = 0; i < html->html.nframes; i++)
	{
		frame = html->html.frames[i];

		_XmHTMLDebug(11, ("frames.c: _XmHTMLReconfigureFrames doing frame "
			"%s.\n", frame->name));

#if WITH_MOTIF
		XtConfigureWidget(frame->frame, frame->x, frame->y,
			frame->width - frame->border,
			frame->height - frame->border, frame->border);
#else
	gtk_xmhtml_set_geometry (frame->frame,
				 frame->x, frame->y,
				 frame->width,
				 frame->height);
#endif
	}
	_XmHTMLDebug(11, ("frames.c: _XmHTMLReconfigureFrames End.\n"));
}

/*****
* Name:			_XmHTMLCreateFrames
* Return Type:	Boolean
* Description:	main frame creator
* In:
*	html_old:	previous XmHTMLWidget id;
*	html:		XmHTMLWidget id;
* Returns:
*	True when all frames could be created, False otherwise.
*****/
Boolean
_XmHTMLCreateFrames(XmHTMLWidget old, XmHTMLWidget html)
{
	int i;
	XmHTMLObject *tmp;
	static TWidget frame;

	frame_stack = &frame_base;
	frame_stack->next = NULL;
	frame_stack->frame_set = NULL;

	/* first destroy all previous frames of this widget */
	if(old && old->html.nframes)
		_XmHTMLDestroyFrames(old, old->html.nframes);

	if(frame_sets)
		destroyFrameSets(frame_sets);
	frame_sets = NULL;

	/*
	* Don't do a thing if we are destroying the previous list, we don't have
	* a frame callback or the new widget doesn't have any frames at all
	*/
	if(html == NULL || !CHECK_CALLBACK (html, frame_callback, FRAME) || html->html.nframes == 0)
		return(False);

	frame = NULL;

	/* create the list of HTML frame childs */
	html->html.frames = (XmHTMLFrameWidget**)calloc(html->html.nframes,
		sizeof(XmHTMLFrameWidget*));

	/* create individual HTML frame child ptrs */
	for(i = 0; i < html->html.nframes; i++)
	{
		XmHTMLFrameWidget *frame_w;
		frame_w = (XmHTMLFrameWidget*)malloc(sizeof(XmHTMLFrameWidget));
		(void)memset(frame_w, 0, sizeof(XmHTMLFrameWidget));
		html->html.frames[i] = frame_w;
	}

	/* move to the first frameset declaration */
	for(tmp = html->html.elements; tmp != NULL && tmp->id != HT_FRAMESET; 
		tmp = tmp->next);

	current_frame = 0;

	/* create all frames (and possibly nested framesets also) */
	makeFrameSets(html, tmp);

	/* adjust framecount, makeFrameSets might have found some invalid sets */
	html->html.nframes = current_frame;

#ifdef DEBUG
	_XmHTMLDebug(11, ("frames.c: _XmHTMLCreateFrames, raw frame listing\n"));
	for(i = 0; i < html->html.nframes; i++)
	{
		_XmHTMLDebug(11, ("frame %i\n"
			"\tname           : %s\n"
			"\tsrc            : %s\n"
			"\twidth by height: %ix%i\n"
			"\tx offset       : %i\n"
			"\ty offset       : %i\n",
			i, html->html.frames[i]->src, html->html.frames[i]->name,
			html->html.frames[i]->width_s, html->html.frames[i]->height_s,
			html->html.frames[i]->xs, html->html.frames[i]->ys));
	}
#endif

	adjustConstraints(html);

#ifdef DEBUG
	_XmHTMLDebug(11, ("frames.c: _XmHTMLCreateFrames, adjusted frame "
		"listing\n"));
	for(i = 0; i < html->html.nframes; i++)
	{
		_XmHTMLDebug(11, ("frame %i\n"
			"\tname           : %s\n"
			"\tsrc            : %s\n"
			"\twidth by height: %ix%i\n"
			"\tx offset       : %i\n"
			"\ty offset       : %i\n",
			i, html->html.frames[i]->src, html->html.frames[i]->name,
			html->html.frames[i]->width, html->html.frames[i]->height,
			html->html.frames[i]->x, html->html.frames[i]->y));
	}
#endif
	/* and now create all frames */
	for(i = 0; i < html->html.nframes; i++)
	{
		html->html.frames[i]->frame = _XmHTMLFrameCreateCallback(html,
			html->html.frames[i]);
	}
	/* erase a few glitches by calling adjustConstraints again */
	_XmHTMLReconfigureFrames(html);

	/* and now map them to screen */
	mapFrames(html);

	return(True);
}

/* doesn't work yet */
#if 0
void
_XmHTMLDrawFrameBorder(XmHTMLWidget html)
{
	int x = html->core.x;
	int y = html->core.y;
	int width = html->html.frame_border;
	int height = html->core.height;
	Display *dsp = XtDisplay((TWidget)html);
	GC gc;
	Window win = XtWindow((Widget)html);
	
	gc = html->manager.bottom_shadow_GC;
	XFillRectangle(dsp, win, gc, x, y, width, 1);
	XFillRectangle(dsp, win, gc, x, y, 1, height-1);

	gc = html->manager.top_shadow_GC;
	XFillRectangle(dsp, win, gc, x+1, y + height-1, width-1, 1);
	XFillRectangle(dsp, win, gc, x + width - 1, y + 1, 1, height-2);
}
#endif

/*****
* Name: 		XmHTMLFrameGetChild
* Return Type: 	TWidget
* Description: 	returns the TWidget id of a frame child given it's name.
* In: 
*	w:			XmHTMLWidget
*	name:		name of frame to locate.
* Returns:
*	If found, the TWidget id of the requested frame, NULL otherwise. 
*****/
TWidget
XmHTMLFrameGetChild(TWidget w, String name)
{
	XmHTMLWidget html;
	int i;

	/* sanity check */
	if(!w || !XmIsHTML(w) || name == NULL)
	{
		_XmHTMLWarning(__WFUNC__(w, "XmHTMLFrameGetChild"),
			"%s passed to XmHTMLFrameGetChild.",
			w ? (name == NULL ? "NULL frame name" : "Invalid parent") :
			"NULL parent");
		return(NULL);
	}

	html = XmHTML (w);

	for(i = 0; i < html->html.nframes; i++)
	{
		if(!(strcmp(html->html.frames[i]->name, name)))
			return(html->html.frames[i]->frame);
	}
	return(NULL);
}

