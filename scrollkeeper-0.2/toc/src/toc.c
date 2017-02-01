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
#include <locale.h>

#include "toc.h"
#include "toc-extract.h"
#include <stdarg.h>
#include <libintl.h>
#include <scrollkeeper.h>

#if 0
#define ERROR_OUTPUT
#endif

#define SCROLLKEEPERLOCALEDIR "/usr/share/locale"

StackElement *
find_first_element (Context *context, List *args)
{
	List *ptr;
	List *element_ptr;

	for (ptr = context->stack; ptr != NULL; ptr = ptr->next) {
		for (element_ptr = args; element_ptr != NULL; element_ptr = element_ptr->next) {
			if (((StackElement*) ptr->data)->info &&
			    ((StackElement*) ptr->data)->info->index == (ElementIndex) (element_ptr->data))
				return (StackElement *) ptr->data;
		}
	}
	return NULL;
}

List *list_prepend(List *list, void *data)
{
    List *node;
    
    node = calloc(1, sizeof(List));
    node->data = data;
    node->next = list;
    return node;
}

List *list_remove_first(List *list)
{
    List *node;

    if (list == NULL)
        return NULL;
	
    node = list->next;
    free(list);
    return node;
}

void list_free(List *list)
{
    if (list == NULL)
        return;
	
    list_free(list->next);
    free(list);
}

/* helper functions */

static ElementInfo *
find_element_info (ElementInfo *elements,
		   const char *name)
{
	while (elements->name != NULL) {
		if (!strcasecmp (elements->name, name))
			return elements;
		elements++;
	}

	return NULL;
}

/* our callbacks for the xmlSAXHandler */

static xmlEntityPtr
get_entity (Context *context, const char *name)
{
	xmlEntityPtr ret;
#ifdef ERROR_OUTPUT
	printf ("in getEntity:%s\n", name);
#endif
	ret = getEntity (context->ParserCtxt, name);

/*	return xmlGetPredefinedEntity (name); */
	return (ret);
}

static void
start_document (Context *context)
{
	startDocument (context->ParserCtxt);
}

static void
end_document (Context *context)
{
	endDocument (context->ParserCtxt);
}

static void
start_element(Context *context,
	      const char *name,
	      const xmlChar **attrs)
{
	ElementInfo *element;
	StackElement *stack_el = calloc(1, sizeof(StackElement));
	
	startElement (context->ParserCtxt, name, attrs);

	element = find_element_info (context->elements, name);

	stack_el->info = element;
	context->stack = list_prepend (context->stack, stack_el);

	if (element && element->start_element_func)
		(* element->start_element_func) (context, name, attrs);
	if (!strcasecmp (name, "xref")) {
		context->stack = list_remove_first (context->stack);
	} else if (!strcasecmp (name, "void")) {
		context->stack = list_remove_first (context->stack);
	       } else if (!strcasecmp (name, "anchor")) {
                	context->stack = list_remove_first (context->stack);
	              }
}

static void
end_element (Context *context,
	     const char *name)
{
	ElementInfo *element;
	StackElement *stack_el;
	char **atrs_ptr;

	endElement (context->ParserCtxt, name);
	
	element = find_element_info (context->elements, name);
	stack_el = (StackElement *) context->stack->data;
	if (stack_el->info != element) {
		/* Prolly a tag we ignored */
		return;
	}
	if (element && element->end_element_func)
		(* element->end_element_func) (context, name);

	context->stack = list_remove_first (context->stack);

	atrs_ptr = stack_el->atrs;
	while (atrs_ptr && *atrs_ptr) {
		free (*atrs_ptr);
		atrs_ptr++;
	};
	free (stack_el->atrs);
	free (stack_el);
}

static void
toc_characters (Context *context,
		     const char *chars,
	    	     int len)
{
	ElementInfo *element;
	
	characters (context->ParserCtxt, chars, len);

	if (context->stack == NULL)
		return;
	element = ((StackElement *)context->stack->data)->info;
	
	if (element && element->characters_func)
		(* element->characters_func) (context, chars, len);
}

static void
toc_comment (Context *context, const char *msg)
{
#ifdef ERROR_OUTPUT
	fprintf(stderr, "XML %s", msg);
#endif
}

static void
toc_warning (Context *context, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
#ifdef ERROR_OUTPUT
	fprintf(stderr, "XML%s", msg);
#endif
	va_end(args);
}

static void
toc_error (Context *context, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
#ifdef ERROR_OUTPUT
	fprintf(stderr, "XML%s", msg);
#endif
	va_end(args);
}

static void
fatal_error (Context *context, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
#ifdef ERROR_OUTPUT
	fprintf(stderr, "XML%s", msg);
#endif
	va_end(args);
}

