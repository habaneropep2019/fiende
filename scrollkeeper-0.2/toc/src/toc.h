/* copyright (C) 2000 Sun Microsystems */
/* copyright (C) 2000 Jonathan Blandford */
/* copyright  (C) 2000 Ali Abdin */
/* copyright  (C) 2000 John Fleck */

/*    
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TOC_H__
#define __TOC_H__

#include <parser.h>
#include <parserInternals.h>
#include <SAX.h>
#include <xmlmemory.h>
#include <string.h>

typedef enum ElementIndex {
	ARTICLE = 0,
	BOOK,
	SECTION,
	SECT1,
	SECT2,
	SECT3,
	SECT4,
	SECT5,
	TITLE,
	CHAPTER,
	APPENDIX,
	UNDEFINED
} ElementIndex;

typedef struct _ElementInfo ElementInfo;
struct _ElementInfo {
	ElementIndex index;
	char *name;
	startElementSAXFunc start_element_func;
	endElementSAXFunc end_element_func;
	charactersSAXFunc characters_func;
};

typedef struct _StackElement StackElement;
struct _StackElement {
	ElementInfo *info;
	char **atrs;
	int title_output;
	int ignore;
};

typedef struct _List List;
struct _List {
    void *data;
    List *next;
};

typedef struct _Context Context;
struct _Context {
	ElementInfo *elements;
	char *base_file;
	List *stack;
	int depth;
	xmlParserCtxtPtr ParserCtxt;
};

extern StackElement *find_first_element (Context *context, List *args);
extern List *list_prepend(List *list, void *data);
extern List *list_unlink_first(List *list);
extern void list_free(List *list);
extern List *list_remove_first(List *list);

#endif
