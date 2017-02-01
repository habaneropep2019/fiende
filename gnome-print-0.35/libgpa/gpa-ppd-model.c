/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000 Jose M Celorio
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *
 */

#include "gpa-defs.h"

#include <glib.h>

#include <unistd.h> /* For getpid () */
#include <time.h>   /* For time() */

#include "gpa-private.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-utils.h"
#include "gpa-vendor.h"
#include "gpa-vendor-private.h"
#include "text-utils.h"

#include "gpa-ppd.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-backend.h"
#include "gpa-ppd-settings.h"
#include "gpa-ppd-utils.h"
#include "gpa-ppd-model.h"
#include "gpa-ppd-options.h"
#include "gpa-ppd-code.h"
#include "gpa-ppd-constraints.h"


typedef enum {
	GPA_VENDOR_DICT_MODEL_NAME,
	GPA_VENDOR_DICT_MANUF_NAME,
	GPA_VENDOR_DICT_ERROR,
} GpaVendorDictType;

typedef struct {
	GpaVendorDictType type;
	gchar *search_for;
	const GpaVendor *vendor;
} PpdVendorDictionary;



/**
 * gpa_ppd_add_model_name:
 * @ppd_info: 
 * 
 * Adds to the ppd_info structure, the model name
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
static gboolean
gpa_ppd_add_model_name (GpaPpdInfo *ppd_info)
{
	debug (FALSE, "");

	g_return_val_if_fail (ppd_info != NULL, FALSE);

	ppd_info->model->name = gpa_ppd_utils_get_string (GPA_PPD_TAG_MODEL_NAME,
							  ppd_info);

	if (ppd_info->model->name == NULL) {
		gpa_ppd_error ("Could not get the model name from the PPD file\n"
			       "The \"%s\" could not be found", GPA_PPD_TAG_MODEL_NAME);
		return FALSE;
	}

	return TRUE;
}



/**
 * gpa_ppd_vendor_dict_to_enum:
 * @string: 
 * 
 * Given the corresponding string for the VendorDictType, it returns the
 * enum asociated by that string, so "ModelName" returns GPA_VENDOR_DICT_MODEL_NAME
 *
 * Return Value: VendorDictType enum.
 **/
static GpaVendorDictType
gpa_ppd_vendor_dict_to_enum (const gchar *string)
{
	debug (FALSE, "");

	if (string == NULL) {
		gpa_ppd_error ("Could not determine the Vendor Dict type, (NULL)");
		return GPA_VENDOR_DICT_ERROR;
	} else if (strcmp (string, "ModelName") == 0) {
		return GPA_VENDOR_DICT_MODEL_NAME;
	} else if (strcmp (string, "Manufacturer") == 0) {
		return GPA_VENDOR_DICT_MANUF_NAME;
	} else {
		gpa_ppd_error ("Could not determine the Vendor Dict type, (%s)", string);
		return GPA_VENDOR_DICT_ERROR;
	}
}

/**
 * gpa_ppd_free_vendor_dictionary:
 * @vendor_dictionary_: 
 * @size: 
 * 
 * Frees the memory used by the PpdVendorDictionary
 * 
 * Return Value: TRUE on sucess, FALSE otherwise
 **/
static gboolean
gpa_ppd_free_vendor_dictionary (PpdVendorDictionary **vendor_dictionary_, gint size)
{
	PpdVendorDictionary *vendor_dictionary;
	gint n;
		
	debug (FALSE, "");

	vendor_dictionary = *vendor_dictionary_;
	
	for (n = 0; n < size; n++) {
		PpdVendorDictionary *item;
		item = &vendor_dictionary [n];
		g_return_val_if_fail (item->search_for != NULL, FALSE);
		g_free (item->search_for);
	}

	g_free (vendor_dictionary);
	*vendor_dictionary_ = NULL;

	return TRUE;
}

#define GPA_NUMBER_OF_ELEMENTS 2
#define GPA_NUMBER_OF_ELEMENTS_GROW 2

/**
 * gpa_ppd_load_vendor_dictionary:
 * @dictionary_: 
 * @size: 
 * 
 * Loads from disk the Vendor Dictionary into the **dictionary strucuture, it allocates
 * the memory necesary and should be freed with gpa_ppd_free_vendor_dictionary
 * 
 * Return Value: 
 **/
