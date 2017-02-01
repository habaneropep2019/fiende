/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef __CAPPLET_WIDGET_H__
#define __CAPPLET_WIDGET_H__

#include <gtk/gtk.h>
#include <gnome.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define CAPPLET_WIDGET(obj)          GTK_CHECK_CAST (obj, capplet_widget_get_type (), CappletWidget)
#define CAPPLET_WIDGET_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, control_center_widget_get_type (), ControlCenterWidgetClass)
#define IS_CAPPLET_WIDGET(obj)       GTK_CHECK_TYPE (obj, capplet_widget_get_type ())

typedef struct _CappletWidget		        CappletWidget;
typedef struct _CappletWidgetClass		CappletWidgetClass;

struct _CappletWidget
{
	GtkPlug			window;
	int			control_center_id;
        int			capid;
        gboolean 		changed;
        guint32			xid;
	gboolean 		immediate;
};

struct _CappletWidgetClass
{
	GtkPlugClass 		parent_class;

        void (* try) 		(CappletWidget *capplet);
        void (* revert) 	(CappletWidget *capplet);
        void (* ok) 		(CappletWidget *capplet);
        void (* cancel)		(CappletWidget *capplet);
        void (* help) 		(CappletWidget *capplet);
        void (* new_multi_capplet) 	(CappletWidget *capplet);
        void (* page_hidden)	(CappletWidget *capplet);
        void (* page_shown)	(CappletWidget *capplet);
};

guint           capplet_widget_get_type       	(void);
GtkWidget*      capplet_widget_new            	(void);
GtkWidget*      capplet_widget_multi_new       	(gint capid);

        

void		capplet_gtk_main  		(void);
/* returns 0 upon successful initialization.
   returns 1 if --init-session-settings was passed on the cmdline
   returns 2 if --ignore was passed on the cmdline
   returns -1 upon error
*/
gint gnome_capplet_init (const char *app_id, const char *app_version,
                         int argc, char **argv, struct poptOption *options,
                         unsigned int flags, poptContext *return_ctx);
void 		capplet_widget_state_changed 	(CappletWidget *cap, gboolean undoable);
void 		capplet_widget_changes_are_immediate (CappletWidget *cap);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CAPPLET_WIDGET_H__ */
