/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
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
 *  medusa-idled-client.h - Daemon that runs as part of 
 *  an X session and notifies the medusa indexer (and possibly others)
 *  to back off when user activity happens
 */

#ifndef MEDUSA_IDLED_CLIENT_H
#define MEDUSA_IDLED_CLIENT_H

typedef enum {
        USER_ACTIVITY_PRESENT,
        USER_ACTIVITY_ABSENT,
        USER_ACTIVITY_UNKNOWN
        
} MedusaIdleStatus; 

typedef struct MedusaIdledConnection MedusaIdledConnection;

/* Create an initial connect with the idle server */
MedusaIdleStatus        medusa_idle_service_register                          (MedusaIdledConnection **idled_connection);

/* Request the machine's current status: idle or non-idle */
MedusaIdleStatus        medusa_idle_service_request_current_idle_status       (MedusaIdledConnection *idled_connection);

void                    medusa_idle_service_unregister_and_destroy_connection (MedusaIdledConnection *idled_connection);
void                    medusa_idle_service_sleep_until_idle                  (MedusaIdledConnection *idled_connection);

#endif /* MEDUSA_IDLED_CLIENT_H */
