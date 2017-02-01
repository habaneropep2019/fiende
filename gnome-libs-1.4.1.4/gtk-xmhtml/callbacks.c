/*****
* callbacks.c : XmHTML callback routines
*
* This file Version	$Revision: 1.6 $
*
* Creation date:		Mon Dec  2 19:58:52 GMT+0100 1996
* Last modification: 	$Date: 1999/07/29 01:26:28 $
* By:					$Author: sopwith $
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
* $Log: callbacks.c,v $
* Revision 1.6  1999/07/29 01:26:28  sopwith
*
*
* Fix all warnings.
*
* Revision 1.5  1998/02/12 03:08:30  unammx
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
* Revision 1.2  1997/12/24 17:53:54  unammx
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
* Revision 1.12  1997/10/26 23:50:15  newt
* Bugfix 10/22/97-01
*
* Revision 1.11  1997/10/23 00:24:50  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.10  1997/08/30 00:45:33  newt
* my_strdup -> strdup and _XmHTMLWarning proto changes.
*
* Revision 1.9  1997/08/01 12:57:23  newt
* my_strdup -> strdup
*
* Revision 1.8  1997/05/28 01:44:43  newt
* Extended XmHTMLGetHeadAttributes to copy the value of the <SCRIPT> and
* <STYLE> head attributes.
*
* Revision 1.7  1997/04/29 14:24:02  newt
* bugfix in linkCallback. Added XmHTMLGetHeadAttributes.
*
* Revision 1.6  1997/03/20 08:07:50  newt
* changes in parseLinks
*
* Revision 1.5  1997/03/02 23:14:55  newt
* Changes due to changes in XmHTMLAnchorCallbackStruct
*
* Revision 1.4  1997/02/11 02:06:18  newt
* Bugfix in ActivateCallback
*
* Revision 1.3  1997/01/09 06:54:56  newt
* expanded copyright marker
*
* Revision 1.2  1997/01/09 06:43:09  newt
* bugfix in ParseLinks
*
* Revision 1.1  1996/12/19 02:17:06  newt
* Initial Revision
*
*****/ 
/*****
* Note:
* 	The XmNarmCallback resource is served in XmHTML.c, routine ExtendEnd.
*****/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Local includes */
#include "XmHTMLP.h"

/* this must be last */
#include "XmHTMLfuncs.h"

#include "toolkit.h"

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/

/*** Private Function Prototype Declarations ****/
static XmHTMLLinkDataPtr ParseLinks(XmHTMLObject *list, int *num_link); 
static XmHTMLMetaDataPtr ParseMeta(XmHTMLObject *list, int *num_meta);

#define HEAD_FREE_DOCTYPE { \
	if(head->doctype) free(head->doctype); head->doctype = NULL; \
}

#define HEAD_FREE_TITLE { \
	if(head->title) free(head->title); head->title = NULL; \
}

#define HEAD_FREE_BASE { \
	if(head->base) free(head->base); head->base = NULL; \
}

#define HEAD_FREE_SCRIPT { \
	if(head->script) free(head->script); head->script = NULL; \
	if(head->script_lang) free(head->script_lang); head->script_lang = NULL; \
}

#define HEAD_FREE_STYLE { \
	if(head->style_type) free(head->style_type); \
	if(head->style) free(head->style); \
	head->style_type = NULL; head->style = NULL; \
}

#define HEAD_FREE_META { \
	if(head->num_meta) { \
		int i; for(i = 0; i < head->num_meta; i++) { \
			if(head->meta[i].http_equiv) free(head->meta[i].http_equiv); \
			if(head->meta[i].name) free(head->meta[i].name); \
			if(head->meta[i].content) free(head->meta[i].content); \
		} free(head->meta); \
	} head->meta = NULL; head->num_meta = 0; \
}

