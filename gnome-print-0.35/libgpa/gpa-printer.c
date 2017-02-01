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
#include <gtk/gtkmain.h>
#include <libgnome/gnome-util.h>

#if 0
  #include <sys/stat.h>
#endif	

#include <unistd.h> /* For getpid () */
#include <time.h>   /* For time () */
#include <dirent.h> /* For the DIR structure stuff */

#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-private.h"
#include "gpa-printer.h"
#include "gpa-constraints.h"
#include "gpa-include.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-utils.h"
#include "gpa-values.h"
#include "gpa-vendor.h"
#include "gpa-vendor-private.h"
#include "xml-utils.h"

#include "gpa-printer-private.h"

gboolean debug_turned_on = FALSE;

static void gpa_printer_init       (GpaPrinter *printer);

/* Static functions */
static void gpa_printer_class_init (GpaClass *klass);
static void gpa_printer_finalize   (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

/* Ciclic dependency */
GtkType
gpa_printer_get_type (void)
{
  static GtkType printer_type = 0;

  if (!printer_type)
    {
      GtkTypeInfo printer_info =
      {
	"GpaPrinter",
	sizeof (GpaPrinter),
	sizeof (GpaClass),
	(GtkClassInitFunc)  gpa_printer_class_init,
	(GtkObjectInitFunc) gpa_printer_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      printer_type = gtk_type_unique (gtk_object_get_type (), &printer_info);
    }

  return printer_type;
}

static void
gpa_printer_class_init (GpaClass *class)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) class;

  parent_class = gtk_type_class (gtk_object_get_type ());

  object_class->finalize = gpa_printer_finalize;
}

static void
gpa_printer_init (GpaPrinter *printer)
{
	printer->name     = NULL;
	printer->model    = NULL;
	printer->id       = NULL;
	printer->settings_list = NULL;
	printer->def      = FALSE;
}

static void
gpa_printer_finalize (GtkObject *object)
{
  GpaPrinter *printer;
  GpaSettings *settings;
  GList *list;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GPA_IS_PRINTER (object));

  printer = GPA_PRINTER (object);

  g_print ("Finalizing Printer ..\n");
  if (printer->id)
    g_free (printer->id);
  if (printer->name)
    g_free (printer->name);

  list = printer->settings_list;
  for (; list != NULL; list = list->next) {
	  settings = (GpaSettings *) list->data;
	  gpa_settings_unref (settings);
  }

  /* We should unref the model here */
  
  (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

/**
 * gnome_printer_get_status:
 * @printer: The printer
 *
 * Returns the current status of @printer
 */
GpaStatus
gpa_printer_get_status (GpaPrinter *printer)
{
	g_return_val_if_fail (printer != NULL, GNOME_PRINT_PRINTER_INACTIVE);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), GNOME_PRINT_PRINTER_INACTIVE);

	return GNOME_PRINT_PRINTER_INACTIVE;
}

/**
 * gnome_printer_str_status
 * @status: A status type
 *
 * Returns a string representation of the printer status code @status
 */
const char *
gpa_printer_str_status (GpaStatus status)
{
	switch (status){
	case GNOME_PRINT_PRINTER_ACTIVE:
		return _("Printer is active");

	case GNOME_PRINT_PRINTER_INACTIVE:
		return _("Printer is ready to print");

	case GNOME_PRINT_PRINTER_OFFLINE:
		return _("Printer is off-line");

	case GNOME_PRINT_PRINTER_NET_FAILURE:
		return _("Can not communicate with printer");

	}
	return _("Unknown status");
}

