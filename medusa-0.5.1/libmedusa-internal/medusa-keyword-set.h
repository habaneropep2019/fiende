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

#ifndef MEDUSA_KEYWORD_SET_H
#define MEDUSA_KEYWORD_SET_H

#include <glib.h>
#include <sys/types.h>

typedef struct MedusaKeywordSet MedusaKeywordSet;

MedusaKeywordSet *medusa_keyword_set_new                            (void);
MedusaKeywordSet *medusa_keyword_set_new_from_string                (const char       *string_form);
void              medusa_keyword_set_destroy                        (MedusaKeywordSet *set);
void              medusa_keyword_set_add_public_keyword             (MedusaKeywordSet *set,
								     const char       *keyword);
void              medusa_keyword_set_add_user_with_private_keywords (MedusaKeywordSet *set,
								     uid_t             user);
void              medusa_keyword_set_add_private_keyword            (MedusaKeywordSet *set,
								     uid_t             user,
								     const char       *keyword);
gboolean          medusa_keyword_set_has_keyword                    (MedusaKeywordSet *set,
								     uid_t             user,
								     const char       *keyword);
char *            medusa_keyword_set_get_string_form                (MedusaKeywordSet *set);

#endif /* MEDUSA_KEYWORD_SET_H */
