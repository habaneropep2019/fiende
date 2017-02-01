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
 *  Author: Dick Porter <dick@acm.org>
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include "trace.h"

/*
 * The function to call to handle trace messages, or NULL to use the default
 * of printing to stderr.
 */
#ifdef ORBIT_DEBUG
static int (* TraceCallback)(char *, va_list)=NULL;
static int TraceModules=0;
static ORBit_TraceLevel TraceMaxLevel=0;

const char *ORBit_Trace_levellist[] = {
	"ALERT   ",
	"CRITICAL",
	"ERROR   ",
	"WARNING ",
	"NOTICE  ",
	"INFO    ",
	"DEBUG   "
};

void ORBit_Trace_setCallback(int (*cbf)(char *, va_list))
{
	TraceCallback=cbf;
}

int (*ORBit_Trace_getCallback(void))(char *, va_list)
{
	return(TraceCallback);
}

void ORBit_Trace_setModules(int modules)
{
	TraceModules=modules;
}

void ORBit_Trace_setLevel(ORBit_TraceLevel level)
{
	TraceMaxLevel=level;
}

int ORBit_Trace(ORBit_TraceModule module, ORBit_TraceLevel level, char *fmt, ...)
{
	va_list args;

	if(!BitTest(TraceModules, module))
		return 0;
	if(TraceMaxLevel < level)
		return 0;

	va_start(args, fmt);
	if(TraceCallback!=NULL)
		return((*TraceCallback)(fmt, args));

	fprintf(stderr, "[%s]: ", ORBit_Trace_levellist[level]);

	return vfprintf(stderr, fmt, args);
}
#endif
