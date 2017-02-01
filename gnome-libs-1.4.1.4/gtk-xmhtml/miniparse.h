/*****
* miniparse.h : required header file when compiling the parser standalone.
*
* This file Version	$Revision: 1.3 $
*
* Creation date:		Wed Mar 19 17:26:15 GMT+0100 1997
* Last modification: 	$Date: 1998/02/12 03:09:33 $
* By:					$Author: unammx $
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
* $Source: /cvs/gnome/gnome-libs/gtk-xmhtml/Attic/miniparse.h,v $
*****/
/*****
* ChangeLog 
* $Log: miniparse.h,v $
* Revision 1.3  1998/02/12 03:09:33  unammx
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
* Revision 1.2  1997/12/29 22:16:32  unammx
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
* Revision 1.6  1997/10/23 00:30:39  newt
* XmHTML Beta 1.1.0 release
*
* Revision 1.5  1997/08/30 02:04:25  newt
* _XmHTMLWarning proto changes.
*
* Revision 1.3  1997/05/28 01:56:39  newt
* Added my_strdup.
*
* Revision 1.2  1997/04/29 14:31:41  newt
* Removed unused structures.
*
* Revision 1.1  1997/03/20 08:01:55  newt
* Initial Revision
*
*****/ 
#ifndef _miniparse_h_
#define _miniparse_h_

#ifndef MINIPARSE
#define MINIPARSE 1
#endif

#include <sys/time.h>
#include <unistd.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <errno.h>	/* perror */


/* required typedefs */
typedef char* String;
typedef unsigned char Byte;
typedef unsigned char Boolean;
typedef unsigned short Dimension;
typedef unsigned char* Widget;
typedef Widget XmHTMLWidget;

/* Set to False if you don't want any warnings being issued */
extern Boolean parser_warnings;

/* Running count of encountered errors */
extern int parser_errors;

/* Count of HTML segments in the input text */
extern int parsed_object_count;

/* Count of text segments in the input text */
extern int parsed_text_object_count;

/* Set to False if you want the parser to be a bit more lenient */
extern Boolean parser_strict_checking;

/* Set to True if you want to see debug output */
extern Boolean parser_debug;

/* Set to True if you want to get timings from the parser tree verification */
extern Boolean parser_verification_timings;

/* we always set debug flag in here */
#ifndef DEBUG
#define DEBUG	1
#endif

#ifndef True
#  define True 1
#  define False 0
#endif

/*****
* HTML Elements internal id's
* This list is alphabetically sorted to speed up the searching process.
* DO NOT MODIFY
*****/
typedef enum{
HT_DOCTYPE, HT_A, HT_ADDRESS, HT_APPLET, HT_AREA, HT_B, HT_BASE, HT_BASEFONT, 
HT_BIG, HT_BLOCKQUOTE, HT_BODY, HT_BR, HT_CAPTION, HT_CENTER, HT_CITE, HT_CODE,
HT_DD, HT_DFN, HT_DIR, HT_DIV, HT_DL, HT_DT, HT_EM, HT_FONT, HT_FORM, HT_FRAME,
HT_FRAMESET, HT_H1, HT_H2, HT_H3, HT_H4, HT_H5, HT_H6, HT_HEAD, HT_HR, HT_HTML,
HT_I, HT_IMG, HT_INPUT, HT_ISINDEX, HT_KBD, HT_LI, HT_LINK, HT_MAP, HT_MENU,
HT_META, HT_NOFRAMES, HT_OL, HT_OPTION, HT_P, HT_PARAM, HT_PRE, HT_SAMP,
HT_SCRIPT, HT_SELECT, HT_SMALL, HT_STRIKE, HT_STRONG, HT_STYLE, HT_SUB,
HT_SUP, HT_TAB, HT_TABLE, HT_TD, HT_TEXTAREA, HT_TH, HT_TITLE,
HT_TR, HT_TT, HT_U, HT_UL, HT_VAR, HT_ZTEXT
}htmlEnum;

/***** 
* and corresponding name table, defined in parse.c
*****/
extern String html_tokens[];

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


/*****
* possible error codes for XmNparserCallback
*****/
typedef enum{
	HTML_UNKNOWN_ELEMENT = 1,	/* unknown HTML element */
	HTML_BAD,					/* very badly placed element */
	HTML_OPEN_BLOCK,			/* block still open while new block started */
	HTML_CLOSE_BLOCK,			/* block closed but was never opened */
	HTML_OPEN_ELEMENT,			/* unbalanced terminator */
	HTML_NESTED,				/* improperly nested element */
	HTML_VIOLATION,				/* bad content for current block/element */
	HTML_NOTIFY,				/* insertion of optional opening/closing */
	HTML_INTERNAL				/* internal parser error */
}parserError;

/*****
* And corresponding values for XmNenableBadHTMLWarnings.
* These are or'd together.
* XmNONE disables warnings and XmHTML_ALL enables all warnings.
* See parserError for their meaning.
*****/
enum{
	XmHTML_NONE = 0,						/* no warnings */
	XmHTML_UNKNOWN_ELEMENT = 1,	
	XmHTML_BAD = 2,
	XmHTML_OPEN_BLOCK = 4,
	XmHTML_CLOSE_BLOCK = 8,
	XmHTML_OPEN_ELEMENT = 16,
	XmHTML_NESTED = 32,
	XmHTML_VIOLATION = 64,
	XmHTML_ALL = 128				/* all warnings */
};

