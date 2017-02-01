/* Gnome Search Tool 
 * (C) 1998 the Free Software Foundation
 *
 * Author:   George Lebl
 */

#ifndef _GSEARCHTOOL_H_
#define _GSEARCHTOOL_H_

#include <gnome.h>

#define PIPE_READ_BUFFER 4096

typedef enum {
	FIND_OPTION_END, /* end the option templates list */
	FIND_OPTION_BOOL, /* if this is enabled, use the option */
	FIND_OPTION_TEXT,
	FIND_OPTION_NUMBER,
	FIND_OPTION_TIME,
	FIND_OPTION_GREP
} FindOptionType;

typedef struct _FindOptionTemplate FindOptionTemplate;
struct _FindOptionTemplate {
	FindOptionType type;
	gchar *option; /*the option string to pass to find or whatever*/
	gchar *desc; /*description*/
};

typedef struct _FindOption FindOption;
struct _FindOption {
	/*the index of the template this uses*/
	int templ;

	/* is this option enabled */
	gboolean enabled;

	union {
		/* this is a char string of the data */
		char *text;

		/* number data */
		int number;

		/* the time data */
		char *time;
	} data;
};
	


#endif
