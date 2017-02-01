/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Dick Porter <dick@cymru.net>
 *
 */

/*
 * Option parsing
 *
 * All ORB options are stripped from the application's argv, and argc is
 * adjusted accordingly
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "options.h"

void ORBit_option_set(ORBit_orb_options *option, const char *val)
{
	g_assert(option!=NULL);

	if(option->type==no_arg && option->arg!=NULL) {
		/* Treat as an int arg with val=1
		*/
		int *int_arg=(int *)option->arg;

		*int_arg=1;
	} else {
		if(option->type==string_arg && option->arg!=NULL) {
			char **str_arg=(char **)option->arg;

			/* free any existing value */
			if(*str_arg!=NULL) {
				g_free(*str_arg);
			}
			*str_arg=g_strdup(val);
		} else if(option->type==int_arg && option->arg!=NULL) {
			int *int_arg=(int *)option->arg;

			*int_arg=atoi(val);
		}

	}
}

void ORBit_option_parse(int *argc, char **argv, ORBit_orb_options *options)
{
	int i,j,numargs;
	char name[1024], *val;
	ORBit_orb_options *search=NULL;
	int *erase;

	numargs=*argc;

	erase=g_new0(int, *argc);

	for(i=1; i< *argc; i++) {
		if(argv[i][0]!='-') {
			if(search==NULL) {
				/* Skip non-option */
				continue;
			} else {
				/* an required option value has been found */
				erase[i]=1;
				numargs-=1;

				if(search->arg==NULL) {
					/* dont store any values, just strip
					 * the argv
					 */
					search=NULL;
					continue;
				}

				ORBit_option_set(search, argv[i]);

				search=NULL;
				continue;
			}
		} else {
			if(search!=NULL &&
			  (search->type==string_arg || search->type==int_arg)) {
				fprintf(stderr, "Option %s requires an argument\n", search->name);
			}
		}

		val=argv[i];
		while(*val && *val=='-')
			val++;

		strncpy(name,val,1023);
		name[1023]='\0';

		val=strchr(name, '=');
		if(val!=NULL) {
			*val++='\0';
		}

		for(search=options;search->name!=NULL;search++) {
			if(!strcmp(name, search->name)) {
				break;
			}
		}

		if(search->name==NULL) {
			/* Didn't find it in our list of interesting options */
			search=NULL;
		} else {
			/* Found it */
			erase[i]=1;
			numargs-=1;

			if(search->type==no_arg || val!=NULL) {
				ORBit_option_set(search, val);
				search=NULL;
			}
		}
	}

	j=1;
	for(i=1; i< *argc; i++) {
		if(erase[i]==1) {
			continue;
		} else {
			if(j<numargs) {
				argv[j++]=argv[i];
			} else {
				argv[j++]='\0';
			}
		}
	}

	*argc=numargs;

	g_free(erase);
}
