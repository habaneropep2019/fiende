/* This file is part of TCD 2.0.
   tcd.h - Header file for Curses interface.

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

#ifndef TCD_TCD_H
#define TCD_TCD_H

void 	print_help();
void 	update_tracklist( void );
void 	update_status( void );
int 	get_i_track();
void 	init_colors();
void	phelp( int y, int x, char key, char *func, int toggle, int enabled );
int 	init_disc();
int 	init_curses( void );
void 	update_display( void );
void 	draw_not_always();
void 	sighandler( int sig ); 

#define NOTOG -10

#define C_BLACK		COLOR_PAIR(0)
#define C_RED		COLOR_PAIR(1)
#define C_GREEN		COLOR_PAIR(2)
#define C_YELLOW	COLOR_PAIR(3)
#define C_BLUE		COLOR_PAIR(4)
#define C_MAGENTA	COLOR_PAIR(5)
#define C_CYAN		COLOR_PAIR(6)
#define C_WHITE		COLOR_PAIR(7)

#define TRK_PLAYING	0x01 /* 00000001b */
#define TRK_DATA	0x02 /* 00000010b */
#define TRK_REPEAT	0x04 /* 00000100b */ 

#endif /* TCD_TCD_H */
