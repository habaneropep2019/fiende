/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#ifndef __CAPPLET_MANAGER_H__
#define __CAPPLET_MANAGER_H__

#include "control-center.h"
#include "tree.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void launch_capplet (node_data *data, gboolean exec_new);
node_data *find_node_by_id (gint id);
void revert_all (void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
