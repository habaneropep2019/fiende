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

#ifndef __DISTRO_H__
#define __DISTRO_H__

#include <glib.h>

typedef struct _Package      Package;
typedef struct _Phylum       Phylum;
typedef struct _Distribution Distribution;

typedef char *(*DistroVersionFunc) (Distribution *distro);
typedef void (*PackageVersionsFunc) (GSList *packages);

struct _Package {
	char *name;
	char *pre_command;
	char *rpm;
	char *deb;
	char *version;     /* this is filled in by the 
			      PackageVersionsFunc, so 
			      should be NULL at define time */
};

struct _Phylum {
	DistroVersionFunc version;
	PackageVersionsFunc packager;
};

struct _Distribution {
	char *name;
	char *version_file;
	Phylum *phylum;
};

extern Phylum debian_phy;
extern Phylum redhat_phy;
extern Phylum turbolinux_phy;
extern Phylum irix_phy;

#endif /* __DISTRO_H__ */
