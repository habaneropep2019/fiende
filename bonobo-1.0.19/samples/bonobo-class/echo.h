/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef ECHO_H_
#define ECHO_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

#define ECHO_TYPE        (echo_get_type ())
#define ECHO(o)          (GTK_CHECK_CAST ((o), ECHO_TYPE, Echo))
#define ECHO_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), ECHO_TYPE, EchoClass))
#define IS_ECHO(o)       (GTK_CHECK_TYPE ((o), ECHO_TYPE))
#define IS_ECHO_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), ECHO_TYPE))

typedef struct {
	BonoboXObject parent;

	char *instance_data;
} Echo;

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_Sample_Echo__epv epv;
} EchoClass;

GtkType   	    echo_get_type  (void);
Echo      	   *echo_new       (void);

END_GNOME_DECLS

#endif /* ECHO_H_ */
