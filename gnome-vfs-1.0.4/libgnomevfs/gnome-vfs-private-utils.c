/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-utils.c - Private utility functions for the GNOME Virtual
   File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it>

   `gnome_vfs_canonicalize_pathname()' derived from code by Brian Fox and Chet
   Ramey in GNU Bash, the Bourne Again SHell.  Copyright (C) 1987, 1988, 1989,
   1990, 1991, 1992 Free Software Foundation, Inc.  */

#include <config.h>
#include "gnome-vfs-private-utils.h"

#include "gnome-vfs-cancellation.h"
#include "gnome-vfs-ops.h"
#include "gnome-vfs-uri.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int
find_next_slash (const char *path, int current_offset)
{
	const char *match;
	
	g_assert (current_offset <= strlen (path));
	
	match = strchr (path + current_offset, GNOME_VFS_URI_PATH_CHR);
	return match == NULL ? -1 : match - path;
}

static int
find_slash_before_offset (const char *path, int to)
{
	int result;
	int next_offset;

	result = -1;
	next_offset = 0;
	for (;;) {
		next_offset = find_next_slash (path, next_offset);
		if (next_offset < 0 || next_offset >= to) {
			break;
		}
		result = next_offset;
		next_offset++;
	}
	return result;
}

static void
collapse_slash_runs (char *path, int from_offset)
{
	int i;
	/* Collapse multiple `/'s in a row. */
	for (i = from_offset;; i++) {
		if (path[i] != GNOME_VFS_URI_PATH_CHR) {
			break;
		}
	}

	if (from_offset < i) {
		strcpy (path + from_offset, path + i);
		i = from_offset + 1;
	}
}

/* Canonicalize path, and return a new path.  Do everything in situ.  The new
   path differs from path in:

     Multiple `/'s are collapsed to a single `/'.
     Leading `./'s and trailing `/.'s are removed.
     Non-leading `../'s and trailing `..'s are handled by removing
     portions of the path.  */
gchar *
gnome_vfs_canonicalize_pathname (gchar *path)
{
	int i, marker;

	if (path == NULL || strlen (path) == 0) {
		return "";
	}

	/* Walk along path looking for things to compact. */
	for (i = 0, marker = 0;;) {
		if (!path[i])
			break;

		/* Check for `../', `./' or trailing `.' by itself. */
		if (path[i] == '.') {
			/* Handle trailing `.' by itself. */
			if (path[i + 1] == '\0') {
				if (i > 1 && path[i - 1] == GNOME_VFS_URI_PATH_CHR) {
					/* strip the trailing /. */
					path[i - 1] = '\0';
				} else {
					/* convert path "/." to "/" */
					path[i] = '\0';
				}
				break;
			}

			/* Handle `./'. */
			if (path[i + 1] == GNOME_VFS_URI_PATH_CHR) {
				strcpy (path + i, path + i + 2);
				if (i == 0) {
					/* don't leave leading '/' for paths that started
					 * as relative (.//foo)
					 */
					collapse_slash_runs (path, i);
					marker = 0;
				}
				continue;
			}

			/* Handle `../' or trailing `..' by itself. 
			 * Remove the previous xxx/ part 
			 */
			if (path[i + 1] == '.'
			    && (path[i + 2] == GNOME_VFS_URI_PATH_CHR
				|| path[i + 2] == '\0')) {

				/* ignore ../ at the beginning of a path */
				if (i != 0) {
					marker = find_slash_before_offset (path, i - 1);

					/* Either advance past '/' or point to the first character */
					marker ++;
					if (path [i + 2] == '\0' && marker > 1) {
						/* If we are looking at a /.. at the end of the uri and we
						 * need to eat the last '/' too.
						 */
						 marker--;
					}
					g_assert(marker < i);
					
					if (path[i + 2] == GNOME_VFS_URI_PATH_CHR) {
						/* strip the entire ../ string */
						i++;
					}

					strcpy (path + marker, path + i + 2);
					i = marker;
				} else {
					i = 2;
					if (path[i] == GNOME_VFS_URI_PATH_CHR) {
						i++;
					}
				}
				collapse_slash_runs (path, i);
				continue;
			}
		}
		
		/* advance to the next '/' */
		i = find_next_slash (path, i);

		/* If we didn't find any slashes, then there is nothing left to do. */
		if (i < 0) {
			break;
		}

		marker = i++;
		collapse_slash_runs (path, i);
	}
	return path;
}

static glong
get_max_fds (void)
{
#if defined _SC_OPEN_MAX
	return sysconf (_SC_OPEN_MAX);
#elif defined RLIMIT_NOFILE
	{
		struct rlimit rlimit;

		if (getrlimit (RLIMIT_NOFILE, &rlimit) == 0)
			return rlimit.rlim_max;
		else
			return -1;
	}
#elif defined HAVE_GETDTABLESIZE
	return getdtablesize();
#else
#warning Cannot determine the number of available file descriptors
	return 1024;		/* bogus */
#endif
}

/* Close all the currrently opened file descriptors.  */
static void
shut_down_file_descriptors (void)
{
	glong i, max_fds;

	max_fds = get_max_fds ();

	for (i = 3; i < max_fds; i++)
		close (i);
}

