#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdio.h>	/* must follow stdarg or varargs on LynxOS */

#include "XmHTMLP.h"
#include "XmHTMLfuncs.h"

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/

/*** Private Datatype Declarations ****/
#ifdef DEBUG
#endif

/*** Private Function Prototype Declarations ****/

/*** Private Variable Declarations ***/
#ifdef DEBUG
static char buf[1152];
static char loc[128];
#else
static char buf[1024];
#endif

/* default string that gets appended when a fatal error occurs */
static String authors = "    Please contact the XmHTML author at "
	"ripley@xs4all.nl.";

/* undefine these to prevent compiler errors */
#undef _XmHTMLWarning
#undef _XmHTMLError

/*****
* Name: 		__XmHTMLWarning
* Return Type: 	void
* Description: 	displays a warning message to stderr
* In: 
*	w:			widget;
*	module:		source file name;
*	line:		line number information;
*	routine:	routine name;
*	fmt:		message to display;
*	...:		extra args to fmt;
* Returns:
*	nothing
* Note:
*	the module, line and routine args are only used when DEBUG was defined
*	during compilation.
*****/
void
#ifdef __STDC__
#ifdef DEBUG
__XmHTMLWarning(TWidget w, String module, int line, String routine, 
	String fmt, ...) 
#else
__XmHTMLWarning(TWidget w, String fmt, ...) 
#endif
{
    va_list arg_list;
    va_start(arg_list, fmt);

#else /* ! __STDC__ */
#ifdef DEBUG
__XmHTMLWarning(TWidget w, String module, int line, String routine,
	String fmt, va_alist)
#else
__XmHTMLWarning(TWidget w, String fmt, va_alist)
#endif
    TWidget w;
#ifdef DEBUG
	String module;
	int line;
	String routine;
#endif
    String fmt;
    va_dcl
{
    va_start(arg_list);
#endif /* __STDC__ */

#ifdef DEBUG
	if(w && debug_disable_warnings)
		return;
	sprintf(loc, "\n    (%s, %s, line %i)\n", module, routine, line);
#endif

	if(w)
	{
		sprintf(buf, "\n    Name: %s\n    Class: %s\n    ",
			gtk_widget_get_name (w), "Dunno");
		vsprintf(buf+strlen(buf), fmt, arg_list);
		va_end(arg_list);
#ifdef DEBUG
		strcat(buf, loc);
#else
		strcat(buf, "\n");
#endif
	}
	else
	{
		vsprintf(buf, fmt, arg_list);
		va_end(arg_list);
#ifdef DEBUG
		strcat(buf, loc);
#else
		strcat(buf, "\n");
#endif
	}
	_XmHTMLDebugMirrorToFile((buf));
}

/*****
* Name: 		__XmHTMLError
* Return Type: 	void
* Description: 	displays an error message on stderr and exits.
* In: 
*	w:			widget;
*	module:		source file name;
*	line:		line information;
*	routine:	routine name;
*	fmt:		message to display;
*	...:		args to message;
* Returns:
*	nothing.
* Note:
*	the module, line and routine args are only used when DEBUG was defined
*	during compilation.
*****/
void
#ifdef __STDC__
#ifdef DEBUG
__XmHTMLError(TWidget w, String module, int line, String routine,
	String fmt, ...) 
#else
__XmHTMLError(TWidget w, String fmt, ...) 
#endif
{
    va_list arg_list;
    va_start(arg_list, fmt);

#else /* ! __STDC__ */
#ifdef DEBUG
__XmHTMLError(TWidget w, String module, int line, String routine, fmt, va_list)
#else
__XmHTMLError(TWidget w, String fmt, ...) 
#endif
    TWidget w;
#ifdef DEBUG
	String module;
	int line;
	String routine;
#endif
    String fmt;
    va_dcl
{
    va_start(arg_list);
#endif /* __STDC__ */

#ifdef DEBUG
	if(w && debug_disable_warnings)
		return;
	sprintf(loc, "\n    (%s, %s, line %i)\n", module, routine, line);
#endif

	if(w)
	{
		sprintf(buf, "\n    Name: %s\n    Class: %s\n    ",
			gtk_widget_get_name(w), "dunno");
		vsprintf(buf+strlen(buf), fmt, arg_list);
		va_end(arg_list);
#ifdef DEBUG
		strcat(buf, loc);
#else
		strcat(buf, "\n");
#endif
		strcat(buf, authors);
	}
	else
	{
		vsprintf(buf, fmt, arg_list);
		va_end(arg_list);
#ifdef DEBUG
		strcat(buf, loc);
#else
		strcat(buf, "\n");
#endif
		strcat(buf, authors);
	}
	_XmHTMLDebugMirrorToFile((buf));
	exit(EXIT_FAILURE);
}

/*****
* Name: 		_XmHTMLAllocError
* Return Type: 	void
* Description: 	displays an error message on stderr and exits.
* In: 
*	w:			widget
*	module:		source file name
*	routine:	routine name
*	func:		function used for allocation
*	size:		size for which allocation was attempted.
* Returns:
*	nothing.
*****/
void
_XmHTMLAllocError(TWidget w, char *module, char *routine, char *func, int size)
{
	if(w)
	{
		sprintf(buf, "\n    Name: %s\n    Class: %s\n    "
			"cannot continue: %s failed for %i bytes.\n    (%s, %s)\n", 
			gtk_widget_get_name(w), "Dunno", func, size, module,
			routine); 
	}
	else
	{
		sprintf(buf, "cannot continue: %s failed for %i bytes.\n"
			"    (%s, %s)\n", func, size, module, routine);
	}
	_XmHTMLDebugMirrorToFile((buf));
	exit(EXIT_FAILURE);
}

/*****
* Name:			__XmHTMLBadParent
* Return Type: 	void
* Description: 	default warning message for a call to a public routine and
*				the widget argument is either NULL or not of class XmHTML.
* In: 
*	w:			offending Widget id;
*	src_file:	source file where function is found;
*	func:		function in which error occured.
* Returns:
*	nothing.
*****/
void
#ifdef DEBUG
__XmHTMLBadParent(TWidget w, String src_file, int line, String func)
#else
__XmHTMLBadParent(TWidget w, String func)
#endif
{
#ifdef DEBUG
	__XmHTMLWarning(w, src_file, line, func, "%s parent passed to %s.",
		(w ? "Invalid" : "NULL"), func);
#else
	__XmHTMLWarning(w, "%s parent passed to %s.", (w ? "Invalid" : "NULL"),
		func);
#endif
}
