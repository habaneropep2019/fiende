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
 *  medusa-test.c: Runs all of the class test functions
 *
 */

#include <medusa-rdb-table.h>
#include <medusa-test-conf.h>
#include <medusa-test.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>




void 
medusa_run_integer_check (char *file_name, int line, int test_value, int expected_result)
{
  if (test_value != expected_result) {
    printf ("Test failed in file %s at line %d\n", file_name, line);
    printf ("Expected value was %d\n", expected_result);
    printf ("Instead received %d\n", test_value);
    exit (1);
  }

}

void 
medusa_run_string_check (char *file_name, int line, char *test_value, char *expected_result)
{
  if (expected_result == NULL && test_value == NULL) {
    printf ("Warning: NULL expected and test_value for string compare in file %s at line %d\n",
	    file_name, line);
    return;
  }
  if (test_value == NULL || expected_result == NULL || 
      strcmp (test_value, expected_result) != 0) {
    printf ("Test failed in file %s at line %d\n", file_name, line);
    printf ("Expected value was %s\n", expected_result);
    printf ("Instead received %s\n", test_value);
    exit (1);
  }

}


void 
medusa_run_nbyte_string_check (char *file_name, int line, char *test_value, char *expected_result, int compare_length)
{
  if (expected_result == NULL && test_value == NULL) {
    printf ("Warning: NULL expected and test_value for string compare in file %s at line %d\n",
	    file_name, line);
    return;
  }
  if (test_value == NULL || expected_result == NULL || 
      strncmp (test_value, expected_result, compare_length) != 0) {
    printf ("Test failed in file %s at line %d\n", file_name, line);
    printf ("Expected value was %s\n", expected_result);
    printf ("Instead received %s\n", test_value);
    exit (1);
  }

}


void 
medusa_run_system_call_check (char *file_name, int line, int test_value)
{
  if (test_value != -0) {
    printf ("Test failed in file %s at line %d\n", file_name, line);
    if (test_value == -1) {
      printf ("Received -1 return on a system call\n");
      printf ("Errno is %d\n", errno);
    }
    else {
      printf ("Non zero value returned on a system call\n");
    }
    exit (1);
  }

  
}

void                  
medusa_run_pointer_check (char *file_name, int line, gpointer test_value, gpointer expected_result)
{
  if (test_value != expected_result) {
    printf ("Test failed in file %s at line %d\n", file_name, line);
    printf ("Expected value was 0x%p\n", expected_result);
    printf ("Test returned 0x%p\n", test_value);
    exit (1);
  }

}

void 
medusa_run_boolean_check (char *file_name, int line, gboolean test_value)
{
  if (test_value != TRUE) {
    printf ("Test failed in file %s at line %d\n", file_name, line);
    printf ("Expected value was TRUE, test returned FALSE\n");
    exit (1);
  }

}