#if 0
static gboolean
gpa_printer_modelinfo_load_from_node (GpaPrinter *printer,
				      xmlNodePtr tree)
{
	xmlNodePtr child;
	xmlNodePtr vendor_node;
	gchar *model_id;
	gchar *model_name;
	gchar *vendor_name;

	debug (FALSE, "");

	child = gpa_xml_search_child_required (tree, GPA_TAG_MODEL_INFO);
	if (child == NULL)
		return FALSE;

	model_name = gpa_xml_get_value_string_required (child, GPA_TAG_NAME, NULL);
	if (model_name == NULL)
		return FALSE;

	model_id = gpa_xml_get_value_string_required (child, GPA_TAG_ID, NULL);
	if (model_id == NULL)
		return FALSE;

	vendor_node =  gpa_xml_search_child_required (child, GPA_TAG_VENDOR_INFO);
	if (vendor_node == NULL)
		return FALSE;
	
	vendor_name = gpa_xml_get_value_string_required (vendor_node, GPA_TAG_NAME, NULL);
	if (vendor_name == NULL)
		return FALSE;

	if (printer->model == NULL)
		printer->model = gpa_model_find_by_vendor_and_model_name (vendor_name,
									  model_name);
	if (printer->model == NULL)
		return FALSE;

	if ((strcmp (vendor_name, printer->model->vendor->name) != 0) ||
	    (strcmp (model_name,  printer->model->name)         != 0) ||
	    (strcmp (model_id,    printer->model->id)           != 0))
	{
		gpa_error ("The File info does not match the index info\n"
			   "pxr:Vendor name %s\ngpi:Vendor name %s\n"
			   "pxr:Model  name %s\ngpi:Model name  %s\n"
			   "pxr:Model  id   %s\ngpi:Model id    %s\n",
			   vendor_name, printer->model->vendor->name,
			   model_name, printer->model->name,
			   model_id,   printer->model->id);
		return FALSE;
	}

	g_free (vendor_name);
	g_free (model_name);
	g_free (model_id);

	gpa_printer_fileinfo_load_from_node (printer, child);
		
	debug (FALSE, "end");
	
	return TRUE;
}
#endif


static xmlNodePtr
gpa_printer_write (XmlParseContext *context, GpaPrinter *printer)
{
	xmlNodePtr child;
	xmlNodePtr node;
	xmlNodePtr item;
	xmlNsPtr   gmr;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	if (!gpa_printer_verify (printer, FALSE)) {
		gpa_error ("Could not save printer, because it could not be verified");
		return NULL;
	}
		
	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_PRINTER, NULL);
	if (node == NULL)
		return NULL;

	gmr = xmlNewNs (node, GPA_TAG_PRINTER_NAME_SPACE, "gmr");
	xmlSetNs (node, gmr);
	context->ns = gmr;

	item = xmlNewChild (node, gmr, GPA_TAG_PRINTER_NAME, printer->name);
	if (item == NULL) {
		gpa_error ("Could not add the key \"%s\" with value \"%s\" to the tree",
			   GPA_TAG_PRINTER_NAME, printer->name);
		return NULL;
	}
	
	item = xmlNewChild (node, gmr, GPA_TAG_PRINTER_ID, printer->id);
	if (item == NULL) {
		gpa_error ("Could not add the key \"%s\" with value \"%s\" to the tree",
			   GPA_TAG_PRINTER_ID, printer->id);
		return NULL;
	}
	
	item = xmlNewChild (node, gmr, GPA_TAG_PRINTER_IS_DEFAULT,
			    printer->def ? GPA_TAG_TRUE : GPA_TAG_FALSE);
	if (item == NULL) {
		gpa_error ("Could not add the key \"%s\" with value \"%s\" to the tree",
			   GPA_TAG_PRINTER_IS_DEFAULT,
			   printer->def ? GPA_TAG_TRUE : GPA_TAG_FALSE);
		return NULL;
	}
	
	item = xmlNewChild (node, gmr, GPA_TAG_MODEL_ID, printer->model->id);
	if (item == NULL) {
		gpa_error ("Could not add the key \"%s\" with value \"%s\" to the tree",
			   GPA_TAG_MODEL_NAME, printer->model->id);
		return NULL;
	}

	child = gpa_settings_list_write (context, printer->settings_list);
	
	if (child != NULL)
		xmlAddChild (node, child);
	
	return node;
}

gboolean
gpa_printer_save (GpaPrinter *printer)
{
	XmlParseContext *context;
	xmlDocPtr xml;
	gchar *file_name;
	gchar *user_dir;
	gint ret;
#if 0	
	struct stat s;
#endif	

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_PRINTER (printer), FALSE);
	g_return_val_if_fail (printer->id != NULL, FALSE);
	g_return_val_if_fail (strlen (printer->id) == GPA_PRINTER_ID_LENGTH, FALSE);
	
	user_dir = gnome_util_home_file ("printers");
#if 1
	if (!g_file_exists (user_dir))
#else	
	if (!stat (user_dir, &s))
