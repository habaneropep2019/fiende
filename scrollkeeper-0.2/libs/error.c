/* copyright (C) 2001 Sun Microsystems, Inc.*/

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <scrollkeeper.h>
#include <errno.h>
#include <string.h>
#include <libintl.h>
#include <stdlib.h>
#include <sys/stat.h>

void sk_warning(int verbose, char *format, ...)
{
    va_list ap;
    struct stat buf;
    FILE *fid;
    
    if (stat("/var/log/scrollkeeper.log", &buf) == -1)
    {
    	if (errno == ENOENT)
	{
	    fid = fopen("/var/log/scrollkeeper.log", "w");
	    if (fid == NULL)
	        return;
	}
	else
	    return;
    }
    else
    {
        if (buf.st_size < (1<<24))
	{
	    fid = fopen("/var/log/scrollkeeper.log", "a");
	    if (fid == NULL)
	        return;
	}
	else
	{
	    rename("/var/log/scrollkeeper.log", "/var/log/scrollkeeper.log.1");
	    fid = fopen("/var/log/scrollkeeper.log", "w");
	    if (fid == NULL)
	        return;
	}
    }
    
    

    va_start(ap, format);

    vfprintf(fid, format, ap);
    if (verbose)
        vfprintf(stderr, format, ap);

    va_end(ap);
    fclose(fid);
}

void
check_ptr (void *p, char *name)
{
    if (p == NULL)
    {
        fprintf (stderr, _("%s: out of memory: %s\n"), name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}
