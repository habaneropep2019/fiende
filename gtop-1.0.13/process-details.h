/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#ifndef __PROCESS_DETAILS_H__
#define __PROCESS_DETAILS_H__

#include <gnome.h>
#include <procview.h>
#include <gtop-graph.h>

typedef struct _GTopProcessDetails	GTopProcessDetails;

struct _GTopProcessDetails {
    gint	pid;
    GtkWidget	*dialog, *vb;
    GtkWidget	*status_clist;
    GtkWidget	*args_clist;
    GtkWidget	*credential_clist;
    guint	timeout_id;
    gtop_proc_t	*p;
};

void gtop_process_details (gint pid);

#endif