#define HEAD_FREE_LINK { \
	if(head->num_link) { \
		int i; for(i = 0; i < head->num_link; i++) { \
			if(head->link[i].url) free(head->link[i].url); \
			if(head->link[i].rel) free(head->link[i].rel); \
			if(head->link[i].rev) free(head->link[i].rev); \
			if(head->link[i].title) free(head->link[i].title); \
		} free(head->link); \
	} head->link = NULL; head->num_link = 0; \
}

/*** Private Variable Declarations ***/

static XmHTMLLinkDataPtr
ParseLinks(XmHTMLObject *list, int *num_link)
{
	static XmHTMLLinkDataPtr link = NULL;
	XmHTMLObject *temp = list;
	int i = 0;
	char *tmp;

	/* We have got some links. Allocate memory */
	link = (XmHTMLLinkDataPtr)calloc(*num_link, sizeof(XmHTMLLinkDataRec));

	for(; temp != NULL && temp->id != HT_BODY && i < *num_link;
		temp = temp->next)
	{
		if(temp->id == HT_LINK)
		{
			/* get value of REL tag */
			if((tmp = _XmHTMLTagGetValue(temp->attributes, "rel")) != NULL)
			{
				/* make lowercase */
				my_locase(tmp);
				link[i].rel = tmp;
			}
			else /* get value of REV tag */
			{
				if((tmp = _XmHTMLTagGetValue(temp->attributes, "rev")) != NULL)
				{
					/* make lowercase */
					my_locase(tmp);
					link[i].rev = tmp;
				}
				else	/* invalid link member */
					continue;
			}

			/* get value of URL tag */
			if((tmp = _XmHTMLTagGetValue(temp->attributes, "href")) != NULL)
				link[i].url = tmp;
			else	/* href is mandatory */
			{
				if(link[i].rel)
					free(link[i].rel);
				if(link[i].rev)
					free(link[i].rel);
				continue;
			}

			/* get value of TITLE tag */
			if((tmp = _XmHTMLTagGetValue(temp->attributes, "title")) != NULL)
				link[i].title = tmp;
			i++;
		}
	}
	/* adjust link count for actually allocated elements */
	*num_link = i;

	return(link);
}

static XmHTMLMetaDataPtr
ParseMeta(XmHTMLObject *list, int *num_meta)
{
	static XmHTMLMetaDataPtr meta = NULL;
	XmHTMLObject *temp = list;
	int i = 0;
	char *tmp;

	/* We have got some links. Allocate memory */
	meta = (XmHTMLMetaDataPtr)calloc(*num_meta, sizeof(XmHTMLMetaDataRec));

	for(; temp != NULL && temp->id != HT_BODY && i < *num_meta;
		temp = temp->next)
	{
		if(temp->id == HT_META)
		{
			/* get value of http-equiv tag */
			if((tmp = _XmHTMLTagGetValue(temp->attributes,
				"http-equiv")) != NULL)
			{
				/* make lowercase */
				my_locase(tmp);
				meta[i].http_equiv = tmp;
			}
			else /* get value of name tag */
			{
				if((tmp = _XmHTMLTagGetValue(temp->attributes, "name")) != NULL)
				{
					/* make lowercase */
					my_locase(tmp);
					meta[i].name = tmp;
				}
				else	/* invalid meta element */
					continue;
			}

			/* get value of content tag */
			if((tmp = _XmHTMLTagGetValue(temp->attributes, "content")) != NULL)
				meta[i].content = tmp;
			else	/* invalid meta element */
			{
				if(meta[i].http_equiv)
					free(meta[i].http_equiv);
				if(meta[i].name)
					free(meta[i].name);
				continue;
			}
			i++;
		}
	}
	/* adjust meta count for actually allocated elements */
	*num_meta = i;
	return(meta);
}

