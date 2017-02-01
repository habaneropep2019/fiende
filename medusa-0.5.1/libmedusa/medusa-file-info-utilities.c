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
 *  medusa-file-index-utilties.c  -- Utility functions to manipulate
 *  dates and user and group information.
 *
 */

#include <glib.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "medusa-file-info-utilities.h"

static gboolean     username_exists                       (const char *username);
static gboolean     group_exists                          (const char *username);

static int          number_of_strsplit_fields_returned    (char **return_value);
static struct tm*   get_time_struct_for_beginning_of_date (const char *date);
static struct tm*   get_time_struct_for_end_of_date       (const char *date);
static gboolean     is_a_leap_year                        (int year);
static void         move_time_struct_a_week_into_the_past (struct tm *time_struct);




static gboolean
username_exists (const char *username) 
{
        const struct passwd *user_info;
        
        user_info = getpwnam (username);
        return user_info != NULL; 
}


static gboolean
group_exists (const char *group) 
{
        const struct group *group_info;
        
        group_info = getgrnam (group);
        return group_info != NULL; 
}
                
gboolean
medusa_username_to_uid (const char *username,
                        uid_t *uid)
{
        struct passwd *user_info;

        if (username_exists (username) == FALSE) {
                return FALSE;
        }
            
        
        user_info = getpwnam (username);
        g_assert (user_info != NULL);
        *uid = user_info->pw_uid;
        
        return TRUE;
}


gboolean
medusa_group_to_gid (const char *group,
                     gid_t *gid)
{
        const struct group *group_info;

        if (group_exists (group) == FALSE) {
                return FALSE;
        }
            
        
        group_info = getgrnam (group);
        g_assert (group_info != NULL);
        *gid = group_info->gr_gid;


        return TRUE;

}


static int
number_of_strsplit_fields_returned (char **return_value)
{
        int i;
        for (i = 0; return_value[i] != NULL; i++);
        return i;
}

static struct tm*
get_time_struct_for_beginning_of_date (const char *date)
{
        char **date_fields;
        int month, day_of_month, year;
        struct tm *time_struct;

        date_fields = g_strsplit (date, "/", 0);
        /* Don't deal with invalid dates */
        if (number_of_strsplit_fields_returned (date_fields) != 3) {
                return NULL;
        }
        month = strtol (date_fields[0], NULL, 10);
        day_of_month = strtol (date_fields[1], NULL, 10);
        year = strtol (date_fields[2], NULL, 10);

        time_struct = g_new0 (struct tm, 1);

        time_struct->tm_sec = 0;
        time_struct->tm_min = 0;
        time_struct->tm_hour = 0;
        time_struct->tm_mday = day_of_month;
        time_struct->tm_mon = month - 1;
        time_struct->tm_year = year - 1900;
        /* FIXME bugzilla.eazel.com 2997:  
           What to do about the tm_isdst (daylight savings time)
           field? */
        return time_struct;
}
        

static struct tm*
get_time_struct_for_end_of_date (const char *date)
{
        char **date_fields;
        int month, day_of_month, year;
        struct tm *time_struct;

        date_fields = g_strsplit (date, "/", 0);
        /* Don't deal with invalid dates */
        if (number_of_strsplit_fields_returned (date_fields) != 3) {
                return NULL;
        }
        month = strtol (date_fields[0], NULL, 10);
        day_of_month = strtol (date_fields[1], NULL, 10);
        year = strtol (date_fields[2], NULL, 10);
        
        time_struct = g_new0 (struct tm, 1);

        time_struct->tm_sec = 59;
        time_struct->tm_min = 59;
        time_struct->tm_hour = 23;
        time_struct->tm_mday = day_of_month;
        time_struct->tm_mon = month - 1;
        time_struct->tm_year = year - 1900;
        /* FIXME bugzilla.eazel.com 2997:  
           What to do about the tm_isdst (daylight savings time)
           field? */
        return time_struct;
}

static gboolean
is_a_leap_year (int year) 
{
        /* I could be anal here, but I think this is enough */
        return ((year % 4) == 0);
}

