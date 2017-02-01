/* This file is part of TCD 2.0.
   cddb.h - Header file for CDDB routines.

   Copyright (C) 1997-98 Tim P. Gerla <timg@rrv.net>
   
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
                                    
   Tim P. Gerla
   RR 1, Box 40
   Climax, MN  56523
   timg@rrv.net
*/


#ifndef TCD_CDDB_H
#define TCD_CDDB_H

int 		tcd_readcddb( cd_struct* cd, char* filename );
int 		tcd_writecddb( cd_struct* cd, char *filename );
int 		cddb_sum(int n);
unsigned long 	cddb_discid( cd_struct *cd );
void tcd_call_cddb_slave(cd_struct *cd, char *package, char *version);
void tcd_call_cddb_submit(cd_struct *cd, char *category, char *service);
int calc_offset(int minute, int second, int frame);

#endif /* TCD_CDDB_H */
