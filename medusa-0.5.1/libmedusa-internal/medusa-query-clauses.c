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

/* medusa-query-clauses.c: Each database registers the queries it knows
   how to perform as a set of query clauses, that are then registered with
   the top level database */

/* FIXME bugzilla.eazel.com 4885: Rename this class to make it more comprehensible, and less confusing
   w.r.t. the clause closures */

#include <ctype.h>
#include <glib.h>
#include <string.h>

#include "medusa-query-clauses.h"




struct MedusaQueryClauses {
  GList *clauses;
  int ref_count;
};


struct MedusaQueryClause {
	char *field_name;
	char *operator_name;
	MedusaQueryFunc evaluate;
	MedusaArgumentType argument_type;	
	/* Function that verifies that the real file 
	   still matches the criteria.  For now we assume it
	takes the same type as the evaluate function*/
	MedusaResultCheckFunc verify;
	int ref_count;
};


/* FIXME bugzilla.eazel.com 4886: Get rid of ref counting here; it is unnecessary */
static void                    medusa_query_clauses_destroy             (MedusaQueryClauses *clause);

static MedusaQueryClause *     medusa_query_clause_new                  (const char *field_name,
									 const char *operator_name,
									 MedusaQueryFunc evaluate,
									 MedusaArgumentType argument_type);
static MedusaQueryClause *     medusa_query_clause_new_with_check       (const char *field_name,
									 const char *operator_name,
									 MedusaQueryFunc evaluate,
									 MedusaResultCheckFunc verify,
									 MedusaArgumentType argument_type);
/*static void                    medusa_query_clause_ref                  (MedusaQueryClause *clause); */
static void                    medusa_query_clause_unref                (MedusaQueryClause *clause);
static void                    medusa_query_clause_destroy              (MedusaQueryClause *clause);

MedusaQueryClauses *
medusa_query_clauses_new (void)
{
	MedusaQueryClauses *clauses;
	
	clauses = g_new0 (MedusaQueryClauses, 1);
	clauses->ref_count = 1;
	
	return clauses;
}

void
medusa_query_clauses_add_clause_with_check (MedusaQueryClauses *clauses,
					    const char *field_name,
					    const char *operator_name,
					    MedusaQueryFunc evaluate,
					    MedusaResultCheckFunc verify,
					    MedusaArgumentType argument_type)
{
	MedusaQueryClause *new_clause;

	new_clause = medusa_query_clause_new_with_check (field_name, 
							 operator_name, 
							 evaluate, 
							 verify,
							 argument_type);
	clauses->clauses = g_list_prepend (clauses->clauses, new_clause);
}
					    

void
medusa_query_clauses_add_clause (MedusaQueryClauses *clauses,
				 char *field_name,
				 char *operator_name,
				 MedusaQueryFunc evaluate,
				 MedusaArgumentType argument_type)
{
	MedusaQueryClause *new_clause;
	
	new_clause = medusa_query_clause_new (field_name, operator_name, evaluate, argument_type);
	clauses->clauses = g_list_prepend (clauses->clauses, new_clause);
}

static gboolean
value_has_correct_type (MedusaArgumentType argument_type,
			const char *value_to_check)
{
	const char *arg;

	switch (argument_type) {
	case MEDUSA_ARGUMENT_TYPE_NONE:
		return value_to_check == NULL ||
			strlen (value_to_check) == 0;
	case MEDUSA_ARGUMENT_TYPE_NUMBER:
		for (arg = value_to_check; *arg != 0; arg++) {
			/* We could use a better check than this */
			if (!isdigit ((int)*arg)) {
				g_print ("Value doesn't have type integer.  returning false\n");
				return FALSE;
			}
		}
		return TRUE;
	case MEDUSA_ARGUMENT_TYPE_STRING:
		return TRUE;
	default:
		g_assert_not_reached ();
		return FALSE;
	}
		
}

