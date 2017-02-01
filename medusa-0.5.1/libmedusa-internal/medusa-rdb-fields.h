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

/* medusa-rdb-fields.h: The API for the medusa Field structure, which
   contains field names, sizes and encoding information for a field in
   a database, the Fields structure, which is a list of fields, and
   the FieldInfo structure, which is an ordered list of all such
   fields that occur in a database, as well as the total number and
   total size.  */

#ifndef MEDUSA_RDB_FIELDS_H
#define MEDUSA_RDB_FIELDS_H

#include <glib.h>
#include "medusa-rdb-record.h"

typedef struct MedusaRDBFieldInfo MedusaRDBFieldInfo;
typedef struct MedusaRDBField MedusaRDBField;
typedef GList MedusaRDBFields;

typedef void    (*MedusaRDBEncodeFunc) (char *result,
					gconstpointer field,
					gpointer encoding_data);

typedef void    (*MedusaRDBDecodeFunc) (gpointer result,
					const char *field,
					gpointer decoding_data);

/* FIXME bugzilla.eazel.com 2714: 
   These should be private */
struct MedusaRDBFieldInfo {
	MedusaRDBFields *fields;
	int number_of_fields;
	int record_size;
};


struct MedusaRDBField {
	char *field_title;
	int field_size;
	MedusaRDBEncodeFunc encode;
	MedusaRDBDecodeFunc decode;
};

MedusaRDBFieldInfo *medusa_rdb_field_info_new            (void);
void                medusa_rdb_field_info_free           (MedusaRDBFieldInfo       *field_info);  
void                medusa_rdb_field_add                 (MedusaRDBFieldInfo       *field_info,
							  const char               *field_title,
							  int                       field_size,
							  MedusaRDBEncodeFunc       encode,
							  MedusaRDBDecodeFunc       decode);
void                medusa_rdb_field_remove              (MedusaRDBFieldInfo       *field_info,
							  char                     *field_title);

/* Return the data stored in the field entitled field_title in the database record "record".
 * This includes decoding it. The buffer must be the correct size; FILE_NAME_MAXIMUM_LENGTH if the field
 * is unknown or the size of the field if it is known.
 */
void                medusa_rdb_record_get_field_value    (MedusaRDBRecord           record,
							  const MedusaRDBFieldInfo *field_info,
							  const char               *field_title,
							  gpointer                  decoder_data,
							  gpointer                  value_buffer);
void                medusa_rdb_record_set_field_value    (MedusaRDBRecord           record,
							  const MedusaRDBFieldInfo *field_info,
							  const char               *field_title,
							  gpointer                  encoder_data,
							  gconstpointer             value_buffer);

/* Returns the field structure with the field title "field_title" */
MedusaRDBField *    medusa_rdb_field_get_field_structure (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);

/* Get methods */
/* Returns offset of the field's data in a record */
int                 medusa_rdb_field_get_offset          (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);
/* Returns the size in bytes of the field's data */
int                 medusa_rdb_field_get_size            (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);

/* Returns the encoder for the field's data */
MedusaRDBEncodeFunc medusa_rdb_field_get_encoder         (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);
/* Returns the decoder for the field's data */
MedusaRDBDecodeFunc medusa_rdb_field_get_decoder         (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);


/* Returns the offset of the field in the header */
int                 medusa_rdb_field_get_header_offset   (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);

/* Returns the size of the field's name in the header */
int                 medusa_rdb_field_get_header_size     (const MedusaRDBFieldInfo       *field_info,
							  const char                     *field_title);



/* FIXME bugzilla.eazel.com 2715:  
   Move this to medusa-rdb-query, I think, since this is the only place
   where it is used */
gboolean            medusa_rdb_field_contents_equal      (gpointer                  a,
							  gpointer                  b,
							  int                       size);


#endif /* MEDUSA_RDB_FIELDS_H */