#endif	
		gpa_error ("\"%s\" does not exist\n"
			   "Please create the directory.\n"
			   "This is broken, yes. But the real solution is not\n"
			   "to mkdir the directory inside the code. \n"
			   "~/gnome/ is NOT the place to store this info\n"
			   "Sorry.",
			   user_dir);
	file_name = g_strdup_printf ("%s/%s.printer", user_dir, printer->id);
	g_free (user_dir);

	xml = xmlNewDoc ("1.0");
	if (xml == NULL) {
		gpa_error ("Could not create xml doc");
		return FALSE;
	}

	context = gpa_xml_parse_context_new (xml, NULL);
	xml->root = gpa_printer_write (context, printer);
	gpa_xml_parse_context_destroy (context);

	ret = xmlSaveFile (file_name, xml);
	xmlFreeDoc (xml);

	if (ret < 0) {
		gpa_error ("Could not save File");
		return FALSE;
	}

	return TRUE;
}




static gboolean
gpa_printer_find_file (const gchar *file_name, gchar **full_path)
{
	gchar *path;
#if 0	
	struct stat s;
#endif	

#if 1	
	if (!g_file_exists (file_name)) {
#else	
	if (!stat (file_name, &s)) {
#endif	
		gpa_error ("%s file, could not be found\n", file_name);
		return FALSE;
	}

	path = g_strdup (file_name);

	*full_path = path;

	return TRUE;
}

gboolean
gpa_printer_verify (GpaPrinter *printer, gboolean fail)
{
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_PRINTER (printer), FALSE);

	if (printer->name == NULL) {
		gpa_error ("The printer does not containt a name");
		return FALSE;
	}	
	if (printer->id == NULL) {
		gpa_error ("The printer \"%s\" does not containt an id", printer->name);
		return FALSE;
	}	
	if (strlen (printer->id) != GPA_PRINTER_ID_LENGTH) {
		gpa_error ("The printer id is does not contain a valid length (%s)(%i)",
			   printer->id,
			   strlen (printer->id));
		return FALSE;
	}	
	if (printer->model == NULL) {
		gpa_error ("The printer does not contain a model");
		return FALSE;
	}

	if (!gpa_settings_list_verify (printer->settings_list, fail))
		return FALSE;
	
	return TRUE;
}










































GpaPrinter *
gpa_printer_new (void)
{
	GpaPrinter *printer;
  
	printer = gtk_type_new (gpa_printer_get_type ());
	
	return printer;
}


GpaPrinter *
gpa_printer_copy (GpaPrinter *printer)
{
	GpaPrinter *new_printer;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	new_printer = gpa_printer_new ();
	new_printer->name  = g_strdup (printer->name);
	new_printer->id    = g_strdup (printer->id);
	new_printer->model = printer->model;

	new_printer->settings_list = printer->settings_list;
	new_printer->settings_list = gpa_settings_list_copy (printer->settings_list);
	
	if (!gpa_printer_verify (new_printer, TRUE)) {
		gpa_error ("Could not copy the printer");
		return NULL;
	}
	
	return new_printer;
}

static gboolean 
gpa_printer_load_from_node (XmlParseContext *context,
			    xmlNodePtr tree,
			    GpaPrinter *printer)
{
	GpaModel *model;
	xmlNodePtr child;
	gchar *name;
	gchar *id;
	gchar *model_id;
	gchar *def;

	debug (FALSE, "");

	g_return_val_if_fail (context != NULL, FALSE);
	g_return_val_if_fail (tree    != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), FALSE);
	
	if (!gpa_xml_node_verify (tree, GPA_TAG_PRINTER))
		return FALSE;
	
	name     = gpa_xml_get_value_string_required (tree, GPA_TAG_PRINTER_NAME, NULL);
	id       = gpa_xml_get_value_string_required (tree, GPA_TAG_PRINTER_ID, NULL);
	def      = gpa_xml_get_value_string_required (tree, GPA_TAG_PRINTER_IS_DEFAULT, NULL);
	model_id = gpa_xml_get_value_string_required (tree, GPA_TAG_MODEL_ID, NULL);

	if ((name == NULL) || (id == NULL) || (model_id == NULL) || (def == NULL))
		return FALSE;

	model = gpa_model_get_from_id (model_id);
	g_free (model_id);

	if (model == NULL)
		return FALSE;
	
	printer->name  = name;
	printer->id    = id;
	printer->model = model;
	printer->def = (strcmp (def, GPA_TAG_TRUE)==0);
	g_free (def);

	/* Load settings list */
	child = gpa_xml_search_child_required (tree, GPA_TAG_SETTINGS_LIST);
	if (child == NULL)
		return FALSE;
	printer->settings_list = gpa_settings_list_load_from_node (child, printer);
	if (printer->settings_list == NULL)
		return FALSE;

	return TRUE;
}


