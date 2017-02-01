/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_MONIKER_HTTP_H_
#define _BONOBO_MONIKER_HTTP_H_

#include <bonobo/bonobo-stream.h>

BEGIN_GNOME_DECLS

#define BONOBO_MONIKER_HTTP_TYPE        (bonobo_moniker_http_get_type ())
#define BONOBO_MONIKER_HTTP(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_HTTP_TYPE, BonoboMonikerHTTP))
#define BONOBO_MONIKER_HTTP_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_HTTP_TYPE, BonoboMonikerHTTPClass))
#define BONOBO_IS_MONIKER_HTTP(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_HTTP_TYPE))
#define BONOBO_IS_MONIKER_HTTP_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_HTTP_TYPE))

typedef struct _BonoboMonikerHTTP        BonoboMonikerHTTP;

struct _BonoboMonikerHTTP {
	BonoboMoniker stream;
};

typedef struct {
	BonoboMonikerClass parent_class;
} BonoboMonikerHTTPClass;

GtkType        bonobo_moniker_http_get_type (void);
BonoboMoniker *bonobo_moniker_http_new      (void);
	
END_GNOME_DECLS

#endif /* _BONOBO_MONIKER_HTTP_H_ */
