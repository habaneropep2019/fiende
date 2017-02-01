#ifndef EEL_DUMB_BOX_H
#define EEL_DUMB_BOX_H

#include <gtk/gtkbin.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#define GTK_TYPE_DUMB_BOX		(eel_dumb_box_get_type ())
#define EEL_DUMB_BOX(obj)		(GTK_CHECK_CAST ((obj), GTK_TYPE_DUMB_BOX, EelDumbBox))
#define EEL_DUMB_BOX_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_DUMB_BOX, EelDumbBoxClass))
#define GTK_IS_DUMB_BOX(obj)		(GTK_CHECK_TYPE ((obj), GTK_TYPE_DUMB_BOX))
#define GTK_IS_DUMB_BOX_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DUMB_BOX))

typedef struct _EelDumbBox	  EelDumbBox;
typedef struct _EelDumbBoxClass  EelDumbBoxClass;

struct _EelDumbBox
{
	GtkBin bin;
};
	
struct _EelDumbBoxClass
{
	GtkBinClass parent_class;
};

GtkType    eel_dumb_box_get_type (void);
GtkWidget* eel_dumb_box_new      (void);

END_GNOME_DECLS

#endif /* EEL_DUMB_BOX_H */
