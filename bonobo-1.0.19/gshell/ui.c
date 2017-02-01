/*
 * menu/toolbar related code
 *
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 1999 Maurer IT Systemlösungen (http://www.maurer-it.com)
 */

#include <config.h>
#include "ui.h"
#include "inout.h"

void 
frame_create_ui (Frame *frame)
{
	Bonobo_UIContainer remote_uic;

	g_return_if_fail (frame != NULL);

	remote_uic = BONOBO_OBJREF (frame->container);

	frame->component = bonobo_ui_component_new ("GShell");
	bonobo_ui_component_set_container (frame->component, remote_uic);

        bonobo_ui_util_set_ui (frame->component, DATADIR, "gshell-ui.xml", "gshell");

	bonobo_ui_component_add_verb_list_with_data (frame->component, gshell_verbs, frame);
	bonobo_ui_component_add_verb_list_with_data (frame->component, inout_verbs, frame);

	bonobo_ui_component_thaw (frame->component, NULL);
}

void
frame_set_sensitive (Frame *frame, gboolean sensitive)
{
	char *ssensitive = sensitive ? "1" : "0";

	g_return_if_fail (frame != NULL);

	bonobo_ui_component_set_prop (frame->component, "/commands/FileLoad",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/FilePrint",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/FileSave",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/FileSaveAs",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/FileKill",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/WindowSplit",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/WindowOne",
				      "sensitive", ssensitive, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/WindowZoom",
				      "sensitive", ssensitive, NULL);
}

void
frame_set_zoomable (Frame *frame, gboolean zoomable)
{
	char *szoomable = zoomable ? "1" : "0";

	g_return_if_fail (frame != NULL);

	bonobo_ui_component_set_prop (frame->component, "/commands/ZoomIn",
				      "sensitive", szoomable, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/ZoomOut",
				      "sensitive", szoomable, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/ZoomToFit",
				      "sensitive", szoomable, NULL);
	bonobo_ui_component_set_prop (frame->component, "/commands/ZoomToDefault",
				      "sensitive", szoomable, NULL);
}

void
update_buffer_menu (void)
{
	GList *f,*b;

	/* fixme: set the right menu pixmap (from mime type) */

	for (f = app.frame_list; f; f = f->next) {
		Frame *frame  = f->data;
		BonoboUINode *parent;

		bonobo_ui_component_rm (frame->component,
					"/menu/Buffers/BufferList/", NULL);

		parent = bonobo_ui_util_new_placeholder ("BufferList", FALSE, FALSE);

		for (b = app.buffer_list; b; b = b->next) {
			Buffer *buffer = b->data;
			BonoboUINode *node;

			node = bonobo_ui_util_new_menu (FALSE, buffer->verb, g_basename (buffer->name),
							NULL, buffer->verb);

			bonobo_ui_node_set_data (node, buffer);

			bonobo_ui_node_add_child (parent, node);
		}

		bonobo_ui_component_set_tree (frame->component,
					      "/menu/Buffers/", parent, NULL);
	}
}