static GpaPrinter *
gpa_printer_new_from_file (const gchar *file)
{
	GpaPrinter *printer;
	XmlParseContext *context;
	gchar *full_path;

	debug (FALSE, "");

	g_return_val_if_fail (file != NULL, NULL);
	
	if (!gpa_printer_find_file (file, &full_path))
		return FALSE;

	printer = gpa_printer_new ();

	context = gpa_xml_parse_context_new_from_path (full_path,
						       GPA_TAG_PRINTER_NAME_SPACE,
						       GPA_TAG_PRINTER);
	if (context == NULL)
		return NULL;

	if (!gpa_printer_load_from_node (context, context->doc->root, printer)) {
		gpa_error ("The printer \"%s\" could not be loaded", file);
		return NULL;
	}

	gpa_xml_parse_context_free (context);

	if (!gpa_printer_verify (printer, TRUE)) {
		gpa_error ("The printer was not loaded from \"%s\" because it contained "
			   "at least one error", full_path);
		return NULL;
	}

	g_free (full_path);
	
	return printer;

}

static void
gpa_printers_load_from_dir (const char *dirname, GList **printers_list)
{
	GpaPrinter *printer;
	DIR *dir;
	gchar *file_name;
	struct dirent *dent;
	const int expected_length = 40;
	GList *list = NULL;

	debug (FALSE, "");

	g_return_if_fail (dirname != NULL);
	
	dir = opendir (dirname);
	if (!dir)
		return;

	while ((dent = readdir (dir)) != NULL){
		int len = strlen (dent->d_name);

		if (len != expected_length)
			continue;

		if (strcmp (dent->d_name + 32, GPA_PRINTER_EXTENSION_NAME))
			continue;

#if 1
		file_name = g_concat_dir_and_file (dirname, dent->d_name);
#else	
		if (dirname [strlen(dirname) - 1] != PATH_SEP)
			file_name = g_strconcat (dirname, PATH_SEP_STR, dent->d_name, NULL);
		else
			file_name = g_strconcat (dirname, dent->d_name, NULL);
#endif	
		printer   = gpa_printer_new_from_file (file_name);

		if (printer != NULL)
			list = g_list_prepend (list, printer);

		g_free (file_name);
	}
	closedir (dir);

	*printers_list = g_list_reverse (list);

	debug (FALSE, "end");
}

static gchar *
gpa_id_new (void)
{
	gchar *id;

	/* TODO: Use a better method for creatig an
	   id, something like uuid. Chema */
	id = g_strdup_printf ("%.12d%.2d%.6d%.12d",
			      (gint) time(NULL),
			      95,
			      getpid(),
			      (gint) time(NULL));

	if (strlen (id) != GPA_PRINTER_ID_LENGTH) {
		gpa_error ("COuld not generate a unique ID !!!!!!");
		return g_strdup ("00096665730295015604000966657302");
	}

	return id;
}

GpaPrinter *
gpa_printer_new_from_model_info (GpaModelInfo *model_info,
				 const gchar *printer_name)
{
	GpaSettings *settings;
	GpaModel *model;
	GpaPrinter *printer;
	
	g_return_val_if_fail (model_info != NULL, NULL);
	g_return_val_if_fail (model_info->id != NULL, NULL);
	g_return_val_if_fail (printer_name != NULL, NULL);

	model = gpa_model_get_from_id (model_info->id);

	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	printer = gpa_printer_new ();
	printer->name  = g_strdup (printer_name);
	printer->id    = gpa_id_new ();
	printer->model = model;

	settings = gpa_settings_new_from_model (model, printer);

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	
	printer->settings_list = g_list_prepend (printer->settings_list, settings);

	if (!gpa_printer_verify (printer, TRUE)) {
		gpa_error ("Could not greate printer from model info, (!verified)");
		return NULL;
	}
			
	return printer;
}
		

gboolean
gpa_printers_list_load (GList **printers)
{
	gchar *user_dir;

#if 0	
	if (gpa_vendor_list_get () == NULL)
		if (!gpa_vendor_list_new_from_file (GPA_DATA_DIR GPA_TAG_INDEX ".xml"))
			return FALSE;
#else	
	if (gpa_vendor_list_get () == NULL)
		if (!gpa_vendor_list_load_all  ())
			return FALSE;


#endif	

	/* Load printers from the user home dir */
	user_dir = gnome_util_home_file ("printers");
	gpa_printers_load_from_dir (user_dir, printers);
	g_free (user_dir);

	return TRUE;
}