static gboolean
gpa_ppd_load_vendor_dictionary (PpdVendorDictionary **dictionary_, gint *size)
{
	PpdVendorDictionary *dictionary;
	GpaVendor *vendor;
	GpaVendorDictType type;
	gboolean dump;
	gchar *full_path;
	gchar *buffer;
	gchar *token;
	gchar *token2;
	gint offset;
	gint buffer_length;
	gint elements_max;
	gint elements_used;
	
	debug (FALSE, "");

	dump = FALSE;
	*dictionary_ = NULL;

	/* Load the buffer */
	full_path = g_strdup_printf ( "%s%s", GPA_DATA_DIR,  "VendorDictionary" /* FIXME, use a tag */);
	buffer = gpa_tu_read_file (full_path, &buffer_length, 0, 0);
	if (buffer == NULL)
		return FALSE;

	if (dump)
		g_print ("Source %s, size %i. File loaded ....\n", full_path, buffer_length);

	/* Allocate the initial size of the dictionary */
	elements_max  = GPA_NUMBER_OF_ELEMENTS;
	elements_used = 0;
	dictionary = g_new (PpdVendorDictionary, elements_max);
	
	offset = 0;
	while (TRUE) {
		token = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
		if (dump)
			g_print ("Token -->%s<--\n", token);
		if ((token == NULL) || token [0] == 0)
			break;

		/* If this line is a comment */
		if (token [0] == '#') {
			if (dump)
				g_print ("Comment found");
			g_free (token);
			gpa_tu_token_next_till_newline (buffer, buffer_length, &offset);
		/* If this line specifies an entry */
		} else {
			type = gpa_ppd_vendor_dict_to_enum (token);
			if (type == GPA_VENDOR_DICT_ERROR)
				return FALSE;
			
			g_free (token);
			token  = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
			gpa_tu_token_next_till (buffer, buffer_length,
						&offset, '\"');
			token2 = gpa_tu_token_next_dup_till (buffer, buffer_length,
							     &offset, '\"');

			if ((token  == NULL) || (token  [0] == 0) ||
			    (token2 == NULL) || (token2 [0] == 0) ||
			    (gpa_tu_string_contains_newline (token2)) ) {
				gpa_ppd_error ("Error while reading VendorDict (%s). The vendor dict line contained "
					       "unbalanced parenthesis in.\n1st token *%s* 2nd token *%s*\n",
					       full_path, token, token2);
				if (token)
					g_free (token);
				if (token2)
					g_free (token2);
				g_free (full_path);
				return FALSE;
			}

			/* Get the vendor. If not found, create it */
			vendor = gpa_vendor_get_from_name (token2);
			g_return_val_if_fail (vendor != NULL, FALSE);
			g_free (token2);

			if (elements_used == elements_max)
				dictionary = g_realloc (dictionary,
							sizeof (PpdVendorDictionary) *
							(elements_max += GPA_NUMBER_OF_ELEMENTS_GROW));
			dictionary [elements_used].type = type;
			dictionary [elements_used].search_for  = token;
			dictionary [elements_used].vendor = vendor;
			elements_used ++;
		}
	}

	*dictionary_ = dictionary;
	*size = elements_used;
	
	return TRUE;
}


static gboolean
gpa_ppd_add_model_vendor (GpaPpdInfo *ppd_info)
{
	const GpaVendor *vendor = NULL;
	PpdVendorDictionary *vendor_dictionary;
	gchar *model_name;
	gchar *manuf_name;
	gint items;
	gint n;
	
	debug (FALSE, "");

	model_name = gpa_ppd_utils_get_string ("ModelName", ppd_info);
	manuf_name = gpa_ppd_utils_get_string ("Manufacturer", ppd_info);
	g_print ("Model Name *%s*\n", model_name);
	g_print ("Manuf Name *%s*\n", manuf_name);

	/* Ok, load the vendor dictionary */
	if (!gpa_ppd_load_vendor_dictionary (&vendor_dictionary, &items))
		return FALSE;
	g_return_val_if_fail (vendor_dictionary != NULL, FALSE);
	
        /* Scan the dictionary to see if we can determine the vendor name */
	for (n = 0; n < items; n++) {
		const PpdVendorDictionary *item;
		item = &vendor_dictionary [n];
		switch (item->type) {
		case GPA_VENDOR_DICT_MODEL_NAME:
			if (model_name)
				if (gpa_tu_search (model_name, item->search_for, FALSE) > -1) {
					vendor = item->vendor;
					break;
				}
			break;
		case GPA_VENDOR_DICT_MANUF_NAME:
			if (manuf_name) 
				if (gpa_tu_search (manuf_name, item->search_for, FALSE) > -1) {
					vendor = item->vendor;
					break;
				}
			break;
		default:
			g_warning ("Vendor dictionary item tag unspecified");
		}
	}

	/* We are done wiht it, free the dictionary */
	if (!gpa_ppd_free_vendor_dictionary (&vendor_dictionary, items))
		return FALSE;

	/* If the PPD had "Vendor" then we can create it */
	if (vendor == NULL && manuf_name)
		vendor = gpa_vendor_get_from_name (manuf_name);
	
	/* FIXME: If we can't determine the vendor, we can either
	 * a) ask the user or
	 * b) add an Unknown vendor
	 * I don't want to do it now so that we can complete the Vendor Dict
	 * as best as we can */
	if (vendor == NULL) {
		gpa_ppd_error ("Could not determine the vendor name from the PPD file\n"
			       "FIXME : we can a) ask the user the vendor or b) use the \"Unknownn\" vendor\n"
			       "ModelName *%s*, Manufacturer *%s*",
			       model_name, manuf_name);
		return FALSE;
	}

	ppd_info->model->vendor = vendor;

	
	g_free (model_name);
	g_free (manuf_name);
	
	return TRUE;
}

