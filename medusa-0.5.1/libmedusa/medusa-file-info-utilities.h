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
 *  medusa-file-info-utilties.h  -- Utility functions to manipulate
 *  dates and user and group information.
 *
 */

#ifndef MEDUSA_FILE_INFO_UTILITIES_H
#define MEDUSA_FILE_INFO_UTILITIES_H

#include <glib.h>
#include <time.h>

/* FIXME bugzilla.eazel.com 2998: 
   These should not have the "file_info" name tagged on */
time_t       medusa_file_info_get_first_unix_time_occurring_on_date   (const char *date);
time_t       medusa_file_info_get_last_unix_time_occurring_on_date    (const char *date);

time_t       medusa_file_info_get_unix_time_a_week_before_date   (const char *date);
time_t       medusa_file_info_get_unix_time_a_week_after_date    (const char *date);

time_t       medusa_file_info_get_unix_time_a_month_before_date  (const char *date);
time_t       medusa_file_info_get_unix_time_a_month_after_date   (const char *date);

/* These functions return a boolean which determines whether a 
   uid or gid exists.  If the uid or gid exists, it is stored
   in the parameter field */
gboolean     medusa_username_to_uid                             (const char *username,
                                                                 uid_t *uid);
gboolean     medusa_group_to_gid                                (const char *group,
                                                                 gid_t *gid);

gboolean     medusa_file_is_newer_than_time                     (const char *filename,
                                                                 time_t time_to_check);
/* Returns TRUE if a group gid contains the user with user id uid, FALSE otherwise */
gboolean       medusa_group_contains                             (int                             gid,
                                                                  int                             uid);
#endif /* MEDUSA_FILE_INFO_UTILITIES_H */
