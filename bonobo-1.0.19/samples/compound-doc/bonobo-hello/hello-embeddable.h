#ifndef _INC_HELLO_BONOBO_EMBEDDABLE_H
#define _INC_HELLO_BONOBO_EMBEDDABLE_H

#include <bonobo.h>

typedef struct _HelloBonoboEmbeddable HelloBonoboEmbeddable;
typedef struct _HelloBonoboView       HelloBonoboView;

#define HELLO_BONOBO_EMBEDDABLE_TYPE        (hello_bonobo_embeddable_get_type ())
#define HELLO_BONOBO_EMBEDDABLE(o)          (GTK_CHECK_CAST ((o), HELLO_BONOBO_EMBEDDABLE_TYPE, HelloBonoboEmbeddable))
#define HELLO_BONOBO_EMBEDDABLE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), HELLO_BONOBO_EMBEDDABLE_TYPE, HelloBonoboEmbeddableClass))
#define HELLO_BONOBO_IS_EMBEDDABLE(o)       (GTK_CHECK_TYPE ((o), HELLO_BONOBO_EMBEDDABLE_TYPE))
#define HELLO_BONOBO_IS_EMBEDDABLE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), HELLO_BONOBO_EMBEDDABLE_TYPE))

struct _HelloBonoboEmbeddable {
	BonoboEmbeddable embeddable;
	
	char            *text;
};

typedef struct {
	BonoboEmbeddableClass parent_class;
} HelloBonoboEmbeddableClass;

GtkType hello_bonobo_embeddable_get_type  (void);

HelloBonoboEmbeddable *
        hello_bonobo_embeddable_construct (HelloBonoboEmbeddable *embeddable);
void    hello_bonobo_embeddable_set_text  (HelloBonoboEmbeddable *embeddable,
					   char                  *text);

#endif /* _INC_HELLO_BONOBO_EMBEDDABLE_H */
