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
 *  Author: Maciej Stachowiak <mjs@eazel.com>
 *
 */


/*  medusa-lock.h -  Utility functions for managing file locking
 */



#include <glib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#include "medusa-lock.h"
#include "medusa-lock-file-paths.h"

#define READ_LOCK_SUFFIX "read_lock"
#define WRITE_LOCK_SUFFIX "write_lock"

typedef struct MedusaLock MedusaLock;

struct MedusaLock {
        int fd;
        char *file_name;
};

struct MedusaReadLock {
        int fd;
        char *file_name;
};

struct MedusaWriteLock {
        int fd;
        char *file_name;
};

typedef enum {
        LOCK_TYPE_READ,
        LOCK_TYPE_WRITE
} LockType;

static MedusaLock *medusa_lock_get     (const char *file_name, 
                                        gboolean    write);
static MedusaLock *medusa_lock_get_with_timeout (const char *file_name,
                                                 gboolean write,
                                                 int timeout_in_milliseconds);
static void        medusa_lock_release (MedusaLock *lock);
static char *      get_lock_file_name  (const char *index_name,
                                        LockType lock_type);

static char *
get_lock_file_name (const char *index_name,
                    LockType lock_type)
{
        switch (lock_type) {
        case LOCK_TYPE_READ:
                return g_strdup_printf ("%s.%s", MEDUSA_FILE_INDEX_READ_LOCK_FILE, index_name);
        case LOCK_TYPE_WRITE:
                return g_strdup_printf ("%s.%s", MEDUSA_FILE_INDEX_WRITE_LOCK_FILE, index_name);
        default:
                g_assert_not_reached ();
                return NULL;
        }

}

static char *
get_unprivileged_lock_file_name (const char *path_name,
                                 LockType lock_type)
{
        switch (lock_type) {
        case LOCK_TYPE_READ:
                return g_strdup_printf ("%s.%s", path_name, READ_LOCK_SUFFIX);
        case LOCK_TYPE_WRITE:
                return g_strdup_printf ("%s.%s", path_name, WRITE_LOCK_SUFFIX);
        default:
                g_assert_not_reached ();
                return NULL;
        }

}

MedusaReadLock *
medusa_read_lock_get (const char * index_name)
{
        char *file_name;
        MedusaReadLock *read_lock;

        file_name = get_lock_file_name (index_name,
                                        LOCK_TYPE_READ);
        read_lock = (MedusaReadLock *) medusa_lock_get (file_name, FALSE);
        g_free (file_name);

        return read_lock;
}

MedusaReadLock *
medusa_read_lock_unprivileged_get_with_timeout (const char *index_name,
                                                int timeout_in_milliseconds)
{
        char *file_name;
        MedusaReadLock *read_lock;
        
        file_name = get_unprivileged_lock_file_name (index_name,
                                                     LOCK_TYPE_READ);
        read_lock = (MedusaReadLock *) medusa_lock_get_with_timeout (file_name, FALSE,
                                                                     timeout_in_milliseconds);
        g_free (file_name);

        return read_lock;
                
}

void
medusa_read_lock_release (MedusaReadLock *lock)
{
        medusa_lock_release ((MedusaLock *) lock);
}

MedusaWriteLock *
medusa_write_lock_get (const char *index_name)
{
        char *file_name;
        MedusaWriteLock *write_lock;

        file_name = get_lock_file_name (index_name,
                                        LOCK_TYPE_WRITE);

        write_lock = (MedusaWriteLock *) medusa_lock_get (file_name, TRUE);
        g_free (file_name);

        return write_lock;
        
}

MedusaWriteLock *
medusa_write_lock_unprivileged_get_with_timeout (const char *path_name,
                                                 int timeout_in_milliseconds)
{
        char *file_name;
        MedusaWriteLock *write_lock;
        
        file_name = get_unprivileged_lock_file_name (path_name,
                                                     LOCK_TYPE_WRITE);

        write_lock = (MedusaWriteLock *) medusa_lock_get_with_timeout (file_name, TRUE,
                                                                       timeout_in_milliseconds);
        g_free (file_name);

        return write_lock;
        
}


void
medusa_write_lock_release (MedusaWriteLock *lock)
{
        medusa_lock_release ((MedusaLock *) lock);
}



static MedusaLock * 
medusa_lock_get (const char *file_name, 
                 gboolean write)
{
        struct flock lock_info;
        MedusaLock *lock;

        lock = g_new0 (MedusaLock, 1);

        lock->fd = open (file_name, O_CREAT | (write ? O_RDWR : O_RDONLY), 
                         S_IRUSR | S_IWUSR);
        
        if (lock->fd == -1) {
                g_free (lock);
                return NULL;
        }

        lock_info.l_type = write ? F_WRLCK : F_RDLCK;
        lock_info.l_start = 0;
        lock_info.l_whence = SEEK_SET;
        lock_info.l_len = 0;
        lock_info.l_pid = 0;

        while (fcntl (lock->fd, F_SETLK, &lock_info) == -1) {
                if (errno != EINTR) {
                        close (lock->fd);
                        g_free (lock);
                        return NULL;
                }
        }
        
        lock->file_name = g_strdup (file_name);

        return lock;

}

static MedusaLock *
medusa_lock_get_with_timeout (const char *file_name,
                              gboolean write,
                              int timeout_in_milliseconds)
{
        int milliseconds_waited;
        struct timeval wait_between_lock_checks;
        MedusaLock *lock;
        
        lock = medusa_lock_get (file_name, write);
        milliseconds_waited = 0;

        while (lock == NULL &&
               (milliseconds_waited < timeout_in_milliseconds || 
                timeout_in_milliseconds == 0)) {

                wait_between_lock_checks.tv_sec = 0;
                wait_between_lock_checks.tv_usec = MIN (1000, 
                                                        timeout_in_milliseconds == 0 ?
                                                        G_MAXINT : milliseconds_waited - timeout_in_milliseconds);
                
                select (1, NULL, NULL, NULL, &wait_between_lock_checks);
                milliseconds_waited += -1 * wait_between_lock_checks.tv_usec / 1000 ;
                /* (Remainders are truncated in integer division) */
                if (wait_between_lock_checks.tv_usec % 1000) {
                        milliseconds_waited++;
                }

                lock = medusa_lock_get (file_name, write);
        }
        
        return lock;
        
}

void
medusa_lock_release (MedusaLock *lock)
{
        struct flock lock_info;

        lock_info.l_type = F_UNLCK;
        lock_info.l_start = 0;
        lock_info.l_whence = SEEK_SET;
        lock_info.l_len = 0;
        lock_info.l_pid = 0;

        fcntl (lock->fd, F_SETLK, &lock_info);
        close (lock->fd);
        unlink (lock->file_name);
        g_free (lock->file_name);
        g_free (lock);
}









