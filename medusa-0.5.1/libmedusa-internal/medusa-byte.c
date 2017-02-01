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
 */

#include <medusa-byte.h>
#include <glib.h>
#include <medusa-tokenize.h>

char *                       
medusa_int_to_bytes (int token, int bytes)
{
        g_assert (bytes <= sizeof (int));
        g_assert (token > (-1 * (1 << (8 * bytes - 1))));
        if (token >= 0) {
                return medusa_token_to_bytes ((unsigned int) token, bytes);
        }
        else {
                return medusa_token_to_bytes ((unsigned int) (-1 * token | (1 << (8 * bytes - 1))), bytes);
        }

}


int
medusa_bytes_to_int (char *data, int bytes)
{
        unsigned int token;
        token = medusa_bytes_to_token (data, bytes);
        if (token & (1 << (8 * bytes - 1))) {
                return (int) -1 * (token - (1 << (8 * bytes - 1)));
        }
        else {
                return token;
        }
}