static int
toc_isStandalone (Context *context)
{
	int ret;
	
	ret = isStandalone (context->ParserCtxt);
	return (ret);
}

static int
toc_hasInternalSubset (Context *context)
{
	int ret;
	
	ret = hasInternalSubset (context->ParserCtxt);
	return (ret);
}

static int
toc_hasExternalSubset (Context *context)
{
	int ret;
	
	ret = hasExternalSubset (context->ParserCtxt);
	return (ret);
}
static void
toc_internalSubset (Context *context, const xmlChar *name,
			 const xmlChar *ExternalID, const xmlChar *SystemID)
{
	/* This function is copied from SAX.c in libxml so we can 'silence'
	 * the warning messages */
	xmlParserCtxtPtr ctxt;
       
	ctxt = context->ParserCtxt;
	
	xmlCreateIntSubset (ctxt->myDoc, name, ExternalID, SystemID);
	if (((ExternalID != NULL) || (SystemID != NULL)) &&
	    (ctxt->validate && ctxt->wellFormed && ctxt->myDoc)) {
		xmlDtdPtr ret = NULL;
		xmlParserCtxtPtr dtdCtxt;
		xmlParserInputPtr input = NULL;
		xmlCharEncoding enc;

		dtdCtxt = xmlNewParserCtxt();
		if (dtdCtxt == NULL) {
			return;
		}

		/* Ask entity resolve to load it */
		if ((ctxt->directory != NULL) && (dtdCtxt->directory == NULL)) {
			dtdCtxt->directory = (char *) xmlStrdup (BAD_CAST ctxt->directory);
		}
		if ((dtdCtxt->sax != NULL) && (dtdCtxt->sax->resolveEntity != NULL)) {
			dtdCtxt->sax->warning = (warningSAXFunc) toc_warning;
			input = dtdCtxt->sax->resolveEntity (dtdCtxt->userData, ExternalID, SystemID);
		}
		if (input == NULL) {
			xmlFreeParserCtxt (dtdCtxt);
			return;
		}

		/* Plug some encoding conversion routines */
		xmlPushInput (dtdCtxt, input);
		enc = xmlDetectCharEncoding (dtdCtxt->input->cur);
		xmlSwitchEncoding (dtdCtxt, enc);

		if (input->filename == NULL) {
			input->filename = (char *) xmlStrdup (SystemID);
		}
		input->line = 1;
		input->col = 1;
		input->base = dtdCtxt->input->cur;
		input->cur = dtdCtxt->input->cur;
		input->free = NULL;

		/* lets parse the entity knowing it's an external subset */
		xmlParseExternalSubset (dtdCtxt, ExternalID, SystemID);

		if (dtdCtxt->myDoc != NULL) {
			if (dtdCtxt->wellFormed) {
				ret = dtdCtxt->myDoc->intSubset;
				dtdCtxt->myDoc->intSubset = NULL;
			} else {
				ret = NULL;
			}
			xmlFreeDoc (dtdCtxt->myDoc);
			dtdCtxt->myDoc = NULL;
		}
		xmlFreeParserCtxt (dtdCtxt);

		ctxt->myDoc->extSubset = ret;
	}
}
		
static xmlParserInputPtr
toc_resolveEntity (Context *context, const xmlChar *publicId, const xmlChar *systemId)
{
	xmlParserInputPtr ret;

	ret = resolveEntity (context->ParserCtxt, publicId, systemId);
	return ret;
}

static void 
toc_entityDecl (Context *context, const xmlChar *name, int type,
	 	     const xmlChar *publicId, const xmlChar *systemId, xmlChar *content)
{
	entityDecl (context->ParserCtxt, name, type, publicId, systemId, content);

}

static void
toc_attributeDecl (Context *context, const xmlChar *elem, const xmlChar *name,
              	        int type, int def, const xmlChar *defaultValue,
	      	        xmlEnumerationPtr tree)
{
    attributeDecl(context->ParserCtxt, elem, name, type, def, defaultValue, tree);
}

static void
toc_elementDecl (Context *context, const xmlChar *name, int type,
	    	      xmlElementContentPtr content)
{
    elementDecl(context->ParserCtxt, name, type, content);
}

static void
toc_notationDecl (Context *context, const xmlChar *name,
	     	       const xmlChar *publicId, const xmlChar *systemId)
{
    notationDecl(context->ParserCtxt, name, publicId, systemId);
}

static void
toc_unparsedEntityDecl (Context *context, const xmlChar *name,
		    	     const xmlChar *publicId, const xmlChar *systemId,
		    	     const xmlChar *notationName)
{
	unparsedEntityDecl (context->ParserCtxt, name, publicId, systemId, notationName);
}

