#ifndef __DETAILS_H__
#define __DETAILS_H__

#include <gnome.h>

#include <procview.h>

#include <gtop-graph.h>

void procview_details (GTopProcViewData *d, gint pid);

GtkWidget *gtop_details_create_mem_graph (gint pid);
GtkWidget *gtop_details_create_mem_map (gint pid);

extern GtkTargetEntry gtop_target_table[];

#endif
