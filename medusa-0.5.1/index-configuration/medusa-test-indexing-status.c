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
 *  medusa-test-indexing-status.c -- Simple program used by shell scripts to determine
 *  whether indexing is enabled.
 */

#include <libmedusa/medusa-system-state.h>
#include <stdio.h>

int
main (int argc, char **argv)
{
        if (medusa_system_services_are_enabled ()) {
                printf ("Indexing is enabled.\n");
                return 0;
        }
        else {
                printf ("Indexing is not enabled.\n");
                return 1;
        }
}
