#ifndef _GNOME_HELP_TRANSPORT_H_
#define _GNOME_HELP_TRANSPORT_H_

#include "docobj.h"
#include "cache.h"

enum _TransportMethod {TRANS_FILE, TRANS_HTTP,
		       TRANS_UNKNOWN, TRANS_UNRESOLVED};

typedef enum _TransportMethod TransportMethod;

gint transport(docObj obj, DataCache cache);

gint transportHTTP(docObj obj);
gint transportFile(docObj obj);
gint transportUnknown(docObj obj);

#endif