static void
move_time_struct_a_week_into_the_past (struct tm *time_struct)
{
        if (time_struct->tm_mday > 7) {
                time_struct->tm_mday -= 7;
                return;
        }
        switch (time_struct->tm_mon) {
                /* roll back the year for early January dates */
        case 0:
                time_struct->tm_year--;
                time_struct->tm_mon = 11;
                time_struct->tm_mday = time_struct->tm_mday - 7 + 31;
                break;
                /* 30 days has September, April, June, and November */
        case 3: case 5: case 8: case 10:
                time_struct->tm_mon--;
                time_struct->tm_mday = time_struct->tm_mday - 7 + 30;
                break;
                /* All the rest have 31, except for February */
        case 2: case 4: case 6: case 7: case 9: case 11:
                time_struct->tm_mon--;
                time_struct->tm_mday = time_struct->tm_mday - 7 + 31;
                break;
                /* Except for February, which has 28 (or 29!) */
        case 1:
                time_struct->tm_mon = 0;
                if (is_a_leap_year (time_struct->tm_year)) {
                        time_struct->tm_mday = time_struct->tm_mday - 7 + 29;
                }
                else {
                        time_struct->tm_mday = time_struct->tm_mday - 7 + 28;
                }
                break;
        }
}                     
                
 
static void
move_time_struct_a_week_into_the_future (struct tm *time_struct)
{
        switch (time_struct->tm_mon) {
                /* advance the date for late December dates */
        case 11:
                if (time_struct->tm_mday > 24) {
                        time_struct->tm_year++;
                        time_struct->tm_mon = 0;
                        time_struct->tm_mday = time_struct->tm_mday + 7 - 31;
                }
                else {
                        time_struct->tm_mday += 7;
                }
                break;
                /* 30 days has September, April, June, and November */
        case 3: case 5: case 8: case 10:
                if (time_struct->tm_mday > 23) {
                        time_struct->tm_mon++;
                        time_struct->tm_mday = time_struct->tm_mday + 7 - 30;
                }
                else {
                        time_struct->tm_mday += 7;
                }
                break;
                /* All the rest have 31, except for February */
        case 0: case 2: case 4: case 6: case 7: case 9:
                if (time_struct->tm_mday > 24) {
                        time_struct->tm_mon++;
                        time_struct->tm_mday = time_struct->tm_mday + 7 - 31;
                }
                else {
                        time_struct->tm_mday += 7;
                }
                break;
                /* Except for February, which has 28 (or 29!) */
        case 1:
                if (is_a_leap_year (time_struct->tm_year)) {
                        if (time_struct->tm_mday > 22) {
                                time_struct->tm_mon++;
                                time_struct->tm_mday = time_struct->tm_mday + 7 - 29;
                        }
                        else {
                                time_struct->tm_mday += 7;
                        }
                }
                else {
                        if (time_struct->tm_mday > 21) {
                                time_struct->tm_mon++;
                                time_struct->tm_mday = time_struct->tm_mday + 7 - 28;
                        }
                        else {
                                time_struct->tm_mday += 7;
                        }
                }
                break;
        }
}                     

static void
move_time_struct_a_month_into_the_past (struct tm *time_struct)
{
        if (time_struct->tm_mon == 0) {
                time_struct->tm_year--;
                time_struct->tm_mon = 11;
        }
        else {
                time_struct->tm_mon--;
        }
}


static void
move_time_struct_a_month_into_the_future (struct tm *time_struct)
{
        if (time_struct->tm_mon == 11) {
                time_struct->tm_year++;
                time_struct->tm_mon = 0;
        }
        else {
                time_struct->tm_mon++;
        }
}

time_t       
medusa_file_info_get_first_unix_time_occurring_on_date (const char *date)
{
        struct tm *time_struct;
        time_t numerical_time;

        time_struct = get_time_struct_for_beginning_of_date (date);
        numerical_time = mktime (time_struct);
        g_free (time_struct);

        return numerical_time;
}

time_t       
medusa_file_info_get_last_unix_time_occurring_on_date (const char *date)
{
        struct tm *time_struct;
        time_t numerical_time;

        time_struct = get_time_struct_for_end_of_date (date);
        numerical_time = mktime (time_struct);
        g_free (time_struct);

        return numerical_time;
}

time_t       
medusa_file_info_get_unix_time_a_week_before_date (const char *date)
{
        struct tm *time_struct;
        time_t numerical_time;

        time_struct = get_time_struct_for_beginning_of_date (date);
        move_time_struct_a_week_into_the_past (time_struct);
        numerical_time = mktime (time_struct);

        g_free (time_struct);

        return numerical_time;
}

time_t       
medusa_file_info_get_unix_time_a_week_after_date (const char *date)
{
        struct tm *time_struct;
        time_t numerical_time;
        
        time_struct = get_time_struct_for_beginning_of_date (date);
        move_time_struct_a_week_into_the_future (time_struct);
        numerical_time = mktime (time_struct);

        g_free (time_struct);

        return numerical_time;
}

time_t       
medusa_file_info_get_unix_time_a_month_before_date (const char *date)
{
        struct tm *time_struct;
        time_t numerical_time;
        
        time_struct = get_time_struct_for_beginning_of_date (date);
        move_time_struct_a_month_into_the_past (time_struct);
        numerical_time = mktime (time_struct);

        g_free (time_struct);
        
        return numerical_time;
}

time_t       
medusa_file_info_get_unix_time_a_month_after_date (const char *date)
{
        struct tm *time_struct;
        time_t numerical_time;
        
        time_struct = get_time_struct_for_beginning_of_date (date);
        move_time_struct_a_month_into_the_future (time_struct);
        numerical_time = mktime (time_struct);

        g_free (time_struct);

        return numerical_time;

}

gboolean
medusa_file_is_newer_than_time (const char *file_name,
                                time_t time_to_check)
{
        struct stat stat_buffer;

        g_return_val_if_fail (file_name != NULL, FALSE);
        lstat (file_name, &stat_buffer);

        return stat_buffer.st_mtime > time_to_check;
        
}

gboolean
medusa_group_contains (int gid, int uid)
{
        struct group *group;
        struct passwd *password_entry;
        int i;
        group = getgrgid (gid);
        if (group == NULL) {
                return FALSE;
        }
        for (i = 0; group->gr_mem[i] != NULL; i++) {
                password_entry = getpwnam (group->gr_mem[i]);
                g_return_val_if_fail (password_entry != NULL, FALSE);
                if (password_entry->pw_uid == uid) {
                        return TRUE;
                }
        }
        return FALSE;
  
}

