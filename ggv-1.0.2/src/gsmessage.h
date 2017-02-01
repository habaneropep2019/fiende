/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef __GSMESSSAGE_H__
#define __GSMESSAGE_H__

#include "gtkgs.h"
#include "ggvwindow.h"

/* set up status window, clear text field, but do not show window */
void init_gs_status (ggv_window *ggv, gchar *title);

/* show status window (create new if none exists yet) */
void show_gs_status (ggv_window *ggv);

/* add text to status window (create new if none exists yet) */
void add_gs_status_text (ggv_window *ggv, gchar *text, gint show);

#endif
