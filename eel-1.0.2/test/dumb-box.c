#include <config.h>
#include "dumb-box.h"
#include <eel/eel-gtk-macros.h>
#include <eel/eel-gtk-extensions.h>

#include <gtk/gtksignal.h>

static void eel_dumb_box_initialize_class (EelDumbBoxClass *dumb_box_class);
static void eel_dumb_box_initialize       (EelDumbBox      *dumb_box);
static gint eel_dumb_box_expose           (GtkWidget       *widget,
					   GdkEventExpose  *event);

EEL_DEFINE_CLASS_BOILERPLATE (EelDumbBox, eel_dumb_box, GTK_TYPE_BIN)

static void
eel_dumb_box_initialize_class (EelDumbBoxClass *dumb_box_class)
{
	GtkWidgetClass *widget_class;
	
	widget_class = GTK_WIDGET_CLASS (dumb_box_class);

	widget_class->realize = eel_gtk_widget_standard_realize;
	widget_class->size_request = eel_gtk_bin_standard_size_request;
	widget_class->size_allocate = eel_gtk_bin_standard_size_allocate;
	widget_class->expose_event = eel_dumb_box_expose;
}

static void
eel_dumb_box_initialize (EelDumbBox *dumb_box)
{
	GTK_WIDGET_UNSET_FLAGS (dumb_box, GTK_NO_WINDOW);
}

GtkWidget*
eel_dumb_box_new (void)
{
	return gtk_widget_new (GTK_TYPE_DUMB_BOX, NULL);
}

static gint
eel_dumb_box_expose (GtkWidget *widget,
		     GdkEventExpose *event)
{
	GtkBin *bin;
	GdkEventExpose child_event;

	g_return_val_if_fail (GTK_IS_DUMB_BOX (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	bin = GTK_BIN (widget);
	
	child_event = *event;
	if (bin->child && GTK_WIDGET_NO_WINDOW (bin->child)
	    && gtk_widget_intersect (bin->child, &event->area, &child_event.area)) {
		gtk_widget_event (bin->child, (GdkEvent*) &child_event);
	}
	
	return FALSE;
}
