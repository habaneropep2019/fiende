#ifndef _GNOME_HELP_DOCOBJ_H_
#define _GNOME_HELP_DOCOBJ_H_

#include <glib.h>

#include "parseUrl.h"

typedef struct _docObj *docObj;
typedef gint(*TransportFunc) (docObj obj);

docObj docObjNew(const gchar *ref, gboolean useCache);
void docObjFree(docObj obj);
void docObjResolveURL(docObj obj, gchar *currentRef);

void   docObjSetRef(docObj obj, gchar *ref);
gchar *docObjGetRef(docObj obj);
void   docObjSetHumanRef(docObj obj, gchar *ref);
gchar *docObjGetHumanRef(docObj obj);
gchar *docObjGetAbsoluteRef(docObj obj);
gchar *docObjGetMimeType(docObj obj);
DecomposedUrl docObjGetDecomposedUrl(docObj obj);
TransportFunc docObjGetTransportFunc(docObj obj);
gboolean docObjUseCache(docObj obj);

void docObjSetMimeType(docObj obj, gchar *s);

void docObjGetRawData(docObj obj, guchar **s, gint *len);
void docObjGetConvData(docObj obj, guchar **s, gint *len);
void docObjSetRawData(docObj obj, guchar *s, gint len, gboolean freeit);
void docObjSetConvData(docObj obj, guchar *s, gint len, gboolean freeit);

#endif