/*****
* Name:			_XmHTMLLinkCallback
* Return Type:	void
* Description:	calls installed callback routines.
* In:
*	w:			widget to check
*	id:			id of callback to call ( = reason field)
* Returns:
*	nothing.
*****/
void
_XmHTMLLinkCallback(XmHTMLWidget html)
{
	XmHTMLObject *temp = html->html.elements, *start = NULL;
	XmHTMLLinkCallbackStruct cbs;
	int i, num_link = 0;	

	/* initialize callback fields */
	cbs.reason = XmCR_HTML_LINK;
	cbs.num_link = 0;
	cbs.event = NULL;

	/* count how many link elements we have */
	for(temp = html->html.elements; temp != NULL && temp->id != HT_BODY;
		temp = temp->next)
	{
		if(temp->id == HT_LINK)
		{
			num_link++;
			start = (num_link == 1 ? temp : start);
		}
	}
	/* no <LINK> found, call with a zero links field */
	if(num_link == 0 || start == NULL)
	{
		cbs.link = NULL;
		Toolkit_Call_Callback ((TWidget)html, html->html.link_callback, LINK, &cbs);
		return;
	}

	/* parse all link elements */
	cbs.link = ParseLinks(start, &num_link);
	cbs.num_link = num_link;

	Toolkit_Call_Callback ((TWidget)html, html->html.link_callback, LINK, &cbs);

	/* free everything */
	for(i = 0; i < num_link; i++)
	{
		if(cbs.link[i].rel)
			free(cbs.link[i].rel);
		if(cbs.link[i].rev)
			free(cbs.link[i].rev);
		if(cbs.link[i].url)
			free(cbs.link[i].url);
		if(cbs.link[i].title)
			free(cbs.link[i].title);
	}
	free(cbs.link);
}

/*****
* Name: 		_XmHTMLTrackCallback
* Return Type: 	void
* Description: 	routine associated with the XmNanchorTrackCallback resource.
*				fills in the appropriate fields in the XmHTMLCallbackStruct.
* In: 
*	html:		widget for which callback is to be activated
*	event:		event that triggered this callback to happen.
*	anchor:		anchor data (if any)
* Returns:
*	nothing.
* Note:
*	not a single member of the anchorCallbackStruct may be freed by the
*	user. Havoc will occur otherwise.
*****/
void
_XmHTMLTrackCallback(XmHTMLWidget html, TEvent *event, XmHTMLAnchor *anchor)
{
	XmHTMLAnchorCallbackStruct cbs;

	_XmHTMLDebug(3, ("callbacks.c: _XmHTMLTrackCallback Start\n"));

	(void)memset(&cbs, 0, sizeof(XmHTMLAnchorCallbackStruct));

	/* initialize callback fields */
	cbs.reason = XmCR_HTML_ANCHORTRACK;
	cbs.event = event;

	if(anchor != NULL)
	{
		cbs.url_type = anchor->url_type;	/* doesn't matter */
		cbs.line     = anchor->line;
		cbs.href     = anchor->href;
		cbs.target   = anchor->target;
		cbs.rel      = anchor->rel;
		cbs.rev      = anchor->rev;
		cbs.title    = anchor->title;
		cbs.doit     = False;				/* doesn't matter */
		cbs.visited  = anchor->visited;		/* doesn't matter */
	}

	Toolkit_Call_Callback((TWidget)html, html->html.anchor_track_callback, ANCHOR_TRACK, &cbs);

	_XmHTMLDebug(3, ("callbacks.c: _XmHTMLTrackCallback End\n"));
}

