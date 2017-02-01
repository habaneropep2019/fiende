/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2001 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 *
 *  medusa-system-state.h -- API to turn off medusa's daily running, turn it back on,
 *  and check if medusa has been blocked on a system.
 */

#ifndef MEDUSA_SYSTEM_STATE_H
#define MEDUSA_SYSTEM_STATE_H

#include <glib.h>
/* This API is used to determine whether medusa is on on the system or not. */
gboolean    medusa_system_services_are_enabled               (void);

const char *medusa_get_configuration_file_path               (void);

/* These are currently disabled, and need repair before they may be used again. */
typedef void (* MedusaSystemStateFunc)     (gpointer data);


/* These functions need to be called within a gtk_main_loop */

/* Returns an update function id, that can be used to remove the callback with  medusa_system_services_remove_update_function */
int         medusa_execute_when_system_state_changes                  (MedusaSystemStateFunc callback,
                                                                       gpointer callback_data);
int         medusa_execute_once_when_system_state_changes             (MedusaSystemStateFunc callback,
                                                                       gpointer callback_data);

void        medusa_remove_state_changed_function                      (int update_function_id);



#endif /* MEDUSA_SYSTEM_STATE_H */


