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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <medusa-byte.h>
#include <medusa-tokenize.h> 



char *
medusa_token_to_string (MedusaHash *hash, MedusaToken token)
{
  char *result;
  result = medusa_hash_fetch (hash, token);
  if (!result) {
    return "";
  }
  else return result;
}


/* Will create a new token, if none exists for that string */
MedusaToken
medusa_string_to_token (MedusaHash *hash, char *str)
{
  MedusaToken token, original_token;
  unsigned int hash_key;
  char *hash_word;

  hash_key = g_str_hash (str); 
  original_token = medusa_unsigned_int_to_token (hash_key, hash->key_bits);
  /* FIXME bugzilla.eazel.com 1353: 
     Need some way here to deal
     with the case of the hash table being full */
  do {
    hash_key++;
    token = medusa_unsigned_int_to_token (hash_key, hash->key_bits);
    if (token == original_token) {
      g_warning ("Hash table %s is full.  Aborting\n", hash->file_name);
      exit (1);
    }
    hash_word = medusa_hash_fetch (hash, token);
  } while (hash_word && strcmp (hash_word, str));
  if (!hash_word) {
    medusa_hash_store (hash, token, str);
  } 
  return token;
}

gboolean
medusa_string_has_token (MedusaHash *hash, char *str)
{
  unsigned int hash_key;
  MedusaToken token;
  char *hash_word;

  hash_key = g_str_hash (str);
  do {
    hash_key++;
    token = medusa_unsigned_int_to_token (hash_key, hash->key_bits);
    hash_word = medusa_hash_fetch (hash, token);
  } while (hash_word && strcmp (hash_word, str));
  if (hash_word) {
    return TRUE;
  } 
  return FALSE;
}

MedusaToken
medusa_unsigned_int_to_token (unsigned int i, int bits) 
{
  return i & ((1 << bits) - 1);
}

char *                       
medusa_token_to_bytes (MedusaToken token, int bytes)
{
  char *result;
  int i;
  g_assert (bytes <= sizeof (MedusaToken));
  result = g_new0 (char, bytes);
  for (i = 0; i < bytes; i++) {
    result[i] = token & ((1 << 8) - 1);
    token = token >> 8;
  }
  return result;
}

MedusaToken
medusa_bytes_to_token (char *data, int bytes)
{
  MedusaToken token;
  int i;
  token = 0;
  for (i = 0; i < bytes; i++) {
    token += (data[i] & 255) << (i * 8);
      
  }
  return token;
}
