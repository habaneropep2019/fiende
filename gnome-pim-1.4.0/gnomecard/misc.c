/* GnomeCard - a graphical contact manager.
 *
 * misc.c: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <gnome.h>
#include <pwd.h>
#include <sys/types.h>

#define PATH_SEP '/'

/* This one stolen from mc (src/utilunix.c) */
extern char *misc_tilde_expand (char *directory)
{
	struct passwd *passwd;
	char *p;
	char *name;
	int  len;
	
	if (*directory != '~')
	  return strdup (directory);
	
	directory++;
	
	p = strchr (directory, PATH_SEP);
	
	/* d = "~" or d = "~/" */
	if (!(*directory) || (*directory == PATH_SEP)){
		passwd = getpwuid (geteuid ());
		p = (*directory == PATH_SEP) ? directory+1 : "";
	} else {
		if (!p){
			p = "";
			passwd = getpwnam (directory);
		} else {
			name = malloc (p - directory + 1);
			strncpy (name, directory, p - directory);
			name [p - directory] = 0;
			passwd = getpwnam (name);
			free (name);
		}
	}
	
	/* If we can't figure the user name, return NULL */
	if (!passwd)
	  return 0;
	
	len = strlen (passwd->pw_dir) + strlen (p) + 2;
	directory = malloc (len);
	strcpy (directory, passwd->pw_dir);
	strcat (directory, PATH_SEP_STR);
	strcat (directory, p);
	return directory;
}

