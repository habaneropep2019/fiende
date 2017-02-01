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

#include <libmedusa/medusa-utils.h>

#include "medusa-rdb-fields.h"

#include <string.h>

static char *                    medusa_rdb_fields_get_first_title     (const MedusaRDBFields *fields);
static int                       medusa_rdb_fields_get_first_size      (const MedusaRDBFields *fields);
static MedusaRDBEncodeFunc       medusa_rdb_fields_get_first_encoder   (const MedusaRDBFields *fields);
static MedusaRDBDecodeFunc       medusa_rdb_fields_get_first_decoder   (const MedusaRDBFields *fields);
static void                      medusa_rdb_field_free                 (gpointer field,
									gpointer callback_data);


MedusaRDBFieldInfo * 
medusa_rdb_field_info_new ()
{
	MedusaRDBFieldInfo *field_info;
  
	field_info = g_new0 (MedusaRDBFieldInfo, 1);
	field_info->fields = NULL;
	field_info->number_of_fields = 0;
	field_info->record_size = 0;

	return field_info;
  
}

void 
medusa_rdb_field_info_free (MedusaRDBFieldInfo *field_info)
{
	medusa_g_list_free_deep_custom (field_info->fields, medusa_rdb_field_free, NULL);
	g_free (field_info);
}

static void
medusa_rdb_field_free (gpointer field_pointer,
		       gpointer callback_data)
{
	MedusaRDBField *field;

	field = (MedusaRDBField *) field_pointer;
	g_free (field->field_title);
	g_free (field);
}

void
medusa_rdb_field_add (MedusaRDBFieldInfo *field_info,
		      const char *field_title,
		      int field_size,
		      MedusaRDBEncodeFunc encode,
		      MedusaRDBDecodeFunc decode)
{
	MedusaRDBField *new_field;

	new_field = g_new0 (MedusaRDBField, 1);
  
	new_field->field_title = g_strdup (field_title);
	new_field->field_size = field_size;
	new_field->encode = encode;
	new_field->decode = decode;

	field_info->fields = g_list_append (field_info->fields, new_field);
  
	field_info->number_of_fields++;
	field_info->record_size += field_size;
}

/* Removes all fields with field title field_title, and their contents 
   from the database */
void
medusa_rdb_field_remove (MedusaRDBFieldInfo *field_info,
			 char *field_title)
{
	MedusaRDBFields *fields;

	for (fields = field_info->fields; fields != NULL ; fields = field_info->fields->next) {
		if (!strcmp (medusa_rdb_fields_get_first_title (fields), field_title)) {
			field_info->fields = g_list_remove (field_info->fields, fields->data);
			break;
		}
	}
  
}


int
medusa_rdb_field_get_offset (const MedusaRDBFieldInfo *field_info,
			     const char *field_title) 
{
	int offset;
	MedusaRDBFields *fields;

	offset = 0;

	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (strcmp (medusa_rdb_fields_get_first_title (fields), field_title)) {
			offset += medusa_rdb_fields_get_first_size (fields);
		}
		else { 
			break;
		}
	}
	return offset;
}


MedusaRDBField *
medusa_rdb_field_get_field_structure (const MedusaRDBFieldInfo *field_info,
				      const char *field_title)
{
	MedusaRDBFields *fields;
	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (strcmp (field_title, medusa_rdb_fields_get_first_title (fields)) == 0) {
			return (MedusaRDBField *) fields->data;
		}
	}
	g_assert_not_reached ();
	return NULL;
}

static char *    
medusa_rdb_record_get_field_contents (MedusaRDBRecord record,
				      const MedusaRDBFieldInfo *field_info,
				      const char *field_title)
{
	MedusaRDBFields *fields;
	int offset;
	offset = 0;
	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (strcmp (field_title, medusa_rdb_fields_get_first_title (fields)) == 0) {
			return &record[offset];
		}
		else {
			offset += medusa_rdb_fields_get_first_size (fields);
		}
	}
	return NULL;
}