/*****
* Name: 		_XmHTMLActivateCallback
* Return Type: 	void
* Description: 	routine associated with the XmNactivateCallback resource.
*				fills in the appropriate fields in the XmHTMLCallbackStruct.
* In: 
*	html:		widget for which callback is to be activated
*	event:		event that triggered this callback to happen.
*	anchor:		data of activated anchor (if any).
* Returns:
*	nothing
* Note:
*	not a single member of the anchorCallbackStruct may be freed by the
*	user. Havoc will occur otherwise.
*****/
void 
_XmHTMLActivateCallback(XmHTMLWidget html, TEvent *event, XmHTMLAnchor *anchor)
{
	XmHTMLAnchorCallbackStruct cbs;
	XmHTMLObjectTableElement jump_anchor = NULL;
	XmHTMLAnchor *tmp;

	_XmHTMLDebug(3, ("callbacks.c: _XmHTMLActivateCallback Start\n"));

	/* sanity check */
	if(anchor == NULL)
		return;

	/* set to zero */
	(void)memset(&cbs, 0, sizeof(XmHTMLAnchorCallbackStruct));

	/* initialize callback fields */
	cbs.reason   = XmCR_ACTIVATE;
	cbs.event    = event;
	cbs.url_type = anchor->url_type;
	cbs.line     = anchor->line;
	cbs.href     = anchor->href;
	cbs.target   = anchor->target;
	cbs.rel      = anchor->rel;
	cbs.rev      = anchor->rev;
	cbs.title    = anchor->title;
	cbs.doit     = False;
	cbs.visited  = anchor->visited;

	Toolkit_Call_Callback((TWidget)html, html->html.activate_callback, ACTIVATE, &cbs);

	/* 
	* If we have a local anchor, see if we should mark it as visited
	* and if we should jump to it. The jumping itself is postponed to the
	* end of this routine.
	*/
	if(anchor->url_type == ANCHOR_JUMP)
	{
		/* set new foreground color */
		if(cbs.visited)
		{
			/* first check if this anchor wasn't already visited */
			if(!anchor->visited)
			{
				int i;
				Byte line_style;
				/* mark all other anchors pointing to the same name as well */
				for(i = 0 ; i < html->html.anchor_words ; i++)
				{
					/* check if this anchor matches */
					tmp = html->html.anchors[i].owner->anchor;
					if(!(strcasecmp(tmp->href, anchor->href)))
					{
						int j;
						/* a match, set the foreground of the master block */
						html->html.anchors[i].owner->fg = 
							html->html.anchor_visited_fg;

						/* change underline style as well! */
						line_style = html->html.anchor_visited_line;
						if(html->html.anchors[i].self->line_data & LINE_STRIKE)
							line_style |= LINE_STRIKE;

						/* update all words for this anchor */
						for(j = 0; j < html->html.anchors[i].owner->n_words;
							j++)
						{
							html->html.anchors[i].owner->words[j].line_data = line_style;
						}
					}
					/* skip remaining anchor words of the master block */
					while(i < html->html.anchor_words-1 && 
						html->html.anchors[i].owner == 
							html->html.anchors[i+1].owner)
						i++;
				}
			}
		}
		if(cbs.doit)
		{
			jump_anchor = _XmHTMLGetAnchorByName(html, anchor->href);

			if(jump_anchor == NULL)
			{
				_XmHTMLWarning(__WFUNC__(html, "_XmHTMLActivateCallback"),
					"can't locate named anchor %s\n", anchor->href);
			}
			else
			{
				_XmHTMLDebug(3, ("callbacks.c: _XmHTMLActivateCallback, "
					"internal anchor referenced: name = %s, id = %i, y = %i\n",
					anchor->href, jump_anchor->id, jump_anchor->y));
			}
		}
	}

	/* jump to the requested anchor only if we have a vertical scrollbar */
	/* fix 01/30/97-05, kdh */
	/* fix 10/22/97-01, kdh */
	if(jump_anchor && html->html.needs_vsb)
	{
		int value, max = 0, size = 0;

		_XmHTMLDebug(3, ("callbacks.c: _XmHTMLActivateCallback, "
			"calling _XmHTMLMoveToPos, y = %i\n", jump_anchor->y));

#ifdef WITH_MOTIF
		XtVaGetValues(html->html.vsb,
				XmNmaximum, &max,
				XmNsliderSize, &size,
				NULL);
#else
		fprintf (stderr, "XmHTML: Slider Pos\n");
#endif
		/* check slider value and adjust if necessary */
		value = jump_anchor->y - jump_anchor->height;

		if(value > max - size)
			value = max - size;

		_XmHTMLMoveToPos(html->html.vsb, html, value);
	}

	_XmHTMLDebug(3, ("callbacks.c: _XmHTMLActivateCallback End\n"));
}

