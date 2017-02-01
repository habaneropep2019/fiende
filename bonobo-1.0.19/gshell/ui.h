/*
 * Author:
 *   Martin Baulig (baulig@suse.de)
 *
 * Copyright 2000 SuSE GmbH
 */

#ifndef _GSHELL_UI_H_
#define _GSHELL_UI_H_

#include "gshell.h"

void frame_create_ui (Frame *frame);

void frame_set_sensitive (Frame *frame, gboolean sensitive);
void frame_set_zoomable (Frame *frame, gboolean zoomable);

void update_buffer_menu (void);

#endif
