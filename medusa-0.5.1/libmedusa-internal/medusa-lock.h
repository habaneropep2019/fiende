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
 *  Author: Maciej Stachowiak <mjs@eazel.com>
 *
 *  medusa-lock.h -  Utility functions for managing file locking
 */

#ifndef MEDUSA_LOCK_H
#define MEDUSA_LOCK_H

#include <glib.h>


typedef struct MedusaReadLock MedusaReadLock;
typedef struct MedusaWriteLock MedusaWriteLock;

MedusaReadLock  *medusa_read_lock_get      (const char      *index_name);
void             medusa_read_lock_release  (MedusaReadLock  *lock);

MedusaWriteLock *medusa_write_lock_get     (const char      *index_name);
void             medusa_write_lock_release (MedusaWriteLock *lock);


#endif /* MEDUSA_LOCK_H */

