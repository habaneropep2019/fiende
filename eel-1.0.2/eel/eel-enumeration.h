/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-enumeration.h: Enumeration data structure.
 
   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_ENUMERATION_H
#define EEL_ENUMERATION_H

#include <eel/eel-string-list.h>

/* Opaque EelEnumeration declaration. */
typedef struct EelEnumeration EelEnumeration;

typedef struct
{
	const char *name;
	const char *description;
	int value;
} EelEnumerationEntry;

typedef struct
{
	const char *id;
	const EelEnumerationEntry *entries;
} EelEnumerationInfo;

EelEnumeration *eel_enumeration_new                               (const char                *id);
EelEnumeration *eel_enumeration_copy                              (const EelEnumeration      *enumeration);
void            eel_enumeration_free                              (EelEnumeration            *enumeration);
void            eel_enumeration_insert                            (EelEnumeration            *enumeration,
								   const char                *name,
								   const char                *description,
								   int                        value);
void            eel_enumeration_insert_entries                    (EelEnumeration            *enumeration,
								   const EelEnumerationEntry  entries[]);
char *          eel_enumeration_get_id                            (const EelEnumeration      *enumeration);
char *          eel_enumeration_get_nth_name                      (const EelEnumeration      *enumeration,
								   guint                      n);
char *          eel_enumeration_get_nth_description               (const EelEnumeration      *enumeration,
								   guint                      n);
char *          eel_enumeration_get_nth_description_translated    (const EelEnumeration      *enumeration,
								   guint                      n);
int             eel_enumeration_get_nth_value                     (const EelEnumeration      *enumeration,
								   guint                      n);
guint           eel_enumeration_get_length                        (const EelEnumeration      *enumeration);
EelEnumeration *eel_enumeration_new_from_tokens                   (const char                *id,
								   const char                *entries,
								   const char                *descriptions,
								   const char                *values,
								   const char                *delimiter);
int             eel_enumeration_get_name_position                 (const EelEnumeration      *enumeration,
								   const char                *name);
int             eel_enumeration_get_description_position          (const EelEnumeration      *enumeration,
								   const char                *description);
int             eel_enumeration_get_value_position                (const EelEnumeration      *enumeration,
								   int                        value);
gboolean        eel_enumeration_contains_name                     (const EelEnumeration      *enumeration,
								   const char                *name);
EelStringList * eel_enumeration_get_names                         (const EelEnumeration      *enumeration);
void            eel_enumeration_register                          (const EelEnumerationInfo   info_array[]);
EelEnumeration *eel_enumeration_lookup                            (const char                *id);
char *          eel_enumeration_id_get_nth_name                   (const char                *id,
								   guint                      n);
char *          eel_enumeration_id_get_nth_description            (const char                *id,
								   guint                      n);
char *          eel_enumeration_id_get_nth_description_translated (const char                *id,
								   guint                      n);
int             eel_enumeration_id_get_nth_value                  (const char                *id,
								   guint                      n);
guint           eel_enumeration_id_get_length                     (const char                *id);
int             eel_enumeration_id_get_name_position              (const char                *id,
								   const char                *name);
int             eel_enumeration_id_get_description_position       (const char                *id,
								   const char                *description);
int             eel_enumeration_id_get_value_position             (const char                *id,
								   int                        value);
gboolean        eel_enumeration_id_contains_name                  (const char                *id,
								   const char                *name);

#endif /* EEL_ENUMERATION_H */

