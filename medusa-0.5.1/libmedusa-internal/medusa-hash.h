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

/* medusa-hash.h:  API for medusa hashes:  On disk hashes that convert strings
   to unsigned ints, and back.  This can be used to create sets of strings and
   test for the membership of strings in that set.  */

#ifndef MEDUSA_HASH_H
#define MEDUSA_HASH_H

#include "medusa-io-handler.h"


typedef struct MedusaHash MedusaHash;

#ifndef MEDUSA_TOKEN_DEFINED
typedef guint32 MedusaToken;
#define MEDUSA_TOKEN_DEFINED
#endif

struct MedusaHash {

        char *file_name;
        int key_bits;
  
        MedusaIOHandler *io_handler;

        /* The memory mapped location of the string addresses in the data file */
        int *addresses;
        int address_length;
        /* The memory mapped location of the strings in the data file */
        char *values;
        /* Length of space taken up by values */
        int value_length;

        int ref_count;
  
};

MedusaHash *           medusa_hash_open                     (const char *file_name, 
							     int bits);
MedusaHash *           medusa_hash_new                      (const char *file_name, 
							     int bits);
void                   medusa_hash_init_new_file            (MedusaHash *hash, 
							     int bits);
char *                 medusa_hash_fetch                    (MedusaHash *hash, 
							     MedusaToken key);
void                   medusa_hash_store                    (MedusaHash *hash, 
							     MedusaToken key, 
							     const char *content);
void                   medusa_hash_ref                      (MedusaHash *hash);
void                   medusa_hash_unref                    (MedusaHash *hash);
int                    medusa_hash_key_exists               (MedusaHash *hash, 
							     MedusaToken key);

#endif /* MEDUSA_HASH_H */
