/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-parse-ls.c - Routines for parsing output from the `ls' command.

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

   Authors: 1995 Miguel de Icaza
   1995 Jakub Jelinek
   1998 Pavel Machek
   1999 Cleanup by Ettore Perazzoli

   finduid, findgid are from GNU tar.  */

#include <config.h>
#include "gnome-vfs-parse-ls.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <glib.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef TUNMLEN 
#define TUNMLEN 256
#endif
#ifndef TGNMLEN
#define TGNMLEN 256
#endif


/* FIXME bugzilla.eazel.com 1179:
 * remove these globals.  */
static gint saveuid = -993;
static gchar saveuname[TUNMLEN];
static gint my_uid = -993;

static gint savegid = -993;
static gchar savegname[TGNMLEN];
static gint my_gid = -993;

#define myuid	( my_uid < 0? (my_uid = getuid ()): my_uid )
#define	mygid	( my_gid < 0? (my_gid = getgid ()): my_gid )

static gint finduid (gchar *uname)
{
	struct passwd *pw;
	
	if (uname[0] != saveuname[0]/* Quick test w/o proc call */
	    || 0 != strncmp (uname, saveuname, TUNMLEN)) {
		strncpy (saveuname, uname, TUNMLEN);
		pw = getpwnam (uname);
		if (pw) {
			saveuid = pw->pw_uid;
		} else {
			saveuid = myuid;
		}
	}
	return saveuid;
}

static gint findgid (gchar *gname)
{
	struct group *gr;
	
	if (gname[0] != savegname[0]/* Quick test w/o proc call */
	    || 0 != strncmp (gname, savegname, TUNMLEN)) {
		strncpy (savegname, gname, TUNMLEN);
		gr = getgrnam (gname);
		if (gr) {
			savegid = gr->gr_gid;
		} else {
			savegid = mygid;
		}
	}
	return savegid;
}


/* FIXME bugzilla.eazel.com 1188: This is ugly.  */
#define MAXCOLS 30

static gint
vfs_split_text (gchar *p,
		gchar *columns[],
		gint column_ptr[])
{
	gchar *original = p;
	gint  numcols;

	for (numcols = 0; *p && numcols < MAXCOLS; numcols++) {
		while (*p == ' ' || *p == '\r' || *p == '\n') {
			*p = 0;
			p++;
		}
		columns [numcols] = p;
		column_ptr [numcols] = p - original;
		while (*p && *p != ' ' && *p != '\r' && *p != '\n')
			p++;
	}
	return numcols;
}

static gint
is_num (const gchar *s)
{
	if (!s || s[0] < '0' || s[0] > '9')
		return 0;
	return 1;
}

static gint
is_dos_date (gchar *str)
{
	if (strlen (str) == 8 && str[2] == str[5] && strchr ("\\-/", (gint)str[2]) != NULL)
		return 1;

	return 0;
}

static gint
is_week (gchar *str, struct tm *tim)
{
	static gchar *week = "SunMonTueWedThuFriSat";
	gchar *pos;

	if ((pos = strstr (week, str)) != NULL) {
		if (tim != NULL)
			tim->tm_wday = (pos - week)/3;
		return 1;
	}
	return 0;    
}

static gint
is_month (const gchar *str, struct tm *tim)
{
	static gchar *month = "JanFebMarAprMayJunJulAugSepOctNovDec";
	gchar *pos;
    
	if ((pos = strstr (month, str)) != NULL) {
		if (tim != NULL)
			tim->tm_mon = (pos - month)/3;
		return 1;
	}
	return 0;
}

static gint
is_time (const gchar *str, struct tm *tim)
{
	gchar *p, *p2;

	if ((p = strchr (str, ':')) && (p2 = strrchr (str, ':'))) {
		if (p != p2) {
			if (sscanf (str, "%2d:%2d:%2d",
				    &tim->tm_hour, &tim->tm_min, &tim->tm_sec)
			    != 3)
				return 0;
		}
		else {
			if (sscanf (str, "%2d:%2d",
				    &tim->tm_hour, &tim->tm_min)
			    != 2)
				return 0;
		}
	}
	else 
		return 0;
    
	return 1;
}

static gint is_year (const gchar *str, struct tm *tim)
{
	glong year;

	if (strchr (str,':'))
		return 0;

	if (strlen (str) != 4)
		return 0;

	if (sscanf (str, "%ld", &year) != 1)
		return 0;

	if (year < 1900 || year > 3000)
		return 0;

	tim->tm_year = (gint) (year - 1900);

	return 1;
}

/*
 * FIXME bugzilla.eazel.com 1182:
 * this is broken. Consider following entry:
 * -rwx------   1 root     root            1 Aug 31 10:04 2904 1234
 * where "2904 1234" is filename. Well, this code decodes it as year :-(.
 */

static gint
vfs_parse_filetype (gchar c)
{
	switch (c) {
        case 'd':
		return S_IFDIR; 
        case 'b':
		return S_IFBLK;
        case 'c':
		return S_IFCHR;
        case 'l':
		return S_IFLNK;
        case 's':
#ifdef IS_IFSOCK /* And if not, we fall through to IFIFO, which is pretty
		    close. */
		return S_IFSOCK;
#endif
        case 'p':
		return S_IFIFO;
        case 'm':
	case 'n':		/* Don't know what these are :-) */
        case '-':
	case '?':
		return S_IFREG;
        default:
		return -1;
	}
}

static gint
vfs_parse_filemode (const gchar *p)
{
	/* converts rw-rw-rw- into 0666 */
	gint res = 0;

	switch (*(p++)) {
	case 'r':
		res |= 0400; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'w':
		res |= 0200; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'x':
		res |= 0100; break;
	case 's':
		res |= 0100 | S_ISUID; break;
	case 'S':
		res |= S_ISUID; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'r':
		res |= 0040; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'w':
		res |= 0020; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'x':
		res |= 0010; break;
	case 's':
		res |= 0010 | S_ISGID; break;
        case 'l':
	/* Solaris produces these */
	case 'S':
		res |= S_ISGID; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'r':
		res |= 0004; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'w':
		res |= 0002; break;
	case '-':
		break;
	default:
		return -1;
	}

	switch (*(p++)) {
	case 'x':
		res |= 0001; break;
	case 't':
		res |= 0001 | S_ISVTX; break;
	case 'T':
		res |= S_ISVTX; break;
	case '-':
		break;
	default:
		return -1;
	}

	return res;
}

static gint
vfs_parse_filedate (gint idx,
		    gchar *columns[],
		    time_t *t)
{	/* This thing parses from idx in columns[] array */

	gchar *p;
	struct tm tim;
	gint d[3];
	gint	got_year = 0;
	gint current_mon;
	time_t now;
    
	/* Let's setup default time values */
	now = time (NULL);
	tim = *localtime (&now);
	current_mon = tim.tm_mon;
	tim.tm_hour = 0;
	tim.tm_min  = 0;
	tim.tm_sec  = 0;
	tim.tm_isdst = -1; /* Let mktime () try to guess correct dst offset */
    
	p = columns [idx++];
    
	/* We eat weekday name in case of extfs */
	if (is_week (p, &tim))
		p = columns [idx++];

	/* Month name */
	if (is_month (p, &tim)) {
		/* And we expect, it followed by day number */
		if (is_num (columns[idx]))
			tim.tm_mday = (gint)atol (columns [idx++]);
		else
			return 0; /* No day */

	} else {
		/* We usually expect:
		   Mon DD hh:mm
		   Mon DD  YYYY
		   But in case of extfs we allow these date formats:
		   Mon DD YYYY hh:mm
		   Mon DD hh:mm YYYY
		   Wek Mon DD hh:mm:ss YYYY
		   MM-DD-YY hh:mm
		   where Mon is Jan-Dec, DD, MM, YY two digit day, month, year,
		   YYYY four digit year, hh, mm, ss two digit hour, minute
		   or second. */

		/* Here just this special case with MM-DD-YY */
		if (is_dos_date (p)) {
			p[2] = p[5] = '-';
	    
			if (sscanf (p, "%2d-%2d-%2d", &d[0], &d[1], &d[2]) == 3) {
				/*  We expect to get:
				    1. MM-DD-YY
				    2. DD-MM-YY
				    3. YY-MM-DD
				    4. YY-DD-MM  */
		
				/* Hmm... maybe, next time :)*/
		
				/* At last, MM-DD-YY */
				d[0]--; /* Months are zerobased */
				/* Y2K madness */
				if (d[2] < 70)
					d[2] += 100;

				tim.tm_mon  = d[0];
				tim.tm_mday = d[1];
				tim.tm_year = d[2];
				got_year = 1;
			} else
				return 0; /* sscanf failed */
		} else
			return 0; /* unsupported format */
	}

	/* Here we expect to find time and/or year */
    
	if (is_num (columns[idx])) {
		if (is_time (columns[idx], &tim) || (got_year = is_year (columns[idx], &tim))) {
			idx++;

			/* This is a special case for ctime () or Mon DD YYYY hh:mm */
			if (is_num (columns[idx]) && 
			   ((got_year = is_year (columns[idx], &tim)) || is_time (columns[idx], &tim)))
				idx++; /* time & year or reverse */
		} /* only time or date */
	}
	else 
		return 0; /* Nor time or date */

	/*
	 * If the date is less than 6 months in the past, it is shown without year
	 * other dates in the past or future are shown with year but without time
	 * This does not check for years before 1900 ... I don't know, how
	 * to represent them at all
	 */
	if (!got_year &&
	    current_mon < 6 && current_mon < tim.tm_mon && 
	    tim.tm_mon - current_mon >= 6)

		tim.tm_year--;

	if ((*t = mktime (&tim)) < 0)
		*t = 0;
	return idx;
}

gint
gnome_vfs_parse_ls_lga (const gchar *p,
			struct stat *s,
			gchar **filename,
			gchar **linkname)
{
	gchar *columns [MAXCOLS]; /* Points to the string in column n */
	gint column_ptr [MAXCOLS]; /* Index from 0 to the starting positions of the columns */
	gint idx, idx2, num_cols;
	gint i;
	gint nlink;
	gchar *p_copy, *p_pristine;

	if (strncmp (p, "total", 5) == 0)
		return 0;

	p_copy = g_strdup (p);
	if ((i = vfs_parse_filetype (*(p++))) == -1)
		goto error;

	s->st_mode = i;
	if (*p == ' ')	/* Notwell 4 */
		p++;
	if (*p == '[') {
		if (strlen (p) <= 8 || p [8] != ']')
			goto error;
		/* Should parse here the Notwell permissions :) */
		if (S_ISDIR (s->st_mode))
			s->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR
				       | S_IXUSR | S_IXGRP | S_IXOTH);
		else
			s->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
		p += 9;
	} else {
		if ((i = vfs_parse_filemode (p)) == -1)
			goto error;
		s->st_mode |= i;
		p += 9;

		/* This is for an extra ACL attribute (HP-UX) */
		if (*p == '+')
			p++;
	}

	g_free (p_copy);
	p_copy = g_strdup (p);
	p_pristine = g_strdup (p);
	num_cols = vfs_split_text (p_copy, columns, column_ptr);

	nlink = atol (columns [0]);
	if (nlink < 0)
		goto error;

	s->st_nlink = nlink;

	if (!is_num (columns[1]))
		s->st_uid = finduid (columns [1]);
	else
		s->st_uid = (uid_t) atol (columns [1]);

	/* Mhm, the ls -lg did not produce a group field */
	for (idx = 3; idx <= 5; idx++) 
		if (is_month (columns [idx], NULL)
		    || is_week (columns [idx], NULL)
		    || is_dos_date (columns[idx]))
			break;

	if (idx == 6 || (idx == 5
			 && !S_ISCHR (s->st_mode)
			 && !S_ISBLK (s->st_mode)))
		goto error;

	/* We don't have gid */	
	if (idx == 3 || (idx == 4 && (S_ISCHR (s->st_mode)
				      || S_ISBLK (s->st_mode))))
		idx2 = 2;
	else { 
		/* We have gid field */
		if (is_num (columns[2]))
			s->st_gid = (gid_t) atol (columns [2]);
		else
			s->st_gid = findgid (columns [2]);
		idx2 = 3;
	}

	/* This is device */
	if (S_ISCHR (s->st_mode) || S_ISBLK (s->st_mode)) {
		gint maj, min;
	
		if (!is_num (columns[idx2])
		    || sscanf (columns [idx2], " %d,", &maj) != 1)
			goto error;
	
		if (!is_num (columns[++idx2])
		    || sscanf (columns [idx2], " %d", &min) != 1)
			goto error;
	
#ifdef HAVE_ST_RDEV
		s->st_rdev = ((maj & 0xff) << 8) | (min & 0xffff00ff);
#endif
		s->st_size = 0;
	
	} else {
		/* Common file size */
		if (!is_num (columns[idx2]))
			goto error;
	
		s->st_size = (gsize) atol (columns [idx2]);
#ifdef HAVE_ST_RDEV
		s->st_rdev = 0;
#endif
	}

	idx = vfs_parse_filedate (idx, columns, &s->st_mtime);
	if (!idx)
		goto error;
	/* Use resulting time value */
	s->st_atime = s->st_ctime = s->st_mtime;
	s->st_dev = 0;
	s->st_ino = 0;
#ifdef HAVE_ST_BLKSIZE
	s->st_blksize = 512;
#endif
#ifdef HAVE_ST_BLOCKS
	s->st_blocks = (s->st_size + 511) / 512;
#endif

	for (i = idx + 1, idx2 = 0; i < num_cols; i++ ) 
		if (strcmp (columns [i], "->") == 0) {
			idx2 = i;
			break;
		}
    
	if (((S_ISLNK (s->st_mode) 
	      || (num_cols == idx + 3 && s->st_nlink > 1))) /* Maybe a hardlink?
							       (in extfs) */
	    && idx2) {
		gint p;
		gchar *s;
	    
		if (filename) {
			s = g_strndup (p_copy + column_ptr [idx],
				       column_ptr [idx2] - column_ptr [idx] - 1);
			*filename = s;
		}
		if (linkname) {
			s = g_strdup (p_copy + column_ptr [idx2+1]);
			p = strlen (s);
			if (s [p-1] == '\r' || s [p-1] == '\n')
				s [p-1] = 0;
			if (s [p-2] == '\r' || s [p-2] == '\n')
				s [p-2] = 0;
		
			*linkname = s;
		}
	} else {
		/* Extract the filename from the string copy, not from the columns
		 * this way we have a chance of entering hidden directories like ". ."
		 */
		if (filename) {
			/* 
			 *filename = g_strdup (columns [idx++]);
			 */
			gint p;
			gchar *s;

			s = g_strdup (p_pristine + column_ptr [idx]);
			p = strcspn (s, "\r\n");
			s[p] = '\0';

			*filename = s;
		}
		if (linkname)
			*linkname = NULL;
	}
	g_free (p_copy);
	g_free (p_pristine);
	return 1;

 error:
	{
		static gint errorcount = 0;

		if (++errorcount < 5)
			g_warning (_("Could not parse: %s"), p_copy);
		else if (errorcount == 5)
			g_warning (_("More parsing errors will be ignored."));
	}

	if (p_copy != p)		/* Carefull! */
		g_free (p_copy);
	return 0;
}