/**
 * gpa_printer_backend_info_get:
 * @printer: The printer to get the info from
 * @backend: the string that distinguish the backend
 * @id: The id for the info
 * 
 * Get information from a printer related to a backend.
 * 
 * Return Value: a pointer to the string for this info, NULL on error
 **/
const gchar *
gpa_printer_backend_info_get (const GpaPrinter *printer,
			      const gchar *backend_name,
			      const gchar *id)
{
	GpaBackend *backend;
	const gchar *info = NULL;
	
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (backend_name != NULL, NULL);
	g_return_val_if_fail (id != NULL, NULL);

	backend = gpa_backend_get_from_id (printer->model, backend_name);
	if (backend == NULL) {
		gpa_error ("Could not get *%s* from printer->model->backend %s\n",
			   id, backend_name);
		return NULL;
	}

	if (backend->values != NULL)
		info = gpa_hash_item_get (backend->values, id);

	return info;
}


void
gpa_printer_set_default (GList *printer_list, GpaPrinter *printer)
{
	GList *list;
	GpaPrinter *item;
	
	g_return_if_fail (GPA_IS_PRINTER (printer));

	/* Scan the printer list for the current default
	 * printer and unset it.
	 */
	list = printer_list;
	for (;list != NULL; list = list->next) {
		item = list->data;
		g_return_if_fail (GPA_IS_PRINTER (item));
		item->def = FALSE;
	}

	printer->def = TRUE;
}

GpaPrinter *
gpa_printer_get_default (GList *printer_list)
{
	GpaPrinter *item;
	GList *list;

	g_return_val_if_fail (printer_list != NULL, NULL);

	/* Scan the printer list for the current default
	 * printer and unset it.
	 */
	list = printer_list;
	for (;list != NULL; list = list->next) {
		item = list->data;
		g_return_val_if_fail (GPA_IS_PRINTER (item), NULL);
		if (item->def)
			return item;
	}

	if (printer_list) {
		GPA_PRINTER (printer_list->data)->def = TRUE;
		return GPA_PRINTER (printer_list->data);
	}
	
	return NULL;
}

/* Access to the struct */
GList *
gpa_printer_settings_list_get (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return printer->settings_list;
}

GList *
gpa_printer_get_backend_list (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
	
	return printer->model->backend_list;
}

gchar *
gpa_printer_dup_name (GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return g_strdup (printer->name);
}

const gchar *
gpa_printer_get_name (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return printer->name;
}

const gchar *
gpa_printer_get_id (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return printer->id;
}

gchar *
gpa_printer_dup_id (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return g_strdup (printer->id);
}


GpaModel *
gpa_printer_get_model (GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return printer->model;
}


void
gpa_printer_settings_append (GpaPrinter *printer,
			     GpaSettings *settings)
{
	g_return_if_fail (GPA_IS_PRINTER (printer));
	g_return_if_fail (GPA_IS_SETTINGS (settings));

	printer->settings_list = g_list_append (printer->settings_list,
						 settings);
}

void
gpa_printer_settings_remove (GpaPrinter *printer,
			     GpaSettings *settings)
{
	g_return_if_fail (GPA_IS_PRINTER (printer));
	g_return_if_fail (GPA_IS_SETTINGS (settings));

	printer->settings_list = g_list_remove (printer->settings_list,
						 settings);
}


GpaSettings *
gpa_printer_settings_get_first (GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return (GpaSettings *) printer->settings_list->data;
}

GpaSettings *
gpa_printer_settings_get_selected (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return gpa_settings_get_selected (printer->settings_list);
}

gint
gpa_printer_settings_list_get_size (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), 0);

	return g_list_length (printer->settings_list);
}

void
gpa_printer_settings_select (const GpaPrinter *printer, GpaSettings *settings)
{
	g_return_if_fail (GPA_IS_PRINTER (printer));
	g_return_if_fail (GPA_IS_SETTINGS (settings));

	gpa_settings_select (settings, printer->settings_list);
}

void
gpa_printer_settings_list_swap (GpaPrinter *printer_1, GpaPrinter *printer_2)
{
	GList *list;
	
	g_return_if_fail (GPA_IS_PRINTER (printer_1));
	g_return_if_fail (GPA_IS_PRINTER (printer_2));
	
	list = printer_1->settings_list;
	printer_1->settings_list = printer_2->settings_list;
	printer_2->settings_list = list;
}

gboolean
gpa_printer_is_default (const GpaPrinter *printer)
{
	g_return_val_if_fail (GPA_IS_PRINTER (printer), FALSE);

	return printer->def;
}
		

