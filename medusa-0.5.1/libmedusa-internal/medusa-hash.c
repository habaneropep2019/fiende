/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

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


#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "medusa-hash.h"


#define MEDUSA_HASH_MAGIC_NUMBER "9121"
#define MEDUSA_HASH_VERSION "0.1"
#define MEDUSA_INSERT_BUFFER_SIZE 5*getpagesize()

static void           medusa_hash_destroy             (MedusaHash *hash) ;


MedusaHash *
medusa_hash_open (const char *file_name,
		  int bits)
{
	MedusaHash *hash;
	
	hash = g_new0 (MedusaHash, 1);
	hash->key_bits = bits;
	hash->file_name = g_strdup (file_name);
	
	/* The first part of the data is an array of offsets of the values.
	   each cell has size sizeof (int), and there are 1<<hash->key_bits cells */
	hash->address_length = (1 << hash->key_bits) * sizeof (int);


	hash->io_handler = medusa_io_handler_open (file_name, 
						   MEDUSA_HASH_MAGIC_NUMBER, 
						   MEDUSA_HASH_VERSION);
	if (hash->io_handler == NULL) {
		/* FIXME bugzilla.eazel.com 4557: need to log the error here */
		return NULL;
	}
	/* values are found by using the offsets that get stored in the address array.
	   a 0 offset means not found, so we start the values at 1.  The other variables
	   here (insertion point and buffer space available) are augmented to 
	   reflect this change */
	hash->value_length = hash->io_handler->file_size - 
		(hash->io_handler->header_length + hash->address_length + hash->io_handler->buffer_space_available) + 1;
	hash->io_handler->insertion_point++;
	hash->io_handler->buffer_space_available--;

	g_assert (hash->value_length > 0);

	/* Keep precomputed pointers to the address section of the
	   hash, and the values section of the hash */
	hash->addresses = (int *) &hash->io_handler->mapped_region[hash->io_handler->header_length];
	hash->values = &hash->io_handler->mapped_region[hash->io_handler->header_length + hash->address_length];
  
	hash->ref_count = 1;
	return hash;
}

MedusaHash *
medusa_hash_new (const char *file_name, int bits)
{
	MedusaHash *hash;

	/* Don't create over files that don't exist */
	g_return_val_if_fail (access (file_name, X_OK) == -1, NULL);
	
	hash = g_malloc (sizeof (MedusaHash));
	hash->key_bits = bits;
	hash->file_name = g_strdup (file_name);

	/* The first part of the data is an array of offsets of the values.
	   each cell has size sizeof (int), and there are 1<<hash->key_bits cells */
	hash->address_length = (1 << hash->key_bits) * sizeof (int);


	hash->io_handler = medusa_io_handler_new (file_name, 
						  MEDUSA_HASH_MAGIC_NUMBER, 
						  MEDUSA_HASH_VERSION, 
						  hash->address_length);

	if (hash->io_handler == NULL) {
		/* FIXME bugzilla.eazel.com 4557: log error */
		g_warning ("Couldn't create a file for %s\n", file_name);
		return NULL;
	}
	/* values are found by using the offsets that get stored in the address array.
	   a 0 offset means not found, so we start the values at 1.  The other variables
	   here (insertion point and buffer space available) are augmented to 
	   reflect this change */
	hash->value_length = hash->io_handler->file_size - 
		(hash->io_handler->header_length + hash->address_length + hash->io_handler->buffer_space_available) + 1;
	hash->io_handler->insertion_point++;
	hash->io_handler->buffer_space_available--;

	g_assert (hash->value_length > 0);

	/* Keep precomputed pointers to the address section of the
	   hash, and the values section of the hash */
	hash->addresses = (int *) &hash->io_handler->mapped_region[hash->io_handler->header_length];
	hash->values = &hash->io_handler->mapped_region[hash->io_handler->header_length + hash->address_length];
  
	hash->ref_count = 1;
	return hash;
}

void
medusa_hash_ref (MedusaHash *hash)
{
	g_assert (hash->ref_count > 0);
	hash->ref_count++;

}

void
medusa_hash_unref (MedusaHash *hash) 
{
	g_assert (hash->ref_count > 0);
	if (hash->ref_count == 1) {
		medusa_hash_destroy (hash);
	}
	else {
		hash->ref_count--;
	}
}

static void 
medusa_hash_destroy (MedusaHash *hash) 
{
	g_assert (hash->ref_count == 1);
	
	g_free (hash->file_name);
	medusa_io_handler_free (hash->io_handler);
	g_free (hash);
}


char *
medusa_hash_fetch (MedusaHash *hash, MedusaToken key) {

	g_return_val_if_fail (key < (1 << hash->key_bits), NULL);

	if (medusa_hash_key_exists (hash, key)) {
		return &hash->values[hash->addresses[key]];
	}
	else {
		return NULL;
	}

}

static void
medusa_hash_remap (MedusaHash *hash) {


	medusa_io_handler_remap (hash->io_handler);

	hash->addresses = (int *) &hash->io_handler->mapped_region[hash->io_handler->header_length];
	hash->values = &hash->io_handler->mapped_region[hash->io_handler->header_length + hash->address_length];


}

void
medusa_hash_store (MedusaHash *hash, MedusaToken key, const char *content) {

	if (strlen (content) >= hash->io_handler->buffer_space_available) {
		medusa_hash_remap (hash);
		medusa_hash_store (hash, key, content);
	}
	else {
		hash->addresses[key] = hash->value_length;
		medusa_io_handler_append_string (hash->io_handler, content);
		hash->value_length += strlen(content) + 1;
	}
    
}

int 
medusa_hash_key_exists (MedusaHash *hash, MedusaToken key) {
	g_assert (key < 1<<hash->key_bits);
	return hash->addresses[key];
}



