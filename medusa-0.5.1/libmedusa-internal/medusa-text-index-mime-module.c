/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  medusa-text-index-mime-module.c : Management of 
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

#include "medusa-text-index-mime-module.h"
#include "medusa-test.h"

#include <glib.h>
#include <ctype.h>
#include <string.h>
#include <regex.h>

struct MedusaTextIndexMimeModule {
  /* These lists should be short */
        GList *accepted_mime_types;
        GList *accepted_mime_patterns;
        MedusaTextParsingFunc read_words;
        int ref_count;
} ;

static void             medusa_text_index_mime_module_destroy             (MedusaTextIndexMimeModule *mime_module);

/* Functions to check whether mime types given
   are actually legal mime types to
   avoid mucking up the regexp algorithms */
static gboolean         is_valid_mime_type                           (char *mime_type);
static gboolean         is_valid_mime_pattern                        (char *mime_pattern);

static char *           valid_mime_pattern_to_regexp_string          (char *mime_pattern);

static int              mime_pattern_matches                         (char *mime_pattern,
                                                                      char *string);


static int              dummy_return_words                            (char *location,
                                                                       char ***results,
                                                                       gpointer data);

MedusaTextIndexMimeModule *
medusa_text_index_mime_module_new (MedusaTextParsingFunc read_words)
{
        MedusaTextIndexMimeModule *mime_module;

        mime_module = g_new0 (MedusaTextIndexMimeModule, 1);

        mime_module->accepted_mime_types = NULL;
        mime_module->accepted_mime_patterns = NULL;
        
        mime_module->read_words = read_words;
        
        mime_module->ref_count = 1;

        return mime_module;
}

void
medusa_text_index_mime_module_add_mime_type (MedusaTextIndexMimeModule *mime_module,
                                             char *mime_type)
{
        g_return_if_fail (is_valid_mime_type (mime_type));
        mime_module->accepted_mime_types = g_list_prepend (mime_module->accepted_mime_types,
                                                             g_strdup (mime_type));
}

void  
medusa_text_index_mime_module_remove_mime_type (MedusaTextIndexMimeModule *mime_module,
                                                char *mime_type)
{
        GList *node_to_remove;

        node_to_remove = g_list_find_custom (mime_module->accepted_mime_types,
                                             mime_type,
                                             (GCompareFunc) strcmp);
        mime_module->accepted_mime_types = g_list_remove (mime_module->accepted_mime_types,
                                                         node_to_remove->data);
        g_free (node_to_remove->data);
        /* FIXME bugzilla.eazel.com 2991:  
           Something is not geting freed here */
}


void
medusa_text_index_mime_module_add_mime_pattern (MedusaTextIndexMimeModule *mime_module,
                                                char *mime_pattern)
{
        g_return_if_fail (is_valid_mime_pattern (mime_pattern));
        mime_module->accepted_mime_patterns = g_list_prepend (mime_module->accepted_mime_types,
                                                                g_strdup (mime_pattern));
        
}


MedusaTextIndexMimeModule * 
medusa_text_index_mime_module_first_valid_module   (GList *mime_modules,
                                                    char *mime_type)
{
        for (; mime_modules != NULL; mime_modules = mime_modules->next) {
                if (medusa_text_index_mime_module_accepts_mime_type (mime_modules->data,
                                                                     mime_type)) {
                        return mime_modules->data;
                }
         }
        return NULL;

}

gboolean
medusa_text_index_mime_module_accepts_mime_type (MedusaTextIndexMimeModule *mime_module,
                                                 char *mime_type)
{
        GList *result;

        g_return_val_if_fail (mime_type != NULL, FALSE);
        /* First see if there is an exact mime type match */
        result = g_list_find_custom (mime_module->accepted_mime_types,
                                     mime_type,
                                     (GCompareFunc) strcmp);
        if (result != NULL) {
                return TRUE;
        }

        /* Next see if there is a pattern that matches */
        result = g_list_find_custom (mime_module->accepted_mime_patterns,
                                     mime_type,
                                     (GCompareFunc) mime_pattern_matches);

        if (result != NULL) {
                return TRUE;
        }
        else {
                return FALSE;
        }
}


MedusaTextParsingFunc       
medusa_text_index_mime_module_get_parser (MedusaTextIndexMimeModule *mime_module)
{
        return mime_module->read_words;
}


void
medusa_text_index_mime_module_ref (MedusaTextIndexMimeModule *mime_module)
{
        mime_module->ref_count++;
}

void
medusa_text_index_mime_module_unref (MedusaTextIndexMimeModule *mime_module)
{
        g_assert (mime_module->ref_count > 0);
        mime_module->ref_count--;

        if (mime_module->ref_count == 0) {
                medusa_text_index_mime_module_destroy (mime_module);
        }
}
void                        
medusa_text_index_mime_module_unref_cover (gpointer data,
                                           gpointer user_data)
{
        g_assert (data != NULL);

        medusa_text_index_mime_module_unref (data);
}

static void
medusa_text_index_mime_module_destroy (MedusaTextIndexMimeModule *mime_module)
{
        /* Should this be an assert? */
        g_return_if_fail (mime_module->ref_count == 0);
        /* Free the list data */
        if (mime_module->accepted_mime_types != NULL) {
                g_list_foreach (mime_module->accepted_mime_types,
                                (GFunc) g_free,
                                NULL);
                g_list_free (mime_module->accepted_mime_types);
        }
        if (mime_module->accepted_mime_patterns != NULL) {
                g_list_foreach (mime_module->accepted_mime_patterns,
                                (GFunc) g_free,
                                NULL);
                g_list_free (mime_module->accepted_mime_patterns);
        }
        
        /* Can't free functions, right? so just ignore the function pointer */
        
        g_free (mime_module);

}