void
medusa_rdb_record_get_field_value (MedusaRDBRecord record,
				   const MedusaRDBFieldInfo *field_info,
				   const char *field_title,
				   gpointer decoder_data,
				   gpointer value_buffer)
{
	const char *field;
	MedusaRDBDecodeFunc decoder;

	/* Get a pointer to the field. */
	field = medusa_rdb_record_get_field_contents (record,
						      field_info,
						      field_title);
	g_return_if_fail (field != NULL);
	
	/* Get the decoder function. */
	decoder = medusa_rdb_field_get_decoder (field_info,
						field_title);
	g_return_if_fail (decoder != NULL);

	/* Decode into the provided buffer. It's the caller's
	 * responsibility to make sure it's big enough.
	 */
	(* decoder) (value_buffer, field, decoder_data);
}

void
medusa_rdb_record_set_field_value (MedusaRDBRecord record,
				   const MedusaRDBFieldInfo *field_info,
				   const char *field_title,
				   gpointer encoder_data,
				   gconstpointer value_buffer)
{
	char *field;
	MedusaRDBEncodeFunc encoder;

	/* Get a pointer to the field. */
	field = medusa_rdb_record_get_field_contents (record,
						      field_info,
						      field_title);
	g_return_if_fail (field != NULL);
	
	/* Get the encoder. */
	encoder = medusa_rdb_field_get_encoder (field_info,
						field_title);
	g_return_if_fail (encoder != NULL);

	/* Since we use memory-mapping, encoding into the
	 * record is all we need to modify the field.
	 */
	(* encoder) (field, value_buffer, encoder_data);
}

int
medusa_rdb_field_get_size (const MedusaRDBFieldInfo *field_info,
			   const char *field_title)
{
	MedusaRDBFields *fields;
	
	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (strcmp (medusa_rdb_fields_get_first_title (fields), field_title) == 0) {
			return medusa_rdb_fields_get_first_size (fields);
		}
	}
	g_assert_not_reached ();
	return 0;
}


MedusaRDBEncodeFunc
medusa_rdb_field_get_encoder  (const MedusaRDBFieldInfo *field_info,
			       const char *field_title) 
{
	MedusaRDBFields *fields;
  
	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (!strcmp (medusa_rdb_fields_get_first_title (fields), field_title)) {
			return medusa_rdb_fields_get_first_encoder (fields);
		}
	}
	g_assert_not_reached ();
	return 0;
}


MedusaRDBDecodeFunc
medusa_rdb_field_get_decoder (const MedusaRDBFieldInfo *field_info,
			      const char *field_title) 
{
	MedusaRDBFields *fields;
  
	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (!strcmp (medusa_rdb_fields_get_first_title (fields), field_title)) {
			return medusa_rdb_fields_get_first_decoder (fields);
		}
	}
	g_assert_not_reached ();
	return 0;
}

int                           
medusa_rdb_field_get_header_offset  (const MedusaRDBFieldInfo *field_info,
				     const char *field_title)
{
	MedusaRDBFields *fields;
	int offset;

	offset = 0;
	for (fields = field_info->fields; fields != NULL; fields = fields->next) {
		if (strcmp (medusa_rdb_fields_get_first_title (fields), field_title)) {
			offset  += strlen (field_title) + 1;
		}
		else {
			return offset;
		}
	}
	g_assert_not_reached ();
	return 0;
}

int
medusa_rdb_field_get_header_size  (const MedusaRDBFieldInfo *field_info,
				   const char *field_title)
{
	return strlen (field_title) + 1;
}



static char *
medusa_rdb_fields_get_first_title (const MedusaRDBFields *fields)
{
	return ((MedusaRDBField *) fields->data)->field_title;
}

static int
medusa_rdb_fields_get_first_size (const MedusaRDBFields *fields)
{
	return ((MedusaRDBField *) fields->data)->field_size;
}


static MedusaRDBEncodeFunc
medusa_rdb_fields_get_first_encoder (const MedusaRDBFields *fields)
{
	return ((MedusaRDBField *) fields->data)->encode;
}


static MedusaRDBDecodeFunc
medusa_rdb_fields_get_first_decoder (const MedusaRDBFields *fields)
{
	return ((MedusaRDBField *) fields->data)->decode;
}

gboolean
medusa_rdb_field_contents_equal (gpointer a, gpointer b, int size)
{
	int i;
#ifdef QUERY_DEBUG
	g_message ("Trying to match %s and %s\n",(char *) a, (char *) b);
#endif
	for (i=0; i < size; i++) {
		if (((char *)a)[i] !=((char *) b)[i]) {
			return FALSE;
		}
#ifdef QUERY_DEBUG
		g_message ("%d\n",i);
#endif
	}
	return TRUE;
}
