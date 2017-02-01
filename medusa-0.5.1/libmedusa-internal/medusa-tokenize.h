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

/* medusa-tokenize.h:  The API for the medusa tokenizing system. */

#ifndef MEDUSA_TOKENIZE_H
#define MEDUSA_TOKENIZE_H

#include "medusa-hash.h"

char *                      medusa_token_to_string                 (MedusaHash *hash,
								    MedusaToken token);
MedusaToken                 medusa_string_to_token                 (MedusaHash *hash,
								    char *str);

gboolean                    medusa_string_has_token                (MedusaHash *hash,
								    char *str);
MedusaToken                 medusa_unsigned_int_to_token           (unsigned int i, 
								    int bits);
/* Convert token to bytes to store in databases */
void                        medusa_token_to_data                   (char *data,
								    MedusaToken token,
								    int bytes);                


char *                      medusa_token_to_bytes                  (MedusaToken token, 
								    int bytes);
MedusaToken                 medusa_bytes_to_token                  (char *data, 
								    int bytes);
#endif /* MEDUSA_TOKENIZE_H */
