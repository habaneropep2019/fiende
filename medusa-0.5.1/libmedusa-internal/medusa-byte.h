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
 */

/* medusa-byte.h:  Correspondence functions between integers and
   fixed length byte strings.  This is used to encode limited value
   numbers into shorter segments to be stored in a relational database */

#ifndef MEDUSA_BYTE_H
#define MEDUSA_BYTE_H

char *                       medusa_int_to_bytes           (int token, 
                                                            int bytes);
int                          medusa_bytes_to_int           (char *data, 
                                                            int bytes);


#endif /* MEDUSA_BYTE_H */

