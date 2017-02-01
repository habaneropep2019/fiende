/*
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 1999 Maurer IT Systemlösungen (http://www.maurer-it.com)
 */

#ifndef _GSHELL_H_
#define _GSHELL_H_

#include <config.h>
#include <gnome.h>
#include <bonobo.h>

typedef struct {
	BonoboUIContainer	*container;
	BonoboUIComponent	*component;
	BonoboWindow		*win;
	GtkWidget		*vbox;
	BonoboViewFrame		*active_view_frame;
	GList			*view_list;
} Frame;

typedef struct {
	BonoboClientSite	*client_site;
	BonoboObjectClient	*server;
	BonoboZoomableFrame	*zoomable_frame;
	gchar			*verb;
	gchar			*name;
	gint			 verb_id;
} Buffer;

typedef struct {
	BonoboItemContainer *container;
	GList *frame_list;
	GList *buffer_list;
	GtkFileSelection *fs;
} Application;

extern Application app;

extern BonoboUIVerb gshell_verbs[];

Frame          *get_active_frame           (GtkWidget *widget);
BonoboViewFrame *get_active_view_frame      (Frame *frame);
Buffer         *buffer_create              (const char *component_goad_id);
Buffer         *buffer_create_for_control  (const char *component_goad_id);
gint            view_remove                (Frame *frame, 
					    BonoboViewFrame *view_frame);
void            buffer_add_view            (Buffer *buffer, 
					    Frame *frame, 
					    gint pos);
gboolean        bonobo_object_has_interface (BonoboObject *obj, 
					    char *interface);
void            set_buffer_cb              (BonoboUIComponent *uih, 
					    void *buffer,  
					    const char *path);

void            open_files                 (Frame *frame,
                                            int argc, char **argv);

void            file_open                  (Frame *frame, char *name);



#endif
