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

#include <glib.h>

#include "gpa-defs.h"
#include "gpa-tags.h"

#include "gpa-generic-ps.h"

#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-printer.h"
#include "gpa-printer-private.h"
#include "gpa-settings.h"
#include "gpa-vendor.h"
#include "gpa-vendor-private.h"


static GpaVendor *
gpa_generic_ps_vendor (void)
{
	GpaVendor * vendor;

	vendor = gpa_vendor_new ("GNOME");

	return vendor;
}

static GHashTable *
gpa_generic_ps_model_info (void)
{
	GHashTable *hash;


	hash = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (hash, GPA_TAG_MODEL_NAME, _("Generic Postscript (Hardcoded)"));
	g_hash_table_insert (hash, GPA_TAG_MODEL_ID, _("GENERIC-POSTSCRIPT-HARDCODED"));
	g_hash_table_insert (hash, GPA_TAG_MODEL_FIRMWARE_VERSION, _("0"));
	g_hash_table_insert (hash, GPA_TAG_VENDOR_NAME, _("GNOME"));
	g_hash_table_insert (hash, GPA_TAG_FILE_VERSION, _("0.0.0"));
	g_hash_table_insert (hash, GPA_TAG_FILE_CREATION_DATE, _("Unknown"));
	g_hash_table_insert (hash, GPA_TAG_FILE_CREATOR_NAME, _("GnomePrint"));

	return hash;
}


static GpaBackend *
gpa_generic_ps_backend (void)
{
	GpaBackend *backend;

	backend = gpa_backend_new ();

	backend->id = g_strdup ("GNOME");
	backend->values = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (backend->values, GPA_TAG_DRIVER, _("gnome-print-ps2"));

	return backend;
}
	

static GList *
gpa_generic_ps_backend_list ()
{
	GpaBackend *backend;
	GList *list = NULL;

	backend = gpa_generic_ps_backend ();
	list = g_list_prepend (list, backend);

	return list;
}

static GpaOption *
gpa_generic_ps_option_paper (GpaOptions *parent)
{
	GpaOption *option;

	option = gpa_option_new ("US LETTER fixme",
						"Letter",
						parent);

	return option;
}

static GpaOptions *
gpa_generic_ps_options (GpaModel *model)
{
	GpaOptions *options;
	GpaOption  *option;
	GList *list = NULL;
	gchar *path;

	options = gpa_options_new (model,
						  GPA_TAG_MEDIA_SIZE,
						  GPA_TAG_MEDIA_SIZE,
						  GPA_GROUP_PAPER);
	
	options->options_type = GPA_OPTIONS_TYPE_PICKONE;

	option = gpa_generic_ps_option_paper (options);
	list = g_list_prepend (list, option);

	options->children = list;

	path = gpa_option_dup_path (option);
	model->default_settings = g_list_prepend (model->default_settings,
									  path);

	return options;
}


static GList *
gpa_generic_ps_options_list (GpaModel *model)
{
	GpaOptions *options;
	GList *list = NULL;

	options = gpa_generic_ps_options (model);
	list = g_list_prepend (list, options);

	return list;
}

static GpaModel *
gpa_generic_ps_model (void)
{
	GpaModel *model;
	GpaVendor *vendor;

	vendor = gpa_generic_ps_vendor ();

	g_return_val_if_fail (GPA_IS_VENDOR (vendor), NULL);
	
	model = gpa_model_new ();

	vendor->models = g_list_prepend (vendor->models, model);

	model->name   = g_strdup (_("Generic Postscript"));
	model->id     = g_strdup ("GENERIC-PS-INTERNAL----");
	model->vendor = vendor;

	model->code_fragments = NULL;
	model->backend_list = gpa_generic_ps_backend_list ();
	model->options_list = gpa_generic_ps_options_list (model);
	
	model->default_values = NULL;
	model->model_info = gpa_generic_ps_model_info ();

	gpa_model_add_missing_default_settings_and_values (model);
	
	return model;
}

GpaPrinter *
gpa_generic_ps_printer (void)
{
	GpaSettings *settings;
	GpaPrinter *printer;
	GpaModel *model;

	model = gpa_generic_ps_model ();
	
	if (!gpa_model_verify (model))
		return NULL;

	printer = gpa_printer_new ();
	
	printer->name = g_strdup (_("Generic Postscript"));
	printer->id   = g_strdup ("GENERIC_POSTSCRIPT_INTERNAL_____");
	printer->model = model;

	settings = gpa_settings_new_from_model (model, printer);
	if (!gpa_settings_verify (settings, FALSE))
		return NULL;
	
	printer->settings_list = g_list_prepend (NULL, settings);
	printer->def = TRUE;

	if (!gpa_printer_verify (printer, FALSE))
		return NULL;
	
	return printer;
}