pid_t
gnome_vfs_forkexec (const gchar *file_name,
		    const gchar * const argv[],
		    GnomeVFSProcessOptions options,
		    GnomeVFSProcessInitFunc init_func,
		    gpointer init_data)
{
	pid_t child_pid;

	child_pid = fork ();
	if (child_pid == 0) {
		if (init_func != NULL)
			(* init_func) (init_data);
		if (options & GNOME_VFS_PROCESS_SETSID)
			setsid ();
		if (options & GNOME_VFS_PROCESS_CLOSEFDS)
			shut_down_file_descriptors ();
		if (options & GNOME_VFS_PROCESS_USEPATH)
			execvp (file_name, (char **) argv);
		else
			execv (file_name, (char **) argv);
		_exit (1);
	}

	return child_pid;
}

/**
 * gnome_vfs_process_run_cancellable:
 * @file_name: Name of the executable to run
 * @argv: NULL-terminated argument list
 * @options: Options
 * @cancellation: Cancellation object
 * @return_value: Pointer to an integer that will contain the exit value
 * on return.
 * 
 * Run @file_name with argument list @argv, according to the specified
 * @options.
 * 
 * Return value: 
 **/
GnomeVFSProcessResult
gnome_vfs_process_run_cancellable (const gchar *file_name,
				   const gchar * const argv[],
				   GnomeVFSProcessOptions options,
				   GnomeVFSCancellation *cancellation,
				   guint *exit_value)
{
	pid_t child_pid;

	child_pid = gnome_vfs_forkexec (file_name, argv, options, NULL, NULL);
	if (child_pid == -1)
		return GNOME_VFS_PROCESS_RUN_ERROR;

	while (1) {
		pid_t pid;
		int status;

		pid = waitpid (child_pid, &status, WUNTRACED);
		if (pid == -1) {
			if (errno != EINTR)
				return GNOME_VFS_PROCESS_RUN_ERROR;
			if (gnome_vfs_cancellation_check (cancellation)) {
				*exit_value = 0;
				return GNOME_VFS_PROCESS_RUN_CANCELLED;
			}
		} else if (pid == child_pid) {
			if (WIFEXITED (status)) {
				*exit_value = WEXITSTATUS (status);
				return GNOME_VFS_PROCESS_RUN_OK;
			}
			if (WIFSIGNALED (status)) {
				*exit_value = WTERMSIG (status);
				return GNOME_VFS_PROCESS_RUN_SIGNALED;
			}
			if (WIFSTOPPED (status)) {
				*exit_value = WSTOPSIG (status);
				return GNOME_VFS_PROCESS_RUN_SIGNALED;
			}
		}
	}

}

/**
 * gnome_vfs_create_temp:
 * @prefix: Prefix for the name of the temporary file
 * @name_return: Pointer to a pointer that, on return, will point to
 * the dynamically allocated name for the new temporary file created.
 * @handle_return: Pointer to a variable that will hold a file handle for
 * the new temporary file on return.
 * 
 * Create a temporary file whose name is prefixed with @prefix, and return an
 * open file handle for it in @*handle_return.
 * 
 * Return value: An integer value representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_create_temp (const gchar *prefix,
		       gchar **name_return,
		       GnomeVFSHandle **handle_return)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult result;
	gchar *name;
	gint fd;

	while (1) {
		name = g_strdup_printf("%sXXXXXX", prefix);
		fd = mkstemp(name);

		if (fd < 0)
			return GNOME_VFS_ERROR_INTERNAL;

		fchmod(fd, 0600);
		close(fd);

		result = gnome_vfs_open
			(&handle, name,
			 GNOME_VFS_OPEN_WRITE | GNOME_VFS_OPEN_READ);

		if (result == GNOME_VFS_OK) {
			*name_return = name;
			*handle_return = handle;
			return GNOME_VFS_OK;
		}

		if (result != GNOME_VFS_ERROR_FILE_EXISTS) {
			*name_return = NULL;
			*handle_return = NULL;
			return result;
		}
	}
}

/* The following comes from GNU Wget with minor changes by myself.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */

/* Converts struct tm to time_t, assuming the data in tm is UTC rather
   than local timezone (mktime assumes the latter).

   Contributed by Roger Beeman <beeman@cisco.com>, with the help of
   Mark Baushke <mdb@cisco.com> and the rest of the Gurus at CISCO.  */
static time_t
mktime_from_utc (struct tm *t)
{
	time_t tl, tb;

	tl = mktime (t);
	if (tl == -1)
		return -1;
	tb = mktime (gmtime (&tl));
	return (tl <= tb ? (tl + (tl - tb)) : (tl - (tb - tl)));
}

/* Check whether the result of strptime() indicates success.
   strptime() returns the pointer to how far it got to in the string.
   The processing has been successful if the string is at `GMT' or
   `+X', or at the end of the string.

   In extended regexp parlance, the function returns 1 if P matches
   "^ *(GMT|[+-][0-9]|$)", 0 otherwise.  P being NULL (a valid result of
   strptime()) is considered a failure and 0 is returned.  */
