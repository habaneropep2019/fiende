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

#ifndef MEDUSA_AUTHENTICATE_H
#define MEDUSA_AUTHENTICATE_H

#include <glib.h>

void         medusa_authenticate_create_cookie_file        (char *request_line);

gboolean     medusa_authenticate_is_cookie_request         (char *request_line);
gboolean     medusa_authenticate_is_correct_cookie         (int uid, 
							    int gid, 
							    int cookie);
gboolean     medusa_authenticate_user_can_see_file         (char *uri,
							    int uid,
							    GHashTable *cache);

#endif
