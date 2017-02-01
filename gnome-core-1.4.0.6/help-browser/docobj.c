/* handles the loading and conversion of help documents into HTML */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "docobj.h"
#include "parseUrl.h"
#include "transport.h"

struct _docObj {
    /* URL information */
    gchar *ref;
    gchar *absoluteRef;
    gchar *humanRef;
    gchar *mimeType;
    DecomposedUrl decomposedUrl;
    
    /* The data */
    guchar *rawData;
    guchar *convData;
    gint rawDataLen;
    gint convDataLen;
    gboolean  freeraw;
    gboolean  freeconv;
    gboolean  useCache;
    gboolean  noMimeOverride;

    /* Transport info */
    TransportMethod   transportMethod;
    TransportFunc     transportFunc;
};

docObj
docObjNew(const gchar *ref, gboolean useCache)
{
	docObj p;

	p = g_malloc(sizeof(*p));
	p->ref = g_strdup(ref);
	p->absoluteRef = NULL;
	p->humanRef = NULL;
	p->mimeType = NULL;
	p->decomposedUrl = NULL;

	p->rawData  = NULL;
	p->convData = NULL;
	p->rawDataLen  = 0;
	p->convDataLen = 0;
	p->freeraw  = FALSE;
	p->freeconv = FALSE;
	p->useCache = useCache;
	p->noMimeOverride = FALSE;
	
	p->transportMethod = TRANS_UNRESOLVED;
	p->transportFunc = transportUnknown;

	return p;
}

void
docObjFree(docObj obj)
{
	g_return_if_fail( obj != NULL );

	g_free(obj->ref);
	g_free(obj->absoluteRef);
	g_free(obj->humanRef);
	g_free(obj->mimeType);
	
	if (obj->freeraw && obj->rawData)
		g_free(obj->rawData);
	if (obj->freeconv && obj->convData)
		g_free(obj->convData);

	if (obj->decomposedUrl)
		freeDecomposedUrl(obj->decomposedUrl);

	g_free(obj);
}

/* parse a URL into component pieces */
void
docObjResolveURL(docObj obj, gchar *currentRef)
{
	DecomposedUrl decomp = NULL;
	gchar *ref;

	g_return_if_fail( obj != NULL );
	g_return_if_fail( obj->ref != NULL );

	if (obj->decomposedUrl)
	    return;

	ref = obj->ref;
	if (*ref == '(') {
	    gchar *type, *s;

	    g_message( "user wants to manually override type\n" );
	    type = ref+1;
	    s = strchr(type, ')');
	    if (s) {
	        ref = s+1;
		*s = 0;
	    }
	    docObjSetMimeType(obj, type);
	    obj->noMimeOverride = TRUE;
	}

	if (isRelative(ref)) {
	    g_message("relative ref: %s", ref);
	    decomp = decomposeUrlRelative(ref, currentRef,
					  &(obj->absoluteRef));
        } else {
	    g_message("absolute ref: %s", ref);
	    decomp = decomposeUrl(ref);
	    obj->absoluteRef = g_strdup(ref);
	}

	g_message("decomposed to: %s, %s, %s, %s", decomp->access, 
		  decomp->host, decomp->path, decomp->anchor);

	/* stupid test for transport types we currently understand */
	if (!strncmp(decomp->access, "file", 4)) {
	    obj->transportMethod = TRANS_FILE;
	    obj->transportFunc   = transportFile;
	} else {
	    obj->transportMethod = TRANS_UNKNOWN;
	    obj->transportFunc   = transportUnknown;
	}

	g_message ("trans: %d", obj->transportMethod);

	if (! obj->humanRef) {
	    docObjSetHumanRef(obj, obj->absoluteRef);
	}

	obj->decomposedUrl = decomp;
}

void
docObjSetRef(docObj obj, gchar *ref)
{
	if (obj->ref)
		g_free(obj->ref);
	obj->ref = g_strdup(ref);
}

gchar *docObjGetRef(docObj obj)
{
    return obj->ref;
}

void
docObjSetHumanRef(docObj obj, gchar *ref)
{
	if (obj->humanRef)
		g_free(obj->humanRef);
	obj->humanRef = g_strdup(ref);
}

gchar *docObjGetHumanRef(docObj obj)
{
    return obj->humanRef;
}

gchar *docObjGetAbsoluteRef(docObj obj)
{
    return obj->absoluteRef;
}

gchar *docObjGetMimeType(docObj obj)
{
    return obj->mimeType;
}

void docObjGetRawData(docObj obj, guchar **s, gint *len)
{
    *s = obj->rawData;
    if (len) {
	*len = obj->rawDataLen;
    }
}

void docObjGetConvData(docObj obj, guchar **s, gint *len)
{
    *s = obj->convData;
    if (len) {
	*len = obj->convDataLen;
    }
}

DecomposedUrl docObjGetDecomposedUrl(docObj obj)
{
    return obj->decomposedUrl;
}

TransportFunc docObjGetTransportFunc(docObj obj)
{
    return obj->transportFunc;
}
    
void docObjSetMimeType(docObj obj, gchar *s)
{
    if (obj->noMimeOverride)
        return;
    if (obj->mimeType)
	g_free(obj->mimeType);

    obj->mimeType = g_strdup(s);
}

void docObjSetRawData(docObj obj, guchar *s, gint len, gboolean freeit)
{
    obj->rawData = s;
    obj->rawDataLen = len;
    obj->freeraw = freeit;
}

void docObjSetConvData(docObj obj, guchar *s, gint len, gboolean freeit)
{
    obj->convData = s;
    obj->convDataLen = len;
    obj->freeconv = freeit;
}

gboolean
docObjUseCache(docObj obj)
{
	return obj->useCache;
}