static int
check_end (const gchar *p)
{
	if (!p)
		return 0;
	while (isspace ((unsigned char)*p))
		++p;
	if (!*p
	    || (p[0] == 'G' && p[1] == 'M' && p[2] == 'T')
	    || ((p[0] == '+' || p[1] == '-') && isdigit ((unsigned char)p[1])))
		return 1;
	else
		return 0;
}

/* Convert TIME_STRING time to time_t.  TIME_STRING can be in any of
   the three formats RFC2068 allows the HTTP servers to emit --
   RFC1123-date, RFC850-date or asctime-date.  Timezones are ignored,
   and should be GMT.

   We use strptime() to recognize various dates, which makes it a
   little bit slacker than the RFC1123/RFC850/asctime (e.g. it always
   allows shortened dates and months, one-digit days, etc.).  It also
   allows more than one space anywhere where the specs require one SP.
   The routine should probably be even more forgiving (as recommended
   by RFC2068), but I do not have the time to write one.

   Return the computed time_t representation, or -1 if all the
   schemes fail.

   Needless to say, what we *really* need here is something like
   Marcus Hennecke's atotm(), which is forgiving, fast, to-the-point,
   and does not use strptime().  atotm() is to be found in the sources
   of `phttpd', a little-known HTTP server written by Peter Erikson.  */
gboolean
gnome_vfs_atotm (const gchar *time_string,
		 time_t *value_return)
{
	struct tm t;

	/* Roger Beeman says: "This function dynamically allocates struct tm
	   t, but does no initialization.  The only field that actually
	   needs initialization is tm_isdst, since the others will be set by
	   strptime.  Since strptime does not set tm_isdst, it will return
	   the data structure with whatever data was in tm_isdst to begin
	   with.  For those of us in timezones where DST can occur, there
	   can be a one hour shift depending on the previous contents of the
	   data area where the data structure is allocated."  */
	t.tm_isdst = -1;

	/* Note that under foreign locales Solaris strptime() fails to
	   recognize English dates, which renders this function useless.  I
	   assume that other non-GNU strptime's are plagued by the same
	   disease.  We solve this by setting only LC_MESSAGES in
	   i18n_initialize(), instead of LC_ALL.

	   Another solution could be to temporarily set locale to C, invoke
	   strptime(), and restore it back.  This is slow and dirty,
	   however, and locale support other than LC_MESSAGES can mess other
	   things, so I rather chose to stick with just setting LC_MESSAGES.

	   Also note that none of this is necessary under GNU strptime(),
	   because it recognizes both international and local dates.  */

	/* NOTE: We don't use `%n' for white space, as OSF's strptime uses
	   it to eat all white space up to (and including) a newline, and
	   the function fails if there is no newline (!).

	   Let's hope all strptime() implementations use ` ' to skip *all*
	   whitespace instead of just one (it works that way on all the
	   systems I've tested it on).  */

	/* RFC1123: Thu, 29 Jan 1998 22:12:57 */
	if (check_end (strptime (time_string, "%a, %d %b %Y %T", &t))) {
		*value_return = mktime_from_utc (&t);
		return TRUE;
	}

	/* RFC850:  Thu, 29-Jan-98 22:12:57 */
	if (check_end (strptime (time_string, "%a, %d-%b-%y %T", &t))) {
		*value_return = mktime_from_utc (&t);
		return TRUE;
	}

	/* asctime: Thu Jan 29 22:12:57 1998 */
	if (check_end (strptime (time_string, "%a %b %d %T %Y", &t))) {
		*value_return = mktime_from_utc (&t);
		return TRUE;
	}

	/* Failure.  */
	return FALSE;
}

/* gnome_vfs_istr_has_prefix
 * copy-pasted from Nautilus
 */
gboolean
gnome_vfs_istr_has_prefix (const char *haystack, const char *needle)
{
	const char *h, *n;
	char hc, nc;

	/* Eat one character at a time. */
	h = haystack == NULL ? "" : haystack;
	n = needle == NULL ? "" : needle;
	do {
		if (*n == '\0') {
			return TRUE;
		}
		if (*h == '\0') {
			return FALSE;
		}
		hc = *h++;
		nc = *n++;
		hc = tolower ((guchar) hc);
		nc = tolower ((guchar) nc);
	} while (hc == nc);
	return FALSE;
}

/* gnome_vfs_istr_has_suffix
 * copy-pasted from Nautilus
 */
gboolean
gnome_vfs_istr_has_suffix (const char *haystack, const char *needle)
{
	const char *h, *n;
	char hc, nc;

	if (needle == NULL) {
		return TRUE;
	}
	if (haystack == NULL) {
		return needle[0] == '\0';
	}
		
	/* Eat one character at a time. */
	h = haystack + strlen (haystack);
	n = needle + strlen (needle);
	do {
		if (n == needle) {
			return TRUE;
		}
		if (h == haystack) {
			return FALSE;
		}
		hc = *--h;
		nc = *--n;
		hc = tolower ((guchar) hc);
		nc = tolower ((guchar) nc);
	} while (hc == nc);
	return FALSE;
}
