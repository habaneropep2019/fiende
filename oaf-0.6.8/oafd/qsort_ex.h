/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* qsrt_ex.h: version of qsort() that allows a user data arg to the
   compare function.
 
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
  
   Author: Maciej Stachowiak <mjs@eazel.com> */

#ifndef QSORT_EX_H
#define QSORT_EX_H

typedef int (*compar_ex_fn_t) (const void *a, const void *b,
			       const void *user_data);

extern void qsort_ex (void *const pbase, size_t total_elems, size_t size,
		      compar_ex_fn_t cmp, void *user_data);

#endif
