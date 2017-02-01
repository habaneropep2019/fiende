/* bug-buddy bug submitting program
 *
 * Copyright (C) Jacob Berkman
 *
 * Author:  Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <config.h>

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <glib.h>
#include <gtk/gtkeditable.h>

void destroy_hash_table (GHashTable *table, gboolean free_data);

pid_t start_commandv (const char *args[], int *fd);
pid_t start_command (const char *command, int *fd);

char *get_line_from_fd (int fd);
char *get_line_from_ioc (GIOChannel *ioc);
char *get_line_from_file (const char *filename);
char *get_line_from_command (const char *command);
char *get_line_from_commandv (const char *argv[]);

char *format_for_width (const char *s);

#endif /* __UTIL_H__ */
