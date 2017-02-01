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

#ifndef MEDUSA_QUERY_OPTIMIZATION_PRIVATE_H
#define MEDUSA_QUERY_OPTIMIZATION_PRIVATE_H

#include "medusa-query-optimizations.h"



typedef gboolean                       (*MedusaOptimizationCheckFunc)    (MedusaUnparsedCriteria *criteria);
typedef MedusaOptimizationResult *     (*MedusaOptimizationFunc)         (MedusaUnparsedCriteria *criteria);


MedusaQueryOptimizationList *   medusa_query_optimizations_new                    (void);

void                            medusa_query_optimizations_add                    (MedusaQueryOptimizationList *optimizations,
										   MedusaOptimizationCheckFunc optimization_is_valid,
										   MedusaOptimizationFunc optimize,
										   MedusaOptimizationPriority priority);
#endif /* MEDUSA_QUERY_OPTIMIZATIONS_PRIVATE_H */