/*****
* Name: 		_XmHTMLDocumentCallback
* Return Type: 	Boolean
* Description: 	XtNdocumentCallback driver
* In: 
*	PARSER:		current parser.
*	verified:	true when the flushed stack is balanced, False when not.
* Returns:
* 	False when another pass on the current text is requested/required,
*	True when not.
*****/
Boolean
_XmHTMLDocumentCallback(XmHTMLWidget html, Boolean html32, Boolean verified,
	Boolean balanced, Boolean terminated, int pass_level)
{
	XmHTMLDocumentCallbackStruct cbs;

	if(CHECK_CALLBACK (html, document_callback, DOCUMENT))
	{
		cbs.reason     = XmCR_HTML_DOCUMENT;
		cbs.event      = (TEvent*)NULL;
		cbs.html32     = html32;
		cbs.verified   = verified;
		cbs.balanced   = balanced;
		cbs.terminated = terminated;
		cbs.pass_level = pass_level;
		cbs.redo       = !balanced;

		Toolkit_Call_Callback((TWidget)html, html->html.document_callback, DOCUMENT, &cbs);

		return(cbs.redo);
	}
	return(True);
}

/*****
* Name: 		freeHeadMembers
* Return Type: 	void
* Description: 	frees the requested members of the given HeadAttributes
* In: 
*	*head:		structure of which to free members
*	mask_bits:	fields to free
* Returns:
*	nothing
*****/
static void
freeHeadAttributes(XmHTMLHeadAttributes *head, Byte mask_bits)
{
	if(mask_bits & HeadDocType)
	{
		HEAD_FREE_DOCTYPE;
	}
	if(mask_bits & HeadTitle)
	{
		HEAD_FREE_TITLE;
	}
	if(mask_bits & HeadBase)
	{
		HEAD_FREE_BASE;
	}
	if(mask_bits & HeadScript)
	{
		HEAD_FREE_SCRIPT;
	}
	if(mask_bits & HeadStyle)
	{
		HEAD_FREE_STYLE;
	}
	if(mask_bits & HeadMeta)
	{
		HEAD_FREE_META;
	}
	if(mask_bits & HeadLink)
	{
		HEAD_FREE_LINK;
	}
}