static void
toc_reference (Context *context, const xmlChar *name)
{
	reference (context->ParserCtxt, name);
}

static void
toc_processingInstruction (Context *context, const xmlChar *target,
				const xmlChar *data)
{
	processingInstruction (context->ParserCtxt, target, data);
}

static xmlEntityPtr
toc_getParameterEntity (Context *context, const xmlChar *name)
{
	xmlEntityPtr ret;

	ret = getParameterEntity (context->ParserCtxt, name);
	return ret;
}

static xmlSAXHandler parser = {
	(internalSubsetSAXFunc) toc_internalSubset,  /* internalSubset */
	(isStandaloneSAXFunc) toc_isStandalone, /* isStandalone */
	(hasInternalSubsetSAXFunc) toc_hasInternalSubset, /* hasInternalSubset */
	(hasExternalSubsetSAXFunc) toc_hasExternalSubset, /* hasExternalSubset */
	(resolveEntitySAXFunc) toc_resolveEntity, /* resolveEntity */
	(getEntitySAXFunc) get_entity, /* getEntity */
	(entityDeclSAXFunc) toc_entityDecl, /* entityDecl */
	(notationDeclSAXFunc) toc_notationDecl, /* notationDecl */
	(attributeDeclSAXFunc) toc_attributeDecl, /* attributeDecl */
	(elementDeclSAXFunc) toc_elementDecl, /* elementDecl */
	(unparsedEntityDeclSAXFunc) toc_unparsedEntityDecl, /* unparsedEntityDecl */
	NULL, /* setDocumentLocator */
	(startDocumentSAXFunc) start_document, /* startDocument */
	(endDocumentSAXFunc) end_document, /* endDocument */
	(startElementSAXFunc) start_element, /* startElement */
	(endElementSAXFunc) end_element, /* endElement */
	(referenceSAXFunc) toc_reference, /* reference */
	(charactersSAXFunc) toc_characters, /* characters */
	NULL, /* ignorableWhitespace */
	(processingInstructionSAXFunc) toc_processingInstruction, /* processingInstruction */
	(commentSAXFunc) toc_comment, /* comment */
	(warningSAXFunc) toc_warning, /* warning */
	(errorSAXFunc) toc_error, /* error */
	(fatalErrorSAXFunc) fatal_error, /* fatalError */
	(getParameterEntitySAXFunc) toc_getParameterEntity, /*parameterEntity */
	(cdataBlockSAXFunc) NULL
};

static xmlDocPtr
xml_parse_document (char *filename)
{
	/* This function is ripped from parser.c in libxml but slightly
	 * modified so as not to spew debug warnings all around */
	xmlDocPtr ret;
	xmlParserCtxtPtr ctxt;
	char *directory;

	directory = NULL;

	ctxt = xmlCreateFileParserCtxt(filename);
	if (ctxt == NULL) {
		return (NULL);
	}
	ctxt->sax = NULL; /* This line specifically stops the warnings */

	if ((ctxt->directory == NULL) && (directory == NULL))
		directory = xmlParserGetDirectory (filename);
	if ((ctxt->directory == NULL) && (directory != NULL))
		ctxt->directory = (char *) xmlStrdup ((xmlChar *) directory);

	xmlParseDocument (ctxt);

	if (ctxt->wellFormed) {
		ret = ctxt->myDoc;
	} else {
		ret = NULL;
		xmlFreeDoc (ctxt->myDoc);
		ctxt->myDoc = NULL;
	}
	xmlFreeParserCtxt (ctxt);
	
	return (ret);
}

static void
parse_file (char *filename)
{
	Context *context = calloc(1, sizeof(Context));
	
	context->ParserCtxt = xmlNewParserCtxt ();
	xmlInitParserCtxt (context->ParserCtxt);
	context->ParserCtxt->sax = &parser;
	context->ParserCtxt->validate = 1;
	context->ParserCtxt->version = xmlStrdup ("1.0"); 
	context->ParserCtxt->myDoc = xml_parse_document (filename);
	xmlSubstituteEntitiesDefault (1);

	context->elements = toc_extract_elements;
 	context->base_file = strdup (filename);
 	if (xmlSAXUserParseFile (&parser, 
				context, context->base_file) < 0) {
 		sk_warning(0, _("error while parsing %s\n"), filename);
 	}
	
	free(context);
 }

int
main (int argc, char *argv[])
{
	setlocale (LC_ALL, "");
  	bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
  	textdomain (PACKAGE);

	if (argc != 2) {
		printf (_("Usage:  toc <filename>\n\n"));
		return 0;
	}

	parse_file (argv[1]);

	return 0;
}
