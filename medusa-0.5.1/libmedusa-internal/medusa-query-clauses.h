/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

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
 */

/* medusa-query-clauses.h:  System for creating a
   vocabulary for medusa databases (phrases that correspond to
   a function that can find things that satisfy a phrase) */

#ifndef MEDUSA_QUERY_CLAUSE_H
#define MEDUSA_QUERY_CLAUSE_H

#include <sys/types.h>
#include "medusa-rdb-table.h"

typedef struct MedusaQueryClauses MedusaQueryClauses;
typedef struct MedusaQueryClause MedusaQueryClause;


typedef enum {
        MEDUSA_ARGUMENT_TYPE_STRING,
        MEDUSA_ARGUMENT_TYPE_NUMBER,
	MEDUSA_ARGUMENT_TYPE_NONE
} MedusaArgumentType;


typedef union {
	char *string;
	long number;
} MedusaQueryArgument;

typedef struct {
	gpointer database;
	uid_t uid_of_user_making_query;
} MedusaDatabaseAndUIDData;

typedef gboolean (* MedusaQueryFunc)   (gpointer database_data,
					gpointer current_record,
					MedusaQueryArgument argument);

typedef gboolean (* MedusaResultCheckFunc) (const char *uri,
					    MedusaQueryArgument argument);
						 

MedusaQueryClauses *    medusa_query_clauses_new                  (void);
void                    medusa_query_clauses_ref                  (MedusaQueryClauses *clause);
void                    medusa_query_clauses_unref                (MedusaQueryClauses *clause);
void                    medusa_query_clauses_add_clause           (MedusaQueryClauses *clauses,
								   char *field_name,
								   char *operator_name,
								   MedusaQueryFunc evaluate,
								   MedusaArgumentType argument_type);
void                    medusa_query_clauses_add_clause_with_check (MedusaQueryClauses *clauses,
								    const char *field_name,
								    const char *operator_name,
								    MedusaQueryFunc evaluate,
								    MedusaResultCheckFunc verify,
								    MedusaArgumentType argument_type);

void                    medusa_query_clauses_get_function         (MedusaQueryClauses *clauses,
								   const char *clause,
								   MedusaQueryFunc *evaluate,
								   MedusaResultCheckFunc *verify,
								   MedusaArgumentType *argument_type);

#endif /* MEDUSA_QUERY_CLAUSE_H */
