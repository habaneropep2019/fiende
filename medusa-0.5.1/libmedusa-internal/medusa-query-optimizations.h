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
 *  medusa-query-optimization.h -- Functions to replace query criteria with
 *  equivalent criteria that will take less time to run, or reorder the
 *  criteria
 */

#ifndef MEDUSA_QUERY_OPTIMIZATION_H
#define MEDUSA_QUERY_OPTIMIZATION_H

#include <glib.h>
#include "medusa-search-uri.h"
typedef struct MedusaQueryOptimizationList MedusaQueryOptimizationList;
typedef struct MedusaOptimizationResult MedusaOptimizationResult;
typedef char * MedusaUnparsedCriteria;

typedef enum {
        MEDUSA_OPTIMIZATION_RUN_FIRST,
        MEDUSA_OPTIMIZATION_RUN_REGULAR,
        MEDUSA_OPTIMIZATION_RUN_LAST
} MedusaOptimizationPriority;


struct MedusaOptimizationResult {
        MedusaSearchURIError error;
        MedusaUnparsedCriteria *criteria;
};


/* Performs the optimizations in the optimizer, and
   if necessary perform the optimizations.
   Free the old list of criteria and
   return a new one */
MedusaQueryOptimizationList *            medusa_query_optimizations_initialize            (void);
MedusaOptimizationResult *               medusa_query_optimizations_perform_and_free_deep (MedusaQueryOptimizationList *optimizations,
                                                                                           MedusaUnparsedCriteria *criteria);

void                                     medusa_query_optimizations_shutdown              (MedusaQueryOptimizationList *optimizations);
void                                     medusa_query_optimizations_destroy               (MedusaQueryOptimizationList *optimization);

void                                     medusa_optimization_result_destroy               (MedusaOptimizationResult *optimization_result);

									    



#endif /* MEDUSA_QUERY_OPTIMIZATION_H */
