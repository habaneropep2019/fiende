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
 *  Author: Darin Adler <darin@eazel.com>
 */

#include <config.h>
#include "medusa-keyword-set.h"

#include <libgnomevfs/gnome-vfs-utils.h>
#include <stdio.h>
#include <string.h>

/* Format of the string is this:
 * "( <public_keyword> <public_keyword> ) <user_id>( <private_keyword> <private_keyword> )".
 */

/* For now, always keep it in string form.
 * We might change this later.
 */
struct MedusaKeywordSet {
        char *string_form;
};

MedusaKeywordSet *
medusa_keyword_set_new (void)
{
        return medusa_keyword_set_new_from_string ("");
}

MedusaKeywordSet *
medusa_keyword_set_new_from_string (const char *string_form)
{
        MedusaKeywordSet *set;

        g_return_val_if_fail (string_form != NULL, NULL);

        set = g_new0 (MedusaKeywordSet, 1);
        set->string_form = g_strdup (string_form);
        return set;
}

void
medusa_keyword_set_destroy (MedusaKeywordSet *set)
{
        g_return_if_fail (set != NULL);

        g_free (set->string_form);
        g_free (set);
}

void
medusa_keyword_set_add_public_keyword (MedusaKeywordSet *set,
                                       const char *keyword)
{
        char *escaped_keyword, *old_string_form;

        g_return_if_fail (set != NULL);
        g_return_if_fail (keyword != NULL);

        /* Just add the escaped keyword to the string.
         * We'll take care of things like duplicates later.
         */
        
        escaped_keyword = gnome_vfs_escape_string (keyword);

        old_string_form = set->string_form;
        set->string_form = g_strconcat (old_string_form,
                                        " ( ", escaped_keyword, " )",
                                        NULL);
        g_free (old_string_form);

        g_free (escaped_keyword);
}

void
medusa_keyword_set_add_user_with_private_keywords (MedusaKeywordSet *set,
                                                   uid_t user)
{
        char *old_string_form;

        g_return_if_fail (set != NULL);
        
        /* Just add the user ID to the string.
         * We'll take care of things like duplicates later.
         */
        
        old_string_form = set->string_form;
        set->string_form = g_strdup_printf ("%s %ld()",
                                            old_string_form,
                                            (long) user);
        g_free (old_string_form);
}

void
medusa_keyword_set_add_private_keyword (MedusaKeywordSet *set,
                                        uid_t user,
                                        const char *keyword)
{
        char *escaped_keyword, *old_string_form;

        g_return_if_fail (set != NULL);
        g_return_if_fail (keyword != NULL);

        /* Just add the escaped keyword to the string.
         * We'll take care of things like duplicates later.
         */
        
        escaped_keyword = gnome_vfs_escape_string (keyword);

        old_string_form = set->string_form;
        set->string_form = g_strdup_printf ("%s %ld( %s )",
                                            old_string_form,
                                            (long) user,
                                            escaped_keyword);
        g_free (old_string_form);

        g_free (escaped_keyword);
}

/* Like strrchr, but starts somewhere other than the end. */
static const char *
find_previous_character (const char *string_start,
                         const char *current_position,
                         char c)
{
        const char *p;

        g_assert (string_start != NULL);
        g_assert (current_position != NULL);
        g_assert (c != 0);

        for (p = current_position; p != string_start; p--) {
                if (*p == c) {
                        return p;
                }
        }
        return NULL;
}

static gboolean
find_previous_user_id (const char *string_start,
                       const char *current_position,
                       long *user_id)
{
        const char *paren, *space;
        char *number_as_string;
        gboolean found;

        g_assert (string_start != NULL);
        g_assert (current_position != NULL);
        g_assert (user_id != NULL);

        paren = find_previous_character (string_start, current_position, '(');
        if (paren == NULL) {
                /* Bad format. */
                return FALSE;
        }
        space = find_previous_character (string_start, paren, ' ');
        if (space == NULL) {
                /* Bad format. */
                return FALSE;
        }
        number_as_string = g_strndup (space + 1, paren - (space + 1));
        found = sscanf ("%ld%*s", number_as_string, user_id) == 1;
        g_free (number_as_string);

        return found;
}

gboolean
medusa_keyword_set_has_keyword (MedusaKeywordSet *set,
                                uid_t user,
                                const char *keyword)
{
        gboolean has_keyword, user_has_private_metafile;
        char *search_user;
        char *escaped_keyword, *search_keyword;
        char *scan_point, *found_keyword;
        long found_user;
        
        g_return_val_if_fail (set != NULL, FALSE);
        g_return_val_if_fail (keyword != NULL, FALSE);

        has_keyword = FALSE;

        /* Check if this user has a private metafile. */
        search_user = g_strdup_printf (" %ld(", (long) user);
        user_has_private_metafile = strstr (set->string_form, search_user) != NULL;
        g_free (search_user);

        /* Convert the keyword into a form we can search for. */
        escaped_keyword = gnome_vfs_escape_string (keyword);
        search_keyword = g_strconcat (" ", escaped_keyword, " ", NULL);
        g_free (escaped_keyword);

        /* For each occurrence of the keyword, check the user. */
        scan_point = set->string_form;
        for (;;) {
                /* Find the next occurence of this keyword. */
                found_keyword = strstr (scan_point, search_keyword);
                if (found_keyword == NULL) {
                        break;
                }

                /* Check for last user ID in the string before this point. */
                if (find_previous_user_id (scan_point, found_keyword, &found_user)) {
                        if (found_user == user) {
                                g_assert (user_has_private_metafile);
                                has_keyword = TRUE;
                                break;
                        }
                } else {
                        if (!user_has_private_metafile) {
                                has_keyword = TRUE;
                                break;
                        }
                }

                /* Move past this keyword on to the next, but be sure
                 * to leave the trailing space so it can be a leading
                 * space for the next one. No need to cache the search
                 * keyword's length because usually we don't loop.
                 */
                scan_point = found_keyword + (strlen (search_keyword) - 1);
        }
        g_free (search_keyword);
        
        return has_keyword;
}

char *
medusa_keyword_set_get_string_form (MedusaKeywordSet *set)
{
        g_return_val_if_fail (set != NULL, NULL);

        /* Should normalize the string here to keep it short. */
        return g_strdup (set->string_form);
}