/*****
* Name: 		XmHTMLGetHeadAttributes
* Return Type: 	Boolean
* Description: 	fills the given HeadAttributes with the requested document
*				head elements.
* In: 
*	w:			XmHTMLWidget id
*	*head:		structure to fill
*	mask_bits:	indicates what members to fill
* Returns:
*	True when a <head></head> block is present, False if not.
*****/
Boolean
XmHTMLGetHeadAttributes(TWidget w, XmHTMLHeadAttributes *head, Byte mask_bits)
{
	XmHTMLWidget html;
	XmHTMLObject *tmp, *link_start = NULL, *meta_start = NULL;
	int num_link = 0, num_meta = 0;
	static String func = "XmHTMLGetHeadAttributes";

	/* sanity check */
	if(!head)
	{
		_XmHTMLWarning(__WFUNC__(w, func), "NULL structure passed to "
			"%s!", func);
		return(False);
	}

	/****
	* Don't bother to check a thing when we only have to clear all
	* attributes.
	****/
	if(mask_bits == HeadClear)
	{
		freeHeadAttributes(head, HeadAll);
		return(False);
	}

	/* another sanity check */
	if(!w || !XmIsHTML(w))
	{
		_XmHTMLBadParent(w, func);
		return(False);
	}
	html = (XmHTMLWidget)w;

	/* free requested members */
	freeHeadAttributes(head, mask_bits);

	/* empty document */
	if(html->html.elements == NULL)
		return(False);

	/* walk until we reach HT_HEAD or HT_BODY, whatever comes first */
	for(tmp = html->html.elements; tmp != NULL && tmp->id != HT_HEAD
		&& tmp->id != HT_BODY; tmp = tmp->next)
	{
		/* pick up doctype if we happen to see it */
		if(tmp->id == HT_DOCTYPE && tmp->attributes && 
			(mask_bits & HeadDocType || mask_bits == HeadAll))
			head->doctype = strdup(tmp->attributes);
	}

	/* no head found */
	if(tmp == NULL || tmp->id == HT_BODY)
		return(False);

	/* we have found the head. */
	tmp = tmp->next;	/* move past it */

	/* Go and fill all members */
	for(; tmp != NULL && tmp->id != HT_HEAD && tmp->id != HT_BODY;
		tmp = tmp->next)
	{
		switch(tmp->id)
		{
			case HT_LINK:
				num_link++;
				link_start = (num_link == 1 ? tmp : link_start);
				break;
			case HT_META:
				num_meta++;
				meta_start = (num_meta == 1 ? tmp : meta_start);
				break;
			case HT_ISINDEX:
				if(mask_bits & HeadIsIndex || mask_bits == HeadAll)
					head->is_index = True;
				break;
			case HT_TITLE:
				if((mask_bits & HeadTitle || mask_bits == HeadAll)
					&& !tmp->is_end)
				{
					String start, end;

					/* pick up the text, its all in a single element */
					tmp = tmp->next;

					/* sanity check */
					if(!tmp->element)
						break;

					/* skip leading... */
					for(start = tmp->element; *start != '\0' &&
						isspace(*start); start++);

					/* sanity */
					if(*start == '\0')
						break;

					/* ...and trailing whitespace */
					for(end = &start[strlen(start)-1]; *end != '\0' &&
						isspace(*end); end--);

					/* always backs up one to many */
					end++;

					/* sanity */
					if(end - start <= 0)
						break;

  					/* duplicate the title */
					head->title = my_strndup(start, end - start);

					/* expand escape sequences */
					_XmHTMLExpandEscapes(head->title,html->html.bad_html_warnings);
				}
				break;
			case HT_BASE:
				if(mask_bits & HeadBase || mask_bits == HeadAll)
					head->base = _XmHTMLTagGetValue(tmp->attributes, "href");
				break;
			case HT_SCRIPT:
				if((mask_bits & HeadScript || mask_bits == HeadAll)
					&& !tmp->is_end)
				{
					head->script_lang = _XmHTMLTagGetValue(tmp->attributes,
						"language");

					/* pick up the text, its all in a single element */
					tmp = tmp->next;

					/* sanity check */
					if(!tmp->element)
						break;

					/* copy contents */
					head->script = strdup(tmp->element);
				}
				break;
			case HT_STYLE:
				if((mask_bits & HeadStyle || mask_bits == HeadAll)
					&& !tmp->is_end)
				{
					head->style_type = _XmHTMLTagGetValue(tmp->attributes,
						"type");

					/* pick up the text, its all in a single element */
					tmp = tmp->next;

					/* sanity check */
					if(!tmp->element)
						break;

					/* copy contents */
					head->style = strdup(tmp->element);
				}
				break;
			default:
				break;
		}
	}
	/* fill in remaining link and meta members */
	if(mask_bits & HeadMeta)
	{
		if(num_meta)
			head->meta = ParseMeta(meta_start, &num_meta);
		head->num_meta = num_meta;
	}
	if(mask_bits & HeadLink)
	{
		if(num_link)
			head->link = ParseLinks(link_start, &num_link);
		head->num_link = num_link;
	}
	/* we found a head */
	return(True);
}
