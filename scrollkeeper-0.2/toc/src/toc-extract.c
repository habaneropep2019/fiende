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

#include <config.h>
#include "toc-extract.h"
#include <string.h>


static void toc_extract_start_element (Context *context, const char *name, const xmlChar **atrs);
static void toc_extract_end_element (Context *context);
static void toc_extract_title_start_element (Context *context, const char *name, const xmlChar **atrs);
static void toc_extract_title_characters (Context *context, const char *chars, int len);
static void print_indent(int depth);

ElementInfo toc_extract_elements[] = {
	{ ARTICLE, "article", (startElementSAXFunc)toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ BOOK, "book", (startElementSAXFunc)toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ SECTION, "section", (startElementSAXFunc) toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ SECT1, "sect1", (startElementSAXFunc) toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ SECT2, "sect2", (startElementSAXFunc) toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ SECT3, "sect3", (startElementSAXFunc) toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ SECT4, "sect4", (startElementSAXFunc) toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ SECT5, "sect5", (startElementSAXFunc) toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ TITLE, "title", (startElementSAXFunc) toc_extract_title_start_element, NULL, (charactersSAXFunc) toc_extract_title_characters },
	{ CHAPTER, "chapter", (startElementSAXFunc)toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ APPENDIX, "appendix", (startElementSAXFunc)toc_extract_start_element, (endElementSAXFunc) toc_extract_end_element, NULL},
	{ UNDEFINED, NULL, NULL, NULL, NULL}
};

static void
toc_extract_start_element (Context *context, const char *name, const xmlChar **atrs)
{
	char **atrs_ptr;

	if (context->depth == 0)
	{
		context->depth++;
		printf ("<toc>\n");
		return;
	}
	
	context->depth++;
			
	((StackElement *)context->stack->data)->ignore = 1;
						
	atrs_ptr = (char **) atrs;
	while (atrs_ptr && *atrs_ptr) {
		if (!strcasecmp (*atrs_ptr, "id")) {
			atrs_ptr++;
			((StackElement *)context->stack->data)->atrs = calloc (3, sizeof(char *));
			((StackElement *)context->stack->data)->atrs[0] = strdup ("id");
			((StackElement *)context->stack->data)->atrs[1] = strdup (*atrs_ptr);
			((StackElement *)context->stack->data)->ignore = 0;
			break;
		}
		atrs_ptr += 2;
	}

}

static void
toc_extract_end_element (Context *context)
{
	if (context->depth > 1)
	{
		List *element_list = NULL;
		StackElement *stack_el;
			
		element_list = list_prepend (element_list, (void *)APPENDIX);
		element_list = list_prepend (element_list, (void *)ARTICLE);
		element_list = list_prepend (element_list, (void *)CHAPTER);
		element_list = list_prepend (element_list, (void *) (SECTION));
		element_list = list_prepend (element_list, (void *)SECT1);
		element_list = list_prepend (element_list, (void *)SECT2);
		element_list = list_prepend (element_list, (void *) (SECT3));
		element_list = list_prepend (element_list, (void *) (SECT4));
		element_list = list_prepend (element_list, (void *) (SECT5));

		stack_el = find_first_element (context, element_list);

		if (!stack_el->ignore)
		{
			print_indent(context->depth);
			printf("</tocsect%d>\n", context->depth-1);
		}
	}

	context->depth--;
	
	if (context->depth == 0)
		printf("</toc>\n");
}

static void
toc_extract_title_start_element (Context *context,
			 const char *name,
			 const xmlChar **atrs)
{
	List *element_list = NULL;
	StackElement *stack_el;
	char **atrs_ptr;
	
	if (context->depth == 1)
		return;
		
	element_list = list_prepend (element_list, (void *) (APPENDIX));
	element_list = list_prepend (element_list, (void *) (ARTICLE));
	element_list = list_prepend (element_list, (void *) (CHAPTER));
	element_list = list_prepend (element_list, (void *) (SECTION));
	element_list = list_prepend (element_list, (void *) (SECT1));
	element_list = list_prepend (element_list, (void *) (SECT2));
	element_list = list_prepend (element_list, (void *) (SECT3));
	element_list = list_prepend (element_list, (void *) (SECT4));
	element_list = list_prepend (element_list, (void *) (SECT5));
	stack_el = find_first_element (context, element_list);

	list_free (element_list);
	if (stack_el == NULL)
		return;
		
	if (stack_el->title_output)
		return;

	switch (stack_el->info->index) {
	case APPENDIX:
	case ARTICLE:
	case CHAPTER:
	case SECT1:
	case SECT2:
	case SECT3:
	case SECT4:
	case SECT5:
	case SECTION:
		atrs_ptr = (stack_el->atrs);
		while (atrs_ptr && *atrs_ptr) {
			if (!strcasecmp (*atrs_ptr, "id")) {
				atrs_ptr++;
				print_indent(context->depth);	
				printf ("<tocsect%d ", context->depth-1);
				printf ("linkid=");
				printf ("\"%s\">", *atrs_ptr);
				break;
			}
			atrs_ptr += 2;
		}
		break;
	default:
		break;
	};
}

static void
toc_extract_title_characters (Context *context, const char *chars, int len)
{
	List *element_list = NULL;
	char *temp;
	StackElement *stack_el;
	
	if (context->depth == 1)
		return;
		
	element_list = list_prepend (element_list, (void *) (APPENDIX));
	element_list = list_prepend (element_list, (void *) (ARTICLE));
	element_list = list_prepend (element_list, (void *) (CHAPTER));
	element_list = list_prepend (element_list, (void *) (SECTION));
	element_list = list_prepend (element_list, (void *) (SECT1));
	element_list = list_prepend (element_list, (void *) (SECT2));
	element_list = list_prepend (element_list, (void *) (SECT3));
	element_list = list_prepend (element_list, (void *) (SECT4));
	element_list = list_prepend (element_list, (void *) (SECT5));

	stack_el = find_first_element (context, element_list);
	
	if (stack_el->title_output)
	{
		list_free (element_list);
		return;
	}

	temp = (char *)calloc(len+1, sizeof(char));
	strncpy(temp, chars, len);

	switch (stack_el->info->index) {
	case APPENDIX:
	case ARTICLE:
	case CHAPTER:
	case SECT1:
	case SECT2:
	case SECT3:
	case SECT4:
	case SECT5:
	case SECTION:
		if (!stack_el->ignore) {				
			printf ("%s\n", temp);
			stack_el->title_output = 1;
			break;
		}
		break;
	default:
		break;
	};
	
	free(temp);

	list_free (element_list);
}

static void print_indent(int depth)
{
	if (depth == 2) {
		printf ("  ");
	} else if (depth == 3) {
		printf ("    ");
	} else if (depth == 4) {
		printf ("      ");
	} else if (depth == 5) {
		printf ("        ");
	} else {
		printf ("          ");     
	}
};
