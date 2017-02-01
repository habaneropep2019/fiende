#include <gnome.h>

#include "hello-embeddable.h"
#include "hello-view.h"

static BonoboViewClass *hello_bonobo_view_parent_class = NULL;

void
hello_view_update (HelloBonoboView       *view,
		   HelloBonoboEmbeddable *embeddable)
{
	if (embeddable && embeddable->text)
		gtk_label_set (GTK_LABEL (view->label),
			       embeddable->text);
}

static void
view_change_string_cb (gchar *txt, gpointer data)
{
	BonoboView *view = data;
	HelloBonoboEmbeddable *embeddable;

	embeddable = HELLO_BONOBO_EMBEDDABLE (
		bonobo_view_get_embeddable (BONOBO_VIEW (view)));
			
	if (txt)
		hello_bonobo_embeddable_set_text (embeddable, txt);
	/* else
		Canceled */
}

static void
button_clicked_cb (GtkWidget *caller, HelloBonoboView *view)
{
	gchar *txt;

	gtk_label_get (GTK_LABEL (view->label), &txt);

	gnome_request_dialog (FALSE, "Enter new text", txt, 0,
			      view_change_string_cb, view, NULL);
}

static GtkWidget *
view_new (HelloBonoboView *view)
{
	view->label = gtk_label_new ("");

	view->button = gtk_button_new_with_label ("Change text");
	gtk_signal_connect (GTK_OBJECT (view->button), "clicked",
			    GTK_SIGNAL_FUNC (button_clicked_cb),
			    view);

	view->vbox = gtk_vbox_new (FALSE, 10);

	gtk_container_add (GTK_CONTAINER (view->vbox), view->label);
	gtk_container_add (GTK_CONTAINER (view->vbox), view->button);

	return view->vbox;
}

BonoboView *
hello_bonobo_view_factory (BonoboEmbeddable      *embeddable,
			   const Bonobo_ViewFrame view_frame,
			   void                  *closure)
{
	HelloBonoboView *view;
	GtkWidget       *widget;

	view = gtk_type_new (HELLO_BONOBO_VIEW_TYPE);

	widget = view_new (view);
	gtk_widget_show_all (widget);
	
	view = HELLO_BONOBO_VIEW (
		bonobo_view_construct (BONOBO_VIEW (view), widget));
	if (!view)
		return NULL;

	bonobo_view_set_view_frame (BONOBO_VIEW (view), view_frame);

	hello_view_update (view, HELLO_BONOBO_EMBEDDABLE (embeddable));

	return BONOBO_VIEW (view);
}

static void
hello_bonobo_view_destroy (GtkObject *object)
{
	HelloBonoboView *view;

	g_return_if_fail (object != NULL);
	g_return_if_fail (HELLO_BONOBO_IS_VIEW (object));
	
	view = HELLO_BONOBO_VIEW (object);

	gtk_widget_destroy (view->vbox);

	GTK_OBJECT_CLASS (hello_bonobo_view_parent_class)->destroy (object);
}

static void 
hello_bonobo_view_activate (BonoboControl *control, gboolean state)
{
	bonobo_view_activate_notify (BONOBO_VIEW (control), state);
}

static void
hello_bonobo_view_class_init (HelloBonoboViewClass *klass)
{
	GtkObjectClass  *object_class = (GtkObjectClass *) klass;
	BonoboControlClass *control_class = (BonoboControlClass *) klass;

	hello_bonobo_view_parent_class =
		gtk_type_class (bonobo_view_get_type ());

	control_class->activate = hello_bonobo_view_activate;

	object_class->destroy = hello_bonobo_view_destroy;
}

static void
hello_bonobo_view_init (HelloBonoboView *view)
{
}

BONOBO_X_TYPE_FUNC (HelloBonoboView, 
		      bonobo_view_get_type (),
		      hello_bonobo_view);
