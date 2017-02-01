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
 *  General Public License for more details
 *
 *  You should have received a copy of the GNU Gene ral Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 */

#ifndef MEDUSA_TEST_H
#define MEDUSA_TEST_H

#define MEDUSA_TEST_STRING_RESULT(test_value, expected_result) \
        medusa_run_string_check (__FILE__, __LINE__, test_value, expected_result)
#define MEDUSA_TEST_NBYTE_STRING_RESULT(test_value, expected_result, compare_length) \
        medusa_run_nbyte_string_check (__FILE__, __LINE__, test_value, expected_result, compare_length)
#define MEDUSA_TEST_INTEGER_RESULT(test_value, expected_result) \
        medusa_run_integer_check (__FILE__, __LINE__, test_value, expected_result)
/* Tests system calls.  Fine if value returned is 0.  If -1, prints errno. */
#define MEDUSA_TEST_SYSTEM_CALL(test_call) \
        medusa_run_system_call_check (__FILE__, __LINE__, test_call)
#define MEDUSA_TEST_POINTER_VALUE(test_value, expected_result) \
        medusa_run_pointer_check (__FILE__, __LINE__, test_value, expected_result)
/* Assumes the result shold be TRUE */
#define MEDUSA_TEST_BOOLEAN_RESULT(test_value) \
        medusa_run_boolean_check (__FILE__, __LINE__, test_value)


void                  medusa_run_integer_check             (char *file_name, 
							    int line, 
							    int test_value, 
							    int expected_result);
void                  medusa_run_string_check              (char *file_name, 
							    int line, 
							    char *test_value, 
							    char *expected_result);
void                  medusa_run_nbyte_string_check        (char *file_name, 
							    int line, 
							    char *test_value, 
							    char *expected_result, 
							    int compare_length);
void                  medusa_run_system_call_check         (char *file_name, 
							    int line, 
							    int test_value);
void                  medusa_run_pointer_check             (char *file_name, 
							    int line, 
							    gpointer test_value,
							    gpointer expected_result);
void                  medusa_run_boolean_check             (char *file_name, 
							    int line, 
							    gboolean test_value);


#endif
