/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#ifndef __CONTROL_PANEL_LIB_H__
#define __CONTROL_PANEL_LIB_H__
#include <gnome.h>

void capplet_corba_gtk_main (void);
void capplet_corba_gtk_main_quit (void);
void capplet_corba_state_changed (gint id, gboolean undoable);
void capplet_corba_changes_are_immediate (gint id);
guint32 _capplet_int_get_xid (gint cid);
gint _capplet_int_get_ccid (gint cid);
gint _capplet_int_get_capid (void);
gint _capplet_int_session_ignore_requested_p(void);
gint _capplet_int_session_initialization_requested_p(void);
void *capplet_widget_corba_init(const char *app_id,
                               const char *app_version,
                               int *argc, char **argv,
                               struct poptOption *options,
                               unsigned int flags,
                               poptContext *return_ctx);
gint get_new_id(void);
#endif
