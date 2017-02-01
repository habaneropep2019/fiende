
/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */


#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "logview.h"

#ifndef __LOGRTNS_H__
#define __LOGRTNS_H__

int ReadNPagesUp (Log * lg, Page * pg, int n);
int ReadNPagesDown (Log * lg, Page * pg, int n);
int ReadPageUp (Log * lg, Page * pg);
int ReadPageDown (Log * lg, Page * pg, gboolean exec_actions);
int isLogFile (char *filename);
int isSameDay (time_t day1, time_t day2);
int WasModified (Log *log);
void reverse (char *line);
void ParseLine (char *buff, LogLine * line);
void MoveToMark ();
void ReadLogStats (Log * log);
void UpdateLogStats( Log *log );
void CloseLog (Log * log);
time_t GetDate (char *line);
Log * OpenLogFile (char *filename);

#endif /* __LOGRTNS_H__ */
