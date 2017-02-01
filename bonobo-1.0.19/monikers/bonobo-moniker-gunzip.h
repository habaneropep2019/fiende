/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_MONIKER_GUNZIP_H_
#define _BONOBO_MONIKER_GUNZIP_H_

#include <bonobo/bonobo-stream.h>

BEGIN_GNOME_DECLS

#define BONOBO_MONIKER_GUNZIP_TYPE        (bonobo_moniker_gunzip_get_type ())
#define BONOBO_MONIKER_GUNZIP(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_GUNZIP_TYPE, BonoboMonikerGunzip))
#define BONOBO_MONIKER_GUNZIP_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_GUNZIP_TYPE, BonoboMonikerGunzipClass))
#define BONOBO_IS_MONIKER_GUNZIP(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_GUNZIP_TYPE))
#define BONOBO_IS_MONIKER_GUNZIP_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_GUNZIP_TYPE))

typedef struct _BonoboMonikerGunzip BonoboMonikerGunzip;

struct _BonoboMonikerGunzip {
	BonoboMoniker stream;
};

typedef struct {
	BonoboMonikerClass parent_class;
} BonoboMonikerGunzipClass;

GtkType        bonobo_moniker_gunzip_get_type (void);
BonoboMoniker *bonobo_moniker_gunzip_new      (void);
	
END_GNOME_DECLS

#endif /* _BONOBO_MONIKER_GUNZIP_H_ */