/*****
* Definition of parsed HTML elements
*****/
typedef struct _XmHTMLObject{
	htmlEnum 	id;			/* ID for this element */
	String		element;	/*
							* Raw text. For HTML elements, freeing this 
							* member also frees attributes.
							*/
	String 		attributes;	/* attributes for this element, if any */
	Boolean		ignore;		/* true if element must be ignored */
	Boolean		is_end;		/* true when this is a closing element */
	Boolean 	terminated;	/* true when element has a closing counterpart */
	Boolean		auto_insert;/* auto inserted element */
	int 		line;		/* line number for this element */
	struct _XmHTMLObject *next;
	struct _XmHTMLObject *prev;
}XmHTMLObject;

/*****
* Function to be called when the parser finished a single pass on the input
*
* ARGS:
*   First : The current list of parser objects, which may NOT be freed.
*   Second: True if input was HTML3.2 conforming, False if not;
*   Third : True if parser verification succeeded;
*   Fourth: True if parser tree was balanced;
*   Fifth : current parser pass (count starts at 0);
*   Sixth : length of input text;
* Return values:
*   True  : make another pass on the input using the current (possibly
*           repaired) output;
*   False : don't make another pass on the input;
*****/
typedef Boolean (*ParserDocumentCallback)(XmHTMLObject*, Boolean, Boolean,
	Boolean, int, int);
extern ParserDocumentCallback parser_document_callback;

/*****
* Function to be called upon completion of a single pass
* ARGS:
*   First : number of elements still on stack (only when document is
*           unbalanced);
*   Second: number of (missing) HTML tags inserted by the parser;
*   Third : number of HTML tags ignored by the parser;
* Return values:
*   None.
*****/
typedef void (*ParserAutoCorrectCallback)(int, int, int);
extern ParserAutoCorrectCallback parser_autocorrect_callback;

/*****
* Parser state stack object
*****/
typedef struct _stateStack{
	htmlEnum id;							/* current state id */
	struct _stateStack *next;				/* ptr to next record */
}stateStack;

/*****
* A Parser
*****/
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

	Boolean strict_checking;	/* HTML 3.2 looseness flag				*/
	Boolean have_body;			/* indicates presence of <body> tag		*/
	Boolean warn;				/* warn about bad html constructs		*/
	Boolean bad_html;			/* bad HTML document flag				*/
	Boolean html32;				/* HTML32 conforming document flag		*/
	Boolean	automatic;			/* when in automatic mode				*/

	Widget widget;				/* for the warning messages				*/
}Parser;

/*****
* Various helper functions used by the parser (and defined by the parser
* when it's compiled with -DMINIPARSE
*****/

extern void my_locase(char *string);
extern char* my_strcasestr(const char *s1, const char *s2);
extern char* my_strndup(const char *s1, size_t len);
extern char* my_strdup(const char *s1);

/*****
* The parser uses strcasecmp and strncasecmp. Since these do not exist
* on every system, the parser carriers fallback copies which will be used
* if you define -DNEED_STRCASECMP at compile time.
*****/
#ifdef NEED_STRCASECMP

extern int my_strcasecmp (const char *s1, const char *s2);
extern int my_strncasecmp (const char *s1, const char *s2, size_t n);

#define strcasecmp(S1,S2) my_strcasecmp(S1,S2)
#define strncasecmp(S1,S2,N) my_strncasecmp(S1,S2,N)

#endif

/*****
* Warning message display function
* When parser_warnings has been set to False, no warnings will be
* generated.
*****/
#define __WFUNC__(WIDGET_ID, FUNC)	(Widget)WIDGET_ID, __FILE__, \
	 __LINE__, FUNC

extern void __XmHTMLWarning(
#ifdef __STDC__ 
	Widget w, String module, int line, String routine,
	String fmt, ...
#endif
);

#define _XmHTMLWarning __XmHTMLWarning

/*****
* Public Parser Functions
*****/

/*****
* Write the list of objects to the given file. If notext is True, HTML
* text segments will not be included in the output file.
*****/
extern void ParserWriteOutputToFile(XmHTMLObject *objects, String prefix,
	Boolean notext);

/* Write the list of objects to the given file as a HTML file */
extern void ParserWriteHTMLOutputToFile(XmHTMLObject *objects, String prefix,
	Boolean notext);

/* compose a HTML output string from the list of objects */
extern String _XmHTMLTextGetString(XmHTMLObject *objects);

/* free the given list of objects */
extern void _XmHTMLFreeObjects(XmHTMLObject *objects);

/*****
* The parser. Takes a two widgets, a previous list of objects and the text
* to be parsed as it's input.
* Returns a list of parsed objects.
*****/
extern XmHTMLObject *_XmHTMLparseHTML(XmHTMLWidget html,
	XmHTMLObject *old_list, char *input, XmHTMLWidget dest);

/* Don't add anything after this endif! */
#endif /* _miniparse_h_ */