static gboolean
is_valid_mime_type (char *mime_type) 
{
        int number_of_slashes_found;

        g_return_val_if_fail (mime_type != NULL, FALSE);

        number_of_slashes_found = 0;
        /* FIXME bugzilla.eazel.com 2641:  
           I am not totally sure this corresponds to
           the mime type spec.  I know it isn't complete */
        while (*mime_type != 0) {
                if (isalpha ((guchar) *mime_type) || isspace ((guchar) *mime_type) || *mime_type == '-') {
                        mime_type++;
                        continue;
                }
                else {
                        if (*mime_type == '/') {
                                number_of_slashes_found++;
                        }
                        else {
                                return FALSE;
                        }
                }
                mime_type++;
        }
        /* FIXME bugzilla.eazel.com 2641:  
           Is 0 a valid number of slashes? */
        if (number_of_slashes_found == 1) {
                return TRUE;
        }
        else {
                return FALSE;
        }
        
}


static gboolean
is_valid_mime_pattern (char *mime_pattern) 
{
        g_return_val_if_fail (mime_pattern != NULL, FALSE);

        /* FIXME bugzilla.eazel.com 2641:  
           I am not totally sure this corresponds to
           the mime pattern spec.  I know it isn't complete */
        while (*mime_pattern != 0) {
                if (isalpha ((guchar) *mime_pattern) || isspace ((guchar) *mime_pattern) || *mime_pattern == '-' || *mime_pattern == '*' ||
                    *mime_pattern == '/') {
                        mime_pattern++;
                        continue;
                }
                else {
                        return FALSE;
                }
        }
        return TRUE;
}

static char *
valid_mime_pattern_to_regexp_string (char *mime_pattern)
{
        char *result, *location;
        

        g_return_val_if_fail (mime_pattern != NULL, FALSE);

        result = g_new0 (char, 2 * strlen (mime_pattern));
        location = result;
        while (*mime_pattern != 0) {
                if (*mime_pattern == '*') {
                        *location = '.';
                        location++;
                        *location = '*';
                        location++;
                }
                else {
                        *location = *mime_pattern;
                        location++;
                }
                mime_pattern++;
        }

        return result;
}

static int         
mime_pattern_matches (char *mime_pattern,
                      char *string)
{
        /* FIXME bugzilla.eazel.com 2642:  
           Keep structs of mime_pattern + compiled regexps instead
         of compiling every time */
        char *mime_regexp;
        regex_t *pattern_data;
        int result;
        
        g_return_val_if_fail (mime_pattern != NULL, 1);
        g_return_val_if_fail (is_valid_mime_pattern (mime_pattern), 1);
        g_return_val_if_fail (string != NULL, FALSE);

        mime_regexp = valid_mime_pattern_to_regexp_string (mime_pattern);

        pattern_data = g_new0 (regex_t, 1);
        g_return_val_if_fail (regcomp (pattern_data, mime_regexp, REG_ICASE | REG_NOSUB) == 0, FALSE);
        g_free (mime_regexp);
        result = regexec (pattern_data, string, 0, NULL, 0);
        regfree (pattern_data); 
        g_free (pattern_data);
        
        return result;

}



static int
dummy_return_words (char *location,
                    char ***results,
                    gpointer data)
{
        *results = g_new0 (char *, 1);
        *results[0] = "dummy";
        return 1;
}

void
medusa_text_index_mime_module_test (void)
{
        MedusaTextIndexMimeModule *mime_module;

        /* Test mime type validity functions */
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_type ("text/plain") ==  TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_type ("x-directory/normal") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_type (NULL) == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_type ("") == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_type ("foo/bar/") == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_type ("foo") == FALSE);

        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("text/plain") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("x-directory/normal") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("foo") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("foo/*") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern (NULL) == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("l444") == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (is_valid_mime_pattern ("[]") == FALSE);


        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches ("*", "foo/bar") == 0);
        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches ("foo", "foo/bar") == 0);
        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches ("foo", "baz/bar") != 0);
        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches ("foo*", "baz/bar") != 0);
        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches (NULL, NULL) != 0);
        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches (NULL, "*") != 0);
        MEDUSA_TEST_BOOLEAN_RESULT (mime_pattern_matches (NULL, "foo") != 0);

        mime_module = medusa_text_index_mime_module_new (dummy_return_words);
        
        medusa_text_index_mime_module_add_mime_type (mime_module, "text/plain");
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "text/plain") == TRUE);
        medusa_text_index_mime_module_remove_mime_type (mime_module, "text/plain");
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "text/plain") == FALSE);

        medusa_text_index_mime_module_add_mime_pattern (mime_module, "x-special/*");
        medusa_text_index_mime_module_add_mime_pattern (mime_module, "x-directory/*");
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "x-directory/normal") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "x-special/foo") == TRUE);
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "foo/bar") == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, NULL) == FALSE);
        
        /* Test invalid mime types */
        medusa_text_index_mime_module_add_mime_type (mime_module,
                                                     "l4444");
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "l4444") == FALSE);
        medusa_text_index_mime_module_add_mime_type (mime_module, "l*");
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "l*") == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "l4444") == FALSE);

        /* Test invalid mime pattern */
        medusa_text_index_mime_module_add_mime_pattern (mime_module,
                                                        "l4444");
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "l*") == FALSE);
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_text_index_mime_module_accepts_mime_type (mime_module, "l4444") == FALSE);

        medusa_text_index_mime_module_unref (mime_module);
}

