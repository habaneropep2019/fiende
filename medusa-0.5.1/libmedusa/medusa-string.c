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
 *  medusa-string.c -- String functions written or copied
 *  for use in medusa
 *
 */

#include <glib.h>
#include <string.h>

#include "medusa-string.h"

gboolean
medusa_str_has_prefix (const char *string,
                       const char *prefix)
{
        g_return_val_if_fail (string != NULL, FALSE);
        g_return_val_if_fail (prefix != NULL, FALSE);

        return strlen (prefix) <= strlen (string) &&
                strncmp (string, prefix, strlen (prefix)) == 0;
}


gboolean
medusa_str_has_suffix (const char *string,
                       const char *suffix)
{
        g_return_val_if_fail (string != NULL, FALSE);
        g_return_val_if_fail (suffix != NULL, FALSE);

        return strlen (suffix) <= strlen (string) &&
                strcmp (string + strlen (string) - strlen (suffix), 
                        suffix) == 0;
}

gboolean       
medusa_strcase_has_prefix (const char                     *string,
			   const char                     *prefix)
{
        g_return_val_if_fail (string != NULL, FALSE);
        g_return_val_if_fail (prefix != NULL, FALSE);

        return strlen (prefix) <= strlen (string) &&
                strncasecmp (string, prefix, strlen (prefix)) == 0;
}

gboolean
medusa_strcase_has_suffix (const char *string,
			   const char *suffix)
{
        g_return_val_if_fail (string != NULL, FALSE);
        g_return_val_if_fail (suffix != NULL, FALSE);
  
        return strlen (suffix) <= strlen (string) &&
                strcasecmp (string + strlen (string) - strlen (suffix), 
                            suffix) == 0;
}



gboolean       
medusa_strstr_case_insensitive  (const char                     *string,
                                 const char                     *substring)
{
        const char *string_location;
        int possible_prefix_locations;

        g_return_val_if_fail (string != NULL, FALSE);
        g_return_val_if_fail (substring != NULL, FALSE);
        
        if (strlen (substring) <= strlen (string)) {
                possible_prefix_locations = strlen (string) - strlen (substring);
                for (string_location = string;
                     possible_prefix_locations >= 0;
                     possible_prefix_locations--,
                             string_location++) {
                        if (medusa_strcase_has_prefix (string_location,
                                                       substring)) {
                                return TRUE;
                        }
                }
        }

        return FALSE;
}