static MedusaQueryClause *
match_search_clause_to_query_clause (MedusaQueryClauses *clauses,
				     const char *clause)
{
	char **words, *field_name, *operator_name, *value;
	GList *i;
	MedusaQueryClause *query_clause;
	
	words = g_strsplit (clause, " ", 3);
	field_name = words[0];
	operator_name = words[1];
	value = words[2];

	for (i = clauses->clauses; i != NULL; i = i->next) {
		query_clause = (MedusaQueryClause *) i->data;
		if (strcmp (query_clause->field_name, field_name) == 0 &&
		    strcmp (query_clause->operator_name, operator_name) == 0 &&
		    value_has_correct_type (query_clause->argument_type, value)) {
			g_strfreev (words);
			return i->data;
		}
	}
	g_strfreev (words);
	return NULL;
}

void	
medusa_query_clauses_get_function (MedusaQueryClauses *clauses,
				   const char *clause,
				   MedusaQueryFunc *evaluate,
				   MedusaResultCheckFunc *verify,
				   MedusaArgumentType *argument_type)
{
	MedusaQueryClause *matching_function;

	matching_function = match_search_clause_to_query_clause (clauses,
								 clause);

	if (matching_function == NULL) {
		*evaluate = NULL;
		*verify = NULL;
		return;
	}
	else {
		*evaluate = matching_function->evaluate;
		*verify = matching_function->verify;
		*argument_type = matching_function->argument_type;
	}

}

void
medusa_query_clauses_ref (MedusaQueryClauses *clauses)
{
	g_assert (clauses->ref_count > 0);
	clauses->ref_count++;
}

void
medusa_query_clauses_unref (MedusaQueryClauses *clauses)
{
	if (clauses == NULL) {
		return;
	}
	g_assert (clauses->ref_count > 0);
	if (clauses->ref_count == 1) {
		medusa_query_clauses_destroy (clauses);
	} else {
		clauses->ref_count--;
	}
}




static void
medusa_query_clauses_destroy (MedusaQueryClauses *clauses)
{
	GList *p;

	for (p = clauses->clauses; p != NULL; p = p->next) {
		medusa_query_clause_unref (p->data);
	}
	g_list_free (clauses->clauses);
	g_free (clauses);
}

static MedusaQueryClause *     
medusa_query_clause_new (const char *field_name, 
			 const char *operator_name, 
			 MedusaQueryFunc evaluate,
			 MedusaArgumentType argument_type)
{
	MedusaQueryClause *clause;
	
	clause = g_new0 (MedusaQueryClause, 1);
	clause->field_name = g_strdup (field_name);
	clause->operator_name = g_strdup (operator_name);
	clause->evaluate = evaluate;
	clause->argument_type = argument_type;
	
	clause->ref_count = 1;
	return clause;
}

static MedusaQueryClause *
medusa_query_clause_new_with_check (const char *field_name,
				    const char *operator_name,
				    MedusaQueryFunc evaluate,
				    MedusaResultCheckFunc verify,
				    MedusaArgumentType argument_type)
{
	MedusaQueryClause *clause;

	clause = medusa_query_clause_new (field_name,
					  operator_name,
					  evaluate,
					  argument_type);
	clause->verify = verify;

	return clause;
}

/*
  static void
  medusa_query_clause_ref (MedusaQueryClause *clause)
  {
  g_assert (clause->ref_count > 0);
  clause->ref_count++;
  }
*/

static void
medusa_query_clause_unref (MedusaQueryClause *clause)
{
	g_assert (clause->ref_count > 0);
	if (clause->ref_count == 1) {
		medusa_query_clause_destroy (clause);
	} else {
		clause->ref_count--;
	}
}

static void
medusa_query_clause_destroy (MedusaQueryClause *clause)
{
	g_assert (clause->ref_count == 1);
	
	g_free (clause->field_name);
	g_free (clause->operator_name);
	g_free (clause);
}
