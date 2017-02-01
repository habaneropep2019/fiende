#include "hello-embeddable.h"
#include "hello-view.h"
#include "hello-object-io.h"
#include "hello-object-print.h"

void
hello_bonobo_embeddable_set_text (HelloBonoboEmbeddable *embeddable,
				  char                  *text)
{
	g_free (embeddable->text);

	embeddable->text = g_strdup (text);

	bonobo_embeddable_foreach_view (
		BONOBO_EMBEDDABLE (embeddable),
		(BonoboEmbeddableForeachViewFn) hello_view_update,
		embeddable);
}

HelloBonoboEmbeddable *
hello_bonobo_embeddable_construct (HelloBonoboEmbeddable *embeddable)
{
	BonoboPersistStream *stream;
	BonoboPrint         *print;

	g_return_val_if_fail (HELLO_BONOBO_IS_EMBEDDABLE (embeddable), NULL);

	bonobo_embeddable_construct (BONOBO_EMBEDDABLE (embeddable),
				     hello_bonobo_view_factory, NULL);


	/* Register the Bonobo::PersistStream interface. */
	stream = bonobo_persist_stream_new (hello_object_pstream_load,
					    hello_object_pstream_save,
					    hello_object_pstream_get_max_size,
					    hello_object_pstream_get_types,
					    embeddable);
	if (!stream) {
		bonobo_object_unref (BONOBO_OBJECT (embeddable));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (embeddable),
				     BONOBO_OBJECT (stream));

	/* Register the Bonobo::Print interface */
	print = bonobo_print_new (hello_object_print, embeddable);
	if (!print) {
		bonobo_object_unref (BONOBO_OBJECT (embeddable));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (embeddable),
				     BONOBO_OBJECT (print));

	return embeddable;
}

static BonoboEmbeddableClass *hello_bonobo_embeddable_parent_class = NULL;

static void
hello_bonobo_embeddable_destroy (GtkObject *object)
{
	HelloBonoboEmbeddable *embeddable = HELLO_BONOBO_EMBEDDABLE (object);

	g_free (embeddable->text);

	GTK_OBJECT_CLASS (hello_bonobo_embeddable_parent_class)->destroy (object);
}

static void
hello_bonobo_embeddable_class_init (BonoboEmbeddableClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	hello_bonobo_embeddable_parent_class =
		gtk_type_class (bonobo_embeddable_get_type ());

	object_class->destroy = hello_bonobo_embeddable_destroy;
}

static void
hello_bonobo_embeddable_init (BonoboObject *object)
{
	HelloBonoboEmbeddable *embeddable = HELLO_BONOBO_EMBEDDABLE (object);

	embeddable->text = g_strdup ("Hello World");
}


BONOBO_X_TYPE_FUNC (HelloBonoboEmbeddable, 
		      bonobo_embeddable_get_type (),
		      hello_bonobo_embeddable);
