/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  medusa-text-index-mime-module.h : Management of 
 *  different functions that can index different kinds
 *  of text files
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

#ifndef MEDUSA_TEXT_INDEX_MIME_MODULE_H
#define MEDUSA_TEXT_INDEX_MIME_MODULE_H


#include <glib.h>
typedef struct MedusaTextIndexMimeModule  MedusaTextIndexMimeModule;



/* Return the address of the resulting word array in results */
typedef int (* MedusaTextParsingFunc)  (char *uri,
                                        char ***results,
                                        gpointer data);



MedusaTextIndexMimeModule * medusa_text_index_mime_module_new                   (MedusaTextParsingFunc read_words);
void                        medusa_text_index_mime_module_add_mime_type         (MedusaTextIndexMimeModule *mime_module,
                                                                                 char *mime_type);
void                        medusa_text_index_mime_module_remove_mime_type      (MedusaTextIndexMimeModule *mime_module,
                                                                                 char *mime_type);
/* Mime type patterns can only be mime types
   with * replacing some sequence of normal text within the mime type (including the /) */
void                        medusa_text_index_mime_module_add_mime_pattern      (MedusaTextIndexMimeModule *mime_module,
                                                                                 char *mime_type_pattern);

MedusaTextIndexMimeModule * medusa_text_index_mime_module_first_valid_module   (GList *mime_modules,
                                                                                char *mime_type);
gboolean                    medusa_text_index_mime_module_accepts_mime_type    (MedusaTextIndexMimeModule *mime_module,
                                                                                char *mime_type);

MedusaTextParsingFunc       medusa_text_index_mime_module_get_parser           (MedusaTextIndexMimeModule *mime_module);
                                                                                

void                        medusa_text_index_mime_module_ref                  (MedusaTextIndexMimeModule *mime_module);
void                        medusa_text_index_mime_module_unref                (MedusaTextIndexMimeModule *mime_module);
void                        medusa_text_index_mime_module_unref_cover          (gpointer data,
                                                                                gpointer user_data);


void                        medusa_text_index_mime_module_test                  (void);
#endif /* MEDUSA_TEXT_INDEX_MIME_MODULE_H */
