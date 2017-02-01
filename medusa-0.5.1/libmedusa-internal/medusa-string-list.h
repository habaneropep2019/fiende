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

/* medusa-on-disk-string-list.h - API for storing strings on disk
   so that they are corresponded to fixed length "tokens" */

#ifndef MEDUSA_STRING_LIST_H
#define MEDUSA_STRING_LIST_H


typedef struct MedusaStringList MedusaStringList;

#ifndef MEDUSA_TOKEN_DEFINED
typedef guint32 MedusaToken;
#define MEDUSA_TOKEN_DEFINED
#endif

typedef enum {
        MEDUSA_STRING_LIST_NO_WRITE_CACHE,
        MEDUSA_STRING_LIST_STRING_TO_KEY_WRITE_CACHE,     
        MEDUSA_STRING_LIST_SINGLE_ELEMENT_WRITE_CACHE  /* for writing the directory names, reuse the last one if it is the same as current */
} MedusaStringListWriteCacheType;

typedef enum {
        MEDUSA_STRING_LIST_NO_READ_CACHE,
        MEDUSA_STRING_LIST_CACHE_KEY_TO_STRING
} MedusaStringListReadCacheType;

MedusaStringList *   medusa_string_list_open           (const char *file_name,
                                                        MedusaStringListReadCacheType cache_type);
MedusaStringList *   medusa_string_list_new            (const char *file_name,
                                                        MedusaStringListWriteCacheType cache_type);

const char       *   medusa_string_list_get_string     (MedusaStringList *string_list,
                                                        MedusaToken key);
MedusaToken          medusa_string_list_store_string   (MedusaStringList *string_list,
                                                        const char *string);

void                 medusa_string_list_destroy        (MedusaStringList *string_list);

void                 medusa_convert_bytes_to_token     (MedusaToken *token,
                                                        const char bytes[4]);
                                                        
void                 medusa_convert_token_to_bytes     (char bytes[4],
                                                        MedusaToken token);
#endif /* MEDUSA_STRING_LIST_H */
