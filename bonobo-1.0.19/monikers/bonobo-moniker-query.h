/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_MONIKER_QUERY_H_
#define _BONOBO_MONIKER_QUERY_H_

BEGIN_GNOME_DECLS

#define BONOBO_MONIKER_QUERY_TYPE        (bonobo_moniker_query_get_type ())
#define BONOBO_MONIKER_QUERY(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_QUERY_TYPE, BonoboMonikerQuery))
#define BONOBO_MONIKER_QUERY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_QUERY_TYPE, BonoboMonikerQueryClass))
#define BONOBO_IS_MONIKER_QUERY(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_QUERY_TYPE))
#define BONOBO_IS_MONIKER_QUERY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_QUERY_TYPE))

typedef struct _BonoboMonikerQuery        BonoboMonikerQuery;
typedef struct _BonoboMonikerQueryPrivate BonoboMonikerQueryPrivate;

struct _BonoboMonikerQuery {
	BonoboMoniker stream;

	BonoboMonikerQueryPrivate *priv;
};

typedef struct {
	BonoboMonikerClass parent_class;
} BonoboMonikerQueryClass;

GtkType        bonobo_moniker_query_get_type  (void);
BonoboMoniker *bonobo_moniker_query_construct (BonoboMonikerQuery *stream,
					       Bonobo_Moniker corba_stream);
BonoboMoniker *bonobo_moniker_query_new       (void);
	
END_GNOME_DECLS

#endif /* _BONOBO_MONIKER_QUERY_H_ */
