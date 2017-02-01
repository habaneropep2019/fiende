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
 *  medusa-string.h  -- String functions written or copied
 *  for use in medusa
 *
 */

#ifndef MEDUSA_STRING_H
#define MEDUSA_STRING_H

#include <glib.h>

gboolean       medusa_str_has_prefix                             (const char                     *string,
                                                                  const char                     *prefix);
gboolean       medusa_str_has_suffix                             (const char                     *string,
                                                                  const char                     *suffix);
/* Same as above, but case insensitive */
gboolean       medusa_strcase_has_prefix                         (const char                     *string,
								  const char                     *prefix);

gboolean       medusa_strcase_has_suffix                         (const char                     *string,
								  const char                     *suffix);

gboolean       medusa_strstr_case_insensitive                    (const char                     *string,
								  const char                     *substring);

#endif /* MEDUSA_STRING_H */
