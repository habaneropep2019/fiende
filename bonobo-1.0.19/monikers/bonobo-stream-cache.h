#ifndef _BONOBO_STREAM_CACHE_H_
#define _BONOBO_STREAM_CACHE_H_

#include <bonobo/bonobo-stream.h>

BEGIN_GNOME_DECLS

#define BONOBO_STREAM_CACHE_TYPE        (bonobo_stream_cache_get_type ())
#define BONOBO_STREAM_CACHE(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_CACHE_TYPE, BonoboStreamCache))
#define BONOBO_STREAM_CACHE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_CACHE_TYPE, BonoboStreamCacheClass))
#define BONOBO_IS_STREAM_CACHE(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_CACHE_TYPE))
#define BONOBO_IS_STREAM_CACHE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_CACHE_TYPE))

typedef struct _BonoboStreamCachePrivate BonoboStreamCachePrivate;

typedef struct {
	BonoboStream stream;

	BonoboStreamCachePrivate *priv;
} BonoboStreamCache;

typedef struct {
	BonoboStreamClass parent_class;
} BonoboStreamCacheClass;

GtkType          bonobo_stream_cache_get_type     (void);
BonoboStream    *bonobo_stream_cache_create       (Bonobo_Stream      cs,
						   CORBA_Environment *opt_ev);
	
END_GNOME_DECLS

#endif /* _BONOBO_STREAM_CACHE_H_ */