static gboolean
gpa_ppd_add_model_id (GpaPpdInfo *ppd_info)
{
	gint temp;

	debug (FALSE, "");

	/* FIXME: Improve ID generation */
	temp = (gint) ((gint)time(NULL) + (long) getpid() * ppd_info->ppd_size);
	temp = 123;

	ppd_info->model->id = g_strdup_printf ("GENERATED-%.12d-", temp);

	if (strlen (ppd_info->model->id) != 23) {
		if (ppd_info->model->id != NULL)
			g_free (ppd_info->model->id);
		gpa_ppd_error ("Could not generate model_id");
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
gpa_ppd_add_model_info (GpaPpdInfo *ppd_info)
{
	GpaModel *model;
	GHashTable *hash;
	gchar *firmware_version;
	gchar *date;
	gchar *creator_name;
	gchar *autogened_source;
	gchar *autogened_type;

	debug (FALSE, "");

	g_return_val_if_fail (ppd_info != NULL, FALSE);
	g_return_val_if_fail (ppd_info->ppd != NULL, FALSE);
	g_return_val_if_fail (ppd_info->model != NULL, FALSE);

	hash = ppd_info->model->model_info;

	model = ppd_info->model;
	g_hash_table_insert (hash, GPA_TAG_MODEL_NAME,  g_strdup (model->name));
	g_hash_table_insert (hash, GPA_TAG_MODEL_ID,    g_strdup (model->id));
	g_hash_table_insert (hash, GPA_TAG_VENDOR_NAME, g_strdup (model->vendor->name));
	
	firmware_version = gpa_ppd_utils_get_string (GPA_PPD_TAG_PSVERSION, ppd_info);
	date             = gpa_utils_convert_date_to_string (time (NULL));
	creator_name     = g_strdup_printf ("gnome-print-admin version %s", VERSION);
	autogened_source = g_strdup (ppd_info->source);
	autogened_type   = g_strdup ("pdd file");

	if (firmware_version != NULL)
		g_hash_table_insert (hash, GPA_TAG_MODEL_FIRMWARE_VERSION,firmware_version);
	g_hash_table_insert (hash, GPA_TAG_FILE_VERSION, g_strdup ("0.0.1"));
	if (date != NULL)
		g_hash_table_insert (hash, GPA_TAG_FILE_CREATION_DATE, date);
	g_hash_table_insert (hash, GPA_TAG_FILE_CREATOR_NAME,     creator_name);
	g_hash_table_insert (hash, GPA_TAG_FILE_AUTOGENED_SOURCE, autogened_source);
	g_hash_table_insert (hash, GPA_TAG_FILE_AUTOGENED_TYPE,   autogened_type);

	return TRUE;
}

gboolean
gpa_ppd_to_model (const gchar *source,
		  GpaModel **model_)
{
	GpaModel *model;
	GpaPpdInfo *ppd_info;
	gchar *ppd_file;
	gint ppd_file_size;

	debug (FALSE, "");

	g_return_val_if_fail (source != NULL, FALSE);

	model = gpa_model_new ();
	model->model_info     = g_hash_table_new (g_str_hash, g_str_equal);
	model->default_values = gpa_ppd_insert_default_settings (NULL);

	ppd_file = gpa_tu_read_file (source, &ppd_file_size, 0, 0);

	if (ppd_file == NULL)
		return FALSE;

	ppd_info = g_new (GpaPpdInfo, 1);
	ppd_info->ppd      = ppd_file;
	ppd_info->ppd_size = ppd_file_size;
	ppd_info->source   = source;
	ppd_info->model    = model;
	ppd_info->backend  = NULL;
	ppd_info->errors   = NULL;

	if (!gpa_ppd_add_model_name     (ppd_info) ||
	    !gpa_ppd_add_model_vendor   (ppd_info) ||
	    !gpa_ppd_add_model_id       (ppd_info) ||
	    !gpa_ppd_add_model_info     (ppd_info) ||
	    !gpa_ppd_add_backend        (ppd_info) ||
	    !gpa_ppd_add_options        (ppd_info) ||
	    !gpa_ppd_add_constraints    (ppd_info) ||
	    !gpa_ppd_add_code_fragments (ppd_info))
		return FALSE;

	g_free (ppd_file);
	g_free (ppd_info);

	if (!gpa_model_verify (model))
		g_warning ("The model could not be verified");

	*model_ = model;
	
	return TRUE;
}

